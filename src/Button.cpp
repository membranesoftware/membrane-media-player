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
#include "App.h"
#include "Input.h"
#include "Widget.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "ButtonGradientMiddleSprite.h"
#include "ButtonGradientEndSprite.h"
#include "Button.h"

constexpr const double focusTextOffset = 2.0f;

constexpr const int EmptyBody = 0;
constexpr const int ColorBody = 1;
constexpr const int GradientBody = 2;
constexpr const int TransparentBody = 3;

Button::Button ()
: Panel ()
, shortcutKey (SDLK_UNKNOWN)
, isShortcutKeyControlPress (false)
, isDropShadowDisabled (false)
, buttonWidth (0.0f)
, maxImageWidth (0.0f)
, maxImageHeight (0.0f)
, isFocused (false)
, isPressed (false)
, isDisabled (false)
, isInverseColor (false)
, bodyType (EmptyBody)
, promptLabel (NULL)
, iconImage (NULL)
, pressClock (0)
{
	classId = ClassId::Button;
	mouseEnterCallback = Widget::EventCallbackContext (Button::mouseEntered, this);
	mouseExitCallback = Widget::EventCallbackContext (Button::mouseExited, this);
	mousePressCallback = Widget::EventCallbackContext (Button::mousePressed, this);
	mouseReleaseCallback = Widget::EventCallbackContext (Button::mouseReleased, this);
	normalTextColor.assign (UiConfiguration::instance->buttonTextColor);
	normalIconColor.assign (UiConfiguration::instance->buttonTextColor);
}

Button::Button (Sprite *iconSprite)
: Button ()
{
	iconImage = add (new Image (iconSprite, Button::NormalFrame));
	iconImage->drawAlpha = UiConfiguration::instance->buttonUnfocusedIconAlpha;
	iconImage->setDrawColor (true, normalIconColor);
	reflow ();
}

Button::Button (Sprite *iconSprite, const Color &iconColor)
: Button ()
{
	normalIconColor.assign (iconColor);
	iconImage = add (new Image (iconSprite, Button::NormalFrame));
	iconImage->drawAlpha = UiConfiguration::instance->buttonUnfocusedIconAlpha;
	iconImage->setDrawColor (true, normalIconColor);
	reflow ();
}

Button::Button (const StdString &labelText)
: Button ()
{
	promptLabel = add (new Label (labelText, UiConfiguration::ButtonFont, normalTextColor));
	reflow ();
}

Button::Button (const StdString &labelText, const Color &labelColor)
: Button ()
{
	normalTextColor.assign (labelColor);
	promptLabel = add (new Label (labelText, UiConfiguration::ButtonFont, normalTextColor));
	reflow ();
}

Button::Button (Sprite *iconSprite, const StdString &labelText)
: Button ()
{
	iconImage = add (new Image (iconSprite, Button::NormalFrame));
	iconImage->drawAlpha = UiConfiguration::instance->buttonUnfocusedIconAlpha;
	iconImage->setDrawColor (true, normalIconColor);
	promptLabel = add (new Label (labelText, UiConfiguration::ButtonFont, normalTextColor));
	reflow ();
}

Button::Button (Sprite *iconSprite, const Color &iconColor, const StdString &labelText, const Color &labelColor)
: Button ()
{
	normalTextColor.assign (labelColor);
	normalIconColor.assign (iconColor);
	iconImage = add (new Image (iconSprite, Button::NormalFrame));
	iconImage->drawAlpha = UiConfiguration::instance->buttonUnfocusedIconAlpha;
	iconImage->setDrawColor (true, normalIconColor);
	promptLabel = add (new Label (labelText, UiConfiguration::ButtonFont, normalTextColor));
	reflow ();
}

Button::~Button () {
}

Button *Button::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::Button) ? (Button *) widget : NULL);
}

StdString Button::getText () const {
	if (! promptLabel) {
		return (StdString ());
	}
	return (promptLabel->text);
}

void Button::setButtonWidth (double widthValue) {
	buttonWidth = widthValue;
	reflow ();
}

void Button::setPressed (bool pressed) {
	if (pressed == isPressed) {
		return;
	}
	isPressed = pressed;
	if (isPressed) {
		setFocused (false);
	}
	reflow ();
}

void Button::setDisabled (bool disabled, const StdString &tooltipTextValue) {
	if (! tooltipTextValue.empty ()) {
		tooltipText.assign (tooltipTextValue);
	}
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFocused (false);
		isInputSuspended = true;
	}
	else {
		isInputSuspended = false;
	}
	reflow ();
}

