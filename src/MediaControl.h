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
// Class that gathers and stores media item records
#ifndef MEDIA_CONTROL_H
#define MEDIA_CONTROL_H

#include "UiLog.h"
#include "AppNews.h"
#include "StringList.h"
#include "HashMap.h"

class MediaItem;
class MediaReader;
class MediaControlTask;
class MediaControlConfigureMainTask;
class MediaControlConfigureFilescanTask;
class MediaControlReadyTask;
class MediaControlFilescanTask;
class MediaControlCleanFilescanTask;

class MediaControl {
public:
	MediaControl ();
	~MediaControl ();
	static MediaControl *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Database table names
	static constexpr const char *metadataTableName = "MediaMetadata";
	static constexpr const char *filescanTableName = "MediaFilescan";
	static constexpr const char *playMarkerTableName = "PlayMarker";
	static constexpr const char *historyTableName = "MediaHistory";
	static constexpr const char *playlistTableName = "MediaPlaylist";
	static constexpr const char *uiLogTableName = "UiLog";
	static constexpr const char *appNewsTableName = "AppNews";

	// Configuration values for mediaThumbnailCount
	static constexpr const int MediaThumbnailEveryHour = 0;
	static constexpr const int MediaThumbnailEvery10Minutes = 1;
	static constexpr const int MediaThumbnailEveryMinute = 2;
	static constexpr const int MediaThumbnailEvery10Seconds = 3;
	static constexpr const int MediaThumbnailDivide1 = 4;
	static constexpr const int MediaThumbnailDivide12 = 5;
	static constexpr const int MediaThumbnailDivide48 = 6;
	static constexpr const int MediaThumbnailDivide100 = 7;
	static constexpr const int mediaThumbnailValuesCount = 8;
	static constexpr const int defaultMediaThumbnailCount = MediaThumbnailDivide12;

	struct MainOptions {
		bool savePlayHistory;
		bool savePlaylists;
		bool mediaScan;
		StdString dataPath;
		MainOptions ():
			savePlayHistory (false),
			savePlaylists (false),
			mediaScan (false) { }
	};

	struct FilescanOptions {
		StringList scanPath;
		int mediaThumbnailCount;
		FilescanOptions ():
			mediaThumbnailCount (MediaControl::defaultMediaThumbnailCount) { }
	};

	// Read-only data members
	bool isStopped;
	bool isReady;
	MainOptions mainOptions;
	FilescanOptions filescanOptions;
	bool isFilescanConfigured;
	StdString agentId;
	AppNews appNews;

	// Start media control operations and return a result value
	OpResult start ();

	// Stop media control operations
	void stop ();

	// Update media control state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Open a connection to the active database path and return a Result value. If the open succeeds, store the database path in pathValue. If a path value is provided by this method, it must be released using the Database::close method when no longer needed.
	OpResult openDatabase (StdString *pathValue);

	// Run a task to execute one or more sql queries targeting the active database
	void execDatabase (const StringList &sql);

	// Run a task to set main configuration values
	void configureMain (const MediaControl::MainOptions &options);

	// Return true if provided fields match the current main configuration
	bool matchMainConfiguration (const MainOptions &options);

	// Run a task to set filescan configuration values
	void configureFilescan (const MediaControl::FilescanOptions &options);

	// Return true if provided fields match the current filescan configuration
	bool matchFilescanConfiguration (const MediaControl::FilescanOptions &options);

	// Run a task to scan for new media files
	void filescan ();

	// Run a task to clean unused media data
	void cleanFilescan ();

	// Cancel a previously queued task
	void cancelTask (int taskType);

	// Clear task result fields of the specified type
	void clearTaskResult (int taskType);

	// Return true if a task matching classId has been queued
	bool isTaskRunning (int classId);

	struct Status {
		int updateCount;
		int mediaCount;
		StdString statusText;
		int taskType;
		bool isTaskRunning;
		StdString taskText1;
		StdString taskText2;
		double taskProgressPercent;
		Status ():
			updateCount (0),
			mediaCount (-1),
			taskType (-1),
			isTaskRunning (false),
			taskProgressPercent (-1.0f) { }
	};
	struct TaskResult {
		bool isSuccess;
		StdString text1;
		StdString text2;
		TaskResult () { }
		TaskResult (bool isSuccess, const StdString &text1, const StdString &text2):
			isSuccess (isSuccess),
			text1 (text1),
			text2 (text2) { }
	};

