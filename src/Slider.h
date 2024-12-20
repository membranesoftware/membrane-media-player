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
// Widget that allows selection of a bounded numeric value
#ifndef SLIDER_H
#define SLIDER_H

#include "Color.h"
#include "Widget.h"

class Slider : public Widget {
public:
	Slider (double minValue = 0.0f, double maxValue = 1.0f);
	~Slider ();

	// Read-write data members
	Widget::EventCallbackContext valueChangeCallback;
	Widget::EventCallbackContext valueHoverCallback;

	// Read-only data members
	bool isDisabled;
	bool isInverseColor;
	bool isVerticalTrack;
	double thumbSize;
	double thumbWidth;
	double thumbHeight;
	double trackWidthScale;
	double value;
	double hoverValue;
	double minValue;
	double maxValue;
	bool isHovering;
	bool isDragging;

	// Set the slider's disabled state, appropriate for use when the slider becomes unavailable for interaction
	void setDisabled (bool disabled);

	// Set the slider's inverse color option
	void setInverseColor (bool inverse);

	// Set the slider's vertical track option
	void setVerticalTrack (bool vertical);

	// Set the slider's track width scale factor
	void setTrackWidthScale (double scale);

	// Set the slider's value, optionally skipping any configured value change callback
	void setValue (double sliderValue, bool shouldSkipChangeCallback = false);

	// Add the specified value as a snap position on the slider. If at least one snap position is present, changes to the slider value are rounded down to the nearest snap value.
	void addSnapValue (double snapValue);

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doDraw (double originX, double originY);
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	void doResetInputState ();
	void doResize ();

private:
	// Return the value indicated by the specified target after applying any configured snap positions
	double getSnappedValue (double targetValue);

	// Reset size values as appropriate for current state
	void resetSize ();

	// Reset draw colors as appropriate for current state
	void resetColors ();

	Color thumbColor;
	double trackWidth;
	double trackHeight;
	Color trackColor;
	double hoverSize;
	Color hoverColor;
	std::list<double> snapValueList;
};
#endif
