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
#include "OsUtil.h"
#include "Input.h"

Input *Input::instance = NULL;

Input::Input ()
: keyRepeatStartThreshold (680)
, keyRepeatDelay (48)
, mouseX (-1)
, mouseY (-1)
, lastMouseX (-1)
, lastMouseY (-1)
, isMouseLeftButtonDown (false)
, isMouseRightButtonDown (false)
, keyDownCount (0)
, mouseLeftDownCount (0)
, mouseRightDownCount (0)
, mouseLeftUpCount (0)
, mouseRightUpCount (0)
, mouseWheelDownCount (0)
, mouseWheelUpCount (0)
, windowCloseCount (0)
, isKeyPressListPopulated (false)
, isKeyRepeating (false)
, keyRepeatCode (SDLK_UNKNOWN)
, keyRepeatStartTime (0)
{
	SdlUtil::createMutex (&keyPressListMutex);
}
Input::~Input () {
	SdlUtil::destroyMutex (&keyPressListMutex);
}

void Input::createInstance () {
	if (! Input::instance) {
		Input::instance = new Input ();
	}
}
void Input::freeInstance () {
	if (Input::instance) {
		delete (Input::instance);
		Input::instance = NULL;
	}
}

OpResult Input::start () {
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_a, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_b, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_c, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_d, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_e, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_f, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_g, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_h, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_i, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_j, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_k, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_l, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_m, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_n, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_o, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_p, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_q, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_r, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_s, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_t, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_u, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_v, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_w, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_x, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_y, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_z, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_0, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_1, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_2, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_3, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_4, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_5, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_6, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_7, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_8, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_9, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SPACE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SLASH, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_COMMA, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PERIOD, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_MINUS, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_EQUALS, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LEFTBRACKET, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RIGHTBRACKET, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_SEMICOLON, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_QUOTE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_BACKSLASH, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_TAB, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LEFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_UP, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_DOWN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RIGHT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LSHIFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RSHIFT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LCTRL, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RCTRL, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_ESCAPE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_BACKSPACE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RETURN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PAGEUP, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_PAGEDOWN, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_HOME, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_END, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_INSERT, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_DELETE, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F1, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F2, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F3, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F4, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F5, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_F6, false));
#if PLATFORM_MACOS
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_LGUI, false));
	keyDownMap.insert (std::pair<SDL_Keycode, bool> (SDLK_RGUI, false));
#endif

	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("a"), SDLK_a));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("b"), SDLK_b));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("c"), SDLK_c));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("d"), SDLK_d));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("e"), SDLK_e));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("f"), SDLK_f));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("g"), SDLK_g));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("h"), SDLK_h));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("i"), SDLK_i));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("j"), SDLK_j));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("k"), SDLK_k));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("l"), SDLK_l));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("m"), SDLK_m));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("n"), SDLK_n));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("o"), SDLK_o));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("p"), SDLK_p));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("q"), SDLK_q));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("r"), SDLK_r));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("s"), SDLK_s));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("t"), SDLK_t));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("u"), SDLK_u));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("v"), SDLK_v));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("w"), SDLK_w));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("x"), SDLK_x));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("y"), SDLK_y));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("z"), SDLK_z));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("0"), SDLK_0));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("1"), SDLK_1));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("2"), SDLK_2));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("3"), SDLK_3));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("4"), SDLK_4));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("5"), SDLK_5));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("6"), SDLK_6));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("7"), SDLK_7));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("8"), SDLK_8));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("9"), SDLK_9));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("comma"), SDLK_COMMA));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("period"), SDLK_PERIOD));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("home"), SDLK_HOME));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("end"), SDLK_END));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("pageup"), SDLK_PAGEUP));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("pagedown"), SDLK_PAGEDOWN));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("left"), SDLK_LEFT));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("right"), SDLK_RIGHT));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("up"), SDLK_UP));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("down"), SDLK_DOWN));
	keyNameMap.insert (std::pair<StdString, SDL_Keycode> (StdString ("escape"), SDLK_ESCAPE));

	return (OpResult::Success);
}

void Input::stop () {
	keyDownMap.clear ();
}

