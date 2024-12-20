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
// Class that presents a set of Ui objects as a stack
#ifndef UI_STACK_H
#define UI_STACK_H

#include "Color.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "PlayerControl.h"

class Sprite;
class TooltipWindow;
class Ui;
class StringList;
class Panel;
class Menu;
class Toolbar;
class ImageWindow;
class SnackbarWindow;
class MainToolbarWindow;
class SettingsWindow;
class HelpWindow;
class UiLogWindow;
class ConsoleWindow;
class MediaPlaylistWindow;

class UiStack {
public:
	UiStack ();
	~UiStack ();
	static UiStack *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Key values for the prefs map
	static constexpr const char *showClockKey = "UiStackA";
	static constexpr const char *windowSizeSettingKey = "UiStackB";

	// Read-only data members
	bool isMouseHoverActive;
	bool isMouseHoverSuspended;
	bool isPointerDrawEnabled;
	Toolbar *mainToolbar;
	Toolbar *secondaryToolbar;
	MainToolbarWindow *mainToolbarWindow;
	double topBarHeight;
	double bottomBarHeight;

	// Create the stack's top-level widgets
	void populateWidgets ();

	// Remove all items from the stack
	void clear ();

	// Return the topmost item in the Ui stack, or NULL if no such item was found. If a Ui object is returned by this method, it has been retained and must be released by the caller when no longer needed.
	Ui *getActiveUi ();

	// Remove all Ui objects and add the provided one as the top item in the stack
	void setUi (Ui *ui);

	// Push the provided Ui object to the top of the stack
	void pushUi (Ui *ui);

	// Remove and unload the top item in the Ui stack
	void popUi ();

	// Execute render operations for the main background
	void drawBackground ();

	// Execute render operations for foreground elements
	void drawForeground ();

	// Execute operations to update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Execute operations to change the top item of the Ui stack, as specified by previously received commands. This method must be invoked only from the application's main thread.
	void executeStackCommands ();

	// Reload sprites for all Ui items in the stack as appropriate for the current window dimensions
	void reloadSprites ();

	// Resize all Ui items in the stack as appropriate for the current window dimensions
	void resize ();

	// Execute actions appropriate for a received application shutdown event
	void processShutdownEvent ();

	// Set a resource path that should be used to load a background texture and render it during draw operations
	void setNextBackgroundTexturePath (const StdString &path);
	void setNextBackgroundTexturePath (const char *path);

	// Set a sprite that should be drawn in the Ui foreground at the mouse cursor position. If sprite is NULL, instead remove any active pointer sprite.
	void setPointerSprite (Sprite *sprite, double offsetScaleX = 0.0f, double offsetScaleY = 0.0f);

	// Execute actions appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Show the specified message in the application's snackbar window, optionally including an action button
	void showSnackbar (const StdString &messageText, const StdString &actionButtonText = StdString (), Widget::EventCallbackContext actionButtonClickCallback = Widget::EventCallbackContext ());

	// Toggle the visible state of the settings window
	void toggleSettingsWindow ();

	// Toggle the visible state of the help window
	void toggleHelpWindow ();

	// Toggle the visible state of the log window
	void toggleLogWindow ();

	// Toggle the visible state of the console window
	void toggleConsoleWindow ();

	// Show the provided panel as a dialog
	void showDialog (Panel *dialog);

	// Show a dialog consisting of an image window loaded from the same source as sourceImage
	void showImageDialog (ImageWindow *sourceImage);

	// Set the widget that should be the target of keypress text edit operations
	void setKeyFocusTarget (Widget *widget);

	// Suspend input for the active UI until dialog widgets close
	void suspendUiInput ();

	// Deactivate the mouse hover widget and prevent reactivation until a new mouse hover widget is acquired
	void suspendMouseHover ();

	// Return the number of players in the view
	int getPlayerCount ();

	// Return the number of players in the unpaused state
	int getUnpausedPlayerCount ();

	// Return the number of players in the paused state
	int getPausedPlayerCount ();

	// Set player sound mix options
	void setSoundMixOptions (int soundMixVolume, bool isSoundMuted);

	// Write player sound mix option values into the provided pointers
	void getSoundMixOptions (int *soundMixVolume = NULL, bool *isSoundMuted = NULL);

	// Start playback of a media item
	void playMedia (const StdString &mediaId, int64_t seekTimestamp = 0, bool isDetached = false);

