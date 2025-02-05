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
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <unistd.h>
#endif
#if PLATFORM_MACOS
#include <mach-o/dyld.h>
#include <libgen.h>
#endif
#if PLATFORM_WINDOWS
#include <io.h>
#endif
#include "SDL2/SDL_image.h"
#include "Log.h"
#include "LuaScript.h"
#include "OsUtil.h"
#include "SdlUtil.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "UiLog.h"
#include "AppNews.h"
#include "PlayerUi.h"
#include "SystemInterface.h"
#include "AppUrl.h"
#include "RecordStore.h"
#include "TaskGroup.h"
#include "Database.h"
#include "Input.h"
#include "Prng.h"
#include "Resource.h"
#include "RenderResource.h"
#include "CaptureWriter.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "MediaUtil.h"
#include "SoundMixer.h"
#include "Network.h"
#include "LuaFunctionList.h"
#include "UiStack.h"
#include "Panel.h"
#include "ConsoleWindow.h"
#include "MediaControl.h"
#include "App.h"

constexpr const SDL_Keycode quitKeycode = SDLK_q;
constexpr const SDL_Keycode settingsKeycode = SDLK_s;
constexpr const SDL_Keycode helpKeycode = SDLK_h;
constexpr const SDL_Keycode logKeycode = SDLK_l;
constexpr const SDL_Keycode consoleKeycode = SDLK_o;
constexpr const int defaultMinFrameDelay = 10;
constexpr const double pointerOffsetScaleX = 0.5f;
constexpr const double pointerOffsetScaleY = 0.25f;

App *App::instance = NULL;

void App::createInstance (bool shouldSkipInit) {
	if (App::instance) {
		return;
	}
	App::instance = new App ();
	MediaUtil::createInstance ();
	Prng::createInstance ();
	TaskGroup::createInstance ();
	Database::createInstance ();
	LuaFunctionList::createInstance ();
	Network::createInstance ();
	RenderResource::createInstance ();
	Resource::createInstance ();
	SpriteGroup::createInstance ();
	SoundMixer::createInstance ();
	SystemInterface::createInstance ();
	AppUrl::createInstance ();
	RecordStore::createInstance ();
	MediaControl::createInstance ();
	Input::createInstance ();
	UiLog::createInstance ();
	AppNews::createInstance ();
	UiStack::createInstance ();
	UiText::createInstance ();
	UiConfiguration::createInstance ();
	CaptureWriter::createInstance ();

	if (! shouldSkipInit) {
		App::instance->init ();
	}
}
void App::freeInstance () {
	if (! App::instance) {
		return;
	}
	CaptureWriter::freeInstance ();
	UiConfiguration::freeInstance ();
	UiText::freeInstance ();
	UiStack::freeInstance ();
	AppNews::freeInstance ();
	UiLog::freeInstance ();
	Input::freeInstance ();
	MediaControl::freeInstance ();
	RecordStore::freeInstance ();
	AppUrl::freeInstance ();
	SystemInterface::freeInstance ();
	SoundMixer::freeInstance ();
	SpriteGroup::freeInstance ();
	Resource::freeInstance ();
	RenderResource::freeInstance ();
	Network::freeInstance ();
	LuaFunctionList::freeInstance ();
	Database::freeInstance ();
	TaskGroup::freeInstance ();
	Prng::freeInstance ();
	MediaUtil::freeInstance ();

	delete (App::instance);
	App::instance = NULL;

	IMG_Quit ();
	SDL_Quit ();
}