	// Copy media control status fields into destStatus. If taskType and taskResult are provided, also copy any stored task result fields into taskResult.
	void getStatus (MediaControl::Status *destStatus, int taskType, MediaControl::TaskResult *taskResult);
	void getStatus (MediaControl::Status *destStatus);
	void getStatus (int taskType, MediaControl::TaskResult *taskResult);

	// Return the path value for the specified thumbnail image file
	StdString getThumbnailPath (const StdString &mediaId, int64_t thumbnailTimestamp);

	typedef void (*PlayHistoryCallback) (void *data, const StdString &recordId);
	// Set callback functions to be invoked when play history records change
	void addPlayHistoryListener (void *callbackData, MediaControl::PlayHistoryCallback addRecordCallback, MediaControl::PlayHistoryCallback removeRecordCallback);

	// Add a record to the play history
	void addPlayHistoryRecord (const MediaItem &mediaItem);

	// Return the count of stored play history records
	int getPlayHistorySize ();

	// Clear destList and set its contents to the list of play history record ID values
	void getPlayHistoryRecordIds (StringList *destList);

	// Remove all play history records
	void clearPlayHistory ();

	// Remove all UiLog records
	void clearUiLog ();

private:
	MediaControl::Status status;
	std::map<int, MediaControl::TaskResult> taskResultMap;
	SDL_mutex *statusMutex;
	StdString databasePath;
	SDL_mutex *databasePathMutex;

	// Lock the status object
	void lockStatus ();

	// Unlock the status object and increase the update count
	void unlockStatus ();

	// Queue a run task. This class becomes responsible for freeing the task when it is no longer needed.
	void addTask (MediaControlTask *task);

	// Execute the top task from taskList
	static void executeTask (void *itPtr);

	// Set ended state after a task completes
	void endTask (MediaControlTask *task);

	// Remove MediaItem directories from the specified path
	void removeMediaItemDirectories (const StdString &targetPath);

	// Task functions
	void executeConfigureMain (MediaControlConfigureMainTask *task);
	void executeConfigureFilescan (MediaControlConfigureFilescanTask *task);

	void executeReady (MediaControlReadyTask *task);
	OpResult executeReady_openDatabase (const StdString &readyDatabasePath);
	void executeReady_loadPlayHistory ();
	static int executeReady_loadPlayHistory_row (void *itPtr, int columnCount, char **columnValues, char **columnNames);

	void executeFilescan (MediaControlFilescanTask *task);
	void executeFilescan_readDirectory (const StdString &scanPath, StringList *destList);
	OpResult executeFilescan_processFile (std::list<MediaItem>::iterator item, StdString *errorMessage);
	OpResult executeFilescan_writeThumbnailImages (std::list<MediaItem>::iterator item, StdString *errorMessage, const MediaReader &metadataReader);

	void executeCleanFilescan (MediaControlCleanFilescanTask *task);
	OpResult executeCleanFilescan_removeRecords (int *removedRecordCount, StdString *errorMessage);
	OpResult executeCleanFilescan_removeFiles (int64_t *removedFileSize, StdString *errorMessage);

	static void executeDatabaseExec (void *taskPtr);

	// Callback functions
	static void uiLogMessageReceived (void *itPtr, const UiLog::Message &message);

	std::list<MediaControlTask *> taskList;
	SDL_mutex *taskListMutex;
	bool isTaskCancelled;

	SDL_mutex *playHistoryMutex;
	StringList playHistoryRecordIds;
	HashMap playHistoryMediaPathMap;
	StringList playHistoryLoadIds;
	int playHistorySize;

	struct PlayHistoryCallbackContext {
		void *callbackData;
		MediaControl::PlayHistoryCallback addRecordCallback;
		MediaControl::PlayHistoryCallback removeRecordCallback;
		PlayHistoryCallbackContext ():
			callbackData (NULL),
			addRecordCallback (NULL),
			removeRecordCallback (NULL) { }
		PlayHistoryCallbackContext (void *callbackData, MediaControl::PlayHistoryCallback addRecordCallback, MediaControl::PlayHistoryCallback removeRecordCallback):
			callbackData (callbackData),
			addRecordCallback (addRecordCallback),
			removeRecordCallback (removeRecordCallback) { }
	};
	SDL_mutex *playHistoryCallbackMutex;
	std::list<MediaControl::PlayHistoryCallbackContext> playHistoryCallbackList;

	// Execute play history callbacks for all record ID values in idList
	void executeAddRecordCallbacks (const StringList &idList);
	void executeRemoveRecordCallbacks (const StringList &idList);
};
#endif
