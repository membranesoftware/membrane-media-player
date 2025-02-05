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
#include "StringList.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Resource.h"
#include "RenderResource.h"
#include "RoundedCornerSprite.h"
#include "Input.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Button.h"
#include "CardView.h"
#include "ComboBox.h"
#include "IconLabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ListView.h"
#include "ProgressBar.h"
#include "ProgressRing.h"
#include "ScrollBar.h"
#include "ScrollView.h"
#include "Shader.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "TextArea.h"
#include "TextField.h"
#include "TextFieldWindow.h"
#include "TextFlow.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Panel.h"

Panel::Panel ()
: Widget ()
, bgColor (0.0f, 0.0f, 0.0f)
, borderColor (0.0f, 0.0f, 0.0f)
, dropShadowColor (0.0f, 0.0f, 0.0f, 0.8f)
, layoutSpacing (-1.0f)
, isDrawClipEnabled (false)
, extentX1 (0.0f)
, extentY1 (0.0f)
, extentX2 (0.0f)
, extentY2 (0.0f)
, maxWidgetZLevel (0)
, viewOriginX (0.0f)
, viewOriginY (0.0f)
, isViewOriginBoundEnabled (false)
, minViewOriginX (0.0f)
, minViewOriginY (0.0f)
, maxViewOriginX (0.0f)
, maxViewOriginY (0.0f)
, widthPadding (0.0f)
, heightPadding (0.0f)
, widthPaddingScale (1.0f)
, heightPaddingScale (1.0f)
, isFixedPadding (false)
, isFilledBg (false)
, topLeftCornerRadius (0)
, topRightCornerRadius (0)
, bottomLeftCornerRadius (0)
, bottomRightCornerRadius (0)
, isBordered (false)
, borderWidth (0.0f)
, isDropShadowed (false)
, dropShadowWidth (0.0f)
, isFixedSize (false)
, isWaiting (false)
, layout (Panel::NoLayout)
, detailSize (-1)
, detailMaxWidth (0.0f)
, isMouseInputStarted (false)
, lastMouseLeftUpCount (0)
, lastMouseLeftDownCount (0)
, lastMouseRightUpCount (0)
, lastMouseRightDownCount (0)
, lastMouseWheelUpCount (0)
, lastMouseWheelDownCount (0)
, lastMouseDownX (-1)
, lastMouseDownY (-1)
, lastMouseDownTime (0)
, cornerCenterDx (0)
, cornerCenterDy (0)
, cornerCenterDw (0)
, cornerCenterDh (0)
, cornerTopDx (0)
, cornerTopDy (0)
, cornerTopDw (0)
, cornerTopDh (0)
, cornerLeftDx (0)
, cornerLeftDy (0)
, cornerLeftDw (0)
, cornerLeftDh (0)
, cornerRightDx (0)
, cornerRightDy (0)
, cornerRightDw (0)
, cornerRightDh (0)
, cornerBottomDx (0)
, cornerBottomDy (0)
, cornerBottomDw (0)
, cornerBottomDh (0)
, cornerSize (0)
, waitPanelHandle (&waitPanel)
, waitProgressBarHandle (&waitProgressBar)
{
	SdlUtil::createMutex (&widgetListMutex);
	SdlUtil::createMutex (&widgetAddListMutex);
}
Panel::~Panel () {
	clear ();
	SdlUtil::destroyMutex (&widgetListMutex);
	SdlUtil::destroyMutex (&widgetAddListMutex);
}

void Panel::clear () {
	std::list<Widget *>::const_iterator i1, i2;

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.cbegin ();
	i2 = widgetAddList.cend ();
	while (i1 != i2) {
		(*i1)->isDestroyed = true;
		(*i1)->release ();
		++i1;
	}
	widgetAddList.clear ();
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		(*i1)->isDestroyed = true;
		(*i1)->release ();
		++i1;
	}
	widgetList.clear ();
	SDL_UnlockMutex (widgetListMutex);

	resetSize ();
}

Widget *Panel::addWidget (Widget *widget) {
	if (widget->id <= 0) {
		widget->id = App::instance->getUniqueId ();
	}
	if (widget->sortKey.empty ()) {
		widget->sortKey.sprintf ("%016llx", (unsigned long long) widget->id);
	}
	widget->retain ();
	SDL_LockMutex (widgetAddListMutex);
	widgetAddList.push_back (widget);
	SDL_UnlockMutex (widgetAddListMutex);

	resetSize ();
	return (widget);
}

Widget *Panel::addWidget (Widget *widget, const Position &position) {
	addWidget (widget);
	widget->position.assign (position);
	return (widget);
}

Widget *Panel::addWidget (Widget *widget, int zLevel) {
	addWidget (widget);
	widget->zLevel = (zLevel < Widget::minZLevel) ? Widget::minZLevel : zLevel;
	return (widget);
}

Widget *Panel::addWidget (Widget *widget, const Position &position, int zLevel) {
	addWidget (widget);
	widget->position.assign (position);
	widget->zLevel = (zLevel < Widget::minZLevel) ? Widget::minZLevel : zLevel;
	return (widget);
}

Button *Panel::add (Button *widget, int zLevel) {
	return ((Button *) addWidget (widget, zLevel));
}

CardView *Panel::add (CardView *widget, int zLevel) {
	return ((CardView *) addWidget (widget, zLevel));
}

ComboBox *Panel::add (ComboBox *widget, int zLevel) {
	return ((ComboBox *) addWidget (widget, zLevel));
}

IconLabelWindow *Panel::add (IconLabelWindow *widget, int zLevel) {
	return ((IconLabelWindow *) addWidget (widget, zLevel));
}

Image *Panel::add (Image *widget, int zLevel) {
	return ((Image *) addWidget (widget, zLevel));
}

ImageWindow *Panel::add (ImageWindow *widget, int zLevel) {
	return ((ImageWindow *) addWidget (widget, zLevel));
}

Label *Panel::add (Label *widget, int zLevel) {
	return ((Label *) addWidget (widget, zLevel));
}

LabelWindow *Panel::add (LabelWindow *widget, int zLevel) {
	return ((LabelWindow *) addWidget (widget, zLevel));
}

ListView *Panel::add (ListView *widget, int zLevel) {
	return ((ListView *) addWidget (widget, zLevel));
}

