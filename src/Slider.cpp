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
#include "Input.h"
#include "UiConfiguration.h"
#include "RenderResource.h"
#include "SliderThumbSprite.h"
#include "Slider.h"

Slider::Slider (double minValue, double maxValue)
: Widget ()
, isDisabled (false)
, isInverseColor (false)
, isVerticalTrack (false)
, thumbSize (0.0f)
, thumbWidth (0.0f)
, thumbHeight (0.0f)
, trackWidthScale (1.0f)
, value (minValue)
, hoverValue (0.0f)
, minValue (minValue)
, maxValue (maxValue)
, isHovering (false)
, isDragging (false)
, trackWidth (0.0f)
, trackHeight (0.0f)
, hoverSize (0.0f)
{
	if (maxValue < minValue) {
		maxValue = minValue;
	}
	thumbSize = UiConfiguration::instance->sliderThumbSize;
	thumbColor.assign (UiConfiguration::instance->lightPrimaryColor);
	trackWidth = UiConfiguration::instance->sliderTrackWidth;
	trackHeight = UiConfiguration::instance->sliderTrackHeight;
	trackColor.assign (UiConfiguration::instance->darkPrimaryColor);
	hoverSize = UiConfiguration::instance->sliderThumbSize;
	hoverColor.assign (UiConfiguration::instance->lightPrimaryColor);
	resetSize ();
}
Slider::~Slider () {
}

void Slider::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	resetColors ();
}

void Slider::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	resetColors ();
}

void Slider::setVerticalTrack (bool vertical) {
	if (isVerticalTrack == vertical) {
		return;
	}
	isVerticalTrack = vertical;
	resetSize ();
}

void Slider::setTrackWidthScale (double scale) {
	if (scale <= 0.0f) {
		return;
	}
	trackWidthScale = scale;
	resetSize ();
}

double Slider::getSnappedValue (double targetValue) {
	std::list<double>::iterator i1, i2;
	double dist, mindist, val;

	if (targetValue < minValue) {
		targetValue = minValue;
	}
	if (targetValue > maxValue) {
		targetValue = maxValue;
	}
	if (! snapValueList.empty ()) {
		mindist = -1.0f;
		val = 0.0f;
		i1 = snapValueList.begin ();
		i2 = snapValueList.end ();
		while (i1 != i2) {
			dist = fabs (targetValue - *i1);
			if ((mindist < 0.0f) || (dist < mindist)) {
				mindist = dist;
				val = *i1;
			}
			++i1;
		}
		targetValue = val;
	}
	return (targetValue);
}

void Slider::setValue (double sliderValue, bool shouldSkipChangeCallback) {
	sliderValue = getSnappedValue (sliderValue);
	if (FLOAT_EQUALS (value, sliderValue)) {
		return;
	}
	value = sliderValue;
	if (! shouldSkipChangeCallback) {
		eventCallback (valueChangeCallback);
	}
}

void Slider::addSnapValue (double snapValue) {
	std::list<double>::iterator i1, i2, pos;

	if (snapValue < minValue) {
		snapValue = minValue;
	}
	if (snapValue > maxValue) {
		snapValue = maxValue;
	}
	i1 = snapValueList.begin ();
	i2 = snapValueList.end ();
	pos = i2;
	while (i1 != i2) {
		if (snapValue <= *i1) {
			pos = i1;
			break;
		}
		++i1;
	}

	if (pos == i2) {
		snapValueList.push_back (snapValue);
	}
	else {
		snapValueList.insert (pos, snapValue);
	}
}

void Slider::doUpdate (int msElapsed) {
	thumbColor.update (msElapsed);
	trackColor.update (msElapsed);
	hoverColor.update (msElapsed);
}

