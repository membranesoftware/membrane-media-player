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
// Panel that holds a Label widget and surrounding features
#ifndef LABEL_WINDOW_H
#define LABEL_WINDOW_H

#include "Widget.h"
#include "UiConfiguration.h"
#include "Panel.h"

class Label;

class LabelWindow : public Panel {
public:
	LabelWindow (Label *label);
	~LabelWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static LabelWindow *castWidget (Widget *widget);

	// Read-only data members
	bool isCrawlEnabled;

	// Set a fixed size for the window. If the window's label does not fill the provided width, it is optionally centered inside a larger background space.
	void setWindowWidth (double windowWidthValue, bool isWidthCenteredValue = false);

	// Set the window to adjust width as needed to fit its text content
	void setFitWidth ();

	// Set the label window's line position option. If enabled, the label widget's y position applies a line position adjustment instead of vertical centering.
	void setLinePosition (bool enable);

	// Set the label's text color
	void setTextColor (const Color &color);

	// Execute a translation operation for the label's text color
	void translateTextColor (const Color &targetColor, int durationMs);
	void translateTextColor (const Color &startColor, const Color &targetColor, int durationMs);

	// Set the label's shadowed state. If enabled, the label renders a shadow effect under each glyph.
	void setShadowed (bool enable, const Color &shadowColor = Color (0.0f, 0.0f, 0.0f), int shadowDx = -1, int shadowDy = 1);

	// Return the text string shown in the window's label
	StdString getText () const;

	// Return the width of the text label
	double getTextWidth () const;

	// Return the maxLineHeight value for the window's label
	double getMaxLineHeight () const;

	// Set the text value shown in the label
	void setText (const StdString &text);

	// Set the font type used in the label
	void setFont (UiConfiguration::FontType fontType);

	// Set the label window's mouseover highlight option. If enabled, the window changes background and text color during mouseover events.
	void setMouseoverHighlight (bool enable, const Color &normalTextColor = Color (), const Color &normalBgColor = Color (), const Color &highlightTextColor = Color (), const Color &highlightBgColor = Color (), int colorTranslateDuration = 0);

	// Set the label window's crawl option. If enabled, the window crawls its text content as needed when text width exceeds window width.
	void setCrawl (bool enable);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	void doUpdate (int msElapsed);

private:
	Label *label;
	bool isFixedWidth;
	bool isWidthCentered;
	bool isLinePositionEnabled;
	double windowWidth;
	bool isMouseoverHighlightEnabled;
	Color mouseoverNormalTextColor;
	Color mouseoverNormalBgColor;
	Color mouseoverHighlightTextColor;
	Color mouseoverHighlightBgColor;
	int mouseoverColorTranslateDuration;
	int crawlStage;
	int crawlClock;
};
#endif
