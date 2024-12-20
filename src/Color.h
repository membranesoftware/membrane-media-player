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
// Object that represents an RGBA color value
#ifndef COLOR_H
#define COLOR_H

class Color {
public:
	Color (double r = 0.0f, double g = 0.0f, double b = 0.0f, double a = 1.0f);
	~Color ();

	// Read-only data members
	double r, g, b, a;
	uint8_t rByte, gByte, bByte, aByte;
	bool isTranslating;
	bool isAnimating;

	// Return a string description of the color
	StdString toString () const;

	// Return a Color object that has been assigned to the specified byte values
	static Color fromByteValues (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

	// Return the decimal color value associated with the specified byte
	static double getByteValue (uint8_t byte);

	// Return a Color object with values copied from this one. If aValue is zero or greater, assign that value to the returned object's alpha component.
	Color copy (double aValue = -1.0f);

	// Return a boolean value indicating if the color holds values equal to another
	bool equals (const Color &other) const;

	// Return the 32-bit pixel data value associated with the color
	Uint32 getPixelValue () const;

	// Assign the color's values
	void assign (double rValue, double gValue, double bValue);
	void assign (double rValue, double gValue, double bValue, double aValue);
	void assign (const Color &sourceColor);

	// Parse the provided text as color data, store color values, and return a boolean value indicating if the parse succeeded
	bool parse (const StdString &text);
	bool parse (const char *text);

	// Modify the color's values by applying the specified blend operation
	void blend (double r, double g, double b, double alpha);
	void blend (const Color &sourceColor, double alpha);

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Begin an operation to change the color's value over time
	void translate (double translateTargetR, double translateTargetG, double translateTargetB, int durationMs);
	void translate (double translateTargetR, double translateTargetG, double translateTargetB, double translateTargetA, int durationMs);
	void translate (const Color &targetColor, int durationMs);
	void translate (const Color &startColor, const Color &targetColor, int durationMs);

	// Begin an operation to animate the color's value over time
	void animate (const Color &color1, const Color &color2, int durationMs, int repeatDelayMs = 0);

private:
	// Clip the r, g, and b data members to valid ranges, and reset dependent data members
	void normalize ();

	int translateDuration;
	int animateDuration;
	int animateRepeatDelay;
	int animateStage;
	int animateClock;
	double targetR, targetG, targetB, targetA;
	double deltaR, deltaG, deltaB, deltaA;
	double animateColor1R, animateColor1G, animateColor1B, animateColor1A;
	double animateColor2R, animateColor2G, animateColor2B, animateColor2A;
};
#endif