void Button::setText (const StdString &text) {
	if (text.empty ()) {
		if (promptLabel) {
			promptLabel->isDestroyed = true;
			promptLabel = NULL;
			reflow ();
		}
	}
	else {
		if (! promptLabel) {
			promptLabel = add (new Label (text, UiConfiguration::ButtonFont, normalTextColor));
		}
		promptLabel->setText (text);
		reflow ();
	}
}

void Button::setTextColor (const Color &textColor) {
	normalTextColor.assign (textColor);
	if (promptLabel) {
		promptLabel->textColor.assign (normalTextColor);
	}
}

void Button::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	if (isInverseColor) {
		normalTextColor.assign (UiConfiguration::instance->inverseTextColor);
		normalIconColor.assign (UiConfiguration::instance->inverseTextColor);
	}
	else {
		normalTextColor.assign (UiConfiguration::instance->buttonTextColor);
		normalIconColor.assign (UiConfiguration::instance->buttonTextColor);
	}
	if (promptLabel) {
		promptLabel->textColor.assign (normalTextColor);
	}
	reflow ();
}

void Button::setFocused (bool focused) {
	if (focused == isFocused) {
		return;
	}
	isFocused = focused;
	reflow ();
	if (isFocused) {
		eventCallback (focusCallback);
	}
	else {
		eventCallback (unfocusCallback);
	}
}

void Button::setImageColor (const Color &imageColor) {
	normalIconColor.assign (imageColor);
	if (iconImage) {
		iconImage->setDrawColor (true, imageColor);
	}
	reflow ();
}

void Button::setTransparentBackground (double shadeAlpha) {
	bodyType = TransparentBody;
	bodyColor.assign (0.0f, 0.0f, 0.0f, shadeAlpha);
	reflow ();
}

void Button::setColorBackground (const Color &bgColor) {
	bodyType = ColorBody;
	bodyColor.assign (bgColor);
	reflow ();
}

void Button::setGradientBackground (const Color &bgColor) {
	bodyType = GradientBody;
	bodyColor.assign (bgColor);
	reflow ();
}

bool Button::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isDisabled || isPressed) {
		return (false);
	}
	if ((shortcutKey != SDLK_UNKNOWN) && (keycode == shortcutKey) && (! isShiftDown)) {
		if ((isShortcutKeyControlPress && isControlDown) || ((! isShortcutKeyControlPress) && (! isControlDown))) {
			setPressed (true);
			pressClock = UiConfiguration::instance->blinkDuration;
			reflow ();
			mouseClick ();
			return (true);
		}
	}
	return (false);
}

void Button::doResetInputState () {
	Panel::doResetInputState ();
	setPressed (false);
	setFocused (false);
	reflow ();
}

void Button::mouseEntered (void *itPtr, Widget *widgetPtr) {
	Button *it = (Button *) itPtr;

	if (it->isDisabled) {
		return;
	}
	it->setFocused (true);
}

void Button::mouseExited (void *itPtr, Widget *widgetPtr) {
	Button *it = (Button *) itPtr;

	if (it->isDisabled) {
		return;
	}
	it->setFocused (false);
}

void Button::mousePressed (void *itPtr, Widget *widgetPtr) {
	Button *it = (Button *) itPtr;

	if (it->isDisabled) {
		return;
	}
	it->setPressed (true);
}

void Button::mouseReleased (void *itPtr, Widget *widgetPtr) {
	Button *it = (Button *) itPtr;

	if (it->isDisabled) {
		return;
	}
	it->setPressed (false);
	it->setFocused (it->isMouseEntered);
}

void Button::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (pressClock > 0) {
		pressClock -= msElapsed;
		if (pressClock <= 0) {
			pressClock = 0;
		}
	}

	if (! isInputSuspended) {
		if (isPressed && (pressClock <= 0)) {
			if ((! isMouseEntered) || (! Input::instance->isMouseLeftButtonDown)) {
				setPressed (false);
				if (isMouseEntered) {
					setFocused (true);
				}
			}
		}
		if (isFocused) {
			if (! isMouseEntered) {
				setFocused (false);
			}
		}
	}
}

