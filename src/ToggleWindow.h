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
// Panel that holds a Toggle widget and a label or image
#ifndef TOGGLE_WINDOW_H
#define TOGGLE_WINDOW_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class Sprite;
class Label;
class Image;
class Toggle;

class ToggleWindow : public Panel {
public:
	ToggleWindow (Toggle *toggle);
	ToggleWindow (Toggle *toggle, const StdString &text);
	ToggleWindow (Toggle *toggle, Sprite *iconSprite);
	~ToggleWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ToggleWindow *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext stateChangeCallback;

	// Read-only data members
	bool isChecked;
	bool isRightAligned;
	bool isInverseColor;

	// Set the toggle to show a text label
	void setText (const StdString &text);

	// Set the toggle to show an icon image
	void setIcon (Sprite *iconSprite);

	// Set the window's right-aligned option. If enabled, the window places the toggle on the right side instead of the left side.
	void setRightAligned (bool enable);

	// Set the window's inverse color state. If enabled, the window uses an inverse color scheme.
	void setInverseColor (bool inverse);

	// Set the draw color for the toggle's button images
	void setImageColor (const Color &imageColor);

	// Set the toggle's checked state and invoke any configured change callback unless shouldSkipChangeCallback is true
	void setChecked (bool checked, bool shouldSkipChangeCallback = false);

	// Set the toggle's disabled state, appropriate for use when the toggle becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Superclass override methods
	void reflow ();

private:
	// Callback functions
	static void mouseEntered (void *itPtr, Widget *widgetPtr);
	static void mouseExited (void *itPtr, Widget *widgetPtr);
	static void mousePressed (void *itPtr, Widget *widgetPtr);
	static void mouseReleased (void *itPtr, Widget *widgetPtr);
	static void mouseClicked (void *itPtr, Widget *widgetPtr);
	static void toggleStateChanged (void *itPtr, Widget *widgetPtr);

	Toggle *toggle;
	WidgetHandle<Label> textLabelHandle;
	Label *textLabel;
	WidgetHandle<Image> iconImageHandle;
	Image *iconImage;
};
#endif
