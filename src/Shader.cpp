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
#include "Color.h"
#include "Resource.h"
#include "Widget.h"
#include "Shader.h"

Shader::Shader ()
: Widget ()
, isAnimationComplete (false)
, isRendering (false)
, shouldRender (false)
, renderMsElapsed (0)
, textureWidth (0)
, textureHeight (0)
, renderTexture (NULL)
, renderPixelFormat (NULL)
, renderPixels (NULL)
, renderPitch (0)
, frameUpdateCount (0)
, frameUpdateInterval (0)
{
	renderPixelFormat = SDL_AllocFormat (SDL_PIXELFORMAT_RGBA32);
}

Shader::~Shader () {
	if (! renderTexturePath.empty ()) {
		Resource::instance->unloadTexture (renderTexturePath);
		renderTexturePath.assign ("");
	}
	renderTexture = NULL;

	if (renderPixelFormat) {
		SDL_FreeFormat (renderPixelFormat);
		renderPixelFormat = NULL;
	}
	renderPixels = NULL;
}

void Shader::doUpdate (int msElapsed) {
	double w, h;
	bool complete;
	int ms;

	if (isRendering) {
		renderMsElapsed += msElapsed;
	}
	else {
		w = width;
		h = height;
		complete = isAnimationComplete;

		ms = msElapsed;
		if (renderMsElapsed > 0) {
			renderMsElapsed += msElapsed;
			if (renderMsElapsed > 0x7FFFFFFF) {
				renderMsElapsed = 0x7FFFFFFF;
			}
			ms = (int) renderMsElapsed;
			renderMsElapsed = 0;
		}
		if (ms > 0) {
			if (updateRenderState (ms)) {
				shouldRender = true;
			}
		}

		if (shouldRender) {
			shouldRender = false;
			isRendering = true;
			retain ();
			App::instance->addPredrawTask (Shader::resetRenderTexture, this);
		}
		if (!(FLOAT_EQUALS (w, width) && FLOAT_EQUALS (h, height))) {
			eventCallback (resizeCallback);
		}
		if ((! complete) && isAnimationComplete) {
			eventCallback (animationCompleteCallback);
		}
	}
}

