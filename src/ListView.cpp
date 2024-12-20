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
#include "Config.h"
#include "App.h"
#include "ClassId.h"
#include "SpriteId.h"
#include "StringList.h"
#include "Ui.h"
#include "UiText.h"
#include "Color.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Button.h"
#include "Panel.h"
#include "ScrollView.h"
#include "ScrollViewWindow.h"
#include "ListView.h"

ListView::ListView (double viewWidth, int minViewItems, int maxViewItems, UiConfiguration::FontType itemFontType)
: ScrollViewWindow ()
, viewWidth (viewWidth)
, isDisabled (false)
, clickItemIndex (-1)
, focusItemIndex (-1)
, minViewItems (minViewItems)
, maxViewItems (maxViewItems)
, itemFontType (itemFontType)
, emptyStateLabelHandle (&emptyStateLabel)
, focusItemHandle (&focusItem)
{
	classId = ClassId::ListView;
	if (minViewItems < 1) {
		minViewItems = 1;
	}
	if (maxViewItems < minViewItems) {
		maxViewItems = minViewItems;
	}

	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	setBorder (true, UiConfiguration::instance->darkBackgroundColor);
	setScrollOptions (ScrollViewWindow::MouseWheelScrollOption);

	deleteButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton)), 2);
	deleteButton->mouseClickCallback = Widget::EventCallbackContext (ListView::deleteButtonClicked, this);
	deleteButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	deleteButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Remove).capitalized ());
	deleteButton->isVisible = false;

	setFixedPadding (true, 0.0f, 0.0f);
	setViewPaddingScale (0.0f, 1.0f);
	setViewLayout (Panel::DownFlowLayoutOption | Panel::LeftGravityLayoutOption, 0.0f);
	reflow ();
}
ListView::~ListView () {
	clearItems (true);
}

void ListView::setViewWidth (double viewWidthValue) {
	std::vector<ListViewItemLabel *>::iterator i1, i2;

	if (FLOAT_EQUALS (viewWidth, viewWidthValue)) {
		return;
	}
	viewWidth = viewWidthValue;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		(*i1)->windowWidth = viewWidth;
		++i1;
	}
	reflow ();
}

void ListView::setDisabled (bool disabled) {
	std::vector<ListViewItemLabel *>::iterator i1, i2;

	if (isDisabled == disabled) {
		return;
	}
	isDisabled = disabled;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		(*i1)->setDisabled (isDisabled);
		++i1;
	}
	reflow ();
}

void ListView::setEmptyStateText (const StdString &text, UiConfiguration::FontType textFontType, const Color &textColor) {
	emptyStateLabelHandle.destroyAndAssign (new Label (text, textFontType, textColor));
	add (emptyStateLabel, 1);
	emptyStateLabel->isVisible = false;
	reflow ();
}

void ListView::setItemDeleteCallback (Widget::EventCallbackContext callback) {
	itemDeleteCallback = callback;
}

void ListView::clearItems (bool shouldSkipChangeCallback) {
	std::vector<ListViewItemLabel *>::iterator i1, i2;

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		(*i1)->isDestroyed = true;
		++i1;
	}
	itemList.clear ();
	focusItemHandle.clear ();
	focusItemIndex = -1;
	reflow ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::setItems (const StringList &itemList, bool shouldSkipChangeCallback) {
	StringList::const_iterator i1, i2;

	clearItems (true);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		addItem (*i1, true);
		++i1;
	}
	reflow ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::getItems (StringList *destList) {
	std::vector<ListViewItemLabel *>::const_iterator i1, i2;

	destList->clear ();
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		destList->push_back ((*i1)->itemText);
		++i1;
	}
}

int ListView::getItemCount () {
	return ((int) itemList.size ());
}

bool ListView::contains (const StdString &itemText) {
	std::vector<ListViewItemLabel *>::const_iterator i1, i2;

	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if ((*i1)->itemText.equals (itemText)) {
			return (true);
		}
		++i1;
	}
	return (false);
}

StdString ListView::getItemText (int itemIndex) {
	if ((itemIndex < 0) || (itemIndex >= (int) itemList.size ())) {
		return (StdString ());
	}
	return (itemList.at (itemIndex)->itemText);
}

Widget::Rectangle ListView::getDeleteButtonScreenRect () {
	return (deleteButton->getScreenRect ());
}

void ListView::addItem (const StdString &itemText, bool shouldSkipChangeCallback) {
	ListViewItemLabel *item;

	item = new ListViewItemLabel (viewWidth, itemText, itemFontType);
	item->mouseClickCallback = Widget::EventCallbackContext (ListView::itemClicked, this);
	item->listPosition = (int) itemList.size ();
	if (isDisabled) {
		item->setDisabled (true);
	}
	addViewItem (item);

	itemList.push_back (item);
	reflow ();
	scrollToBottom ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::removeItem (int itemIndex, bool shouldSkipChangeCallback) {
	std::vector<ListViewItemLabel *>::iterator i1, i2;
	int index;

	if ((itemIndex < 0) || (itemIndex >= (int) itemList.size ())) {
		return;
	}
	i1 = itemList.begin () + itemIndex;
	(*i1)->isDestroyed = true;
	itemList.erase (i1);
	if (itemIndex == focusItemIndex) {
		focusItemHandle.clear ();
		focusItemIndex = -1;
	}

	index = 0;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		(*i1)->listPosition = index;
		++index;
		++i1;
	}

	reflow ();
	if (! shouldSkipChangeCallback) {
		eventCallback (listChangeCallback);
	}
}

