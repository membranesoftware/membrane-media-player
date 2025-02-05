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
// Object that holds a set of position coordinates and executes translation operations against them
#ifndef POSITION_H
#define POSITION_H

class Position {
public:
	Position (double x = 0.0f, double y = 0.0f);
	~Position ();

	// Read-only data members
	double x, y;
	bool isTranslating;
	double translateTargetX, translateTargetY;
	double translateDx, translateDy;
	int translateDuration;
	int translateClock;

	// Return a string description of the position
	StdString toString () const;

	// Update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Set the position's coordinate values
	void assign (double positionX, double positionY);
	void assign (const Position &otherPosition);
	void assign (const Position &otherPosition, double dx, double dy);

	// Parse the provided text as position fields and return a boolean value indicating if the parse succeeded
	bool parse (const StdString &text);
	bool parse (const char *text);

	// Set the position's x coordinate value
	void assignX (double positionX);

	// Set the position's y coordinate value
	void assignY (double positionY);

	// Set the position's coordinate values, clipped to fit within the specified range
	void assignBounded (double positionX, double positionY, double minX, double minY, double maxX, double maxY);

	// Change the position's coordinate values by adding the specified deltas
	void move (double dx, double dy);

	// Begin a translation operation using the provided parameters
	void translate (double targetX, double targetY, int durationMs);
	void translate (const Position &targetPosition, int durationMs);
	void translate (double startX, double startY, double targetX, double targetY, int durationMs);
	void translate (const Position &startPosition, const Position &targetPosition, int durationMs);

	// Begin a translation operation for the position's x coordinate value
	void translateX (double targetX, int durationMs);
	void translateX (double startX, double targetX, int durationMs);

	// Begin a translation operation for the position's y coordinate value
	void translateY (double targetY, int durationMs);
	void translateY (double startY, double targetY, int durationMs);

	// Add a translation that should be executed as part of a sequence
	void plot (double deltaX, double deltaY, int durationMs);

	// Add a translation for the position's x coordinate value that should be executed as part of a sequence
	void plotX (double deltaX, int durationMs);

	// Add a translation for the position's y coordinate value that should be executed as part of a sequence
	void plotY (double deltaY, int durationMs);

	// Return a boolean value indicating if the position is equivalent to the provided one
	bool equals (double positionX, double positionY) const;
	bool equals (const Position &otherPosition) const;

	// Return the distance from the position to another position
	double distance (double positionX, double positionY) const;
	double distance (const Position &otherPosition) const;

private:
	struct Translation {
		double deltaX, deltaY;
		int duration;
		Translation ():
			deltaX (0.0f),
			deltaY (0.0f),
			duration (0) { }
	};
	std::queue<Position::Translation> translationQueue;
};
#endif