void Button::doDraw (double originX, double originY) {
	SDL_Renderer *render;
	SDL_Rect rect;
	SDL_Texture *endtexture, *middletexture;
	int x0, y0, endtexturew, endtextureh, middletexturew, middletextureh;

	if (bodyType == GradientBody) {
		render = App::instance->render;
		x0 = (int) (originX + position.x);
		y0 = (int) (originY + position.y);

		middletexture = NULL;
		endtexture = UiConfiguration::instance->buttonGradientEndSprite->getScaleTexture (height, &endtexturew, &endtextureh);
		if (endtexture) {
			middletexture = UiConfiguration::instance->buttonGradientMiddleSprite->getScaleTexture (height, &middletexturew, &middletextureh);
		}
		if (endtexture && middletexture) {
			rect.x = x0;
			rect.y = y0;
			rect.w = (int) width;
			rect.h = (int) height;
			App::instance->pushClipRect (rect);

			rect.w = endtexturew;
			rect.h = endtextureh;
			SDL_SetTextureColorMod (endtexture, gradientColor.rByte, gradientColor.gByte, gradientColor.bByte);
			SDL_SetTextureBlendMode (endtexture, SDL_BLENDMODE_BLEND);
			SDL_RenderCopy (render, endtexture, NULL, &rect);

			rect.x += endtexturew;
			rect.w = middletexturew - endtexturew;
			rect.h = middletextureh;
			SDL_SetTextureColorMod (middletexture, gradientColor.rByte, gradientColor.gByte, gradientColor.bByte);
			SDL_SetTextureBlendMode (middletexture, SDL_BLENDMODE_BLEND);
			SDL_RenderCopy (render, middletexture, NULL, &rect);

			rect.x = x0 + (int) width - endtexturew;
			rect.w = endtexturew;
			rect.h = endtextureh;
			SDL_RenderCopyEx (render, endtexture, NULL, &rect, 0.0f, NULL, SDL_FLIP_HORIZONTAL);

			App::instance->popClipRect ();
		}
	}

	Panel::doDraw (originX, originY);
}

