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
#include "LabelWindow.h"
#include "Image.h"
#include "UiConfiguration.h"
#include "Slider.h"
#include "SliderWindow.h"

SliderWindow::SliderWindow (Slider *slider)
: Panel ()
, isDisabled (false)
, isInverseColor (false)
, isVerticalTrack (false)
, value (0.0f)
, isHovering (false)
, slider (slider)
, iconImage (NULL)
, valueNameFunction (NULL)
{
	classId = ClassId::SliderWindow;
	value = slider->value;
	addWidget (slider);
	valueLabel = add (new LabelWindow (new Label (StdString::createSprintf ("%.2f", slider->value), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor)));
	valueLabel->setFixedPadding (true, 0.0f, 0.0f);
	valueLabel->isVisible = false;

	slider->valueChangeCallback = Widget::EventCallbackContext (SliderWindow::sliderValueChanged, this);
	slider->valueHoverCallback = Widget::EventCallbackContext (SliderWindow::sliderValueHovered, this);
	isVerticalTrack = slider->isVerticalTrack;

	reflow ();
}
SliderWindow::~SliderWindow () {
}

SliderWindow *SliderWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::SliderWindow) ? (SliderWindow *) widget : NULL);
}

void SliderWindow::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	slider->setDisabled (isDisabled);
	reflow ();
}

void SliderWindow::setInverseColor (bool inverse) {
	if (isInverseColor == inverse) {
		return;
	}
	isInverseColor = inverse;
	slider->setInverseColor (isInverseColor);
	if (iconImage) {
		iconImage->setDrawColor (true, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
	}
	reflow ();
}

void SliderWindow::setVerticalTrack (bool vertical) {
	if (isVerticalTrack == vertical) {
		return;
	}
	isVerticalTrack = vertical;
	slider->setVerticalTrack (isVerticalTrack);
	reflow ();
}

void SliderWindow::setIcon (Sprite *iconSprite) {
	if (iconImage) {
		iconImage->isDestroyed = true;
	}
	iconImage = add (new Image (iconSprite));
	iconImage->setDrawColor (true, isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
	iconImage->isInputSuspended = true;
	reflow ();
}

void SliderWindow::reflow () {
	Color color;

	resetPadding ();
	topLeftLayoutFlow ();
	if (iconImage) {
		iconImage->flowRight (&layoutFlow);
	}
	if (valueLabel->isVisible) {
		valueLabel->flowRight (&layoutFlow);
		slider->position.assign (valueLabel->position.x, layoutFlow.y + valueLabel->height + (UiConfiguration::instance->marginSize / 2.0f));
	}
	else {
		slider->flowRight (&layoutFlow);
	}
	resetSize ();

	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = height;
	if (iconImage) {
		iconImage->centerVertical (&layoutFlow);
	}

	if (valueLabel->isVisible) {
		if (isDisabled) {
			color.assign (isInverseColor ? UiConfiguration::instance->darkInverseTextColor : UiConfiguration::instance->lightPrimaryTextColor);
		}
		else {
			color.assign (isInverseColor ? UiConfiguration::instance->darkBackgroundColor : UiConfiguration::instance->lightPrimaryColor);
		}
		valueLabel->translateTextColor (color, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		slider->centerVertical (&layoutFlow);
	}
}

void SliderWindow::setValueNameFunction (SliderWindow::ValueNameFunction fn, double valueNameLabelWidth) {
	valueNameFunction = fn;
	if (! valueNameFunction) {
		valueLabel->isVisible = false;
	}
	else {
		valueLabel->setText (valueNameFunction (slider->value));
		if (valueNameLabelWidth > 0.0f) {
			valueLabel->setWindowWidth (valueNameLabelWidth);
		}
		else {
			valueLabel->setFitWidth ();
		}
		valueLabel->isVisible = true;
	}
	reflow ();
}

void SliderWindow::setValue (double sliderValue, bool shouldSkipChangeCallback) {
	slider->setValue (sliderValue, shouldSkipChangeCallback);
	value = slider->value;
	if (shouldSkipChangeCallback && valueNameFunction) {
		valueLabel->setText (valueNameFunction (slider->value));
		reflow ();
	}
}

void SliderWindow::setTrackWidthScale (double scale) {
	slider->setTrackWidthScale (scale);
	reflow ();
}

void SliderWindow::addSnapValue (double snapValue) {
	slider->addSnapValue (snapValue);
}

void SliderWindow::sliderValueChanged (void *itPtr, Widget *widgetPtr) {
	SliderWindow *it = (SliderWindow *) itPtr;
	Slider *slider = (Slider *) widgetPtr;

	it->value = slider->value;
	if (it->valueNameFunction) {
		it->valueLabel->setText (it->valueNameFunction (slider->value));
	}
	it->reflow ();
	it->eventCallback (it->valueChangeCallback);
}

void SliderWindow::sliderValueHovered (void *itPtr, Widget *widgetPtr) {
	SliderWindow *it = (SliderWindow *) itPtr;
	Slider *slider = (Slider *) widgetPtr;
	double val;

	it->isHovering = slider->isHovering;
	if (slider->isHovering) {
		val = slider->hoverValue;
	}
	else {
		val = slider->value;
	}

	if (it->valueNameFunction) {
		it->valueLabel->setText (it->valueNameFunction (val));
	}
	it->reflow ();
}
