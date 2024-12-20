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
// Panel that shows controls for executing an action with a set of options
#ifndef ACTION_WINDOW_H
#define ACTION_WINDOW_H

#include "Widget.h"
#include "UiConfiguration.h"
#include "Panel.h"

class StringList;
class Label;
class TextFlow;
class Button;
class ComboBox;
class TextFieldWindow;
class Toggle;
class SliderWindow;

class ActionWindow : public Panel {
public:
	ActionWindow ();
	~ActionWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ActionWindow *castWidget (Widget *widget);

	static const UiConfiguration::FontType titleTextFont;

	// Read-write data members
	Widget::EventCallbackContext optionChangeCallback;
	Widget::EventCallbackContext closeCallback;

	// Read-only data members
	bool isOptionDataValid;
	bool isConfirmed;
	bool isInverseColor;

	// Set the visible state for the window's cancel and confirm buttons (visible by default)
	void setButtonsVisible (bool visible);

	// Set the window's inverse color option
	void setInverseColor (bool inverse);

	// Set the window's title text (empty by default)
	void setTitleText (const StdString &text);

	// Set the window's description text (empty by default)
	void setDescriptionText (const StdString &text);

	// Set the tooltip text that should be shown on the window's confirm button (defaults to "Confirm")
	void setConfirmTooltipText (const StdString &text);

	// Add the provided panel to the window as a footer element
	void setFooterPanel (Panel *panel);

	// Add the provided widget to the window as an option item
	void addOption (const StdString &optionName, ComboBox *comboBox, const StdString &descriptionText = StdString ());
	void addOption (const StdString &optionName, TextFieldWindow *textFieldWindow, const StdString &descriptionText = StdString ());
	void addOption (const StdString &optionName, Toggle *toggle, const StdString &descriptionText = StdString ());
	void addOption (const StdString &optionName, SliderWindow *slider, const StdString &descriptionText = StdString ());

	// Set name label text for the named option
	void setOptionNameText (const StdString &optionName, const StdString &nameText);

	// Set description text for the named option
	void setOptionDescriptionText (const StdString &optionName, const StdString &descriptionText);

	// Set the named option to evaluate as invalid if its value is an empty string
	void setOptionNotEmptyString (const StdString &optionName);

	// Set the disabled state for the named option
	void setOptionDisabled (const StdString &optionName, bool disable);

	// Set the value of the named option
	void setOptionValue (const StdString &optionName, const char *optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, const StdString &optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, int optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, double optionValue, bool shouldSkipChangeCallback = false);
	void setOptionValue (const StdString &optionName, bool optionValue, bool shouldSkipChangeCallback = false);

	// Return the string value of the named option, or the specified default value if no such option was found
	StdString getStringValue (const StdString &optionName, const StdString &defaultValue);
	StdString getStringValue (const StdString &optionName, const char *defaultValue);

	// Return the number value of the named option, or the specified default value if no such option was found
	int getNumberValue (const StdString &optionName, int defaultValue);
	double getNumberValue (const StdString &optionName, double defaultValue);

	// Return the boolean value of the named option, or the specified default value if no such option was found
	bool getBooleanValue (const StdString &optionName, bool defaultValue);

	// Clear destList and add the window's option names
	void getOptionNames (StringList *destList);

	// Set widgetName values for contained controls, each composed from widgetNamePrefix and the control's name
	void setWidgetNames (const StdString &widgetNamePrefix);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doResize ();

private:
	// Callback functions
	static void confirmButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *itPtr, Widget *widgetPtr);
	static void optionValueChanged (void *itPtr, Widget *widgetPtr);

	struct Item {
		StdString name;
		int type;
		Label *nameLabel;
		TextFlow *descriptionText;
		Widget *optionWidget;
		bool isNotEmptyString;
		bool isDisabled;
		Item ():
			type (0),
			nameLabel (NULL),
			descriptionText (NULL),
			optionWidget (NULL),
			isNotEmptyString (false),
			isDisabled (false) { }
	};

	// Return an iterator positioned at the specified item in itemList, or the end of itemList if the item wasn't found
	std::list<ActionWindow::Item>::iterator findItem (const StdString &optionName, bool createNewItem = false);

	// Utility method that executes item add operations as needed by addOption variants
	void doAddOption (int itemType, const StdString &optionName, Widget *optionWidget, const StdString &descriptionText);

	// Check the validity of all option values and reset isOptionDataValid
	void verifyOptions ();

	std::list<ActionWindow::Item> itemList;
	double appWindowWidth;
	double appWindowHeight;
	Label *titleLabel;
	TextFlow *headerDescriptionText;
	Panel *footerPanel;
	Button *confirmButton;
	StdString confirmButtonTooltipText;
	Button *cancelButton;
};
#endif
