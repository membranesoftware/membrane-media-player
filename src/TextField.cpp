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
#include "ClassId.h"
#include "UiStack.h"
#include "Input.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Label.h"
#include "TextField.h"

constexpr const SDL_Keycode pasteKeycode = SDLK_v;
constexpr const SDL_Keycode deleteBehindKeycode = SDLK_u;
constexpr const SDL_Keycode deleteAheadKeycode = SDLK_k;

TextField::TextField (double fieldWidth, const StdString &promptText)
: Panel ()
, shouldRetainFocusOnReturnKey (false)
, fieldWidth (fieldWidth)
, rightTextPadding (0.0f)
, isDisabled (false)
, isInverseColor (false)
, isPromptErrorColor (false)
, isObscured (false)
, isOvertype (false)
, cursorPosition (0)
, promptLabel (NULL)
, rightTextPaddingPanel (NULL)
, isFocused (false)
, cursorClock (0)
{
	classId = ClassId::TextField;
	normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	focusBorderColor.assign (UiConfiguration::instance->lightPrimaryColor);
	disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
	disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
	editBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
	editBorderColor.assign (UiConfiguration::instance->darkPrimaryColor);
	normalValueTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
	editValueTextColor.assign (UiConfiguration::instance->primaryTextColor);
	disabledValueTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
	promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);
	setPaddingScale (1.0f, 0.5f);

	valueLabel = add (new Label (StdString (), UiConfiguration::CaptionFont, normalValueTextColor));

	cursorPanel = add (new Panel (), 2);
	cursorPanel->setFixedSize (true, UiConfiguration::instance->textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	cursorPanel->setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
	cursorPanel->isVisible = false;

	if (! promptText.empty ()) {
		setPromptText (promptText);
	}

	reflow ();
}
TextField::~TextField () {
}

TextField *TextField::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::TextField) ? (TextField *) widget : NULL);
}

void TextField::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		setKeyFocus (false);
	}
	reflow ();
}

void TextField::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		normalBgColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->mediumInverseBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		disabledBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		editBgColor.assign (UiConfiguration::instance->lightInverseBackgroundColor);
		editBorderColor.assign (UiConfiguration::instance->darkInverseBackgroundColor);
		normalValueTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		editValueTextColor.assign (UiConfiguration::instance->inverseTextColor);
		disabledValueTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		}
	}
	else {
		normalBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		normalBorderColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		focusBorderColor.assign (UiConfiguration::instance->lightPrimaryColor);
		disabledBgColor.assign (UiConfiguration::instance->darkBackgroundColor);
		disabledBorderColor.assign (UiConfiguration::instance->mediumBackgroundColor);
		editBgColor.assign (UiConfiguration::instance->lightBackgroundColor);
		editBorderColor.assign (UiConfiguration::instance->darkPrimaryColor);
		normalValueTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		editValueTextColor.assign (UiConfiguration::instance->primaryTextColor);
		disabledValueTextColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
		if (isPromptErrorColor) {
			promptTextColor.assign (UiConfiguration::instance->errorTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		}
	}

	setFillBg (true, normalBgColor);
	setBorder (true, normalBorderColor);
	if (promptLabel) {
		promptLabel->textColor.assign (promptTextColor);
	}
	if (rightTextPaddingPanel) {
		rightTextPaddingPanel->setFillBg (true, normalBgColor);
	}
	valueLabel->textColor.assign (normalValueTextColor);
	cursorPanel->setFillBg (true, normalValueTextColor);
	reflow ();
}

void TextField::setPromptText (const StdString &text) {
	if (! promptLabel) {
		promptLabel = add (new Label (text, UiConfiguration::CaptionFont, promptTextColor));
	}
	else {
		promptLabel->setText (text);
	}
	reflow ();
}

