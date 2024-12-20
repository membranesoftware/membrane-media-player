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
// Widget that behaves as a clickable button
#ifndef BUTTON_H
#define BUTTON_H

#include "Color.h"
#include "Panel.h"

class Sprite;
class Label;
class Image;

class Button : public Panel {
public:
	Button ();
	Button (Sprite *iconSprite);
	Button (Sprite *iconSprite, const Color &iconColor);
	Button (const StdString &labelText);
	Button (const StdString &labelText, const Color &labelColor);
	Button (Sprite *iconSprite, const StdString &labelText);
	Button (Sprite *iconSprite, const Color &iconColor, const StdString &labelText, const Color &labelColor);
	virtual ~Button ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Button *castWidget (Widget *widget);

	static constexpr const double gradientBackgroundMinAlpha = 0.02f;
	static constexpr const double gradientBackgroundOpacity = 0.98f;
	static constexpr const double gradientBackgroundCornerScale = 0.05f;
	static constexpr const double gradientBackgroundCornerOpacity = 0.26f;

	// Sprite frame indexes
	static constexpr const int NormalFrame = 0;
	static constexpr const int LargeFrame = 1;

	// Read-write data members
	SDL_Keycode shortcutKey;
	bool isShortcutKeyControlPress;
	bool isDropShadowDisabled;
	Widget::EventCallbackContext focusCallback;
	Widget::EventCallbackContext unfocusCallback;

	// Read-only data members
	double buttonWidth;
	double maxImageWidth, maxImageHeight;
	Color normalTextColor;
	Color normalIconColor;
	bool isFocused;
	bool isPressed;
	bool isDisabled;
	bool isInverseColor;
	int bodyType;
	Color bodyColor;
	Color gradientColor;

	// Return the button's text value
	StdString getText () const;

	// Set the button's fixed width attribute
	void setButtonWidth (double widthValue);

	// Set the button's text
	void setText (const StdString &text);

	// Set the button's text color
	void setTextColor (const Color &textColor);

	// Set the button's image color
	void setImageColor (const Color &imageColor);

	// Set the button's background to draw an alpha shade
	void setTransparentBackground (double shadeAlpha);

	// Set the button's background to draw a fill color
	void setColorBackground (const Color &bgColor);

	// Set the button's background to draw a color gradient
	void setGradientBackground (const Color &bgColor);

	// Set the button's inverse color state. If enabled, the button renders using an inverse color scheme.
	void setInverseColor (bool inverse);

	// Set the button's pressed state, appropriate for use when the button has been activated
	void setPressed (bool pressed);

	// Set the button's disabled state, appropriate for use when the button becomes unavailable for interaction. If tooltipTextValue is provided, also change the button's tooltip text string.
	void setDisabled (bool disabled, const StdString &tooltipTextValue = StdString ());

	// Set the button's focused state, appropriate for use when the button has input focus
	void setFocused (bool focused);

	// Superclass override methods
	virtual void reflow ();

	// Callback functions
	static void mouseEntered (void *itPtr, Widget *widgetPtr);
	static void mouseExited (void *itPtr, Widget *widgetPtr);
	static void mousePressed (void *itPtr, Widget *widgetPtr);
	static void mouseReleased (void *itPtr, Widget *widgetPtr);

protected:
	// Superclass override methods
	virtual void doUpdate (int msElapsed);
	virtual void doDraw (double originX, double originY);
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	virtual void doResetInputState ();

	Label *promptLabel;
	Image *iconImage;
	int pressClock;
};
#endif
