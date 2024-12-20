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
// Widget that holds a text flow and provides vertical scrolling
#ifndef TEXT_AREA_H
#define TEXT_AREA_H

#include "UiConfiguration.h"
#include "ScrollView.h"

class TextFlow;
class ScrollBar;

class TextArea : public ScrollView {
public:
	TextArea (double viewWidth, double viewHeight, UiConfiguration::FontType fontType = UiConfiguration::BodyFont, bool isScrollEnabled = false);
	~TextArea ();

	// Read-only data members
	UiConfiguration::FontType textFontType;

	// Set the text area's content, changing its active font if a type is provided
	void setText (const StdString &textContent, UiConfiguration::FontType fontType = UiConfiguration::NoFont, bool forceFontReload = false);

	// Append new lines to the text area's content, optionally moving the view origin to the lowest extent if scrolling is enabled
	void appendText (const StdString &textContent, bool scrollToBottom = false);

	// Set the text area's font
	void setFont (UiConfiguration::FontType fontType);

	// Set the text area's text color
	void setTextColor (const Color &color);

	// Set the text area's inverse color state. If enabled, the text area renders using an inverse color scheme.
	void setInverseColor (bool isInverseColor);

	// Set the maximum number of lines that should be held in the text area. Appending text lines beyond this limit causes old lines to be discarded.
	void setMaxLineCount (int max);

	// Superclass override methods
	void setViewSize (double viewWidth, double viewHeight);
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void scrollBarPositionChanged (void *itPtr, Widget *widgetPtr);

	TextFlow *textFlow;
	ScrollBar *scrollBar;
};
#endif
