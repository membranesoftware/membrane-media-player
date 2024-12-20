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
#include "SdlUtil.h"
#include "ClassId.h"
#include "Input.h"
#include "Ui.h"
#include "Widget.h"
#include "Label.h"
#include "UiConfiguration.h"
#include "ScrollBar.h"
#include "CardLabelWindow.h"
#include "CardView.h"

const int highlightAnimationDuration = 80;

CardView::CardView (double viewWidth, double viewHeight)
: ScrollView ()
, shouldSortItemList (false)
, cardAreaWidth (viewWidth)
, cardAreaBottomPadding (0.0f)
, itemMarginSize (0.0f)
, rowCount (0)
{
	classId = ClassId::CardView;

	SdlUtil::createMutex (&itemMutex);
	itemMarginSize = UiConfiguration::instance->marginSize;
	setRowCount (1);

	scrollBar = add (new ScrollBar (viewHeight - (UiConfiguration::instance->paddingSize * 2.0f)), 6);
	scrollBar->positionChangeCallback = Widget::EventCallbackContext (CardView::scrollBarPositionChanged, this);
	scrollBar->isVisible = false;

	setViewSize (viewWidth, viewHeight);
	cardAreaWidth = width - scrollBar->width - (UiConfiguration::instance->paddingSize * 2.0f) - (UiConfiguration::instance->marginSize * 0.25f);
	cardAreaBottomPadding = UiConfiguration::instance->paddingSize * 2.0f;
}
CardView::~CardView () {
	std::list<CardView::Item>::iterator i1, i2;

	SDL_LockMutex (itemMutex);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		clearItem (&(*i1));
		++i1;
	}
	itemList.clear ();
	SDL_UnlockMutex (itemMutex);

	SdlUtil::destroyMutex (&itemMutex);
}

void CardView::clearItem (CardView::Item *item) {
	if (item->itemPanel) {
		item->itemPanel->isDestroyed = true;
		item->itemPanel->release ();
		item->itemPanel = NULL;
	}
	if (item->itemLabel) {
		item->itemLabel->isDestroyed = true;
		item->itemLabel->release ();
		item->itemLabel = NULL;
	}
}

CardLabelWindow *CardView::createItemLabel (Panel *itemPanel) {
	CardLabelWindow *label;

	label = new CardLabelWindow (itemPanel->width);
	label->retain ();
	addWidget (label, 3);
	return (label);
}

void CardView::setViewSize (double viewWidth, double viewHeight) {
	ScrollView::setViewSize (viewWidth, viewHeight);
	scrollBar->setMaxTrackLength (viewHeight - (UiConfiguration::instance->paddingSize * 2.0f));
	cardAreaWidth = width - scrollBar->width - (UiConfiguration::instance->paddingSize * 2.0f) - (UiConfiguration::instance->marginSize * 0.25f);
	reflow ();
}

void CardView::setBottomPadding (double paddingSize) {
	if (FLOAT_EQUALS (paddingSize, cardAreaBottomPadding)) {
		return;
	}
	cardAreaBottomPadding = paddingSize;
	reflow ();
}

void CardView::setItemMarginSize (double marginSize) {
	if (FLOAT_EQUALS (marginSize, itemMarginSize)) {
		return;
	}
	itemMarginSize = marginSize;
	reflow ();
}

void CardView::setRowCount (int count) {
	int i;

	if (count < 1) {
		count = 1;
	}
	if (rowCount == count) {
		return;
	}
	rowCount = count;
	rowList.clear ();
	for (i = 0; i < rowCount; ++i) {
		rowList.push_back (CardView::Row ());
	}
}

CardView::Row *CardView::getRow (int rowNumber) {
	if ((rowNumber < 0) || (rowNumber >= rowCount)) {
		return (NULL);
	}
	return (&(rowList.at (rowNumber)));
}

