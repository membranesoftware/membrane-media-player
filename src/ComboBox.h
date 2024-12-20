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
// Widget that holds a text value selected from a list of options
#ifndef COMBO_BOX_H
#define COMBO_BOX_H

#include "Color.h"
#include "WidgetHandle.h"
#include "Panel.h"

class HashMap;
class StringList;
class LabelWindow;
class ScrollView;
class ScrollBar;

class ComboBox : public Panel {
public:
	ComboBox ();
	~ComboBox ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ComboBox *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext valueChangeCallback;

	// Read-only data members
	StdString selectedItemValue;
	StdString selectedItemData;
	bool isDisabled;
	bool isInverseColor;
	bool hasItemData;

	// Set the combo box's disabled state, appropriate for use when the combo box becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the combo box's inverse color state. If enabled, the combo box renders using an inverse color scheme.
	void setInverseColor (bool inverse);

	// Add an item to the combo box. If itemData is provided, the combo box provides it in place of itemValue when reporting its value.
	void addItem (const StdString &itemValue);
	void addItem (const StdString &itemValue, const StdString &itemData);

	// Add a set of items to the combo box. If a HashMap is provided, it is treated as mapping itemName to itemData strings.
	void addItems (const StringList &nameList);
	void addItems (HashMap *itemMap);

	// Set the combo box's value to the item matching the specified value string and invoke any configured change callback unless shouldSkipChangeCallback is true
	void setValue (const StdString &value, bool shouldSkipChangeCallback = false);

	// Set the combo box's value to the item matching the specified itemData string and invoke any configured change callback unless shouldSkipChangeCallback is true
	void setValueByItemData (const StdString &itemData, bool shouldSkipChangeCallback = false);

	// Return true if the combo box contains an item matching value
	bool containsItem (const StdString &value) const;

	// Return the combo box's current value, or the corresponding data string if non-empty
	StdString getValue ();

	// Expand the combo box, causing its parent UI to populate an item panel
	void expand ();

	// Clear a previously enabled expand state
	void unexpand ();

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	void doResize ();

private:
	struct Item {
		StdString value;
		StdString itemData;
		LabelWindow *label;
		Item (): label (NULL) { }
	};

	// Callback functions
	static void expandItemClicked (void *itPtr, Widget *widgetPtr);
	static void scrollBarPositionChanged (void *itPtr, Widget *widgetPtr);
	static void scrollBarUpdated (void *itPtr, int msElapsed, Widget *widgetPtr);

	// Set the combo box's value to the item matching the provided LabelWindow's text and invoke any configured change callback unless shouldSkipChangeCallback is true
	void setValueByLabelWindow (LabelWindow *choiceLabel, bool shouldSkipChangeCallback = false);

	// Set the combo box's focus state
	void setFocused (bool focused);

	std::list<ComboBox::Item> itemList;
	WidgetHandle<ScrollView> expandViewHandle;
	ScrollView *expandView;
	WidgetHandle<ScrollBar> expandScrollBarHandle;
	ScrollBar *expandScrollBar;
	bool isExpanded;
	double expandScreenX, expandScreenY;
	bool isFocused;
	LabelWindow *selectedItemLabel;
	double maxTextWidth;
	Color normalBgColor;
	Color normalBorderColor;
	Color focusBgColor;
	Color focusBorderColor;
	Color disabledBgColor;
	Color disabledBorderColor;
	Color normalItemTextColor;
	Color focusItemTextColor;
	Color disabledTextColor;
};
#endif
