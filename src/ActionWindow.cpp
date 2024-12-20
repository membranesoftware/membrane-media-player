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
#include "Label.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "ComboBox.h"
#include "TextField.h"
#include "TextFieldWindow.h"
#include "TextFlow.h"
#include "Toggle.h"
#include "SliderWindow.h"
#include "ActionWindow.h"

const UiConfiguration::FontType ActionWindow::titleTextFont = UiConfiguration::TitleFont;

// Item types
constexpr const int ComboBoxItem = 1;
constexpr const int TextFieldWindowItem = 2;
constexpr const int ToggleItem = 3;
constexpr const int SliderItem = 4;

ActionWindow::ActionWindow ()
: Panel ()
, isOptionDataValid (true)
, isConfirmed (false)
, isInverseColor (false)
, footerPanel (NULL)
{
	classId = ClassId::ActionWindow;
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	appWindowWidth = App::instance->windowWidth;
	appWindowHeight = App::instance->windowHeight;

	titleLabel = add (new Label (StdString (), ActionWindow::titleTextFont, UiConfiguration::instance->primaryTextColor));
	titleLabel->isInputSuspended = true;
	titleLabel->isVisible = false;

	headerDescriptionText = add (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	headerDescriptionText->setTextColor (UiConfiguration::instance->primaryTextColor);
	headerDescriptionText->isInputSuspended = true;
	headerDescriptionText->isVisible = false;

	confirmButtonTooltipText.assign (UiText::instance->getText (UiTextId::Confirm).capitalized ());
	confirmButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (ActionWindow::confirmButtonClicked, this), confirmButtonTooltipText));

	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (ActionWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::Cancel).capitalized ()));

	reflow ();
}
ActionWindow::~ActionWindow () {
}

ActionWindow *ActionWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ActionWindow) ? (ActionWindow *) widget : NULL);
}

void ActionWindow::setButtonsVisible (bool visible) {
	cancelButton->isVisible = visible;
	confirmButton->isVisible = visible;
	reflow ();
}

void ActionWindow::setTitleText (const StdString &text) {
	titleLabel->setText (text);
	if (text.empty ()) {
		titleLabel->isVisible = false;
	}
	else {
		titleLabel->isVisible = true;
	}
	reflow ();
}

void ActionWindow::setDescriptionText (const StdString &text) {
	headerDescriptionText->setText (text);
	if (text.empty ()) {
		headerDescriptionText->isVisible = false;
	}
	else {
		headerDescriptionText->isVisible = true;
	}
	reflow ();
}

void ActionWindow::setConfirmTooltipText (const StdString &text) {
	confirmButtonTooltipText.assign (text);
	if (isOptionDataValid) {
		confirmButton->setMouseHoverTooltip (confirmButtonTooltipText);
	}
	else {
		confirmButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", confirmButtonTooltipText.c_str (), UiText::instance->getText (UiTextId::ActionWindowInvalidDataTooltip).c_str ()));
	}
}

