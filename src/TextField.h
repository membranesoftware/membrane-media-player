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
// Widget that holds an editable text value
#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include "Color.h"
#include "Panel.h"

class Label;

class TextField : public Panel {
public:
	TextField (double fieldWidth, const StdString &promptText = StdString ());
	~TextField ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static TextField *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext valueChangeCallback;
	Widget::EventCallbackContext valueEditCallback;
	bool shouldRetainFocusOnReturnKey;

	// Read-only data members
	double fieldWidth;
	double rightTextPadding;
	bool isDisabled;
	bool isInverseColor;
	bool isPromptErrorColor;
	bool isObscured;
	bool isOvertype;
	int cursorPosition;

	// Set the text field's width
	void setFieldWidth (double widthValue);

	// Set the text field's width by specifying a line length in characters
	void setLineWidth (int lineLength);

	// Set a right padding width that should be applied for the text field's value
	void setRightTextPadding (double rightTextPaddingValue);

	// Set the text field's disabled state, appropriate for use when the field becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the text field's inverse color state. If enabled, the text field renders using an inverse color scheme.
	void setInverseColor (bool inverse);

	// Set text to show when the field value is empty
	void setPromptText (const StdString &text);

	// Set the text field's prompt error color state. If enabled, the text field shows its prompt text in the UiConfiguration error color.
	void setPromptErrorColor (bool enable);

	// Set the text field's obscured state. If enabled, the text field renders using spacer characters to conceal its value.
	void setObscured (bool enable);

	// Set the text field's overtype state. If enabled, text field edits overwrite characters at the cursor position instead of inserting.
	void setOvertype (bool enable);

	// Return the text field's value
	StdString getValue () const;

	// Set the text field's value and invoke any configured change and edit callbacks unless shouldSkipChangeCallback or shouldSkipEditCallback are true
	void setValue (const StdString &valueText, bool shouldSkipChangeCallback = false, bool shouldSkipEditCallback = false);

	// Read a string from the clipboard and append it to the text field's value
	void appendClipboardText ();

	// Set the widget's key focus mode, indicating whether it should handle keypress events with edit focus
	void setKeyFocus (bool enable);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

private:
	// Set the field's text label and update other elements as needed
	void setText (const StdString &valueText);

	// Set the field's focused state
	void setFocused (bool enable);

	// Reset cursorPosition if it falls outside the valid range for the current text field value
	void clipCursorPosition ();

	Label *promptLabel;
	Label *valueLabel;
	Panel *cursorPanel;
	Panel *rightTextPaddingPanel;
	bool isFocused;
	int cursorClock;
	StdString lastValue;
	Color normalBgColor;
	Color normalBorderColor;
	Color focusBgColor;
	Color focusBorderColor;
	Color disabledBgColor;
	Color disabledBorderColor;
	Color editBgColor;
	Color editBorderColor;
	Color normalValueTextColor;
	Color editValueTextColor;
	Color disabledValueTextColor;
	Color promptTextColor;
};
#endif
