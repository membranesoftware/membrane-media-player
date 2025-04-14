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
#include "ClassId.h"
#include "SdlUtil.h"
#include "PrefsKey.h"
#include "StringList.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Log.h"
#include "UiLog.h"
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
#include "MediaControlTask.h"
#include "MediaControl.h"

MediaControl *MediaControl::instance = NULL;

constexpr const char *databaseName = "media.db";
constexpr const int metadataVersion = 1;
constexpr const char *thumbnailDirectoryName = "thumbnail";
constexpr const double writeThumbnailImagesProgressPercent = 95.0f;

MediaControl::MediaControl ()
: isStopped (false)
, isReady (false)
, isFilescanConfigured (false)
, isTaskCancelled (false)
, playHistorySize (0)
{
	SdlUtil::createMutex (&statusMutex);
	SdlUtil::createMutex (&databasePathMutex);
	SdlUtil::createMutex (&taskListMutex);
	SdlUtil::createMutex (&playHistoryMutex);
	SdlUtil::createMutex (&playHistoryCallbackMutex);
}
MediaControl::~MediaControl () {
	std::list<MediaControlTask *>::const_iterator i1, i2;

	SDL_LockMutex (taskListMutex);
	i1 = taskList.cbegin ();
	i2 = taskList.cend ();
	while (i1 != i2) {
		(*i1)->release ();
		++i1;
	}
	taskList.clear ();
	SDL_UnlockMutex (taskListMutex);

	SDL_LockMutex (databasePathMutex);
	if (! databasePath.empty ()) {
		Database::instance->close (databasePath);
		databasePath.assign ("");
	}
	SDL_UnlockMutex (databasePathMutex);

	SdlUtil::destroyMutex (&statusMutex);
	SdlUtil::destroyMutex (&databasePathMutex);
	SdlUtil::destroyMutex (&taskListMutex);
	SdlUtil::destroyMutex (&playHistoryMutex);
	SdlUtil::destroyMutex (&playHistoryCallbackMutex);
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
	agentId = prefs->find (PrefsKey::agentId, "");
	mainOptions.savePlayHistory = prefs->find (PrefsKey::savePlayHistory, false);
	mainOptions.savePlaylists = prefs->find (PrefsKey::savePlaylists, false);
	mainOptions.mediaScan = prefs->find (PrefsKey::mediaScan, false);
	mainOptions.dataPath = prefs->find (PrefsKey::mediaDataPath, "");
	prefs->find (PrefsKey::mediaFilescanPath, &(filescanOptions.scanPath));
	filescanOptions.mediaThumbnailCount = prefs->find (PrefsKey::mediaThumbnailCount, MediaControl::defaultMediaThumbnailCount);
	App::instance->unlockPrefs ();

	if (agentId.empty ()) {
		agentId = Prng::instance->getUuid ();
		prefs = App::instance->lockPrefs ();
		prefs->insert (PrefsKey::agentId, agentId);
		App::instance->unlockPrefs ();
	}

	isStopped = false;
	isTaskCancelled = false;
	isReady = false;

	if (mainOptions.dataPath.empty ()) {
		mainOptions.savePlayHistory = false;
		mainOptions.savePlaylists = false;
		mainOptions.mediaScan = false;
	}

	isFilescanConfigured = false;
	if (! filescanOptions.scanPath.empty ()) {
		isFilescanConfigured = true;
	}

	addTask (new MediaControlReadyTask ());
	return (OpResult::Success);
}

void MediaControl::stop () {
	StringList ids;

	isStopped = true;
	isTaskCancelled = true;

	UiLog::instance->removeListener (this);
	SDL_LockMutex (playHistoryMutex);
	ids.assign (playHistoryRecordIds);
	playHistoryRecordIds.clear ();
	playHistoryMediaPathMap.clear ();
	playHistorySize = 0;
	SDL_UnlockMutex (playHistoryMutex);
	RecordStore::instance->remove (ids);
}

