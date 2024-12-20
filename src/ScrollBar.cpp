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
#include "SpriteId.h"
#include "Input.h"
#include "Color.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "ScrollBar.h"

ScrollBar::ScrollBar (double maxScrollTrackLength)
: Panel ()
, scrollPosition (0.0f)
, maxScrollPosition (0.0f)
, maxTrackLength (maxScrollTrackLength)
, trackLength (0.0f)
, trackWidth (0.0f)
, isFollowingMouse (false)
{
	double w, h;

	setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));

	upArrowImage = (Image *) addWidget (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_scrollUpArrow)));
	upArrowImage->zLevel = 1;
	upArrowImage->isInputSuspended = true;
	w = upArrowImage->width;
	h = upArrowImage->height;

	downArrowImage = (Image *) addWidget (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_scrollDownArrow)));
	downArrowImage->zLevel = 1;
	downArrowImage->isInputSuspended = true;
	if (downArrowImage->width > w) {
		w = downArrowImage->width;
	}
	if (downArrowImage->height > h) {
		h = downArrowImage->height;
	}

	trackWidth = w;
	arrowPanel = add (new Panel ());
	arrowPanel->setFillBg (true, UiConfiguration::instance->mediumPrimaryColor);
	arrowPanel->setFixedSize (true, trackWidth, upArrowImage->height + downArrowImage->height + (h * 0.25f));
	if (maxTrackLength < arrowPanel->height) {
		maxTrackLength = arrowPanel->height;
	}

	reflow ();
}
ScrollBar::~ScrollBar () {
}

StdString ScrollBar::toStringDetail () {
	StdString s;
	return (s);
}

void ScrollBar::setPosition (double positionValue, bool shouldSkipCallback) {
	double pos;

	pos = positionValue;
	if (pos < 0.0f) {
		pos = 0.0f;
	}
	if (pos > maxScrollPosition) {
		pos = maxScrollPosition;
	}
	if (FLOAT_EQUALS (scrollPosition, pos)) {
		return;
	}
	scrollPosition = pos;
	reflow ();
	if (! shouldSkipCallback) {
		eventCallback (positionChangeCallback);
	}
}

void ScrollBar::setScrollBounds (double scrollViewHeight, double scrollAreaHeight) {
	double ratio;

	if ((scrollViewHeight <= 0.0f) || (scrollAreaHeight <= 0.0f)) {
		return;
	}
	ratio = (scrollAreaHeight / scrollViewHeight);
	trackLength = (1.0f + ratio) * arrowPanel->height;
	if (trackLength > maxTrackLength) {
		trackLength = maxTrackLength;
	}

	maxScrollPosition = scrollAreaHeight - scrollViewHeight;
	if (maxScrollPosition < 0.0f) {
		maxScrollPosition = 0.0f;
	}
	if (scrollPosition > maxScrollPosition) {
		scrollPosition = maxScrollPosition;
	}

	reflow ();
}

void ScrollBar::setMaxTrackLength (double maxScrollTrackLength) {
	double len;

	len = maxScrollTrackLength;
	if (len < arrowPanel->height) {
		len = arrowPanel->height;
	}
	if (FLOAT_EQUALS (len, maxTrackLength)) {
		return;
	}
	maxTrackLength = len;
	reflow ();
}

void ScrollBar::reflow () {
	double y;

	setFixedSize (true, trackWidth, trackLength);
	if (maxScrollPosition <= 0.0f) {
		y = 0.0f;
	}
	else {
		y = scrollPosition / maxScrollPosition;
		y *= trackLength;
		y -= (arrowPanel->height / 2.0f);
		if (y < 0.0f) {
			y = 0.0f;
		}
		if (y > (height - arrowPanel->height)) {
			y = height - arrowPanel->height;
		}
	}
	arrowPanel->position.assign (0.0f, y);
	upArrowImage->position.assign (0.0f, arrowPanel->position.y);
	downArrowImage->position.assign (0.0f, arrowPanel->position.y + arrowPanel->height - downArrowImage->height);
}

void ScrollBar::doUpdate (int msElapsed) {
	double dy, pos;

	Panel::doUpdate (msElapsed);
	if (isFollowingMouse) {
		if (! Input::instance->isMouseLeftButtonDown) {
			isFollowingMouse = false;
		}
		else {
			dy = ((double) Input::instance->mouseY) - screenY;
			if (dy < 0.0f) {
				dy = 0.0f;
			}
			if (dy > trackLength) {
				dy = trackLength;
			}
			pos = (dy / trackLength);
			pos *= maxScrollPosition;
			setPosition (pos);
		}
	}
}

bool ScrollBar::doProcessMouseState (const Widget::MouseState &mouseState) {
	double pos, dy;

	if (! isFollowingMouse) {
		if (mouseState.isEntered && mouseState.isLeftClicked) {
			isFollowingMouse = true;
			dy = ((double) Input::instance->mouseY) - screenY;
			if (dy < 0.0f) {
				dy = 0.0f;
			}
			if (dy > trackLength) {
				dy = trackLength;
			}
			pos = (dy / trackLength);
			pos *= maxScrollPosition;
			setPosition (pos);
		}
	}
	else {
		if (! Input::instance->isMouseLeftButtonDown) {
			isFollowingMouse = false;
		}
	}

	if (mouseState.isEntered || isFollowingMouse) {
		arrowPanel->bgColor.translate (UiConfiguration::instance->mediumPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		arrowPanel->bgColor.translate (UiConfiguration::instance->lightPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}

	return (false);
}

void ScrollBar::doResize () {
	double w, h;

	Panel::doResize ();
	w = upArrowImage->width;
	h = upArrowImage->height;
	if (downArrowImage->width > w) {
		w = downArrowImage->width;
	}
	if (downArrowImage->height > h) {
		h = downArrowImage->height;
	}

	trackWidth = w;
	arrowPanel->setFixedSize (true, trackWidth, upArrowImage->height + downArrowImage->height + (h * 0.25f));
	if (maxTrackLength < arrowPanel->height) {
		maxTrackLength = arrowPanel->height;
	}
}

void ScrollBar::doResetInputState () {
	isFollowingMouse = false;
}
