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
#include "SdlUtil.h"
#include "ClassId.h"
#include "Sprite.h"
#include "Resource.h"
#include "Panel.h"
#include "Label.h"

Label::Label (const StdString &text, UiConfiguration::FontType fontType, const Color &color)
: Widget ()
, textFontType (UiConfiguration::NoFont)
, textFont (NULL)
, textFontSize (0)
, spaceWidth (0.0f)
, maxGlyphWidth (0.0f)
, maxLineHeight (0.0f)
, maxCharacterHeight (0.0f)
, descenderHeight (0.0f)
, isUnderlined (false)
, isObscured (false)
, maxGlyphTopBearing (0)
, underlineMargin (0.0f)
{
	classId = ClassId::Label;
	textColor.assign (color);
	SdlUtil::createMutex (&textMutex);
	setText (text, fontType);
}
Label::~Label () {
	if (isObscured) {
		text.wipe ();
	}
	SDL_LockMutex (textMutex);
	glyphList.clear ();
	kerningList.clear ();
	SDL_UnlockMutex (textMutex);
	if (textFont) {
		Resource::instance->unloadFont (textFontName, textFontSize);
		textFont = NULL;
	}
	SdlUtil::destroyMutex (&textMutex);
}

Label *Label::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::Label) ? (Label *) widget : NULL);
}

StdString Label::toStringDetail () {
	StdString s;
	return (s);
}

double Label::getLinePosition (double targetY) {
	double y;

	y = targetY + maxLineHeight - maxCharacterHeight + descenderHeight;
	return (y);
}

double Label::getCharacterPosition (int position) {
	double x;
	char *buf, c, lastc;
	int textlen, i, kerning;
	Font::Glyph *glyph;

	if ((position <= 0) || (! textFont)) {
		return (0.0f);
	}
	buf = (char *) text.c_str ();
	textlen = (int) text.length ();
	if (position >= textlen) {
		x = width;
		c = isObscured ? Label::obscureCharacter : buf[textlen - 1];
		x += textFont->getKerning (c, c);
		if (x < (width + 2.0f)) {
			x = width + 2.0f;
		}
		return (x);
	}

	lastc = 0;
	x = 0.0f;
	i = 0;
	while (i < position) {
		c = isObscured ? Label::obscureCharacter : buf[i];
		glyph = textFont->getGlyph (c);
		if (i > 0) {
			kerning = textFont->getKerning (lastc, c);
			x += kerning;
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += textFont->spaceWidth;
			}
		}
		else {
			if (i == (textlen - 1)) {
				x += glyph->leftBearing;
				x += glyph->width;
			}
			else {
				x += glyph->advanceWidth;
			}
		}
		++i;
	}

	return (x);
}

void Label::setUnderlined (bool enable) {
	if (isUnderlined == enable) {
		return;
	}
	isUnderlined = enable;
	underlineMargin = UiConfiguration::instance->textUnderlineMargin;
	if (isUnderlined) {
		height = maxGlyphTopBearing + underlineMargin + 1.0f;
	}
	else {
		height = maxCharacterHeight;
	}
}

void Label::setObscured (bool enable) {
	if (isObscured == enable) {
		return;
	}
	isObscured = enable;
	setText (text, textFontType, true);
}

void Label::doDraw (double originX, double originY) {
	Font::Glyph *glyph;
	std::list<Font::Glyph *>::iterator i1, i2;
	IntList::iterator j1, j2;
	SDL_Rect rect;
	int x, y, x0, y0, xmax, ymax, kerning;
	bool first;

	SDL_LockMutex (textMutex);
	if (glyphList.empty ()) {
		SDL_UnlockMutex (textMutex);
		return;
	}

	x0 = (int) (originX + position.x);
	y0 = (int) (originY + position.y);
	xmax = (int) App::instance->drawableWidth;
	ymax = (int) App::instance->drawableHeight;
	x = 0;
	y = 0;
	kerning = 0;
	first = true;
	j1 = kerningList.begin ();
	j2 = kerningList.end ();
	i1 = glyphList.begin ();
	i2 = glyphList.end ();
	while (i1 != i2) {
		glyph = *i1;
		if ((! first) && (j1 != j2)) {
			kerning = *j1;
		}
		else {
			kerning = 0;
		}

		if (! glyph) {
			x += (int) spaceWidth;
		}
		else {
			rect.x = x + x0 + glyph->leftBearing + kerning;
			rect.y = y + y0 + maxGlyphTopBearing - glyph->topBearing;
			if (((rect.x + glyph->advanceWidth) >= 0) && (rect.x < xmax) && ((rect.y + maxGlyphTopBearing) >= 0) && (rect.y < ymax)) {
				rect.w = glyph->width;
				rect.h = glyph->height;
				SDL_SetTextureColorMod (glyph->texture, textColor.rByte, textColor.gByte, textColor.bByte);
				SDL_RenderCopy (App::instance->render, glyph->texture, NULL, &rect);
			}

			x += glyph->advanceWidth;
		}

		++i1;
		if (first) {
			first = false;
		}
		else {
			if (j1 != j2) {
				++j1;
			}
		}
	}

	if (isUnderlined) {
		y = y0 + maxGlyphTopBearing + (int) underlineMargin;
		SDL_SetRenderDrawColor (App::instance->render, textColor.rByte, textColor.gByte, textColor.bByte, 255);
		SDL_RenderDrawLine (App::instance->render, x0, y, (int) (x0 + width), y);
	}
	SDL_UnlockMutex (textMutex);
}

