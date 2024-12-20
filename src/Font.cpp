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
#include "ft2build.h"
#include FT_FREETYPE_H
#include "Log.h"
#include "Resource.h"
#include "RenderResource.h"
#include "Font.h"

const char *Font::glyphCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=+[]{}\\\"';:,.<>/?!@#$%^&*()|";
const StdString Font::dotTruncateSuffix = StdString ("...");

Font::Font (FT_Library freetype, const StdString &name)
: name (name)
, spaceWidth (0)
, maxGlyphWidth (0)
, maxLineHeight (0)
, freetype (freetype)
, isLoaded (false)
{
}
Font::~Font () {
	clearGlyphMap ();
	if (isLoaded) {
		FT_Done_Face (face);
		isLoaded = false;
	}
}

void Font::clearGlyphMap () {
	std::map<char, Font::Glyph>::iterator i1, i2;

	i1 = glyphMap.begin ();
	i2 = glyphMap.end ();
	while (i1 != i2) {
		if (i1->second.texture) {
			Resource::instance->unloadTexture (i1->second.texturePath);
			i1->second.texture = NULL;
		}
		++i1;
	}
	glyphMap.clear ();
}

OpResult Font::load (Buffer *fontData, int pointSize) {
	Font::Glyph glyph;
	FT_GlyphSlot slot;
	SDL_Surface *surface;
	char *s, c;
	int result, charindex, x, y, w, h, pitch, maxw, maxtopbearing;
	uint8_t *row, *bitmap, alpha;
	Uint32 *pixels, *dest, color, rmask, gmask, bmask, amask;
	std::map<char, Font::Glyph>::iterator i1, i2;

	result = FT_New_Memory_Face (freetype, (FT_Byte *) fontData->data, fontData->length, 0, &face);
	if (result != 0) {
		Log::err ("Failed to load font; name=\"%s\" err=\"FT_New_Memory_Face: %i\"", name.c_str (), result);
		return (OpResult::FreetypeOperationFailedError);
	}
	result = FT_Set_Char_Size (face, pointSize << 6, 0, 100, 0);
	if (result != 0) {
		Log::err ("Failed to load font; name=\"%s\" err=\"FT_Set_Char_Size: %i\"", name.c_str (), result);
		return (OpResult::FreetypeOperationFailedError);
	}

	rmask = RenderResource::instance->pixelRMask;
	gmask = RenderResource::instance->pixelGMask;
	bmask = RenderResource::instance->pixelBMask;
	amask = RenderResource::instance->pixelAMask;
	maxw = 0;
	maxtopbearing = 0;
	s = (char *) Font::glyphCharacters;
	while (1) {
		c = *s;
		if (c == 0) {
			break;
		}
		++s;

		charindex = FT_Get_Char_Index (face, c);
		result = FT_Load_Glyph (face, charindex, FT_LOAD_RENDER);
		if (result != 0) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"FT_Load_Glyph: %i\"", name.c_str (), c, result);
			continue;
		}
		slot = face->glyph;
		w = slot->bitmap.width;
		h = slot->bitmap.rows;
		if ((w <= 0) || (h <= 0)) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"Invalid bitmap dimensions %ix%i\"", name.c_str (), c, w, h);
			continue;
		}
		pixels = (Uint32 *) malloc (w * h * sizeof (Uint32));
		if (! pixels) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"Out of memory, bitmap dimensions %ix%i\"", name.c_str (), c, w, h);
			continue;
		}
		dest = pixels;
		row = (uint8_t *) slot->bitmap.buffer;
		pitch = slot->bitmap.pitch;
		y = 0;
		while (y < h) {
			bitmap = row;
			x = 0;
			while (x < w) {
				alpha = *bitmap;
				++bitmap;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				color = 0xFFFFFF00 | (alpha & 0xFF);
#else
				color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
				*dest = color;
				++dest;
				++x;
			}
			row += pitch;
			++y;
		}
		surface = SDL_CreateRGBSurfaceFrom (pixels, w, h, 32, w * sizeof (Uint32), rmask, gmask, bmask, amask);
		if (! surface) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"SDL_CreateRGBSurfaceFrom, %s\"", name.c_str (), c, SDL_GetError ());
			free (pixels);
			continue;
		}
		glyph.texturePath.sprintf ("*_Font_%s_%i_%i", name.c_str (), pointSize, (int) c);
		glyph.texture = Resource::instance->createTexture (glyph.texturePath, surface);
		SDL_FreeSurface (surface);
		free (pixels);
		if (! glyph.texture) {
			Log::warning ("Failed to load font character; name=\"%s\" index=\"%c\" err=\"SDL_CreateTextureFromSurface, %s\"", name.c_str (), c, SDL_GetError ());
			continue;
		}
		glyph.width = w;
		glyph.height = h;
		glyph.leftBearing = (int) slot->bitmap_left;
		glyph.topBearing = (int) slot->bitmap_top;
		glyph.advanceWidth = (int) ((FT_CeilFix (slot->linearHoriAdvance) >> 16) & 0xFFFF);
		glyphMap.insert (std::pair<char, Font::Glyph> (c, glyph));
		if (w > maxw) {
			maxw = w;
		}
		if ((maxtopbearing <= 0) || (glyph.topBearing > maxtopbearing)) {
			maxtopbearing = glyph.topBearing;
		}
	}
	if (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) {
		spaceWidth = maxw;
	}
	else {
		spaceWidth = (maxw / 3);
	}

	maxGlyphWidth = 0;
	maxLineHeight = 0;
	i1 = glyphMap.begin ();
	i2 = glyphMap.end ();
	while (i1 != i2) {
		if (i1->second.width > maxGlyphWidth) {
			maxGlyphWidth = i1->second.width;
		}
		h = maxtopbearing - i1->second.topBearing + i1->second.height;
		if (h > maxLineHeight) {
			maxLineHeight = h;
		}
		++i1;
	}

	isLoaded = true;
	return (OpResult::Success);
}