App::App ()
: isStartUpdateEnabled (true)
, shouldResizeUi (false)
, shouldSyncRecordStore (false)
, window (NULL)
, render (NULL)
, isShuttingDown (false)
, isShutdown (false)
, startTime (0)
, isTextureRenderEnabled (false)
, rootPanel (NULL)
, displayDdpi (0.0f)
, displayHdpi (0.0f)
, displayVdpi (0.0f)
, windowWidth (0)
, windowHeight (0)
, isFullscreen (false)
, drawableWidth (0.0f)
, drawableHeight (0.0f)
, minDrawFrameDelay (0)
, minUpdateFrameDelay (0)
, fontScale (1.0f)
, drawCount (0)
, updateCount (0)
, uiActivityCount (0)
, networkActivityCount (0)
, isPrefsWriteDisabled (false)
, nextUniqueId (1)
, mouseCursor (NULL)
, isUpdateThreadEnded (false)
, isSuspendingUpdate (false)
, consoleWindowHandle (&consoleWindow)
{
	SdlUtil::createMutex (&uniqueIdMutex);
	SdlUtil::createMutex (&prefsMapMutex);
	SdlUtil::createMutex (&updateTaskMutex);
	SdlUtil::createMutex (&predrawTaskMutex);
	SdlUtil::createMutex (&postdrawTaskMutex);
	SdlUtil::createMutex (&taskStateMapMutex);
	SdlUtil::createMutex (&suspendUpdateMutex);
	SdlUtil::createCond (&suspendUpdateCond);
	SdlUtil::createMutex (&uiActivityMutex);
	SdlUtil::createMutex (&networkActivityMutex);
	SdlUtil::createMutex (&consoleWindowMutex);
}
App::~App () {
	SDL_LockMutex (consoleWindowMutex);
	consoleWindowHandle.clear ();
	SDL_UnlockMutex (consoleWindowMutex);

	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}

	if (mouseCursor) {
		SDL_FreeCursor (mouseCursor);
		mouseCursor = NULL;
	}

	SdlUtil::destroyMutex (&uniqueIdMutex);
	SdlUtil::destroyMutex (&prefsMapMutex);
	SdlUtil::destroyMutex (&updateTaskMutex);
	SdlUtil::destroyMutex (&predrawTaskMutex);
	SdlUtil::destroyMutex (&postdrawTaskMutex);
	SdlUtil::destroyMutex (&taskStateMapMutex);
	SdlUtil::destroyMutex (&suspendUpdateMutex);
	SdlUtil::destroyCond (&suspendUpdateCond);
	SdlUtil::destroyMutex (&uiActivityMutex);
	SdlUtil::destroyMutex (&networkActivityMutex);
	SdlUtil::destroyMutex (&consoleWindowMutex);
}

void App::init () {
	StdString path;
	OpResult result;
#if PLATFORM_MACOS
	char exepath[4096], dirpath[4096];
	uint32_t sz;
#endif

	log.setLevelByName (OsUtil::getEnvValue (OsUtil::logLevelEnvKey, "ERR"));
	if (OsUtil::getEnvValue (OsUtil::logConsoleEnvKey, false)) {
		log.isStdoutWriteEnabled = true;
	}
	path = OsUtil::getEnvValue (OsUtil::logFilenameEnvKey, "");
	if (! path.empty ()) {
		log.openLogFile (path);
	}

#if PLATFORM_MACOS
	memset (exepath, 0, sizeof (exepath));
	sz = sizeof (exepath);
	if (_NSGetExecutablePath (exepath, &sz) != 0) {
		memset (exepath, 0, sizeof (exepath));
		Log::warning ("Failed to determine executable path");
	}
	else {
		memset (dirpath, 0, sizeof (dirpath));
		if (chdir (dirname_r (exepath, dirpath)) != 0) {
			Log::warning ("Failed to change working directory");
		}
	}
#endif

	path = OsUtil::getEnvValue (OsUtil::resourcePathEnvKey, "");
	if (path.empty ()) {
		path.sprintf ("%s.dat", APPLICATION_PACKAGE_NAME);
#if PLATFORM_MACOS
		if (exepath[0] != '\0') {
			memset (dirpath, 0, sizeof (dirpath));
			path.sprintf ("%s/%s.dat", dirname_r (exepath, dirpath), APPLICATION_PACKAGE_NAME);
		}
#endif
	}
	Resource::instance->setSource (path);

	path = OsUtil::getEnvValue (OsUtil::appDataPathEnvKey, "");
	if (path.empty ()) {
		path = OsUtil::getAppDataPath ();
		if (! path.empty ()) {
			result = OsUtil::createDirectory (path);
			if (result != OpResult::Success) {
				Log::warning ("Application data cannot be saved (failed to create directory); path=\"%s\" err=%i", path.c_str (), result);
			}
		}
	}
	if (path.empty ()) {
		Log::warning ("Application data cannot be saved (set APPDATA_PATH to specify destination directory)");
	}
	else {
		prefsPath.assign (OsUtil::getJoinedPath (path, StdString::createSprintf ("%s.conf", APPLICATION_PACKAGE_NAME)));
		if (! log.isFileWriteEnabled) {
			log.openLogFile (OsUtil::getJoinedPath (path, StdString::createSprintf ("%s.log", APPLICATION_PACKAGE_NAME)));
		}
	}

	minDrawFrameDelay = OsUtil::getEnvValue (OsUtil::minDrawFrameDelayEnvKey, 0);
	minUpdateFrameDelay = OsUtil::getEnvValue (OsUtil::minUpdateFrameDelayEnvKey, 0);
}