void CardView::setRowHeader (int rowNumber, Panel *headerPanel) {
	CardView::Row *row;
	bool visible;

	row = getRow (rowNumber);
	if (! row) {
		return;
	}
	visible = true;
	if (row->headerPanel) {
		visible = row->headerPanel->isVisible;
		row->headerPanel->isDestroyed = true;
	}
	addWidget (headerPanel);
	row->headerPanel = headerPanel;
	row->headerPanel->zLevel = 1;
	row->headerPanel->isVisible = visible;
	reflow ();
}

void CardView::setRowItemMarginSize (int rowNumber, double marginSize) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if ((! row) || FLOAT_EQUALS (marginSize, row->itemMarginSize)) {
		return;
	}
	row->itemMarginSize = marginSize;
	reflow ();
}

void CardView::setRowReverseSorted (int rowNumber, bool enable) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if ((! row) || (enable == row->isReverseSorted)) {
		return;
	}
	row->isReverseSorted = enable;
	reflow ();
}

void CardView::setRowSelectionAnimated (int rowNumber, bool enable) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if ((! row) || (enable == row->isSelectionAnimated)) {
		return;
	}
	row->isSelectionAnimated = enable;
	reflow ();
}

void CardView::setRowRepositionAnimated (int rowNumber, bool enable) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if ((! row) || (enable == row->isRepositionAnimated)) {
		return;
	}
	row->isRepositionAnimated = enable;
	reflow ();
}

void CardView::setRowLabeled (int rowNumber, bool enable, CardView::ItemLabelFunction labelFn, void *labelFnData) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if (! row) {
		return;
	}
	row->isLabeled = enable;
	if (! row->isLabeled) {
		row->itemLabelFn = NULL;
		row->itemLabelFnData = NULL;
	}
	else {
		row->itemLabelFn = labelFn;
		row->itemLabelFnData = labelFnData;
	}
}

bool CardView::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::list<CardView::Item>::iterator j1, j2, item;
	Panel *itempanel;
	double x1, y1, x2, y2;
	bool consumed, highlight, shouldreflow;
	int mousex, mousey, i, duration;

	consumed = ScrollView::doProcessMouseState (mouseState);
	shouldreflow = false;
	mousex = Input::instance->mouseX;
	mousey = Input::instance->mouseY;
	if (! highlightedItemId.empty ()) {
		SDL_LockMutex (itemMutex);
		item = findItemPosition (highlightedItemId);
		if (item == itemList.end ()) {
			highlightedItemId.assign ("");
		}
		else {
			itempanel = item->itemPanel;
			highlight = true;
			if (! mouseState.isEntered) {
				highlight = false;
			}
			else {
				x1 = itempanel->screenX;
				y1 = itempanel->screenY;
				if (itempanel->isComposeDrawEnabled) {
					x2 = x1 + itempanel->composeWidth;
					y2 = y1 + itempanel->composeHeight;
				}
				else {
					x2 = x1 + itempanel->width;
					y2 = y1 + itempanel->height;
				}
				if (!((mousex >= (int) x1) && (mousey >= (int) y1) && (mousex <= (int) x2) && (mousey <= (int) y2))) {
					highlight = false;
				}
			}
			if (! highlight) {
				item->isHighlighted = false;
				itempanel->zLevel = -1;
				if (item->itemLabel) {
					item->itemLabel->zLevel = 3;
				}
				highlightedItemId.assign ("");
			}
		}
		SDL_UnlockMutex (itemMutex);
	}
	if (highlightedItemId.empty ()) {
		for (i = 0; i < rowCount; ++i) {
			if (rowList.at (i).isSelectionAnimated) {
				SDL_LockMutex (itemMutex);
				j1 = itemList.begin ();
				j2 = itemList.end ();
				while (j1 != j2) {
					itempanel = j1->itemPanel;
					if ((j1->row == i) && j1->isAnimatingSelection && itempanel && itempanel->isVisible && (! itempanel->isInputSuspended)) {
						highlight = false;
						if (mouseState.isEntered && highlightedItemId.empty ()) {
							x1 = itempanel->screenX;
							y1 = itempanel->screenY;
							if (itempanel->isComposeDrawEnabled) {
								x2 = x1 + itempanel->composeWidth;
								y2 = y1 + itempanel->composeHeight;
							}
							else {
								x2 = x1 + itempanel->width;
								y2 = y1 + itempanel->height;
							}
							if ((mousex >= (int) x1) && (mousey >= (int) y1) && (mousex <= (int) x2) && (mousey <= (int) y2)) {
								highlight = true;
							}
						}

						if (highlight) {
							j1->isHighlighted = true;
							itempanel->zLevel = 7;
							if (j1->itemLabel) {
								j1->itemLabel->zLevel = 7;
							}
							duration = highlightAnimationDuration;
							if (itempanel->composeScale > CardView::reducedSizeItemScale) {
								duration = (int) (((1.0f - itempanel->composeScale) / (1.0f - CardView::reducedSizeItemScale)) * (double) highlightAnimationDuration);
								if (duration < 1) {
									duration = 1;
								}
							}
							itempanel->animateScale (itempanel->composeScale, 1.0f, duration, true);
							highlightedItemId.assign (j1->id);
							shouldreflow = true;
							break;
						}
						else {
							if (j1->isHighlighted) {
								j1->isHighlighted = false;
								itempanel->zLevel = -1;
								if (j1->itemLabel) {
									j1->itemLabel->zLevel = 3;
								}
							}
						}
					}
					++j1;
				}
				SDL_UnlockMutex (itemMutex);
			}
			if (! highlightedItemId.empty ()) {
				break;
			}
		}
	}

	if (shouldreflow) {
		reflow ();
	}
	return (consumed);
}