Panel *Panel::add (Panel *widget, int zLevel) {
	return ((Panel *) addWidget (widget, zLevel));
}

ProgressBar *Panel::add (ProgressBar *widget, int zLevel) {
	return ((ProgressBar *) addWidget (widget, zLevel));
}

ProgressRing *Panel::add (ProgressRing *widget, int zLevel) {
	return ((ProgressRing *) addWidget (widget, zLevel));
}

ScrollBar *Panel::add (ScrollBar *widget, int zLevel) {
	return ((ScrollBar *) addWidget (widget, zLevel));
}

ScrollView *Panel::add (ScrollView *widget, int zLevel) {
	return ((ScrollView *) addWidget (widget, zLevel));
}

Shader *Panel::add (Shader *widget, int zLevel) {
	return ((Shader *) addWidget (widget, zLevel));
}

Slider *Panel::add (Slider *widget, int zLevel) {
	return ((Slider *) addWidget (widget, zLevel));
}

SliderWindow *Panel::add (SliderWindow *widget, int zLevel) {
	return ((SliderWindow *) addWidget (widget, zLevel));
}

TextArea *Panel::add (TextArea *widget, int zLevel) {
	return ((TextArea *) addWidget (widget, zLevel));
}

TextField *Panel::add (TextField *widget, int zLevel) {
	return ((TextField *) addWidget (widget, zLevel));
}

TextFieldWindow *Panel::add (TextFieldWindow *widget, int zLevel) {
	return ((TextFieldWindow *) addWidget (widget, zLevel));
}

TextFlow *Panel::add (TextFlow *widget, int zLevel) {
	return ((TextFlow *) addWidget (widget, zLevel));
}

Toggle *Panel::add (Toggle *widget, int zLevel) {
	return ((Toggle *) addWidget (widget, zLevel));
}

ToggleWindow *Panel::add (ToggleWindow *widget, int zLevel) {
	return ((ToggleWindow *) addWidget (widget, zLevel));
}

void Panel::removeWidget (Widget *targetWidget) {
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.begin ();
	i2 = widgetAddList.end ();
	while (i1 != i2) {
		widget = *i1;
		if (widget == targetWidget) {
			widgetAddList.erase (i1);
			widget->release ();
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		if (widget == targetWidget) {
			widgetList.erase (i1);
			widget->release ();
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (widgetListMutex);
}

Widget *Panel::findWidget (double screenPositionX, double screenPositionY, bool requireMouseHoverEnabled) {
	std::list<Widget *>::reverse_iterator i1, i2;
	Widget *widget, *item, *nextitem;
	double x, y, w, h;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.rbegin ();
	i2 = widgetList.rend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}
		w = widget->width;
		h = widget->height;
		if ((w <= 0.0f) || (h <= 0.0f)) {
			continue;
		}
		x = widget->screenX;
		y = widget->screenY;
		if ((screenPositionX >= x) && (screenPositionX <= (x + w)) && (screenPositionY >= y) && (screenPositionY <= (y + h))) {
			item = widget;
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	if (item) {
		nextitem = item->findWidget (screenPositionX, screenPositionY, requireMouseHoverEnabled);
		if (nextitem) {
			item = nextitem;
		}
	}
	if (item && (requireMouseHoverEnabled && (! item->isMouseHoverEnabled))) {
		item = NULL;
	}
	return (item);
}

Widget *Panel::findWidget (const StdString &widgetName, bool shouldRetain) {
	std::list<Widget *>::const_iterator i1, i2;
	Widget *widget, *item;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}
		if (widget->widgetName.equals (widgetName)) {
			item = widget;
			if (shouldRetain) {
				item->retain ();
			}
			break;
		}
		item = widget->findWidget (widgetName, shouldRetain);
		if (item) {
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);
	return (item);
}

Widget *Panel::findWidget (Widget::FindMatchFunction fn, void *fnData, bool shouldRetain) {
	std::list<Widget *>::const_iterator i1, i2;
	Widget *widget, *item;

	item = NULL;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (fn (fnData, widget)) {
			item = widget;
			if (shouldRetain) {
				item->retain ();
			}
			break;
		}
		item = widget->findWidget (fn, fnData, shouldRetain);
		if (item) {
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);
	return (item);
}

void Panel::processWidgets (Widget::EventCallback fn, void *fnData) {
	std::list<Widget *>::const_iterator i1, i2;

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		fn (fnData, *i1);
		++i1;
	}
	SDL_UnlockMutex (widgetListMutex);

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.cbegin ();
	i2 = widgetAddList.cend ();
	while (i1 != i2) {
		fn (fnData, *i1);
		++i1;
	}
	SDL_UnlockMutex (widgetAddListMutex);
}

void Panel::getWidgetNames (StringList *destList) {
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}
		if (! widget->widgetName.empty ()) {
			destList->push_back (widget->widgetName);
		}
		widget->getWidgetNames (destList);
	}
	SDL_UnlockMutex (widgetListMutex);
}

void Panel::doUpdate (int msElapsed) {
	std::list<Widget *> addwidgets;
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;
	bool found;

	bgColor.update (msElapsed);
	borderColor.update (msElapsed);

	SDL_LockMutex (widgetAddListMutex);
	addwidgets.swap (widgetAddList);
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	widgetList.splice (widgetList.end (), addwidgets);
	while (true) {
		found = false;
		i1 = widgetList.begin ();
		i2 = widgetList.end ();
		while (i1 != i2) {
			widget = *i1;
			if (widget->isDestroyed) {
				found = true;
				widgetList.erase (i1);
				widget->release ();
				break;
			}
			++i1;
		}
		if (! found) {
			break;
		}
	}
	sortWidgetList ();
	resetExtents ();

	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		widget->update (msElapsed, screenX - viewOriginX, screenY - viewOriginY);
		++i1;
	}
	SDL_UnlockMutex (widgetListMutex);

	waitPanelHandle.compact ();
	waitProgressBarHandle.compact ();
	if (waitPanel) {
		waitPanel->setFixedSize (true, width, height);
		if (waitProgressBar) {
			waitProgressBar->setSize (width, UiConfiguration::instance->progressBarHeight);
		}
	}
}

