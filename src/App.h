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
// Class that runs the application
#ifndef APP_H
#define APP_H

#include "AppUtil.h"
#include "Log.h"
#include "HashMap.h"
#include "StringList.h"
#include "WidgetHandle.h"

class Widget;
class Panel;
class ConsoleWindow;

class App {
public:
	App ();
	~App ();
	static App *instance;

	// Populate the App instance pointer with a newly created object
	static void createInstance (bool shouldSkipInit = false);

	// Destroy and clear the App instance pointer
	static void freeInstance ();

	static constexpr const char *defaultImagePrefix = "1600x900";
	static constexpr const int prefsVersion = 1;
	static constexpr const char *databaseWriteQueueId = "databaseWrite";

	// Key values for the prefs map
	static constexpr const char *networkThreadsKey = "AppA";
	static constexpr const char *displayModeKey = "AppB";
	static constexpr const char *fontScaleKey = "AppC";
	static constexpr const char *allowUnverifiedHttpsKey = "AppD";
	static constexpr const char *prefsVersionKey = "AppE";
	static constexpr const char *soundVolumeKey = "AppF";
	static constexpr const char *fsBrowserPathKey = "AppG";
	static constexpr const char *languageKey = "AppH";

	// Read-write data members
	Log log;
	StdString prefsPath;
	bool isStartUpdateEnabled;
	bool shouldResizeUi;
	bool shouldSyncRecordStore;

	// Read-only data members
	StringList argv;
	AppUtil appUtil;
	StdString language;
	SDL_Window *window;
	SDL_Renderer *render; // The renderer must be accessed only from the application's main thread
	bool isShuttingDown;
	bool isShutdown;
	int64_t startTime;
	bool isTextureRenderEnabled;
	Panel *rootPanel;
	float displayDdpi;
	float displayHdpi;
	float displayVdpi;
	int windowWidth;
	int windowHeight;
	bool isFullscreen;
	double drawableWidth;
	double drawableHeight;
	int minDrawFrameDelay; // milliseconds
	int minUpdateFrameDelay; // milliseconds
	double fontScale;
	StdString imagePrefix;
	int64_t drawCount;
	int64_t updateCount;
	SDL_Rect clipRect;
	int uiActivityCount;
	int networkActivityCount;
	bool isPrefsWriteDisabled;

	// Run the application, returning only after the application exits
	int run (int argCount, char **argValues);

	// Begin the application's shutdown process and halt execution when complete
	void shutdown ();

	// Return an int64_t value for use as a unique ID
	int64_t getUniqueId ();

	// Lock the application prefs map and return a pointer to its HashMap object
	HashMap *lockPrefs ();

	// Unlock the application prefs map
	void unlockPrefs ();

	// Begin a display mode change, targeting the specified RenderResource display mode index
	void setDisplayMode (int mode);

	// Begin a font scale change, targeting the specified RenderResource font scale index
	void setFontScale (int scale);

	// Suspend the application's update thread and block until the current update cycle completes
	void suspendUpdate ();

	// Unsuspend the application's update thread after a previous call to suspendUpdate
	void unsuspendUpdate ();

	// Push the provided rectangle onto the clip stack and apply it to the application's renderer. Apply the new clip rectangle as an intersection of any existing clip rectangle unless disableIntersection is true.
	void pushClipRect (const SDL_Rect &rect, bool disableIntersection = false);

	// Pop the clip stack
	void popClipRect ();

	// Disable any active clip rectangle, to be restored by a call to unsuspendClipRect
	void suspendClipRect ();

	// Restore a previously suspended clip rectangle
	void unsuspendClipRect ();

	// Increase uiActivityCount
	void setUiActive ();

	// Decrease uiActivityCount
	void unsetUiActive ();

	// Increase networkActivityCount
	void setNetworkActive ();

	// Decrease networkActivityCount
	void unsetNetworkActive ();