void Button::reflow () {
	double x, y, h, spacew, paddingh, dropshadow, shadealpha, promptdx;
	Color bordercolor, shadecolor, bgcolor;

	if (iconImage) {
		maxImageWidth = iconImage->maxSpriteWidth;
		maxImageHeight = iconImage->maxSpriteHeight;
	}
	else {
		maxImageWidth = 0.0f;
		maxImageHeight = 0.0f;
	}
	resetPadding ();
	dropshadow = 0.0f;
	promptdx = 0.0f;
	constexpr const double normalShade = 0.0f;
	constexpr const double inverseShade = 0.89f;

	switch (bodyType) {
		case ColorBody: {
			shadecolor.assign (isInverseColor ? Color (inverseShade, inverseShade, inverseShade) : Color (normalShade, normalShade, normalShade));
			bgcolor.assign (bodyColor);
			if (isDisabled) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonDisabledShadeAlpha);
			}
			else if (isFocused) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonFocusedShadeAlpha);
			}
			else if (isPressed) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonPressedShadeAlpha);
			}
			else {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonUnfocusedShadeAlpha);
			}
			setFillBg (true, bgcolor);
			dropshadow = isFocused ? UiConfiguration::instance->buttonFocusedDropShadowWidth : UiConfiguration::instance->buttonUnfocusedDropShadowWidth;
			break;
		}
		case TransparentBody: {
			shadealpha = bodyColor.a;
			if (isDisabled) {
				shadealpha -= UiConfiguration::instance->buttonDisabledShadeAlpha;
			}
			else if (isFocused) {
				shadealpha -= UiConfiguration::instance->buttonFocusedShadeAlpha;
			}
			else if (isPressed) {
				shadealpha -= UiConfiguration::instance->buttonPressedShadeAlpha;
			}
			if (shadealpha < 0.01f) {
				shadealpha = 0.01f;
			}
			if (shadealpha > 1.0f) {
				shadealpha = 1.0f;
			}
			shadecolor.assign (isInverseColor ? Color (inverseShade, inverseShade, inverseShade, shadealpha) : Color (normalShade, normalShade, normalShade, shadealpha));
			setFillBg (true, shadecolor);
			dropshadow = isFocused ? UiConfiguration::instance->buttonFocusedDropShadowWidth : UiConfiguration::instance->buttonUnfocusedDropShadowWidth;
			break;
		}
		case GradientBody: {
			shadecolor.assign (isInverseColor ? Color (inverseShade, inverseShade, inverseShade) : Color (normalShade, normalShade, normalShade));
			bgcolor.assign (bodyColor);
			if (isDisabled) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonDisabledShadeAlpha);
			}
			else if (isFocused) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonFocusedShadeAlpha);
			}
			else if (isPressed) {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonPressedShadeAlpha);
			}
			else {
				bgcolor.blend (shadecolor, UiConfiguration::instance->buttonUnfocusedShadeAlpha);
			}
			gradientColor.assign (bgcolor);
			setFillBg (false);
			dropshadow = isFocused ? UiConfiguration::instance->buttonFocusedDropShadowWidth : UiConfiguration::instance->buttonUnfocusedDropShadowWidth;
			break;
		}
		default: {
			if (isFocused) {
				shadealpha = isInverseColor ? UiConfiguration::instance->buttonInverseFocusedShadeAlpha : UiConfiguration::instance->buttonFocusedShadeAlpha;
				shadecolor.assign (isInverseColor ? Color (inverseShade, inverseShade, inverseShade, shadealpha) : Color (normalShade, normalShade, normalShade, shadealpha));
				setFillBg (true, shadecolor);
				dropshadow = UiConfiguration::instance->buttonFocusedDropShadowWidth;
			}
			else if (isPressed) {
				shadealpha = isInverseColor ? UiConfiguration::instance->buttonInversePressedShadeAlpha : UiConfiguration::instance->buttonPressedShadeAlpha;
				shadecolor.assign (isInverseColor ? Color (inverseShade, inverseShade, inverseShade, shadealpha) : Color (normalShade, normalShade, normalShade, shadealpha));
				setFillBg (true, shadecolor);
				dropshadow = UiConfiguration::instance->buttonUnfocusedDropShadowWidth;
			}
			else {
				setFillBg (false);
			}
			break;
		}
	}

	if (iconImage) {
		if (isDisabled) {
			iconImage->setFrame (Button::NormalFrame);
			iconImage->drawAlpha = UiConfiguration::instance->buttonDisabledIconAlpha;
			if (isInverseColor) {
				iconImage->setDrawColor (true, UiConfiguration::instance->darkInverseTextColor);
			}
			else {
				iconImage->setDrawColor (true, UiConfiguration::instance->lightPrimaryTextColor);
			}
		}
		else if (isFocused) {
			iconImage->setFrame (Button::LargeFrame);
			iconImage->drawAlpha = UiConfiguration::instance->buttonFocusedIconAlpha;
			iconImage->setDrawColor (true, normalIconColor);
		}
		else if (isPressed) {
			iconImage->setFrame (Button::NormalFrame);
			iconImage->drawAlpha = UiConfiguration::instance->buttonFocusedIconAlpha;
			iconImage->setDrawColor (true, normalIconColor);
		}
		else {
			iconImage->setFrame (Button::NormalFrame);
			iconImage->drawAlpha = UiConfiguration::instance->buttonUnfocusedIconAlpha;
			iconImage->setDrawColor (true, normalIconColor);
		}
	}
	if (isFocused) {
		promptdx = -(focusTextOffset);
	}
	if ((dropshadow > 0.0f) && (! isDropShadowDisabled)) {
		setDropShadow (true, UiConfiguration::instance->dropShadowColor, dropshadow);
	}
	else {
		setDropShadow (false);
	}

	if (promptLabel) {
		if (isDisabled) {
			if (isInverseColor) {
				promptLabel->textColor.assign (UiConfiguration::instance->darkInverseTextColor);
			}
			else {
				promptLabel->textColor.assign (UiConfiguration::instance->lightPrimaryTextColor);
			}
		}
		else {
			promptLabel->textColor.assign (normalTextColor);
		}
	}

	paddingh = (heightPadding * 2.0f);
	if (iconImage && (! promptLabel)) {
		paddingh /= 2.0f;
	}
	x = widthPadding;
	y = paddingh / 2.0f;
	spacew = 0.0f;
	h = 0.0f;
	if (iconImage) {
		x += spacew;
		iconImage->position.assign (x, y);
		x += maxImageWidth;
		spacew = UiConfiguration::instance->marginSize;
		if (maxImageHeight > h) {
			h = maxImageHeight;
		}
	}
	if (promptLabel) {
		x += spacew;
		promptLabel->position.assign (x + promptdx, promptLabel->getLinePosition (y));
		x += promptLabel->width;
		spacew = UiConfiguration::instance->marginSize;
		if (promptLabel->maxLineHeight > h) {
			h = promptLabel->maxLineHeight;
		}
	}

	setFixedSize (true, (buttonWidth > 0.0f) ? buttonWidth : x + widthPadding, h + paddingh);
	if (iconImage) {
		iconImage->position.assign (iconImage->position.x + ((maxImageWidth - iconImage->width) / 2.0f), (height / 2.0f) - (iconImage->height / 2.0f));
	}
	if (promptLabel) {
		promptLabel->position.assignY ((height / 2.0f) - (promptLabel->height / 2.0f));
	}
}
