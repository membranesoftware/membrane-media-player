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
// Class that tracks state of user input devices
#ifndef INPUT_H
#define INPUT_H

class Input {
public:
	Input ();
	~Input ();
	static Input *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Read-write data members
	int keyRepeatStartThreshold;
	int keyRepeatDelay;

	// Read-only data members
	int mouseX, mouseY;
	int lastMouseX, lastMouseY;
	bool isMouseLeftButtonDown, isMouseRightButtonDown;
	int keyDownCount;
	int mouseLeftDownCount, mouseRightDownCount;
	int mouseLeftUpCount, mouseRightUpCount;
	int mouseWheelDownCount, mouseWheelUpCount;
	int windowCloseCount;

	// Initialize input functionality and acquire resources as needed. Returns a Result value.
	OpResult start ();

	// Stop the input engine and release acquired resources
	void stop ();

	// Poll events to update input state. This method must be invoked only from the application's main thread.
	void pollEvents ();

	// Return a boolean value indicating if the specified key is down
	bool isKeyDown (SDL_Keycode keycode);

	// Return a boolean value indicating if a shift key is down
	bool isShiftDown ();

	// Return a boolean value indicating if a control key is down
	bool isControlDown ();

	// Return the character value associated with the specified keycode, or 0 if no such character was found
	char getKeyCharacter (SDL_Keycode keycode, bool isShiftDown);

	struct KeyPressEvent {
		SDL_Keycode keycode;
		bool isShiftDown;
		bool isControlDown;
		KeyPressEvent ():
			keycode (SDLK_UNKNOWN),
			isShiftDown (false),
			isControlDown (false) { }
		KeyPressEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown):
			keycode (keycode),
			isShiftDown (isShiftDown),
			isControlDown (isControlDown) { }
	};
	// Consume keypress events that have occurred since the last poll and append the resulting items to destVector if provided
	void pollKeyPressEvents (std::vector<Input::KeyPressEvent> *destVector = NULL);

	// Add an item to the keypress event list
	void insertKeyPressEvent (SDL_Keycode keycode);

	// Return the SDL_Keycode value associated with keyName, or SDLK_UNKNOWN if no key code was found
	SDL_Keycode getKeyCode (const StdString &keyName);
	SDL_Keycode getKeyCode (const char *keyName);

	// Move the mouse cursor to a position in the application window
	void warpMouse (int x, int y);

	// Generate input events for a click of the left mouse button
	void leftClickMouse ();

	// Generate a window close event for consumption by Ui classes
	void windowClose ();

private:
	std::map<SDL_Keycode, bool> keyDownMap;
	std::vector<Input::KeyPressEvent> keyPressList;
	std::map<StdString, SDL_Keycode> keyNameMap;
	SDL_mutex *keyPressListMutex;
	bool isKeyPressListPopulated;
	bool isKeyRepeating;
	int keyRepeatCode;
	int64_t keyRepeatStartTime;
};
#endif
