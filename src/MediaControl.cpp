/*
* Membrane Software Reference Source License
* Version 2024 Sep 18
* This license is a legal agreement between you and Membrane Software
*
* This license agreement governs use of the accompanying source code. If you use the source code, you accept this license. If you do not accept the license, do not use the source code.
*
* DEFINITIONS
* "compilation" means to compile the code from source code to machine code.
* “non-commercial distribution” means distribution of the code or any compilation of the code, or of any other application or program containing the code or any compilation of the code, where such distribution is not intended for or directed towards commercial advantage or monetary compensation.
* "review" means to access, analyse, test and otherwise review the code as a reference
* "you" means the licensee of rights set out in this license.
*
* GRANT OF RIGHTS
* Subject to the terms of this license, we grant you a non-transferable, non-exclusive, worldwide, royalty-free license to access and use the source code solely for the purposes of review, compilation and non-commercial distribution.
*
* LIMITATIONS
* This license does not grant you any rights to use Membrane Software's name, logo, or trademarks.
*
* If you issue proceedings in any jurisdiction against Membrane Software because you consider Membrane Software has infringed copyright or any patent right in respect of the code (including any joinder or counterclaim), your license to the code is automatically terminated.
*
* This source code is provided by the copyright holders and contributors "as is" and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the copyright holder or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this source code, even if advised of the possibility of such damage.
*
* NO IMPLIED RIGHTS
* All rights not expressly granted by Membrane Software to you in this License Agreement are hereby reserved by Membrane Software and its suppliers. There are no implied rights in this License Agreement.
*
* GOVERNING LAW
* This EULA agreement, and any dispute arising out of or in connection with this EULA agreement, shall be governed by and construed in accordance with the laws of Washington State, United States of America.
*
* QUESTIONS OR ADDITIONAL INFORMATION
* If you have questions regarding this License Agreement, please contact Membrane Software by sending an email to support@membranesoftware.com.
*/
#include "Config.h"
#include "App.h"
#include "SdlUtil.h"
#include "StringList.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Log.h"
#include "UiLog.h"
#include "AppNews.h"
#include "UiText.h"
#include "Prng.h"
#include "TaskGroup.h"
#include "HashMap.h"
#include "Database.h"
#include "RecordStore.h"
#include "CaptureWriter.h"
#include "SystemInterface.h"
#include "MediaItem.h"
#include "PlayMarker.h"
#include "MediaPlaylist.h"
#include "MediaReader.h"
#include "MediaControl.h"

MediaControl *MediaControl::instance = NULL;

constexpr const char *databaseName = "media.db";
constexpr const char *metadataTableName = "MediaMetadata";
constexpr const int metadataVersion = 1;
constexpr const char *thumbnailDirectoryName = "thumbnail";
constexpr const double writeThumbnailImagesProgressPercent = 95.0f;
constexpr const int uiLogMaxMessageAge = (30 * 86400);

MediaControl::MediaControl ()
: isStopped (false)
, isReady (false)
, isConfigured (false)
, mediaThumbnailCount (MediaControl::defaultMediaThumbnailCount)
, configureMediaThumbnailCount (MediaControl::defaultMediaThumbnailCount)
, isTaskCancelled (false)
{
	SdlUtil::createMutex (&statusMutex);
	SdlUtil::createMutex (&taskListMutex);
}
MediaControl::~MediaControl () {
	if (! databasePath.empty ()) {
		Database::instance->close (databasePath);
		databasePath.assign ("");
	}
	SdlUtil::destroyMutex (&statusMutex);
	SdlUtil::destroyMutex (&taskListMutex);
}

void MediaControl::createInstance () {
	if (! MediaControl::instance) {
		MediaControl::instance = new MediaControl ();
	}
}
void MediaControl::freeInstance () {
	if (MediaControl::instance) {
		delete (MediaControl::instance);
		MediaControl::instance = NULL;
	}
}

OpResult MediaControl::start () {
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	agentId = prefs->find (MediaControl::agentIdKey, "");
	prefs->find (MediaControl::mediaSourcePathKey, &mediaSourcePath);
	dataPath = prefs->find (MediaControl::dataPathKey, "");
	mediaThumbnailCount = prefs->find (MediaControl::mediaThumbnailCountKey, MediaControl::defaultMediaThumbnailCount);
	App::instance->unlockPrefs ();

	if (agentId.empty ()) {
		agentId = Prng::instance->getUuid ();
		prefs = App::instance->lockPrefs ();
		prefs->insert (MediaControl::agentIdKey, agentId);
		App::instance->unlockPrefs ();
	}

	isReady = false;
	isConfigured = false;
	if ((! mediaSourcePath.empty ()) && (! dataPath.empty ())) {
		isConfigured = true;
	}
	if (isConfigured) {
		runTask (MediaControl::ReadyTask);
	}
	return (OpResult::Success);
}

void MediaControl::stop () {
	isStopped = true;
	isTaskCancelled = true;
}

