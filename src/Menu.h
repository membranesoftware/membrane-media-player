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
// Widget that shows a menu of action items
#ifndef MENU_H
#define MENU_H

#include "Panel.h"

class Sprite;
class Label;
class Image;

class Menu : public Panel {
public:
	Menu ();
	~Menu ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Menu *castWidget (Widget *widget);

	// Read-only data members
	StdString selectedItemName;

	// Read-write data members
	bool isClickDestroyEnabled;

	// Add a menu item containing the provided name and an optional sprite icon. If a selection group number of zero or greater is specified, selecting the item causes the menu to show a check mark next to it while clearing any check marks from other items in the same group.
	void addItem (const StdString &name, Sprite *sprite = NULL, const Widget::EventCallbackContext &callback = Widget::EventCallbackContext (), int selectionGroup = -1, bool isSelected = false);

	// Add a divider line to the menu
	void addDivider ();

	// Return true if the menu contains an item matching itemName
	bool contains (const StdString &itemName) const;

	// Execute the select action for the named item
	void selectItem (const StdString &itemName);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	bool doProcessMouseState (const Widget::MouseState &mouseState);

private:
	// Clear the isSelected state for each item in the specified group
	void unselectItemGroup (int selectionGroup);

	struct Item {
		bool isChoice;
		bool isDivider;
		bool isSelected;
		int selectionGroup;
		Panel *panel;
		Label *label;
		Image *image;
		Widget::EventCallbackContext callback;
		Item ():
			isChoice (false),
			isDivider (false),
			isSelected (false),
			selectionGroup (-1),
			panel (NULL),
			label (NULL),
			image (NULL) { }
	};
	std::list<Menu::Item> itemList;

	// A map of selection group numbers to checkmark images
	std::map<int, Image *> checkmarkImageMap;

	double selectionMarginSize;
	Panel *focusBackgroundPanel;
	double itemHeight;
	bool isItemFocused;
	Panel *lastFocusPanel;
};
#endif
