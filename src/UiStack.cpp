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
#include "Log.h"
#include "Input.h"
#include "HashMap.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Resource.h"
#include "Button.h"
#include "UiText.h"
#include "AppUrl.h"
#include "Menu.h"
#include "TextField.h"
#include "Toolbar.h"
#include "MainToolbarWindow.h"
#include "TooltipWindow.h"
#include "SettingsWindow.h"
#include "HelpWindow.h"
#include "UiLogWindow.h"
#include "SnackbarWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "ConsoleWindow.h"
#include "TaskGroup.h"
#include "LuaScript.h"
#include "MediaUtil.h"
#include "TextFieldWindow.h"
#include "UiStack.h"

UiStack *UiStack::instance = NULL;

constexpr const int SetUiCommand = 0;
constexpr const int PushUiCommand = 1;
constexpr const int PopUiCommand = 2;

constexpr const double consoleWindowScale = 0.84f;
constexpr const double sidebarWindowWidthScale = 0.33f;
constexpr const int clearOverlayKeycode = SDLK_ESCAPE;

UiStack::UiStack ()
: isMouseHoverActive (false)
, isMouseHoverSuspended (false)
, isPointerDrawEnabled (false)
, mainToolbar (NULL)
, secondaryToolbar (NULL)
, mainToolbarWindow (NULL)
, topBarHeight (0.0f)
, bottomBarHeight (0.0f)
, activeUi (NULL)
, nextCommandType (-1)
, nextCommandUi (NULL)
, tooltipHandle (&tooltip)
, keyFocusTargetHandle (&keyFocusTarget)
, mouseHoverTargetHandle (&mouseHoverTarget)
, appMenuHandle (&appMenu)
, darkenPanelHandle (&darkenPanel)
, settingsWindowHandle (&settingsWindow)
, helpWindowHandle (&helpWindow)
, logWindowHandle (&logWindow)
, dialogWindowHandle (&dialogWindow)
, consoleWindowHandle (&consoleWindow)
, snackbarWindowHandle (&snackbarWindow)
, isUiInputSuspended (false)
, mouseHoverClock (0)
, backgroundTexture (NULL)
, backgroundTextureWidth (0)
, backgroundTextureHeight (0)
, nextBackgroundTexture (NULL)
, nextBackgroundTextureWidth (0)
, nextBackgroundTextureHeight (0)
, backgroundCrossFadeAlpha (0.0f)
, pointerTexture (NULL)
, pointerTextureWidth (0)
, pointerTextureHeight (0)
, pointerOffsetScaleX (0.0f)
, pointerOffsetScaleY (0.0f)
{
	SdlUtil::createMutex (&uiMutex);
	SdlUtil::createMutex (&nextCommandMutex);
	SdlUtil::createMutex (&backgroundMutex);
	SdlUtil::createMutex (&pointerMutex);
}
UiStack::~UiStack () {
	clear ();
	if (activeUi) {
		activeUi->release ();
		activeUi = NULL;
	}
	if (nextCommandUi) {
		nextCommandUi->release ();
		nextCommandUi = NULL;
	}
	SdlUtil::destroyMutex (&uiMutex);
	SdlUtil::destroyMutex (&nextCommandMutex);
	SdlUtil::destroyMutex (&backgroundMutex);
	SdlUtil::destroyMutex (&pointerMutex);
}

void UiStack::createInstance () {
	if (! UiStack::instance) {
		UiStack::instance = new UiStack ();
	}
}
void UiStack::freeInstance () {
	if (UiStack::instance) {
		delete (UiStack::instance);
		UiStack::instance = NULL;
	}
}

void UiStack::populateWidgets () {
	HashMap *prefs;
	bool enable;

	if (! mainToolbar) {
		mainToolbar = (Toolbar *) App::instance->rootPanel->addWidget (new Toolbar (App::instance->drawableWidth), 1);
		mainToolbar->retain ();

		prefs = App::instance->lockPrefs ();
		enable = prefs->find (UiStack::showClockKey, false);
		App::instance->unlockPrefs ();

		mainToolbarWindow = new MainToolbarWindow ();
		mainToolbarWindow->retain ();
		mainToolbarWindow->setClockEnabled (enable);
		mainToolbar->setLeftCorner (mainToolbarWindow);
	}
	topBarHeight = mainToolbar->position.y + mainToolbar->height;

	if (! secondaryToolbar) {
		secondaryToolbar = (Toolbar *) App::instance->rootPanel->addWidget (new Toolbar (App::instance->drawableWidth), 1);
		secondaryToolbar->retain ();
		secondaryToolbar->modeChangeCallback = Widget::EventCallbackContext (UiStack::secondaryToolbarModeChanged, this);
		secondaryToolbar->isVisible = false;
		secondaryToolbar->position.assign (0.0f, App::instance->drawableHeight - secondaryToolbar->height);
	}
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;

	assignOverlayZLevels ();
}

