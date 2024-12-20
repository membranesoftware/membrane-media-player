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
#include "StringList.h"
#include "Color.h"

Color::Color (double r, double g, double b, double a)
: r (r)
, g (g)
, b (b)
, a (a)
, isTranslating (false)
, isAnimating (false)
, translateDuration (0)
, animateDuration (0)
, animateRepeatDelay (0)
, animateStage (0)
, animateClock (0)
, targetR (0.0f)
, targetG (0.0f)
, targetB (0.0f)
, targetA (0.0f)
, deltaR (0.0f)
, deltaG (0.0f)
, deltaB (0.0f)
, deltaA (0.0f)
, animateColor1R (0.0f)
, animateColor1G (0.0f)
, animateColor1B (0.0f)
, animateColor1A (0.0f)
, animateColor2R (0.0f)
, animateColor2G (0.0f)
, animateColor2B (0.0f)
, animateColor2A (0.0f)
{
	normalize ();
}
Color::~Color () {
}

StdString Color::toString () const {
	StdString s;

	s.sprintf ("{r%.3f,g%.3f,b%.3f,a%.3f r%i,g%i,b%i,a%i #%02X%02X%02X/a%02X", r, g, b, a, rByte, gByte, bByte, aByte, rByte, gByte, bByte, aByte);
	if (isTranslating) {
		s.appendSprintf (" translate delta=r%.3f,g%.3f,b%.3f,a%.3f", deltaR, deltaG, deltaB, deltaA);
	}
	s.append ("}");
	return (s);
}

void Color::normalize () {
	if (r < 0.0f) {
		r = 0.0f;
	}
	else if (r > 1.0f) {
		r = 1.0f;
	}
	if (g < 0.0f) {
		g = 0.0f;
	}
	else if (g > 1.0f) {
		g = 1.0f;
	}
	if (b < 0.0f) {
		b = 0.0f;
	}
	else if (b > 1.0f) {
		b = 1.0f;
	}
	if (a < 0.0f) {
		a = 0.0f;
	}
	else if (a > 1.0f) {
		a = 1.0f;
	}

	rByte = (uint8_t) (r * 255.0f);
	gByte = (uint8_t) (g * 255.0f);
	bByte = (uint8_t) (b * 255.0f);
	aByte = (uint8_t) (a * 255.0f);
}

Color Color::copy (double aValue) {
	return (Color (r, g, b, (aValue >= 0.0f) ? aValue : a));
}

void Color::assign (double rValue, double gValue, double bValue) {
	r = rValue;
	g = gValue;
	b = bValue;
	isTranslating = false;
	normalize ();
}

void Color::assign (double rValue, double gValue, double bValue, double aValue) {
	r = rValue;
	g = gValue;
	b = bValue;
	a = aValue;
	isTranslating = false;
	normalize ();
}

void Color::assign (const Color &sourceColor) {
	assign (sourceColor.r, sourceColor.g, sourceColor.b, sourceColor.a);
}

Uint32 Color::getPixelValue () const {
	Uint32 result;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	result = (((Uint32) rByte) << 24) | (((Uint32) gByte) << 16) | (((Uint32) bByte) << 8) | ((Uint32) aByte);
#else
	result = (((Uint32) aByte) << 24) | (((Uint32) bByte) << 16) | (((Uint32) gByte) << 8) | ((Uint32) rByte);
#endif
	return (result);
}

bool Color::parse (const StdString &text) {
	StringList parts;
	StringList::const_iterator i;

	text.split (" ", &parts);
	if (parts.size () < 3) {
		return (false);
	}
	i = parts.cbegin ();
	if (! i->parseFloat (&r)) {
		return (false);
	}
	++i;
	if (! i->parseFloat (&g)) {
		return (false);
	}
	++i;
	if (! i->parseFloat (&b)) {
		return (false);
	}
	if (parts.size () < 4) {
		a = 1.0f;
	}
	else {
		++i;
		if (! i->parseFloat (&a)) {
			return (false);
		}
	}

	normalize ();
	return (true);
}

bool Color::parse (const char *text) {
	return (parse (StdString (text)));
}