void Panel::processInput () {
	std::list<Widget *>::reverse_iterator i1, i2;
	std::vector<Input::KeyPressEvent> keyevents;
	std::vector<Input::KeyPressEvent>::const_iterator j1, j2;
	Widget *widget, *mousewidget;
	Widget::MouseState mousestate;
	double x, y, enterdx, enterdy;
	bool isleftdown, isconsumed;

	Input::instance->pollKeyPressEvents (&keyevents);
	isleftdown = (Input::instance->mouseLeftDownCount != Input::instance->mouseLeftUpCount);

	if (isMouseInputStarted) {
		if (Input::instance->mouseLeftDownCount != lastMouseLeftDownCount) {
			mousestate.isLeftClicked = true;
		}
		if (Input::instance->mouseLeftUpCount != lastMouseLeftUpCount) {
			mousestate.isLeftClickReleased = true;
		}
		mousestate.positionDeltaX = Input::instance->mouseX - Input::instance->lastMouseX;
		mousestate.positionDeltaY = Input::instance->mouseY - Input::instance->lastMouseY;
		mousestate.wheelUp = Input::instance->mouseWheelUpCount - lastMouseWheelUpCount;
		mousestate.wheelDown = Input::instance->mouseWheelDownCount - lastMouseWheelDownCount;
	}
	lastMouseLeftUpCount = Input::instance->mouseLeftUpCount;
	lastMouseLeftDownCount = Input::instance->mouseLeftDownCount;
	lastMouseRightUpCount = Input::instance->mouseRightUpCount;
	lastMouseRightDownCount = Input::instance->mouseRightDownCount;
	lastMouseWheelUpCount = Input::instance->mouseWheelUpCount;
	lastMouseWheelDownCount = Input::instance->mouseWheelDownCount;
	isMouseInputStarted = true;

	mousewidget = NULL;
	x = Input::instance->mouseX;
	y = Input::instance->mouseY;
	enterdx = 0.0f;
	enterdy = 0.0f;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.rbegin ();
	i2 = widgetList.rend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}
		if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
			if ((x >= (int) widget->screenX) && (x <= (int) (widget->screenX + widget->width)) && (y >= (int) widget->screenY) && (y <= (int) (widget->screenY + widget->height))) {
				mousewidget = widget;
				enterdx = x - widget->screenX;
				enterdy = y - widget->screenY;
				break;
			}
		}
	}

	if (mousestate.isLeftClicked) {
		if (mousewidget) {
			lastMouseDownX = (int) x;
			lastMouseDownY = (int) y;
			lastMouseDownTime = OsUtil::getTime ();
		}
		else {
			lastMouseDownX = -1;
			lastMouseDownY = -1;
			lastMouseDownTime = 0;
		}
	}

	if (keyEventCallback.callback && (keyevents.size () > 0)) {
		isconsumed = false;
		j1 = keyevents.cbegin ();
		j2 = keyevents.cend ();
		while (j1 != j2) {
			if (keyEventCallback.callback (keyEventCallback.callbackData, j1->keycode, j1->isShiftDown, j1->isControlDown)) {
				isconsumed = true;
			}
			++j1;
		}
		if (isconsumed) {
			keyevents.clear ();
		}
	}

	i1 = widgetList.rbegin ();
	i2 = widgetList.rend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}
		if (keyevents.size () > 0) {
			isconsumed = false;
			j1 = keyevents.cbegin ();
			j2 = keyevents.cend ();
			while (j1 != j2) {
				if (widget->processKeyEvent (j1->keycode, j1->isShiftDown, j1->isControlDown)) {
					isconsumed = true;
				}
				++j1;
			}
			if (isconsumed) {
				keyevents.clear ();
			}
		}

		mousestate.isLeftClickEntered = false;
		mousestate.isLongPressed = false;
		if (widget == mousewidget) {
			mousestate.isEntered = true;
			mousestate.enterDeltaX = enterdx;
			mousestate.enterDeltaY = enterdy;

			if (mousestate.isLeftClickReleased) {
				if ((lastMouseDownX >= 0) && (lastMouseDownY >= 0) && (lastMouseDownX >= (int) widget->screenX) && (lastMouseDownX <= (int) (widget->screenX + widget->width)) && (lastMouseDownY >= (int) widget->screenY) && (lastMouseDownY <= (int) (widget->screenY + widget->height))) {
					mousestate.isLeftClickEntered = true;
					lastMouseDownX = -1;
					lastMouseDownY = -1;
				}
			}
			else if (isleftdown) {
				if ((lastMouseDownX >= 0) && (lastMouseDownY >= 0) && (lastMouseDownTime > 0) && (lastMouseDownX >= (int) widget->screenX) && (lastMouseDownX <= (int) (widget->screenX + widget->width)) && (lastMouseDownY >= (int) widget->screenY) && (lastMouseDownY <= (int) (widget->screenY + widget->height)) && ((OsUtil::getTime () - lastMouseDownTime) >= UiConfiguration::instance->longPressThreshold)) {
					mousestate.isLongPressed = true;
					lastMouseDownTime = 0;
				}
			}
		}
		else {
			mousestate.isEntered = false;
			mousestate.enterDeltaX = 0.0f;
			mousestate.enterDeltaY = 0.0f;
		}

		if (widget->processMouseState (mousestate)) {
			mousestate.wheelUp = 0;
			mousestate.wheelDown = 0;
		}
	}
	SDL_UnlockMutex (widgetListMutex);
}

bool Panel::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;
	bool result;

	result = false;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || widget->isInputSuspended) {
			continue;
		}
		result = widget->processKeyEvent (keycode, isShiftDown, isControlDown);
		if (result) {
			break;
		}
	}
	SDL_UnlockMutex (widgetListMutex);
	return (result);
}

bool Panel::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::list<Widget *>::reverse_iterator i1, i2;
	Widget *widget;
	bool found, consumed;
	Widget::MouseState m;
	double x, y;

	x = Input::instance->mouseX;
	y = Input::instance->mouseY;
	consumed = false;
	found = false;
	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.rbegin ();
	i2 = widgetList.rend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || widget->isInputSuspended || (! widget->isVisible) || (! widget->hasScreenPosition)) {
			continue;
		}

		m = mouseState;
		m.isEntered = false;
		m.enterDeltaX = 0.0f;
		m.enterDeltaY = 0.0f;
		if ((! found) && mouseState.isEntered) {
			if ((widget->width > 0.0f) && (widget->height > 0.0f)) {
				if ((x >= (int) widget->screenX) && (x <= (int) (widget->screenX + widget->width)) && (y >= (int) widget->screenY) && (y <= (int) (widget->screenY + widget->height))) {
					m.isEntered = true;
					m.enterDeltaX = x - widget->screenX;
					m.enterDeltaY = y - widget->screenY;
					found = true;
				}
			}
		}

		if (consumed) {
			m.wheelUp = 0;
			m.wheelDown = 0;
		}
		if (widget->processMouseState (m)) {
			consumed = true;
		}
	}
	SDL_UnlockMutex (widgetListMutex);

	return (consumed);
}

