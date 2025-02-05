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
#include "SdlUtil.h"
#include "ClassId.h"
#include "StringList.h"
#include "MathUtil.h"
#include "Resource.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Widget.h"

constexpr const int ScaleAnimation = 1;
constexpr const int ScaleEndComposeAnimation = 2;

Widget::Widget ()
: id (0)
, isDestroyed (false)
, isVisible (true)
, isInputSuspended (false)
, isPanelSizeClipEnabled (false)
, zLevel (0)
, isMouseHoverEnabled (false)
, mouseClickExecuteCount (0)
, shouldComposeRender (false)
, classId (-1)
, refcount (0)
, hasScreenPosition (false)
, screenX (0.0f)
, screenY (0.0f)
, isKeyFocused (false)
, isMouseEntered (false)
, isMousePressed (false)
, tooltipAlignment (Widget::BottomAlignment)
, isComposeDrawEnabled (false)
, isComposeRendering (false)
, isComposeAnimating (false)
, composeWidth (0.0f)
, composeHeight (0.0f)
, composeScale (1.0f)
, composeRotation (0.0f)
, width (0.0f)
, height (0.0f)
, detailStringFn (NULL)
, destroyClock (0)
, composeTexture (NULL)
, composeTextureWidth (0)
, composeTextureHeight (0)
, composeTargetWidth (0.0f)
, composeTargetHeight (0.0f)
, composeAnimationType (0)
{
	SdlUtil::createMutex (&refcountMutex);
}
Widget::~Widget () {
	if (! composeTexturePath.empty ()) {
		Resource::instance->unloadTexture (composeTexturePath);
		composeTexturePath.assign ("");
	}
	composeTexture = NULL;
	SdlUtil::destroyMutex (&refcountMutex);
}

void Widget::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}
void Widget::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);
	if (isdestroyed) {
		delete (this);
	}
}

bool Widget::isWidgetClass (Widget *widget, int classIdValue) {
	return (widget && (widget->classId == classIdValue));
}

void Widget::resetInputState () {
	isMouseEntered = false;
	isMousePressed = false;
	doResetInputState ();
}
void Widget::doResetInputState () {
	// Default implementation does nothing
}

void Widget::setMouseHoverTooltip (const StdString &text, Widget::Alignment alignment) {
	tooltipText.assign (text);
	tooltipAlignment = alignment;
	isMouseHoverEnabled = true;
}

void Widget::update (int msElapsed, double originX, double originY) {
	if (destroyClock > 0) {
		destroyClock -= msElapsed;
		if (destroyClock <= 0) {
			isDestroyed = true;
		}
	}
	if (isDestroyed) {
		return;
	}

	position.update (msElapsed);
	screenX = position.x + originX;
	screenY = position.y + originY;
	hasScreenPosition = true;

	while (mouseClickExecuteCount > 0) {
		mouseClick ();
		--mouseClickExecuteCount;
	}

	switch (composeAnimationType) {
		case ScaleAnimation: {
			composeAnimationScale.update (msElapsed);
			setComposeScale (composeAnimationScale.x);
			if (! composeAnimationScale.isTranslating) {
				composeAnimationType = 0;
			}
			break;
		}
		case ScaleEndComposeAnimation: {
			composeAnimationScale.update (msElapsed);
			setComposeScale (composeAnimationScale.x);
			if (! composeAnimationScale.isTranslating) {
				composeAnimationType = 0;
				setComposeDraw (false);
			}
			break;
		}
	}
	if (! isComposeAnimating) {
		if (composeAnimationType != 0) {
			isComposeAnimating = true;
		}
	}
	else {
		if (composeAnimationType == 0) {
			isComposeAnimating = false;
			eventCallback (composeAnimationCompleteCallback);
		}
	}

	doUpdate (msElapsed);

	if (isComposeDrawEnabled) {
		if (! isComposeRendering) {
			if (! shouldComposeRender) {
				if ((! composeTexture) || (! FLOAT_EQUALS (composeTargetWidth, width)) || (! FLOAT_EQUALS (composeTargetHeight, height))) {
					shouldComposeRender = true;
				}
			}
			if (shouldComposeRender) {
				shouldComposeRender = false;
				composeTargetWidth = width;
				composeTargetHeight = height;
				composeWidth = width * composeScale;
				composeHeight = height * composeScale;
				isComposeRendering = true;
				retain ();
				App::instance->addPredrawTask (Widget::renderComposeTexture, this);
			}
		}
	}
	else {
		if (! composeTexturePath.empty ()) {
			Resource::instance->unloadTexture (composeTexturePath);
			composeTexturePath.assign ("");
		}
		composeTexture = NULL;
	}

	if (updateCallback.callback) {
		updateCallback.callback (updateCallback.callbackData, msElapsed, this);
	}
}
void Widget::doUpdate (int msElapsed) {
	// Default implementation does nothing
}

