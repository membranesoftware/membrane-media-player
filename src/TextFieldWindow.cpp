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
#include "OsUtil.h"
#include "HashMap.h"
#include "Prng.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Ui.h"
#include "UiStack.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "Button.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Toggle.h"
#include "FsBrowserWindow.h"
#include "TextField.h"
#include "TextFieldWindow.h"

constexpr const double fsBrowserWindowScale = 0.83f;

TextFieldWindow::TextFieldWindow (double windowWidth, const StdString &promptText, Sprite *iconSprite)
: Panel ()
, shouldSkipTextClearCallbacks (false)
, windowWidth (windowWidth)
, isDisabled (false)
, isInverseColor (false)
, isObscured (false)
, iconImage (NULL)
, visibilityToggle (NULL)
, fsBrowserPanelHandle (&fsBrowserPanel)
, isCancelled (false)
{
	Image *image;

	classId = ClassId::TextFieldWindow;
	setPaddingScale (1.0f, 0.5f);

	textField = add (new TextField (windowWidth - UiConfiguration::instance->marginSize - UiConfiguration::instance->paddingSize, promptText));
	textField->valueChangeCallback = Widget::EventCallbackContext (TextFieldWindow::textFieldValueChanged, this);
	textField->valueEditCallback = Widget::EventCallbackContext (TextFieldWindow::textFieldValueEdited, this);
	if (iconSprite) {
		image = new Image (iconSprite);
		iconImage = add (new ImageWindow (image));
		iconImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
		iconImage->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
		iconImage->setWindowSize (true, image->width + UiConfiguration::instance->paddingSize, textField->height);
	}

	enterButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_enterTextButton)), 1);
	enterButton->setInverseColor (true);
	enterButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::enterButtonClicked, this);
	enterButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldEnterTooltip));
	enterButton->isVisible = false;

	cancelButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton)), 1);
	cancelButton->setInverseColor (true);
	cancelButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::cancelButtonClicked, this);
	cancelButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Cancel).capitalized ());
	cancelButton->isVisible = false;

	pasteButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_pasteButton)), 1);
	pasteButton->isDropShadowDisabled = true;
	pasteButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::pasteButtonClicked, this);
	pasteButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldPasteTooltip));
	pasteButton->isVisible = false;

	clearButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_clearTextButton)), 1);
	clearButton->isDropShadowDisabled = true;
	clearButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::clearButtonClicked, this);
	clearButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldClearTooltip));
	clearButton->isVisible = false;

	randomizeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_randomizeButton)), 1);
	randomizeButton->isDropShadowDisabled = true;
	randomizeButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::randomizeButtonClicked, this);
	randomizeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldRandomizeTooltip));
	randomizeButton->isVisible = false;

	fsBrowseButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_directoryButton)), 1);
	fsBrowseButton->isDropShadowDisabled = true;
	fsBrowseButton->mouseClickCallback = Widget::EventCallbackContext (TextFieldWindow::fsBrowseButtonClicked, this);
	fsBrowseButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldFsBrowseTooltip));
	fsBrowseButton->isVisible = false;

	reflow ();
}
TextFieldWindow::~TextFieldWindow () {
	fsBrowserPanelHandle.destroyAndClear ();
}

TextFieldWindow *TextFieldWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::TextFieldWindow) ? (TextFieldWindow *) widget : NULL);
}

void TextFieldWindow::setDisabled (bool disabled) {
	if (isDisabled == disabled) {
		return;
	}
	isDisabled = disabled;
	textField->setDisabled (isDisabled);
	enterButton->setDisabled (isDisabled);
	cancelButton->setDisabled (isDisabled);
	pasteButton->setDisabled (isDisabled);
	clearButton->setDisabled (isDisabled);
	randomizeButton->setDisabled (isDisabled);
	fsBrowseButton->setDisabled (isDisabled);
	if (visibilityToggle) {
		visibilityToggle->setDisabled (isDisabled);
	}
	reflow ();
}

void TextFieldWindow::setRetainFocusOnReturnKey (bool enable) {
	textField->shouldRetainFocusOnReturnKey = enable;
}

void TextFieldWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	textField->setInverseColor (isInverseColor);
	enterButton->setInverseColor (isInverseColor);
	cancelButton->setInverseColor (isInverseColor);
	pasteButton->setInverseColor (isInverseColor);
	clearButton->setInverseColor (isInverseColor);
	randomizeButton->setInverseColor (isInverseColor);
	fsBrowseButton->setInverseColor (isInverseColor);
	if (visibilityToggle) {
		visibilityToggle->setInverseColor (isInverseColor);
	}
	reflow ();
}

