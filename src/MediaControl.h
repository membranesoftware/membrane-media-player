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
// Class that gathers metadata from files in a media directory
#ifndef MEDIA_CONTROL_H
#define MEDIA_CONTROL_H

#include "StringList.h"

class MediaItem;
class MediaReader;

class MediaControl {
public:
	MediaControl ();
	~MediaControl ();
	static MediaControl *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Prefs keys
	static constexpr const char *agentIdKey = "MediaControlA";
	static constexpr const char *mediaSourcePathKey = "MediaControlB";
	static constexpr const char *dataPathKey = "MediaControlC";
	static constexpr const char *mediaThumbnailCountKey = "MediaControlD";

	// Status.taskType values
	static constexpr const int NoTask = 0;
	static constexpr const int PrimeTask = 1;
	static constexpr const int ReadyTask = 2;
	static constexpr const int ScanTask = 3;
	static constexpr const int CleanTask = 4;
	static constexpr const int ConfigureTask = 5;

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

	// Read-only data members
	bool isStopped;
	bool isReady;
	bool isConfigured;
	StdString agentId;
	StdString databasePath;
	StringList mediaSourcePath;
	StdString dataPath;
	int mediaThumbnailCount;

	// Start media control operations and return a Result value.
	OpResult start ();

	// Stop media control operations
	void stop ();

	// Update media control state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Run a task to set configuration values for the first start of media control
	void prime (const StdString &mediaSourcePathValue, const StdString &dataPathValue);

	// Run a task to change the media control configuration
	void configure (const StringList &mediaSourcePathValue, const StdString &dataPathValue, int mediaThumbnailCountValue);

	// Return true if provided fields match the current media control configuration
	bool matchConfiguration (const StringList &mediaSourcePathValue, const StdString &dataPathValue, int mediaThumbnailCountValue);

	// Run a task to scan for new media files
	void scan ();

	// Run a task to clean unused media data
	void clean ();

	// Return the path value for the specified thumbnail image file
	StdString getThumbnailPath (const StdString &mediaId, int64_t thumbnailTimestamp);

	// Queue a run task
	void runTask (int taskType);

	// Cancel a previously queued task
	void cancelTask (int taskType);

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
			taskType (MediaControl::NoTask),
			isTaskRunning (false),
			taskProgressPercent (-1.0f) { }
	};
	struct TaskResult {
		StdString text1;
		StdString text2;
		TaskResult () { }
		TaskResult (const StdString &text1, const StdString &text2):
			text1 (text1),
			text2 (text2) { }
	};

	// Copy media control status fields into destStatus. If taskType and taskResult are provided, also copy any stored task result fields into taskResult.
	void getStatus (MediaControl::Status *destStatus, int taskType, MediaControl::TaskResult *taskResult);
	void getStatus (MediaControl::Status *destStatus);
	void getStatus (int taskType, MediaControl::TaskResult *taskResult);

	// Clear task result fields of the specified type
	void clearTaskResult (int taskType);

	// Return true if a task matching taskType has been queued
	bool isRunningTask (int taskType);

private:
	MediaControl::Status status;
	std::map<int, MediaControl::TaskResult> taskResultMap;
	SDL_mutex *statusMutex;
	StdString primeMediaSourcePath;
	StdString primeDataPath;
	StringList configureMediaSourcePath;
	StdString configureDataPath;
	int configureMediaThumbnailCount;

	// Open the media control's database connection and return a Result value
	OpResult openDatabase ();

	// Lock the status object
	void lockStatus ();

	// Unlock the status object and increase the update count
	void unlockStatus ();

	// Set ended state after a task completes
	void endTask (int taskType, const StdString &resultText1, const StdString &resultText2 = StdString (), const StdString &uiLogMessage = StdString (), const char *logErrorMessage = NULL);

	struct Task {
		int taskType;
		bool isRunning;
		bool isEnded;
		Task ():
			taskType (MediaControl::NoTask),
			isRunning (false),
			isEnded (false) { }
		Task (int taskType):
			taskType (taskType),
			isRunning (false),
			isEnded (false) { }
	};
	std::list<MediaControl::Task> taskList;
	SDL_mutex *taskListMutex;
	bool isTaskCancelled;

	// Task functions
	static void applyPrimeSettings (void *itPtr);
	void executeApplyPrimeSettings ();

	static void readyMediaControl (void *itPtr);
	void executeReadyMediaControl ();

	static void applyConfigureSettings (void *itPtr);
	void executeApplyConfigureSettings ();

	static void scanMediaFiles (void *itPtr);
	void executeScanMediaFiles ();
	void executeScanMediaFiles_readDirectory (const StdString &scanPath, StringList *destList);
	OpResult executeScanMediaFiles_processFile (std::list<MediaItem>::iterator item, StdString *errorMessage);
	OpResult executeScanMediaFiles_writeThumbnailImages (std::list<MediaItem>::iterator item, StdString *errorMessage, const MediaReader &metadataReader);

	static void cleanMediaData (void *itPtr);
	void executeCleanMediaData ();
	OpResult executeCleanMediaData_removeRecords (int *removedRecordCount, StdString *errorMessage);
	OpResult executeCleanMediaData_removeFiles (int64_t *removedFileSize, StdString *errorMessage);
};
#endif