void Widget::draw (double originX, double originY) {
	SDL_Rect rect;
	double w, h;

	if (isDestroyed) {
		return;
	}
	if (isComposeDrawEnabled && composeTexture) {
		w = ((double) composeTextureWidth) * composeScale;
		h = ((double) composeTextureHeight) * composeScale;
		rect.x = (int) (originX + position.x + (composeTargetWidth / 2.0f) - (w / 2.0f));
		rect.y = (int) (originY + position.y + (composeTargetHeight / 2.0f) - (h / 2.0f));
		rect.w = (int) w;
		rect.h = (int) h;
		SDL_RenderCopyEx (App::instance->render, composeTexture, NULL, &rect, composeRotation, NULL, SDL_FLIP_NONE);
	}
	else {
		doDraw (originX, originY);
	}
}
void Widget::doDraw (double originX, double originY) {
	// Default implementation does nothing
}

void Widget::resize () {
	if (isDestroyed) {
		return;
	}
	doResize ();
}
void Widget::doResize () {
	// Default implementation does nothing
}

StdString Widget::toString () {
	StdString s, detail;

	s.sprintf ("<#%llu.%i", (unsigned long long) id, classId);
	if (classId >= 0) {
		s.appendSprintf (" %s", ClassId::classIdName (classId));
	}
	if (! widgetName.empty ()) {
		s.append (" \"");
		s.append (widgetName);
		s.append ("\"");
	}
	if (detailStringFn) {
		detail.assign (detailStringFn (this));
	}
	if (! detail.empty ()) {
		s.append (" ");
		s.append (detail);
	}
	s.append (">");
	return (s);
}

void Widget::setDestroyDelay (int delayMs) {
	if (delayMs <= 0) {
		isDestroyed = true;
		return;
	}
	destroyClock = delayMs;
}

void Widget::syncRecordStore () {
	// Default implementation does nothing
}

void Widget::setKeyFocus (bool enable) {
	// Default implementation does nothing
}

bool Widget::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isInputSuspended) {
		return (false);
	}
	if (keyEventCallback.callback && keyEventCallback.callback (keyEventCallback.callbackData, keycode, isShiftDown, isControlDown)) {
		return (true);
	}
	return (doProcessKeyEvent (keycode, isShiftDown, isControlDown));
}
bool Widget::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Default implementation does nothing
	return (false);
}

Widget *Widget::findWidget (double screenPositionX, double screenPositionY, bool requireMouseHoverEnabled) {
	// Default implementation returns NULL
	return (NULL);
}

Widget *Widget::findWidget (const StdString &widgetName, bool shouldRetain) {
	// Default implementation returns NULL
	return (NULL);
}

Widget *Widget::findWidget (Widget::FindMatchFunction fn, void *fnData, bool shouldRetain) {
	// Default implementation returns NULL
	return (NULL);
}

void Widget::getWidgetNames (StringList *destList) {
	// Default implementation does nothing
}

bool Widget::processMouseState (const Widget::MouseState &mouseState) {
	if (isInputSuspended) {
		return (false);
	}
	if (mouseState.isEntered) {
		if (! isMouseEntered) {
			isMouseEntered = true;
			eventCallback (mouseEnterCallback);
		}

		if (mouseState.isLeftClicked) {
			if (! isMousePressed) {
				isMousePressed = true;
				eventCallback (mousePressCallback);
			}
		}

		if (isMousePressed && mouseState.isLeftClickReleased) {
			isMousePressed = false;
			eventCallback (mouseReleaseCallback);
			if (mouseState.isLeftClickEntered) {
				eventCallback (mouseClickCallback);
			}
		}

		if (isMousePressed && mouseState.isLongPressed) {
			if (mouseLongPressCallback.callback) {
				isMousePressed = false;
				mouseLongPressCallback.callback (mouseLongPressCallback.callbackData, this);
			}
		}
	}
	else {
		if (isMousePressed) {
			isMousePressed = false;
			eventCallback (mouseReleaseCallback);
		}
		if (isMouseEntered) {
			isMouseEntered = false;
			eventCallback (mouseExitCallback);
		}
	}

	return (doProcessMouseState (mouseState));
}
bool Widget::doProcessMouseState (const Widget::MouseState &mouseState) {
	// Default implementation does nothing
	return (false);
}

bool Widget::compareZLevel (Widget *first, Widget *second) {
	if (first->zLevel == second->zLevel) {
		return (first->id < second->id);
	}
	return (first->zLevel < second->zLevel);
}

void Widget::mouseEnter () {
	eventCallback (mouseEnterCallback);
}

void Widget::mouseExit () {
	eventCallback (mouseExitCallback);
}

void Widget::mousePress () {
	eventCallback (mousePressCallback);
}

void Widget::mouseRelease () {
	eventCallback (mouseReleaseCallback);
}

void Widget::mouseClick () {
	eventCallback (mouseClickCallback);
}

bool Widget::eventCallback (const Widget::EventCallbackContext &callback) {
	if (! callback.callback) {
		return (false);
	}
	callback.callback (callback.callbackData, this);
	return (true);
}

