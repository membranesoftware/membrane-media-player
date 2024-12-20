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
#include "ScrollBar.h"
#include "ScrollView.h"
#include "ScrollViewWindow.h"

ScrollViewWindow::ScrollViewWindow ()
: Panel ()
, scrollViewWidth (0.0f)
, scrollViewHeight (0.0f)
, scrollViewOriginX (0.0f)
, scrollViewOriginY (0.0f)
, scrollViewExtentX2 (0.0f)
, scrollViewExtentY2 (0.0f)
, scrollViewBottomPadding (0.0f)
, scrollBarPosition (ScrollViewWindow::LeftEdgeScrollBarPosition)
, isScrollBarVerticalMarginEnabled (false)
{
	classId = ClassId::ScrollViewWindow;

	windowScrollView = add (new ScrollView ());
	windowScrollBar = add (new ScrollBar (1.0f), 1);
	windowScrollBar->positionChangeCallback = Widget::EventCallbackContext (ScrollViewWindow::scrollBarPositionChanged, this);
	windowScrollBar->isVisible = false;
}
ScrollViewWindow::~ScrollViewWindow () {
}

void ScrollViewWindow::setViewSize (double viewWidth, double viewHeight) {
	if (FLOAT_EQUALS (viewWidth, width) && FLOAT_EQUALS (viewHeight, height)) {
		return;
	}
	setFixedSize (true, viewWidth, viewHeight);
	windowScrollView->setViewSize (width, height);
	scrollViewWidth = width;
	scrollViewHeight = height;
}

void ScrollViewWindow::setViewLayout (int layoutType, double layoutSpacingValue) {
	windowScrollView->setLayout (layoutType);
	if (layoutSpacingValue >= 0.0f) {
		windowScrollView->layoutSpacing = layoutSpacingValue;
	}
}

void ScrollViewWindow::setViewPaddingScale (double widthScale, double heightScale) {
	windowScrollView->setPaddingScale (widthScale, heightScale);
	reflow ();
}

void ScrollViewWindow::setViewBottomPadding (double paddingSize) {
	if (paddingSize < 0.0f) {
		paddingSize = 0.0f;
	}
	scrollViewBottomPadding = paddingSize;
	resetScrollBounds ();
}

void ScrollViewWindow::setScrollOptions (int scrollOptions) {
	windowScrollView->isKeyboardScrollEnabled = (scrollOptions & ScrollViewWindow::KeyboardScrollOption) != 0;
	windowScrollView->isMouseWheelScrollEnabled = (scrollOptions & ScrollViewWindow::MouseWheelScrollOption) != 0;
	windowScrollView->isExitedMouseWheelScrollEnabled = (scrollOptions & ScrollViewWindow::ExitedMouseWheelScrollOption) != 0;
}

void ScrollViewWindow::setScrollBarPosition (int positionType, bool enableVerticalMargin) {
	scrollBarPosition = positionType;
	isScrollBarVerticalMarginEnabled = enableVerticalMargin;
	reflow ();
}

Widget *ScrollViewWindow::addViewItem (Widget *itemWidget) {
	windowScrollView->addWidget (itemWidget);
	return (itemWidget);
}

static void clearViewItems_processWidget (void *data, Widget *widgetPtr) {
	widgetPtr->isDestroyed = true;
}
void ScrollViewWindow::clearViewItems () {
	windowScrollView->processWidgets (clearViewItems_processWidget, NULL);
	reflow ();
}

void ScrollViewWindow::processViewItems (Widget::EventCallback fn, void *fnData) {
	windowScrollView->processWidgets (fn, fnData);
}

void ScrollViewWindow::setViewScrollOrigin (double positionX, double positionY, int translateDuration) {
	windowScrollView->setScrollOrigin (positionX, positionY, translateDuration);
}

void ScrollViewWindow::scrollToItem (Widget *itemWidget, int scrollPositionType, int scrollTranslateDuration) {
	double y1, y2;

	switch (scrollPositionType) {
		case ScrollViewWindow::NearestScrollPosition: {
			y1 = itemWidget->position.y;
			y2 = y1 + itemWidget->height;
			if (y1 < windowScrollView->viewOriginY) {
				windowScrollView->setScrollOrigin (0.0f, y1, scrollTranslateDuration);
			}
			else if (y2 > (windowScrollView->viewOriginY + windowScrollView->height)) {
				windowScrollView->setScrollOrigin (0.0f, y2 - windowScrollView->height, scrollTranslateDuration);
			}
			break;
		}
		case ScrollViewWindow::BottomScrollPosition: {
			windowScrollView->setScrollOrigin (0.0f, itemWidget->position.y - windowScrollView->height + itemWidget->height, scrollTranslateDuration);
			break;
		}
		case ScrollViewWindow::CenterScrollPosition: {
			windowScrollView->setScrollOrigin (0.0f, itemWidget->position.y - (windowScrollView->height / 2.0f) + (itemWidget->height / 2.0f), scrollTranslateDuration);
			break;
		}
		default: {
			windowScrollView->setScrollOrigin (0.0f, itemWidget->position.y, scrollTranslateDuration);
			break;
		}
	}
}

