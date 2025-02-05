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
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"

LabelWindow::LabelWindow (Label *label)
: Panel ()
, isCrawlEnabled (false)
, label (label)
, isFixedWidth (false)
, isWidthCentered (false)
, isLinePositionEnabled (false)
, windowWidth (0.0f)
, isMouseoverHighlightEnabled (false)
, mouseoverColorTranslateDuration (0)
, crawlStage (0)
, crawlClock (0)
{
	classId = ClassId::LabelWindow;
	addWidget (label);
	reflow ();
}
LabelWindow::~LabelWindow () {
}

LabelWindow *LabelWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::LabelWindow) ? (LabelWindow *) widget : NULL);
}

void LabelWindow::setWindowWidth (double windowWidthValue, bool isWidthCenteredValue) {
	if (isFixedWidth && FLOAT_EQUALS (windowWidth, windowWidthValue) && (isWidthCentered == isWidthCenteredValue)) {
		return;
	}
	isFixedWidth = true;
	isWidthCentered = isWidthCenteredValue;
	windowWidth = windowWidthValue;
	reflow ();
}

void LabelWindow::setFitWidth () {
	isFixedWidth = false;
	isWidthCentered = false;
	reflow ();
}

void LabelWindow::setTextColor (const Color &color) {
	label->textColor.assign (color);
}

void LabelWindow::translateTextColor (const Color &targetColor, int durationMs) {
	label->textColor.translate (targetColor, durationMs);
}

void LabelWindow::translateTextColor (const Color &startColor, const Color &targetColor, int durationMs) {
	label->textColor.translate (startColor, targetColor, durationMs);
}

void LabelWindow::setShadowed (bool enable, const Color &shadowColor, int shadowDx, int shadowDy) {
	label->setShadowed (enable, shadowColor, shadowDx, shadowDy);
}

StdString LabelWindow::getText () const {
	return (label->text);
}

double LabelWindow::getTextWidth () const {
	return (label->width);
}

double LabelWindow::getMaxLineHeight () const {
	return (label->maxLineHeight);
}

void LabelWindow::setText (const StdString &text) {
	label->setText (text);
	reflow ();
}

void LabelWindow::setFont (UiConfiguration::FontType fontType) {
	label->setFont (fontType);
	reflow ();
}

void LabelWindow::setLinePosition (bool enable) {
	if (isLinePositionEnabled == enable) {
		return;
	}
	isLinePositionEnabled = enable;
	reflow ();
}

void LabelWindow::setMouseoverHighlight (bool enable, const Color &normalTextColor, const Color &normalBgColor, const Color &highlightTextColor, const Color &highlightBgColor, int colorTranslateDuration) {
	isMouseoverHighlightEnabled = enable;
	if (isMouseoverHighlightEnabled) {
		mouseoverNormalTextColor = normalTextColor;
		mouseoverNormalBgColor = normalBgColor;
		mouseoverHighlightTextColor = highlightTextColor;
		mouseoverHighlightBgColor = highlightBgColor;
		mouseoverColorTranslateDuration = colorTranslateDuration;
		if (mouseoverColorTranslateDuration < 1) {
			mouseoverColorTranslateDuration = 1;
		}
		setFillBg (true, mouseoverNormalBgColor);
	}
}

void LabelWindow::setCrawl (bool enable) {
	if (isCrawlEnabled == enable) {
		return;
	}
	isCrawlEnabled = enable;
	label->position.assignX (widthPadding);
	if (isCrawlEnabled) {
		crawlStage = 0;
	}
}

bool LabelWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (isMouseoverHighlightEnabled) {
		if (mouseState.isEntered) {
			label->textColor.translate (mouseoverHighlightTextColor, mouseoverColorTranslateDuration);
			bgColor.translate (mouseoverHighlightBgColor, mouseoverColorTranslateDuration);
		}
		else {
			label->textColor.translate (mouseoverNormalTextColor, mouseoverColorTranslateDuration);
			bgColor.translate (mouseoverNormalBgColor, mouseoverColorTranslateDuration);
		}
	}

	return (false);
}

void LabelWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isCrawlEnabled) {
		switch (crawlStage) {
			case 0: {
				crawlClock = UiConfiguration::instance->textCrawlRepeatDuration;
				crawlStage = 1;
				break;
			}
			case 1: {
				crawlClock -= msElapsed;
				if (crawlClock <= 0) {
					crawlStage = 2;
					label->position.translateX (-(label->width), UiConfiguration::instance->textCrawlAdvanceDuration * (int) label->text.length ());
				}
				break;
			}
			case 2: {
				if (! label->position.isTranslating) {
					crawlStage = 0;
					label->position.assignX (widthPadding);
				}
				break;
			}
		}
	}
}

void LabelWindow::reflow () {
	double x, y, w, h;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	if (isLinePositionEnabled) {
		label->position.assign (x, label->getLinePosition (y));
		h = label->position.y + label->height + (heightPadding * 2.0f);
	}
	else {
		h = label->maxLineHeight + (heightPadding * 2.0f);
		label->position.assign (x, (h / 2.0f) - (label->height / 2.0f));
	}
	if (isFixedWidth) {
		w = windowWidth;
		if (isWidthCentered) {
			label->position.assignX ((w / 2.0f) - (label->width / 2.0f));
		}
	}
	else {
		w = label->width;
		w += (widthPadding * 2.0f);
	}
	setFixedSize (true, w, h);
}