void Panel::doResetInputState () {
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;

	lastMouseLeftUpCount = Input::instance->mouseLeftUpCount;
	lastMouseLeftDownCount = Input::instance->mouseLeftDownCount;
	lastMouseRightUpCount = Input::instance->mouseRightUpCount;
	lastMouseRightDownCount = Input::instance->mouseRightDownCount;
	lastMouseWheelUpCount = Input::instance->mouseWheelUpCount;
	lastMouseWheelDownCount = Input::instance->mouseWheelDownCount;

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		widget->resetInputState ();
		++i1;
	}
	SDL_UnlockMutex (widgetListMutex);
}

void Panel::doDraw (double originX, double originY) {
	SDL_Renderer *render;
	SDL_Texture *cornertexture;
	SDL_Rect rect;
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;
	int x0, y0, texturew, textureh;
	double drawx1, drawy1, drawx2, drawy2;

	render = App::instance->render;
	x0 = (int) (originX + position.x);
	y0 = (int) (originY + position.y);
	rect.x = x0;
	rect.y = y0;
	rect.w = (int) width;
	rect.h = (int) height;
	App::instance->pushClipRect (rect);

	if (isFilledBg && (bgColor.aByte > 0)) {
		if (bgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, bgColor.rByte, bgColor.gByte, bgColor.bByte, bgColor.aByte);

		if ((cornerSize > 0) && ((int) width >= cornerSize) && ((int) height >= cornerSize)) {
			if (topLeftCornerRadius > 0) {
				cornertexture = RenderResource::instance->roundedCornerSprite->getScaleTexture (topLeftCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0;
					rect.y = y0;
					rect.w = topLeftCornerRadius;
					rect.h = topLeftCornerRadius;
					App::instance->pushClipRect (rect);

					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureAlphaMod (cornertexture, bgColor.aByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);

					App::instance->popClipRect ();
				}
			}
			if (topRightCornerRadius > 0) {
				cornertexture = RenderResource::instance->roundedCornerSprite->getScaleTexture (topRightCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0 + (int) width - topRightCornerRadius;
					rect.y = y0;
					rect.w = topRightCornerRadius;
					rect.h = topRightCornerRadius;
					App::instance->pushClipRect (rect);

					rect.x = x0 + (int) width - texturew;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureAlphaMod (cornertexture, bgColor.aByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);

					App::instance->popClipRect ();
				}
			}
			if (bottomLeftCornerRadius > 0) {
				cornertexture = RenderResource::instance->roundedCornerSprite->getScaleTexture (bottomLeftCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0;
					rect.y = y0 + (int) height - bottomLeftCornerRadius;
					rect.w = bottomLeftCornerRadius;
					rect.h = bottomLeftCornerRadius;
					App::instance->pushClipRect (rect);

					rect.y = y0 + (int) height - textureh;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureAlphaMod (cornertexture, bgColor.aByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);

					App::instance->popClipRect ();
				}
			}
			if (bottomRightCornerRadius > 0) {
				cornertexture = RenderResource::instance->roundedCornerSprite->getScaleTexture (bottomRightCornerRadius, &texturew, &textureh);
				if (cornertexture) {
					rect.x = x0 + (int) width - bottomRightCornerRadius;
					rect.y = y0 + (int) height - bottomRightCornerRadius;
					rect.w = bottomRightCornerRadius;
					rect.h = bottomRightCornerRadius;
					App::instance->pushClipRect (rect);

					rect.x = x0 + (int) width - texturew;
					rect.y = y0 + (int) height - textureh;
					rect.w = texturew;
					rect.h = textureh;
					SDL_SetTextureColorMod (cornertexture, bgColor.rByte, bgColor.gByte, bgColor.bByte);
					SDL_SetTextureAlphaMod (cornertexture, bgColor.aByte);
					SDL_SetTextureBlendMode (cornertexture, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy (render, cornertexture, NULL, &rect);

					App::instance->popClipRect ();
				}
			}

			rect.x = x0;
			rect.y = y0;
			rect.w = (int) width;
			rect.h = (int) height;
			rect.x += cornerCenterDx;
			rect.y += cornerCenterDy;
			rect.w += cornerCenterDw;
			rect.h += cornerCenterDh;
			SDL_RenderFillRect (render, &rect);

			if (cornerTopDh > 0) {
				rect.x = x0 + cornerTopDx;
				rect.y = y0 + cornerTopDy;
				rect.w = ((int) width) + cornerTopDw;
				rect.h = cornerTopDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerLeftDw > 0) {
				rect.x = x0 + cornerLeftDx;
				rect.y = y0 + cornerLeftDy;
				rect.w = cornerLeftDw;
				rect.h = ((int) height) + cornerLeftDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerRightDw > 0) {
				rect.x = x0 + (int) width + cornerRightDx;
				rect.y = y0 + cornerRightDy;
				rect.w = cornerRightDw;
				rect.h = ((int) height) + cornerRightDh;
				SDL_RenderFillRect (render, &rect);
			}
			if (cornerBottomDh > 0) {
				rect.x = x0 + cornerBottomDx;
				rect.y = y0 + (int) height + cornerBottomDy;
				rect.w = ((int) width) + cornerBottomDw;
				rect.h = cornerBottomDh;
				SDL_RenderFillRect (render, &rect);
			}
		}
		else {
			rect.x = x0;
			rect.y = y0;
			rect.w = (int) width;
			rect.h = (int) height;
			SDL_RenderFillRect (render, &rect);
		}

		if (bgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
	}

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible)) {
			continue;
		}
		if (isDrawClipEnabled) {
			drawx1 = widget->position.x - viewOriginX;
			drawy1 = widget->position.y - viewOriginY;
			drawx2 = drawx1 + widget->width;
			drawy2 = drawy1 + widget->height;
			if ((drawx2 < 0.0f) || (drawy2 < 0.0f) || (drawx1 > width) || (drawy1 > height)) {
				continue;
			}
		}
		widget->draw (x0 - (int) viewOriginX, y0 - (int) viewOriginY);
	}
	SDL_UnlockMutex (widgetListMutex);

	if (isBordered && (borderColor.aByte > 0) && (borderWidth >= 1.0f)) {
		if (borderColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, borderColor.rByte, borderColor.gByte, borderColor.bByte, borderColor.aByte);

		rect.x = x0;
		rect.y = y0;
		rect.w = (int) width;
		rect.h = (int) borderWidth;
		SDL_RenderFillRect (render, &rect);

		rect.y = y0 + (int) (height - borderWidth);
		SDL_RenderFillRect (render, &rect);

		rect.y = y0 + (int) borderWidth;
		rect.w = (int) borderWidth;
		rect.h = ((int) height) - (int) (borderWidth * 2.0f);
		SDL_RenderFillRect (render, &rect);

		rect.x = x0 + (int) (width - borderWidth);
		SDL_RenderFillRect (render, &rect);

		if (borderColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
	}
	App::instance->popClipRect ();

	if (isDropShadowed && (dropShadowColor.aByte > 0) && (dropShadowWidth >= 1.0f)) {
		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor (render, dropShadowColor.rByte, dropShadowColor.gByte, dropShadowColor.bByte, dropShadowColor.aByte);

		rect.x = App::instance->clipRect.x;
		rect.y = App::instance->clipRect.y;
		rect.w = App::instance->clipRect.w + (int) dropShadowWidth;
		rect.h = App::instance->clipRect.h + (int) dropShadowWidth;
		App::instance->pushClipRect (rect, true);

		rect.x = x0 + (int) width;
		rect.y = y0 + (int) dropShadowWidth;
		rect.w = (int) dropShadowWidth;
		rect.h = (int) height;
		SDL_RenderFillRect (render, &rect);

		rect.x = x0 + (int) dropShadowWidth;
		rect.y = y0 + (int) height;
		rect.w = (int) (width - dropShadowWidth);
		rect.h = (int) dropShadowWidth;
		SDL_RenderFillRect (render, &rect);

		App::instance->popClipRect ();

		SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
	}
}