void ScrollViewWindow::scrollToTop () {
	windowScrollView->setViewOrigin (0.0f, 0.0f);
}

bool ScrollViewWindow::isScrolledToBottom (double marginHeight) {
	return (windowScrollView->isScrolledToBottom (marginHeight));
}

void ScrollViewWindow::scrollToBottom () {
	windowScrollView->setViewOrigin (0.0f, scrollViewExtentY2);
}

void ScrollViewWindow::reflow () {
	double x, w, tracklength;

	windowScrollView->reflow ();
	scrollViewExtentX2 = windowScrollView->extentX2;
	scrollViewExtentY2 = windowScrollView->extentY2 + scrollViewBottomPadding;
	x = 0.0f;
	w = width;
	if ((scrollViewExtentY2 - windowScrollView->extentY1 + windowScrollView->heightPadding) <= height) {
		windowScrollBar->isVisible = false;
	}
	else {
		tracklength = height;
		if (isScrollBarVerticalMarginEnabled) {
			tracklength -= (UiConfiguration::instance->paddingSize * 2.0f);
		}
		windowScrollBar->setMaxTrackLength (tracklength);
		windowScrollBar->setScrollBounds (height, scrollViewExtentY2);
		switch (scrollBarPosition) {
			case ScrollViewWindow::LeftInsetScrollBarPosition: {
				windowScrollBar->position.assign (UiConfiguration::instance->paddingSize, (height / 2.0f) - (windowScrollBar->height / 2.0f));
				x += (windowScrollBar->width + UiConfiguration::instance->paddingSize + (UiConfiguration::instance->marginSize / 2.0f));
				w -= (windowScrollBar->width + UiConfiguration::instance->paddingSize + (UiConfiguration::instance->marginSize / 2.0f));
				break;
			}
			case ScrollViewWindow::RightInsetScrollBarPosition: {
				windowScrollBar->position.assign (width - windowScrollBar->width - UiConfiguration::instance->paddingSize, (height / 2.0f) - (windowScrollBar->height / 2.0f));
				w -= (windowScrollBar->width + UiConfiguration::instance->paddingSize + (UiConfiguration::instance->marginSize / 2.0f));
				break;
			}
			case ScrollViewWindow::RightEdgeScrollBarPosition: {
				windowScrollBar->position.assign (width - windowScrollBar->width, (height / 2.0f) - (windowScrollBar->height / 2.0f));
				w -= (windowScrollBar->width + (UiConfiguration::instance->marginSize / 2.0f));
				break;
			}
			default: {
				windowScrollBar->position.assign (0.0f, (height / 2.0f) - (windowScrollBar->height / 2.0f));
				x += (windowScrollBar->width + (UiConfiguration::instance->marginSize / 2.0f));
				w -= (windowScrollBar->width + (UiConfiguration::instance->marginSize / 2.0f));
				break;
			}
		}
		windowScrollBar->isVisible = true;
	}

	if ((! FLOAT_EQUALS (windowScrollView->width, w)) || (! FLOAT_EQUALS (windowScrollView->height, height))) {
		windowScrollView->setViewSize (w, height);
		windowScrollView->reflow ();
		scrollViewWidth = w;
		scrollViewHeight = height;
	}
	windowScrollView->position.assign (x, 0.0f);
	resetScrollBounds ();
	windowScrollBar->setPosition (windowScrollView->viewOriginY, true);
}

void ScrollViewWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	resetScrollBounds ();
	windowScrollBar->setPosition (windowScrollView->viewOriginY, true);
	scrollViewOriginX = windowScrollView->viewOriginX;
	scrollViewOriginY = windowScrollView->viewOriginY;
}

void ScrollViewWindow::resetScrollBounds () {
	scrollViewExtentX2 = windowScrollView->extentX2;
	scrollViewExtentY2 = windowScrollView->extentY2 + scrollViewBottomPadding;
	if ((scrollViewExtentY2 - windowScrollView->extentY1 + windowScrollView->heightPadding) <= height) {
		windowScrollView->setVerticalScrollBounds (windowScrollView->extentY1 - windowScrollView->heightPadding, windowScrollView->extentY1 - windowScrollView->heightPadding);
	}
	else {
		windowScrollView->setVerticalScrollBounds (windowScrollView->extentY1 - windowScrollView->heightPadding, scrollViewExtentY2 - height + windowScrollView->heightPadding);
	}
}

void ScrollViewWindow::scrollBarPositionChanged (void *itPtr, Widget *widgetPtr) {
	ScrollViewWindow *it = (ScrollViewWindow *) itPtr;
	ScrollBar *scrollbar = (ScrollBar *) widgetPtr;

	it->windowScrollView->setScrollOrigin (0.0f, scrollbar->scrollPosition);
	it->reflow ();
}