void UiStack::clear () {
	Ui *ui;

	tooltipHandle.destroyAndClear ();
	keyFocusTargetHandle.clear ();
	mouseHoverTargetHandle.clear ();
	snackbarWindowHandle.destroyAndClear ();
	appMenuHandle.destroyAndClear ();
	darkenPanelHandle.destroyAndClear ();
	settingsWindowHandle.destroyAndClear ();
	helpWindowHandle.destroyAndClear ();
	logWindowHandle.destroyAndClear ();
	dialogWindowHandle.destroyAndClear ();
	consoleWindowHandle.destroyAndClear ();
	stopPlayers ();

	if (mainToolbarWindow) {
		mainToolbarWindow->isDestroyed = true;
		mainToolbarWindow->release ();
		mainToolbarWindow = NULL;
	}
	if (mainToolbar) {
		mainToolbar->isDestroyed = true;
		mainToolbar->release ();
		mainToolbar = NULL;
	}
	if (secondaryToolbar) {
		secondaryToolbar->isDestroyed = true;
		secondaryToolbar->release ();
		secondaryToolbar = NULL;
	}
	if (backgroundTexture) {
		backgroundTexture = NULL;
		Resource::instance->unloadTexture (backgroundTexturePath);
		backgroundTexturePath.assign ("");
	}
	if (nextBackgroundTexture) {
		nextBackgroundTexture = NULL;
		Resource::instance->unloadTexture (nextBackgroundTexturePath);
		nextBackgroundTexturePath.assign ("");
	}

	isPointerDrawEnabled = false;
	SDL_LockMutex (pointerMutex);
	if (pointerTexture) {
		pointerTexture = NULL;
		Resource::instance->unloadTexture (pointerTexturePath);
		pointerTexturePath.assign ("");
	}
	SDL_UnlockMutex (pointerMutex);

	SDL_LockMutex (uiMutex);
	while (! uiList.empty ()) {
		ui = uiList.back ();
		ui->pause ();
		ui->unload ();
		ui->release ();
		uiList.pop_back ();
	}
	SDL_UnlockMutex (uiMutex);
}

Ui *UiStack::getActiveUi () {
	Ui *ui;

	ui = NULL;
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		ui = activeUi;
		ui->retain ();
	}
	SDL_UnlockMutex (uiMutex);
	return (ui);
}