void TextFieldWindow::setPromptText (const StdString &text) {
	textField->setPromptText (text);
}

void TextFieldWindow::setPromptErrorColor (bool enable) {
	textField->setPromptErrorColor (enable);
}

void TextFieldWindow::setObscured (bool enable) {
	if (isObscured == enable) {
		return;
	}
	if (visibilityToggle) {
		visibilityToggle->isDestroyed = true;
		visibilityToggle = NULL;
	}
	isObscured = enable;
	if (isObscured) {
		textField->setObscured (true);
		visibilityToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_visibilityOffButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_visibilityOnButton)), 1);
		visibilityToggle->isDropShadowDisabled = true;
		visibilityToggle->stateChangeCallback = Widget::EventCallbackContext (TextFieldWindow::visibilityToggleStateChanged, this);
		visibilityToggle->setInverseColor (isInverseColor);
		visibilityToggle->setMouseHoverTooltip (UiText::instance->getText (UiTextId::TextFieldVisibilityToggleTooltip));
	}
	else {
		textField->setObscured (false);
	}
	reflow ();
}

StdString TextFieldWindow::getValue () {
	if (isCancelled) {
		return (cancelValue);
	}
	return (textField->getValue ());
}

void TextFieldWindow::setValue (const StdString &valueText, bool shouldSkipChangeCallback, bool shouldSkipEditCallback) {
	textField->setValue (valueText, shouldSkipChangeCallback, shouldSkipEditCallback);
	cancelValue.assign (valueText);
	reflow ();
}

void TextFieldWindow::assignKeyFocus () {
	UiStack::instance->setKeyFocusTarget (textField);
}

void TextFieldWindow::setWindowWidth (double windowWidthValue) {
	if (FLOAT_EQUALS (windowWidth, windowWidthValue)) {
		return;
	}
	windowWidth = windowWidthValue;
	reflow ();
}

void TextFieldWindow::setButtonsEnabled (int buttonOptionsValue) {
	buttonOptions = buttonOptionsValue;
	enterButton->isVisible = (buttonOptions & TextFieldWindow::EnterButtonOption) != 0;
	cancelButton->isVisible = (buttonOptions & TextFieldWindow::CancelButtonOption) != 0;
	pasteButton->isVisible = (buttonOptions & TextFieldWindow::PasteButtonOption) != 0;
	clearButton->isVisible = (buttonOptions & TextFieldWindow::ClearButtonOption) != 0;
	randomizeButton->isVisible = (buttonOptions & TextFieldWindow::RandomizeButtonOption) != 0;
	fsBrowseButton->isVisible = (buttonOptions & TextFieldWindow::FsBrowseButtonOption) != 0;
	reflow ();
}

void TextFieldWindow::reflow () {
	double x, y, w, h;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	w = windowWidth - (widthPadding * 2.0f);
	if (iconImage) {
		w -= iconImage->width;
	}
	if (enterButton->isVisible) {
		w -= enterButton->width + UiConfiguration::instance->marginSize;
	}
	if (cancelButton->isVisible) {
		w -= cancelButton->width + UiConfiguration::instance->marginSize;
	}
	textField->setFieldWidth (w);

	w = 0.0f;
	if (visibilityToggle) {
		w += visibilityToggle->width;
	}
	if (clearButton->isVisible) {
		w += clearButton->width;
	}
	if (pasteButton->isVisible) {
		w += pasteButton->width;
	}
	if (randomizeButton->isVisible) {
		w += randomizeButton->width;
	}
	if (fsBrowseButton->isVisible) {
		w += fsBrowseButton->width;
	}
	textField->setRightTextPadding (w);

	h = 0.0f;
	if (iconImage) {
		iconImage->position.assign (x, y);
		x += iconImage->width;
		if (iconImage->height > h) {
			h = iconImage->height;
		}
	}

	if (textField->height > h) {
		h = textField->height;
	}
	textField->position.assign (x, y);
	x += textField->width + UiConfiguration::instance->marginSize;

	if (cancelButton->isVisible) {
		if (cancelButton->height > h) {
			h = cancelButton->height;
		}
		cancelButton->position.assign (x, y);
		x += cancelButton->width + UiConfiguration::instance->marginSize;
	}

	if (enterButton->isVisible) {
		if (enterButton->height > h) {
			h = enterButton->height;
		}
		enterButton->position.assign (x, y);
		x += enterButton->width + UiConfiguration::instance->marginSize;
	}
	h += (heightPadding * 2.0f);
	setFixedSize (true, windowWidth, h);

	textField->position.assignY ((h / 2.0f) - (textField->height / 2.0f));
	x = textField->position.x + textField->width;
	y = textField->position.y + (textField->height / 2.0f);
	if (iconImage) {
		iconImage->position.assignY (textField->position.y + (textField->height / 2.0f) - (iconImage->height / 2.0f));
	}
	if (visibilityToggle) {
		x -= visibilityToggle->width;
		visibilityToggle->position.assign (x, y - (visibilityToggle->height / 2.0f));
	}
	if (clearButton->isVisible) {
		x -= clearButton->width;
		clearButton->position.assign (x, y - (clearButton->height / 2.0f));
	}
	if (pasteButton->isVisible) {
		x -= pasteButton->width;
		pasteButton->position.assign (x, y - (pasteButton->height / 2.0f));
	}
	if (randomizeButton->isVisible) {
		x -= randomizeButton->width;
		randomizeButton->position.assign (x, y - (randomizeButton->height / 2.0f));
	}
	if (fsBrowseButton->isVisible) {
		x -= fsBrowseButton->width;
		fsBrowseButton->position.assign (x, y - (randomizeButton->height / 2.0f));
	}
}