void Shader::resetRenderTexture (void *itPtr) {
	Shader *it;

	it = (Shader *) itPtr;
	it->executeResetRenderTexture ();
	it->isRendering = false;
	it->release ();
}
void Shader::executeResetRenderTexture () {
	int w, h;

	if ((width >= 1.0f) && (height >= 1.0f)) {
		w = (int) ceil (width);
		h = (int) ceil (height);
		if (renderTexture) {
			if ((textureWidth != w) || (textureHeight != h)) {
				if (! renderTexturePath.empty ()) {
					Resource::instance->unloadTexture (renderTexturePath);
					renderTexturePath.assign ("");
				}
				renderTexture = NULL;
			}
		}
		if (! renderTexture) {
			textureWidth = w;
			textureHeight = h;
			renderTexturePath.sprintf ("*_Shader_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
			renderTexture = Resource::instance->createTexture (renderTexturePath, textureWidth, textureHeight, true);
			if (! renderTexture) {
				renderTexturePath.assign ("");
			}
			else {
				SDL_SetTextureBlendMode (renderTexture, SDL_BLENDMODE_BLEND);
				if (lockRenderTexture ()) {
					memset (renderPixels, 0, textureHeight * renderPitch);
					unlockRenderTexture ();
				}
			}
		}
	}

	if (renderTexture) {
		if (lockRenderTexture ()) {
			updateRenderTexture ();
			unlockRenderTexture ();
		}
	}
}

bool Shader::lockRenderTexture () {
	if (SDL_LockTexture (renderTexture, NULL, &renderPixels, &renderPitch) != 0) {
		Log::err ("Failed to update render texture, SDL_LockTexture: %s", SDL_GetError ());
		return (false);
	}
	return (true);
}

void Shader::unlockRenderTexture () {
	SDL_UnlockTexture (renderTexture);
	renderPixels = NULL;
}

void Shader::drawLine (const Color &drawColor, int lineX1, int lineY1, int lineX2, int lineY2) {
	Uint32 *dest;
	int x, y, dx, dy, xdir, ydir, absdx, absdy, count, bpp;
	uint8_t *pixels;
	Uint32 pixelvalue;
	bool draw;

	if (lineX1 < 0) {
		lineX1 = 0;
	}
	if (lineX1 > (textureWidth - 1)) {
		lineX1 = textureWidth - 1;
	}
	if (lineY1 < 0) {
		lineY1 = 0;
	}
	if (lineY1 > (textureHeight - 1)) {
		lineY1 = textureHeight - 1;
	}
	if (lineX2 < 0) {
		lineX2 = 0;
	}
	if (lineX2 > (textureWidth - 1)) {
		lineX2 = textureWidth - 1;
	}
	if (lineY2 < 0) {
		lineY2 = 0;
	}
	if (lineY2 > (textureHeight - 1)) {
		lineY2 = textureHeight - 1;
	}
	x = lineX1;
	y = lineY1;
	dx = lineX2 - lineX1;
	dy = lineY2 - lineY1;
	if (dx < 0) {
		xdir = -1;
	}
	else if (dx > 0) {
		xdir = 1;
	}
	else {
		xdir = 0;
	}
	if (dy < 0) {
		ydir = -1;
	}
	else if (dy > 0) {
		ydir = 1;
	}
	else {
		ydir = 0;
	}

	pixels = (uint8_t *) renderPixels;
	bpp = renderPixelFormat->BytesPerPixel;
	pixelvalue = SDL_MapRGBA (renderPixelFormat, drawColor.rByte, drawColor.gByte, drawColor.bByte, drawColor.aByte);
	draw = true;
	if ((x < 0) || (y < 0) || (x >= textureWidth) || (y >= textureHeight)) {
		draw = false;
	}
	if (draw) {
		dest = (Uint32 *) (pixels + (y * renderPitch) + (x * bpp));
		*dest = pixelvalue;
	}

	absdx = abs (dx);
	absdy = abs (dy);
	count = 0;
	if (absdx > absdy) {
		dx = absdx - 1;
		while (dx > 0) {
			x += xdir;
			count += absdy;
			if (count >= absdx) {
				count -= absdx;
				y += ydir;
			}

			draw = true;
			if ((x < 0) || (y < 0) || (x >= textureWidth) || (y >= textureHeight)) {
				draw = false;
			}
			if (draw) {
				dest = (Uint32 *) (pixels + (y * renderPitch) + (x * bpp));
				*dest = pixelvalue;
			}

			--dx;
		}
	}
	else {
		dy = absdy - 1;
		while (dy > 0) {
			y += ydir;
			count += absdx;
			if (count >= absdy) {
				count -= absdy;
				x += xdir;
			}

			draw = true;
			if ((x < 0) || (y < 0) || (x >= textureWidth) || (y >= textureHeight)) {
				draw = false;
			}
			if (draw) {
				dest = (Uint32 *) (pixels + (y * renderPitch) + (x * bpp));
				*dest = pixelvalue;
			}

			--dy;
		}
	}

	x = lineX2;
	y = lineY2;
	draw = true;
	if ((x < 0) || (y < 0) || (x >= textureWidth) || (y >= textureHeight)) {
		draw = false;
	}
	if (draw) {
		dest = (Uint32 *) (pixels + (y * renderPitch) + (x * bpp));
		*dest = pixelvalue;
	}
}

void Shader::doDraw (double originX, double originY) {
	SDL_Rect rect;

	if (! renderTexture) {
		return;
	}
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;
	SDL_RenderCopy (App::instance->render, renderTexture, NULL, &rect);
}

bool Shader::updateRenderState (int msElapsed) {
	// Superclass method takes no action
	return (false);
}

void Shader::updateRenderTexture () {
	// Superclass method takes no action
}
