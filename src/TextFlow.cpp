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
#include "Ui.h"
#include "Resource.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Font.h"
#include "TextFlow.h"

TextFlow::TextFlow (double viewWidth, UiConfiguration::FontType fontType)
: Panel ()
, maxLineCount (0)
, viewWidth (viewWidth)
, lineCount (0)
, textFontType (UiConfiguration::NoFont)
, textFont (NULL)
, textFontSize (0)
{
	setTextColor (UiConfiguration::instance->primaryTextColor);
	setText (StdString (), fontType);
	reflow ();
}
TextFlow::~TextFlow () {
	if (textFont) {
		Resource::instance->unloadFont (textFontName, textFontSize);
		textFont = NULL;
	}
}

void TextFlow::setViewWidth (double targetWidth) {
	if (FLOAT_EQUALS (viewWidth, targetWidth)) {
		return;
	}
	viewWidth = targetWidth;
	setText (text, textFontType, true);
}

void TextFlow::setFont (UiConfiguration::FontType fontType) {
	if (fontType == textFontType) {
		return;
	}
	setText (text, fontType);
}

void TextFlow::setTextColor (const Color &color) {
	std::list<LabelWindow *>::iterator i1, i2;

	textColor.assign (color);
	i1 = lineList.begin ();
	i2 = lineList.end ();
	while (i1 != i2) {
		(*i1)->setTextColor (textColor);
		++i1;
	}
}

void TextFlow::setText (const StdString &textContent, UiConfiguration::FontType fontType, bool forceFontReload) {
	std::list<LabelWindow *>::iterator i1, i2;
	Font *loadfont;

	loadfont = NULL;
	if (fontType >= 0) {
		if (forceFontReload || (! textFont) || (textFontType != fontType)) {
			loadfont = Resource::instance->loadFont (UiConfiguration::instance->fontNames[fontType], UiConfiguration::instance->fontSizes[fontType]);
			if (loadfont) {
				if (textFont) {
					Resource::instance->unloadFont (textFontName, textFontSize);
				}
				textFont = loadfont;
				textFontType = fontType;
				textFontName.assign (UiConfiguration::instance->fontNames[fontType]);
				textFontSize = UiConfiguration::instance->fontSizes[fontType];
			}
		}
	}
	if (! textFont) {
		return;
	}
	if ((! loadfont) && text.equals (textContent)) {
		return;
	}

	i1 = lineList.begin ();
	i2 = lineList.end ();
	while (i1 != i2) {
		(*i1)->isDestroyed = true;
		++i1;
	}
	lineList.clear ();

	text.assign (textContent);
	addLines (text);
}

void TextFlow::appendText (const StdString &textContent) {
	LabelWindow *label;
	size_t pos;

	text.append ("\n");
	text.append (textContent);
	addLines (textContent);
	if ((maxLineCount > 0) && (lineCount > maxLineCount)) {
		while (lineCount > maxLineCount) {
			label = lineList.front ();
			pos = text.find (label->getText ());
			if (pos != StdString::npos) {
				text = text.substr (pos + label->getText ().length ());
			}
			label->isDestroyed = true;
			lineList.pop_front ();
			--lineCount;
		}
		reflow ();
	}
}

void TextFlow::addLines (const StdString &linesText) {
	Font::Metrics metrics;
	LabelWindow *label;
	double maxw;
	int breakpos;
	char lastc;

	if ((! textFont) || linesText.empty ()) {
		return;
	}
	maxw = viewWidth - (widthPadding * 2.0f);
	textFont->resetMetrics (&metrics, linesText, 0);
	lastc = '\0';
	breakpos = -1;
	while (! metrics.isComplete) {
		textFont->advanceMetrics (&metrics);
		if ((metrics.lastCharacter == ' ') && (lastc != ' ')) {
			breakpos = metrics.textPosition;
		}
		if ((metrics.textWidth > maxw) || (metrics.lastCharacter == '\n')) {
			if ((breakpos < 0) || (metrics.lastCharacter == '\n')) {
				breakpos = metrics.textPosition;
			}
			label = add (new LabelWindow (new Label (StdString (metrics.text.substr (0, breakpos)).trimmed (), textFontType, textColor)));
			label->setFixedPadding (true, 0.0f, 0.0f);
			label->setWindowWidth (metrics.textWidth);
			label->setLinePosition (true);
			lineList.push_back (label);

			textFont->resetMetrics (&metrics, metrics.text.substr (breakpos), 0);
			breakpos = -1;
		}
		lastc = metrics.lastCharacter;
	}
	if (metrics.textPosition > 0) {
		label = add (new LabelWindow (new Label (metrics.text, textFontType, textColor)));
		label->setFixedPadding (true, 0.0f, 0.0f);
		label->setWindowWidth (metrics.textWidth);
		label->setLinePosition (true);
		lineList.push_back (label);
	}
	reflow ();
}

void TextFlow::reflow () {
	std::list<LabelWindow *>::iterator i1, i2;
	LabelWindow *label;
	double x, y, h;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	h = 0.0f;
	i1 = lineList.begin ();
	i2 = lineList.end ();
	while (i1 != i2) {
		label = *i1;
		label->position.assign (x, y);
		y += label->getMaxLineHeight () + UiConfiguration::instance->textLineHeightMargin;
		h = label->position.y + label->height;
		++i1;
	}
	setFixedSize (true, viewWidth, h + heightPadding);
	lineCount = (int) lineList.size ();
}

void TextFlow::doResize () {
	Panel::doResize ();
	setText (text, textFontType, true);
}