	// Start playback of a media playlist
	void playPlaylist (MediaPlaylistWindow *playlist);

	// Stop all players
	void stopPlayers ();

	// Execute the pause/resume operation for all players
	void pausePlayers ();

	// Reposition players into fullscreen view
	void fullscreenPlayers ();

	// Execute the stop operation for all playlist players
	void stopPlaylists ();

	// Execute the stop operation for the specified playlist player
	void stopPlaylist (const StdString &playlistId);

	// Callback functions
	static void appMenuButtonClicked (void *itPtr, Widget *widgetPtr);
	static void backButtonClicked (void *itPtr, Widget *widgetPtr);
	static void settingsActionClicked (void *itPtr, Widget *widgetPtr);
	static void helpActionClicked (void *itPtr, Widget *widgetPtr);
	static void logActionClicked (void *itPtr, Widget *widgetPtr);
	static void consoleActionClicked (void *itPtr, Widget *widgetPtr);
	static void exitActionClicked (void *itPtr, Widget *widgetPtr);
	static void secondaryToolbarModeChanged (void *itPtr, Widget *widgetPtr);
	static void imageDialogClicked (void *itPtr, Widget *widgetPtr);
	static void imageDialogLoaded (void *itPtr, Widget *widgetPtr);
	static void consoleTextEntered (void *itPtr, Widget *widgetPtr);
	static void consoleFileRun (void *itPtr, Widget *widgetPtr);

private:
	// Execute operations appropriate when mouseHoverTarget has held its current value beyond the hover threshold
	void activateMouseHover ();

	// Deactivate any previously activated mouse hover widgets
	void deactivateMouseHover ();

	// Reset toolbar content as appropriate for the active Ui item. This method should be invoked only while holding a lock on uiMutex.
	void resetToolbars ();

	// Remove and destroy overlay widgets
	void clearOverlay ();

	// Return a boolean value indicating if any active overlay widgets require the darkened background panel
	bool isDarkenOverlayActive ();

	// Assign zLevel values to displayed overlays
	void assignOverlayZLevels ();

	std::list<Ui *> uiList;
	Ui *activeUi;
	SDL_mutex *uiMutex;
	int nextCommandType;
	Ui *nextCommandUi;
	SDL_mutex *nextCommandMutex;
	WidgetHandle<TooltipWindow> tooltipHandle;
	TooltipWindow *tooltip;
	WidgetHandle<Widget> keyFocusTargetHandle;
	Widget *keyFocusTarget;
	WidgetHandle<Widget> mouseHoverTargetHandle;
	Widget *mouseHoverTarget;
	WidgetHandle<Menu> appMenuHandle;
	Menu *appMenu;
	WidgetHandle<Panel> darkenPanelHandle;
	Panel *darkenPanel;
	WidgetHandle<SettingsWindow> settingsWindowHandle;
	SettingsWindow *settingsWindow;
	WidgetHandle<HelpWindow> helpWindowHandle;
	HelpWindow *helpWindow;
	WidgetHandle<UiLogWindow> logWindowHandle;
	UiLogWindow *logWindow;
	WidgetHandle<Panel> dialogWindowHandle;
	Panel *dialogWindow;
	WidgetHandle<ConsoleWindow> consoleWindowHandle;
	ConsoleWindow *consoleWindow;
	WidgetHandle<SnackbarWindow> snackbarWindowHandle;
	SnackbarWindow *snackbarWindow;
	PlayerControl playerControl;
	bool isUiInputSuspended;
	int mouseHoverClock;
	SDL_mutex *backgroundMutex;
	StdString backgroundTextureBasePath;
	SDL_Texture *backgroundTexture;
	StdString backgroundTexturePath;
	int backgroundTextureWidth;
	int backgroundTextureHeight;
	SDL_Texture *nextBackgroundTexture;
	StdString nextBackgroundTexturePath;
	int nextBackgroundTextureWidth;
	int nextBackgroundTextureHeight;
	double backgroundCrossFadeAlpha;
	SDL_Texture *pointerTexture;
	StdString pointerTexturePath;
	int pointerTextureWidth;
	int pointerTextureHeight;
	double pointerOffsetScaleX;
	double pointerOffsetScaleY;
	Color pointerColor;
	SDL_mutex *pointerMutex;
};
#endif