OpResult MediaControl::openDatabase () {
	OpResult result;
	int version;

	if (databasePath.empty ()) {
		return (OpResult::InvalidConfigurationError);
	}
	result = Database::instance->open (databasePath);
	if (result == OpResult::Success) {
		result = Database::instance->exec (databasePath, MediaItem::createTableSql);
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (databasePath, PlayMarker::createTableSql);
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (databasePath, MediaPlaylist::createTableSql);
	}
	if (result == OpResult::Success) {
		result = Database::instance->createMetadataTable (databasePath, StdString (metadataTableName));
	}
	if (result == OpResult::Success) {
		version = Database::instance->readMetadataVersion (databasePath, StdString (metadataTableName));
		if (version < metadataVersion) {
			Database::instance->writeMetadataVersion (databasePath, StdString (metadataTableName), metadataVersion);
		}
	}
	return (result);
}

StdString MediaControl::getThumbnailPath (const StdString &mediaId, int64_t thumbnailTimestamp) {
	if (mediaId.empty () || (thumbnailTimestamp < 0)) {
		return (StdString ());
	}
	return (OsUtil::getJoinedPath (dataPath, mediaId, StdString (thumbnailDirectoryName), StdString::createSprintf ("%lli.jpg", (long long int) thumbnailTimestamp)));
}

void MediaControl::lockStatus () {
	SDL_LockMutex (statusMutex);
}
void MediaControl::unlockStatus () {
	++(status.updateCount);
	SDL_UnlockMutex (statusMutex);
}

void MediaControl::getStatus (MediaControl::Status *destStatus, int taskType, MediaControl::TaskResult *taskResult) {
	std::map<int, MediaControl::TaskResult>::iterator pos;

	SDL_LockMutex (statusMutex);
	if (destStatus) {
		*destStatus = status;
	}
	if ((taskType != MediaControl::NoTask) && taskResult) {
		pos = taskResultMap.find (taskType);
		if (pos == taskResultMap.end ()) {
			*taskResult = MediaControl::TaskResult ();
		}
		else {
			*taskResult = pos->second;
		}
	}
	SDL_UnlockMutex (statusMutex);
}

void MediaControl::getStatus (MediaControl::Status *destStatus) {
	if (! destStatus) {
		return;
	}
	SDL_LockMutex (statusMutex);
	*destStatus = status;
	SDL_UnlockMutex (statusMutex);
}

void MediaControl::getStatus (int taskType, MediaControl::TaskResult *taskResult) {
	std::map<int, MediaControl::TaskResult>::iterator pos;

	if (! taskResult) {
		return;
	}
	SDL_LockMutex (statusMutex);
	pos = taskResultMap.find (taskType);
	if (pos == taskResultMap.end ()) {
		*taskResult = MediaControl::TaskResult ();
	}
	else {
		*taskResult = pos->second;
	}
	SDL_UnlockMutex (statusMutex);
}

void MediaControl::clearTaskResult (int taskType) {
	lockStatus ();
	taskResultMap.erase (taskType);
	unlockStatus ();
}

void MediaControl::update (int msElapsed) {
	std::list<MediaControl::Task>::iterator t;
	MediaControl::Status updatestatus;
	bool shouldupdatestatus;

	shouldupdatestatus = false;
	getStatus (&updatestatus);
	SDL_LockMutex (taskListMutex);
	if (! taskList.empty ()) {
		t = taskList.begin ();
		if (t->isEnded) {
			taskList.erase (t);
			shouldupdatestatus = true;
		}
		if (taskList.empty ()) {
			if (updatestatus.isTaskRunning) {
				updatestatus.isTaskRunning = false;
				updatestatus.taskType = MediaControl::NoTask;
				updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Ready).capitalized ());
				shouldupdatestatus = true;
			}
		}
		else {
			if (! updatestatus.isTaskRunning) {
				updatestatus.isTaskRunning = true;
				shouldupdatestatus = true;
			}

			t = taskList.begin ();
			if (! t->isRunning) {
				isTaskCancelled = false;
				t->isRunning = true;
				switch (t->taskType) {
					case MediaControl::PrimeTask: {
						updatestatus.taskType = t->taskType;
						updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Configuring).capitalized ());
						updatestatus.taskText1.assign (UiText::instance->getText (UiTextId::Configuring).capitalized ());
						updatestatus.taskText2.assign ("");
						updatestatus.taskProgressPercent = -1.0f;
						shouldupdatestatus = true;
						TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::applyPrimeSettings, this));
						break;
					}
					case MediaControl::ReadyTask: {
						updatestatus.taskType = t->taskType;
						updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Initializing).capitalized ());
						updatestatus.taskText1.assign (UiText::instance->getText (UiTextId::Initializing).capitalized ());
						updatestatus.taskText2.assign ("");
						updatestatus.taskProgressPercent = -1.0f;
						shouldupdatestatus = true;
						TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::readyMediaControl, this));
						break;
					}
					case MediaControl::ScanTask: {
						updatestatus.taskType = t->taskType;
						updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Scanning).capitalized ());
						updatestatus.taskText1.assign (UiText::instance->getText (UiTextId::Scanning).capitalized ());
						updatestatus.taskText2.assign ("");
						updatestatus.taskProgressPercent = -1.0f;
						shouldupdatestatus = true;
						TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::scanMediaFiles, this));
						break;
					}
					case MediaControl::CleanTask: {
						updatestatus.taskType = t->taskType;
						updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Cleaning).capitalized ());
						updatestatus.taskText1.assign (UiText::instance->getText (UiTextId::Cleaning).capitalized ());
						updatestatus.taskText2.assign ("");
						updatestatus.taskProgressPercent = -1.0f;
						shouldupdatestatus = true;
						TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::cleanMediaData, this));
						break;
					}
					case MediaControl::ConfigureTask: {
						updatestatus.taskType = t->taskType;
						updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Configuring).capitalized ());
						updatestatus.taskText1.assign ("");
						updatestatus.taskText2.assign ("");
						updatestatus.taskProgressPercent = -1.0f;
						shouldupdatestatus = true;
						TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::applyConfigureSettings, this));
						break;
					}
					default: {
						t->isEnded = true;
						break;
					}
				}
			}
		}
	}
	SDL_UnlockMutex (taskListMutex);

	if (shouldupdatestatus) {
		lockStatus ();
		status.taskType = updatestatus.taskType;
		status.isTaskRunning = updatestatus.isTaskRunning;
		status.statusText.assign (updatestatus.statusText);
		status.taskText1.assign (updatestatus.taskText1);
		status.taskText2.assign (updatestatus.taskText2);
		status.taskProgressPercent = updatestatus.taskProgressPercent;
		unlockStatus ();
	}
}

