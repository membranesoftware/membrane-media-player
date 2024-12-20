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
// Panel that holds a set of item rows
#ifndef LIST_VIEW_H
#define LIST_VIEW_H

#include "UiConfiguration.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "ScrollViewWindow.h"

class StringList;
class Color;
class Label;
class LabelWindow;
class Button;
class Panel;
class ListViewItemLabel;

class ListView : public ScrollViewWindow {
public:
	ListView (double viewWidth, int minViewItems = 4, int maxViewItems = 4, UiConfiguration::FontType itemFontType = UiConfiguration::BodyFont);
	~ListView ();

	// Read-write data members
	Widget::EventCallbackContext listChangeCallback;
	Widget::EventCallbackContext itemClickCallback;

	// Read-only data members
	double viewWidth;
	bool isDisabled;
	int clickItemIndex;
	int focusItemIndex;

	// Set the width of the viewable area
	void setViewWidth (double viewWidthValue);

	// Set the view's disabled state, appropriate for use when it becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the view's empty state text
	void setEmptyStateText (const StdString &text, UiConfiguration::FontType textFontType, const Color &textColor);

	// Set a callback function that should be executed when the item delete button is clicked, overriding default behavior that removes the item
	void setItemDeleteCallback (Widget::EventCallbackContext callback);

	// Remove all items from the view
	void clearItems (bool shouldSkipChangeCallback = false);

	// Set the view's item list
	void setItems (const StringList &itemList, bool shouldSkipChangeCallback = false);

	// Clear the provided StringList object and insert items from the view's list
	void getItems (StringList *destList);

	// Return the number of items in the view's list
	int getItemCount ();

	// Return a boolean value indicating if the specified item exists in the list
	bool contains (const StdString &itemText);

	// Add an item to the view's list
	void addItem (const StdString &itemText, bool shouldSkipChangeCallback = false);

	// Return the text associated with the specified item index, or an empty string if no such item was found
	StdString getItemText (int itemIndex);

	// Return the screen extent rectangle for the list view's delete button
	Widget::Rectangle getDeleteButtonScreenRect ();

	// Remove the item with the specified index
	void removeItem (int itemIndex, bool shouldSkipChangeCallback = false);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void itemClicked (void *itPtr, Widget *widgetPtr);
	static void deleteButtonClicked (void *itPtr, Widget *widgetPtr);

	Widget::EventCallbackContext itemDeleteCallback;
	int minViewItems;
	int maxViewItems;
	UiConfiguration::FontType itemFontType;
	std::vector<ListViewItemLabel *> itemList;
	WidgetHandle<Label> emptyStateLabelHandle;
	Label *emptyStateLabel;
	Button *deleteButton;
	WidgetHandle<ListViewItemLabel> focusItemHandle;
	ListViewItemLabel *focusItem;
};

class ListViewItemLabel : public Panel {
public:
	ListViewItemLabel (double windowWidth, const StdString &text, UiConfiguration::FontType textFontType);
	~ListViewItemLabel ();

	double windowWidth;
	int listPosition;
	StdString itemText;
	LabelWindow *label;

	// Set panel highlighted state
	void setHighlighted (bool highlighted);

	// Set panel disabled state
	void setDisabled (bool disabled);

	// Superclass override methods
	void reflow ();
};
#endif