StdString MediaControl::getThumbnailPath (const StdString &mediaId, int64_t thumbnailTimestamp) {
	if (mediaId.empty () || (thumbnailTimestamp < 0) || mainOptions.dataPath.empty ()) {
		return (StdString ());
	}
	return (OsUtil::getJoinedPath (mainOptions.dataPath, mediaId, StdString (thumbnailDirectoryName), StdString::createSprintf ("%lli.jpg", (long long int) thumbnailTimestamp)));
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
	if ((taskType >= 0) && taskResult) {
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
	std::list<MediaControlTask *>::const_iterator i;
	MediaControlTask *task;
	MediaControl::Status updatestatus;
	bool shouldupdatestatus;

	shouldupdatestatus = false;
	getStatus (&updatestatus);
	SDL_LockMutex (taskListMutex);
	if (! taskList.empty ()) {
		i = taskList.cbegin ();
		task = *i;
		if (task->isEnded) {
			taskList.erase (i);
			task->release ();
			shouldupdatestatus = true;
		}
		if (taskList.empty ()) {
			if (updatestatus.isTaskRunning) {
				updatestatus.isTaskRunning = false;
				updatestatus.taskType = -1;
				updatestatus.statusText.assign (UiText::instance->getText (UiTextId::Ready).capitalized ());
				shouldupdatestatus = true;
			}
		}
		else {
			if (! updatestatus.isTaskRunning) {
				updatestatus.isTaskRunning = true;
				shouldupdatestatus = true;
			}

			i = taskList.cbegin ();
			task = *i;
			if (! task->isRunning) {
				isTaskCancelled = false;
				task->isRunning = true;

				updatestatus.taskType = task->classId;
				updatestatus.statusText.assign (task->statusText);
				updatestatus.taskText1.assign (task->taskText1);
				updatestatus.taskText2.assign (task->taskText2);
				updatestatus.taskProgressPercent = -1.0f;
				shouldupdatestatus = true;
				TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::executeTask, this));
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

void MediaControl::executeTask (void *itPtr) {
	MediaControl *it = (MediaControl *) itPtr;
	std::list<MediaControlTask *>::const_iterator i;
	MediaControlTask *task;

	task = NULL;
	SDL_LockMutex (it->taskListMutex);
	if (! it->taskList.empty ()) {
		i = it->taskList.cbegin ();
		task = *i;
		task->retain ();
	}
	SDL_UnlockMutex (it->taskListMutex);
	if (! task) {
		return;
	}
	switch (task->classId) {
		case ClassId::MediaControlConfigureMainTask: {
			it->executeConfigureMain ((MediaControlConfigureMainTask *) task);
			break;
		}
		case ClassId::MediaControlConfigureFilescanTask: {
			it->executeConfigureFilescan ((MediaControlConfigureFilescanTask *) task);
			break;
		}
		case ClassId::MediaControlReadyTask: {
			it->executeReady ((MediaControlReadyTask *) task);
			break;
		}
		case ClassId::MediaControlFilescanTask: {
			it->executeFilescan ((MediaControlFilescanTask *) task);
			break;
		}
		case ClassId::MediaControlCleanFilescanTask: {
			it->executeCleanFilescan ((MediaControlCleanFilescanTask *) task);
			break;
		}
	}
	it->endTask (task);
	task->release ();
}

void MediaControl::endTask (MediaControlTask *task) {
	std::map<int, MediaControl::TaskResult>::iterator i;

	if (! task->logErrorMessage.empty ()) {
		Log::debug ("Task error: taskType=%i %s", task->classId, task->logErrorMessage.c_str ());
	}
	if (! task->uiLogMessage.empty ()) {
		UiLog::write (UiLog::NoOptions, "%s", task->uiLogMessage.c_str ());
	}
	lockStatus ();
	status.taskText1.assign ("");
	status.taskText2.assign ("");
	status.taskProgressPercent = -1.0f;
	i = taskResultMap.find (task->classId);
	if (i == taskResultMap.end ()) {
		taskResultMap.insert (std::pair<int, MediaControl::TaskResult> (task->classId, MediaControl::TaskResult (task->isSuccess, task->resultText1, task->resultText2)));
	}
	else {
		i->second = MediaControl::TaskResult (task->isSuccess, task->resultText1, task->resultText2);
	}
	unlockStatus ();
	task->isEnded = true;
}

bool MediaControl::isTaskRunning (int classId) {
	std::list<MediaControlTask *>::const_iterator i1, i2;
	bool result;

	result = false;
	SDL_LockMutex (taskListMutex);
	i1 = taskList.cbegin ();
	i2 = taskList.cend ();
	while (i1 != i2) {
		if ((*i1)->classId == classId) {
			result = true;
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (taskListMutex);
	return (result);
}

void MediaControl::addTask (MediaControlTask *task) {
	task->retain ();
	if (isTaskRunning (task->classId)) {
		task->release ();
		return;
	}

	lockStatus ();
	taskResultMap.erase (task->classId);
	unlockStatus ();

	task->isRunning = false;
	task->isEnded = false;
	SDL_LockMutex (taskListMutex);
	taskList.push_back (task);
	SDL_UnlockMutex (taskListMutex);
}

void MediaControl::cancelTask (int taskType) {
	std::list<MediaControlTask *>::const_iterator i1, i2;
	MediaControlTask *task;

	SDL_LockMutex (taskListMutex);
	if (! taskList.empty ()) {
		i1 = taskList.cbegin ();
		i2 = taskList.cend ();
		task = *i1;
		if (task->classId == taskType) {
			if (task->isRunning) {
				isTaskCancelled = true;
			}
			else {
				taskList.erase (i1);
				task->release ();
			}
		}
		else {
			++i1;
			while (i1 != i2) {
				task = *i1;
				if (task->classId == taskType) {
					taskList.erase (i1);
					task->release ();
					break;
				}
				++i1;
			}
		}
	}
	SDL_UnlockMutex (taskListMutex);
}

OpResult MediaControl::openDatabase (StdString *pathValue) {
	StdString path;
	OpResult result;

	if (! isReady) {
		return (OpResult::InvalidStateError);
	}
	SDL_LockMutex (databasePathMutex);
	path.assign (databasePath);
	SDL_UnlockMutex (databasePathMutex);
	if (path.empty ()) {
		return (OpResult::InvalidStateError);
	}
	result = Database::instance->open (path);
	if (result != OpResult::Success) {
		return (result);
	}
	if (pathValue) {
		pathValue->assign (path);
	}
	return (OpResult::Success);
}

void MediaControl::execDatabase (const StringList &sql) {
	StdString path;
	MediaControlDatabaseExecTask *task;

	if (sql.empty () || (! isReady)) {
		return;
	}
	SDL_LockMutex (databasePathMutex);
	path.assign (databasePath);
	SDL_UnlockMutex (databasePathMutex);
	if (path.empty ()) {
		return;
	}
	task = new MediaControlDatabaseExecTask (path, sql);
	task->retain ();
	TaskGroup::instance->run (TaskGroup::RunContext (MediaControl::executeDatabaseExec, task, App::databaseWriteQueueId));
}

bool MediaControl::matchMainConfiguration (const MainOptions &options) {
	if (options.savePlayHistory != mainOptions.savePlayHistory) {
		return (false);
	}
	if (options.savePlaylists != mainOptions.savePlaylists) {
		return (false);
	}
	if (options.mediaScan != mainOptions.mediaScan) {
		return (false);
	}
	if (! options.dataPath.equals (mainOptions.dataPath)) {
		return (false);
	}
	return (true);
}

void MediaControl::configureMain (const MediaControl::MainOptions &options) {
	addTask (new MediaControlConfigureMainTask (options));
}
void MediaControl::executeConfigureMain (MediaControlConfigureMainTask *task) {
	HashMap *prefs;
	StdString dbpath;

	if (matchMainConfiguration (task->options)) {
		prefs = App::instance->lockPrefs ();
		prefs->insert (PrefsKey::skipPrimePanel, true);
		App::instance->unlockPrefs ();
		task->setResult (true, UiText::instance->getText (UiTextId::ConfigurationUpdated).capitalized ());
		return;
	}
	if (task->options.savePlayHistory || task->options.savePlaylists || task->options.mediaScan) {
		if (task->options.dataPath.empty ()) {
			task->setResult (false, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::DataDirectoryNotFoundErrorText));
			return;
		}
		if (! OsUtil::directoryExists (task->options.dataPath)) {
			task->setResult (false, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::DataDirectoryNotFoundErrorText));
			return;
		}
	}
	if (! mainOptions.dataPath.equals (task->options.dataPath)) {
		SDL_LockMutex (databasePathMutex);
		dbpath.assign (databasePath);
		databasePath.assign ("");
		SDL_UnlockMutex (databasePathMutex);
		if (! dbpath.empty ()) {
			Database::instance->close (dbpath);
			OsUtil::removeFile (dbpath);
		}
		if (! mainOptions.dataPath.empty ()) {
			removeMediaItemDirectories (mainOptions.dataPath);
			OsUtil::removeFile (OsUtil::getJoinedPath (mainOptions.dataPath, databaseName));
		}
	}

	mainOptions = task->options;
	prefs = App::instance->lockPrefs ();
	prefs->insert (PrefsKey::savePlayHistory, mainOptions.savePlayHistory, false);
	prefs->insert (PrefsKey::savePlaylists, mainOptions.savePlaylists, false);
	prefs->insert (PrefsKey::mediaScan, mainOptions.mediaScan, false);
	prefs->insert (PrefsKey::mediaDataPath, mainOptions.dataPath, "");
	prefs->insert (PrefsKey::skipPrimePanel, true);
	App::instance->unlockPrefs ();

	task->setResult (true, UiText::instance->getText (UiTextId::ConfigurationUpdated).capitalized ());
	isReady = false;
	addTask (new MediaControlReadyTask ());
}

bool MediaControl::matchFilescanConfiguration (const MediaControl::FilescanOptions &options) {
	if (! options.scanPath.equals (filescanOptions.scanPath)) {
		return (false);
	}
	if (options.mediaThumbnailCount != filescanOptions.mediaThumbnailCount) {
		return (false);
	}
	return (true);
}
void MediaControl::configureFilescan (const MediaControl::FilescanOptions &options) {
	addTask (new MediaControlConfigureFilescanTask (options));
}
void MediaControl::executeConfigureFilescan (MediaControlConfigureFilescanTask *task) {
	HashMap *prefs;
	StdString dbpath, errmsg, basepath;
	OpResult result;

	if (matchFilescanConfiguration (task->options)) {
		task->setResult (true, UiText::instance->getText (UiTextId::ConfigurationUpdated).capitalized ());
		return;
	}
	if (task->options.scanPath.empty ()) {
		task->setResult (false, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaSourceDirectoryNotFoundErrorText));
		return;
	}
	basepath.assign (mainOptions.dataPath);
	if (basepath.empty ()) {
		task->setResult (false, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::DataDirectoryNotFoundErrorText));
		return;
	}
	result = openDatabase (&dbpath);
	if (result == OpResult::Success) {
		result = Database::instance->exec (dbpath, PlayMarker::getDeleteAllSql (MediaControl::playMarkerTableName), &errmsg);
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (dbpath, MediaPlaylist::getDeleteAllSql (MediaControl::playlistTableName), &errmsg);
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (dbpath, MediaItem::getDeleteAllSql (MediaControl::filescanTableName), &errmsg);
	}
	if (! dbpath.empty ()) {
		Database::instance->close (dbpath);
	}
	if (result != OpResult::Success) {
		task->setResult (false, UiText::instance->getText (UiTextId::ConfigurationError).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), errmsg.c_str ());
		return;
	}
	removeMediaItemDirectories (basepath);

	filescanOptions.scanPath.assign (task->options.scanPath);
	filescanOptions.mediaThumbnailCount = task->options.mediaThumbnailCount;

	prefs = App::instance->lockPrefs ();
	prefs->insert (PrefsKey::mediaFilescanPath, filescanOptions.scanPath);
	prefs->insert (PrefsKey::mediaThumbnailCount, filescanOptions.mediaThumbnailCount, MediaControl::defaultMediaThumbnailCount);
	App::instance->unlockPrefs ();

	isFilescanConfigured = true;
	App::instance->showNotification (UiText::instance->getText (UiTextId::MediaFilescanConfiguredText));
	filescan ();
	task->setResult (true, UiText::instance->getText (UiTextId::ConfigurationUpdated).capitalized ());
}