void ActionWindow::setInverseColor (bool inverse) {
	std::list<ActionWindow::Item>::iterator i1, i2;

	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
		}
		titleLabel->textColor.assign (UiConfiguration::instance->inverseTextColor);
		headerDescriptionText->setTextColor (UiConfiguration::instance->inverseTextColor);
		confirmButton->setInverseColor (true);
		cancelButton->setInverseColor (true);
	}
	else {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		}
		titleLabel->textColor.assign (UiConfiguration::instance->primaryTextColor);
		headerDescriptionText->setTextColor (UiConfiguration::instance->primaryTextColor);
		confirmButton->setInverseColor (false);
		cancelButton->setInverseColor (false);
	}

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		i1->nameLabel->textColor.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		switch (i1->type) {
			case ComboBoxItem: {
				((ComboBox *) i1->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case TextFieldWindowItem: {
				((TextFieldWindow *) i1->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
			case ToggleItem: {
				((Toggle *) i1->optionWidget)->setInverseColor (isInverseColor);
				break;
			}
		}
		if (i1->descriptionText) {
			i1->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		}
		++i1;
	}
}

void ActionWindow::setFooterPanel (Panel *panel) {
	if (footerPanel) {
		footerPanel->isDestroyed = true;
	}
	footerPanel = panel;
	addWidget (footerPanel);
	reflow ();
}

void ActionWindow::addOption (const StdString &optionName, ComboBox *comboBox, const StdString &descriptionText) {
	comboBox->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	comboBox->setInverseColor (isInverseColor);
	doAddOption (ComboBoxItem, optionName, comboBox, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, TextFieldWindow *textFieldWindow, const StdString &descriptionText) {
	textFieldWindow->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textFieldWindow->valueEditCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	textFieldWindow->setInverseColor (isInverseColor);
	textFieldWindow->setFixedPadding (true, 0.0f, 0.0f);
	textFieldWindow->reflow ();
	doAddOption (TextFieldWindowItem, optionName, textFieldWindow, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, Toggle *toggle, const StdString &descriptionText) {
	toggle->stateChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	toggle->setInverseColor (isInverseColor);
	doAddOption (ToggleItem, optionName, toggle, descriptionText);
}

void ActionWindow::addOption (const StdString &optionName, SliderWindow *slider, const StdString &descriptionText) {
	slider->valueChangeCallback = Widget::EventCallbackContext (ActionWindow::optionValueChanged, this);
	slider->setFixedPadding (true, 0.0f, 0.0f);
	slider->setInverseColor (isInverseColor);
	slider->reflow ();
	doAddOption (SliderItem, optionName, slider, descriptionText);
}

void ActionWindow::doAddOption (int itemType, const StdString &optionName, Widget *optionWidget, const StdString &descriptionText) {
	std::list<ActionWindow::Item>::iterator item;

	addWidget (optionWidget);
	item = findItem (optionName, true);
	if (item != itemList.end ()) {
		item->type = itemType;
		if (! item->nameLabel) {
			item->nameLabel = add (new Label (item->name, UiConfiguration::CaptionFont, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor));
		}
		else {
			item->nameLabel->setText (item->name);
		}

		if (item->descriptionText) {
			item->descriptionText->isDestroyed = true;
			item->descriptionText = NULL;
		}
		if (! descriptionText.empty ()) {
			item->descriptionText = add (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
			item->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
			item->descriptionText->setText (descriptionText);
		}

		if (item->optionWidget) {
			item->optionWidget->isDestroyed = true;
		}
		item->optionWidget = optionWidget;
	}
	verifyOptions ();
	reflow ();
}

void ActionWindow::setOptionValue (const StdString &optionName, const char *optionValue, bool shouldSkipChangeCallback) {
	setOptionValue (optionName, StdString (optionValue), shouldSkipChangeCallback);
}

void ActionWindow::setOptionValue (const StdString &optionName, const StdString &optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback);
			break;
		}
		case TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback, true);
			break;
		}
		case ToggleItem: {
			((Toggle *) item->optionWidget)->setChecked (optionValue.equals ("true"), shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	reflow ();
}

void ActionWindow::setOptionValue (const StdString &optionName, int optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (StdString::createSprintf ("%i", optionValue), shouldSkipChangeCallback);
			break;
		}
		case TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (StdString::createSprintf ("%i", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case SliderItem: {
			((SliderWindow *) item->optionWidget)->setValue ((double) optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	reflow ();
}

void ActionWindow::setOptionValue (const StdString &optionName, double optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setValue (StdString::createSprintf ("%f", optionValue), shouldSkipChangeCallback);
			break;
		}
		case TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setValue (StdString::createSprintf ("%f", optionValue), shouldSkipChangeCallback, true);
			break;
		}
		case SliderItem: {
			((SliderWindow *) item->optionWidget)->setValue (optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	reflow ();
}

void ActionWindow::setOptionValue (const StdString &optionName, bool optionValue, bool shouldSkipChangeCallback) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	switch (item->type) {
		case ToggleItem: {
			((Toggle *) item->optionWidget)->setChecked (optionValue, shouldSkipChangeCallback);
			break;
		}
	}

	if (shouldSkipChangeCallback) {
		verifyOptions ();
	}
	reflow ();
}

StdString ActionWindow::getStringValue (const StdString &optionName, const StdString &defaultValue) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}
	switch (item->type) {
		case ComboBoxItem: {
			return (((ComboBox *) item->optionWidget)->getValue ());
		}
		case TextFieldWindowItem: {
			return (((TextFieldWindow *) item->optionWidget)->getValue ());
		}
		case ToggleItem: {
			return (((Toggle *) item->optionWidget)->isChecked ? "true" : "false");
		}
		case SliderItem: {
			return (StdString::createSprintf ("%.2f", ((SliderWindow *) item->optionWidget)->value));;
		}
	}
	return (defaultValue);
}

StdString ActionWindow::getStringValue (const StdString &optionName, const char *defaultValue) {
	return (getStringValue (optionName, StdString (defaultValue)));
}

int ActionWindow::getNumberValue (const StdString &optionName, int defaultValue) {
	std::list<ActionWindow::Item>::iterator item;
	StdString s;
	double val;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}
	switch (item->type) {
		case ComboBoxItem: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case TextFieldWindowItem: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return ((int) val);
			}
			break;
		}
		case SliderItem: {
			return ((int) ((SliderWindow *) item->optionWidget)->value);
		}
	}
	return (defaultValue);
}