void Panel::doResize () {
	std::list<Widget *>::iterator i1, i2;
	Widget *widget;

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.begin ();
	i2 = widgetList.end ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed) {
			continue;
		}
		widget->resize ();
	}
	SDL_UnlockMutex (widgetListMutex);

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.begin ();
	i2 = widgetAddList.end ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed) {
			continue;
		}
		widget->resize ();
	}
	SDL_UnlockMutex (widgetAddListMutex);

	reflow ();
}

void Panel::resetExtents () {
	std::list<Widget *>::const_iterator i1, i2;
	Widget *widget;
	double x1, y1, x2, y2, x, y;
	bool first;

	x1 = 0.0f;
	y1 = 0.0f;
	x2 = 0.0f;
	y2 = 0.0f;
	first = true;

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.cbegin ();
	i2 = widgetAddList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible) || widget->isPanelSizeClipEnabled) {
			continue;
		}
		x = widget->position.x;
		y = widget->position.y;
		if (first || (x < x1)) {
			x1 = x;
		}
		if (first || (y < y1)) {
			y1 = y;
		}

		x = widget->position.x + widget->width;
		y = widget->position.y + widget->height;
		if (first || (x > x2)) {
			x2 = x;
		}
		if (first || (y > y2)) {
			y2 = y;
		}

		first = false;
	}
	SDL_UnlockMutex (widgetAddListMutex);

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed || (! widget->isVisible) || widget->isPanelSizeClipEnabled) {
			continue;
		}
		x = widget->position.x;
		y = widget->position.y;
		if (first || (x < x1)) {
			x1 = x;
		}
		if (first || (y < y1)) {
			y1 = y;
		}

		x = widget->position.x + widget->width;
		y = widget->position.y + widget->height;
		if (first || (x > x2)) {
			x2 = x;
		}
		if (first || (y > y2)) {
			y2 = y;
		}

		first = false;
	}
	SDL_UnlockMutex (widgetListMutex);

	if ((! FLOAT_EQUALS (x1, extentX1)) || (! FLOAT_EQUALS (y1, extentY1)) || (! FLOAT_EQUALS (x2, extentX2)) || (! FLOAT_EQUALS (y2, extentY2))) {
		extentX1 = x1;
		extentY1 = y1;
		extentX2 = x2;
		extentY2 = y2;
		doSetExtents ();
	}
}

void Panel::resetSize () {
	resetExtents ();
	if (! isFixedSize) {
		width = extentX2 + widthPadding;
		height = extentY2 + heightPadding;
	}
}

void Panel::resetPadding () {
	if (! isFixedPadding) {
		widthPadding = UiConfiguration::instance->paddingSize * widthPaddingScale;
		heightPadding = UiConfiguration::instance->paddingSize * heightPaddingScale;
	}
}

void Panel::topLeftLayoutFlow () {
	layoutFlow.x = widthPadding;
	layoutFlow.y = heightPadding;
	layoutFlow.xExtent = 0.0f;
	layoutFlow.yExtent = 0.0f;
}

void Panel::bottomRightLayoutFlow () {
	layoutFlow.x = width - widthPadding;
	layoutFlow.y = height - heightPadding;
	layoutFlow.xExtent = 0.0f;
	layoutFlow.yExtent = 0.0f;
}

void Panel::nextRowLayoutFlow () {
	layoutFlow.x = widthPadding;
	layoutFlow.y = layoutFlow.yExtent + UiConfiguration::instance->marginSize;
	layoutFlow.xExtent = 0.0f;
}

void Panel::nextColumnLayoutFlow () {
	layoutFlow.x = layoutFlow.xExtent + UiConfiguration::instance->marginSize;
	layoutFlow.y = heightPadding;
	layoutFlow.yExtent = 0.0f;
}