void TextFieldWindow::textFieldValueChanged (void *itPtr, Widget *widgetPtr) {
	((TextFieldWindow *) itPtr)->eventCallback (((TextFieldWindow *) itPtr)->valueChangeCallback);
}

void TextFieldWindow::textFieldValueEdited (void *itPtr, Widget *widgetPtr) {
	((TextFieldWindow *) itPtr)->eventCallback (((TextFieldWindow *) itPtr)->valueEditCallback);
}

void TextFieldWindow::enterButtonClicked (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;

	it->textField->setKeyFocus (false);
	it->eventCallback (it->enterButtonClickCallback);
}

void TextFieldWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;

	it->isCancelled = true;
	it->isDestroyed = true;
}

void TextFieldWindow::pasteButtonClicked (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;

	UiStack::instance->setKeyFocusTarget (it->textField);
	it->textField->appendClipboardText ();
}

void TextFieldWindow::clearButtonClicked (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;

	it->cancelValue.assign ("");
	it->textField->setValue (StdString (), it->shouldSkipTextClearCallbacks, it->shouldSkipTextClearCallbacks);
}

void TextFieldWindow::randomizeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((TextFieldWindow *) itPtr)->textField->setValue (Prng::instance->getRandomString (TextFieldWindow::randomizeStringLength));
}

void TextFieldWindow::fsBrowseButtonClicked (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;
	FsBrowserWindow *fs;
	HashMap *prefs;
	StdString path;

	prefs = App::instance->lockPrefs ();
	path = prefs->find (App::fsBrowserPathKey, "");
	App::instance->unlockPrefs ();
	if (path.empty ()) {
		path = OsUtil::getUserHomePath ();
	}
	it->fsBrowserPanelHandle.destroyAndAssign (Ui::createDarkWindowOverlayPanel ());

	fs = (FsBrowserWindow *) it->fsBrowserPanel->add (new FsBrowserWindow (App::instance->drawableWidth * fsBrowserWindowScale, App::instance->drawableHeight * fsBrowserWindowScale, path));
	if (it->buttonOptions & TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption) {
		fs->sortOrder = FsBrowserWindow::DirectoriesFirstSort;
	}
	if (it->buttonOptions & TextFieldWindow::FsBrowseButtonSelectDirectoriesOption) {
		fs->selectType = FsBrowserWindow::SelectDirectories;
	}
	fs->closeCallback = Widget::EventCallbackContext (TextFieldWindow::fsBrowserWindowClosed, it);
	fs->position.assign ((App::instance->drawableWidth - fs->width) / 2.0f, (App::instance->drawableHeight - fs->height) / 2.0f);

	App::instance->rootPanel->addWidget (it->fsBrowserPanel, App::instance->rootPanel->maxWidgetZLevel + 1);
}

void TextFieldWindow::fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;
	FsBrowserWindow *fs = (FsBrowserWindow *) widgetPtr;
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	prefs->insert (App::fsBrowserPathKey, fs->browsePath, "");
	App::instance->unlockPrefs ();

	if (fs->isPathSelectionConfirmed) {
		it->setValue (fs->selectedPath);
	}
	it->fsBrowserPanelHandle.destroyAndClear ();
}

void TextFieldWindow::visibilityToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	TextFieldWindow *it = (TextFieldWindow *) itPtr;

	if (it->visibilityToggle->isChecked) {
		it->textField->setObscured (false);
	}
	else {
		it->textField->setObscured (true);
	}
}
