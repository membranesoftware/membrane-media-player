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
#include "Label.h"
#include "Image.h"
#include "Toggle.h"
#include "ToggleWindow.h"

ToggleWindow::ToggleWindow (Toggle *toggle)
: Panel ()
, isChecked (false)
, isRightAligned (false)
, isInverseColor (false)
, toggle (toggle)
, textLabelHandle (&textLabel)
, iconImageHandle (&iconImage)
{
	classId = ClassId::ToggleWindow;

	addWidget (toggle);
	toggle->isInputSuspended = true;
	toggle->stateChangeCallback = Widget::EventCallbackContext (ToggleWindow::toggleStateChanged, this);

	mouseEnterCallback = Widget::EventCallbackContext (ToggleWindow::mouseEntered, this);
	mouseExitCallback = Widget::EventCallbackContext (ToggleWindow::mouseExited, this);
	mousePressCallback = Widget::EventCallbackContext (ToggleWindow::mousePressed, this);
	mouseReleaseCallback = Widget::EventCallbackContext (ToggleWindow::mouseReleased, this);
	mouseClickCallback = Widget::EventCallbackContext (ToggleWindow::mouseClicked, this);

	reflow ();
}

ToggleWindow::ToggleWindow (Toggle *toggle, const StdString &text)
: ToggleWindow (toggle)
{
	setText (text);
}

ToggleWindow::ToggleWindow (Toggle *toggle, Sprite *iconSprite)
: ToggleWindow (toggle)
{
	setIcon (iconSprite);
}

ToggleWindow::~ToggleWindow () {
}

ToggleWindow *ToggleWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ToggleWindow) ? (ToggleWindow *) widget : NULL);
}

void ToggleWindow::setText (const StdString &text) {
	textLabelHandle.destroyAndAssign (new Label (text, UiConfiguration::CaptionFont, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor));
	textLabel->isInputSuspended = true;
	add (textLabel);

	iconImageHandle.destroyAndClear ();
	reflow ();
}

void ToggleWindow::setIcon (Sprite *iconSprite) {
	iconImageHandle.destroyAndAssign (new Image (iconSprite));
	iconImage->setDrawColor (true, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
	iconImage->isInputSuspended = true;
	add (iconImage);

	textLabelHandle.destroyAndClear ();
	reflow ();
}

void ToggleWindow::setRightAligned (bool enable) {
	if (enable == isRightAligned) {
		return;
	}
	isRightAligned = enable;
	reflow ();
}

void ToggleWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	toggle->setInverseColor (isInverseColor);
	if (isInverseColor) {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
		}
		if (textLabel) {
			textLabel->textColor.assign (UiConfiguration::instance->inverseTextColor);
		}
		if (iconImage) {
			iconImage->drawColor.assign (UiConfiguration::instance->inverseTextColor);
		}
	}
	else {
		if (isFilledBg) {
			setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		}
		if (textLabel) {
			textLabel->textColor.assign (UiConfiguration::instance->primaryTextColor);
		}
		if (iconImage) {
			iconImage->drawColor.assign (UiConfiguration::instance->primaryTextColor);
		}
	}
}

void ToggleWindow::setImageColor (const Color &imageColor) {
	toggle->setImageColor (imageColor);
}

void ToggleWindow::setDisabled (bool disabled) {
	toggle->setDisabled (disabled);
}

void ToggleWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();
	if (isRightAligned) {
		if (textLabel || iconImage) {
			layoutFlow.x += UiConfiguration::instance->paddingSize;
		}
	}
	else {
		toggle->flowRight (&layoutFlow);
	}
	if (textLabel) {
		layoutFlow.x -= (UiConfiguration::instance->marginSize / 2.0f);
		textLabel->flowRight (&layoutFlow);
	}
	if (iconImage) {
		iconImage->flowRight (&layoutFlow);
	}
	if (isRightAligned) {
		toggle->flowRight (&layoutFlow);
	}

	resetSize ();
	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = height;
	if (textLabel) {
		textLabel->centerVertical (&layoutFlow);
	}
	if (iconImage) {
		iconImage->centerVertical (&layoutFlow);
	}
	toggle->centerVertical (&layoutFlow);
}

void ToggleWindow::mouseEntered (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;

	if (it->toggle->isDisabled) {
		return;
	}
	it->toggle->mouseEnter ();
}

void ToggleWindow::mouseExited (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;

	if (it->toggle->isDisabled) {
		return;
	}
	it->toggle->mouseExit ();
}

void ToggleWindow::mousePressed (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;

	if (it->toggle->isDisabled) {
		return;
	}
	it->toggle->mousePress ();
}

void ToggleWindow::mouseReleased (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;

	if (it->toggle->isDisabled) {
		return;
	}
	it->toggle->mouseRelease ();
	it->toggle->setFocused (it->isMouseEntered);
}

void ToggleWindow::mouseClicked (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;

	if (it->toggle->isDisabled) {
		return;
	}
	it->toggle->mouseClick ();
}

void ToggleWindow::setChecked (bool checked, bool shouldSkipChangeCallback) {
	toggle->setChecked (checked, shouldSkipChangeCallback);
	isChecked = toggle->isChecked;
}

void ToggleWindow::toggleStateChanged (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *it = (ToggleWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->isChecked = toggle->isChecked;
	it->eventCallback (it->stateChangeCallback);
}
