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
#include "Log.h"
#include "RenderResource.h"
#include "Ui.h"
#include "PlayerUi.h"
#include "AppUtil.h"

AppUtil::AppUtil () {
}
AppUtil::~AppUtil () {
}

Uint32 AppUtil::getSdlInitFlags () {
	return (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
}

OpResult AppUtil::getDisplaySize (int *displayWidth, int *displayHeight) {
	SDL_Rect rect;

	if (SDL_GetDisplayUsableBounds (0, &rect) != 0) {
		Log::warning ("Failed to determine display usable bounds: %s", SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	Log::debug ("Display bounds: x%i,y%i,w%i,h%i", rect.x, rect.y, rect.w, rect.h);
	if (displayWidth) {
		*displayWidth = rect.w;
	}
	if (displayHeight) {
		*displayHeight = rect.h;
	}
	return (OpResult::Success);
}

OpResult AppUtil::createWindowAndRenderer (int windowWidth, int windowHeight, bool isFullscreen, SDL_Window **window, SDL_Renderer **render, Uint32 *windowFlags) {
	SDL_Rect rect;
	Uint32 flags;
	int result;

	flags = 0;
	if (isFullscreen) {
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS;
		if (SDL_GetDisplayUsableBounds (0, &rect) == 0) {
			windowWidth = rect.w;
			windowHeight = rect.h;
		}
	}
	result = SDL_CreateWindowAndRenderer (windowWidth, windowHeight, flags, window, render);
	if (result != 0) {
		Log::err ("Failed to create application window, SDL_CreateWindowAndRenderer: %s", SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	SDL_SetWindowTitle (*window, APPLICATION_NAME);
	if (windowFlags) {
		*windowFlags = SDL_GetWindowFlags (*window);
	}
	return (OpResult::Success);
}

void AppUtil::setDrawableSize (SDL_Window *window, int *windowWidth, int *windowHeight, double *drawableWidth, double *drawableHeight, StdString *imagePrefix) {
	int winw, winh, draww, drawh, mode;
	StdString prefix;

	if (! window) {
		return;
	}
	SDL_GetWindowSize (window, &winw, &winh);
	if ((winw <= 0) || (winh <= 0)) {
		return;
	}
	mode = RenderResource::instance->getFitWindowDisplayMode (winw, winh);
	if (mode < 0) {
		mode = RenderResource::instance->getSmallestWindowDisplayMode ();
	}
	draww = RenderResource::instance->displayModes[mode].width;
	drawh = RenderResource::instance->displayModes[mode].height;
	prefix.sprintf ("%ix%i", draww, drawh);
	if (windowWidth) {
		*windowWidth = winw;
	}
	if (windowHeight) {
		*windowHeight = winh;
	}
	if (drawableWidth) {
		*drawableWidth = (double) winw;
	}
	if (drawableHeight) {
		*drawableHeight = (double) winh;
	}
	if (imagePrefix) {
		imagePrefix->assign (prefix);
	}
}

Ui *AppUtil::createMainUi () {
	return (new PlayerUi ());
}
