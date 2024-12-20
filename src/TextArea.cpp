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
#include "Color.h"
#include "Widget.h"
#include "UiConfiguration.h"
#include "ScrollBar.h"
#include "TextFlow.h"
#include "TextArea.h"

TextArea::TextArea (double viewWidth, double viewHeight, UiConfiguration::FontType fontType, bool isScrollEnabled)
: ScrollView ()
, textFontType (fontType)
, scrollBar (NULL)
{
	double textw;

	textw = viewWidth;
	if (isScrollEnabled) {
		isMouseWheelScrollEnabled = true;
		scrollBar = add (new ScrollBar (1.0f), 3);
		scrollBar->positionChangeCallback = Widget::EventCallbackContext (TextArea::scrollBarPositionChanged, this);
		textw -= (scrollBar->width + UiConfiguration::instance->marginSize);
	}
	textFlow = add (new TextFlow (textw, textFontType));

	setViewSize (viewWidth, viewHeight);
	setInverseColor (false);
	reflow ();
}
TextArea::~TextArea () {
}

void TextArea::setViewSize (double viewWidth, double viewHeight) {
	ScrollView::setViewSize (viewWidth, viewHeight);
	textFlow->setViewWidth (viewWidth);
	reflow ();
}

void TextArea::setFont (UiConfiguration::FontType fontType) {
	if (fontType == textFontType) {
		return;
	}
	textFontType = fontType;
	textFlow->setFont (textFontType);
	reflow ();
}

void TextArea::setTextColor (const Color &color) {
	textFlow->setTextColor (color);
}

void TextArea::setInverseColor (bool isInverseColor) {
	if (isInverseColor) {
		setFillBg (true, UiConfiguration::instance->mediumInverseBackgroundColor);
		setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
		setTextColor (UiConfiguration::instance->inverseTextColor);
	}
	else {
		setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		setBorder (true, UiConfiguration::instance->darkBackgroundColor);
		setTextColor (UiConfiguration::instance->primaryTextColor);
	}
}

void TextArea::setMaxLineCount (int max) {
	if (max < 0) {
		max = 0;
	}
	textFlow->maxLineCount = max;
}

void TextArea::setText (const StdString &textContent, UiConfiguration::FontType fontType, bool forceFontReload) {
	textFlow->setText (textContent, fontType, forceFontReload);
	reflow ();
}

void TextArea::appendText (const StdString &textContent, bool scrollToBottom) {
	textFlow->appendText (textContent);
	reflow ();
	if (scrollBar && scrollToBottom) {
		setViewOrigin (0.0f, maxViewOriginY);
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY);
	}
}

void TextArea::reflow () {
	double x, y;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	textFlow->position.assign (x, y);

	y += textFlow->height;
	y -= height;
	if (y < 0.0f) {
		y = 0.0f;
	}
	setVerticalScrollBounds (0.0f, y);
	if (viewOriginY < 0.0f) {
		setViewOrigin (0.0f, 0.0f);
	}
	else if (viewOriginY > y) {
		setViewOrigin (0.0f, y);
	}

	if (scrollBar) {
		scrollBar->setMaxTrackLength (height);
		scrollBar->setScrollBounds (height, textFlow->height + (heightPadding * 2.0f));
		scrollBar->position.assign (width - scrollBar->width, viewOriginY);
		if (scrollBar->maxScrollPosition <= 0.0f) {
			isMouseWheelScrollEnabled = false;
			scrollBar->isInputSuspended = true;
		}
		else {
			isMouseWheelScrollEnabled = true;
			scrollBar->isInputSuspended = false;
		}
	}
}

void TextArea::scrollBarPositionChanged (void *itPtr, Widget *widgetPtr) {
	TextArea *it = (TextArea *) itPtr;
	ScrollBar *scrollbar = (ScrollBar *) widgetPtr;

	it->setViewOrigin (0.0f, scrollbar->scrollPosition);
	scrollbar->position.assignY (it->viewOriginY);
}

void TextArea::doUpdate (int msElapsed) {
	ScrollView::doUpdate (msElapsed);
	if (scrollBar) {
		scrollBar->setPosition (viewOriginY, true);
		scrollBar->position.assignY (viewOriginY);
	}
}
