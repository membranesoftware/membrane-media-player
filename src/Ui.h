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
// Base class for objects that manage application interface elements and associated resources
#ifndef UI_H
#define UI_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "SpriteGroup.h"

class Panel;
class Color;
class Sprite;
class Button;
class Toolbar;
class CardView;
class IconLabelWindow;
class HelpWindow;

class Ui {
public:
	Ui ();
	virtual ~Ui ();

	typedef void (*EventCallback) (void *data, Ui *ui);
	struct EventCallbackContext {
		Ui::EventCallback callback;
		void *callbackData;
		EventCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		EventCallbackContext (Ui::EventCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Read-only data members
	int classId;
	Panel *rootPanel;
	bool isLoaded;
	bool isFirstResumeComplete;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return a boolean value indicating if a Ui pointer matches the provided ClassId value
	static bool isUiClass (Ui *ui, int classIdValue);

	// Load resources as needed to prepare the UI and return a result value
	OpResult load ();

	// Free resources allocated by any previous load operation
	void unload ();

	// Remove and destroy any popup widgets that have been created by the UI
	void clearPopupWidgets ();

	// Reset interface elements as appropriate when the UI becomes active
	void resume ();

	// Reset interface elements as appropriate when the UI becomes inactive
	void pause ();

	// Update interface state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Add draw commands for execution by the application
	void draw ();

	// Reload sprites as appropriate for the current window dimensions
	void reloadSprites ();

	// Reload interface resources as needed to account for a new application window size
	void resize ();

	// Add a widget to the root panel and return the same pointer
	Widget *addWidget (Widget *widget, int zLevel = 0);

	// Remove and destroy all widgets from the root panel
	void clearWidgets ();

	// Invoke any function contained in callback and return a boolean value indicating if a function executed
	bool eventCallback (const Ui::EventCallbackContext &callback);

	// Execute actions appropriate for a received application shutdown event
	void processShutdownEvent ();

	// Change the provided main toolbar object to contain items appropriate for the UI
	void addMainToolbarItems (Toolbar *toolbar);

	// Change the provided secondary toolbar object to contain items appropriate for the UI
	void addSecondaryToolbarItems (Toolbar *toolbar);

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	virtual void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute an interface action to open the named widget, to be completed in an update task with its ID value stored to updateTaskId. Returns a boolean value indicating if the widget was found.
	bool luaOpen (const char *targetName, int64_t *updateTaskId);

	// Execute an interface action to target the named widget, to be completed in an update task with its ID value stored to updateTaskId. Returns a boolean value indicating if the widget was found.
	bool luaTarget (const char *targetName, int64_t *updateTaskId);

	// Execute an interface action to untarget the named widget, to be completed in an update task with its ID value stored to updateTaskId. Returns a boolean value indicating if the widget was found.
	bool luaUntarget (const char *targetName, int64_t *updateTaskId);

	// Execute actions to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Execute actions appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Execute actions appropriate when the secondary toolbar mode changes
	virtual void handleSecondaryToolbarModeChange (int modeValue);

	// detailImageSize values
	static constexpr const int SmallSize = 1;
	static constexpr const int MediumSize = 2;
	static constexpr const int LargeSize = 3;

	// Set the detailImageSize value
	void setDetailImageSize (int detailImageSizeValue);

	// Utility functions for creating widget instances
	static Button *createIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText = StdString (), const char *widgetName = NULL);
	static Button *createScrimIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText = StdString (), const char *widgetName = NULL);
	static Panel *createDarkWindowOverlayPanel ();
	static Button *createToolbarIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText = StdString (), const char *widgetName = NULL, SDL_Keycode shortcutKey = SDLK_UNKNOWN);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	virtual StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	virtual Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	virtual OpResult doLoad ();

	// Unload subclass-specific resources
	virtual void doUnload ();

	// Execute subclass-specific actions appropriate for a clearWidgets operation
	virtual void doClearWidgets ();

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	virtual void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	virtual void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	virtual void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the application
	virtual void doDraw ();

	// Reload subclass-specific interface resources as needed to account for a new application window size
	virtual void doResize ();

	// Execute subclass-specific actions appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Execute subclass-specific actions appropriate for a received window close event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessWindowCloseEvent ();

	// Execute subclass-specific actions appropriate for a received application shutdown event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessShutdownEvent ();

	// Add subclass-specific items to the provided main toolbar object
	virtual void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	virtual void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Return the widget matching targetName that is eligible for a lua open operation, or NULL if no such widget was found. If a widget is returned, retain it and the caller is responsible for releasing it.
	virtual Widget *findLuaOpenWidget (const char *targetName);

	// Return the widget matching targetName that is eligible for a lua target or untarget operation, or NULL if no such widget was found. If a widget is returned, retain it and the caller is responsible for releasing it.
	virtual Widget *findLuaTargetWidget (const char *targetName);

	// Execute the lua open operation using targetWidget
	virtual void executeLuaOpen (Widget *targetWidget);

	// Execute the lua target operation using targetWidget
	virtual void executeLuaTarget (Widget *targetWidget);

	// Execute the lua untarget operation using targetWidget
	virtual void executeLuaUntarget (Widget *targetWidget);

	// Execute subclass-specific actions to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	virtual void doSyncRecordStore ();

	// Add a widget to the root panel and position it as an action popup
	void showActionPopup (Widget *action, Widget *target, Widget::EventCallback sourceFn, const Widget::Rectangle &sourceRect, int xAlignment, int yAlignment);

	// Clear popup widgets and return a boolean value indicating if an action popup matching target and sourceFn was removed
	bool clearActionPopup (Widget *target, Widget::EventCallback sourceFn);

	// Return a newly created Button for use as a toolbar command control
	Button *createToolPopupButton (Sprite *sprite, Widget::EventCallback clickCallback, Widget::EventCallback focusCallback, const StdString &tooltipText, const char *widgetName = NULL, SDL_Keycode shortcutKey = SDLK_UNKNOWN);

	// Show the tool popup panel
	void showToolPopup (Widget *sourceWidget, const StdString &titleText, Widget *detailWidget1 = NULL, Widget *detailWidget2 = NULL);
	static void toolPopupSourceUnfocused (void *itPtr, Widget *widgetPtr);

	// Return a newly created IconLabelWindow for use as a tool popup detail widget
	IconLabelWindow *createToolPopupLabel (Sprite *sprite, const StdString &labelText, const Color &textColor);

	static constexpr const int LeftOfAlignment = 0;
	static constexpr const int LeftEdgeAlignment = 1;
	static constexpr const int XCenteredAlignment = 2;
	static constexpr const int RightEdgeAlignment = 3;
	static constexpr const int RightOfAlignment = 4;
	static constexpr const int TopOfAlignment = 5;
	static constexpr const int TopEdgeAlignment = 6;
	static constexpr const int YCenteredAlignment = 7;
	static constexpr const int BottomEdgeAlignment = 8;
	static constexpr const int BottomOfAlignment = 9;
	// Assign a popup widget's screen position using the provided source rectangle and alignment values
	void assignPopupPosition (Widget *popupWidget, const Widget::Rectangle &popupSourceRect, int xAlignment, int yAlignment);

	// Return a newly created Panel containing the provided elements, as appropriate for use as a CardView row header
	Panel *createRowHeaderPanel (const StdString &headerText = StdString (), Panel *sidePanel = NULL);

	// Return a newly created IconLabelWindow containing elements appropriate for use as a loading icon window
	IconLabelWindow *createLoadingIconWindow ();

	// Return a newly created Button implementing the main toolbar image size function
	Button *createImageSizeButton ();

	// Execute actions appropriate when the detailImageSize value has changed
	virtual void handleDetailImageSizeChange ();

	SpriteGroup sprites;
	CardView *cardView;
	WidgetHandle<Widget> actionWidgetHandle;
	Widget *actionWidget;
	WidgetHandle<Widget> actionTargetHandle;
	Widget *actionTarget;
	Widget::EventCallback actionSource;
	WidgetHandle<Widget> breadcrumbWidgetHandle;
	Widget *breadcrumbWidget;
	WidgetHandle<Panel> toolPopupHandle;
	Panel *toolPopup;
	WidgetHandle<Widget> toolPopupSourceHandle;
	Widget *toolPopupSource;
	int detailImageSize;

private:
	// Callback functions
	static bool keyEvent (void *itPtr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	static void imageSizeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *itPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *itPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *itPtr, Widget *widgetPtr);

	struct LuaWidgetContext {
		Ui *ui;
		Widget *widget;
		LuaWidgetContext ():
			ui (NULL),
			widget (NULL) { }
	};
	// App update tasks for lua operations
	static void executeLuaOpenTask (void *ctxPtr);
	static void executeLuaTargetTask (void *ctxPtr);
	static void executeLuaUntargetTask (void *ctxPtr);

	int refcount;
	SDL_mutex *refcountMutex;
	int lastWindowCloseCount;
};
#endif
