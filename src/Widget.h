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
// Base class for user interface elements
#ifndef WIDGET_H
#define WIDGET_H

#include <stdint.h>
#include "Position.h"

class StringList;
class PanelLayoutFlow;

class Widget {
public:
	Widget ();
	virtual ~Widget ();

	typedef bool (*FindMatchFunction) (void *data, Widget *itemWidget);
	typedef void (*UpdateCallback) (void *data, int msElapsed, Widget *widget);
	typedef void (*EventCallback) (void *data, Widget *widget);
	typedef bool (*KeyEventCallback) (void *data, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	struct UpdateCallbackContext {
		Widget::UpdateCallback callback;
		void *callbackData;
		UpdateCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		UpdateCallbackContext (Widget::UpdateCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	struct EventCallbackContext {
		Widget::EventCallback callback;
		void *callbackData;
		EventCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		EventCallbackContext (Widget::EventCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	struct KeyEventCallbackContext {
		Widget::KeyEventCallback callback;
		void *callbackData;
		KeyEventCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		KeyEventCallbackContext (Widget::KeyEventCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	struct Rectangle {
		double x;
		double y;
		double w;
		double h;
		Rectangle ():
			x (0.0f),
			y (0.0f),
			w (0.0f),
			h (0.0f) { }
		Rectangle (double x, double y, double w, double h):
			x (x),
			y (y),
			w (w),
			h (h) { }
	};
	enum Alignment {
		TopAlignment = 0,
		LeftAlignment = 1,
		RightAlignment = 2,
		BottomAlignment = 3
	};

	static constexpr const int minZLevel = -10;

	// Read-write data members
	uint64_t id;
	StdString widgetName;
	bool isDestroyed;
	bool isVisible;
	bool isInputSuspended;
	bool isPanelSizeClipEnabled;
	Position position;
	int zLevel;
	bool isMouseHoverEnabled;
	int mouseClickExecuteCount;
	bool shouldComposeRender;
	StdString sortKey;
	Widget::EventCallbackContext mouseEnterCallback;
	Widget::EventCallbackContext mouseExitCallback;
	Widget::EventCallbackContext mousePressCallback;
	Widget::EventCallbackContext mouseReleaseCallback;
	Widget::EventCallbackContext mouseClickCallback;
	Widget::EventCallbackContext mouseLongPressCallback;
	Widget::EventCallbackContext composeAnimationCompleteCallback;
	Widget::KeyEventCallbackContext keyEventCallback;
	Widget::UpdateCallbackContext updateCallback;

	// Read-only data members
	int classId;
	int refcount;
	bool hasScreenPosition;
	double screenX;
	double screenY;
	bool isKeyFocused;
	bool isMouseEntered;
	bool isMousePressed;
	StdString tooltipText;
	Widget::Alignment tooltipAlignment;
	bool isComposeDrawEnabled;
	bool isComposeRendering;
	bool isComposeAnimating;
	double composeWidth;
	double composeHeight;
	double composeScale;
	double composeRotation;

	// Read-only data members. Widget subclasses should maintain these values for proper layout handling.
	double width;
	double height;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return a boolean value indicating if a widget pointer matches the provided ClassId value
	static bool isWidgetClass (Widget *widget, int classIdValue);

	// Return a string description of the widget
	virtual StdString toString ();

	// Reset the widget's input state
	void resetInputState ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void update (int msElapsed, double originX, double originY);

	// Add draw commands for execution by the App
	void draw (double originX = 0.0f, double originY = 0.0f);

	// Update widget state as appropriate after a size change to the application window or loaded fonts
	void resize ();

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	virtual void syncRecordStore ();

	// Return the topmost child widget at the specified screen position, or NULL if no such widget was found. If requireMouseHoverEnabled is true, return a widget only if it has enabled the isMouseHoverEnabled option.
	virtual Widget *findWidget (double screenPositionX, double screenPositionY, bool requireMouseHoverEnabled = false);

	// Return the child widget that holds a matching widgetName value, or NULL if no such widget was found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	virtual Widget *findWidget (const StdString &widgetName, bool shouldRetain = false);

	// Return the child widget matching the provided function, or NULL if no such widget was found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	virtual Widget *findWidget (Widget::FindMatchFunction fn, void *fnData, bool shouldRetain = false);

	// Find all child widgets with a non-empty widgetName value and append the names to destList
	virtual void getWidgetNames (StringList *destList);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	struct MouseState {
		int positionDeltaX;
		int positionDeltaY;
		int wheelUp;
		int wheelDown;
		bool isEntered;
		double enterDeltaX;
		double enterDeltaY;
		bool isLeftClicked;
		bool isLeftClickReleased;
		bool isLeftClickEntered;
		bool isLongPressed;
		MouseState ():
			positionDeltaX (0),
			positionDeltaY (0),
			wheelUp (0),
			wheelDown (0),
			isEntered (false),
			enterDeltaX (0.0f),
			enterDeltaY (0.0f),
			isLeftClicked (false),
			isLeftClickReleased (false),
			isLeftClickEntered (false),
			isLongPressed (false) { }
	};
	// Update the widget as appropriate for the specified mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	bool processMouseState (const Widget::MouseState &mouseState);

	// Set the widget to display the specified tooltip text on mouse hover
	void setMouseHoverTooltip (const StdString &text, Widget::Alignment alignment = Widget::BottomAlignment);

	// Set the widget to destroy itself after the specified number of milliseconds passes
	void setDestroyDelay (int delayMs);

	// Set the widget's key focus mode, indicating whether it should handle keypress events with edit focus
	virtual void setKeyFocus (bool enable);

	// Invoke the widget's mouse enter callback
	void mouseEnter ();

	// Invoke the widget's mouse exit callback
	void mouseExit ();

	// Invoke the widget's mouse press callback
	void mousePress ();

	// Invoke the widget's mouse release callback
	void mouseRelease ();

	// Invoke the widget's mouse click callback
	void mouseClick ();

	// Invoke any function contained in callback and return a boolean value indicating if a function executed
	bool eventCallback (const Widget::EventCallbackContext &callback);

	// Assign the widget's position as specified in the provided flow state, then update flow state as appropriate for a rightward flow
	virtual void flowRight (PanelLayoutFlow *flow);

	// Assign the widget's position as specified in the provided flow state, then update flow state as appropriate for a downward flow
	virtual void flowDown (PanelLayoutFlow *flow);

	// Assign the widget's position as specified in the provided flow state, then update flow state as appropriate for a leftward flow
	virtual void flowLeft (PanelLayoutFlow *flow);

	// Assign the widget's position as specified in the provided flow state, then update flow state as appropriate for an upward flow
	virtual void flowUp (PanelLayoutFlow *flow);

	// Assign the widget's x position to a centered value within horizontal extents of the provided flow
	virtual void centerHorizontal (PanelLayoutFlow *flow);

	// Assign the widget's y position to a centered value within vertical extents of the provided flow
	virtual void centerVertical (PanelLayoutFlow *flow);

	// Return a Rectangle struct containing the widget's screen extent values
	Widget::Rectangle getScreenRect ();

	// Set the value of the widget's compose draw option. If enabled, the widget draws from a writable texture and provides transform operations.
	void setComposeDraw (bool enable);

	// Set the scale factor for use in drawing the compose texture
	void setComposeScale (double scale);

	// Set the degree rotation for use in drawing the compose texture
	void setComposeRotation (double rotation);

	// Start an operation to translate the compose scale over the specified duration
	void animateScale (double startScale, double targetScale, int duration, bool endCompose = false);

	// Execute a scale bump animation
	void animateScaleBump ();

	// Callback functions
	static bool compareZLevel (Widget *first, Widget *second);

protected:
	// Execute subclass-specific operations to update object state as appropriate for an elapsed millisecond time period
	virtual void doUpdate (int msElapsed);

	// Add subclass-specific draw commands for execution by the App
	virtual void doDraw (double originX, double originY);

	// Execute subclass-specific operations to update widget state as appropriate after a size change to the application window or loaded fonts
	virtual void doResize ();

	// Execute operations appropriate when the widget's input state is reset
	virtual void doResetInputState ();

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Update the widget as appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Update the widget as appropriate when compose draw has become enabled or disabled
	virtual void doProcessComposeDrawChange ();

	// Return a string that should be included as part of the toString method's output
	virtual StdString toStringDetail ();

	int destroyClock;

private:
	// Render widget content to composeTexture
	static void renderComposeTexture (void *itPtr);
	void executeRenderComposeTexture ();

	SDL_mutex *refcountMutex;
	SDL_Texture *composeTexture;
	StdString composeTexturePath;
	int composeTextureWidth;
	int composeTextureHeight;
	double composeTargetWidth;
	double composeTargetHeight;
	int composeAnimationType;
	Position composeAnimationScale;
};
#endif