void TextField::setPromptErrorColor (bool enable) {
	if (isPromptErrorColor == enable) {
		return;
	}
	isPromptErrorColor = enable;
	if (isPromptErrorColor) {
		promptTextColor.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		if (isInverseColor) {
			promptTextColor.assign (UiConfiguration::instance->darkInverseTextColor);
		}
		else {
			promptTextColor.assign (UiConfiguration::instance->lightPrimaryColor);
		}
	}
	if (promptLabel) {
		promptLabel->textColor.translate (promptTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
}

void TextField::setObscured (bool enable) {
	if (isObscured == enable) {
		return;
	}
	isObscured = enable;
	if (isObscured) {
		valueLabel->setObscured (true);
	}
	else {
		valueLabel->setObscured (false);
	}
	reflow ();
}

void TextField::setOvertype (bool enable) {
	if (isOvertype == enable) {
		return;
	}
	isOvertype = enable;
	if (isOvertype) {
		cursorPanel->setFixedSize (true, valueLabel->maxGlyphWidth * UiConfiguration::instance->textFieldOvertypeCursorScale, valueLabel->maxLineHeight);
	}
	else {
		cursorPanel->setFixedSize (true, UiConfiguration::instance->textFieldInsertCursorWidth, valueLabel->maxLineHeight);
	}
}

StdString TextField::getValue () const {
	return (valueLabel->text);
}

void TextField::setText (const StdString &valueText) {
	if (valueText.equals (valueLabel->text)) {
		reflow ();
		return;
	}
	valueLabel->setText (valueText);
	clipCursorPosition ();
	reflow ();
}

void TextField::setValue (const StdString &valueText, bool shouldSkipChangeCallback, bool shouldSkipEditCallback) {
	setText (valueText);
	lastValue.assign (valueText);
	cursorPosition = (int) valueText.length ();
	if (! shouldSkipChangeCallback) {
		eventCallback (valueChangeCallback);
	}
	if (! shouldSkipEditCallback) {
		eventCallback (valueEditCallback);
	}
}

void TextField::appendClipboardText () {
	char *text;
	int textlen;
	StdString val;

	if (! SDL_HasClipboardText ()) {
		return;
	}
	text = SDL_GetClipboardText ();
	if (! text) {
		return;
	}
	textlen = (int) StdString (text).length ();
	clipCursorPosition ();
	val.assign (valueLabel->text);
	if (isOvertype) {
		val.replace ((size_t) cursorPosition, (size_t) textlen, text);
	}
	else {
		val.insert ((size_t) cursorPosition, text);
	}
	SDL_free (text);

	cursorPosition += textlen;
	setText (val);
	eventCallback (valueChangeCallback);
	if (! isKeyFocused) {
		eventCallback (valueEditCallback);
	}
}

void TextField::setFieldWidth (double widthValue) {
	if (FLOAT_EQUALS (fieldWidth, widthValue)) {
		return;
	}
	fieldWidth = widthValue;
	reflow ();
}

void TextField::setLineWidth (int lineLength) {
	setFieldWidth (valueLabel->maxGlyphWidth * (double) lineLength);
}

void TextField::setRightTextPadding (double rightTextPaddingValue) {
	rightTextPadding = rightTextPaddingValue;
	if (! rightTextPaddingPanel) {
		rightTextPaddingPanel = add (new Panel (), 1);
		rightTextPaddingPanel->setFillBg (true, normalBgColor);
	}
	reflow ();
}

void TextField::reflow () {
	double x, y, cursorx, dx;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	if (promptLabel) {
		promptLabel->position.assign (x + (promptLabel->spaceWidth * 2.0f), promptLabel->getLinePosition (y - (heightPadding / 2.0f)));
	}

	if (valueLabel->text.empty () || (cursorPosition <= 0)) {
		cursorx = 0.0f;
		dx = 0.0f;
	}
	else {
		cursorx = valueLabel->getCharacterPosition (cursorPosition);
		dx = fieldWidth - widthPadding - (cursorx + cursorPanel->width);
		if (rightTextPadding > 0.0f) {
			dx -= rightTextPadding;
		}
		else {
			dx -= widthPadding;
		}
		if (dx >= 0.0f) {
			dx = 0.0f;
		}
	}
	valueLabel->position.assign (x + dx, valueLabel->getLinePosition (y - (heightPadding / 2.0f)));
	cursorPanel->position.assign (x + cursorx + dx, (height / 2.0f) - (cursorPanel->height / 2.0f));
	if (rightTextPaddingPanel) {
		rightTextPaddingPanel->setFixedSize (true, rightTextPadding, valueLabel->maxLineHeight + (heightPadding * 2.0f));
		rightTextPaddingPanel->position.assign (width - rightTextPaddingPanel->width, 0.0f);
	}

	if (isKeyFocused) {
		bgColor.translate (editBgColor, UiConfiguration::instance->shortColorTranslateDuration);
		borderColor.translate (editBorderColor, UiConfiguration::instance->shortColorTranslateDuration);
		if (valueLabel->text.empty ()) {
			if (promptLabel) {
				promptLabel->isVisible = true;
			}
			valueLabel->isVisible = false;
		}
		else {
			if (promptLabel) {
				promptLabel->isVisible = false;
			}
			valueLabel->textColor.translate (editValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			valueLabel->isVisible = true;
		}
	}
	else {
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

		if (valueLabel->text.empty ()) {
			valueLabel->isVisible = false;
			if (promptLabel) {
				if (isDisabled) {
					promptLabel->isVisible = false;
				}
				else {
					promptLabel->isVisible = true;
				}
			}
		}
		else {
			if (promptLabel) {
				promptLabel->isVisible = false;
			}
			if (isDisabled) {
				valueLabel->textColor.translate (disabledValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			else {
				valueLabel->textColor.translate (normalValueTextColor, UiConfiguration::instance->shortColorTranslateDuration);
			}
			valueLabel->isVisible = true;
		}
	}
	setFixedSize (true, fieldWidth, valueLabel->maxLineHeight + (heightPadding * 2.0f));
}

bool TextField::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	StdString val;
	char c;
	int len;

	if (isDisabled || (! isKeyFocused)) {
		return (false);
	}
	switch (keycode) {
		case SDLK_ESCAPE: {
			setText (lastValue);
			eventCallback (valueChangeCallback);
			setKeyFocus (false);
			return (true);
		}
		case SDLK_RETURN: {
			if (shouldRetainFocusOnReturnKey) {
				if (! lastValue.equals (valueLabel->text)) {
					eventCallback (valueEditCallback);
				}
			}
			else {
				setKeyFocus (false);
			}
			return (true);
		}
		case SDLK_LEFT: {
			if (cursorPosition > 0) {
				--cursorPosition;
				reflow ();
			}
			return (true);
		}
		case SDLK_RIGHT: {
			if (cursorPosition < (int) valueLabel->text.length ()) {
				++cursorPosition;
				reflow ();
			}
			return (true);
		}
		case SDLK_HOME: {
			if (cursorPosition != 0) {
				cursorPosition = 0;
				reflow ();
			}
			return (true);
		}
		case SDLK_END: {
			len = (int) valueLabel->text.length ();
			if (cursorPosition != len) {
				cursorPosition = len;
				reflow ();
			}
			return (true);
		}
		case SDLK_INSERT: {
			setOvertype (! isOvertype);
			return (true);
		}
		case SDLK_BACKSPACE: {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			len = val.length ();
			if ((len > 0) && (cursorPosition > 0)) {
				val.erase (cursorPosition - 1, 1);
				--cursorPosition;
				setText (val);
				eventCallback (valueChangeCallback);
			}
			return (true);
		}
		case SDLK_DELETE: {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			if (cursorPosition < (int) val.length ()) {
				val.erase ((size_t) cursorPosition, 1);
				setText (val);
				eventCallback (valueChangeCallback);
			}
			return (true);
		}
	}

	if (isControlDown && (keycode == pasteKeycode)) {
		appendClipboardText ();
		return (true);
	}
	if (isControlDown && (keycode == deleteBehindKeycode)) {
		clipCursorPosition ();
		val.assign (valueLabel->text);
		len = val.length ();
		if ((len > 0) && (cursorPosition > 0)) {
			val.erase (0, cursorPosition);
			cursorPosition = 0;
			setText (val);
			eventCallback (valueChangeCallback);
		}
		return (true);
	}
	if (isControlDown && (keycode == deleteAheadKeycode)) {
		clipCursorPosition ();
		val.assign (valueLabel->text);
		len = val.length ();
		if ((len > 0) && (cursorPosition < len)) {
			val.erase (cursorPosition);
			cursorPosition = val.length ();
			setText (val);
			eventCallback (valueChangeCallback);
		}
		return (true);
	}
	if (! isControlDown) {
		c = Input::instance->getKeyCharacter (keycode, isShiftDown);
		if (c > 0) {
			clipCursorPosition ();
			val.assign (valueLabel->text);
			if (isOvertype) {
				val.replace ((size_t) cursorPosition, 1, 1, c);
			}
			else {
				val.insert ((size_t) cursorPosition, 1, c);
			}
			++cursorPosition;
			setText (val);
			eventCallback (valueChangeCallback);
			return (true);
		}
	}

	return (false);
}

void TextField::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);

	if (isKeyFocused && (! isDisabled) && (UiConfiguration::instance->blinkDuration > 0)) {
		cursorClock -= msElapsed;
		if (cursorClock <= 0) {
			cursorPanel->isVisible = (! cursorPanel->isVisible);
			if (cursorPanel->isVisible) {
				clipCursorPosition ();
				reflow ();
			}
			cursorClock %= UiConfiguration::instance->blinkDuration;
			cursorClock += UiConfiguration::instance->blinkDuration;
		}
	}
	else {
		cursorPanel->isVisible = false;
	}
}

bool TextField::doProcessMouseState (const Widget::MouseState &mouseState) {
	if (isDisabled) {
		return (false);
	}
	if (mouseState.isEntered) {
		setFocused (true);
		if (mouseState.isLeftClickReleased && mouseState.isLeftClickEntered) {
			UiStack::instance->setKeyFocusTarget (this);
		}
	}
	else {
		setFocused (false);
	}
	return (false);
}

void TextField::setFocused (bool enable) {
	if (enable == isFocused) {
		return;
	}
	isFocused = enable;
	reflow ();
}

void TextField::setKeyFocus (bool enable) {
	if (enable == isKeyFocused) {
		return;
	}
	if (enable) {
		isKeyFocused = true;
		lastValue.assign (valueLabel->text);
		cursorPosition = (int) valueLabel->text.length ();
		setOvertype (false);
	}
	else {
		isKeyFocused = false;
		if (! lastValue.equals (valueLabel->text)) {
			eventCallback (valueEditCallback);
		}
	}
	reflow ();
}

void TextField::clipCursorPosition () {
	int len;

	if (cursorPosition < 0) {
		cursorPosition = 0;
	}
	len = (int) valueLabel->text.length ();
	if (cursorPosition > len) {
		cursorPosition = len;
	}
}