void Slider::doDraw (double originX, double originY) {
	SDL_Renderer *render;
	SDL_Rect rect;
	SDL_Texture *texture;
	int x0, y0, texturew, textureh;
	double w, h;

	render = App::instance->render;
	x0 = (int) (originX + position.x);
	y0 = (int) (originY + position.y);
	h = (thumbHeight - trackHeight) / 2.0f;

	if (isVerticalTrack) {
		rect.x = (int) (x0 + h);
		rect.y = y0;
		rect.w = (int) trackHeight;
		rect.h = (int) trackWidth;
	}
	else {
		rect.x = x0;
		rect.y = (int) (y0 + h);
		rect.w = (int) trackWidth;
		rect.h = (int) trackHeight;
	}
	SDL_SetRenderDrawColor (render, trackColor.rByte, trackColor.gByte, trackColor.bByte, 255);
	SDL_RenderFillRect (render, &rect);

	if (isHovering && (! isDragging) && (! FLOAT_EQUALS (hoverValue, value))) {
		w = trackWidth - hoverSize;
		if (isVerticalTrack) {
			rect.x = (int) (x0 + h);
			rect.y = y0;
			if (maxValue > minValue) {
				rect.y += (int) (trackWidth - (w * ((hoverValue - minValue) / (maxValue - minValue))) - hoverSize);
			}
			rect.w = (int) trackHeight;
			rect.h = (int) hoverSize;
		}
		else {
			rect.x = x0;
			rect.y = (int) (y0 + h);
			if (maxValue > minValue) {
				rect.x += (int) (w * ((hoverValue - minValue) / (maxValue - minValue)));
			}
			rect.w = (int) hoverSize;
			rect.h = (int) trackHeight;
		}
		SDL_SetRenderDrawColor (render, hoverColor.rByte, hoverColor.gByte, hoverColor.bByte, 255);
		SDL_RenderFillRect (render, &rect);
	}

	rect.x = x0;
	rect.y = y0;
	texture = RenderResource::instance->sliderThumbSprite->getScaleTexture (thumbSize, &texturew, &textureh);
	if (texture) {
		w = trackWidth - texturew;
		if (isVerticalTrack) {
			if (maxValue > minValue) {
				rect.y += (int) (trackWidth - (w * ((value - minValue) / (maxValue - minValue))) - (textureh / 2));
			}
			rect.w = textureh;
			rect.h = texturew;
		}
		else {
			if (maxValue > minValue) {
				rect.x += (int) (w * ((value - minValue) / (maxValue - minValue)));
			}
			rect.w = texturew;
			rect.h = textureh;
		}
		SDL_SetTextureColorMod (texture, thumbColor.rByte, thumbColor.gByte, thumbColor.bByte);
		SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_BLEND);
		SDL_RenderCopy (render, texture, NULL, &rect);
	}
	else {
		w = trackWidth - thumbWidth;
		if (isVerticalTrack) {
			if (maxValue > minValue) {
				rect.y += (int) (trackWidth - (w * ((value - minValue) / (maxValue - minValue))) - (thumbHeight / 2.0f));
			}
			rect.w = (int) thumbHeight;
			rect.h = (int) thumbWidth;
		}
		else {
			if (maxValue > minValue) {
				rect.x += (int) (w * ((value - minValue) / (maxValue - minValue)));
			}
			rect.w = (int) thumbWidth;
			rect.h = (int) thumbHeight;
		}
		SDL_SetRenderDrawColor (render, thumbColor.rByte, thumbColor.gByte, thumbColor.bByte, 255);
		SDL_RenderFillRect (render, &rect);
	}

	SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
}

bool Slider::doProcessMouseState (const Widget::MouseState &mouseState) {
	double val, dx;
	bool firsthover;

	if (isDisabled) {
		return (false);
	}
	firsthover = false;
	if (mouseState.isEntered) {
		if (isDragging) {
			if (! Input::instance->isMouseLeftButtonDown) {
				isDragging = false;
			}
		}
		else {
			if (mouseState.isLeftClicked) {
				isDragging = true;
			}
		}

		if (! isHovering) {
			if (! isDragging) {
				isHovering = true;
				firsthover = true;
			}
		}
	}
	else {
		if (isDragging) {
			if (! Input::instance->isMouseLeftButtonDown) {
				isDragging = false;
			}
		}
		if (isHovering) {
			isHovering = false;
			eventCallback (valueHoverCallback);
		}
	}

	if (isDragging || isHovering) {
		if (isVerticalTrack) {
			dx = trackWidth - (((double) Input::instance->mouseY) - screenY);
		}
		else {
			dx = ((double) Input::instance->mouseX) - screenX;
		}

		if (dx < 0.0f) {
			dx = 0.0f;
		}
		if (dx > trackWidth) {
			dx = trackWidth;
		}
		val = (dx / trackWidth);
		val *= (maxValue - minValue);
		val += minValue;

		if (isDragging) {
			setValue (val);
		}
		else {
			if (firsthover || (! FLOAT_EQUALS (val, hoverValue))) {
				hoverValue = getSnappedValue (val);
				eventCallback (valueHoverCallback);
			}
		}
	}

	return (false);
}

void Slider::doResize () {
	resetSize ();
}

void Slider::doResetInputState () {
	isDragging = false;
	isHovering = false;
}

void Slider::resetSize () {
	int texturew, textureh;

	thumbSize = UiConfiguration::instance->sliderThumbSize;
	trackWidth = UiConfiguration::instance->sliderTrackWidth * trackWidthScale;
	trackHeight = UiConfiguration::instance->sliderTrackHeight;
	hoverSize = UiConfiguration::instance->sliderThumbSize;

	if (RenderResource::instance->sliderThumbSprite->getScaleTexture (thumbSize, &texturew, &textureh)) {
		thumbWidth = (double) texturew;
		thumbHeight = (double) textureh;
	}
	else {
		thumbWidth = thumbSize;
		thumbHeight = thumbSize;
	}

	if (isVerticalTrack) {
		width = thumbHeight;
		height = trackWidth;
	}
	else {
		width = trackWidth;
		height = thumbHeight;
	}
}

void Slider::resetColors () {
	Color color;

	color.assign (isInverseColor ? UiConfiguration::instance->darkBackgroundColor : UiConfiguration::instance->lightPrimaryColor);
	if (isDisabled) {
		color.blend (0.0f, 0.0f, 0.0f, (1.0f - UiConfiguration::instance->buttonDisabledShadeAlpha));
	}
	thumbColor.translate (color, UiConfiguration::instance->shortColorTranslateDuration);
	hoverColor.translate (color, UiConfiguration::instance->shortColorTranslateDuration);

	color.assign (isInverseColor ? UiConfiguration::instance->darkInverseBackgroundColor : UiConfiguration::instance->darkPrimaryColor);
	if (isDisabled) {
		color.blend (0.5f, 0.5f, 0.5f, (1.0f - UiConfiguration::instance->buttonDisabledShadeAlpha));
	}
	trackColor.translate (color, UiConfiguration::instance->shortColorTranslateDuration);
}