void ListView::itemClicked (void *itPtr, Widget *widgetPtr) {
	ListView *it = (ListView *) itPtr;
	ListViewItemLabel *item = (ListViewItemLabel *) widgetPtr;

	it->clickItemIndex = item->listPosition;
	it->eventCallback (it->itemClickCallback);
}

void ListView::reflow () {
	std::vector<ListViewItemLabel *>::iterator i1, i2;
	double x, itemh, h;
	int sz;

	resetPadding ();
	x = widthPadding;
	if (! itemList.empty ()) {
		if (emptyStateLabel) {
			emptyStateLabel->isVisible = false;
		}
		i1 = itemList.begin ();
		i2 = itemList.end ();
		while (i1 != i2) {
			(*i1)->reflow ();
			++i1;
		}
	}

	itemh = UiConfiguration::instance->fonts[itemFontType]->maxLineHeight + UiConfiguration::instance->paddingSize;
	sz = (int) itemList.size ();
	if (sz < minViewItems) {
		sz = minViewItems;
	}
	if (sz > maxViewItems) {
		sz = maxViewItems;
	}
	h = (itemh * sz) + (windowScrollView->heightPadding * 2.0f);
	setViewSize (viewWidth, h);
	ScrollViewWindow::reflow ();

	if (emptyStateLabel && itemList.empty ()) {
		emptyStateLabel->position.assign ((width / 2.0f) - (emptyStateLabel->width / 2.0f), (h / 2.0f) - (emptyStateLabel->height / 2.0f));
		emptyStateLabel->isVisible = true;
	}

	if (focusItem) {
		x = windowScrollView->extentX2 - (UiConfiguration::instance->paddingSize * 2.0f) - deleteButton->maxImageWidth;
		deleteButton->position.assign (x, focusItem->position.y - windowScrollView->viewOriginY + (focusItem->height / 2.0f) - (deleteButton->height / 2.0f));
		deleteButton->isVisible = true;
	}
	else {
		deleteButton->isVisible = false;
	}
}

void ListView::deleteButtonClicked (void *itPtr, Widget *widgetPtr) {
	ListView *it = (ListView *) itPtr;

	if (it->focusItemIndex < 0) {
		return;
	}
	if (it->eventCallback (it->itemDeleteCallback)) {
		return;
	}
	it->removeItem (it->focusItemIndex, true);
	it->focusItemHandle.clear ();
	it->focusItemIndex = -1;
	it->reflow ();
	it->eventCallback (it->listChangeCallback);
}

bool ListView::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed, shouldreflow, found;
	std::vector<ListViewItemLabel *>::iterator i1, i2;
	ListViewItemLabel *item;

	consumed = ScrollViewWindow::doProcessMouseState (mouseState);
	shouldreflow = false;
	if (isDisabled || (! mouseState.isEntered)) {
		if (focusItem) {
			focusItem->setHighlighted (false);
			focusItemHandle.clear ();
			focusItemIndex = -1;
			shouldreflow = true;
		}
	}
	else {
		found = false;
		i1 = itemList.begin ();
		i2 = itemList.end ();
		while (i1 != i2) {
			item = *i1;
			if ((mouseState.enterDeltaY >= (item->position.y - windowScrollView->viewOriginY)) && (mouseState.enterDeltaY < (item->position.y - windowScrollView->viewOriginY + item->height))) {
				found = true;
				if (item != focusItem) {
					shouldreflow = true;
					if (focusItem) {
						focusItem->setHighlighted (false);
					}
					focusItemHandle.assign (item);
					focusItem->setHighlighted (true);
					focusItemIndex = focusItem->listPosition;
				}
				break;
			}
			++i1;
		}
		if ((! found) && focusItem) {
			focusItem->setHighlighted (false);
			focusItemHandle.clear ();
			focusItemIndex = -1;
			shouldreflow = true;
		}
	}
	if (shouldreflow) {
		reflow ();
	}
	return (consumed);
}

void ListView::doUpdate (int msElapsed) {
	focusItemHandle.compact ();
	ScrollViewWindow::doUpdate (msElapsed);
}

ListViewItemLabel::ListViewItemLabel (double windowWidth, const StdString &text, UiConfiguration::FontType textFontType)
: Panel ()
, windowWidth (windowWidth)
, listPosition (-1)
, itemText (text)
{
	label = add (new LabelWindow (new Label (itemText, textFontType, UiConfiguration::instance->primaryTextColor)));
	label->setPaddingScale (1.0f, 0.5f);
	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
ListViewItemLabel::~ListViewItemLabel () {
}

void ListViewItemLabel::reflow () {
	label->reflow ();
	label->position.assign (0.0f, 0.0f);
	setFixedSize (true, windowWidth, label->height);
}

void ListViewItemLabel::setHighlighted (bool highlighted) {
	if (highlighted) {
		setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	}
	else {
		setFillBg (false);
	}
}

void ListViewItemLabel::setDisabled (bool disabled) {
	if (disabled) {
		label->setTextColor (UiConfiguration::instance->lightPrimaryTextColor);
	}
	else {
		label->setTextColor (UiConfiguration::instance->primaryTextColor);
	}
}