double ActionWindow::getNumberValue (const StdString &optionName, double defaultValue) {
	std::list<ActionWindow::Item>::iterator item;
	StdString s;
	double val;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}
	switch (item->type) {
		case ComboBoxItem: {
			s = ((ComboBox *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case TextFieldWindowItem: {
			s = ((TextFieldWindow *) item->optionWidget)->getValue ();
			if (s.parseFloat (&val)) {
				return (val);
			}
			break;
		}
		case SliderItem: {
			return (((SliderWindow *) item->optionWidget)->value);
		}
	}
	return (defaultValue);
}

bool ActionWindow::getBooleanValue (const StdString &optionName, bool defaultValue) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return (defaultValue);
	}
	switch (item->type) {
		case ToggleItem: {
			return (((Toggle *) item->optionWidget)->isChecked);
		}
	}
	return (defaultValue);
}

void ActionWindow::setWidgetNames (const StdString &widgetNamePrefix) {
	std::list<ActionWindow::Item>::const_iterator i1, i2;

	confirmButton->widgetName.sprintf ("%sConfirmButton", widgetNamePrefix.c_str ());
	cancelButton->widgetName.sprintf ("%sCancelButton", widgetNamePrefix.c_str ());
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->optionWidget && (! i1->name.empty ())) {
			i1->optionWidget->widgetName.sprintf ("%s%s", widgetNamePrefix.c_str (), i1->name.c_str ());
		}
		++i1;
	}
}

std::list<ActionWindow::Item>::iterator ActionWindow::findItem (const StdString &optionName, bool createNewItem) {
	std::list<ActionWindow::Item>::iterator i1, i2;
	StdString option;
	ActionWindow::Item item;

	option = optionName.lowercased ();
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (option.equals (i1->name.lowercased ())) {
			return (i1);
		}
		++i1;
	}
	if (createNewItem) {
		item.name.assign (optionName);
		itemList.push_back (item);

		i1 = itemList.begin ();
		i2 = itemList.end ();
		while (i1 != i2) {
			if (option.equals (i1->name.lowercased ())) {
				return (i1);
			}
			++i1;
		}
	}
	return (i2);
}

void ActionWindow::reflow () {
	std::list<ActionWindow::Item>::iterator i1, i2;
	double w, h;

	resetPadding ();
	topLeftLayoutFlow ();

	if (titleLabel->isVisible) {
		titleLabel->flowDown (&layoutFlow);
	}
	if (headerDescriptionText->isVisible) {
		headerDescriptionText->flowDown (&layoutFlow);
	}

	w = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->nameLabel->width > w) {
			w = i1->nameLabel->width;
		}
		++i1;
	}

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		i1->nameLabel->position.assign (layoutFlow.x, i1->nameLabel->getLinePosition (layoutFlow.y));
		i1->optionWidget->position.assign (layoutFlow.x + w + UiConfiguration::instance->marginSize, layoutFlow.y);

		h = i1->nameLabel->height;
		if (i1->optionWidget->height > h) {
			h = i1->optionWidget->height;
			i1->nameLabel->position.assignY (layoutFlow.y + (h / 2.0f) - (i1->nameLabel->height / 2.0f));
		}

		layoutFlow.y += h;
		if (i1->descriptionText) {
			i1->descriptionText->position.assign (layoutFlow.x + w + UiConfiguration::instance->marginSize, layoutFlow.y);
			layoutFlow.y += i1->descriptionText->height + UiConfiguration::instance->marginSize;
		}
		layoutFlow.yExtent = layoutFlow.y;
		layoutFlow.y += UiConfiguration::instance->marginSize;

		++i1;
	}

	nextRowLayoutFlow ();
	if (footerPanel) {
		footerPanel->flowRight (&layoutFlow);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowRight (&layoutFlow);
	}
	if (confirmButton->isVisible) {
		confirmButton->flowRight (&layoutFlow);
	}
	if (footerPanel) {
		footerPanel->centerVertical (&layoutFlow);
	}

	resetSize ();

	bottomRightLayoutFlow ();
	if (confirmButton->isVisible) {
		confirmButton->flowLeft (&layoutFlow);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowLeft (&layoutFlow);
	}
}