void MediaControl::executeReady (MediaControlReadyTask *task) {
	OpResult result;
	StdString errmsg;
	int mediacount;
	StdString dbpath;

	isReady = false;
	mediacount = 0;
	SDL_LockMutex (databasePathMutex);
	dbpath.assign (databasePath);
	databasePath.assign ("");
	SDL_UnlockMutex (databasePathMutex);
	if (! dbpath.empty ()) {
		Database::instance->close (dbpath);
		dbpath.assign ("");
	}

	if (mainOptions.savePlayHistory || mainOptions.savePlaylists || mainOptions.mediaScan) {
		if (mainOptions.dataPath.empty () || (! OsUtil::directoryExists (mainOptions.dataPath))) {
			mainOptions.savePlayHistory = false;
			mainOptions.savePlaylists = false;
			mainOptions.mediaScan = false;
		}
	}
	if (!(mainOptions.savePlayHistory || mainOptions.savePlaylists || mainOptions.mediaScan)) {
		CaptureWriter::instance->setBaseWritePath (StdString ());
		UiLog::instance->removeListener (this);
	}
	else {
		dbpath = OsUtil::getJoinedPath (mainOptions.dataPath, databaseName);
		result = executeReady_openDatabase (dbpath);
		if (result != OpResult::Success) {
			task->setResult (false, UiText::instance->getText (UiTextId::MediaControlReadyErrorText), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString (), StdString::createSprintf ("Failed to open media database; databasePath=\"%s\" err=%i", dbpath.c_str (), result).c_str ());
			return;
		}
		CaptureWriter::instance->setBaseWritePath (mainOptions.dataPath);

		mediacount = MediaItem::countDatabaseRecords (dbpath, MediaControl::filescanTableName, &errmsg);
		if (mediacount < 0) {
			mediacount = 0;
			Log::debug ("Failed to load media data; err=%s", errmsg.c_str ());
		}

		if (! UiLog::instance->loadMessages (dbpath, MediaControl::uiLogTableName, &errmsg)) {
			Log::debug ("Failed to load media data; err=%s", errmsg.c_str ());
		}
		else {
			UiLog::instance->addListener (this, MediaControl::uiLogMessageReceived);
		}

		appNews.readRecord (dbpath, MediaControl::appNewsTableName);

		SDL_LockMutex (databasePathMutex);
		databasePath.assign (dbpath);
		SDL_UnlockMutex (databasePathMutex);

		if (mainOptions.savePlayHistory) {
			executeReady_loadPlayHistory ();
		}
	}

	isReady = true;
	lockStatus ();
	status.statusText.assign (UiText::instance->getText (UiTextId::Ready).capitalized ());
	status.mediaCount = mediacount;
	unlockStatus ();
	task->setResult (true, UiText::instance->getText (UiTextId::Ready).capitalized ());
}
OpResult MediaControl::executeReady_openDatabase (const StdString &readyDatabasePath) {
	OpResult result;
	int version;

	if (readyDatabasePath.empty ()) {
		return (OpResult::InvalidConfigurationError);
	}
	result = Database::instance->open (readyDatabasePath);
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, MediaItem::getCreateTableSql (MediaControl::filescanTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, MediaItem::getCreateTableSql (MediaControl::historyTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, PlayMarker::getCreateTableSql (MediaControl::playMarkerTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, MediaPlaylist::getCreateTableSql (MediaControl::playlistTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, UiLog::getCreateTableSql (MediaControl::uiLogTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->exec (readyDatabasePath, AppNews::getCreateTableSql (MediaControl::appNewsTableName));
	}
	if (result == OpResult::Success) {
		result = Database::instance->createMetadataTable (readyDatabasePath, StdString (MediaControl::metadataTableName));
	}
	if (result == OpResult::Success) {
		version = Database::instance->readMetadataVersion (readyDatabasePath, StdString (MediaControl::metadataTableName));
		if (version < metadataVersion) {
			Database::instance->writeMetadataVersion (readyDatabasePath, StdString (MediaControl::metadataTableName), metadataVersion);
		}
	}
	return (result);
}
void MediaControl::executeReady_loadPlayHistory () {
	StdString errmsg;
	OpResult result;

	SDL_LockMutex (playHistoryMutex);
	playHistoryLoadIds.assign (playHistoryRecordIds);
	playHistoryRecordIds.clear ();
	playHistoryMediaPathMap.clear ();
	playHistorySize = 0;
	SDL_UnlockMutex (playHistoryMutex);
	RecordStore::instance->remove (playHistoryLoadIds);
	executeRemoveRecordCallbacks (playHistoryLoadIds);
	playHistoryLoadIds.clear ();

	result = Database::instance->exec (databasePath, MediaItem::getSelectAllSql (MediaControl::historyTableName), &errmsg, MediaControl::executeReady_loadPlayHistory_row, this);
	if (result != OpResult::Success) {
		Log::debug ("Failed to load media data; err=%s", errmsg.c_str ());
		return;
	}
	executeAddRecordCallbacks (playHistoryLoadIds);
	playHistoryLoadIds.clear ();
}
int MediaControl::executeReady_loadPlayHistory_row (void *itPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaControl *it = (MediaControl *) itPtr;
	MediaItem m;
	Json *record;

	if (! m.copyDatabaseRowValues (columnCount, columnValues, columnNames)) {
		return (-1);
	}
	record = m.createRecord (it->agentId);
	RecordStore::instance->insert (record, true);
	delete (record);

	it->playHistoryLoadIds.push_back (m.id);
	SDL_LockMutex (it->playHistoryMutex);
	it->playHistoryRecordIds.push_back (m.id);
	it->playHistoryMediaPathMap.insert (m.mediaPath, m.id);
	++(it->playHistorySize);
	SDL_UnlockMutex (it->playHistoryMutex);
	return (0);
}

void MediaControl::filescan () {
	addTask (new MediaControlFilescanTask ());
}
void MediaControl::executeFilescan (MediaControlFilescanTask *task) {
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

	if ((! isReady) || databasePath.empty ()) {
		task->setResult (false, UiText::instance->getText (UiTextId::MediaScanFailed).capitalized (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::MediaScanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	UiLog::write (UiLog::NoOptions, "%s", UiText::instance->getText (UiTextId::BeginMediaScan).capitalized ().c_str ());
	recordcount = MediaItem::countDatabaseRecords (databasePath, MediaControl::filescanTableName, &errmsg);
	if (recordcount < 0) {
		task->setResult (false, UiText::instance->getText (UiTextId::MediaScanFailed).capitalized (), UiText::instance->getText (UiTextId::MediaControlReadDataErrorText), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::MediaScanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::MediaControlReadDataErrorText).c_str ()), errmsg.c_str ());
		return;
	}
	lockStatus ();
	status.taskText2.assign (UiText::instance->getText (UiTextId::ReadingMediaDirectory).capitalized ());
	unlockStatus ();

	i1 = filescanOptions.scanPath.cbegin ();
	i2 = filescanOptions.scanPath.cend ();
	while (i1 != i2) {
		if (isTaskCancelled) {
			task->setResult (true, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		executeFilescan_readDirectory (*i1, &findfiles);
		++i1;
	}
	if (isTaskCancelled) {
		task->setResult (true, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
		return;
	}
	if (findfiles.empty ()) {
		task->setResult (true, UiText::instance->getText (UiTextId::ScanComplete).capitalized (), StdString::createSprintf ("0 %s", UiText::instance->getText (UiTextId::NewFilesFound).c_str ()), StdString::createSprintf ("%s: 0 %s", UiText::instance->getText (UiTextId::EndMediaScan).capitalized ().c_str (), UiText::instance->getText (UiTextId::NewFilesFound).c_str ()));
		return;
	}
	findfiles.sort ();
	errorcount = 0;
	i1 = findfiles.cbegin ();
	i2 = findfiles.cend ();
	while (i1 != i2) {
		if (isTaskCancelled) {
			task->setResult (true, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		path = *i1;
		++i1;

		mtime = OsUtil::getFileMtime (path);
		if (mtime < 0) {
			++errorcount;
			Log::debug ("Failed to read media file; path=\"%s\" err=\"Error reading file mtime\"", path.c_str ());
			UiLog::write (UiLog::NoOptions, "%s: %s, \"%s\" in directory \"%s\"", UiText::instance->getText (UiTextId::ScanError).capitalized ().c_str (), UiText::instance->getText (UiTextId::FileOpenFailed).capitalized ().c_str (), OsUtil::getPathBasename (path).c_str (), OsUtil::getPathDirname (path).c_str ());
			continue;
		}
		found = item.readDatabaseMediaPathRow (databasePath, MediaControl::filescanTableName, NULL, path);
		if (found) {
			if (item.mtime == mtime) {
				continue;
			}
			item.clear (item.id);
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
			task->setResult (true, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
			return;
		}
		++scancount;
		lockStatus ();
		status.taskProgressPercent = 0.0f;
		status.taskText2.sprintf ("(%i/%i) ", scancount, filecount);
		status.taskText2.append (OsUtil::getPathBasename (j1->mediaPath));
		unlockStatus ();

		result = executeFilescan_processFile (j1, &errmsg);
		if (isTaskCancelled) {
			task->setResult (true, UiText::instance->getText (UiTextId::ScanCancelled).capitalized (), StdString (), UiText::instance->getText (UiTextId::MediaScanCancelled).capitalized ());
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
			UiLog::write (UiLog::NoOptions, "%s: %s, \"%s\" in directory \"%s\"", UiText::instance->getText (UiTextId::ScanError).capitalized ().c_str (), errtype.c_str (), OsUtil::getPathBasename (j1->mediaPath).c_str (), OsUtil::getPathDirname (j1->mediaPath).c_str ());
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

	recordcount = MediaItem::countDatabaseRecords (databasePath, MediaControl::filescanTableName, &errmsg);
	if (recordcount < 0) {
		task->setResult (false, UiText::instance->getText (UiTextId::MediaScanFailed).capitalized (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::MediaScanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::MediaControlWriteDataErrorText).c_str ()), errmsg.c_str ());
		return;
	}
	lockStatus ();
	status.mediaCount = recordcount;
	unlockStatus ();
	task->setResult (true, UiText::instance->getText (UiTextId::ScanComplete).capitalized (), UiText::instance->getCountText (addcount, UiTextId::NewFileFound, UiTextId::NewFilesFound), StdString::createSprintf ("%s: %s, %s", UiText::instance->getText (UiTextId::EndMediaScan).capitalized ().c_str (), UiText::instance->getCountText (addcount, UiTextId::NewFileFound, UiTextId::NewFilesFound).c_str (), UiText::instance->getCountText (errorcount, UiTextId::ScanError, UiTextId::ScanErrors).c_str ()));
}
void MediaControl::executeFilescan_readDirectory (const StdString &scanPath, StringList *destList) {
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
			executeFilescan_readDirectory (path, destList);
		}
		else if (filetype == OsUtil::RegularFile) {
			if (MediaUtil::isMediaFileExtension (OsUtil::getPathExtension (path))) {
				destList->push_back (path);
			}
		}
	}
}
OpResult MediaControl::executeFilescan_processFile (std::list<MediaItem>::iterator item, StdString *errorMessage) {
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
	if (filescanOptions.mediaThumbnailCount >= 0) {
		result = executeFilescan_writeThumbnailImages (item, errorMessage, reader);
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
	sql = item->getUpsertSql (MediaControl::filescanTableName);
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
OpResult MediaControl::executeFilescan_writeThumbnailImages (std::list<MediaItem>::iterator item, StdString *errorMessage, const MediaReader &metadataReader) {
	StdString dirpath;
	MediaReader reader;
	OpResult result;
	double progressdelta;
	int64_t lasttimestamp, seektimestamp, seektimestampdelta;
	int imagecount, maximagecount;

	if (filescanOptions.mediaThumbnailCount < 0) {
		return (OpResult::Success);
	}
	if (!(metadataReader.isVideo || metadataReader.hasAudioAlbumArt)) {
		return (OpResult::Success);
	}
	if (metadataReader.duration <= 0) {
		errorMessage->assign ("Invalid media duration");
		return (OpResult::MalformedDataError);
	}
	dirpath = OsUtil::getJoinedPath (mainOptions.dataPath, item->id);
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
		switch (filescanOptions.mediaThumbnailCount) {
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

void MediaControl::cleanFilescan () {
	addTask (new MediaControlCleanFilescanTask ());
}
void MediaControl::executeCleanFilescan (MediaControlCleanFilescanTask *task) {
	OpResult result;
	StdString errmsg;
	int recordcount;
	int64_t filesize;

	if ((! isReady) || databasePath.empty ()) {
		task->setResult (true, UiText::instance->getText (UiTextId::CleanComplete).capitalized ());
		return;
	}
	UiLog::write (UiLog::NoOptions, "%s", UiText::instance->getText (UiTextId::BeginMediaDataClean).capitalized ().c_str ());
	result = executeCleanFilescan_removeRecords (&recordcount, &errmsg);
	if (result != OpResult::Success) {
		task->setResult (false, UiText::instance->getText (UiTextId::CleanFailed).capitalized (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CleanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::InternalApplicationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	result = executeCleanFilescan_removeFiles (&filesize, &errmsg);
	if (result != OpResult::Success) {
		task->setResult (false, UiText::instance->getText (UiTextId::CleanFailed).capitalized (), UiText::instance->getText (UiTextId::FileOperationError).capitalized (), StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CleanFailed).capitalized ().c_str (), UiText::instance->getText (UiTextId::FileOperationError).capitalized ().c_str ()), errmsg.c_str ());
		return;
	}
	task->setResult (true, UiText::instance->getText (UiTextId::CleanComplete).capitalized (), UiText::instance->getCountText (recordcount, UiTextId::MediaRecordRemoved, UiTextId::MediaRecordsRemoved), StdString::createSprintf ("%s: %s, %s %s", UiText::instance->getText (UiTextId::EndMediaDataClean).capitalized ().c_str (), UiText::instance->getCountText (recordcount, UiTextId::MediaRecordRemoved, UiTextId::MediaRecordsRemoved).c_str (), UiText::instance->getByteCountText (filesize).c_str (), UiText::instance->getText (UiTextId::Freed).c_str ()));
}
OpResult MediaControl::executeCleanFilescan_removeRecords (int *removedRecordCount, StdString *errorMessage) {
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
		if (! MediaItem::readDatabaseRows (databasePath, MediaControl::filescanTableName, errorMessage, &items, StdString (), offset, pageSize)) {
			return (OpResult::SqliteOperationFailedError);
		}
		if (items.empty ()) {
			break;
		}
		i1 = items.cbegin ();
		i2 = items.cend ();
		while (i1 != i2) {
			if (! OsUtil::fileExists (i1->mediaPath)) {
				removeids.push_back (i1->id);
			}
			++i1;
		}
		offset += (int) items.size ();
	}

	j1 = removeids.cbegin ();
	j2 = removeids.cend ();
	while (j1 != j2) {
		sql = MediaItem::getDeleteSql (MediaControl::filescanTableName, *j1);
		result = Database::instance->exec (databasePath, sql, errorMessage);
		if (result != OpResult::Success) {
			return (result);
		}
		++removecount;
		++j1;
	}

	recordcount = MediaItem::countDatabaseRecords (databasePath, MediaControl::filescanTableName, errorMessage);
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
OpResult MediaControl::executeCleanFilescan_removeFiles (int64_t *removedFileSize, StdString *errorMessage) {
	OpResult result;
	StringList files;
	StringList::const_iterator i1, i2;
	MediaItem item;
	StdString id, path;
	int64_t removesize, filesize;

	result = OsUtil::readDirectory (mainOptions.dataPath, &files);
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
		path = OsUtil::getJoinedPath (mainOptions.dataPath, id);
		if (OsUtil::getFileType (path) != OsUtil::DirectoryFile) {
			continue;
		}
		if (item.readDatabaseMediaIdRow (databasePath, MediaControl::filescanTableName, errorMessage, id)) {
			continue;
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

void MediaControl::executeDatabaseExec (void *taskPtr) {
	MediaControlDatabaseExecTask *task = (MediaControlDatabaseExecTask *) taskPtr;
	StdString dbpath, errmsg;
	OpResult result;

	if (MediaControl::instance->openDatabase (&dbpath) == OpResult::Success) {
		if (task->databasePath.equals (dbpath)) {
			result = Database::instance->execTransaction (dbpath, task->sql, &errmsg);
			if (result != OpResult::Success) {
				Log::debug ("Failed to write media data; err=%i,%s", result, errmsg.c_str ());
			}
		}
		Database::instance->close (dbpath);
	}
	task->release ();
}

void MediaControl::removeMediaItemDirectories (const StdString &targetPath) {
	StringList files;
	StringList::const_iterator i1, i2;
	StdString path;
	OpResult result;

	if (targetPath.empty ()) {
		return;
	}
	result = OsUtil::readDirectory (targetPath, &files);
	if (result != OpResult::Success) {
		return;
	}
	i1 = files.cbegin ();
	i2 = files.cend ();
	while (i1 != i2) {
		path = *i1;
		++i1;
		if (path.isUuid () && (RecordStore::instance->getRecordIdCommand (path) == SystemInterface::CommandId_MediaItem)) {
			path = OsUtil::getJoinedPath (targetPath, path);
			result = OsUtil::removeDirectory (path, true);
			if (result != OpResult::Success) {
				Log::debug ("Failed to remove data directory; path=\"%s\" err=%i", path.c_str (), result);
			}
		}
	}
}

void MediaControl::addPlayHistoryListener (void *callbackData, MediaControl::PlayHistoryCallback addRecordCallback, MediaControl::PlayHistoryCallback removeRecordCallback) {
	std::list<MediaControl::PlayHistoryCallbackContext>::iterator i1, i2;
	bool found;

	SDL_LockMutex (playHistoryCallbackMutex);
	found = false;
	i1 = playHistoryCallbackList.begin ();
	i2 = playHistoryCallbackList.end ();
	while (i1 != i2) {
		if (i1->callbackData == callbackData) {
			found = true;
			i1->addRecordCallback = addRecordCallback;
			i1->removeRecordCallback = removeRecordCallback;
			break;
		}
		++i1;
	}
	if (! found) {
		playHistoryCallbackList.push_back (MediaControl::PlayHistoryCallbackContext (callbackData, addRecordCallback, removeRecordCallback));
	}
	SDL_UnlockMutex (playHistoryCallbackMutex);
}

void MediaControl::addPlayHistoryRecord (const MediaItem &mediaItem) {
	MediaItem m;
	StdString createid, removeid;
	Json *record;
	StringList sql;
	std::list<MediaControl::PlayHistoryCallbackContext>::const_iterator i1, i2;

	if (isStopped) {
		return;
	}
	m.copyValues (mediaItem);
	m.id = RecordStore::instance->getRecordId (SystemInterface::CommandId_MediaItem);
	m.thumbnailTimestamps.clear ();
	m.tags.clear ();
	m.mtime = OsUtil::getTime ();
	createid = m.id;
	record = m.createRecord (agentId);
	RecordStore::instance->insert (record, true);
	delete (record);

	SDL_LockMutex (playHistoryMutex);
	playHistoryRecordIds.push_back (createid);
	removeid = playHistoryMediaPathMap.find (m.mediaPath, "");
	if (removeid.empty ()) {
		++playHistorySize;
	}
	else {
		playHistoryRecordIds.remove (removeid);
	}
	playHistoryMediaPathMap.insert (m.mediaPath, createid);
	SDL_UnlockMutex (playHistoryMutex);

	if (! removeid.empty ()) {
		RecordStore::instance->remove (removeid);
	}
	if (mainOptions.savePlayHistory) {
		if (! removeid.empty ()) {
			sql.push_back (MediaItem::getDeleteSql (MediaControl::historyTableName, removeid));
		}
		sql.push_back (m.getUpsertSql (MediaControl::historyTableName));
		execDatabase (sql);
	}

	SDL_LockMutex (playHistoryCallbackMutex);
	i1 = playHistoryCallbackList.cbegin ();
	i2 = playHistoryCallbackList.cend ();
	while (i1 != i2) {
		i1->addRecordCallback (i1->callbackData, createid);
		if (! removeid.empty ()) {
			i1->removeRecordCallback (i1->callbackData, removeid);
		}
		++i1;
	}
	SDL_UnlockMutex (playHistoryCallbackMutex);
}

void MediaControl::executeAddRecordCallbacks (const StringList &idList) {
	std::list<MediaControl::PlayHistoryCallbackContext>::const_iterator i1, i2;
	StringList::const_iterator j1, j2;

	SDL_LockMutex (playHistoryCallbackMutex);
	i1 = playHistoryCallbackList.cbegin ();
	i2 = playHistoryCallbackList.cend ();
	while (i1 != i2) {
		j1 = idList.cbegin ();
		j2 = idList.cend ();
		while (j1 != j2) {
			i1->addRecordCallback (i1->callbackData, *j1);
			++j1;
		}
		++i1;
	}
	SDL_UnlockMutex (playHistoryCallbackMutex);
}

void MediaControl::executeRemoveRecordCallbacks (const StringList &idList) {
	std::list<MediaControl::PlayHistoryCallbackContext>::const_iterator i1, i2;
	StringList::const_iterator j1, j2;

	SDL_LockMutex (playHistoryCallbackMutex);
	i1 = playHistoryCallbackList.cbegin ();
	i2 = playHistoryCallbackList.cend ();
	while (i1 != i2) {
		j1 = idList.cbegin ();
		j2 = idList.cend ();
		while (j1 != j2) {
			i1->removeRecordCallback (i1->callbackData, *j1);
			++j1;
		}
		++i1;
	}
	SDL_UnlockMutex (playHistoryCallbackMutex);
}

int MediaControl::getPlayHistorySize () {
	int count;

	SDL_LockMutex (playHistoryMutex);
	count = playHistorySize;
	SDL_UnlockMutex (playHistoryMutex);
	return (count);
}

void MediaControl::getPlayHistoryRecordIds (StringList *destList) {
	destList->clear ();
	SDL_LockMutex (playHistoryMutex);
	destList->assign (playHistoryRecordIds);
	SDL_UnlockMutex (playHistoryMutex);
}

void MediaControl::clearPlayHistory () {
	StringList ids;

	SDL_LockMutex (playHistoryMutex);
	ids.assign (playHistoryRecordIds);
	playHistoryRecordIds.clear ();
	playHistoryMediaPathMap.clear ();
	playHistorySize = 0;
	SDL_UnlockMutex (playHistoryMutex);
	RecordStore::instance->remove (ids);
	execDatabase (StringList (MediaItem::getDeleteAllSql (MediaControl::historyTableName)));
	executeRemoveRecordCallbacks (ids);
}

void MediaControl::uiLogMessageReceived (void *itPtr, const UiLog::Message &message) {
	MediaControl *it = (MediaControl *) itPtr;
	StringList sql;
	StdString dbpath, errmsg;

	UiLog::getInsertMessageSql (message, MediaControl::uiLogTableName, &sql);
	SDL_LockMutex (it->databasePathMutex);
	dbpath.assign (it->databasePath);
	SDL_UnlockMutex (it->databasePathMutex);
	if (dbpath.empty ()) {
		return;
	}
	if (Database::instance->execTransaction (dbpath, sql, &errmsg) != OpResult::Success) {
		Log::debug ("Failed to write application data; err=\"%s\"", errmsg.c_str ());
	}
}

void MediaControl::clearUiLog () {
	StdString dbpath, errmsg;

	UiLog::instance->clear ();
	SDL_LockMutex (databasePathMutex);
	dbpath.assign (databasePath);
	SDL_UnlockMutex (databasePathMutex);
	if (dbpath.empty ()) {
		return;
	}
	if (Database::instance->exec (dbpath, StdString::createSprintf ("DELETE FROM %s;", MediaControl::uiLogTableName), &errmsg) != OpResult::Success) {
		Log::debug ("Failed to write application data; err=\"%s\"", errmsg.c_str ());
	}
}