void Input::pollEvents () {
	std::map<SDL_Keycode, bool>::iterator i;
	SDL_Event event;
	int64_t now;

	now = OsUtil::getTime ();
	while (SDL_PollEvent (&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				i = keyDownMap.find (event.key.keysym.sym);
				if (i == keyDownMap.end ()) {
					break;
				}
				if (! i->second) {
					i->second = true;
					isKeyRepeating = false;
					keyRepeatStartTime = now;
					keyRepeatCode = event.key.keysym.sym;
					++keyDownCount;

					SDL_LockMutex (keyPressListMutex);
					keyPressList.push_back (Input::KeyPressEvent (event.key.keysym.sym, isShiftDown (), isControlDown ()));
					isKeyPressListPopulated = true;
					SDL_UnlockMutex (keyPressListMutex);
				}
				break;
			}
			case SDL_KEYUP: {
				i = keyDownMap.find (event.key.keysym.sym);
				if (i == keyDownMap.end ()) {
					break;
				}
				i->second = false;
				if (event.key.keysym.sym == keyRepeatCode) {
					isKeyRepeating = false;
					keyRepeatCode = SDLK_UNKNOWN;
				}
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					isMouseLeftButtonDown = true;
					++mouseLeftDownCount;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT) {
					isMouseRightButtonDown = true;
					++mouseRightDownCount;
				}
				break;
			}
			case SDL_MOUSEBUTTONUP: {
				if (event.button.button == SDL_BUTTON_LEFT) {
					isMouseLeftButtonDown = false;
					++mouseLeftUpCount;
				}
				else if (event.button.button == SDL_BUTTON_RIGHT) {
					isMouseRightButtonDown = false;
					++mouseRightUpCount;
				}
				break;
			}
			case SDL_MOUSEWHEEL: {
				if (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL) {
					if (event.wheel.y < 0) {
						++mouseWheelDownCount;
					}
					else if (event.wheel.y > 0) {
						++mouseWheelUpCount;
					}
				}
				else {
					if (event.wheel.y < 0) {
						++mouseWheelUpCount;
					}
					else if (event.wheel.y > 0) {
						++mouseWheelDownCount;
					}
				}
				break;
			}
			case SDL_WINDOWEVENT: {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					++windowCloseCount;
				}
				break;
			}
		}
	}

	if (keyRepeatCode == SDLK_UNKNOWN) {
		isKeyRepeating = false;
	}
	else {
		if (! isKeyRepeating) {
			if (((now - keyRepeatStartTime) >= keyRepeatStartThreshold) && isKeyDown (keyRepeatCode)) {
				isKeyRepeating = true;
				keyRepeatStartTime = 0;
			}
		}
		if (isKeyRepeating) {
			if ((keyRepeatStartTime <= 0) || ((now - keyRepeatStartTime) >= keyRepeatDelay)) {
				keyRepeatStartTime = now;
				SDL_LockMutex (keyPressListMutex);
				keyPressList.push_back (Input::KeyPressEvent (keyRepeatCode, isShiftDown (), isControlDown ()));
				isKeyPressListPopulated = true;
				SDL_UnlockMutex (keyPressListMutex);
			}
		}
	}

	lastMouseX = mouseX;
	lastMouseY = mouseY;
	SDL_GetMouseState (&mouseX, &mouseY);
}

bool Input::isKeyDown (SDL_Keycode keycode) {
	std::map<SDL_Keycode, bool>::iterator i;

	i = keyDownMap.find (keycode);
	if (i == keyDownMap.end ()) {
		return (false);
	}
	return (i->second);
}

bool Input::isShiftDown () {
	return (isKeyDown (SDLK_LSHIFT) || isKeyDown (SDLK_RSHIFT));
}

bool Input::isControlDown () {
	if (isKeyDown (SDLK_LCTRL) || isKeyDown (SDLK_RCTRL)) {
		return (true);
	}
#if PLATFORM_MACOS
	if (isKeyDown (SDLK_LGUI) || isKeyDown (SDLK_RGUI)) {
		return (true);
	}
#endif
	return (false);
}

void Input::pollKeyPressEvents (std::vector<Input::KeyPressEvent> *destVector) {
	if (! isKeyPressListPopulated) {
		return;
	}
	SDL_LockMutex (keyPressListMutex);
	if (destVector) {
		destVector->swap (keyPressList);
	}
	else {
		keyPressList.clear ();
	}
	isKeyPressListPopulated = false;
	SDL_UnlockMutex (keyPressListMutex);
}

void Input::insertKeyPressEvent (SDL_Keycode keycode) {
	SDL_LockMutex (keyPressListMutex);
	keyPressList.push_back (Input::KeyPressEvent (keycode, false, false));
	isKeyPressListPopulated = true;
	SDL_UnlockMutex (keyPressListMutex);
}

void Input::windowClose () {
	++windowCloseCount;
}