void Panel::reflow () {
	std::list<Widget *>::iterator i1, i2;
	std::list<Widget *>::reverse_iterator j1, j2;
	Widget *widget;
	double x, y, margin;

	if (layout == 0) {
		resetSize ();
		return;
	}
	margin = (layoutSpacing >= 0.0f) ? layoutSpacing : UiConfiguration::instance->marginSize;
	resetPadding ();
	x = widthPadding;
	y = heightPadding;

	if (layout & Panel::DownFlowLayoutOption) {
		SDL_LockMutex (widgetListMutex);
		i1 = widgetList.begin ();
		i2 = widgetList.end ();
		while (i1 != i2) {
			widget = *i1;
			++i1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			y += widget->height + margin;
		}
		SDL_UnlockMutex (widgetListMutex);

		SDL_LockMutex (widgetAddListMutex);
		i1 = widgetAddList.begin ();
		i2 = widgetAddList.end ();
		while (i1 != i2) {
			widget = *i1;
			++i1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			y += widget->height + margin;
		}
		SDL_UnlockMutex (widgetAddListMutex);
	}
	else if (layout & Panel::LeftFlowLayoutOption) {
		SDL_LockMutex (widgetAddListMutex);
		j1 = widgetAddList.rbegin ();
		j2 = widgetAddList.rend ();
		while (j1 != j2) {
			widget = *j1;
			++j1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			x += widget->width + margin;
		}
		SDL_UnlockMutex (widgetAddListMutex);

		SDL_LockMutex (widgetListMutex);
		j1 = widgetList.rbegin ();
		j2 = widgetList.rend ();
		while (j1 != j2) {
			widget = *j1;
			++j1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			x += widget->width + margin;
		}
		SDL_UnlockMutex (widgetListMutex);
	}
	else if (layout & Panel::UpFlowLayoutOption) {
		SDL_LockMutex (widgetAddListMutex);
		j1 = widgetAddList.rbegin ();
		j2 = widgetAddList.rend ();
		while (j1 != j2) {
			widget = *j1;
			++j1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			y += widget->height + margin;
		}
		SDL_UnlockMutex (widgetAddListMutex);

		SDL_LockMutex (widgetListMutex);
		j1 = widgetList.rbegin ();
		j2 = widgetList.rend ();
		while (j1 != j2) {
			widget = *j1;
			++j1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			y += widget->height + margin;
		}
		SDL_UnlockMutex (widgetListMutex);
	}
	else {
		SDL_LockMutex (widgetListMutex);
		i1 = widgetList.begin ();
		i2 = widgetList.end ();
		while (i1 != i2) {
			widget = *i1;
			++i1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			x += widget->width + margin;
		}
		SDL_UnlockMutex (widgetListMutex);

		SDL_LockMutex (widgetAddListMutex);
		i1 = widgetAddList.begin ();
		i2 = widgetAddList.end ();
		while (i1 != i2) {
			widget = *i1;
			++i1;
			if (widget->isDestroyed || (! widget->isVisible)) {
				continue;
			}
			widget->position.assign (x, y);
			x += widget->width + margin;
		}
		SDL_UnlockMutex (widgetAddListMutex);
	}

	resetSize ();

	if (layout & (Panel::UpFlowLayoutOption | Panel::DownFlowLayoutOption)) {
		if (layout & Panel::RightGravityLayoutOption) {
			SDL_LockMutex (widgetListMutex);
			i1 = widgetList.begin ();
			i2 = widgetList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignX (width - widthPadding - widget->width);
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i1 = widgetAddList.begin ();
			i2 = widgetAddList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignX (width - widthPadding - widget->width);
			}
			SDL_UnlockMutex (widgetAddListMutex);
		}
		else if (layout & Panel::HorizontalCenterLayoutOption) {
			SDL_LockMutex (widgetListMutex);
			i1 = widgetList.begin ();
			i2 = widgetList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignX ((width / 2.0f) - (widget->width / 2.0f));
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i1 = widgetAddList.begin ();
			i2 = widgetAddList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignX ((width / 2.0f) - (widget->width / 2.0f));
			}
			SDL_UnlockMutex (widgetAddListMutex);
		}
	}
	else if (layout & (Panel::LeftFlowLayoutOption | Panel::RightFlowLayoutOption)) {
		if (layout & Panel::DownGravityLayoutOption) {
			SDL_LockMutex (widgetListMutex);
			i1 = widgetList.begin ();
			i2 = widgetList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignY (height - heightPadding - widget->height);
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i1 = widgetAddList.begin ();
			i2 = widgetAddList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignY (height - heightPadding - widget->height);
			}
			SDL_UnlockMutex (widgetAddListMutex);
		}
		else if (layout & Panel::VerticalCenterLayoutOption) {
			SDL_LockMutex (widgetListMutex);
			i1 = widgetList.begin ();
			i2 = widgetList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignY ((height / 2.0f) - (widget->height / 2.0f));
			}
			SDL_UnlockMutex (widgetListMutex);

			SDL_LockMutex (widgetAddListMutex);
			i1 = widgetAddList.begin ();
			i2 = widgetAddList.end ();
			while (i1 != i2) {
				widget = *i1;
				++i1;
				if (widget->isDestroyed || (! widget->isVisible)) {
					continue;
				}
				widget->position.assignY ((height / 2.0f) - (widget->height / 2.0f));
			}
			SDL_UnlockMutex (widgetAddListMutex);
		}
	}
}

void Panel::sortWidgetList () {
	std::list<Widget *>::const_iterator i1, i2;
	Widget *widget;
	int minlevel, maxlevel;
	bool sorted;

	sorted = true;
	minlevel = Widget::minZLevel - 1;
	maxlevel = Widget::minZLevel - 1;
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		widget = *i1;
		if (minlevel < Widget::minZLevel) {
			minlevel = widget->zLevel;
		}
		if (maxlevel < Widget::minZLevel) {
			maxlevel = widget->zLevel;
		}

		if (widget->zLevel < minlevel) {
			sorted = false;
			minlevel = widget->zLevel;
		}
		if (widget->zLevel < maxlevel) {
			sorted = false;
		}
		if (widget->zLevel > maxlevel) {
			maxlevel = widget->zLevel;
		}
		++i1;
	}
	maxWidgetZLevel = maxlevel;
	if (sorted) {
		return;
	}

	widgetList.sort (Widget::compareZLevel);
}

void Panel::setLayout (int layoutType) {
	if (layout == layoutType) {
		return;
	}
	layout = layoutType;
	reflow ();
}

void Panel::setFillBg (bool enable, const Color &color) {
	if (enable) {
		bgColor.assign (color);
		isFilledBg = true;
	}
	else {
		isFilledBg = false;
	}
}

