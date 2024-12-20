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
#include "Input.h"
#include "Sprite.h"
#include "HashMap.h"
#include "StringList.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ScrollView.h"
#include "ScrollBar.h"
#include "ComboBox.h"

ComboBox::ComboBox ()
: Panel ()
, isDisabled (false)
, isInverseColor (false)
, hasItemData (false)
, expandViewHandle (&expandView)
, expandScrollBarHandle (&expandScrollBar)
, isExpanded (false)
, expandScreenX (0.0f)
, expandScreenY (0.0f)
, isFocused (false)
, selectedItemLabel (NULL)
, maxTextWidth (0.0f)
{
	classId = ClassId::ComboBox;
	normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBorderColor.assign (UiConfiguration::instance->lightBackgroundColor);
	disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
	normalItemTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
	focusItemTextColor.assign (UiConfiguration::instance->primaryTextColor);
	disabledTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);

	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);
}
ComboBox::~ComboBox () {
	expandViewHandle.destroyAndClear ();
	expandScrollBarHandle.destroyAndClear ();
}

ComboBox *ComboBox::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ComboBox) ? (ComboBox *) widget : NULL);
}

void ComboBox::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		unexpand ();
	}
	reflow ();
}

void ComboBox::setInverseColor (bool inverse) {
	std::list<ComboBox::Item>::iterator i1, i2;

	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalItemTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		focusItemTextColor.assign (UiConfiguration::instance->inverseTextColor);
		disabledTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
	}
	else {
		normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->lightBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
		normalItemTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		focusItemTextColor.assign (UiConfiguration::instance->primaryTextColor);
		disabledTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
	}

	bgColor.assign (normalBgColor);
	borderColor.assign (normalBorderColor);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		i1->label->setTextColor (normalItemTextColor);
		i1->label->setFillBg (true, normalBgColor);
		i1->label->setBorder (true, normalBorderColor);
		++i1;
	}

	reflow ();
}

void ComboBox::setValue (const StdString &value, bool shouldSkipChangeCallback) {
	std::list<ComboBox::Item>::iterator i1, i2;
	bool found;

	found = false;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->value.equals (value)) {
			selectedItemLabel = i1->label;
			selectedItemValue.assign (i1->value);
			selectedItemData.assign (i1->itemData);
			found = true;
			break;
		}
		++i1;
	}
	if (found) {
		reflow ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}

void ComboBox::setValueByItemData (const StdString &itemData, bool shouldSkipChangeCallback) {
	std::list<ComboBox::Item>::iterator i1, i2;
	bool found;

	if (! hasItemData) {
		return;
	}
	found = false;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->itemData.equals (itemData)) {
			selectedItemLabel = i1->label;
			selectedItemValue.assign (i1->value);
			selectedItemData.assign (i1->itemData);
			found = true;
			break;
		}
		++i1;
	}

	if (found) {
		reflow ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}

StdString ComboBox::getValue () {
	if (hasItemData) {
		return (selectedItemData);
	}
	return (selectedItemValue);
}