char Input::getKeyCharacter (SDL_Keycode keycode, bool isShiftDown) {
	// TODO: Possibly use a different character map (i.e. for a non-US keyboard layout)
	switch (keycode) {
		case SDLK_a: { return (isShiftDown ? 'A' : 'a'); }
		case SDLK_b: { return (isShiftDown ? 'B' : 'b'); }
		case SDLK_c: { return (isShiftDown ? 'C' : 'c'); }
		case SDLK_d: { return (isShiftDown ? 'D' : 'd'); }
		case SDLK_e: { return (isShiftDown ? 'E' : 'e'); }
		case SDLK_f: { return (isShiftDown ? 'F' : 'f'); }
		case SDLK_g: { return (isShiftDown ? 'G' : 'g'); }
		case SDLK_h: { return (isShiftDown ? 'H' : 'h'); }
		case SDLK_i: { return (isShiftDown ? 'I' : 'i'); }
		case SDLK_j: { return (isShiftDown ? 'J' : 'j'); }
		case SDLK_k: { return (isShiftDown ? 'K' : 'k'); }
		case SDLK_l: { return (isShiftDown ? 'L' : 'l'); }
		case SDLK_m: { return (isShiftDown ? 'M' : 'm'); }
		case SDLK_n: { return (isShiftDown ? 'N' : 'n'); }
		case SDLK_o: { return (isShiftDown ? 'O' : 'o'); }
		case SDLK_p: { return (isShiftDown ? 'P' : 'p'); }
		case SDLK_q: { return (isShiftDown ? 'Q' : 'q'); }
		case SDLK_r: { return (isShiftDown ? 'R' : 'r'); }
		case SDLK_s: { return (isShiftDown ? 'S' : 's'); }
		case SDLK_t: { return (isShiftDown ? 'T' : 't'); }
		case SDLK_u: { return (isShiftDown ? 'U' : 'u'); }
		case SDLK_v: { return (isShiftDown ? 'V' : 'v'); }
		case SDLK_w: { return (isShiftDown ? 'W' : 'w'); }
		case SDLK_x: { return (isShiftDown ? 'X' : 'x'); }
		case SDLK_y: { return (isShiftDown ? 'Y' : 'y'); }
		case SDLK_z: { return (isShiftDown ? 'Z' : 'z'); }
		case SDLK_0: { return (isShiftDown ? ')' : '0'); }
		case SDLK_1: { return (isShiftDown ? '!' : '1'); }
		case SDLK_2: { return (isShiftDown ? '@' : '2'); }
		case SDLK_3: { return (isShiftDown ? '#' : '3'); }
		case SDLK_4: { return (isShiftDown ? '$' : '4'); }
		case SDLK_5: { return (isShiftDown ? '%' : '5'); }
		case SDLK_6: { return (isShiftDown ? '^' : '6'); }
		case SDLK_7: { return (isShiftDown ? '&' : '7'); }
		case SDLK_8: { return (isShiftDown ? '*' : '8'); }
		case SDLK_9: { return (isShiftDown ? '(' : '9'); }
		case SDLK_SPACE: { return (' '); }
		case SDLK_COMMA: { return (isShiftDown ? '<' : ','); }
		case SDLK_PERIOD: { return (isShiftDown ? '>' : '.'); }
		case SDLK_SLASH: { return (isShiftDown ? '?' : '/'); }
		case SDLK_MINUS: { return (isShiftDown ? '_' : '-'); }
		case SDLK_EQUALS: { return (isShiftDown ? '+' : '='); }
		case SDLK_LEFTBRACKET: { return (isShiftDown ? '{' : '['); }
		case SDLK_RIGHTBRACKET: { return (isShiftDown ? '}' : ']'); }
		case SDLK_SEMICOLON: { return (isShiftDown ? ':' : ';'); }
		case SDLK_QUOTE: { return (isShiftDown ? '"' : '\''); }
		case SDLK_BACKSLASH: { return (isShiftDown ? '|' : '\\'); }
	}
	return (0);
}

SDL_Keycode Input::getKeyCode (const StdString &keyName) {
	std::map<StdString, SDL_Keycode>::iterator pos;

	pos = keyNameMap.find (keyName);
	if (pos != keyNameMap.end ()) {
		return (pos->second);
	}
	return (SDLK_UNKNOWN);
}

SDL_Keycode Input::getKeyCode (const char *keyName) {
	return (getKeyCode (StdString (keyName)));
}

void Input::warpMouse (int x, int y) {
	if (x < 0) {
		x = 0;
	}
	if (x > (App::instance->windowWidth - 1)) {
		x = App::instance->windowWidth - 1;
	}
	if (y < 0) {
		y = 0;
	}
	if (y > (App::instance->windowHeight - 1)) {
		y = App::instance->windowHeight - 1;
	}
	SDL_WarpMouseInWindow (App::instance->window, x, y);
}

void Input::leftClickMouse () {
	++mouseLeftDownCount;
	++mouseLeftUpCount;
}