void Panel::setCornerRadius (int radius) {
	setCornerRadius (radius, radius, radius, radius);
}

void Panel::setCornerRadius (int topLeftRadius, int topRightRadius, int bottomLeftRadius, int bottomRightRadius) {
	int centerx, centery, centerw, centerh, topx, topy, topw, toph, leftx, lefty, leftw, lefth, rightx, righty, rightw, righth, bottomx, bottomy, bottomw, bottomh, amt;

	if (topLeftRadius < 0) {
		topLeftRadius = 0;
	}
	if (topLeftRadius > (int) RoundedCornerSprite::maxCornerRadius) {
		topLeftRadius = (int) RoundedCornerSprite::maxCornerRadius;
	}
	if (topRightRadius < 0) {
		topRightRadius = 0;
	}
	if (topRightRadius > (int) RoundedCornerSprite::maxCornerRadius) {
		topRightRadius = (int) RoundedCornerSprite::maxCornerRadius;
	}
	if (bottomLeftRadius < 0) {
		bottomLeftRadius = 0;
	}
	if (bottomLeftRadius > (int) RoundedCornerSprite::maxCornerRadius) {
		bottomLeftRadius = (int) RoundedCornerSprite::maxCornerRadius;
	}
	if (bottomRightRadius < 0) {
		bottomRightRadius = 0;
	}
	if (bottomRightRadius > (int) RoundedCornerSprite::maxCornerRadius) {
		bottomRightRadius = (int) RoundedCornerSprite::maxCornerRadius;
	}
	if ((topLeftRadius <= 0) && (topRightRadius <= 0) && (bottomLeftRadius <= 0) && (bottomRightRadius <= 0)) {
		cornerSize = 0;
		return;
	}

	centerx = 0;
	centery = 0;
	centerw = 0;
	centerh = 0;
	topx = 0;
	topy = 0;
	topw = 0;
	toph = 0;
	leftx = 0;
	lefty = 0;
	leftw = 0;
	lefth = 0;
	rightx = 0;
	righty = 0;
	rightw = 0;
	righth = 0;
	bottomx = 0;
	bottomy = 0;
	bottomw = 0;
	bottomh = 0;

	if ((topLeftRadius > 0) || (topRightRadius > 0)) {
		amt = (topLeftRadius > topRightRadius) ? topLeftRadius : topRightRadius;
		centery += amt;
		centerh -= amt;
		toph = amt;
		if (topLeftRadius > 0) {
			topx = topLeftRadius;
			topw -= topLeftRadius;
		}
		if (topRightRadius > 0) {
			topw -= topRightRadius;
		}
	}
	if ((topLeftRadius > 0) || (bottomLeftRadius > 0)) {
		amt = (topLeftRadius > bottomLeftRadius) ? topLeftRadius : bottomLeftRadius;
		centerx += amt;
		centerw -= amt;
		leftw = amt;
		if (topLeftRadius > 0) {
			lefty = topLeftRadius;
			lefth -= topLeftRadius;
		}
		if (bottomLeftRadius > 0) {
			lefth -= bottomLeftRadius;
		}
	}
	if ((topRightRadius > 0) || (bottomRightRadius > 0)) {
		amt = (topRightRadius > bottomRightRadius) ? topRightRadius : bottomRightRadius;
		centerw -= amt;
		rightx -= amt;
		rightw = amt;
		if (topRightRadius > 0) {
			righty = topRightRadius;
			righth -= topRightRadius;
		}
		if (bottomRightRadius > 0) {
			righth -= bottomRightRadius;
		}
	}
	if ((bottomLeftRadius > 0) || (bottomRightRadius > 0)) {
		amt = (bottomLeftRadius > bottomRightRadius) ? bottomLeftRadius : bottomRightRadius;
		centerh -= amt;
		bottomy -= amt;
		bottomh = amt;
		if (bottomLeftRadius > 0) {
			bottomx = bottomLeftRadius;
			bottomw -= bottomLeftRadius;
		}
		if (bottomRightRadius > 0) {
			bottomw -= bottomRightRadius;
		}
	}
	if ((topLeftRadius <= 0) && (leftw > 0) && (toph > 0)) {
		if (topRightRadius > 0) {
			lefty += topRightRadius;
			lefth -= topRightRadius;
		}
		else if (bottomLeftRadius > 0) {
			topx += bottomLeftRadius;
			topw -= bottomLeftRadius;
		}
	}
	if ((topRightRadius <= 0) && (rightw > 0) && (toph > 0)) {
		if (topLeftRadius > 0) {
			righty += topLeftRadius;
			righth -= topLeftRadius;
		}
		else if (bottomRightRadius > 0) {
			topw -= bottomRightRadius;
		}
	}
	if ((bottomLeftRadius <= 0) && (leftw > 0) && (bottomh > 0)) {
		if (topLeftRadius > 0) {
			bottomx += topLeftRadius;
			bottomw -= topLeftRadius;
		}
		else if (bottomRightRadius > 0) {
			lefth -= bottomRightRadius;
		}
	}
	if ((bottomRightRadius <= 0) && (rightw > 0) && (bottomh > 0)) {
		if (topRightRadius > 0) {
			bottomw -= topRightRadius;
		}
		else if (bottomLeftRadius > 0) {
			righty += bottomLeftRadius;
			righth -= bottomLeftRadius;
		}
	}

	cornerCenterDx = centerx;
	cornerCenterDy = centery;
	cornerCenterDw = centerw;
	cornerCenterDh = centerh;
	cornerTopDx = topx;
	cornerTopDy = topy;
	cornerTopDw = topw;
	cornerTopDh = toph;
	cornerLeftDx = leftx;
	cornerLeftDy = lefty;
	cornerLeftDw = leftw;
	cornerLeftDh = lefth;
	cornerRightDx = rightx;
	cornerRightDy = righty;
	cornerRightDw = rightw;
	cornerRightDh = righth;
	cornerBottomDx = bottomx;
	cornerBottomDy = bottomy;
	cornerBottomDw = bottomw;
	cornerBottomDh = bottomh;
	topLeftCornerRadius = topLeftRadius;
	topRightCornerRadius = topRightRadius;
	bottomLeftCornerRadius = bottomLeftRadius;
	bottomRightCornerRadius = bottomRightRadius;

	amt = topLeftRadius;
	if (topRightRadius > amt) {
		amt = topRightRadius;
	}
	if (bottomLeftRadius > amt) {
		amt = bottomLeftRadius;
	}
	if (bottomRightRadius > amt) {
		amt = bottomRightRadius;
	}
	cornerSize = amt * 2;
}

