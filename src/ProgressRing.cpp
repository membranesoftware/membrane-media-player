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
#include "ProgressRingSprite.h"
#include "ProgressRing.h"

constexpr const double animationFactor = 10.0f; // milliseconds per percentage point

ProgressRing::ProgressRing (double ringWidth)
: ProgressBar (ringWidth, ringWidth)
{
	classId = ClassId::ProgressRing;
}
ProgressRing::~ProgressRing () {
}

ProgressRing *ProgressRing::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ProgressRing) ? (ProgressRing *) widget : NULL);
}

void ProgressRing::setSize (double ringWidth) {
	if (FLOAT_EQUALS (width, ringWidth)) {
		return;
	}
	width = ringWidth;
	height = ringWidth;
	resetFill ();
}

void ProgressRing::doUpdate (int msElapsed) {
	if (isIndeterminate) {
		switch (fillStage) {
			case 0: {
				fillEnd = 0.0f;
				fillEndTarget = 100.0f;
				fillStage = 1;
				break;
			}
			case 1: {
				fillEnd += ((double) msElapsed) / animationFactor;
				if (fillEnd >= fillEndTarget) {
					fillEnd = fillEndTarget;
					fillStage = 2;
				}
				break;
			}
			case 2: {
				fillEnd = 0.0f;
				fillEndTarget = 100.0f;
				fillStage = 3;
				break;
			}
			case 3: {
				fillEnd += ((double) msElapsed) / animationFactor;
				if (fillEnd >= fillEndTarget) {
					fillEnd = fillEndTarget;
					fillStage = 0;
				}
				break;
			}
		}
	}
	fillColor.update (msElapsed);
}

void ProgressRing::doDraw (double originX, double originY) {
	SDL_Texture *bgtexture, *filltexture;
	SDL_Rect rect;

	bgtexture = UiConfiguration::instance->progressRingSprite->getRingTexture (100.0f);
	if (! bgtexture) {
		return;
	}
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;

	if (isIndeterminate) {
		if (fillStage <= 1) {
			SDL_SetTextureColorMod (bgtexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
		}
		else {
			SDL_SetTextureColorMod (bgtexture, fillColor.rByte, fillColor.gByte, fillColor.bByte);
		}
		SDL_RenderCopy (App::instance->render, bgtexture, NULL, &rect);
		if (fillEnd > 0.0f) {
			filltexture = UiConfiguration::instance->progressRingSprite->getRingTexture (fillEnd);
			if (filltexture) {
				if (fillStage <= 1) {
					SDL_SetTextureColorMod (filltexture, fillColor.rByte, fillColor.gByte, fillColor.bByte);
				}
				else {
					SDL_SetTextureColorMod (filltexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
				}
				SDL_RenderCopy (App::instance->render, filltexture, NULL, &rect);
			}
		}
	}
	else {
		SDL_SetTextureColorMod (bgtexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
		SDL_RenderCopy (App::instance->render, bgtexture, NULL, &rect);
		if (progressValue > 0.0f) {
			filltexture = UiConfiguration::instance->progressRingSprite->getRingTexture ((progressValue / targetProgressValue) * 100.0f);
			if (filltexture) {
				SDL_SetTextureColorMod (filltexture, fillColor.rByte, fillColor.gByte, fillColor.bByte);
				SDL_RenderCopy (App::instance->render, filltexture, NULL, &rect);
			}
		}
	}
}