void ComboBox::addItem (const StdString &itemValue) {
	ComboBox::Item item;
	LabelWindow *label;
	double w;

	label = add (new LabelWindow (new Label (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (itemValue, UiConfiguration::instance->comboBoxLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Font::dotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
	label->setFillBg (true, normalBgColor);
	label->setBorder (true, normalBorderColor);
	w = label->getTextWidth ();
	if (w > maxTextWidth) {
		maxTextWidth = w;
	}
	if (! selectedItemLabel) {
		selectedItemLabel = label;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign ("");
	}

	item.value.assign (itemValue);
	item.itemData.assign ("");
	item.label = label;
	itemList.push_back (item);
	reflow ();
}

void ComboBox::addItem (const StdString &itemValue, const StdString &itemData) {
	ComboBox::Item item;
	LabelWindow *label;
	double w;

	label = add (new LabelWindow (new Label (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (itemValue, UiConfiguration::instance->comboBoxLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, Font::dotTruncateSuffix), UiConfiguration::CaptionFont, normalItemTextColor)));
	label->setFillBg (true, normalBgColor);
	label->setBorder (true, normalBorderColor);
	w = label->getTextWidth ();
	if (w > maxTextWidth) {
		maxTextWidth = w;
	}
	if (! selectedItemLabel) {
		selectedItemLabel = label;
		selectedItemValue.assign (itemValue);
		selectedItemData.assign (itemData);
	}

	item.value.assign (itemValue);
	item.itemData.assign (itemData);
	item.label = label;
	itemList.push_back (item);
	hasItemData = true;
	reflow ();
}

void ComboBox::addItems (const StringList &nameList) {
	StringList::const_iterator i1, i2;

	i1 = nameList.cbegin ();
	i2 = nameList.cend ();
	while (i1 != i2) {
		addItem (*i1);
		++i1;
	}
}

void ComboBox::addItems (HashMap *itemMap) {
	HashMap::Iterator i;
	StdString key, value;

	i = itemMap->begin ();
	while (itemMap->next (&i, &key)) {
		value = itemMap->find (key, StdString ());
		addItem (key, value);
	}
}

bool ComboBox::containsItem (const StdString &value) const {
	std::list<ComboBox::Item>::const_iterator i1, i2;
	bool found;

	found = false;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->value.equals (value)) {
			found = true;
			break;
		}
		++i1;
	}
	return (found);
}

void ComboBox::reflow () {
	std::list<ComboBox::Item>::iterator i1, i2;
	LabelWindow *label;
	double x, y;

	if (isDisabled) {
		bgColor.translate (disabledBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (disabledBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else if (isFocused) {
		bgColor.translate (focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (focusBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		bgColor.translate (normalBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (normalBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
	}

	x = 0.0f;
	y = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		label = i1->label;
		if (label != selectedItemLabel) {
			label->isVisible = false;
		}
		else {
			label->setWindowWidth (maxTextWidth + (UiConfiguration::instance->paddingSize * 2.0f), true);
			label->position.assign (x, y);
			if (isDisabled) {
				label->bgColor.translate (disabledBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (disabledBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (disabledTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else if (isFocused) {
				label->bgColor.translate (focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (focusBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (focusItemTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else {
				label->bgColor.translate (normalBgColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->borderColor.translate (normalBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
				label->translateTextColor (normalItemTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			y += label->height;
			label->isVisible = true;
		}
		++i1;
	}

	resetSize ();
}

void ComboBox::doResize () {
	LabelWindow *label;
	std::list<ComboBox::Item>::iterator i1, i2;
	double w, maxw;

	Panel::doResize ();
	maxw = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		label = i1->label;
		label->resize ();
		w = label->getTextWidth ();
		if (w > maxw) {
			maxw = w;
		}
		++i1;
	}
	maxTextWidth = maxw;
	reflow ();
}

void ComboBox::setFocused (bool focused) {
	if (isFocused == focused) {
		return;
	}
	isFocused = focused;
	reflow ();
}

bool ComboBox::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool shouldunexpand;
	int x, y, x1, x2, y1, y2;

	if (isDisabled) {
		return (false);
	}
	if (isExpanded) {
		setFocused (false);
		if (mouseState.isLeftClicked) {
			shouldunexpand = false;
			if (mouseState.isEntered) {
				shouldunexpand = true;
			}

			if ((! shouldunexpand) && expandView) {
				x = Input::instance->mouseX;
				y = Input::instance->mouseY;
				x1 = (int) expandView->screenX;
				y1 = (int) expandView->screenY;
				x2 = x1 + (int) expandView->width;
				y2 = y1 + (int) expandView->height;
				if ((x < x1) || (x > x2) || (y < y1) || (y > y2)) {
					shouldunexpand = true;
				}
			}
			if (shouldunexpand) {
				unexpand ();
			}
		}
		return (false);
	}

	setFocused (mouseState.isEntered);
	if (mouseState.isEntered && mouseState.isLeftClicked) {
		expand ();
	}
	return (false);
}

void ComboBox::expand () {
	std::list<ComboBox::Item>::const_iterator i1, i2;
	LabelWindow *label;
	double x, y, w, h;
	int count, z;

	if (isExpanded || isDisabled || (itemList.size () < 2)) {
		return;
	}
	expandViewHandle.destroyAndAssign (new ScrollView ());
	expandView->setFillBg (true, normalBgColor);
	expandView->setBorder (true, normalBorderColor);
	expandView->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);

	count = 0;
	x = 0.0f;
	y = 0.0f;
	w = maxTextWidth + (UiConfiguration::instance->paddingSize * 2.0f);
	h = 0.0f;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->label != selectedItemLabel) {
			label = expandView->add (new LabelWindow (new Label (i1->label->getText (), UiConfiguration::CaptionFont, normalItemTextColor)));
			label->position.assign (x, y);
			label->mouseClickCallback = Widget::EventCallbackContext (ComboBox::expandItemClicked, this);
			label->setFillBg (true, normalBgColor);
			label->setWindowWidth (w);
			label->setMouseoverHighlight (true, normalItemTextColor, normalBgColor, focusItemTextColor, focusBgColor, UiConfiguration::instance->shortColorTranslateDuration);
			y += label->height;
			if (count < UiConfiguration::instance->comboBoxExpandViewItems) {
				h = y;
			}
			++count;
		}
		++i1;
	}

	expandView->setViewSize (w, h);
	expandView->setVerticalScrollBounds (0.0f, y - h);

	expandScrollBarHandle.destroyAndClear ();
	if (count > UiConfiguration::instance->comboBoxExpandViewItems) {
		expandScrollBarHandle.assign (new ScrollBar (h));
		expandScrollBar->positionChangeCallback = Widget::EventCallbackContext (ComboBox::scrollBarPositionChanged, this);
		expandScrollBar->updateCallback = Widget::UpdateCallbackContext (ComboBox::scrollBarUpdated, this);
		expandScrollBar->setScrollBounds (h, y);
		expandView->isMouseWheelScrollEnabled = true;
	}
	expandView->reflow ();
	expandScreenX = screenX;
	expandScreenY = screenY;

	x = screenX;
	y = screenY + height;
	if ((y + expandView->height) >= App::instance->drawableHeight) {
		y = screenY - expandView->height;
	}
	z = App::instance->rootPanel->maxWidgetZLevel;
	App::instance->rootPanel->addWidget (expandView, Position (x, y), z + 1);
	if (expandScrollBar) {
		App::instance->rootPanel->addWidget (expandScrollBar, Position (x + w - expandScrollBar->width, y), z + 2);
	}
	isExpanded = true;
}

void ComboBox::scrollBarPositionChanged (void *itPtr, Widget *widgetPtr) {
	ComboBox *it = (ComboBox *) itPtr;
	ScrollBar *scrollbar = (ScrollBar *) widgetPtr;

	if (! it->expandView) {
		return;
	}
	it->expandView->setViewOrigin (0.0f, scrollbar->scrollPosition);
}

void ComboBox::scrollBarUpdated (void *itPtr, int msElapsed, Widget *widgetPtr) {
	ComboBox *it = (ComboBox *) itPtr;
	ScrollBar *scrollbar = (ScrollBar *) widgetPtr;

	if (! it->expandView) {
		return;
	}
	scrollbar->setPosition (it->expandView->viewOriginY, true);
}

void ComboBox::unexpand () {
	isExpanded = false;
	expandViewHandle.destroyAndClear ();
	expandScrollBarHandle.destroyAndClear ();
}

void ComboBox::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isExpanded) {
		expandViewHandle.compact ();
		if ((! expandView) || (! FLOAT_EQUALS (screenX, expandScreenX)) || (! FLOAT_EQUALS (screenY, expandScreenY))) {
			unexpand ();
		}
	}
}

void ComboBox::expandItemClicked (void *itPtr, Widget *widgetPtr) {
	ComboBox *it = (ComboBox *) itPtr;
	LabelWindow *label = (LabelWindow *) widgetPtr;

	it->setValueByLabelWindow (label);
	it->unexpand ();
}

void ComboBox::setValueByLabelWindow (LabelWindow *choiceLabel, bool shouldSkipChangeCallback) {
	StdString choicetext;
	std::list<ComboBox::Item>::iterator i1, i2;
	bool found;

	choicetext = choiceLabel->getText ();
	found = false;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (choicetext.equals (i1->label->getText ())) {
			selectedItemLabel = i1->label;
			selectedItemValue.assign (i1->value);
			selectedItemData.assign (i1->itemData);
			found = true;
			break;
		}
		++i1;
	}
	if (found) {
		reflow ();
		if (! shouldSkipChangeCallback) {
			eventCallback (valueChangeCallback);
		}
	}
}