void CardView::doUpdate (int msElapsed) {
	std::vector<CardView::Row>::iterator i1, i2;
	std::list<CardView::Item>::iterator j1, j2;
	Panel *itempanel;
	int rownum, duration;
	bool shouldreflow;

	shouldreflow = false;
	rownum = 0;
	i1 = rowList.begin ();
	i2 = rowList.end ();
	while (i1 != i2) {
		if (i1->isSelectionAnimated) {
			SDL_LockMutex (itemMutex);
			j1 = itemList.begin ();
			j2 = itemList.end ();
			while (j1 != j2) {
				if (j1->row == rownum) {
					j1->isAnimatingSelection = true;
					itempanel = j1->itemPanel;
					if (itempanel && (! j1->isHighlighted)) {
						itempanel->composeAnimationCompleteCallback = Widget::EventCallbackContext (CardView::composeAnimationComplete, this);
						if ((! itempanel->isComposeDrawEnabled) || ((! itempanel->isComposeAnimating) && (itempanel->composeScale > CardView::reducedSizeItemScale))) {
							duration = highlightAnimationDuration;
							if (itempanel->isComposeDrawEnabled && (itempanel->composeScale < 1.0f)) {
								duration = (int) (((itempanel->composeScale - CardView::reducedSizeItemScale) / (1.0f - CardView::reducedSizeItemScale)) * (double) highlightAnimationDuration);
								if (duration < 1) {
									duration = 1;
								}
							}
							itempanel->animateScale (itempanel->composeScale, CardView::reducedSizeItemScale, duration);
							shouldreflow = true;
						}
					}
				}
				++j1;
			}
			SDL_UnlockMutex (itemMutex);
		}
		++rownum;
		++i1;
	}
	if (shouldreflow) {
		reflow ();
	}

	ScrollView::doUpdate (msElapsed);
	if (scrollBar->isVisible) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY + UiConfiguration::instance->paddingSize);
	}
}

bool CardView::empty () {
	bool result;

	SDL_LockMutex (itemMutex);
	result = itemList.empty ();
	SDL_UnlockMutex (itemMutex);
	return (result);
}

bool CardView::contains (const StdString &itemId) {
	bool result;

	SDL_LockMutex (itemMutex);
	result = itemIdMap.exists (itemId);
	SDL_UnlockMutex (itemMutex);
	return (result);
}
bool CardView::contains (const char *itemId) {
	return (contains (StdString (itemId)));
}