void Label::doResize () {
	if (textFontType >= 0) {
		if ((! textFontName.equals (UiConfiguration::instance->fontNames[textFontType])) || (textFontSize != UiConfiguration::instance->fontSizes[textFontType])) {
			setText (text, textFontType, true);
		}
	}
}

void Label::doUpdate (int msElapsed) {
	textColor.update (msElapsed);
}

void Label::setText (const StdString &textContent, UiConfiguration::FontType fontType, bool forceFontReload) {
	Font *font;
	Font::Glyph *glyph;
	int i, maxbearing, textlen, kerning, maxh, h, descenderh;
	double x;
	char *buf, c, lastc;

	font = NULL;
	if (fontType >= 0) {
		if (forceFontReload || (! textFont) || (textFontType != fontType)) {
			font = Resource::instance->loadFont (UiConfiguration::instance->fontNames[fontType], UiConfiguration::instance->fontSizes[fontType]);
			if (font) {
				if (textFont) {
					Resource::instance->unloadFont (textFontName, textFontSize);
				}
				textFont = font;
				textFontType = fontType;
				textFontName.assign (UiConfiguration::instance->fontNames[fontType]);
				textFontSize = UiConfiguration::instance->fontSizes[fontType];
				spaceWidth = (double) textFont->spaceWidth;
				maxGlyphWidth = (double) textFont->maxGlyphWidth;
				maxLineHeight = (double) textFont->maxLineHeight;
			}
		}
	}
	if (! textFont) {
		return;
	}
	if ((! font) && text.equals (textContent)) {
		return;
	}

	SDL_LockMutex (textMutex);
	text.assign (textContent);
	glyphList.clear ();
	kerningList.clear ();
	textlen = text.length ();
	if (textlen <= 0) {
		width = 0.0f;
		height = 0.0f;
		maxCharacterHeight = 0.0f;
		descenderHeight = 0.0f;
		SDL_UnlockMutex (textMutex);
		return;
	}

	lastc = 0;
	x = 0.0f;
	descenderh = 0;
	maxbearing = -1;
	buf = (char *) text.c_str ();
	for (i = 0; i < textlen; ++i) {
		c = isObscured ? Label::obscureCharacter : buf[i];
		glyph = textFont->getGlyph (c);
		glyphList.push_back (glyph);

		if (i > 0) {
			kerning = textFont->getKerning (lastc, c);
			kerningList.push_back (kerning);
			x += kerning;
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spaceWidth;
			}
		}
		else {
			if ((maxbearing < 0) || (glyph->topBearing > maxbearing)) {
				maxbearing = glyph->topBearing;
			}

			if (i == (textlen - 1)) {
				x += glyph->leftBearing;
				x += glyph->width;
			}
			else {
				x += glyph->advanceWidth;
			}

			h = glyph->height - glyph->topBearing;
			if (h > descenderh) {
				descenderh = h;
			}
		}
	}
	maxGlyphTopBearing = maxbearing;
	descenderHeight = (double) descenderh;

	maxh = 0;
	for (i = 0; i < textlen; ++i) {
		c = isObscured ? Label::obscureCharacter : buf[i];
		glyph = textFont->getGlyph (c);
		if (glyph) {
			h = maxGlyphTopBearing - glyph->topBearing + glyph->height;
			if (h > maxh) {
				maxh = h;
			}
		}
	}

	width = x;
	maxCharacterHeight = (double) maxh;
	if (isUnderlined) {
		height = maxGlyphTopBearing + underlineMargin + 1.0f;
	}
	else {
		height = maxCharacterHeight;
	}
	SDL_UnlockMutex (textMutex);
}

void Label::setFont (UiConfiguration::FontType fontType) {
	if (fontType == textFontType) {
		return;
	}
	setText (text, fontType);
}

void Label::flowRight (PanelLayoutFlow *flow) {
	double pos;

	position.assign (flow->x, getLinePosition (flow->y));
	flow->x += width + UiConfiguration::instance->marginSize;
	pos = position.x + width;
	if (pos > flow->xExtent) {
		flow->xExtent = pos;
	}
	pos = flow->y + maxLineHeight;
	if (pos > flow->yExtent) {
		flow->yExtent = pos;
	}
}

void Label::flowDown (PanelLayoutFlow *flow) {
	double x, y;

	y = flow->y;
	position.assign (flow->x, getLinePosition (y));
	flow->y += maxLineHeight + UiConfiguration::instance->marginSize;
	x = position.x + width;
	if (x > flow->xExtent) {
		flow->xExtent = x;
	}
	y = y + maxLineHeight;
	if (y > flow->yExtent) {
		flow->yExtent = y;
	}
}

void Label::centerVertical (PanelLayoutFlow *flow) {
	position.assignY (flow->y + ((flow->yExtent - flow->y) / 2.0f) - (maxLineHeight / 2.0f));
}
