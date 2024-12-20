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
#include "Sprite.h"
#include "Widget.h"
#include "Label.h"
#include "Image.h"
#include "ProgressBar.h"
#include "IconLabelWindow.h"

IconLabelWindow::IconLabelWindow (Sprite *iconSprite, const StdString &iconText, UiConfiguration::FontType textFontType, const Color &textColor)
: Panel ()
, textLabelX (0.0f)
, textFontType (textFontType)
, iconSprite (iconSprite)
, isRightAligned (false)
, isTextChangeHighlightEnabled (false)
, progressBar (NULL)
{
	classId = ClassId::IconLabelWindow;
	setPaddingScale (1.0f, 0.0f);

	normalTextColor.assign (textColor);
	label = add (new Label (iconText, textFontType, normalTextColor));
	label->mouseClickCallback = Widget::EventCallbackContext (IconLabelWindow::labelClicked, this);

	image = add (new Image (iconSprite));
	image->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	reflow ();
}
IconLabelWindow::~IconLabelWindow () {
}

IconLabelWindow *IconLabelWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::IconLabelWindow) ? (IconLabelWindow *) widget : NULL);
}

void IconLabelWindow::setText (const StdString &text) {
	if (isTextChangeHighlightEnabled) {
		if ((! label->text.empty ()) && (! text.equals (label->text))) {
			label->textColor.assign (highlightTextColor);
			label->textColor.translate (normalTextColor, UiConfiguration::instance->longColorTranslateDuration);
		}
	}
	label->setText (text);
	reflow ();
}

StdString IconLabelWindow::getText () const {
	return (label->text);
}

void IconLabelWindow::labelClicked (void *itPtr, Widget *widgetPtr) {
	((IconLabelWindow *) itPtr)->eventCallback (((IconLabelWindow *) itPtr)->textClickCallback);
}

void IconLabelWindow::setTextColor (const Color &textColor) {
	label->textColor.assign (textColor);
	normalTextColor.assign (textColor);
}

void IconLabelWindow::setTextUnderlined (bool enable) {
	label->setUnderlined (enable);
	reflow ();
}

void IconLabelWindow::setTextFont (UiConfiguration::FontType fontType) {
	label->setFont (fontType);
	reflow ();
}

void IconLabelWindow::setTextChangeHighlight (bool enable, const Color &highlightColor) {
	isTextChangeHighlightEnabled = enable;
	if (isTextChangeHighlightEnabled) {
		highlightTextColor.assign (highlightColor);
	}
}

void IconLabelWindow::setRightAligned (bool enable) {
	if (enable == isRightAligned) {
		return;
	}
	isRightAligned = enable;
	reflow ();
}

void IconLabelWindow::setIconSprite (Sprite *sprite) {
	if (iconSprite == sprite) {
		return;
	}
	iconSprite = sprite;
	if (image) {
		image->isDestroyed = true;
	}
	image = add (new Image (iconSprite));
	image->setDrawColor (true, normalTextColor);
	reflow ();
}

void IconLabelWindow::setIconImageColor (const Color &imageColor) {
	image->setDrawColor (true, imageColor);
}

void IconLabelWindow::setIconImageFrame (int frame) {
	image->setFrame (frame);
	reflow ();
}

void IconLabelWindow::setIconImageScale (double scale) {
	image->setScale (scale);
	reflow ();
}

void IconLabelWindow::setProgressBar (bool enable, double progressValue, double targetProgressValue) {
	if (! enable) {
		if (progressBar) {
			progressBar->isDestroyed = true;
			progressBar = NULL;
		}
	}
	else {
		if (! progressBar) {
			progressBar = add (new ProgressBar ());
			progressBar->zLevel = 1;
		}
		if ((progressValue < 0.0f) && (targetProgressValue < 0.0f)) {
			progressBar->setIndeterminate (true);
		}
		else {
			progressBar->setIndeterminate (false);
			if (targetProgressValue >= 0.0f) {
				progressBar->setProgress (progressValue, targetProgressValue);
			}
			else {
				progressBar->setProgress (progressValue);
			}
		}
	}
	reflow ();
}

void IconLabelWindow::reflow () {
	double w, h;

	resetPadding ();
	topLeftLayoutFlow ();

	if (isRightAligned) {
		label->flowRight (&layoutFlow);
		layoutFlow.x -= (UiConfiguration::instance->marginSize / 2.0f);
		image->flowRight (&layoutFlow);
	}
	else {
		image->flowRight (&layoutFlow);
		layoutFlow.x -= (UiConfiguration::instance->marginSize / 2.0f);
		label->flowRight (&layoutFlow);
	}

	w = layoutFlow.xExtent + widthPadding;
	h = layoutFlow.yExtent;
	if (progressBar) {
		progressBar->setSize (w, UiConfiguration::instance->progressBarHeight);
		progressBar->position.assign (0.0f, h);
		h += progressBar->height;
	}
	else {
		h += heightPadding;
	}
	setFixedSize (true, w, h);

	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = height;
	image->centerVertical (&layoutFlow);
	label->centerVertical (&layoutFlow);
	textLabelX = label->position.x;
}