StdString CardView::getAvailableItemId () {
	StdString id;

	while (true) {
		id.sprintf ("CardView_item_%016llx", (long long int) App::instance->getUniqueId ());
		if (! contains (id)) {
			break;
		}
	}
	return (id);
}

Widget *CardView::addItem (Panel *itemPanel, const StdString &itemId, int rowNumber, bool shouldSkipReflow) {
	CardView::Item item;
	CardView::Row *row;
	StdString id;

	if (itemId.empty ()) {
		id.assign (getAvailableItemId ());
	}
	else {
		id.assign (itemId);
	}
	if (rowNumber < 0) {
		rowNumber = 0;
	}
	if (rowNumber > (rowCount - 1)) {
		rowNumber = rowCount - 1;
	}

	addWidget (itemPanel);
	item.id.assign (id);
	item.itemPanel = itemPanel;
	item.itemPanel->retain ();
	item.row = rowNumber;
	row = getRow (rowNumber);
	if (row->isLabeled && row->itemLabelFn) {
		item.itemLabel = createItemLabel (item.itemPanel);
		row->itemLabelFn (row->itemLabelFnData, item.itemPanel, item.itemLabel);
	}

	SDL_LockMutex (itemMutex);
	itemList.push_back (item);
	shouldSortItemList = true;
	SDL_UnlockMutex (itemMutex);

	if (! shouldSkipReflow) {
		reflow ();
	}
	return (itemPanel);
}
Widget *CardView::addItem (Panel *itemPanel, int rowNumber, bool shouldSkipReflow) {
	return (addItem (itemPanel, StdString (), rowNumber, shouldSkipReflow));
}

void CardView::removeItem (const StdString &itemId, bool shouldSkipReflow) {
	std::list<CardView::Item>::iterator pos;
	bool found;

	found = false;
	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		found = true;
		clearItem (&(*pos));
		itemList.erase (pos);
		resetItemIdMap ();
		shouldSortItemList = true;
	}
	SDL_UnlockMutex (itemMutex);

	if (found && (! shouldSkipReflow)) {
		reflow ();
	}
}
void CardView::removeItem (const char *itemId, bool shouldSkipReflow) {
	removeItem (StdString (itemId), shouldSkipReflow);
}

void CardView::removeRowItems (int row) {
	std::list<CardView::Item>::iterator i1, i2;
	bool found;

	SDL_LockMutex (itemMutex);
	while (true) {
		found = false;
		i1 = itemList.begin ();
		i2 = itemList.end ();
		while (i1 != i2) {
			if (i1->row == row) {
				clearItem (&(*i1));
				itemList.erase (i1);
				found = true;
				break;
			}
			++i1;
		}
		if (! found) {
			break;
		}
	}
	shouldSortItemList = true;
	SDL_UnlockMutex (itemMutex);

	reflow ();
}

void CardView::setItemRow (const StdString &itemId, int targetRow, bool shouldSkipReflow) {
	CardView::Row *row;
	std::list<CardView::Item>::iterator pos;

	row = getRow (targetRow);
	if (! row) {
		return;
	}
	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if ((pos != itemList.end ()) && (pos->row != targetRow)) {
		pos->row = targetRow;
		shouldSortItemList = true;
	}
	SDL_UnlockMutex (itemMutex);

	if (! shouldSkipReflow) {
		reflow ();
	}
}

void CardView::removeAllItems () {
	std::list<CardView::Item>::iterator i1, i2;

	SDL_LockMutex (itemMutex);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		clearItem (&(*i1));
		++i1;
	}
	itemList.clear ();
	itemIdMap.clear ();
	shouldSortItemList = false;
	SDL_UnlockMutex (itemMutex);

	reflow ();
}