	typedef void (*UpdateTaskFunction) (void *fnData);
	struct UpdateTaskContext {
		int64_t id;
		App::UpdateTaskFunction fn;
		void *fnData;
		UpdateTaskContext ():
			id (0),
			fn (NULL),
			fnData (NULL) { }
	};
	// Schedule a task function to execute at the top of the next update loop and return the task's ID value
	int64_t addUpdateTask (App::UpdateTaskFunction fn, void *fnData);

	typedef void (*RenderTaskFunction) (void *fnData);
	struct RenderTaskContext {
		int64_t id;
		App::RenderTaskFunction fn;
		void *fnData;
		RenderTaskContext ():
			id (0),
			fn (NULL),
			fnData (NULL) { }
	};
	// Schedule a task function to execute at the predraw step of the next render loop and return the task's ID value
	int64_t addPredrawTask (App::RenderTaskFunction fn, void *fnData);

	// Schedule a task function to execute at the postdraw step of the next render loop and return the task's ID value
	int64_t addPostdrawTask (App::RenderTaskFunction fn, void *fnData);

	// Return true if taskId matches a render or update task in progress
	bool isTaskRunning (int64_t taskId);

	// Set a ConsoleWindow widget that should receive output from the Log::printf method until destroyed
	void setConsoleWindow (ConsoleWindow *window);

	// Append text to any previously set console window
	void writeConsoleOutput (const StdString &text);

	// Show or hide the UiStack mouse pointer icon
	void toggleMousePointer ();

	// Show the provided message as a notification in the application interface
	void showNotification (const StdString &messageText);

	// Task functions
	static void shutdownApplication (void *ptr);
	static void resizeWindow (void *modePtr);
	void executeResizeWindow (void *modePtr);
	static void resizeFonts (void *doublePtr);
	void executeResizeFonts (void *doublePtr);

	// Callback functions
	static bool keyEvent (void *ptr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

private:
	// Read environment settings and configure the app
	void init ();

	// Run the application window
	int runWindow ();

	// Create the root panel and other top-level widgets
	void populateWidgets ();

	// Execute draw operations to update the application window
	void draw ();

	// Execute all operations in predrawTaskList
	void executePredrawTasks ();

	// Execute all operations in postdrawTaskList
	void executePostdrawTasks ();

	// Execute operations to update application state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Execute all operations in updateTaskList
	void executeUpdateTasks ();

	// Run the application's state update thread
	static int runUpdates (void *itPtr);

	// Write the prefs file if any prefsMap keys have changed since the last write
	void writePrefs ();

	std::list<App::UpdateTaskContext> updateTaskList;
	std::list<App::UpdateTaskContext> updateTaskAddList;
	SDL_mutex *updateTaskMutex;
	std::list<App::RenderTaskContext> predrawTaskList;
	std::list<App::RenderTaskContext> predrawTaskAddList;
	SDL_mutex *predrawTaskMutex;
	std::list<App::RenderTaskContext> postdrawTaskList;
	std::list<App::RenderTaskContext> postdrawTaskAddList;
	SDL_mutex *postdrawTaskMutex;
	std::map<int64_t, bool> taskStateMap;
	SDL_mutex *taskStateMapMutex;
	int64_t nextUniqueId;
	SDL_mutex *uniqueIdMutex;
	SDL_mutex *uiActivityMutex;
	SDL_mutex *networkActivityMutex;
	HashMap prefsMap;
	SDL_mutex *prefsMapMutex;
	SDL_Cursor *mouseCursor;
	std::stack<SDL_Rect> clipRectStack;
	bool isUpdateThreadEnded;
	bool isSuspendingUpdate;
	SDL_mutex *suspendUpdateMutex;
	SDL_cond *suspendUpdateCond;
	WidgetHandle<ConsoleWindow> consoleWindowHandle;
	ConsoleWindow *consoleWindow;
	SDL_mutex *consoleWindowMutex;
};
#endif
