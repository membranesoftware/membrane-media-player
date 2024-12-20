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
#include "Ui.h"
#include "UiConfiguration.h"
#include "ScrollView.h"

ScrollView::ScrollView ()
: Panel ()
, isKeyboardScrollEnabled (false)
, isMouseWheelScrollEnabled (false)
, isExitedMouseWheelScrollEnabled (false)
, verticalScrollSpeed (0.0f)
{
	isDrawClipEnabled = true;
}
ScrollView::~ScrollView () {
}

void ScrollView::setViewSize (double viewWidth, double viewHeight) {
	setFixedSize (true, viewWidth, viewHeight);
	setVerticalScrollSpeed (App::instance->drawableHeight * UiConfiguration::instance->mouseWheelScrollSpeed);
}

void ScrollView::setVerticalScrollSpeed (double speed) {
	verticalScrollSpeed = speed;
	if (verticalScrollSpeed < 1.0f) {
		verticalScrollSpeed = 1.0f;
	}
	if (verticalScrollSpeed > height) {
		verticalScrollSpeed = height;
	}
}

void ScrollView::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (originPosition.isTranslating) {
		originPosition.update (msElapsed);
		setViewOrigin (originPosition.x, originPosition.y);
	}
}

bool ScrollView::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed;
	double dy, delta, mx, my;

	consumed = Panel::doProcessMouseState (mouseState);
	if ((! consumed) && isMouseWheelScrollEnabled) {
		if (mouseState.isEntered || isExitedMouseWheelScrollEnabled) {
			consumed = true;
		}
		if ((! consumed) && hasScreenPosition && (width > 0.0f) && (height > 0.0f)) {
			mx = Input::instance->mouseX;
			my = Input::instance->mouseY;
			if ((mx >= (int) screenX) && (mx <= (int) (screenX + width)) && (my >= (int) screenY) && (my <= (int) (screenY + height))) {
				consumed = true;
			}
		}
		if (consumed) {
			delta = 0.0f;
			if (mouseState.wheelUp > 0) {
				dy = ((double) mouseState.wheelUp) * verticalScrollSpeed * -1.0f;
				if (dy > -1.0f) {
					dy = -1.0f;
				}
				delta += dy;
			}
			if (mouseState.wheelDown > 0) {
				dy = ((double) mouseState.wheelDown) * verticalScrollSpeed;
				if (dy < 1.0f) {
					dy = 1.0f;
				}
				delta += dy;
			}
			if (fabs (delta) > 0.0f) {
				setScrollOrigin (0.0f, viewOriginY + delta, UiConfiguration::instance->shortScrollPositionTranslateDuration);
			}
		}
	}
	return (consumed);
}

void ScrollView::setVerticalScrollBounds (double minY, double maxY) {
	setViewOriginBounds (0.0f, minY, 0.0f, maxY);
}

void ScrollView::setScrollOrigin (double positionX, double positionY, int translateDuration) {
	if (translateDuration <= 0) {
		originPosition.assign (positionX, positionY);
		setViewOrigin (positionX, positionY);
	}
	else {
		originPosition.translate (viewOriginX, viewOriginY, positionX, positionY, translateDuration);
	}
}

bool ScrollView::isScrolledToBottom (double marginHeight) {
	if (! isViewOriginBoundEnabled) {
		return (false);
	}
	return (viewOriginY >= (maxViewOriginY - marginHeight));
}

bool ScrollView::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (Panel::doProcessKeyEvent (keycode, isShiftDown, isControlDown)) {
		return (true);
	}
	if (isShiftDown || isControlDown) {
		return (false);
	}
	if (isKeyboardScrollEnabled) {
		switch (keycode) {
			case SDLK_UP: {
				setScrollOrigin (0.0f, viewOriginY - verticalScrollSpeed, UiConfiguration::instance->shortScrollPositionTranslateDuration);
				return (true);
			}
			case SDLK_DOWN: {
				setScrollOrigin (0.0f, viewOriginY + verticalScrollSpeed, UiConfiguration::instance->shortScrollPositionTranslateDuration);
				return (true);
			}
			case SDLK_HOME: {
				setScrollOrigin (0.0f, 0.0f, UiConfiguration::instance->longScrollPositionTranslateDuration);
				return (true);
			}
			case SDLK_END: {
				if (isViewOriginBoundEnabled) {
					setScrollOrigin (0.0f, maxViewOriginY, UiConfiguration::instance->longScrollPositionTranslateDuration);
					return (true);
				}
				break;
			}
			case SDLK_PAGEUP: {
				setScrollOrigin (0.0f, viewOriginY - height, UiConfiguration::instance->shortScrollPositionTranslateDuration);
				return (true);
			}
			case SDLK_PAGEDOWN: {
				setScrollOrigin (0.0f, viewOriginY + height, UiConfiguration::instance->shortScrollPositionTranslateDuration);
				return (true);
			}
		}
	}
	return (false);
}

void ScrollView::reflow () {
	Panel::reflow ();
	resetExtents ();
}