void CardView::processItems (Widget::EventCallback fn, void *fnData, bool shouldReflow) {
	std::list<CardView::Item>::const_iterator i1, i2;

	SDL_LockMutex (itemMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		fn (fnData, i1->itemPanel);
		++i1;
	}
	SDL_UnlockMutex (itemMutex);

	if (shouldReflow) {
		reflow ();
	}
}

void CardView::processRowItems (int rowNumber, Widget::EventCallback fn, void *fnData, bool shouldReflow) {
	std::list<CardView::Item>::const_iterator i1, i2;

	SDL_LockMutex (itemMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->row == rowNumber) {
			fn (fnData, i1->itemPanel);
		}
		++i1;
	}
	SDL_UnlockMutex (itemMutex);

	if (shouldReflow) {
		reflow ();
	}
}

void CardView::scrollToRow (int rowNumber, double positionDeltaY) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if (! row) {
		return;
	}
	setViewOrigin (0.0f, row->positionY + positionDeltaY);
}

void CardView::scrollToItem (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;

	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		setViewOrigin (0.0f, pos->itemPanel->position.y - (height / 2.0f) + (pos->itemPanel->height / 2.0f));
	}
	SDL_UnlockMutex (itemMutex);
}

void CardView::resetItemLabels () {
	std::vector<CardView::Row>::iterator i1, i2;
	std::list<CardView::Item>::iterator j1, j2;
	int rownum;

	rownum = 0;
	i1 = rowList.begin ();
	i2 = rowList.end ();
	while (i1 != i2) {
		if (i1->isLabeled && i1->itemLabelFn) {
			SDL_LockMutex (itemMutex);
			j1 = itemList.begin ();
			j2 = itemList.end ();
			while (j1 != j2) {
				if ((j1->row == rownum) && j1->itemPanel) {
					if (! j1->itemLabel) {
						j1->itemLabel = createItemLabel (j1->itemPanel);
					}
					i1->itemLabelFn (i1->itemLabelFnData, j1->itemPanel, j1->itemLabel);
				}
				++j1;
			}
			SDL_UnlockMutex (itemMutex);
		}
		++rownum;
		++i1;
	}
}