void Panel::setBorder (bool enable, const Color &color, double borderWidthValue) {
	if (enable) {
		borderColor.assign (color);
		if (borderWidthValue < 1.0f) {
			borderWidthValue = 1.0f;
		}
		borderWidth = borderWidthValue;
		isBordered = true;
	}
	else {
		isBordered = false;
	}
}

void Panel::setDropShadow (bool enable, const Color &color, double dropShadowWidthValue) {
	if (enable) {
		dropShadowColor.assign (color);
		if (dropShadowWidthValue < 1.0f) {
			dropShadowWidthValue = 1.0f;
		}
		dropShadowWidth = dropShadowWidthValue;
		isDropShadowed = true;
	}
	else {
		isDropShadowed = false;
	}
}

void Panel::doSetExtents () {
	// Default implementation does nothing
}

void Panel::setViewOrigin (double originX, double originY) {
	double x, y;

	x = originX;
	y = originY;
	if (isViewOriginBoundEnabled) {
		if (x < minViewOriginX) {
			x = minViewOriginX;
		}
		if (x > maxViewOriginX) {
			x = maxViewOriginX;
		}
		if (y < minViewOriginY) {
			y = minViewOriginY;
		}
		if (y > maxViewOriginY) {
			y = maxViewOriginY;
		}
	}
	if (FLOAT_EQUALS (viewOriginX, x) && FLOAT_EQUALS (viewOriginY, y)) {
		return;
	}
	viewOriginX = x;
	viewOriginY = y;
	doSetViewOrigin ();
}

void Panel::doSetViewOrigin () {
	// Default implementation does nothing
}

void Panel::setViewOriginBounds (double originX1, double originY1, double originX2, double originY2) {
	isViewOriginBoundEnabled = true;
	minViewOriginX = originX1;
	minViewOriginY = originY1;
	maxViewOriginX = originX2;
	maxViewOriginY = originY2;
	setViewOrigin (viewOriginX, viewOriginY);
}

void Panel::setFixedPadding (bool enable, double widthPaddingSize, double heightPaddingSize) {
	if (enable) {
		isFixedPadding = true;
		if (widthPaddingSize < 0.0f) {
			widthPaddingSize = 0.0f;
		}
		if (heightPaddingSize < 0.0f) {
			heightPaddingSize = 0.0f;
		}
		widthPadding = widthPaddingSize;
		heightPadding = heightPaddingSize;
	}
	else {
		isFixedPadding = false;
	}
}

void Panel::setPaddingScale (double widthScale, double heightScale) {
	if (widthScale < 0.0f) {
		widthScale = 0.0f;
	}
	if (heightScale < 0.0f) {
		heightScale = 0.0f;
	}
	widthPaddingScale = widthScale;
	heightPaddingScale = heightScale;
}

void Panel::setFixedSize (bool enable, double fixedWidth, double fixedHeight) {
	if (enable) {
		isFixedSize = true;
		width = fixedWidth;
		height = fixedHeight;
	}
	else {
		isFixedSize = false;
	}
}

void Panel::setWaiting (bool enable) {
	Panel *panel;
	ProgressBar *bar;

	if (isWaiting == enable) {
		return;
	}
	isWaiting = enable;
	if (isWaiting) {
		isInputSuspended = true;

		panel = (Panel *) addWidget (new Panel ());
		panel->setFixedSize (true, width, height);
		panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->waitingShadeAlpha));
		panel->zLevel = maxWidgetZLevel + 1;

		bar = (ProgressBar *) panel->addWidget (new ProgressBar (width, UiConfiguration::instance->progressBarHeight));
		bar->setIndeterminate (true);
		bar->position.assign (0.0f, height - bar->height);

		waitPanelHandle.assign (panel);
		waitProgressBarHandle.assign (bar);
	}
	else {
		isInputSuspended = false;
		waitPanelHandle.destroyAndClear ();
		waitProgressBarHandle.clear ();
	}
}

void Panel::syncRecordStore () {
	std::list<Widget *>::const_iterator i1, i2;
	Widget *widget;

	SDL_LockMutex (widgetListMutex);
	i1 = widgetList.cbegin ();
	i2 = widgetList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed) {
			continue;
		}
		widget->syncRecordStore ();
	}
	SDL_UnlockMutex (widgetListMutex);

	SDL_LockMutex (widgetAddListMutex);
	i1 = widgetAddList.cbegin ();
	i2 = widgetAddList.cend ();
	while (i1 != i2) {
		widget = *i1;
		++i1;
		if (widget->isDestroyed) {
			continue;
		}
		widget->syncRecordStore ();
	}
	SDL_UnlockMutex (widgetAddListMutex);
}

void Panel::setDetailSize (int detailSizeValue, double detailMaxWidthValue) {
	if ((detailSize == detailSizeValue) && FLOAT_EQUALS (detailMaxWidth, detailMaxWidthValue)) {
		return;
	}
	detailSize = detailSizeValue;
	detailMaxWidth = detailMaxWidthValue;
	refreshDetailSize ();
	reflow ();
}

double Panel::getDetailThumbnailScale () {
	double scale;

	constexpr const double defaultThumbnailScale = 0.25f;
	scale = 0.0f;
	switch (detailSize) {
		case Ui::SmallSize: {
			scale = UiConfiguration::instance->smallThumbnailImageScale;
			break;
		}
		case Ui::MediumSize: {
			scale = UiConfiguration::instance->mediumThumbnailImageScale;
			break;
		}
		case Ui::LargeSize: {
			scale = UiConfiguration::instance->largeThumbnailImageScale;
			break;
		}
	}
	return ((scale > 0.0f) ? scale : defaultThumbnailScale);
}

void Panel::refreshDetailSize () {
	// Default implementation does nothing
}

PanelLayoutFlow::PanelLayoutFlow ()
: x (0.0f)
, y (0.0f)
, xExtent (0.0f)
, yExtent (0.0f)
{
}
PanelLayoutFlow::~PanelLayoutFlow () {
}