void Color::blend (double r, double g, double b, double alpha) {
	double rval, gval, bval, aval;

	if (r < 0.0f) {
		r = 0.0f;
	}
	else if (r > 1.0f) {
		r = 1.0f;
	}
	if (g < 0.0f) {
		g = 0.0f;
	}
	else if (g > 1.0f) {
		g = 1.0f;
	}
	if (b < 0.0f) {
		b = 0.0f;
	}
	else if (b > 1.0f) {
		b = 1.0f;
	}
	if (alpha < 0.0f) {
		alpha = 0.0f;
	}
	else if (alpha > 1.0f) {
		alpha = 1.0f;
	}

	aval = 1.0f - alpha;
	rval = (this->r * aval) + (r * alpha);
	gval = (this->g * aval) + (g * alpha);
	bval = (this->b * aval) + (b * alpha);
	assign (rval, gval, bval);
}

void Color::blend (const Color &sourceColor, double alpha) {
	blend (sourceColor.r, sourceColor.g, sourceColor.b, alpha);
}

void Color::update (int msElapsed) {
	int matchcount;

	if (isTranslating) {
		matchcount = 0;

		r += (deltaR * (double) msElapsed);
		if (deltaR < 0.0f) {
			if (r <= targetR) {
				r = targetR;
				++matchcount;
			}
		}
		else {
			if (r >= targetR) {
				r = targetR;
				++matchcount;
			}
		}

		g += (deltaG * (double) msElapsed);
		if (deltaG < 0.0f) {
			if (g <= targetG) {
				g = targetG;
				++matchcount;
			}
		}
		else {
			if (g >= targetG) {
				g = targetG;
				++matchcount;
			}
		}

		b += (deltaB * (double) msElapsed);
		if (deltaB < 0.0f) {
			if (b <= targetB) {
				b = targetB;
				++matchcount;
			}
		}
		else {
			if (b >= targetB) {
				b = targetB;
				++matchcount;
			}
		}

		a += (deltaA * (double) msElapsed);
		if (deltaA < 0.0f) {
			if (a <= targetA) {
				a = targetA;
				++matchcount;
			}
		}
		else {
			if (a >= targetA) {
				a = targetA;
				++matchcount;
			}
		}

		normalize ();
		if (matchcount >= 4) {
			isTranslating = false;
		}
	}

	if (isAnimating) {
		switch (animateStage) {
			case 0: {
				if (! isTranslating) {
					translate (animateColor2R, animateColor2G, animateColor2B, animateColor2A, animateDuration / 2);
					animateStage = 1;
				}
				break;
			}
			case 1: {
				if (! isTranslating) {
					translate (animateColor1R, animateColor1G, animateColor1B, animateColor1A, animateDuration / 2);
					animateStage = 2;
					animateClock = animateRepeatDelay;
				}
				break;
			}
			case 2: {
				if (! isTranslating) {
					animateClock -= msElapsed;
					if (animateClock <= 0) {
						animateStage = 0;
					}
				}
				break;
			}
		}
	}
}

void Color::translate (double translateTargetR, double translateTargetG, double translateTargetB, int durationMs) {
	double dr, dg, db;

	if (translateTargetR < 0.0f) {
		translateTargetR = 0.0f;
	}
	else if (translateTargetR > 1.0f) {
		translateTargetR = 1.0f;
	}
	if (translateTargetG < 0.0f) {
		translateTargetG = 0.0f;
	}
	else if (translateTargetG > 1.0f) {
		translateTargetG = 1.0f;
	}
	if (translateTargetB < 0.0f) {
		translateTargetB = 0.0f;
	}
	else if (translateTargetB > 1.0f) {
		translateTargetB = 1.0f;
	}

	if (durationMs <= 0) {
		assign (translateTargetR, translateTargetG, translateTargetB);
		return;
	}
	dr = translateTargetR - r;
	dg = translateTargetG - g;
	db = translateTargetB - b;
	if ((fabs (dr) < CONFIG_FLOAT_EPSILON) && (fabs (dg) < CONFIG_FLOAT_EPSILON) && (fabs (db) < CONFIG_FLOAT_EPSILON)) {
		isTranslating = false;
		return;
	}
	if (isTranslating && FLOAT_EQUALS (translateTargetR, targetR) && FLOAT_EQUALS (translateTargetG, targetG) && FLOAT_EQUALS (translateTargetB, targetB) && (translateDuration == durationMs)) {
		return;
	}
	isTranslating = true;
	translateDuration = durationMs;
	targetR = translateTargetR;
	targetG = translateTargetG;
	targetB = translateTargetB;
	targetA = a;
	deltaR = dr / ((double) durationMs);
	deltaG = dg / ((double) durationMs);
	deltaB = db / ((double) durationMs);
	deltaA = 0.0f;
}