void CardView::reflow () {
	std::list<CardView::Item>::iterator i1, i2;
	CardView::Row *row;
	Panel *itempanel, *headerpanel;
	CardLabelWindow *itemlabel;
	double x, y, dx, dy, x0, itemw, itemh, rowh, rowmargin;
	Position itempos, labelpos;
	int rownum;

	row = NULL;
	rownum = -1;
	x0 = UiConfiguration::instance->paddingSize;
	y = UiConfiguration::instance->paddingSize;
	x = x0;
	rowmargin = itemMarginSize;
	rowh = 0.0f;

	SDL_LockMutex (itemMutex);
	if (shouldSortItemList) {
		doSort ();
		shouldSortItemList = false;
	}
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		itempanel = i1->itemPanel;
		itemlabel = i1->itemLabel;
		itemw = itempanel->width;
		itemh = itempanel->height;
		dx = 0.0f;
		dy = 0.0f;
		if (i1->isAnimatingSelection) {
			itemw *= CardView::reducedSizeItemScale;
			itemh *= CardView::reducedSizeItemScale;
			dx = (itempanel->width - itemw) / -4.0f;
			dy = (itempanel->height - itemh) / -4.0f;
		}

		if (rownum != i1->row) {
			if (rownum >= 0) {
				x = x0;
				y += rowh + itemMarginSize;
				rowh = 0.0f;
			}
			rownum = i1->row;
			row = getRow (rownum);
			if (row) {
				row->positionY = y;
				headerpanel = row->headerPanel;
				if (headerpanel && headerpanel->isVisible) {
					headerpanel->position.assign (x0, y);
					y += headerpanel->height + itemMarginSize;
				}
				rowmargin = row->itemMarginSize;
				if (rowmargin < 0.0f) {
					rowmargin = itemMarginSize;
				}
			}
		}
		if ((x + itemw) >= cardAreaWidth) {
			x = x0;
			y += rowh + rowmargin;
			rowh = 0.0f;
		}
		itempos.assign (x + dx, y + dy);

		if (i1->isPositionAssigned && row && row->isRepositionAnimated) {
			itempanel->position.translate (itempos, UiConfiguration::instance->cardViewRepositionTranslateDuration);
		}
		else {
			itempanel->position.assign (itempos);
		}

		if (itemlabel) {
			if (itempanel->isComposeDrawEnabled) {
				itemlabel->setWindowWidth (itemw);
				labelpos.assign (itempos.x + (itempanel->width / 2.0f) - (itemw / 2.0f), itempos.y + (itempanel->height / 2.0f) - (itemh / 2.0f) + itemh);
			}
			else {
				itemlabel->setWindowWidth (itempanel->width);
				labelpos.assign (itempos.x, itempos.y + itempanel->height);
			}
			if (i1->isPositionAssigned && row && row->isRepositionAnimated) {
				itemlabel->position.translate (labelpos, UiConfiguration::instance->cardViewRepositionTranslateDuration);
			}
			else {
				itemlabel->position.assign (labelpos);
			}
		}

		if (itemlabel) {
			itemh += itemlabel->height;
		}
		if (itemh > rowh) {
			rowh = itemh;
		}
		i1->isPositionAssigned = true;
		x += itemw + rowmargin;
		++i1;
	}
	SDL_UnlockMutex (itemMutex);

	y += rowh;
	y += cardAreaBottomPadding;
	scrollBar->setScrollBounds (height, y);
	y -= height;
	if (y < 0.0f) {
		y = 0.0f;
	}
	setVerticalScrollBounds (0.0f, y);
	if (viewOriginY < 0.0f) {
		setViewOrigin (0.0f, 0.0f);
	}
	else if (viewOriginY > y) {
		setViewOrigin (0.0f, y);
	}

	if (scrollBar->maxScrollPosition <= 0.0f) {
		scrollBar->isVisible = false;
	}
	else {
		scrollBar->position.assign (width - UiConfiguration::instance->paddingSize - scrollBar->width, viewOriginY + UiConfiguration::instance->paddingSize);
		scrollBar->isVisible = true;
	}
}

void CardView::doSort () {
	std::vector<CardView::Row>::iterator i1, i2;
	CardView::Row *targetrow;
	std::list<CardView::Item> outlist, rowitemlist;
	std::list<CardView::Item>::const_iterator j1, j2;
	int row, nextrow;

	i1 = rowList.begin ();
	i2 = rowList.end ();
	while (i1 != i2) {
		i1->itemCount = 0;
		++i1;
	}

	row = -1;
	j1 = itemList.cbegin ();
	j2 = itemList.cend ();
	while (j1 != j2) {
		if ((row < 0) || (j1->row < row)) {
			row = j1->row;
		}
		targetrow = getRow (j1->row);
		if (targetrow) {
			++(targetrow->itemCount);
		}
		++j1;
	}

	while (true) {
		rowitemlist.clear ();
		nextrow = -1;
		j1 = itemList.cbegin ();
		j2 = itemList.cend ();
		while (j1 != j2) {
			if (j1->row == row) {
				rowitemlist.push_back (*j1);
			}
			else if (j1->row > row) {
				if ((nextrow < 0) || (j1->row < nextrow)) {
					nextrow = j1->row;
				}
			}
			++j1;
		}

		targetrow = getRow (row);
		if (targetrow) {
			rowitemlist.sort (targetrow->isReverseSorted ? CardView::compareItemsDescending : CardView::compareItemsAscending);
		}

		j1 = rowitemlist.cbegin ();
		j2 = rowitemlist.cend ();
		while (j1 != j2) {
			outlist.push_back (*j1);
			++j1;
		}

		if (nextrow < 0) {
			break;
		}
		row = nextrow;
	}

	itemList.swap (outlist);
	resetItemIdMap ();

	i1 = rowList.begin ();
	i2 = rowList.end ();
	while (i1 != i2) {
		if (i1->headerPanel) {
			if (i1->itemCount <= 0) {
				i1->headerPanel->isVisible = false;
			}
			else {
				i1->headerPanel->isVisible = true;
			}
		}
		++i1;
	}
}

