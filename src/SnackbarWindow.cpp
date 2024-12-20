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
#include "Button.h"
#include "UiConfiguration.h"
#include "SnackbarWindow.h"

SnackbarWindow::SnackbarWindow (double maxWidth)
: Panel ()
, maxWidth (maxWidth)
, timeoutClock (0)
, isTimeoutStarted (false)
, isTimeoutSuspended (false)
, isTimeoutDismissed (false)
, scrollDuration (0)
{
	classId = ClassId::SnackbarWindow;

	backgroundPanel = add (new Panel ());
	backgroundPanel->mouseClickCallback = Widget::EventCallbackContext (SnackbarWindow::backgroundPanelClicked, this);
	backgroundPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));

	messageLabel = backgroundPanel->add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor), 1);

	actionButton = backgroundPanel->add (new Button (), 2);
	actionButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	actionButton->setTextColor (UiConfiguration::instance->buttonTextColor);
	actionButton->isVisible = false;

	backgroundPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	reflow ();
}
SnackbarWindow::~SnackbarWindow () {
}

void SnackbarWindow::setMaxWidth (double maxWidthValue) {
	maxWidth = maxWidthValue;
	reflow ();
}

void SnackbarWindow::setMessageText (const StdString &messageText) {
	messageLabel->setText (messageText);
	reflow ();
}

void SnackbarWindow::setActionButtonEnabled (bool enable, const StdString &buttonText, Widget::EventCallbackContext buttonClickCallback) {
	if (enable && (! buttonText.empty ()) && buttonClickCallback.callback) {
		actionButton->setText (buttonText);
		actionButton->mouseClickCallback = buttonClickCallback;
		actionButton->isVisible = true;
	}
	else {
		actionButton->isVisible = false;
	}
	reflow ();
}

void SnackbarWindow::startScroll (int duration) {
	reflow ();

	scrollDuration = duration;
	if (scrollDuration < 1) {
		scrollDuration = 1;
	}
	backgroundPanel->position.assign (0.0f, -(backgroundPanel->height));
	backgroundPanel->position.translate (0.0f, 0.0f, scrollDuration);
}

void SnackbarWindow::startTimeout (int timeout) {
	if (timeout < 1) {
		timeout = 1;
	}
	isTimeoutStarted = true;
	isTimeoutDismissed = false;
	isInputSuspended = false;
	timeoutClock = timeout;
}

void SnackbarWindow::backgroundPanelClicked (void *itPtr, Widget *widgetPtr) {
	((SnackbarWindow *) itPtr)->isTimeoutDismissed = true;
}

void SnackbarWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isVisible && isTimeoutStarted) {
		if (timeoutClock > 0) {
			if (! isTimeoutSuspended) {
				timeoutClock -= msElapsed;
			}
			if (isTimeoutDismissed) {
				timeoutClock = 0;
				isTimeoutDismissed = false;
			}

			if (timeoutClock <= 0) {
				backgroundPanel->position.translate (0.0f, -(backgroundPanel->height), scrollDuration);
				isInputSuspended = true;
			}
		}
		else {
			if (! backgroundPanel->position.isTranslating) {
				isDestroyed = true;
			}
		}
	}
}

void SnackbarWindow::doResize () {
	Panel::doResize ();
	if (isTimeoutStarted && (timeoutClock > 0)) {
		backgroundPanel->position.assign (0.0f, 0.0f);
	}
}

void SnackbarWindow::reflow () {
	double x, y, w;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	backgroundPanel->reflow ();
	backgroundPanel->position.assign (x, y);
	w = backgroundPanel->width;
	if (w > maxWidth) {
		w = maxWidth;
	}
	setFixedSize (true, w, backgroundPanel->height);
}

bool SnackbarWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);

	if (mouseState.isEntered) {
		isTimeoutSuspended = true;
	}
	else {
		isTimeoutSuspended = false;
	}
	return (false);
}