int App::run (int argCount, char **argValues) {
	int i, result;

	startTime = OsUtil::getTime ();
	if (minDrawFrameDelay <= 0) {
		minDrawFrameDelay = defaultMinFrameDelay;
	}
	if (minUpdateFrameDelay <= 0) {
		minUpdateFrameDelay = defaultMinFrameDelay;
	}

	argv.clear ();
	if (argValues) {
		for (i = 0; i < argCount; ++i) {
			argv.push_back (StdString (argValues[i]));
		}
	}

	Prng::instance->seed ((uint32_t) (OsUtil::getTime () & 0xFFFFFFFF));
	prefsMap.clear ();
	if (prefsPath.empty ()) {
		isPrefsWriteDisabled = true;
	}
	else {
		result = prefsMap.read (prefsPath, true);
		if (result != OpResult::Success) {
			Log::debug ("Failed to read preferences file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			prefsMap.clear ();
		}
	}
	prefsMap.insert (App::prefsVersionKey, App::prefsVersion);
	language = prefsMap.find (App::languageKey, UiText::defaultLanguage);
	isStartUpdateEnabled = prefsMap.find (PlayerUi::startUpdateKey, true);

	result = Resource::instance->open ();
	if (result != OpResult::Success) {
		Log::err ("Failed to open application resources (%i): %s", result, Resource::instance->lastErrorMessage.c_str ());
		return (result);
	}
	result = UiText::instance->load (language);
	if (result != OpResult::Success) {
		Log::err ("Failed to load text resources; language=\"%s\" err=%i", language.c_str (), result);
		return (result);
	}
	Network::instance->maxRequestThreads = prefsMap.find (App::networkThreadsKey, Network::defaultMaxRequestThreads);
	Network::instance->allowUnverifiedHttps = prefsMap.find (App::allowUnverifiedHttpsKey, false);
	Network::instance->httpUserAgent.sprintf ("Membrane Media Player/%s_%s", BUILD_ID, PLATFORM_ID);
	result = Network::instance->start ();
	if (result != OpResult::Success) {
		Log::err ("Failed to start network resources; err=%i", result);
		return (result);
	}

	result = MediaControl::instance->start ();
	if (result != OpResult::Success) {
		Log::err ("Failed to start media library processes; err=%i", result);
		return (result);
	}

	result = runWindow ();
	writePrefs ();
	return (result);
}

int App::runWindow () {
	SDL_version version1, version2;
	SDL_Thread *thread;
	SDL_RendererInfo renderinfo;
	Uint32 flags;
	StdString modename;
	RenderResource::DisplayMode *mode;
	int result, delay, i, w, h;
	int64_t endtime, elapsed, t1, t2;
	double fps;

	if (SDL_Init (appUtil.getSdlInitFlags ()) != 0) {
		Log::err ("Failed to start SDL, SDL_Init: %s", SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	if (IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
		Log::err ("Failed to start SDL_image, IMG_Init: %s", IMG_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	result = Input::instance->start ();
	if (result != OpResult::Success) {
		Log::err ("Failed to acquire application input devices; err=%i", result);
		return (result);
	}
	result = SoundMixer::instance->start ();
	if (result != OpResult::Success) {
		Log::debug ("Failed to open audio output: %s", SoundMixer::instance->lastErrorMessage.c_str ());
	}
	result = SDL_GetDisplayDPI (0, &displayDdpi, &displayHdpi, &displayVdpi);
	if (result != 0) {
		Log::debug ("Failed to determine display DPI, SDL_GetDisplayDPI: %s", SDL_GetError ());
		displayHdpi = 72.0f;
		displayVdpi = 72.0f;
	}

	i = -1;
	modename = OsUtil::getEnvValue (OsUtil::displayModeEnvKey, "");
	if (modename.empty ()) {
		modename = prefsMap.find (App::displayModeKey, "");
	}
	if (! modename.empty ()) {
		i = RenderResource::instance->getNamedDisplayMode (modename);
		if (i < 0) {
			Log::debug ("Configuration display mode \"%s\" could not be applied (unknown mode)", modename.c_str ());
		}
	}
	if (i < 0) {
		if (appUtil.getDisplaySize (&w, &h) == OpResult::Success) {
			i = RenderResource::instance->getFitWindowDisplayMode (w, h);
			if (i >= 0) {
				Log::debug ("Set display mode \"%s\" for drawable bounds %ix%i", RenderResource::instance->displayModes[i].name.c_str (), w, h);
				prefsMap.insert (App::displayModeKey, RenderResource::instance->displayModes[i].name);
			}
		}
	}
	if (i < 0) {
		i = RenderResource::instance->getSmallestWindowDisplayMode ();
	}
	mode = &(RenderResource::instance->displayModes[i]);
	isFullscreen = mode->isFullscreen;
	result = appUtil.createWindowAndRenderer (mode->width, mode->height, mode->isFullscreen, &window, &render, &flags);
	if (result != OpResult::Success) {
		return (result);
	}
	appUtil.setDrawableSize (window, &windowWidth, &windowHeight, &drawableWidth, &drawableHeight, &imagePrefix);

	result = SDL_GetRendererInfo (render, &renderinfo);
	if (result != 0) {
		Log::err ("Failed to create application renderer, SDL_GetRendererInfo: %s", SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	if (renderinfo.flags & SDL_RENDERER_TARGETTEXTURE) {
		isTextureRenderEnabled = true;
	}

	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = windowWidth;
	clipRect.h = windowHeight;

	mouseCursor = SDL_CreateSystemCursor (SDL_SYSTEM_CURSOR_ARROW);
	if (! mouseCursor) {
		Log::debug ("Failed to create mouse cursor, SDL_CreateSystemCursor: %s", SDL_GetError ());
	}
	else {
		SDL_SetCursor (mouseCursor);
		SDL_ShowCursor (SDL_ENABLE);
	}

	i = prefsMap.find (App::fontScaleKey, RenderResource::fontScaleCount / 2);
	if ((i >= 0) && (i < RenderResource::fontScaleCount)) {
		fontScale = RenderResource::instance->fontScales[i];
	}

	UiConfiguration::instance->resetScale ();
	result = UiConfiguration::instance->load (fontScale);
	if (result != OpResult::Success) {
		Log::err ("Failed to load application resources; err=%i", result);
		return (result);
	}
	result = SpriteGroup::instance->load (SpriteId::SpriteGroup_prefix);
	if (result != OpResult::Success) {
		Log::err ("Failed to load application resources; err=%i", result);
		return (result);
	}
	result = RenderResource::instance->load ();
	if (result != OpResult::Success) {
		Log::err ("Failed to load render resources; err=%i", result);
		return (result);
	}

	populateWidgets ();
	UiStack::instance->setUi (appUtil.createMainUi ());
	isUpdateThreadEnded = false;
	thread = SDL_CreateThread (App::runUpdates, "App::runUpdates", (void *) this);
	Log::info ("Application started; buildId=%s windowSize=%ix%i drawableSize=%ix%i isFullscreen=%s lang=%s pid=%i", BUILD_ID, windowWidth, windowHeight, (int) drawableWidth, (int) drawableHeight, BOOL_STRING (isFullscreen), language.c_str (), OsUtil::getProcessId ());
	SDL_VERSION (&version1);
	SDL_GetVersion (&version2);
	Log::debug ("* sdlBuildVersion=%i.%i.%i sdlLinkVersion=%i.%i.%i isTextureRenderEnabled=%s diagonalDpi=%.2f horizontalDpi=%.2f verticalDpi=%.2f imagePrefix=%s minDrawFrameDelay=%i minUpdateFrameDelay=%i", version1.major, version1.minor, version1.patch, version2.major, version2.minor, version2.patch, BOOL_STRING (isTextureRenderEnabled), displayDdpi, displayHdpi, displayVdpi, imagePrefix.c_str (), minDrawFrameDelay, minUpdateFrameDelay);
	Log::debug ("* renderName=%s renderMaxTextureSize=%ix%i renderFlags=%s windowFlags=%s", renderinfo.name, renderinfo.max_texture_width, renderinfo.max_texture_height, UiText::instance->getSdlRendererFlagsText (renderinfo.flags).c_str (), UiText::instance->getSdlWindowFlagsText (flags).c_str ());

	while (true) {
		if (isShutdown) {
			break;
		}
		t1 = OsUtil::getTime ();
		Input::instance->pollEvents ();

		draw ();
		UiStack::instance->executeStackCommands ();
		Resource::instance->compact ();

		t2 = OsUtil::getTime ();
		delay = minDrawFrameDelay - ((int) (t2 - t1));
		if (delay < 1) {
			delay = 1;
		}
		SDL_Delay (delay);
	}
	SDL_WaitThread (thread, &result);

	executePredrawTasks ();
	executePostdrawTasks ();
	setConsoleWindow (NULL);
	if (isFullscreen) {
		i = RenderResource::instance->getSmallestWindowDisplayMode ();
		mode = &(RenderResource::instance->displayModes[i]);
		SDL_SetWindowFullscreen (window, 0);
		SDL_SetWindowBordered (window, SDL_TRUE);
		SDL_SetWindowSize (window, mode->width, mode->height);
		isFullscreen = false;
	}

	UiStack::instance->clear ();
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}
	if (mouseCursor) {
		SDL_ShowCursor (SDL_DISABLE);
		SDL_FreeCursor (mouseCursor);
		mouseCursor = NULL;
	}
	UiConfiguration::instance->unload ();
	SpriteGroup::instance->unload ();
	RenderResource::instance->unload ();
	Resource::instance->compact ();
	Resource::instance->close ();
	SDL_DestroyRenderer (render);
	render = NULL;
	SDL_DestroyWindow (window);
	window = NULL;

	endtime = OsUtil::getTime ();
	elapsed = endtime - startTime;
	fps = (double) drawCount;
	if (elapsed > 1000) {
		fps /= ((double) elapsed) / 1000.0f;
	}
	Log::info ("Application ended; updateCount=%lli drawCount=%lli runtime=%.3fs FPS=%f pid=%i", (long long) updateCount, (long long) drawCount, ((double) elapsed) / 1000.0f, fps, OsUtil::getProcessId ());

	return (OpResult::Success);
}

void App::populateWidgets () {
	if (! rootPanel) {
		rootPanel = new Panel ();
		rootPanel->retain ();
		rootPanel->id = getUniqueId ();
		rootPanel->setFixedSize (true, drawableWidth, drawableHeight);
		rootPanel->keyEventCallback = Widget::KeyEventCallbackContext (App::keyEvent, NULL);
	}
	UiStack::instance->populateWidgets ();
}

void App::shutdown () {
	if (isShuttingDown) {
		return;
	}
	isShuttingDown = true;

	CaptureWriter::instance->stopMediaWriter ();
	UiStack::instance->processShutdownEvent ();
	MediaControl::instance->stop ();
	TaskGroup::instance->stop ();
	Network::instance->stop ();
	Input::instance->stop ();
	SoundMixer::instance->stop ();
}

void App::shutdownApplication (void *ptr) {
	App::instance->shutdown ();
}

HashMap *App::lockPrefs () {
	SDL_LockMutex (prefsMapMutex);
	return (&prefsMap);
}
void App::unlockPrefs () {
	SDL_UnlockMutex (prefsMapMutex);
}

void App::executePredrawTasks () {
	std::list<App::RenderTaskContext>::const_iterator i1, i2;

	predrawTaskList.clear ();
	SDL_LockMutex (predrawTaskMutex);
	predrawTaskList.swap (predrawTaskAddList);
	SDL_UnlockMutex (predrawTaskMutex);

	i1 = predrawTaskList.cbegin ();
	i2 = predrawTaskList.cend ();
	while (i1 != i2) {
		i1->fn (i1->fnData);
		SDL_LockMutex (taskStateMapMutex);
		taskStateMap.erase (i1->id);
		SDL_UnlockMutex (taskStateMapMutex);
		++i1;
	}
	predrawTaskList.clear ();
}

void App::executePostdrawTasks () {
	std::list<App::RenderTaskContext>::const_iterator i1, i2;

	postdrawTaskList.clear ();
	SDL_LockMutex (postdrawTaskMutex);
	postdrawTaskList.swap (postdrawTaskAddList);
	SDL_UnlockMutex (postdrawTaskMutex);

	i1 = postdrawTaskList.cbegin ();
	i2 = postdrawTaskList.cend ();
	while (i1 != i2) {
		i1->fn (i1->fnData);
		SDL_LockMutex (taskStateMapMutex);
		taskStateMap.erase (i1->id);
		SDL_UnlockMutex (taskStateMapMutex);
		++i1;
	}
	postdrawTaskList.clear ();
}

void App::draw () {
	Ui *ui;

	executePredrawTasks ();
	SDL_RenderClear (render);
	UiStack::instance->drawBackground ();
	ui = UiStack::instance->getActiveUi ();
	if (ui) {
		ui->draw ();
		rootPanel->draw ();
		ui->release ();
	}
	UiStack::instance->drawForeground ();
	executePostdrawTasks ();

	SDL_RenderPresent (render);
	++drawCount;
}

int App::runUpdates (void *itPtr) {
	App *it = (App *) itPtr;
	StdString line;
	int64_t t1, t2, last;
	int delay;

	line = OsUtil::getEnvValue (OsUtil::runScriptEnvKey, "");
	if (! line.empty ()) {
		TaskGroup::instance->run (TaskGroup::RunContext (LuaScript::run, new LuaScript (line, true)));
		line.assign ("");
	}
	line = OsUtil::getEnvValue (OsUtil::runFileEnvKey, "");
	if (! line.empty ()) {
		TaskGroup::instance->run (TaskGroup::RunContext (LuaScript::run, new LuaScript (StdString::createSprintf ("dofile(\"%s\")", line.c_str ()), true)));
		line.assign ("");
	}

	last = OsUtil::getTime ();
	while (true) {
		if (it->isShutdown) {
			break;
		}
		t1 = OsUtil::getTime ();
		it->update ((int) (t1 - last));
		t2 = OsUtil::getTime ();
		last = t1;

		delay = it->minUpdateFrameDelay - ((int) (t2 - t1));
		if (delay < 1) {
			delay = 1;
		}
		SDL_Delay (delay);
	}
	it->isUpdateThreadEnded = true;
	it->executeUpdateTasks ();
	SDL_LockMutex (it->suspendUpdateMutex);
	SDL_CondBroadcast (it->suspendUpdateCond);
	SDL_UnlockMutex (it->suspendUpdateMutex);
	return (0);
}

void App::update (int msElapsed) {
	Ui *ui;

	TaskGroup::instance->update (msElapsed);
	UiLog::instance->update (msElapsed);
	MediaControl::instance->update (msElapsed);
	CaptureWriter::instance->update (msElapsed);

	if (shouldResizeUi) {
		rootPanel->resize ();
		UiStack::instance->resize ();
		rootPanel->resetInputState ();
		shouldResizeUi = false;
	}
	UiStack::instance->update (msElapsed);
	ui = UiStack::instance->getActiveUi ();

	if (shouldSyncRecordStore) {
		rootPanel->syncRecordStore ();
		if (ui) {
			ui->syncRecordStore ();
		}
		shouldSyncRecordStore = false;
	}

	rootPanel->processInput ();
	executeUpdateTasks ();
	if (ui) {
		ui->update (msElapsed);
	}
	rootPanel->update (msElapsed, 0.0f, 0.0f);
	if (ui) {
		ui->release ();
	}

	writePrefs ();
	++updateCount;

	SDL_LockMutex (suspendUpdateMutex);
	while (isSuspendingUpdate) {
		SDL_CondBroadcast (suspendUpdateCond);
		SDL_CondWait (suspendUpdateCond, suspendUpdateMutex);
	}
	SDL_UnlockMutex (suspendUpdateMutex);

	if (isShuttingDown) {
		if (SoundMixer::instance->isStopComplete () && Network::instance->isStopComplete () && TaskGroup::instance->isStopComplete ()) {
			SoundMixer::instance->waitThreads ();
			Network::instance->waitThreads ();
			TaskGroup::instance->waitThreads ();
			isShutdown = true;
		}
	}
}

void App::executeUpdateTasks () {
	std::list<App::UpdateTaskContext>::const_iterator i1, i2;

	updateTaskList.clear ();
	SDL_LockMutex (updateTaskMutex);
	updateTaskList.swap (updateTaskAddList);
	SDL_UnlockMutex (updateTaskMutex);

	i1 = updateTaskList.cbegin ();
	i2 = updateTaskList.cend ();
	while (i1 != i2) {
		i1->fn (i1->fnData);
		SDL_LockMutex (taskStateMapMutex);
		taskStateMap.erase (i1->id);
		SDL_UnlockMutex (taskStateMapMutex);
		++i1;
	}
	updateTaskList.clear ();
}

bool App::keyEvent (void *ptr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isControlDown) {
		if (keycode == quitKeycode) {
			Input::instance->windowClose ();
			return (true);
		}
		if (keycode == settingsKeycode) {
			UiStack::instance->toggleSettingsWindow ();
			return (true);
		}
		if (keycode == helpKeycode) {
			UiStack::instance->toggleHelpWindow ();
			return (true);
		}
		if (keycode == logKeycode) {
			UiStack::instance->toggleLogWindow ();
			return (true);
		}
		if (keycode == consoleKeycode) {
			UiStack::instance->toggleConsoleWindow ();
			return (true);
		}
	}
	if (UiStack::instance->processKeyEvent (keycode, isShiftDown, isControlDown)) {
		return (true);
	}
	return (false);
}

int64_t App::getUniqueId () {
	int64_t id;

	SDL_LockMutex (uniqueIdMutex);
	id = nextUniqueId;
	++nextUniqueId;
	SDL_UnlockMutex (uniqueIdMutex);
	return (id);
}

void App::suspendUpdate () {
	if (isUpdateThreadEnded) {
		return;
	}
	SDL_LockMutex (suspendUpdateMutex);
	if (! isUpdateThreadEnded) {
		isSuspendingUpdate = true;
		SDL_CondWait (suspendUpdateCond, suspendUpdateMutex);
	}
	SDL_UnlockMutex (suspendUpdateMutex);
}

void App::unsuspendUpdate () {
	SDL_LockMutex (suspendUpdateMutex);
	isSuspendingUpdate = false;
	SDL_CondBroadcast (suspendUpdateCond);
	SDL_UnlockMutex (suspendUpdateMutex);
}

void App::pushClipRect (const SDL_Rect &rect, bool disableIntersection) {
	int x, y, w, h, diff;

	x = rect.x;
	y = rect.y;
	w = rect.w;
	h = rect.h;
	if ((! clipRectStack.empty ()) && (! disableIntersection)) {
		diff = x - clipRect.x;
		if (diff < 0) {
			w += diff;
			x = clipRect.x;
		}
		diff = y - clipRect.y;
		if (diff < 0) {
			h += diff;
			y = clipRect.y;
		}
		diff = (x + w) - (clipRect.x + clipRect.w);
		if (diff > 0) {
			w -= diff;
		}
		diff = (y + h) - (clipRect.y + clipRect.h);
		if (diff > 0) {
			h -= diff;
		}

		if (w < 0) {
			w = 0;
		}
		if (h < 0) {
			h = 0;
		}
	}

	clipRect.x = x;
	clipRect.y = y;
	clipRect.w = w;
	clipRect.h = h;
	SDL_RenderSetClipRect (render, &clipRect);
	clipRectStack.push (clipRect);
}

void App::popClipRect () {
	if (clipRectStack.empty ()) {
		return;
	}
	clipRectStack.pop ();
	if (clipRectStack.empty ()) {
		clipRect.x = 0;
		clipRect.y = 0;
		clipRect.w = windowWidth;
		clipRect.h = windowHeight;
		SDL_RenderSetClipRect (render, NULL);
	}
	else {
		clipRect = clipRectStack.top ();
		SDL_RenderSetClipRect (render, &clipRect);
	}
}

void App::suspendClipRect () {
	SDL_RenderSetClipRect (render, NULL);
}
void App::unsuspendClipRect () {
	SDL_RenderSetClipRect (render, &clipRect);
}

void App::setDisplayMode (int mode) {
	if ((mode < 0) || (mode >= RenderResource::displayModeCount)) {
		return;
	}
	CaptureWriter::instance->stopMediaWriter ();
	addPostdrawTask (App::resizeWindow, &(RenderResource::instance->displayModes[mode]));
	SDL_LockMutex (prefsMapMutex);
	prefsMap.insert (App::displayModeKey, RenderResource::instance->displayModes[mode].name);
	SDL_UnlockMutex (prefsMapMutex);
}
void App::resizeWindow (void *modePtr) {
	App::instance->executeResizeWindow (modePtr);
}
void App::executeResizeWindow (void *modePtr) {
	RenderResource::DisplayMode *m;
	int result;

	m = (RenderResource::DisplayMode *) modePtr;
	CaptureWriter::instance->stopMediaWriter ();
	UiStack::instance->setPointerSprite (NULL);
	suspendUpdate ();

	if (m->isFullscreen) {
		if (! isFullscreen) {
			SDL_SetWindowBordered (window, SDL_FALSE);
			SDL_SetWindowFullscreen (window, SDL_WINDOW_FULLSCREEN_DESKTOP);
			isFullscreen = true;
		}
	}
	else {
		if (isFullscreen) {
			SDL_SetWindowFullscreen (window, 0);
			SDL_SetWindowBordered (window, SDL_TRUE);
			isFullscreen = false;
		}
		SDL_SetWindowSize (window, m->width, m->height);
	}

	appUtil.setDrawableSize (window, &windowWidth, &windowHeight, &drawableWidth, &drawableHeight, &imagePrefix);
	UiConfiguration::instance->resetScale ();

	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = windowWidth;
	clipRect.h = windowHeight;
	rootPanel->setFixedSize (true, drawableWidth, drawableHeight);

	SpriteGroup::instance->resize ();
	result = UiConfiguration::instance->reloadFonts (fontScale);
	if (result != OpResult::Success) {
		Log::debug ("Failed to reload fonts; fontScale=%.2f err=%i", fontScale, result);
	}
	result = RenderResource::instance->resize ();
	if (result != OpResult::Success) {
		Log::debug ("Failed to reload render resources; err=%i", result);
	}

	UiStack::instance->reloadSprites ();
	rootPanel->resize ();
	UiStack::instance->resize ();
	rootPanel->resetInputState ();
	unsuspendUpdate ();
}

void App::setFontScale (int scale) {
	if ((scale < 0) || (scale >= RenderResource::fontScaleCount)) {
		return;
	}
	addPredrawTask (App::resizeFonts, &(RenderResource::instance->fontScales[scale]));
	SDL_LockMutex (prefsMapMutex);
	prefsMap.insert (App::fontScaleKey, scale);
	SDL_UnlockMutex (prefsMapMutex);
}
void App::resizeFonts (void *doublePtr) {
	App::instance->executeResizeFonts (doublePtr);
}
void App::executeResizeFonts (void *doublePtr) {
	double *scale = (double *) doublePtr;
	int result;

	suspendUpdate ();
	result = UiConfiguration::instance->reloadFonts (*scale);
	if (result != OpResult::Success) {
		Log::debug ("Failed to resize fonts; scale=%.3f err=%i", *scale, result);
	}
	else {
		fontScale = *scale;
		rootPanel->resize ();
		UiStack::instance->resize ();
		rootPanel->resetInputState ();
	}
	unsuspendUpdate ();
}

void App::setUiActive () {
	SDL_LockMutex (uiActivityMutex);
	++uiActivityCount;
	SDL_UnlockMutex (uiActivityMutex);
}
void App::unsetUiActive () {
	SDL_LockMutex (uiActivityMutex);
	if (uiActivityCount > 0) {
		--uiActivityCount;
	}
	SDL_UnlockMutex (uiActivityMutex);
}

void App::setNetworkActive () {
	SDL_LockMutex (networkActivityMutex);
	++networkActivityCount;
	SDL_UnlockMutex (networkActivityMutex);
}
void App::unsetNetworkActive () {
	SDL_LockMutex (networkActivityMutex);
	if (networkActivityCount > 0) {
		--networkActivityCount;
	}
	SDL_UnlockMutex (networkActivityMutex);
}

int64_t App::addUpdateTask (App::UpdateTaskFunction fn, void *fnData) {
	App::UpdateTaskContext ctx;
	int64_t id;

	if (! fn) {
		return (-1);
	}
	id = getUniqueId ();
	ctx.id = id;
	ctx.fn = fn;
	ctx.fnData = fnData;

	SDL_LockMutex (taskStateMapMutex);
	taskStateMap.insert (std::pair<int64_t, bool> (id, true));
	SDL_UnlockMutex (taskStateMapMutex);

	SDL_LockMutex (updateTaskMutex);
	updateTaskAddList.push_back (ctx);
	SDL_UnlockMutex (updateTaskMutex);
	return (id);
}

int64_t App::addPredrawTask (App::RenderTaskFunction fn, void *fnData) {
	App::RenderTaskContext ctx;
	int64_t id;

	if (! fn) {
		return (-1);
	}
	id = getUniqueId ();
	ctx.id = id;
	ctx.fn = fn;
	ctx.fnData = fnData;

	SDL_LockMutex (taskStateMapMutex);
	taskStateMap.insert (std::pair<int64_t, bool> (id, true));
	SDL_UnlockMutex (taskStateMapMutex);

	SDL_LockMutex (predrawTaskMutex);
	predrawTaskAddList.push_back (ctx);
	SDL_UnlockMutex (predrawTaskMutex);
	return (id);
}

int64_t App::addPostdrawTask (App::RenderTaskFunction fn, void *fnData) {
	App::RenderTaskContext ctx;
	int64_t id;

	if (! fn) {
		return (-1);
	}
	id = getUniqueId ();
	ctx.id = id;
	ctx.fn = fn;
	ctx.fnData = fnData;

	SDL_LockMutex (taskStateMapMutex);
	taskStateMap.insert (std::pair<int64_t, bool> (id, true));
	SDL_UnlockMutex (taskStateMapMutex);

	SDL_LockMutex (postdrawTaskMutex);
	postdrawTaskAddList.push_back (ctx);
	SDL_UnlockMutex (postdrawTaskMutex);
	return (id);
}

bool App::isTaskRunning (int64_t taskId) {
	std::map<int64_t, bool>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (taskStateMapMutex);
	pos = taskStateMap.find (taskId);
	if (pos != taskStateMap.end ()) {
		result = true;
	}
	SDL_UnlockMutex (taskStateMapMutex);
	return (result);
}

void App::setConsoleWindow (ConsoleWindow *window) {
	SDL_LockMutex (consoleWindowMutex);
	consoleWindowHandle.assign (window);
	SDL_UnlockMutex (consoleWindowMutex);
}

void App::writeConsoleOutput (const StdString &text) {
	if (! consoleWindow) {
		return;
	}
	SDL_LockMutex (consoleWindowMutex);
	consoleWindowHandle.compact ();
	if (consoleWindow) {
		consoleWindow->appendText (text);
	}
	SDL_UnlockMutex (consoleWindowMutex);
}

void App::toggleMousePointer () {
	if (UiStack::instance->isPointerDrawEnabled) {
		UiStack::instance->setPointerSprite (NULL);
	}
	else {
		UiStack::instance->setPointerSprite (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_pointerIcon), pointerOffsetScaleX, pointerOffsetScaleY);
	}
}

void App::writePrefs () {
	int result;

	if (isPrefsWriteDisabled) {
		return;
	}
	SDL_LockMutex (prefsMapMutex);
	if (prefsMap.isWriteDirty) {
		result = prefsMap.write (prefsPath);
		if (result != OpResult::Success) {
			Log::err ("Failed to write prefs file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			isPrefsWriteDisabled = true;
		}
	}
	SDL_UnlockMutex (prefsMapMutex);
}

void App::showNotification (const StdString &messageText) {
	UiStack::instance->showSnackbar (messageText);
}