void CardView::resetItemIdMap () {
	std::list<CardView::Item>::const_iterator i1, i2;
	int index;

	itemIdMap.clear ();
	index = 0;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		itemIdMap.insert (i1->id, index);
		++index;
		++i1;
	}
}

bool CardView::compareItemsAscending (const CardView::Item &a, const CardView::Item &b) {
	return (a.itemPanel->sortKey.compare (b.itemPanel->sortKey) <= 0);
}
bool CardView::compareItemsDescending (const CardView::Item &a, const CardView::Item &b) {
	return (a.itemPanel->sortKey.compare (b.itemPanel->sortKey) > 0);
}

std::list<CardView::Item>::iterator CardView::findItemPosition (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;
	int index;

	index = itemIdMap.find (itemId, -1);
	if ((index < 0) || (index >= (int) itemList.size ())) {
		return (itemList.end ());
	}
	pos = itemList.begin ();
	while (index > 0) {
		++pos;
		--index;
	}
	return (pos);
}

Widget *CardView::getItem (const StdString &itemId, bool shouldRetain) {
	std::list<CardView::Item>::iterator pos;
	Widget *result;

	result = NULL;
	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		result = pos->itemPanel;
		if (shouldRetain) {
			result->retain ();
		}
	}
	SDL_UnlockMutex (itemMutex);
	return (result);
}
Widget *CardView::getItem (const char *itemId, bool shouldRetain) {
	return (getItem (StdString (itemId), shouldRetain));
}

int CardView::getItemCount () {
	int result;

	SDL_LockMutex (itemMutex);
	result = (int) itemList.size ();
	SDL_UnlockMutex (itemMutex);
	return (result);
}

int CardView::getRowItemCount (int rowNumber) {
	CardView::Row *row;

	row = getRow (rowNumber);
	if (! row) {
		return (0);
	}
	return (row->itemCount);
}

Widget *CardView::findItem (CardView::MatchFunction fn, void *fnData, bool shouldRetain) {
	std::list<CardView::Item>::const_iterator i1, i2;
	Widget *result;

	result = NULL;
	SDL_LockMutex (itemMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->itemPanel) {
			if (fn (fnData, i1->itemPanel)) {
				result = i1->itemPanel;
				if (shouldRetain) {
					result->retain ();
				}
				break;
			}
		}
		++i1;
	}
	SDL_UnlockMutex (itemMutex);
	return (result);
}

StdString CardView::findItemId (CardView::MatchFunction fn, void *fnData) {
	std::list<CardView::Item>::const_iterator i1, i2;
	StdString id;

	SDL_LockMutex (itemMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->itemPanel) {
			if (fn (fnData, i1->itemPanel)) {
				id.assign (i1->id);
				break;
			}
		}
		++i1;
	}
	SDL_UnlockMutex (itemMutex);
	return (id);
}

bool CardView::matchPointerValue (void *ptr, Widget *itemWidget) {
	return (ptr == itemWidget);
}

void CardView::scrollBarPositionChanged (void *itPtr, Widget *widgetPtr) {
	CardView *it = (CardView *) itPtr;
	ScrollBar *scrollbar = (ScrollBar *) widgetPtr;

	it->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (it->viewOriginY + UiConfiguration::instance->paddingSize);
}

void CardView::animateItemScaleBump (const StdString &itemId) {
	std::list<CardView::Item>::iterator pos;

	SDL_LockMutex (itemMutex);
	pos = findItemPosition (itemId);
	if (pos != itemList.end ()) {
		if (pos->itemPanel) {
			pos->itemPanel->animateScaleBump ();
		}
	}
	SDL_UnlockMutex (itemMutex);
}

void CardView::composeAnimationComplete (void *itPtr, Widget *widgetPtr) {
	((CardView *) itPtr)->reflow ();
}
