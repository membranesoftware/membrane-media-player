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
// Panel that contains an image and a text label
#ifndef ICON_LABEL_WINDOW_H
#define ICON_LABEL_WINDOW_H

#include "UiConfiguration.h"
#include "Color.h"
#include "Panel.h"

class Sprite;
class Image;
class Label;
class ProgressBar;

class IconLabelWindow : public Panel {
public:
	IconLabelWindow (Sprite *iconSprite, const StdString &iconText = StdString (), UiConfiguration::FontType textFontType = UiConfiguration::BodyFont, const Color &textColor = Color (0.0f, 0.0f, 0.0f));
	~IconLabelWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static IconLabelWindow *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext textClickCallback;

	// Read-only data members
	double textLabelX;
	UiConfiguration::FontType textFontType;

	// Set the window's text
	void setText (const StdString &labelText);

	// Return the window's text string
	StdString getText () const;

	// Set the window's text color
	void setTextColor (const Color &textColor);

	// Set the window's underlined state
	void setTextUnderlined (bool enable);

	// Set the window's text font
	void setTextFont (UiConfiguration::FontType fontType);

	// Set the window's text change highlight option. If enabled, the window highlights text changes with a rotation from the specified color.
	void setTextChangeHighlight (bool enable, const Color &highlightColor = Color (0.0f, 0.0f, 0.0f));

	// Set the window's right-aligned option. If enabled, the window places the icon on the right side instead of the left side.
	void setRightAligned (bool enable);

	// Set the sprite that should be shown as the window's icon image
	void setIconSprite (Sprite *iconSprite);

	// Set the color that should be used to draw the window's icon image
	void setIconImageColor (const Color &imageColor);

	// Set the sprite frame that should be drawn by the window's icon image
	void setIconImageFrame (int frame);

	// Set the scale value that should be applied to the window's icon image
	void setIconImageScale (double scale);

	// Set the window's progress bar option. If enabled, the window shows a progress bar with the provided progress values; negative progress values specify an indeterminate progress bar.
	void setProgressBar (bool enable, double progressValue = -1.0f, double targetProgressValue = -1.0f);

	// Superclass override methods
	void reflow ();

private:
	// Callback functions
	static void labelClicked (void *itPtr, Widget *widgetPtr);

	Label *label;
	Image *image;
	Sprite *iconSprite;
	bool isRightAligned;
	bool isTextChangeHighlightEnabled;
	Color normalTextColor;
	Color highlightTextColor;
	ProgressBar *progressBar;
};
#endif