bool MediaControl::isRunningTask (int taskType) {
	std::list<MediaControl::Task>::const_iterator i1, i2;
	bool result;

	result = false;
	SDL_LockMutex (taskListMutex);
	i1 = taskList.cbegin ();
	i2 = taskList.cend ();
	while (i1 != i2) {
		if (i1->taskType == taskType) {
			result = true;
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (taskListMutex);
	return (result);
}

void MediaControl::endTask (int taskType, const StdString &resultText1, const StdString &resultText2, const StdString &uiLogMessage, const char *logErrorMessage) {
	std::list<MediaControl::Task>::iterator i;
	std::map<int, MediaControl::TaskResult>::iterator j;

	if (logErrorMessage) {
		Log::debug ("Task error: taskType=%i %s", taskType, logErrorMessage);
	}
	if (! uiLogMessage.empty ()) {
		UiLog::instance->write (0, "%s", uiLogMessage.c_str ());
	}
	lockStatus ();
	status.taskText1.assign ("");
	status.taskText2.assign ("");
	status.taskProgressPercent = -1.0f;
	j = taskResultMap.find (taskType);
	if (j == taskResultMap.end ()) {
		taskResultMap.insert (std::pair<int, MediaControl::TaskResult> (taskType, MediaControl::TaskResult (resultText1, resultText2)));
	}
	else {
		j->second = MediaControl::TaskResult (resultText1, resultText2);
	}
	unlockStatus ();

	SDL_LockMutex (taskListMutex);
	if (! taskList.empty ()) {
		i = taskList.begin ();
		i->isEnded = true;
	}
	SDL_UnlockMutex (taskListMutex);
}

void MediaControl::runTask (int taskType) {
	if (isRunningTask (taskType)) {
		return;
	}
	lockStatus ();
	taskResultMap.erase (taskType);
	unlockStatus ();

	SDL_LockMutex (taskListMutex);
	taskList.push_back (MediaControl::Task (taskType));
	SDL_UnlockMutex (taskListMutex);
}

void MediaControl::cancelTask (int taskType) {
	std::list<MediaControl::Task>::iterator i1, i2;

	SDL_LockMutex (taskListMutex);
	if (! taskList.empty ()) {
		i1 = taskList.begin ();
		i2 = taskList.end ();
		if (i1->taskType == taskType) {
			if (i1->isRunning) {
				isTaskCancelled = true;
			}
			else {
				taskList.erase (i1);
			}
		}
		else {
			++i1;
			while (i1 != i2) {
				if (i1->taskType == taskType) {
					taskList.erase (i1);
					break;
				}
				++i1;
			}
		}
	}
	SDL_UnlockMutex (taskListMutex);
}

void MediaControl::prime (const StdString &mediaSourcePathValue, const StdString &dataPathValue) {
	if (isRunningTask (MediaControl::PrimeTask)) {
		return;
	}
	primeMediaSourcePath.assign (mediaSourcePathValue);
	primeDataPath.assign (dataPathValue);
	runTask (MediaControl::PrimeTask);
}
void MediaControl::applyPrimeSettings (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;

	it->executeApplyPrimeSettings ();
}
void MediaControl::executeApplyPrimeSettings () {
	HashMap *prefs;

	if (! OsUtil::directoryExists (primeMediaSourcePath)) {
		endTask (MediaControl::PrimeTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaSourceDirectoryNotFoundErrorText));
		return;
	}
	if (! OsUtil::directoryExists (primeDataPath)) {
		endTask (MediaControl::PrimeTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::DataDirectoryNotFoundErrorText));
		return;
	}
	mediaSourcePath.assign (StringList (primeMediaSourcePath));
	dataPath.assign (primeDataPath);
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaControl::mediaSourcePathKey, mediaSourcePath);
	prefs->insert (MediaControl::dataPathKey, dataPath);
	App::instance->unlockPrefs ();

	primeMediaSourcePath.assign ("");
	primeDataPath.assign ("");
	CaptureWriter::instance->setBaseWritePath (dataPath);
	isConfigured = true;
	runTask (MediaControl::ReadyTask);
	endTask (MediaControl::PrimeTask, StdString ("Configuration complete"));
}