Font::Glyph *Font::getGlyph (char glyphCharacter) {
	std::map<char, Font::Glyph>::iterator i;

	i = glyphMap.find (glyphCharacter);
	if (i == glyphMap.end ()) {
		return (NULL);
	}
	return (&(i->second));
}

int Font::getKerning (char leftCharacter, char rightCharacter) {
	int leftindex, rightindex;
	FT_Vector vector;

	leftindex = FT_Get_Char_Index (face, leftCharacter);
	rightindex = FT_Get_Char_Index (face, rightCharacter);
	FT_Get_Kerning (face, leftindex, rightindex, FT_KERNING_DEFAULT, &vector);
	return (vector.x >> 6);
}

void Font::resetMetrics (Font::Metrics *metrics, const StdString &text, int textPosition) {
	metrics->text.assign (text);
	metrics->textLength = (int) metrics->text.length ();
	metrics->textPosition = 0;
	metrics->lastCharacter = '\0';
	metrics->textWidth = 0.0f;
	metrics->isComplete = false;

	if (textPosition < 0) {
		textPosition = metrics->textLength;
	}
	if (textPosition > 0) {
		advanceMetrics (metrics, textPosition);
	}
	if (metrics->textPosition >= metrics->textLength) {
		metrics->isComplete = true;
	}
}

void Font::advanceMetrics (Font::Metrics *metrics, int advanceLength) {
	Font::Glyph *glyph;
	int i, kerning;
	char *buf, c;

	if (advanceLength <= 0) {
		return;
	}
	if (metrics->isComplete || (metrics->textPosition < 0) || (metrics->textPosition >= metrics->textLength)) {
		return;
	}
	buf = (char *) metrics->text.c_str ();
	for (i = 0; i < advanceLength; ++i) {
		if (metrics->textPosition >= metrics->textLength) {
			break;
		}
		c = buf[metrics->textPosition];
		glyph = getGlyph (c);

		if (metrics->lastCharacter != '\0') {
			kerning = getKerning (metrics->lastCharacter, c);
			metrics->textWidth += (double) kerning;
		}
		metrics->lastCharacter = c;

		if (! glyph) {
			metrics->textWidth += (double) spaceWidth;
		}
		else {
			if (metrics->textPosition == (metrics->textLength - 1)) {
				metrics->textWidth += (double) glyph->leftBearing;
				metrics->textWidth += (double) glyph->width;
			}
			else {
				metrics->textWidth += (double) glyph->advanceWidth;
			}
		}

		++(metrics->textPosition);
	}

	if (metrics->textPosition >= metrics->textLength) {
		metrics->isComplete = true;
	}
}

void Font::truncateText (StdString *text, double maxWidth, const StdString &truncateSuffix) {
	Font::Glyph *glyph;
	double x, spacew, suffixw;
	char *buf, c, lastc, suffixc;
	int i, textlen, truncatepos, suffixkerning;

	spacew = (double) spaceWidth;
	suffixc = 0;
	x = 0.0f;
	lastc = 0;
	buf = (char *) truncateSuffix.c_str ();
	textlen = truncateSuffix.length ();
	for (i = 0; i < textlen; ++i) {
		c = buf[i];
		if (suffixc <= 0) {
			suffixc = c;
		}
		glyph = getGlyph (c);
		if (i > 0) {
			x += getKerning (lastc, c);
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spacew;
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
	}
	suffixw = x;

	truncatepos = 0;
	x = 0.0f;
	lastc = 0;
	buf = (char *) text->c_str ();
	textlen = text->length ();
	for (i = 0; i < textlen; ++i) {
		c = buf[i];
		glyph = getGlyph (c);
		if (i > 0) {
			x += getKerning (lastc, c);
		}
		lastc = c;

		if (! glyph) {
			if (i < (textlen - 1)) {
				x += spacew;
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

		suffixkerning = 0;
		if (suffixc > 0) {
			suffixkerning = getKerning (c, suffixc);
		}
		if ((x + suffixkerning + suffixw) <= maxWidth) {
			truncatepos = i;
		}
		if (x > maxWidth) {
			text->assign (text->substr (0, truncatepos + 1));
			text->append (truncateSuffix);
			break;
		}
	}
}

StdString Font::truncatedText (const StdString &text, double maxWidth, const StdString &truncateSuffix) {
	StdString s;

	s.assign (text);
	truncateText (&s, maxWidth, truncateSuffix);
	return (s);
}