void Widget::flowRight (PanelLayoutFlow *flow) {
	double pos;

	position.assign (flow->x, flow->y);
	flow->x += width + UiConfiguration::instance->marginSize;
	pos = position.x + width;
	if (pos > flow->xExtent) {
		flow->xExtent = pos;
	}
	pos = position.y + height;
	if (pos > flow->yExtent) {
		flow->yExtent = pos;
	}
}

void Widget::flowDown (PanelLayoutFlow *flow) {
	double pos;

	position.assign (flow->x, flow->y);
	flow->y += height + UiConfiguration::instance->marginSize;
	pos = position.x + width;
	if (pos > flow->xExtent) {
		flow->xExtent = pos;
	}
	pos = position.y + height;
	if (pos > flow->yExtent) {
		flow->yExtent = pos;
	}
}

void Widget::flowLeft (PanelLayoutFlow *flow) {
	flow->x -= width;
	position.assignX (flow->x);
	flow->x -= UiConfiguration::instance->marginSize;
}

void Widget::flowUp (PanelLayoutFlow *flow) {
	flow->y -= height;
	position.assignY (flow->y);
	flow->y -= UiConfiguration::instance->marginSize;
}

void Widget::centerHorizontal (PanelLayoutFlow *flow) {
	position.assignX (flow->x + ((flow->xExtent - flow->x) / 2.0f) - (width / 2.0f));
}

void Widget::centerVertical (PanelLayoutFlow *flow) {
	position.assignY (flow->y + ((flow->yExtent - flow->y) / 2.0f) - (height / 2.0f));
}

Widget::Rectangle Widget::getScreenRect () {
	Widget::Rectangle rect;

	if (hasScreenPosition) {
		rect.x = screenX;
		rect.y = screenY;
		rect.w = width;
		rect.h = height;
	}
	return (rect);
}

void Widget::setComposeDraw (bool enable) {
	if (isComposeDrawEnabled == enable) {
		return;
	}
	isComposeDrawEnabled = enable;
	if (isComposeDrawEnabled) {
		composeTargetWidth = 0.0f;
		composeTargetHeight = 0.0f;
		composeScale = 1.0f;
		composeRotation = 0.0f;
		composeWidth = width;
		composeHeight = height;
	}
	doProcessComposeDrawChange ();
}
void Widget::doProcessComposeDrawChange () {
	// Default implementation does nothing
}

void Widget::setComposeScale (double scale) {
	if ((scale <= 0.0f) || FLOAT_EQUALS (scale, composeScale)) {
		return;
	}
	composeScale = scale;
	composeWidth = width * composeScale;
	composeHeight = height * composeScale;
}

void Widget::setComposeRotation (double rotation) {
	composeRotation = MathUtil::normalizeDirection (rotation);
}

void Widget::animateScale (double startScale, double targetScale, int duration, bool endCompose) {
	if (! isComposeDrawEnabled) {
		setComposeDraw (true);
	}
	composeAnimationScale.translateX (startScale, targetScale, duration);
	composeAnimationType = endCompose ? ScaleEndComposeAnimation : ScaleAnimation;
	isComposeAnimating = true;
}

void Widget::animateScaleBump () {
	if (! isComposeDrawEnabled) {
		setComposeDraw (true);
	}
	constexpr const int transitionDuration = 80;
	composeAnimationScale.assignX (0.8f);
	composeAnimationScale.plotX (0.4f, transitionDuration);
	composeAnimationScale.plotX (-0.2f, transitionDuration);
	composeAnimationType = ScaleEndComposeAnimation;
	isComposeAnimating = true;
}

void Widget::renderComposeTexture (void *itPtr) {
	Widget *it = (Widget *) itPtr;

	it->executeRenderComposeTexture ();
	it->isComposeRendering = false;
	it->release ();
}

void Widget::executeRenderComposeTexture () {
	int w, h;

	if (isDestroyed) {
		return;
	}
	w = (int) composeTargetWidth;
	h = (int) composeTargetHeight;
	if (composeTexture) {
		if ((composeTextureWidth != w) || (composeTextureHeight != h)) {
			if (! composeTexturePath.empty ()) {
				Resource::instance->unloadTexture (composeTexturePath);
				composeTexturePath.assign ("");
			}
			composeTexture = NULL;
		}
	}
	if (! composeTexture) {
		composeTextureWidth = w;
		composeTextureHeight = h;
		composeTexturePath.sprintf ("*_Widget_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
		composeTexture = Resource::instance->createTexture (composeTexturePath, composeTextureWidth, composeTextureHeight);
		if (! composeTexture) {
			composeTexturePath.assign ("");
		}
	}
	if (! composeTexture) {
		return;
	}
	SDL_SetRenderTarget (App::instance->render, composeTexture);
	SDL_SetRenderDrawBlendMode (App::instance->render, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor (App::instance->render, 0, 0, 0, 0);
	SDL_RenderClear (App::instance->render);
	doDraw (-(position.x), -(position.y));
	SDL_SetRenderTarget (App::instance->render, NULL);
}
