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
#include "UiText.h"
#include "Label.h"
#include "Widget.h"
#include "TextFlow.h"
#include "Image.h"
#include "Button.h"
#include "HyperlinkWindow.h"
#include "TextCardWindow.h"

const UiConfiguration::FontType TextCardWindow::titleTextFont = UiConfiguration::HeadlineFont;
const UiConfiguration::FontType TextCardWindow::subtitleTextFont = UiConfiguration::CaptionFont;
const UiConfiguration::FontType TextCardWindow::detailTextFont = UiConfiguration::CaptionFont;

TextCardWindow::TextCardWindow ()
: Panel ()
, iconImageHandle (&iconImage)
, titleLabelHandle (&titleLabel)
, subtitleLabelHandle (&subtitleLabel)
, detailTextHandle (&detailText)
, actionButtonHandle (&actionButton)
, linkWindowHandle (&linkWindow)
{
	classId = ClassId::TextCardWindow;
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	reflow ();
}
TextCardWindow::~TextCardWindow () {
}

void TextCardWindow::setTitleText (const StdString &text) {
	titleLabelHandle.destroyAndAssign (new Label (text, TextCardWindow::titleTextFont, UiConfiguration::instance->primaryTextColor));
	addWidget (titleLabel);
	reflow ();
}
void TextCardWindow::setTitleText (const StdString &text, const Color &textColor) {
	setTitleText (text);
	titleLabel->textColor.assign (textColor);
}

void TextCardWindow::setTitleIcon (Sprite *iconSprite) {
	iconImageHandle.destroyAndAssign (new Image (iconSprite));
	iconImage->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	addWidget (iconImage);
	reflow ();
}

void TextCardWindow::setSubtitleText (const StdString &text) {
	subtitleLabelHandle.destroyAndAssign (new Label (text, TextCardWindow::subtitleTextFont, UiConfiguration::instance->primaryTextColor));
	addWidget (subtitleLabel);
	reflow ();
}
void TextCardWindow::setSubtitleText (const StdString &text, const Color &textColor) {
	setSubtitleText (text);
	subtitleLabel->textColor.assign (textColor);
}

void TextCardWindow::setActionButton (const StdString &text) {
	actionButtonHandle.destroyAndAssign (new Button (text));
	actionButton->mouseClickCallback = Widget::EventCallbackContext (actionButtonClicked, this);
	actionButton->setTextColor (UiConfiguration::instance->buttonTextColor);
	addWidget (actionButton);
	reflow ();
}
void TextCardWindow::setActionButton (Sprite *iconSprite) {
	actionButtonHandle.destroyAndAssign (new Button (iconSprite));
	actionButton->mouseClickCallback = Widget::EventCallbackContext (actionButtonClicked, this);
	actionButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	addWidget (actionButton);
	reflow ();
}

void TextCardWindow::actionButtonClicked (void *itPtr, Widget *widgetPtr) {
	((TextCardWindow *) itPtr)->eventCallback (((TextCardWindow *) itPtr)->actionButtonClickCallback);
}

void TextCardWindow::setDetailText (const StdString &text) {
	detailTextHandle.destroyAndAssign (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[TextCardWindow::detailTextFont]->maxGlyphWidth, TextCardWindow::detailTextFont));
	detailText->setText (text);
	addWidget (detailText);
	reflow ();
}
void TextCardWindow::setDetailText (const StdString &text, const Color &textColor) {
	setDetailText (text);
	detailText->setTextColor (textColor);
}

void TextCardWindow::setLink (const StdString &text, const StdString &url) {
	linkWindowHandle.destroyAndAssign (new HyperlinkWindow (text, url));
	linkWindow->linkOpenCallback = Widget::EventCallbackContext (TextCardWindow::linkWindowOpened, this);
	addWidget (linkWindow);
	reflow ();
}

void TextCardWindow::linkWindowOpened (void *itPtr, Widget *widgetPtr) {
	((TextCardWindow *) itPtr)->eventCallback (((TextCardWindow *) itPtr)->linkOpenCallback);
}

void TextCardWindow::reflow () {
	double w1, w2;

	resetPadding ();
	topLeftLayoutFlow ();

	if (iconImage) {
		iconImage->flowRight (&layoutFlow);
	}
	if (titleLabel && subtitleLabel) {
		titleLabel->flowRight (&layoutFlow);
		layoutFlow.x = titleLabel->position.x;
		w1 = titleLabel->maxGlyphWidth;
		w2 = subtitleLabel->maxGlyphWidth;
		layoutFlow.x += ((w1 - w2) * 0.25f);
		layoutFlow.y += titleLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		subtitleLabel->flowRight (&layoutFlow);
	}
	else if (titleLabel) {
		titleLabel->flowRight (&layoutFlow);
		titleLabel->centerVertical (&layoutFlow);
	}
	else if (subtitleLabel) {
		subtitleLabel->flowRight (&layoutFlow);
		subtitleLabel->centerVertical (&layoutFlow);
	}

	nextRowLayoutFlow ();
	if (detailText) {
		detailText->flowDown (&layoutFlow);
	}

	nextRowLayoutFlow ();
	if (actionButton) {
		actionButton->flowRight (&layoutFlow);
	}
	if (linkWindow) {
		linkWindow->flowRight (&layoutFlow);
	}

	resetSize ();

	bottomRightLayoutFlow ();
	if (linkWindow) {
		linkWindow->flowLeft (&layoutFlow);
	}
	if (actionButton) {
		actionButton->flowLeft (&layoutFlow);
		if (linkWindow) {
			layoutFlow.y = actionButton->position.y;
			layoutFlow.yExtent = actionButton->position.y + actionButton->height;
			linkWindow->centerVertical (&layoutFlow);
		}
	}
}

void TextCardWindow::doResize () {
	if (detailText) {
		detailText->setViewWidth (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[TextCardWindow::detailTextFont]->maxGlyphWidth);
	}
	Panel::doResize ();
}