void ActionWindow::doResize () {
	std::list<ActionWindow::Item>::iterator i1, i2;
	TextFieldWindow *textfield;
	double w, h;

	Panel::doResize ();

	headerDescriptionText->setViewWidth (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);

	w = App::instance->windowWidth;
	h = App::instance->windowHeight;
	if ((appWindowWidth <= 0.0f) || (appWindowHeight <= 0.0f) || (w <= 0.0f) || (h <= 0.0f)) {
		return;
	}
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->descriptionText) {
			i1->descriptionText->setViewWidth (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
		}
		switch (i1->type) {
			case TextFieldWindowItem: {
				textfield = (TextFieldWindow *) i1->optionWidget;
				textfield->setWindowWidth (textfield->windowWidth * w / appWindowWidth);
				break;
			}
		}
		++i1;
	}
	appWindowWidth = w;
	appWindowHeight = h;
	reflow ();
}

void ActionWindow::confirmButtonClicked (void *itPtr, Widget *widgetPtr) {
	ActionWindow *it = (ActionWindow *) itPtr;

	it->isConfirmed = true;
	it->eventCallback (it->closeCallback);
	it->isDestroyed = true;
}

void ActionWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	ActionWindow *it = (ActionWindow *) itPtr;

	it->isConfirmed = false;
	it->eventCallback (it->closeCallback);
	it->isDestroyed = true;
}

void ActionWindow::optionValueChanged (void *itPtr, Widget *widgetPtr) {
	ActionWindow *it = (ActionWindow *) itPtr;

	it->verifyOptions ();
	it->eventCallback (it->optionChangeCallback);
	it->reflow ();
}

void ActionWindow::setOptionNameText (const StdString &optionName, const StdString &nameText) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	if (item->nameLabel) {
		item->nameLabel->setText (nameText);
	}
	reflow ();
}

void ActionWindow::setOptionDescriptionText (const StdString &optionName, const StdString &descriptionText) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	if (item->descriptionText) {
		item->descriptionText->isDestroyed = true;
		item->descriptionText = NULL;
	}
	if (! descriptionText.empty ()) {
		item->descriptionText = add (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
		item->descriptionText->setTextColor (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		item->descriptionText->setText (descriptionText);
	}
	reflow ();
}

void ActionWindow::setOptionNotEmptyString (const StdString &optionName) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	item->isNotEmptyString = true;
	verifyOptions ();
}

void ActionWindow::setOptionDisabled (const StdString &optionName, bool disable) {
	std::list<ActionWindow::Item>::iterator item;

	item = findItem (optionName);
	if (item == itemList.end ()) {
		return;
	}
	item->isDisabled = disable;
	switch (item->type) {
		case ComboBoxItem: {
			((ComboBox *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case TextFieldWindowItem: {
			((TextFieldWindow *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case ToggleItem: {
			((Toggle *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
		case SliderItem: {
			((SliderWindow *) item->optionWidget)->setDisabled (item->isDisabled);
			break;
		}
	}
}

void ActionWindow::getOptionNames (StringList *destList) {
	std::list<ActionWindow::Item>::const_iterator i1, i2;

	destList->clear ();
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		destList->push_back (i1->name);
		++i1;
	}
}

void ActionWindow::verifyOptions () {
	std::list<ActionWindow::Item>::const_iterator i1, i2;
	StdString s;
	bool windowvalid, optionvalid;

	windowvalid = true;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		optionvalid = true;
		switch (i1->type) {
			case ComboBoxItem: {
				s = ((ComboBox *) i1->optionWidget)->getValue ();
				if (i1->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}
				break;
			}
			case TextFieldWindowItem: {
				s = ((TextFieldWindow *) i1->optionWidget)->getValue ();
				if (i1->isNotEmptyString) {
					if (s.empty ()) {
						optionvalid = false;
						break;
					}
				}
				break;
			}
			case ToggleItem: {
				break;
			}
		}

		if (optionvalid) {
			i1->nameLabel->textColor.translate (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		else {
			windowvalid = false;
			i1->nameLabel->textColor.translate (UiConfiguration::instance->errorTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		}
		++i1;
	}

	isOptionDataValid = windowvalid;
	if (isOptionDataValid) {
		confirmButton->setDisabled (false);
		confirmButton->setMouseHoverTooltip (confirmButtonTooltipText);
	}
	else {
		confirmButton->setDisabled (true);
		confirmButton->setMouseHoverTooltip (StdString::createSprintf ("%s %s", confirmButtonTooltipText.c_str (), UiText::instance->getText (UiTextId::ActionWindowInvalidDataTooltip).c_str ()));
	}
}
