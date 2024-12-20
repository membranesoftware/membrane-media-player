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
// Panel that holds a TextField widget and control buttons
#ifndef TEXT_FIELD_WINDOW_H
#define TEXT_FIELD_WINDOW_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class Sprite;
class ImageWindow;
class Toggle;
class Button;
class TextField;

class TextFieldWindow : public Panel {
public:
	TextFieldWindow (double windowWidth, const StdString &promptText = StdString (), Sprite *iconSprite = NULL);
	~TextFieldWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static TextFieldWindow *castWidget (Widget *widget);

	static constexpr const int randomizeStringLength = 16;

	// Read-write data members
	Widget::EventCallbackContext valueChangeCallback;
	Widget::EventCallbackContext valueEditCallback;
	Widget::EventCallbackContext enterButtonClickCallback;
	bool shouldSkipTextClearCallbacks;

	// Read-only data members
	double windowWidth;
	bool isDisabled;
	bool isInverseColor;
	bool isObscured;
	int buttonOptions;

	// Set the window's inverse color option
	void setInverseColor (bool inverse);

	// Set text to show when the field value is empty
	void setPromptText (const StdString &text);

	// Set the text field's prompt error color state. If enabled, the text field shows its prompt text in the UiConfiguration error color.
	void setPromptErrorColor (bool enable);

	// Set the text field's disabled state, appropriate for use when the field becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the text field's option to retain key focus after a return keypress
	void setRetainFocusOnReturnKey (bool enable);

	// Set the window's text field obscure option
	void setObscured (bool enable);

	// Set the window's width
	void setWindowWidth (double windowWidthValue);

	static constexpr const int EnterButtonOption = 0x1;
	static constexpr const int CancelButtonOption = 0x2;
	static constexpr const int PasteButtonOption = 0x4;
	static constexpr const int ClearButtonOption = 0x8;
	static constexpr const int RandomizeButtonOption = 0x10;
	static constexpr const int FsBrowseButtonOption = 0x20;
	static constexpr const int FsBrowseButtonSortDirectoriesFirstOption = 0x40;
	static constexpr const int FsBrowseButtonSelectDirectoriesOption = 0x80;
	// Set the enable state for the window's utility buttons
	void setButtonsEnabled (int buttonOptionsValue);

	// Return the text field's value
	StdString getValue ();

	// Set the text field's value and invoke any configured change and edit callbacks unless shouldSkipChangeCallback or shouldSkipEditCallback are true
	void setValue (const StdString &valueText, bool shouldSkipChangeCallback = false, bool shouldSkipEditCallback = false);

	// Assign keypress edit focus to the text field
	void assignKeyFocus ();

	// Superclass override methods
	void reflow ();

private:
	// Callback functions
	static void textFieldValueChanged (void *itPtr, Widget *widgetPtr);
	static void textFieldValueEdited (void *itPtr, Widget *widgetPtr);
	static void enterButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *itPtr, Widget *widgetPtr);
	static void pasteButtonClicked (void *itPtr, Widget *widgetPtr);
	static void clearButtonClicked (void *itPtr, Widget *widgetPtr);
	static void randomizeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void fsBrowseButtonClicked (void *itPtr, Widget *widgetPtr);
	static void fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr);
	static void visibilityToggleStateChanged (void *itPtr, Widget *widgetPtr);

	TextField *textField;
	Button *enterButton;
	Button *cancelButton;
	Button *pasteButton;
	Button *clearButton;
	Button *randomizeButton;
	Button *fsBrowseButton;
	ImageWindow *iconImage;
	Toggle *visibilityToggle;
	WidgetHandle<Panel> fsBrowserPanelHandle;
	Panel *fsBrowserPanel;
	StdString cancelValue;
	bool isCancelled;
};
#endif