void Color::translate (double translateTargetR, double translateTargetG, double translateTargetB, double translateTargetA, int durationMs) {
	double dr, dg, db, da;

	if (translateTargetR < 0.0f) {
		translateTargetR = 0.0f;
	}
	else if (translateTargetR > 1.0f) {
		translateTargetR = 1.0f;
	}
	if (translateTargetG < 0.0f) {
		translateTargetG = 0.0f;
	}
	else if (translateTargetG > 1.0f) {
		translateTargetG = 1.0f;
	}
	if (translateTargetB < 0.0f) {
		translateTargetB = 0.0f;
	}
	else if (translateTargetB > 1.0f) {
		translateTargetB = 1.0f;
	}
	if (translateTargetA < 0.0f) {
		translateTargetA = 0.0f;
	}
	else if (translateTargetA > 1.0f) {
		translateTargetA = 1.0f;
	}

	if (durationMs <= 0) {
		assign (translateTargetR, translateTargetG, translateTargetB, translateTargetA);
		return;
	}
	dr = translateTargetR - r;
	dg = translateTargetG - g;
	db = translateTargetB - b;
	da = translateTargetA - a;
	if ((fabs (dr) < CONFIG_FLOAT_EPSILON) && (fabs (dg) < CONFIG_FLOAT_EPSILON) && (fabs (db) < CONFIG_FLOAT_EPSILON) && (fabs (da) <= CONFIG_FLOAT_EPSILON)) {
		isTranslating = false;
		return;
	}
	if (isTranslating && FLOAT_EQUALS (translateTargetR, targetR) && FLOAT_EQUALS (translateTargetG, targetG) && FLOAT_EQUALS (translateTargetB, targetB) && FLOAT_EQUALS (translateTargetA, targetA) && (translateDuration == durationMs)) {
		return;
	}
	isTranslating = true;
	translateDuration = durationMs;
	targetR = translateTargetR;
	targetG = translateTargetG;
	targetB = translateTargetB;
	targetA = translateTargetA;
	deltaR = dr / ((double) durationMs);
	deltaG = dg / ((double) durationMs);
	deltaB = db / ((double) durationMs);
	deltaA = da / ((double) durationMs);
}

void Color::translate (const Color &targetColor, int durationMs) {
	translate (targetColor.r, targetColor.g, targetColor.b, targetColor.a, durationMs);
}

void Color::translate (const Color &startColor, const Color &targetColor, int durationMs) {
	assign (startColor);
	translate (targetColor.r, targetColor.g, targetColor.b, targetColor.a, durationMs);
}

void Color::animate (const Color &color1, const Color &color2, int durationMs, int repeatDelayMs) {
	assign (color1);
	if (durationMs <= 0) {
		return;
	}
	animateColor1R = color1.r;
	animateColor1G = color1.g;
	animateColor1B = color1.b;
	animateColor1A = color1.a;
	animateColor2R = color2.r;
	animateColor2G = color2.g;
	animateColor2B = color2.b;
	animateColor2A = color2.a;
	animateDuration = durationMs;
	animateRepeatDelay = repeatDelayMs;
	if (animateRepeatDelay < 0) {
		animateRepeatDelay = 0;
	}
	animateStage = 0;
	animateClock = 0;
	isAnimating = true;
}

bool Color::equals (const Color &other) const {
	return ((rByte == other.rByte) && (gByte == other.gByte) && (bByte == other.bByte) && (aByte == other.aByte));
}

Color Color::fromByteValues (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (Color (Color::getByteValue (r), Color::getByteValue (g), Color::getByteValue (b), Color::getByteValue (a)));
}

double Color::getByteValue (uint8_t byte) {
	return (((double) byte) / 255.0f);
}