void MediaControl::readyMediaControl (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;

	it->executeReadyMediaControl ();
}
void MediaControl::executeReadyMediaControl () {
	OpResult result;
	StdString errmsg;
	int mediacount;
	bool dbexists;

	if (! databasePath.empty ()) {
		Database::instance->close (databasePath);
	}
	databasePath = OsUtil::getJoinedPath (dataPath, databaseName);
	dbexists = OsUtil::fileExists (databasePath);
	result = openDatabase ();
	if (result != OpResult::Success) {
		endTask (MediaControl::ReadyTask, UiText::instance->getText (UiTextId::MediaControlReadyErrorText), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), StdString::createSprintf ("Failed to open media database; databasePath=\"%s\" err=%i", databasePath.c_str (), result).c_str ());
		return;
	}
	mediacount = MediaItem::countDatabaseRecords (databasePath, &errmsg);
	if (mediacount < 0) {
		endTask (MediaControl::ReadyTask, UiText::instance->getText (UiTextId::MediaControlReadyErrorText), UiText::instance->getText (UiTextId::MediaControlReadDataErrorText), StdString (), errmsg.c_str ());
		return;
	}
	UiLog::instance->configure (OsUtil::getJoinedPath (dataPath, StdString (UiLog::databaseName)), uiLogMaxMessageAge);
	AppNews::instance->configure (OsUtil::getJoinedPath (dataPath, StdString (UiLog::databaseName)));
	CaptureWriter::instance->setBaseWritePath (dataPath);
	isReady = true;
	lockStatus ();
	status.statusText.assign (UiText::instance->getText (UiTextId::Ready).capitalized ());
	status.mediaCount = mediacount;
	unlockStatus ();
	if (! dbexists) {
		scan ();
	}
	endTask (MediaControl::ReadyTask, UiText::instance->getText (UiTextId::Ready).capitalized ());
}

bool MediaControl::matchConfiguration (const StringList &mediaSourcePathValue, const StdString &dataPathValue, int mediaThumbnailCountValue) {
	if (! mediaSourcePathValue.equals (mediaSourcePath)) {
		return (false);
	}
	if (! dataPathValue.equals (dataPath)) {
		return (false);
	}
	if (mediaThumbnailCountValue != mediaThumbnailCount) {
		return (false);
	}
	return (true);
}

void MediaControl::configure (const StringList &mediaSourcePathValue, const StdString &dataPathValue, int mediaThumbnailCountValue) {
	if (isRunningTask (MediaControl::ConfigureTask)) {
		return;
	}
	configureMediaSourcePath.assign (mediaSourcePathValue);
	configureDataPath.assign (dataPathValue);
	configureMediaThumbnailCount = mediaThumbnailCountValue;
	runTask (MediaControl::ConfigureTask);
}
void MediaControl::applyConfigureSettings (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;

	it->executeApplyConfigureSettings ();
}
void MediaControl::executeApplyConfigureSettings () {
	StringList files;
	StringList::const_iterator i1, i2;
	StdString path, errmsg, prevpath;
	OpResult result;
	HashMap *prefs;

	if (! dataPath.equals (configureDataPath)) {
		if (! OsUtil::directoryExists (configureDataPath)) {
			endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::DataDirectoryNotFoundErrorText));
			return;
		}
	}
	result = OsUtil::readDirectory (dataPath, &files);
	if (result != OpResult::Success) {
		endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlReadDataErrorText));
		return;
	}
	result = Database::instance->exec (databasePath, PlayMarker::getDeleteAllSql (), &errmsg);
	if (result != OpResult::Success) {
		endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), errmsg.c_str ());
		return;
	}
	result = Database::instance->exec (databasePath, MediaPlaylist::getDeleteAllSql (), &errmsg);
	if (result != OpResult::Success) {
		endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), errmsg.c_str ());
		return;
	}
	result = Database::instance->exec (databasePath, MediaItem::getDeleteAllSql (), &errmsg);
	if (result != OpResult::Success) {
		endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), errmsg.c_str ());
		return;
	}

	i1 = files.cbegin ();
	i2 = files.cend ();
	while (i1 != i2) {
		path = *i1;
		++i1;
		if ((! path.isUuid ()) || (RecordStore::instance->getRecordIdCommand (path) != SystemInterface::CommandId_MediaItem)) {
			continue;
		}
		path = OsUtil::getJoinedPath (dataPath, path);
		result = OsUtil::removeDirectory (path, true);
		if (result != OpResult::Success) {
			Log::debug ("Failed to remove data directory; path=\"%s\"", path.c_str ());
		}
	}

	if (! dataPath.equals (configureDataPath)) {
		prevpath = OsUtil::getJoinedPath (dataPath, StdString (UiLog::databaseName));
		result = UiLog::instance->setDatabasePath (OsUtil::getJoinedPath (configureDataPath, StdString (UiLog::databaseName)));
		if (result != OpResult::Success) {
			endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), StdString::createSprintf ("Failed to create data file; err=%i", result).c_str ());
			return;
		}
		AppNews::instance->configure (OsUtil::getJoinedPath (configureDataPath, StdString (UiLog::databaseName)));
		result = OsUtil::removeFile (prevpath);
		if (result != OpResult::Success) {
			Log::debug ("Failed to remove database file; path=\"%s\"", prevpath.c_str ());
		}

		prevpath.assign (databasePath);
		if (! databasePath.empty ()) {
			Database::instance->close (databasePath);
		}
		databasePath = OsUtil::getJoinedPath (configureDataPath, databaseName);
		result = openDatabase ();
		if (result != OpResult::Success) {
			endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), StdString::createSprintf ("Failed to open media database; databasePath=\"%s\" err=%i", databasePath.c_str (), result).c_str ());
			return;
		}
		result = OsUtil::removeFile (prevpath);
		if (result != OpResult::Success) {
			Log::debug ("Failed to remove database file; path=\"%s\"", prevpath.c_str ());
		}
	}

	mediaSourcePath.assign (configureMediaSourcePath);
	dataPath.assign (configureDataPath);
	mediaThumbnailCount = configureMediaThumbnailCount;
	configureMediaSourcePath.clear ();
	configureDataPath.assign ("");
	configureMediaThumbnailCount = MediaControl::defaultMediaThumbnailCount;
	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaControl::mediaSourcePathKey, mediaSourcePath);
	prefs->insert (MediaControl::dataPathKey, dataPath);
	prefs->insert (MediaControl::mediaThumbnailCountKey, mediaThumbnailCount, MediaControl::defaultMediaThumbnailCount);
	App::instance->unlockPrefs ();
	CaptureWriter::instance->setBaseWritePath (dataPath);

	lockStatus ();
	status.mediaCount = 0;
	unlockStatus ();
	scan ();
	endTask (MediaControl::ConfigureTask, UiText::instance->getText (UiTextId::ConfigurationUpdated).capitalized ());
}

