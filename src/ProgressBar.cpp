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
#include "UiConfiguration.h"
#include "ProgressBar.h"

constexpr const double animationFactor = 2.0f; // milliseconds per pixel

ProgressBar::ProgressBar (double barWidth, double barHeight)
: Widget ()
, isIndeterminate (false)
, progressValue (0.0f)
, targetProgressValue (ProgressBar::defaultProgressTarget)
, fillStage (0)
, fillStart (0.0f)
, fillStartTarget (0.0f)
, fillEnd (0.0f)
, fillEndTarget (0.0f)
{
	classId = ClassId::ProgressBar;
	width = barWidth;
	height = barHeight;
	bgColor.assign (UiConfiguration::instance->lightPrimaryColor);
	fillColor.animate (UiConfiguration::instance->darkPrimaryColor, UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->longColorAnimateDuration, UiConfiguration::instance->longColorAnimateDuration * 2);
}
ProgressBar::~ProgressBar () {
}

ProgressBar *ProgressBar::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ProgressBar) ? (ProgressBar *) widget : NULL);
}

void ProgressBar::setSize (double barWidth, double barHeight) {
	if (FLOAT_EQUALS (width, barWidth) && FLOAT_EQUALS (height, barHeight)) {
		return;
	}
	width = barWidth;
	height = barHeight;
	resetFill ();
}

void ProgressBar::setProgress (double value) {
	if (FLOAT_EQUALS (progressValue, value)) {
		return;
	}
	if (value < 0.0f) {
		value = 0.0f;
	}
	progressValue = value;
	resetFill ();
}

void ProgressBar::setProgress (double value, double targetValue) {
	if (FLOAT_EQUALS (progressValue, value) && FLOAT_EQUALS (targetProgressValue, targetValue)) {
		return;
	}
	if (value < 0.0f) {
		value = 0.0f;
	}
	if (targetValue < 1.0f) {
		targetValue = 1.0f;
	}
	progressValue = value;
	targetProgressValue = targetValue;
	resetFill ();
}

void ProgressBar::setIndeterminate (bool indeterminate) {
	if (indeterminate == isIndeterminate) {
		return;
	}
	isIndeterminate = indeterminate;
	if (isIndeterminate) {
		fillStage = 0;
	}
	resetFill ();
}

void ProgressBar::doUpdate (int msElapsed) {
	if (isIndeterminate) {
		switch (fillStage) {
			case 0: {
				fillStart = 0.0f;
				fillStartTarget = 0.0f;
				fillEnd = 0.0f;
				fillEndTarget = (width / 2.0f);
				fillStage = 1;
				break;
			}
			case 1: {
				if (fillEnd < fillEndTarget) {
					fillEnd += ((double) msElapsed) / animationFactor;
					if (fillEnd >= fillEndTarget) {
						fillEnd = fillEndTarget;
					}
				}
				if (fillEnd >= fillEndTarget) {
					fillStartTarget = (width / 2.0f);
					fillEndTarget = width;
					fillStage = 2;
				}
				break;
			}
			case 2: {
				if (fillEnd < fillEndTarget) {
					fillEnd += ((double) msElapsed) / animationFactor;
					if (fillEnd >= fillEndTarget) {
						fillEnd = fillEndTarget;
					}
				}
				if (fillStart < fillStartTarget) {
					fillStart += ((double) msElapsed) / animationFactor;
					if (fillStart >= fillStartTarget) {
						fillStart = fillStartTarget;
					}
				}
				if ((fillEnd >= fillEndTarget) && (fillStart >= fillStartTarget)) {
					fillEnd = width;
					fillStartTarget = width;
					fillStage = 3;
				}
				break;
			}
			case 3: {
				if (fillStart < fillStartTarget) {
					fillStart += ((double) msElapsed) / animationFactor;
					if (fillStart >= fillStartTarget) {
						fillStart = fillStartTarget;
					}
				}
				if (fillStart >= fillStartTarget) {
					fillStage = 4;
				}
				break;
			}
			default: {
				fillStage = 0;
				break;
			}
		}
	}

	fillColor.update (msElapsed);
}

void ProgressBar::doDraw (double originX, double originY) {
	SDL_Renderer *render;
	SDL_Rect rect;
	double x1, x2, w;

	render = App::instance->render;
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;
	SDL_SetRenderDrawColor (render, bgColor.rByte, bgColor.gByte, bgColor.bByte, 255);
	SDL_RenderFillRect (render, &rect);

	x1 = floor (fillStart);
	x2 = floor (fillEnd);
	w = x2 - x1;
	if (w > 0.0f) {
		rect.x = (int) (originX + position.x + x1);
		rect.w = (int) w;
		SDL_SetRenderDrawColor (render, fillColor.rByte, fillColor.gByte, fillColor.bByte, 255);
		SDL_RenderFillRect (render, &rect);
	}

	SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor (render, 0, 0, 0, 128);
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = 1;
	SDL_RenderFillRect (render, &rect);
	++(rect.y);
	rect.w = 1;
	rect.h = ((int) height) - 1;
	SDL_RenderFillRect (render, &rect);
	++(rect.x);
	rect.y = (int) (originY + position.y) + ((int) height) - 1;
	rect.w = ((int) width) - 1;
	rect.h = 1;
	SDL_RenderFillRect (render, &rect);
	rect.x = (int) (originX + position.x) + ((int) width) - 1;
	rect.y = (int) (originY + position.y + 1);
	rect.w = 1;
	rect.h = ((int) height) - 2;
	SDL_RenderFillRect (render, &rect);
	SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
}

void ProgressBar::resetFill () {
	if ((! isIndeterminate) && (targetProgressValue > 0.0f)) {
		fillStart = 0.0f;
		fillEnd = (width * progressValue) / targetProgressValue;
		if (fillEnd > width) {
			fillEnd = width;
		}
	}
}
