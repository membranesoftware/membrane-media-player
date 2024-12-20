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
#include "RoundedCornerSprite.h"
#include "SliderThumbSprite.h"
#include "RenderResource.h"

RenderResource *RenderResource::instance = NULL;

RenderResource::RenderResource ()
: pixelRMask (0)
, pixelGMask (0)
, pixelBMask (0)
, pixelAMask (0)
, roundedCornerSprite (NULL)
, sliderThumbSprite (NULL)
{
	int i;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pixelRMask = 0xFF000000;
	pixelGMask = 0x00FF0000;
	pixelBMask = 0x0000FF00;
	pixelAMask = 0x000000FF;
#else
	pixelRMask = 0x000000FF;
	pixelGMask = 0x0000FF00;
	pixelBMask = 0x00FF0000;
	pixelAMask = 0xFF000000;
#endif

	displayModes[0] = RenderResource::DisplayMode (768, 432, false);
	displayModes[1] = RenderResource::DisplayMode (1024, 576, false);
	displayModes[2] = RenderResource::DisplayMode (1280, 720, false);
	displayModes[3] = RenderResource::DisplayMode (1600, 900, false);
	displayModes[4] = RenderResource::DisplayMode (1920, 1080, false);
	displayModes[5] = RenderResource::DisplayMode (0, 0, true);
	for (i = 0; i < RenderResource::displayModeCount; ++i) {
		displayModes[i].name.sprintf ("%ix%i%s", displayModes[i].width, displayModes[i].height, displayModes[i].isFullscreen ? "f": "");
	}

	fontScales[0] = 0.66f;
	fontScales[1] = 0.8f;
	fontScales[2] = 1.0f;
	fontScales[3] = 1.25f;
	fontScales[4] = 1.5f;
}
RenderResource::~RenderResource () {
	unload ();
}

void RenderResource::createInstance () {
	if (! RenderResource::instance) {
		RenderResource::instance = new RenderResource ();
	}
}
void RenderResource::freeInstance () {
	if (RenderResource::instance) {
		delete (RenderResource::instance);
		RenderResource::instance = NULL;
	}
}

void RenderResource::unload () {
	if (roundedCornerSprite) {
		delete (roundedCornerSprite);
		roundedCornerSprite = NULL;
	}
	if (sliderThumbSprite) {
		delete (sliderThumbSprite);
		sliderThumbSprite = NULL;
	}
}

OpResult RenderResource::load () {
	OpResult result;

	if (roundedCornerSprite) {
		delete (roundedCornerSprite);
	}
	roundedCornerSprite = new RoundedCornerSprite ();
	result = roundedCornerSprite->load ();
	if (result != OpResult::Success) {
		return (result);
	}

	if (sliderThumbSprite) {
		delete (sliderThumbSprite);
	}
	sliderThumbSprite = new SliderThumbSprite ();
	result = sliderThumbSprite->load ();
	if (result != OpResult::Success) {
		return (result);
	}

	return (OpResult::Success);
}

int RenderResource::getSmallestWindowDisplayMode () {
	RenderResource::DisplayMode *mode;
	int result, h, i;

	result = -1;
	h = -1;
	for (i = 0; i < RenderResource::displayModeCount; ++i) {
		mode = &(displayModes[i]);
		if (! mode->isFullscreen) {
			if ((h < 0) || (mode->height < h)) {
				result = i;
				h = mode->height;
			}
		}
	}
	return (result);
}

int RenderResource::getFitWindowDisplayMode (int maxWindowWidth, int maxWindowHeight) {
	RenderResource::DisplayMode *mode;
	int result, w, h, i;
	bool match;

	result = -1;
	w = -1;
	h = -1;
	for (i = 0; i < RenderResource::displayModeCount; ++i) {
		mode = &(displayModes[i]);
		match = false;
		if (! mode->isFullscreen) {
			if ((mode->width <= maxWindowWidth) && (mode->height <= maxWindowHeight)) {
				if ((h < 0) || ((mode->width > w) && (mode->height > h))) {
					match = true;
				}
			}
		}
		if (match) {
			result = i;
			w = mode->width;
			h = mode->height;
		}
	}
	return (result);
}

int RenderResource::getNamedDisplayMode (const StdString &modeName) {
	RenderResource::DisplayMode *mode;
	int i;

	for (i = 0; i < RenderResource::displayModeCount; ++i) {
		mode = &(displayModes[i]);
		if (modeName.equals (StdString::createSprintf ("%ix%i%s", mode->width, mode->height, mode->isFullscreen ? "f" : ""))) {
			return (i);
		}
	}
	return (-1);
}

OpResult RenderResource::resize () {
	return (OpResult::Success);
}