void MediaControl::scan () {
	runTask (MediaControl::ScanTask);
}
void MediaControl::scanMediaFiles (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;

	it->executeScanMediaFiles ();
}
void MediaControl::executeScanMediaFiles () {
	StringList findfiles;
	StringList::const_iterator i1, i2;
	std::list<MediaItem> scanitems;
	std::list<MediaItem>::iterator j1, j2;
	MediaItem item;
	StdString path, errmsg, errtype;
	OpResult result;
	int64_t mtime;
	int filecount, scancount, recordcount, addcount, errorcount;
	bool found;

	UiLog::instance->write (0, "%s", UiText::instance->getText (UiTextId::BeginMediaScan).capitalized ().c_str ());
	recordcount = MediaItem::countDatabaseRecords (databasePath, &errmsg);
	if (recordcount < 0) {
		endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::MediaScanFailed).capitalized (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::MediaScanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	lockStatus ();
	status.taskText2.assign (UiText::instance->getText (UiTextId::ReadingMediaDirectory).capitalized ());
	unlockStatus ();

	i1 = mediaSourcePath.cbegin ();
	i2 = mediaSourcePath.cend ();
	while (i1 != i2) {
		if (isTaskCancelled) {
			endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		executeScanMediaFiles_readDirectory (*i1, &findfiles);
		++i1;
	}
	if (isTaskCancelled) {
		endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
		return;
	}
	if (findfiles.empty ()) {
		endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanComplete).capitalized (), StdString::createSprintf ("0 %s", UiText::instance->getText (UiTextId::NewFilesFound).c_str ()), StdString::createSprintf ("%s: 0 %s", UiText::instance->getText (UiTextId::EndMediaScan).capitalized ().c_str (), UiText::instance->getText (UiTextId::NewFilesFound).c_str ()));
		return;
	}
	findfiles.sort ();
	errorcount = 0;
	i1 = findfiles.cbegin ();
	i2 = findfiles.cend ();
	while (i1 != i2) {
		if (isTaskCancelled) {
			endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		path = *i1;
		++i1;

		mtime = OsUtil::getFileMtime (path);
		if (mtime < 0) {
			++errorcount;
			Log::debug ("Failed to read media file; path=\"%s\" err=\"Error reading file mtime\"", path.c_str ());
			UiLog::instance->write (0, "%s: %s, \"%s\" in directory \"%s\"", UiText::instance->getText (UiTextId::ScanError).capitalized ().c_str (), UiText::instance->getText (UiTextId::FileOpenFailed).capitalized ().c_str (), OsUtil::getPathBasename (path).c_str (), OsUtil::getPathDirname (path).c_str ());
			continue;
		}
		found = item.readDatabaseMediaPathRow (databasePath, &errmsg, path);
		if (! errmsg.empty ()) {
			Log::debug ("Failed to read database record; err=\"%s\"", errmsg.c_str ());
			continue;
		}
		if (found) {
			if (item.mtime == mtime) {
				continue;
			}
			item.clear (item.mediaId);
		}
		else {
			item.clear (RecordStore::instance->getRecordId (SystemInterface::CommandId_MediaItem));
		}
		item.mtime = mtime;
		item.mediaPath.assign (path);
		item.mediaDirname.assign (OsUtil::getPathDirname (path));
		item.name.assign (OsUtil::getPathBasename (path));
		item.sortKey.assign (item.name.lowercased ().filtered (MediaItem::sortKeyCharacters));
		scanitems.push_back (item);
	}
	filecount = (int) scanitems.size ();
	addcount = 0;
	scancount = 0;
	j1 = scanitems.begin ();
	j2 = scanitems.end ();
	while (j1 != j2) {
		if (isTaskCancelled) {
			endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		++scancount;
		lockStatus ();
		status.taskProgressPercent = 0.0f;
		status.taskText2.sprintf ("(%i/%i) ", scancount, filecount);
		status.taskText2.append (OsUtil::getPathBasename (j1->mediaPath));
		unlockStatus ();

		result = executeScanMediaFiles_processFile (j1, &errmsg);
		if (isTaskCancelled) {
			endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		if (result != OpResult::Success) {
			++errorcount;
			Log::debug ("Failed to read media file; path=\"%s\" err=\"%s\"", j1->mediaPath.c_str (), errmsg.c_str ());

			if (result == OpResult::MalformedDataError) {
				errtype = UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized ();
			}
			else if (result == OpResult::FileOperationFailedError) {
				errtype = UiText::instance->getText (UiTextId::FileOperationError).capitalized ();
			}
			else {
				errtype = UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ();
			}
			UiLog::instance->write (0, "%s: %s, \"%s\" in directory \"%s\"", UiText::instance->getText (UiTextId::ScanError).capitalized ().c_str (), errtype.c_str (), OsUtil::getPathBasename (j1->mediaPath).c_str (), OsUtil::getPathDirname (j1->mediaPath).c_str ());
		}
		else {
			++addcount;
		}
		lockStatus ();
		status.mediaCount = recordcount + addcount;
		status.taskProgressPercent = 100.0f;
		unlockStatus ();
		++j1;
	}

	recordcount = MediaItem::countDatabaseRecords (databasePath, &errmsg);
	if (recordcount < 0) {
		endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::MediaScanFailed).capitalized (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::MediaScanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	lockStatus ();
	status.mediaCount = recordcount;
	unlockStatus ();
	endTask (MediaControl::ScanTask, UiText::instance->getText (UiTextId::ScanComplete).capitalized (), UiText::instance->getCountText (addcount, UiTextId::NewFileFound, UiTextId::NewFilesFound), StdString::createSprintf ("%s: %s, %s", UiText::instance->getText (UiTextId::EndMediaScan).capitalized ().c_str (), UiText::instance->getCountText (addcount, UiTextId::NewFileFound, UiTextId::NewFilesFound).c_str (), UiText::instance->getCountText (errorcount, UiTextId::ScanError, UiTextId::ScanErrors).c_str ()));
}
void MediaControl::executeScanMediaFiles_readDirectory (const StdString &scanPath, StringList *destList) {
	OpResult result;
	StringList files;
	StringList::const_iterator i1, i2;
	StdString path;
	int filetype;

	if (isTaskCancelled) {
		return;
	}
	result = OsUtil::readDirectory (scanPath, &files);
	if (result != OpResult::Success) {
		return;
	}
	i1 = files.cbegin ();
	i2 = files.cend ();
	while (i1 != i2) {
		if (isTaskCancelled) {
			break;
		}
		path = *i1;
		++i1;
		if (path.equals (".") || path.equals ("..")) {
			continue;
		}
		path = OsUtil::getJoinedPath (scanPath, path);
		filetype = OsUtil::getFileType (path);
		if (filetype == OsUtil::DirectoryFile) {
			executeScanMediaFiles_readDirectory (path, destList);
		}
		else if (filetype == OsUtil::RegularFile) {
			if (MediaUtil::isMediaFileExtension (OsUtil::getPathExtension (path))) {
				destList->push_back (path);
			}
		}
	}
}
OpResult MediaControl::executeScanMediaFiles_processFile (std::list<MediaItem>::iterator item, StdString *errorMessage) {
	MediaReader reader;
	StdString sql;
	OpResult result;

	reader.setMediaPath (item->mediaPath);
	result = reader.readMetadata ();
	if (result != OpResult::Success) {
		errorMessage->assign (reader.lastErrorMessage.empty () ? "readMetadata failed" : reader.lastErrorMessage.c_str ());
		return (result);
	}
	if (reader.duration <= 0) {
		errorMessage->assign ("Invalid media duration");
		return (OpResult::MalformedDataError);
	}
	if (mediaThumbnailCount >= 0) {
		result = executeScanMediaFiles_writeThumbnailImages (item, errorMessage, reader);
		if (result != OpResult::Success) {
			return (result);
		}
		if (isTaskCancelled) {
			return (OpResult::Success);
		}
	}
	lockStatus ();
	status.taskProgressPercent = writeThumbnailImagesProgressPercent;
	unlockStatus ();

	if (! item->readMediaReader (reader)) {
		errorMessage->assign ("Invalid media metadata");
		return (OpResult::MalformedDataError);
	}
	if (isTaskCancelled) {
		return (OpResult::Success);
	}
	sql = item->getUpsertSql ();
	if (sql.empty ()) {
		errorMessage->assign ("Invalid media metadata");
		return (OpResult::MalformedDataError);
	}
	result = Database::instance->exec (databasePath, sql, errorMessage);
	if (result != OpResult::Success) {
		return (result);
	}
	errorMessage->assign ("");
	return (OpResult::Success);
}
OpResult MediaControl::executeScanMediaFiles_writeThumbnailImages (std::list<MediaItem>::iterator item, StdString *errorMessage, const MediaReader &metadataReader) {
	StdString dirpath;
	MediaReader reader;
	OpResult result;
	double progressdelta;
	int64_t lasttimestamp, seektimestamp, seektimestampdelta;
	int imagecount, maximagecount;

	if (mediaThumbnailCount < 0) {
		return (OpResult::Success);
	}
	if (!(metadataReader.isVideo || metadataReader.hasAudioAlbumArt)) {
		return (OpResult::Success);
	}
	if (metadataReader.duration <= 0) {
		errorMessage->assign ("Invalid media duration");
		return (OpResult::MalformedDataError);
	}
	dirpath = OsUtil::getJoinedPath (dataPath, item->mediaId);
	result = OsUtil::createDirectory (dirpath);
	if (result != OpResult::Success) {
		errorMessage->assign ("Failed to create data directory");
		return (result);
	}
	dirpath = OsUtil::getJoinedPath (dirpath, StdString (thumbnailDirectoryName));
	result = OsUtil::createDirectory (dirpath);
	if (result != OpResult::Success) {
		errorMessage->assign ("Failed to create data directory");
		return (result);
	}

	seektimestamp = 0;
	maximagecount = 0;
	reader.setMediaPath (item->mediaPath);
	if (! metadataReader.isVideo) {
		seektimestampdelta = (int64_t) (((double) metadataReader.duration) * 0.99f / 2.0f);
		progressdelta = writeThumbnailImagesProgressPercent * 0.5f;
		maximagecount = 1;
	}
	else {
		switch (mediaThumbnailCount) {
			case MediaThumbnailEveryHour: {
				seektimestampdelta = 3600 * 1000;
				progressdelta = writeThumbnailImagesProgressPercent * (double) seektimestampdelta / (double) metadataReader.duration;
				break;
			}
			case MediaThumbnailEvery10Minutes: {
				seektimestampdelta = 600 * 1000;
				progressdelta = writeThumbnailImagesProgressPercent * (double) seektimestampdelta / (double) metadataReader.duration;
				break;
			}
			case MediaThumbnailEveryMinute: {
				seektimestampdelta = 60 * 1000;
				progressdelta = writeThumbnailImagesProgressPercent * (double) seektimestampdelta / (double) metadataReader.duration;
				break;
			}
			case MediaThumbnailEvery10Seconds: {
				seektimestampdelta = 10 * 1000;
				progressdelta = writeThumbnailImagesProgressPercent * (double) seektimestampdelta / (double) metadataReader.duration;
				break;
			}
			case MediaThumbnailDivide1: {
				seektimestampdelta = (int64_t) (((double) metadataReader.duration) * 0.99f / 2.0f);
				progressdelta = writeThumbnailImagesProgressPercent * 0.5f;
				maximagecount = 1;
				break;
			}
			case MediaThumbnailDivide48: {
				seektimestampdelta = (int64_t) (((double) metadataReader.duration) * 0.99f / 47.0f);
				progressdelta = writeThumbnailImagesProgressPercent * 0.02f;
				maximagecount = 48;
				break;
			}
			case MediaThumbnailDivide100: {
				seektimestampdelta = (int64_t) (((double) metadataReader.duration) * 0.99f / 99.0f);
				progressdelta = writeThumbnailImagesProgressPercent * 0.01f;
				maximagecount = 100;
				break;
			}
			default: {
				seektimestampdelta = (int64_t) (((double) metadataReader.duration) * 0.99f / 12.0f);
				progressdelta = writeThumbnailImagesProgressPercent / 12.0f;
				maximagecount = 12;
				break;
			}
		}
	}

	if (seektimestampdelta < 1) {
		seektimestampdelta = 1;
	}
	imagecount = 0;
	lasttimestamp = -1;
	while (seektimestamp < metadataReader.duration) {
		if ((maximagecount > 0) && (imagecount >= maximagecount)) {
			break;
		}
		if (isTaskCancelled) {
			break;
		}
		reader.setVideoFrameSeekTimestamp (seektimestamp);
		reader.readVideoFrame ();
		if (! reader.videoFrameData) {
			Log::debug ("Failed to read media file frame; path=\"%s\" err=\"%s\"", item->mediaPath.c_str (), reader.lastErrorMessage.empty () ? "readVideoFrame failed" : reader.lastErrorMessage.c_str ());
			break;
		}
		if (lasttimestamp != reader.videoFrameTimestamp) {
			lasttimestamp = reader.videoFrameTimestamp;
			reader.writeVideoFrameJpeg (OsUtil::getJoinedPath (dirpath, StdString::createSprintf ("%lli.jpg", (long long int) reader.videoFrameTimestamp)));
			if (! reader.lastErrorMessage.empty ()) {
				return (OpResult::FileOperationFailedError);
			}
			item->thumbnailTimestamps.push_back (reader.videoFrameTimestamp);
			++imagecount;
		}

		lockStatus ();
		status.taskProgressPercent += progressdelta;
		unlockStatus ();
		seektimestamp += seektimestampdelta;
	}
	errorMessage->assign ("");
	return (OpResult::Success);
}

void MediaControl::clean () {
	runTask (MediaControl::CleanTask);
}
void MediaControl::cleanMediaData (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;

	it->executeCleanMediaData ();
}
void MediaControl::executeCleanMediaData () {
	OpResult result;
	StdString errmsg;
	int recordcount;
	int64_t filesize;

	UiLog::instance->write (0, "%s", UiText::instance->getText (UiTextId::BeginMediaDataClean).capitalized ().c_str ());
	result = executeCleanMediaData_removeRecords (&recordcount, &errmsg);
	if (result != OpResult::Success) {
		endTask (MediaControl::CleanTask, UiText::instance->getText (UiTextId::CleanFailed).capitalized (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CleanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	result = executeCleanMediaData_removeFiles (&filesize, &errmsg);
	if (result != OpResult::Success) {
		endTask (MediaControl::CleanTask, UiText::instance->getText (UiTextId::CleanFailed).capitalized (), UiText::instance->getText (UiTextId::FileOperationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CleanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::FileOperationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	endTask (MediaControl::CleanTask, UiText::instance->getText (UiTextId::CleanComplete).capitalized (), UiText::instance->getCountText (recordcount, UiTextId::MediaRecordRemoved, UiTextId::MediaRecordsRemoved), StdString::createSprintf ("%s: %s, %s %s", UiText::instance->getText (UiTextId::EndMediaDataClean).capitalized ().c_str (), UiText::instance->getCountText (recordcount, UiTextId::MediaRecordRemoved, UiTextId::MediaRecordsRemoved).c_str (), UiText::instance->getByteCountText (filesize).c_str (), UiText::instance->getText (UiTextId::Freed).c_str ()));
}
OpResult MediaControl::executeCleanMediaData_removeRecords (int *removedRecordCount, StdString *errorMessage) {
	constexpr const int pageSize = 64;
	std::list<MediaItem> items;
	std::list<MediaItem>::const_iterator i1, i2;
	StringList removeids;
	StringList::const_iterator j1, j2;
	StdString sql;
	OpResult result;
	int offset, removecount, recordcount;

	offset = 0;
	removecount = 0;
	while (true) {
		if (! MediaItem::readDatabaseRows (databasePath, errorMessage, &items, StdString (), offset, pageSize)) {
			return (OpResult::SqliteOperationFailedError);
		}
		if (items.empty ()) {
			break;
		}
		i1 = items.cbegin ();
		i2 = items.cend ();
		while (i1 != i2) {
			if (! OsUtil::fileExists (i1->mediaPath)) {
				removeids.push_back (i1->mediaId);
			}
			++i1;
		}
		offset += (int) items.size ();
	}

	j1 = removeids.cbegin ();
	j2 = removeids.cend ();
	while (j1 != j2) {
		sql = MediaItem::getDeleteSql (*j1);
		result = Database::instance->exec (databasePath, sql, errorMessage);
		if (result != OpResult::Success) {
			return (result);
		}
		++removecount;
		++j1;
	}

	recordcount = MediaItem::countDatabaseRecords (databasePath, errorMessage);
	if (recordcount < 0) {
		return (OpResult::SqliteOperationFailedError);
	}
	lockStatus ();
	status.mediaCount = recordcount;
	unlockStatus ();
	if (removedRecordCount) {
		*removedRecordCount = removecount;
	}
	return (OpResult::Success);
}
OpResult MediaControl::executeCleanMediaData_removeFiles (int64_t *removedFileSize, StdString *errorMessage) {
	OpResult result;
	StringList files;
	StringList::const_iterator i1, i2;
	MediaItem item;
	StdString id, path;
	int64_t removesize, filesize;

	result = OsUtil::readDirectory (dataPath, &files);
	if (result != OpResult::Success) {
		errorMessage->assign ("Failed to read data directory");
		return (result);
	}
	removesize = 0;
	i1 = files.cbegin ();
	i2 = files.cend ();
	while (i1 != i2) {
		id = *i1;
		++i1;
		if ((! id.isUuid ()) || (RecordStore::instance->getRecordIdCommand (id) != SystemInterface::CommandId_MediaItem)) {
			continue;
		}
		path = OsUtil::getJoinedPath (dataPath, id);
		if (OsUtil::getFileType (path) != OsUtil::DirectoryFile) {
			continue;
		}
		if (item.readDatabaseMediaIdRow (databasePath, errorMessage, id)) {
			continue;
		}
		if (! errorMessage->empty ()) {
			return (OpResult::SqliteOperationFailedError);
		}
		filesize = OsUtil::getDirectorySize (path);
		if (filesize > 0) {
			removesize += filesize;
		}
		result = OsUtil::removeDirectory (path, true);
		if (result != OpResult::Success) {
			errorMessage->sprintf ("Failed to remove directory; path=\"%s\" result=%i", path.c_str (), result);
			return (result);
		}
	}
	if (removedFileSize) {
		*removedFileSize = removesize;
	}
	return (OpResult::Success);
}
