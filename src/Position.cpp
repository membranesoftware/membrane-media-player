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
#include "Position.h"

Position::Position (double x, double y)
: x (x)
, y (y)
, isTranslating (false)
, translateTargetX (0.0f)
, translateTargetY (0.0f)
, translateDx (0.0f)
, translateDy (0.0f)
, translateDuration (0)
, translateClock (0)
{
}
Position::~Position () {
}

StdString Position::toString () const {
	StdString s;

	s.sprintf ("{x%.4f,y%.4f", x, y);
	if (isTranslating) {
		s.appendSprintf (" translate target=x%.4f,y%.4f delta=x%.4f,y%.4f duration=%i clock=%i", translateTargetX, translateTargetY, translateDx, translateDy, translateDuration, translateClock);
	}
	s.append ("}");
	return (s);
}

void Position::update (int msElapsed) {
	Position::Translation t;
	double dx, dy;
	int ms, dt;

	ms = msElapsed;
	while (isTranslating && (ms > 0)) {
		dt = ms;
		if (dt > translateClock) {
			dt = translateClock;
		}
		translateClock -= dt;
		ms -= dt;

		dx = translateDx * (double) dt;
		x += dx;
		if (translateDx < 0.0f) {
			if (x < translateTargetX) {
				x = translateTargetX;
			}
		}
		else {
			if (x > translateTargetX) {
				x = translateTargetX;
			}
		}

		dy = translateDy * (double) dt;
		y += dy;
		if (translateDy < 0.0f) {
			if (y < translateTargetY) {
				y = translateTargetY;
			}
		}
		else {
			if (y > translateTargetY) {
				y = translateTargetY;
			}
		}

		if (translateClock <= 0) {
			x = translateTargetX;
			y = translateTargetY;
			if (translationQueue.empty ()) {
				isTranslating = false;
			}
			else {
				t = translationQueue.front ();
				translationQueue.pop ();
				translate (x + t.deltaX, y + t.deltaY, t.duration);
			}
		}
	}
}

void Position::assign (double positionX, double positionY) {
	x = positionX;
	y = positionY;
	isTranslating = false;
	while (! translationQueue.empty ()) {
		translationQueue.pop ();
	}
}

void Position::assign (const Position &otherPosition) {
	assign (otherPosition.x, otherPosition.y);
}

void Position::assign (const Position &otherPosition, double dx, double dy) {
	assign (otherPosition.x + dx, otherPosition.y + dy);
}

bool Position::parse (const StdString &text) {
	StringList parts;
	StringList::const_iterator i;

	text.split (",", &parts);
	if (parts.size () < 2) {
		return (false);
	}
	i = parts.cbegin ();
	if (! i->replaced (StdString (" "), StdString ()).parseFloat (&x)) {
		return (false);
	}
	++i;
	if (! i->replaced (StdString (" "), StdString ()).parseFloat (&y)) {
		return (false);
	}
	isTranslating = false;
	while (! translationQueue.empty ()) {
		translationQueue.pop ();
	}
	return (true);
}

bool Position::parse (const char *text) {
	return (parse (StdString (text)));
}

void Position::assignX (double positionX) {
	assign (positionX, y);
}

void Position::assignY (double positionY) {
	assign (x, positionY);
}

void Position::assignBounded (double positionX, double positionY, double minX, double minY, double maxX, double maxY) {
	if (positionX < minX) {
		positionX = minX;
	}
	if (positionX > maxX) {
		positionX = maxX;
	}
	if (positionY < minY) {
		positionY = minY;
	}
	if (positionY > maxY) {
		positionY = maxY;
	}
	assign (positionX, positionY);
}

bool Position::equals (double positionX, double positionY) const {
	return (FLOAT_EQUALS (x, positionX) && FLOAT_EQUALS (y, positionY));
}
bool Position::equals (const Position &otherPosition) const {
	return (FLOAT_EQUALS (x, otherPosition.x) && FLOAT_EQUALS (y, otherPosition.y));
}

void Position::move (double dx, double dy) {
	assign (x + dx, y + dy);
}

void Position::translate (double targetX, double targetY, int durationMs) {
	double dx, dy;

	if (durationMs <= 0) {
		x = targetX;
		y = targetY;
		translateTargetX = targetX;
		translateTargetY = targetY;
		translateDuration = 0;
		translateClock = 0;
		translateDx = 0.0f;
		translateDy = 0.0f;
		if (translationQueue.empty ()) {
			isTranslating = false;
		}
		return;
	}

	dx = targetX - x;
	dy = targetY - y;
	if ((fabs (dx) <= CONFIG_FLOAT_EPSILON) && (fabs (dy) <= CONFIG_FLOAT_EPSILON) && translationQueue.empty ()) {
		isTranslating = false;
		return;
	}
	if (isTranslating && FLOAT_EQUALS (translateTargetX, targetX) && FLOAT_EQUALS (translateTargetY, targetY) && (translateDuration == durationMs)) {
		return;
	}

	isTranslating = true;
	translateTargetX = targetX;
	translateTargetY = targetY;
	translateDuration = durationMs;
	translateClock = durationMs;
	translateDx = dx / (double) durationMs;
	translateDy = dy / (double) durationMs;
}
void Position::translate (const Position &targetPosition, int durationMs) {
	translate (targetPosition.x, targetPosition.y, durationMs);
}
void Position::translate (double startX, double startY, double targetX, double targetY, int durationMs) {
	assign (startX, startY);
	translate (targetX, targetY, durationMs);
}
void Position::translate (const Position &startPosition, const Position &targetPosition, int durationMs) {
	assign (startPosition);
	translate (targetPosition.x, targetPosition.y, durationMs);
}

void Position::translateX (double targetX, int durationMs) {
	translate (targetX, y, durationMs);
}
void Position::translateX (double startX, double targetX, int durationMs) {
	assignX (startX);
	translate (targetX, y, durationMs);
}

void Position::translateY (double targetY, int durationMs) {
	translate (x, targetY, durationMs);
}
void Position::translateY (double startY, double targetY, int durationMs) {
	assignY (startY);
	translate (x, targetY, durationMs);
}

void Position::plot (double deltaX, double deltaY, int durationMs) {
	Position::Translation t;

	if (! isTranslating) {
		while (! translationQueue.empty ()) {
			translationQueue.pop ();
		}
		translate (x + deltaX, y + deltaY, durationMs);
		return;
	}

	t.deltaX = deltaX;
	t.deltaY = deltaY;
	t.duration = durationMs;
	translationQueue.push (t);
}

void Position::plotX (double deltaX, int durationMs) {
	plot (deltaX, 0.0f, durationMs);
}

void Position::plotY (double deltaY, int durationMs) {
	plot (0.0f, deltaY, durationMs);
}

double Position::distance (double positionX, double positionY) const {
	double dx, dy;

	dx = x - positionX;
	dy = y - positionY;
	return (sqrt ((dx * dx) + (dy * dy)));
}
double Position::distance (const Position &otherPosition) const {
	return (distance (otherPosition.x, otherPosition.y));
}
