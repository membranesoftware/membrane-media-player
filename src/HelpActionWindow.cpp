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
#include "Sprite.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Color.h"
#include "UiText.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextFlow.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "HyperlinkWindow.h"
#include "HelpActionWindow.h"

HelpActionWindow::HelpActionWindow (double windowWidth, Sprite *iconSprite, const StdString &helpActionText, const Color &helpActionTextColor, const StdString &helpLinkText, const StdString &helpLinkUrl, const StdString &helpLinkDisplayUrl)
: Panel ()
, windowWidth (windowWidth)
, linkWindow (NULL)
{
	iconImage = add (new Image (iconSprite));
	iconImage->setDrawColor (true, helpActionTextColor);

	actionText = add (new TextFlow (windowWidth - iconImage->width - (UiConfiguration::instance->paddingSize * 2.0f) - UiConfiguration::instance->marginSize, UiConfiguration::CaptionFont));
	actionText->setFixedPadding (true, 0.0f, 0.0f);
	actionText->setTextColor (helpActionTextColor);
	actionText->setText (helpActionText);

	if ((! helpLinkText.empty ()) && (! helpLinkUrl.empty ())) {
		linkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (helpLinkText, helpLinkUrl));
		linkWindow->linkOpenCallback = Widget::EventCallbackContext (HelpActionWindow::linkOpened, this);
		if (! helpLinkDisplayUrl.empty ()) {
			linkDisplayUrl.assign (helpLinkDisplayUrl);
		}
		else {
			linkDisplayUrl.assign (helpLinkUrl);
		}
	}

	setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	reflow ();
}
HelpActionWindow::~HelpActionWindow () {
}

void HelpActionWindow::setWindowWidth (double windowWidthValue) {
	if (FLOAT_EQUALS (windowWidthValue, windowWidth)) {
		return;
	}
	windowWidth = windowWidthValue;
	actionText->setViewWidth (windowWidth - iconImage->width - (UiConfiguration::instance->paddingSize * 2.0f));
	reflow ();
}

void HelpActionWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();
	iconImage->flowRight (&layoutFlow);
	actionText->flowDown (&layoutFlow);

	if (linkWindow) {
		layoutFlow.y += UiConfiguration::instance->marginSize;
		linkWindow->flowDown (&layoutFlow);
	}
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
	if (linkWindow) {
		bottomRightLayoutFlow ();
		linkWindow->flowLeft (&layoutFlow);
	}
}

void HelpActionWindow::linkOpened (void *itPtr, Widget *widgetPtr) {
	HelpActionWindow *it = (HelpActionWindow *) itPtr;

	if (! it->linkWindow) {
		return;
	}
	if (it->linkWindow->linkOpenResult != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenHelpUrlError));
	}
	else {
		App::instance->showNotification (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextId::LaunchedWebBrowser).capitalized ().c_str (), it->linkDisplayUrl.c_str ()));
	}
}