void UiStack::setUi (Ui *ui) {
	if (! ui) {
		return;
	}
	SDL_LockMutex (nextCommandMutex);
	nextCommandType = SetUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = ui;
	nextCommandUi->retain ();
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::pushUi (Ui *ui) {
	if (! ui) {
		return;
	}
	SDL_LockMutex (nextCommandMutex);
	nextCommandType = PushUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = ui;
	nextCommandUi->retain ();
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::popUi () {
	SDL_LockMutex (nextCommandMutex);
	nextCommandType = PopUiCommand;
	if (nextCommandUi) {
		nextCommandUi->release ();
	}
	nextCommandUi = NULL;
	SDL_UnlockMutex (nextCommandMutex);
}

void UiStack::drawBackground () {
	SDL_Rect rect;

	SDL_LockMutex (backgroundMutex);
	if (! nextBackgroundTexturePath.empty ()) {
		if (! nextBackgroundTexture) {
			nextBackgroundTextureWidth = 0;
			nextBackgroundTextureHeight = 0;
			nextBackgroundTexture = Resource::instance->loadTexture (nextBackgroundTexturePath);
			if (! nextBackgroundTexture) {
				Log::err ("Failed to load background texture; path=%s", nextBackgroundTexturePath.c_str ());
				nextBackgroundTexturePath.assign ("");
			}
			else {
				SDL_QueryTexture (nextBackgroundTexture, NULL, NULL, &nextBackgroundTextureWidth, &nextBackgroundTextureHeight);
				backgroundCrossFadeAlpha = 0.0f;
			}
		}
	}

	if (backgroundTexture && nextBackgroundTexture) {
		rect.w = backgroundTextureWidth;
		rect.h = backgroundTextureHeight;
		rect.x = (int) ((App::instance->drawableWidth - (double) rect.w) / 2.0f);
		rect.y = (int) ((App::instance->drawableHeight - (double) rect.h) / 2.0f);
		SDL_SetTextureBlendMode (backgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (App::instance->render, backgroundTexture, NULL, &rect);

		rect.w = nextBackgroundTextureWidth;
		rect.h = nextBackgroundTextureHeight;
		rect.x = (int) ((App::instance->drawableWidth - (double) rect.w) / 2.0f);
		rect.y = (int) ((App::instance->drawableHeight - (double) rect.h) / 2.0f);
		SDL_SetTextureBlendMode (nextBackgroundTexture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod (nextBackgroundTexture, (Uint8) (backgroundCrossFadeAlpha * 255.0f));
		SDL_RenderCopy (App::instance->render, nextBackgroundTexture, NULL, &rect);
	}
	else if (nextBackgroundTexture) {
		rect.w = nextBackgroundTextureWidth;
		rect.h = nextBackgroundTextureHeight;
		rect.x = (int) ((App::instance->drawableWidth - (double) rect.w) / 2.0f);
		rect.y = (int) ((App::instance->drawableHeight - (double) rect.h) / 2.0f);
		SDL_SetTextureBlendMode (nextBackgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (App::instance->render, nextBackgroundTexture, NULL, &rect);
	}
	else if (backgroundTexture) {
		rect.w = backgroundTextureWidth;
		rect.h = backgroundTextureHeight;
		rect.x = (int) ((App::instance->drawableWidth - (double) rect.w) / 2.0f);
		rect.y = (int) ((App::instance->drawableHeight - (double) rect.h) / 2.0f);
		SDL_SetTextureBlendMode (backgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (App::instance->render, backgroundTexture, NULL, &rect);
	}
	SDL_UnlockMutex (backgroundMutex);
}

void UiStack::drawForeground () {
	SDL_Rect rect;

	if (isPointerDrawEnabled) {
		SDL_LockMutex (pointerMutex);
		if (pointerTexture) {
			rect.x = Input::instance->mouseX - (int) (pointerOffsetScaleX * (double) pointerTextureWidth);
			rect.y = Input::instance->mouseY - (int) (pointerOffsetScaleY * (double) pointerTextureHeight);
			rect.w = pointerTextureWidth;
			rect.h = pointerTextureHeight;
			SDL_SetTextureColorMod (pointerTexture, pointerColor.rByte, pointerColor.gByte, pointerColor.bByte);
			SDL_SetTextureAlphaMod (pointerTexture, pointerColor.aByte);
			SDL_RenderCopy (App::instance->render, pointerTexture, NULL, &rect);
		}
		SDL_UnlockMutex (pointerMutex);
	}
}

void UiStack::update (int msElapsed) {
	Ui *ui;
	Widget *mousewidget;
	bool fullscreen;

	SDL_LockMutex (uiMutex);
	if (! uiList.empty ()) {
		ui = uiList.back ();
		if (ui != activeUi) {
			if (activeUi) {
				activeUi->pause ();
				activeUi->release ();
			}
			activeUi = ui;
			activeUi->retain ();

			tooltipHandle.destroyAndClear ();
			keyFocusTargetHandle.clear ();
			mouseHoverTargetHandle.clear ();
			snackbarWindowHandle.destroyAndClear ();
			clearOverlay ();
			resetToolbars ();
			playerControl.assignPlayerPositions ();
			assignOverlayZLevels ();
			activeUi->resume ();
			App::instance->shouldSyncRecordStore = true;
			mainToolbar->isInputSuspended = false;
			secondaryToolbar->isInputSuspended = false;
			isUiInputSuspended = false;
		}
	}
	SDL_UnlockMutex (uiMutex);

	SDL_LockMutex (backgroundMutex);
	if (nextBackgroundTexture) {
		if (UiConfiguration::instance->backgroundCrossFadeDuration <= 0) {
			backgroundCrossFadeAlpha = 1.0f;
		}
		else {
			backgroundCrossFadeAlpha += (1.0f / ((double) UiConfiguration::instance->backgroundCrossFadeDuration)) * (double) msElapsed;
		}
		if (backgroundCrossFadeAlpha >= 1.0f) {
			backgroundCrossFadeAlpha = 1.0f;
			if (backgroundTexture) {
				Resource::instance->unloadTexture (backgroundTexturePath);
			}
			backgroundTexture = nextBackgroundTexture;
			backgroundTexturePath.assign (nextBackgroundTexturePath);
			backgroundTextureWidth = nextBackgroundTextureWidth;
			backgroundTextureHeight = nextBackgroundTextureHeight;
			nextBackgroundTexture = NULL;
			nextBackgroundTexturePath.assign ("");
			nextBackgroundTextureWidth = 0;
			nextBackgroundTextureHeight = 0;
		}
	}
	SDL_UnlockMutex (backgroundMutex);

	if (isPointerDrawEnabled) {
		SDL_LockMutex (pointerMutex);
		pointerColor.update (msElapsed);
		SDL_UnlockMutex (pointerMutex);
	}

	tooltipHandle.compact ();
	keyFocusTargetHandle.compact ();
	mouseHoverTargetHandle.compact ();
	appMenuHandle.compact ();
	darkenPanelHandle.compact ();
	settingsWindowHandle.compact ();
	helpWindowHandle.compact ();
	logWindowHandle.compact ();
	dialogWindowHandle.compact ();
	consoleWindowHandle.compact ();

	fullscreen = playerControl.isFullscreenPlaying;
	playerControl.update (msElapsed);
	if (fullscreen != playerControl.isFullscreenPlaying) {
		assignOverlayZLevels ();
	}

	if (keyFocusTarget && (! keyFocusTarget->isKeyFocused)) {
		keyFocusTargetHandle.clear ();
	}

	mousewidget = App::instance->rootPanel->findWidget ((double) Input::instance->mouseX, (double) Input::instance->mouseY, true);
	if (! mousewidget) {
		mouseHoverTargetHandle.clear ();
		deactivateMouseHover ();
	}
	else {
		if (! mouseHoverTargetHandle.equals (mousewidget)) {
			mouseHoverTargetHandle.assign (mousewidget);
			deactivateMouseHover ();
		}
		else {
			if ((! isMouseHoverActive) && (! isMouseHoverSuspended)) {
				mouseHoverClock -= msElapsed;
				if (mouseHoverClock <= 0) {
					activateMouseHover ();
				}
			}
		}
	}

	if (darkenPanel && (! isDarkenOverlayActive ())) {
		darkenPanelHandle.destroyAndClear ();
		assignOverlayZLevels ();
	}

	if (isUiInputSuspended) {
		if ((! darkenPanel) && (! isDarkenOverlayActive ())) {
			ui = getActiveUi ();
			if (ui) {
				ui->rootPanel->isInputSuspended = false;
				ui->release ();
			}
			mainToolbar->isInputSuspended = false;
			secondaryToolbar->isInputSuspended = false;
			isUiInputSuspended = false;
		}
	}
}

void UiStack::resetToolbars () {
	mainToolbar->clearRightItems ();
	secondaryToolbar->clearAll ();

	mainToolbar->addRightItem (Ui::createToolbarIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_appMenuButton), Widget::EventCallbackContext (UiStack::appMenuButtonClicked, this), UiText::instance->getText (UiTextId::MainMenuTooltip), "mainToolbarMenuButton"));
	if (activeUi) {
		activeUi->addMainToolbarItems (mainToolbar);
		activeUi->addSecondaryToolbarItems (secondaryToolbar);
	}

	if (uiList.size () > 1) {
		mainToolbar->addRightItem (Ui::createToolbarIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (UiStack::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mainToolbarBackButton"));
	}

	if (secondaryToolbar->empty ()) {
		secondaryToolbar->isVisible = false;
	}
	else {
		secondaryToolbar->isVisible = true;
	}
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	secondaryToolbar->position.assign (0.0f, App::instance->drawableHeight - secondaryToolbar->height);
}

void UiStack::executeStackCommands () {
	Ui *ui, *item;
	int cmd, result;

	SDL_LockMutex (nextCommandMutex);
	cmd = nextCommandType;
	ui = nextCommandUi;
	nextCommandType = -1;
	nextCommandUi = NULL;
	SDL_UnlockMutex (nextCommandMutex);

	if (cmd < 0) {
		if (ui) {
			ui->release ();
		}
		return;
	}

	switch (cmd) {
		case SetUiCommand: {
			if (! ui) {
				break;
			}
			result = ui->load ();
			if (result != OpResult::Success) {
				Log::err ("Failed to load UI resources; err=%i", result);
				ui->release ();
				break;
			}

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			while (! uiList.empty ()) {
				item = uiList.back ();
				item->pause ();
				item->unload ();
				item->release ();
				uiList.pop_back ();
			}
			uiList.push_back (ui);
			SDL_UnlockMutex (uiMutex);
			App::instance->unsuspendUpdate ();
			break;
		}
		case PushUiCommand: {
			if (! ui) {
				break;
			}
			result = ui->load ();
			if (result != OpResult::Success) {
				Log::err ("Failed to load UI resources; err=%i", result);
				ui->release ();
				break;
			}

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			uiList.push_back (ui);
			SDL_UnlockMutex (uiMutex);
			App::instance->unsuspendUpdate ();
			break;
		}
		case PopUiCommand: {
			if (ui) {
				ui->release ();
			}
			ui = NULL;

			App::instance->suspendUpdate ();
			SDL_LockMutex (uiMutex);
			if (! uiList.empty ()) {
				ui = uiList.back ();
				uiList.pop_back ();
				if (activeUi == ui) {
					activeUi->release ();
					activeUi = NULL;
				}
			}
			SDL_UnlockMutex (uiMutex);

			if (ui) {
				ui->pause ();
				ui->unload ();
				ui->release ();
			}
			App::instance->unsuspendUpdate ();
			break;
		}
		default: {
			if (ui) {
				ui->release ();
			}
			break;
		}
	}
}

void UiStack::reloadSprites () {
	std::list<Ui *>::iterator i1, i2;

	SDL_LockMutex (uiMutex);
	i1 = uiList.begin ();
	i2 = uiList.end ();
	while (i1 != i2) {
		(*i1)->reloadSprites ();
		++i1;
	}
	SDL_UnlockMutex (uiMutex);

	mainToolbar->reflow ();
	secondaryToolbar->reflow ();
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	secondaryToolbar->position.assign (0.0f, App::instance->drawableHeight - secondaryToolbar->height);
}

void UiStack::resize () {
	std::list<Ui *>::iterator i1, i2;

	mainToolbar->setWidth (App::instance->drawableWidth);
	secondaryToolbar->setWidth (App::instance->drawableWidth);
	mainToolbar->resize ();
	secondaryToolbar->resize ();
	mainToolbar->reflow ();
	secondaryToolbar->reflow ();
	secondaryToolbar->position.assign (0.0f, App::instance->drawableHeight - secondaryToolbar->height);
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;

	SDL_LockMutex (uiMutex);
	i1 = uiList.begin ();
	i2 = uiList.end ();
	while (i1 != i2) {
		(*i1)->resize ();
		++i1;
	}
	SDL_UnlockMutex (uiMutex);

	if (! backgroundTextureBasePath.empty ()) {
		setNextBackgroundTexturePath (backgroundTextureBasePath);
	}

	if (settingsWindow) {
		settingsWindow->reflow ();
		settingsWindow->position.assign (App::instance->drawableWidth - settingsWindow->width, 0.0f);
		if (darkenPanel) {
			darkenPanel->setFixedSize (true, App::instance->rootPanel->width - settingsWindow->width, App::instance->rootPanel->height);
		}
	}
	mainToolbar->resetInputState ();
	secondaryToolbar->resetInputState ();
	mainToolbar->reflow ();
	secondaryToolbar->reflow ();
	secondaryToolbar->position.assign (0.0f, App::instance->drawableHeight - secondaryToolbar->height);
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	if (snackbarWindow) {
		snackbarWindow->resize ();
		snackbarWindow->position.assign (App::instance->drawableWidth - snackbarWindow->width, topBarHeight);
	}
	playerControl.resize ();
	assignOverlayZLevels ();
}

void UiStack::processShutdownEvent () {
	Ui *ui;

	clearOverlay ();
	stopPlayers ();
	ui = getActiveUi ();
	if (ui) {
		ui->processShutdownEvent ();
		ui->release ();
	}
}

void UiStack::setNextBackgroundTexturePath (const StdString &path) {
	StdString filepath;

	backgroundTextureBasePath.assign (path);
	filepath.sprintf ("%s/%s/000.png", backgroundTextureBasePath.c_str (), App::instance->imagePrefix.c_str ());
	SDL_LockMutex (backgroundMutex);
	if (! filepath.equals (nextBackgroundTexturePath)) {
		if (nextBackgroundTexture) {
			nextBackgroundTexture = NULL;
			Resource::instance->unloadTexture (nextBackgroundTexturePath);
			nextBackgroundTexturePath.assign ("");
		}
		nextBackgroundTexturePath.assign (filepath);
	}
	SDL_UnlockMutex (backgroundMutex);
}
void UiStack::setNextBackgroundTexturePath (const char *path) {
	setNextBackgroundTexturePath (StdString (path));
}

void UiStack::setPointerSprite (Sprite *sprite, double offsetScaleX, double offsetScaleY) {
	constexpr const double pointerColorBrightness1 = 0.5f;
	constexpr const double pointerColorBrightness2 = 1.0f;
	constexpr const double pointerColorAlpha = 0.78f;
	constexpr const int pointerColorAnimationDuration = 1000;
	constexpr const int pointerColorAnimationDelay = 200;

	if (! sprite) {
		isPointerDrawEnabled = false;
		SDL_LockMutex (pointerMutex);
		if (pointerTexture) {
			pointerTexture = NULL;
			Resource::instance->unloadTexture (pointerTexturePath);
			pointerTexturePath.assign ("");
		}
		SDL_UnlockMutex (pointerMutex);
	}
	else {
		SDL_LockMutex (pointerMutex);
		if (pointerTexture) {
			pointerTexture = NULL;
			Resource::instance->unloadTexture (pointerTexturePath);
			pointerTexturePath.assign ("");
		}
		pointerTexture = sprite->getTexture (0, &pointerTextureWidth, &pointerTextureHeight, &pointerTexturePath);
		if (! Resource::instance->loadTexture (pointerTexturePath, true)) {
			isPointerDrawEnabled = false;
			pointerTexturePath.assign ("");
			pointerTexture = NULL;
		}
		else {
			pointerOffsetScaleX = offsetScaleX;
			pointerOffsetScaleY = offsetScaleY;
			pointerColor.animate (Color (pointerColorBrightness1, pointerColorBrightness1, pointerColorBrightness1, pointerColorAlpha), Color (pointerColorBrightness2, pointerColorBrightness2, pointerColorBrightness2, pointerColorAlpha), pointerColorAnimationDuration, pointerColorAnimationDelay);
			isPointerDrawEnabled = true;
		}
		SDL_UnlockMutex (pointerMutex);
	}
}

void UiStack::showSnackbar (const StdString &messageText, const StdString &actionButtonText, Widget::EventCallbackContext actionButtonClickCallback) {
	SnackbarWindow *snackbar;
	double y;

	snackbar = new SnackbarWindow (App::instance->drawableWidth - (UiConfiguration::instance->paddingSize * 2.0f));
	snackbar->setMessageText (messageText);
	if ((! actionButtonText.empty ()) && actionButtonClickCallback.callback) {
		snackbar->setActionButtonEnabled (true, actionButtonText, actionButtonClickCallback);
	}
	else {
		snackbar->setActionButtonEnabled (false);
	}

	if (settingsWindow || helpWindow || logWindow) {
		y = 0.0f;
	}
	else {
		y = topBarHeight;
	}
	snackbar->position.assign (App::instance->drawableWidth - snackbar->width, y);
	snackbar->startTimeout (UiConfiguration::instance->snackbarTimeout);
	snackbar->startScroll (UiConfiguration::instance->snackbarScrollDuration);
	snackbarWindowHandle.destroyAndAssign (snackbar);
	App::instance->rootPanel->addWidget (snackbar);
	assignOverlayZLevels ();
}

bool UiStack::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (keycode == clearOverlayKeycode) {
		if (isDarkenOverlayActive ()) {
			clearOverlay ();
			return (true);
		}
	}
	return (false);
}

void UiStack::setKeyFocusTarget (Widget *widget) {
	if (keyFocusTarget == widget) {
		return;
	}
	if (keyFocusTarget && (! keyFocusTarget->isDestroyed)) {
		keyFocusTarget->setKeyFocus (false);
	}
	keyFocusTargetHandle.assign (widget);
	if (keyFocusTarget) {
		keyFocusTarget->setKeyFocus (true);
	}
}

void UiStack::activateMouseHover () {
	StdString text;
	double x, y, max;

	tooltipHandle.destroyAndClear ();
	if (mouseHoverTarget) {
		text.assign (mouseHoverTarget->tooltipText);
	}
	if (mouseHoverTarget && (! text.empty ())) {
		tooltipHandle.destroyAndAssign (new TooltipWindow (text));

		x = mouseHoverTarget->screenX;
		y = mouseHoverTarget->screenY;
		switch (mouseHoverTarget->tooltipAlignment) {
			case Widget::TopAlignment: {
				x += ((mouseHoverTarget->width / 2.0f) - (tooltip->width / 2.0f));
				y -= (tooltip->height + UiConfiguration::instance->marginSize);

				if (x < UiConfiguration::instance->paddingSize) {
					x = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltip->width - UiConfiguration::instance->paddingSize;
				if (x > max) {
					x = max;
				}

				if (y < UiConfiguration::instance->paddingSize) {
					y = mouseHoverTarget->screenY + mouseHoverTarget->height + UiConfiguration::instance->marginSize;
				}
				break;
			}
			case Widget::LeftAlignment: {
				x -= (tooltip->width + UiConfiguration::instance->marginSize);
				y += ((mouseHoverTarget->height / 2.0f) - (tooltip->height / 2.0f));

				if (y < UiConfiguration::instance->paddingSize) {
					y = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltip->height - UiConfiguration::instance->paddingSize;
				if (y > max) {
					y = max;
				}

				if (x < UiConfiguration::instance->paddingSize) {
					x = mouseHoverTarget->screenX + mouseHoverTarget->width + UiConfiguration::instance->marginSize;
				}
				break;
			}
			case Widget::RightAlignment: {
				x += (mouseHoverTarget->width + UiConfiguration::instance->marginSize);
				y += ((mouseHoverTarget->height / 2.0f) - (tooltip->height / 2.0f));

				if (y < UiConfiguration::instance->paddingSize) {
					y = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->height - tooltip->height - UiConfiguration::instance->paddingSize;
				if (y > max) {
					y = max;
				}

				if ((x + tooltip->width) >= (App::instance->rootPanel->width - UiConfiguration::instance->paddingSize)) {
					x = mouseHoverTarget->screenX - (tooltip->width + UiConfiguration::instance->marginSize);
				}
				break;
			}
			case Widget::BottomAlignment:
			default: {
				x += ((mouseHoverTarget->width / 2.0f) - (tooltip->width / 2.0f));
				y += (mouseHoverTarget->height + UiConfiguration::instance->marginSize);

				if (x < UiConfiguration::instance->paddingSize) {
					x = UiConfiguration::instance->paddingSize;
				}
				max = App::instance->rootPanel->width - tooltip->width - UiConfiguration::instance->paddingSize;
				if (x > max) {
					x = max;
				}

				if ((y + tooltip->height) >= (App::instance->rootPanel->height - UiConfiguration::instance->paddingSize)) {
					y = mouseHoverTarget->screenY - (tooltip->height + UiConfiguration::instance->marginSize);
				}
				break;
			}
		}

		App::instance->rootPanel->addWidget (tooltip, App::instance->rootPanel->maxWidgetZLevel + 1);
		tooltip->position.assign (x, y);
	}

	isMouseHoverSuspended = false;
	isMouseHoverActive = true;
}

void UiStack::deactivateMouseHover () {
	tooltipHandle.destroyAndClear ();
	mouseHoverClock = UiConfiguration::instance->mouseHoverThreshold;
	isMouseHoverSuspended = false;
	isMouseHoverActive = false;
}

void UiStack::suspendUiInput () {
	Ui *ui;

	if (isUiInputSuspended) {
		return;
	}
	isUiInputSuspended = true;
	ui = getActiveUi ();
	if (ui) {
		ui->rootPanel->isInputSuspended = true;
		ui->release ();
	}
	mainToolbar->isInputSuspended = true;
	secondaryToolbar->isInputSuspended = true;
}

void UiStack::suspendMouseHover () {
	tooltipHandle.destroyAndClear ();
	mouseHoverClock = UiConfiguration::instance->mouseHoverThreshold;
	isMouseHoverActive = false;
	isMouseHoverSuspended = true;
}

void UiStack::appMenuButtonClicked (void *itPtr, Widget *widgetPtr) {
	UiStack *it = (UiStack *) itPtr;

	it->suspendMouseHover ();
	if (it->appMenu) {
		it->appMenuHandle.destroyAndClear ();
		return;
	}

	SDL_LockMutex (it->uiMutex);
	if (it->activeUi) {
		it->activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (it->uiMutex);

	it->appMenuHandle.assign (new Menu ());
	it->appMenu->isClickDestroyEnabled = true;
	it->appMenu->addItem (UiText::instance->getText (UiTextId::Help).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_helpButton), Widget::EventCallbackContext (UiStack::helpActionClicked, it));
	it->appMenu->addItem (UiText::instance->getText (UiTextId::Settings).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton), Widget::EventCallbackContext (UiStack::settingsActionClicked, it));
	it->appMenu->addItem (UiText::instance->getText (UiTextId::Log).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_historyButton), Widget::EventCallbackContext (UiStack::logActionClicked, it));
	it->appMenu->addItem (UiText::instance->getText (UiTextId::Console).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_consoleIcon), Widget::EventCallbackContext (UiStack::consoleActionClicked, it));
	it->appMenu->addItem (UiText::instance->getText (UiTextId::Exit).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton), Widget::EventCallbackContext (UiStack::exitActionClicked, it));
	App::instance->rootPanel->addWidget (it->appMenu, App::instance->rootPanel->maxWidgetZLevel + 1);
	it->appMenu->position.assign (widgetPtr->screenX + widgetPtr->width - it->appMenu->width, widgetPtr->screenY + widgetPtr->height);
}

void UiStack::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->popUi ();
}

void UiStack::settingsActionClicked (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->toggleSettingsWindow ();
}

void UiStack::helpActionClicked (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->toggleHelpWindow ();
}

void UiStack::logActionClicked (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->toggleLogWindow ();
}

void UiStack::consoleActionClicked (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->toggleConsoleWindow ();
}

void UiStack::exitActionClicked (void *itPtr, Widget *widgetPtr) {
	App::instance->shutdown ();
}

void UiStack::secondaryToolbarModeChanged (void *itPtr, Widget *widgetPtr) {
	UiStack *it = (UiStack *) itPtr;
	Toolbar *toolbar = (Toolbar *) widgetPtr;
	Ui *ui;

	ui = it->getActiveUi ();
	if (ui) {
		ui->handleSecondaryToolbarModeChange (toolbar->mode);
		ui->release ();
	}
}

void UiStack::clearOverlay () {
	appMenuHandle.destroyAndClear ();
	darkenPanelHandle.destroyAndClear ();
	settingsWindowHandle.destroyAndClear ();
	helpWindowHandle.destroyAndClear ();
	logWindowHandle.destroyAndClear ();
	dialogWindowHandle.destroyAndClear ();
	consoleWindowHandle.destroyAndClear ();
	App::instance->setConsoleWindow (NULL);
}

bool UiStack::isDarkenOverlayActive () {
	return (settingsWindow || helpWindow || logWindow || dialogWindow || consoleWindow);
}

void UiStack::assignOverlayZLevels () {
	int z;
	bool fullscreen;

	z = 1;
	if (mainToolbar) {
		mainToolbar->zLevel = z;
	}
	if (secondaryToolbar) {
		secondaryToolbar->zLevel = z;
	}
	++z;

	fullscreen = playerControl.isFullscreenPlaying;
	if (! fullscreen) {
		z += playerControl.assignPlayerZLevels (z);
	}

	if (darkenPanel) {
		darkenPanel->zLevel = z;
		++z;
	}
	if (logWindow) {
		logWindow->zLevel = z;
		++z;
	}
	if (helpWindow) {
		helpWindow->zLevel = z;
		++z;
	}
	if (consoleWindow) {
		consoleWindow->zLevel = z;
		++z;
	}
	if (settingsWindow) {
		settingsWindow->zLevel = z;
		++z;
	}
	if (snackbarWindow) {
		snackbarWindow->zLevel = z;
		++z;
	}
	if (dialogWindow) {
		dialogWindow->zLevel = z;
		++z;
	}
	if (fullscreen) {
		z += playerControl.assignPlayerZLevels (z);
	}
}

void UiStack::toggleSettingsWindow () {
	if (dialogWindow) {
		return;
	}
	if (settingsWindow) {
		clearOverlay ();
		return;
	}
	clearOverlay ();
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	settingsWindowHandle.assign (new SettingsWindow ());
	darkenPanelHandle.assign (new Panel ());
	darkenPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	darkenPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	darkenPanel->setFixedSize (true, App::instance->rootPanel->width - settingsWindow->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (darkenPanel);
	App::instance->rootPanel->addWidget (settingsWindow);
	settingsWindow->position.assignY (0.0f);
	settingsWindow->position.translateX (App::instance->rootPanel->width, App::instance->rootPanel->width - settingsWindow->width, UiConfiguration::instance->rightDialogTranslateDuration);
	assignOverlayZLevels ();
	suspendUiInput ();
}

void UiStack::toggleHelpWindow () {
	if (dialogWindow) {
		return;
	}
	if (helpWindow) {
		clearOverlay ();
		return;
	}
	clearOverlay ();
	helpWindowHandle.assign (new HelpWindow (App::instance->rootPanel->width * sidebarWindowWidthScale, App::instance->rootPanel->height));
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
		activeUi->setHelpWindowContent (helpWindow);
	}
	SDL_UnlockMutex (uiMutex);

	darkenPanelHandle.assign (new Panel ());
	darkenPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	darkenPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	darkenPanel->setFixedSize (true, App::instance->rootPanel->width - helpWindow->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (darkenPanel);
	App::instance->rootPanel->addWidget (helpWindow);
	helpWindow->position.assignY (0.0f);
	helpWindow->position.translateX (App::instance->rootPanel->width, App::instance->rootPanel->width - helpWindow->width, UiConfiguration::instance->rightDialogTranslateDuration);
	assignOverlayZLevels ();
	suspendUiInput ();
}

void UiStack::toggleLogWindow () {
	if (dialogWindow) {
		return;
	}
	if (logWindow) {
		clearOverlay ();
		return;
	}
	clearOverlay ();
	logWindowHandle.assign (new UiLogWindow (App::instance->rootPanel->width * sidebarWindowWidthScale, App::instance->rootPanel->height));
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	darkenPanelHandle.assign (new Panel ());
	darkenPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	darkenPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	darkenPanel->setFixedSize (true, App::instance->rootPanel->width - logWindow->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (darkenPanel);
	App::instance->rootPanel->addWidget (logWindow);
	logWindow->position.assignY (0.0f);
	logWindow->position.translateX (App::instance->rootPanel->width, App::instance->rootPanel->width - logWindow->width, UiConfiguration::instance->rightDialogTranslateDuration);
	assignOverlayZLevels ();
	suspendUiInput ();
}

void UiStack::toggleConsoleWindow () {
	StringList lines;

	if (dialogWindow) {
		return;
	}
	if (consoleWindow) {
		clearOverlay ();
		return;
	}
	clearOverlay ();
	SDL_LockMutex (uiMutex);
	if (activeUi) {
		activeUi->clearPopupWidgets ();
	}
	SDL_UnlockMutex (uiMutex);

	consoleWindowHandle.assign (new ConsoleWindow (App::instance->drawableWidth * consoleWindowScale, App::instance->drawableHeight * consoleWindowScale));
	consoleWindow->setTitleText (UiText::instance->getText (UiTextId::Console).capitalized ());
	consoleWindow->setTextFieldPrompt (UiText::instance->getText (UiTextId::ConsoleWindowLuaPrompt));
	consoleWindow->textEnterCallback = Widget::EventCallbackContext (UiStack::consoleTextEntered, this);
	consoleWindow->fileRunCallback = Widget::EventCallbackContext (UiStack::consoleFileRun, this);
	consoleWindow->showRunButton (UiText::instance->getText (UiTextId::ConsoleWindowSelectRunFilePrompt), UiText::instance->getText (UiTextId::ConsoleWindowRunButtonTooltip));
	consoleWindow->setWidgetNames (StdString ("consoleWindow"));
	lines.push_back (StdString ());
	lines.push_back (UiText::instance->getText (UiTextId::ConsoleStartText1));
	lines.push_back (UiText::instance->getText (UiTextId::ConsoleStartText2));
	lines.push_back (UiText::instance->getText (UiTextId::ConsoleStartText3));
	consoleWindow->appendText (lines.join ("\n"));

	darkenPanelHandle.assign (new Panel ());
	darkenPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	darkenPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	darkenPanel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (darkenPanel);
	App::instance->rootPanel->addWidget (consoleWindow);
	consoleWindow->position.assign (App::instance->rootPanel->width - consoleWindow->width, (App::instance->rootPanel->height - consoleWindow->height) / 2.0f);
	assignOverlayZLevels ();
	suspendUiInput ();
	App::instance->setConsoleWindow (consoleWindow);
	consoleWindow->assignKeyFocus ();
}

void UiStack::consoleTextEntered (void *itPtr, Widget *widgetPtr) {
	ConsoleWindow *console = (ConsoleWindow *) widgetPtr;
	LuaScript *lua;

	lua = new LuaScript (console->textEnterValue);
	if (! TaskGroup::instance->run (TaskGroup::RunContext (LuaScript::run, lua))) {
		Log::debug ("Failed to execute task LuaScript::run");
		delete (lua);
	}
}

void UiStack::consoleFileRun (void *itPtr, Widget *widgetPtr) {
	ConsoleWindow *console = (ConsoleWindow *) widgetPtr;
	StdString text;
	LuaScript *lua;

	if (console->fileRunPath.empty ()) {
		return;
	}
	text.assign ("dofile(\"");
	text.append (console->fileRunPath.replaced (StdString ("\""), StdString ("\\\"")));
	text.append ("\");");
	lua = new LuaScript (text);
	if (! TaskGroup::instance->run (TaskGroup::RunContext (LuaScript::run, lua))) {
		Log::debug ("Failed to execute task LuaScript::run");
		delete (lua);
	}
}

void UiStack::showDialog (Panel *dialog) {
	clearOverlay ();

	dialogWindowHandle.assign (dialog);
	darkenPanelHandle.assign (new Panel ());
	darkenPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	darkenPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	darkenPanel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	App::instance->rootPanel->addWidget (darkenPanel);
	App::instance->rootPanel->addWidget (dialogWindow);
	dialogWindow->position.assign ((App::instance->drawableWidth - dialogWindow->width) / 2.0f, (App::instance->drawableHeight - dialogWindow->height) / 2.0f);
	assignOverlayZLevels ();
	suspendUiInput ();
}

void UiStack::showImageDialog (ImageWindow *sourceImage) {
	ImageWindow *image;
	double w, h;

	if ((sourceImage->imageLoadType == ImageWindow::NoLoadType) || (! sourceImage->isLoaded ())) {
		return;
	}
	image = new ImageWindow ();
	image->loadEndCallback = Widget::EventCallbackContext (UiStack::imageDialogLoaded, this);
	image->mouseClickCallback = Widget::EventCallbackContext (UiStack::imageDialogClicked, this);
	image->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	image->setLoadingSize (App::instance->drawableWidth * 0.15f, App::instance->drawableHeight * 0.15f);
	image->onLoadFit (App::instance->drawableWidth * 0.99f, App::instance->drawableHeight * 0.99f);

	w = App::instance->drawableWidth * 0.25f;
	if ((sourceImage->imageLoadSourceWidth > 0.0f) && (sourceImage->imageLoadSourceHeight > 0.0f)) {
		h = w * sourceImage->imageLoadSourceHeight / sourceImage->imageLoadSourceWidth;
	}
	else if ((sourceImage->width > 0.0f) && (sourceImage->height > 0.0f)) {
		h = w * sourceImage->height / sourceImage->width;
	}
	else {
		h = w / MediaUtil::defaultAspectRatio;
	}
	image->setFixedSize (true, floor (w), floor (h));

	image->loadSourceCopy (sourceImage);
	showDialog (image);
}

void UiStack::imageDialogClicked (void *itPtr, Widget *widgetPtr) {
	UiStack *it = (UiStack *) itPtr;

	it->dialogWindowHandle.destroyAndClear ();
	it->darkenPanelHandle.destroyAndClear ();
}

void UiStack::imageDialogLoaded (void *itPtr, Widget *widgetPtr) {
	UiStack *it = (UiStack *) itPtr;
	ImageWindow *image = (ImageWindow *) widgetPtr;

	if (! image->isLoaded ()) {
		it->dialogWindowHandle.destroyAndClear ();
		it->darkenPanelHandle.destroyAndClear ();
		it->showSnackbar (UiText::instance->getText (UiTextId::ImageDialogLoadError));
		return;
	}
	image->position.assign ((App::instance->drawableWidth - image->width) / 2.0f, (App::instance->drawableHeight - image->height) / 2.0f);
}

int UiStack::getPlayerCount () {
	return (playerControl.getPlayerCount ());
}

int UiStack::getUnpausedPlayerCount () {
	return (playerControl.getUnpausedPlayerCount ());
}

int UiStack::getPausedPlayerCount () {
	return (playerControl.getPausedPlayerCount ());
}

void UiStack::setPlayerControlOptions (int soundMixVolume, bool isSoundMuted, int visualizerType, bool isSubtitleEnabled) {
	playerControl.playerSoundMixVolume = soundMixVolume;
	playerControl.isPlayerSoundMuted = isSoundMuted;
	playerControl.playerVisualizerType = visualizerType;
	playerControl.playerSubtitleEnabled = isSubtitleEnabled;
}

void UiStack::getPlayerControlOptions (int *soundMixVolume, bool *isSoundMuted, int *visualizerType, bool *isSubtitleEnabled) {
	if (soundMixVolume) {
		*soundMixVolume = playerControl.playerSoundMixVolume;
	}
	if (isSoundMuted) {
		*isSoundMuted = playerControl.isPlayerSoundMuted;
	}
	if (visualizerType) {
		*visualizerType = playerControl.playerVisualizerType;
	}
	if (isSubtitleEnabled) {
		*isSubtitleEnabled = playerControl.playerSubtitleEnabled;
	}
}

void UiStack::playMedia (const StdString &mediaId, int64_t seekTimestamp, bool isDetached) {
	playerControl.playMedia (mediaId, seekTimestamp, isDetached);
	assignOverlayZLevels ();
}

void UiStack::playPlaylist (MediaPlaylistWindow *playlist) {
	playerControl.playPlaylist (playlist);
	assignOverlayZLevels ();
}

void UiStack::stopPlayers () {
	playerControl.stopPlayers ();
}

void UiStack::pausePlayers () {
	playerControl.pausePlayers ();
}

void UiStack::fullscreenPlayers () {
	playerControl.fullscreenPlayers ();
	assignOverlayZLevels ();
}

void UiStack::stopPlaylists () {
	playerControl.stopPlaylists ();
	assignOverlayZLevels ();
}

void UiStack::stopPlaylist (const StdString &playlistId) {
	playerControl.stopPlaylist (playlistId);
	assignOverlayZLevels ();
}
