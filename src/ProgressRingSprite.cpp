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
#include "MathUtil.h"
#include "Resource.h"
#include "RenderResource.h"
#include "ProgressRingSprite.h"

const double fillIncrement = 2.5f;

ProgressRingSprite::ProgressRingSprite ()
: Sprite ()
{
	memset (fillFrames, 0, sizeof (fillFrames));
}
ProgressRingSprite::~ProgressRingSprite () {
}

SDL_Texture *ProgressRingSprite::getRingTexture (double percentFill, int *width, int *height) const {
	int index;

	index = (int) percentFill;
	if (index < 0) {
		index = 0;
	}
	else if (index > 99) {
		index = 99;
	}
	return (getTexture (fillFrames[index], width, height));
}

OpResult ProgressRingSprite::load (double scale) {
	OpResult result;
	double fill, lastfill;
	int framesize, fillindex;
	Uint32 *pixels;
	SDL_Surface *surface;
	SDL_Texture *texture;
	StdString path;

	result = OpResult::Success;
	framesize = (int) scale;
	fillindex = 0;
	fillFrames[fillindex] = 0;
	++fillindex;

	fill = 0.0f;
	while (fill <= 100.0f) {
		pixels = (Uint32 *) malloc (framesize * framesize * sizeof (Uint32));
		if (! pixels) {
			Log::err ("Failed to create texture; err=\"Out of memory, dimensions %ix%i\"", framesize, framesize);
			result = OpResult::OutOfMemoryError;
			break;
		}
		writeProgressRingRenderPixels (scale, fill, pixels);
		surface = SDL_CreateRGBSurfaceFrom (pixels, framesize, framesize, 32, framesize * sizeof (Uint32), RenderResource::instance->pixelRMask, RenderResource::instance->pixelGMask, RenderResource::instance->pixelBMask, RenderResource::instance->pixelAMask);
		if (! surface) {
			free (pixels);
			Log::err ("Failed to create texture; err=\"SDL_CreateRGBSurfaceFrom, %s\"", SDL_GetError ());
			result = OpResult::SdlOperationFailedError;
			break;
		}
		path.sprintf ("*_ProgressRingSprite::load_%llx", (long long int) App::instance->getUniqueId ());
		texture = Resource::instance->createTexture (path, surface);
		SDL_FreeSurface (surface);
		free (pixels);
		if (! texture) {
			result = OpResult::SdlOperationFailedError;
			break;
		}
		addTexture (texture, path);

		while (fillindex < (int) fill) {
			fillFrames[fillindex] = frameCount - 1;
			++fillindex;
		}
		lastfill = fill;
		fill += fillIncrement;
		if ((lastfill < 100.0f) && (fill > 100.0f)) {
			fill = 100.0f;
		}
	}
	return (result);
}

void ProgressRingSprite::writeProgressRingRenderPixels (double scale, double percentFill, Uint32 *pixels) {
	Uint32 *dest, color;
	double dist, targetalpha, framedist, cx, cy, x, y, mindist, maxdist, ringdist, direction, pct;
	uint8_t alpha;

	if (scale <= 0.0f) {
		return;
	}
	const double minAlpha = 0.02f;
	const double innerRingScale = 0.21f;
	const double outerRingScale = 0.72f;
	const double opacity = 1.0f;
	dest = pixels;
	cx = scale / 2.0f;
	cy = scale / 2.0f;
	framedist = MathUtil::getDistance (cx, cy);
	mindist = framedist * innerRingScale;
	maxdist = framedist * outerRingScale;
	ringdist = mindist + ((maxdist - mindist) / 2.0f);
	y = 0.0f;
	while (y < scale) {
		x = 0.0f;
		while (x < scale) {
			dist = MathUtil::getDistance (cx - (x + 0.5f), cy - (y + 0.5f), 0.0f, 0.0f);
			if (dist < mindist) {
				targetalpha = ((1.0f - minAlpha) * (dist / framedist));
			}
			else if (dist > maxdist) {
				targetalpha = 1.0f - ((1.0f - minAlpha) * (dist / framedist));
				targetalpha *= 0.33f;
			}
			else if (dist > (maxdist * 0.9f)) {
				dist = fabs (dist - ringdist);
				targetalpha = 1.0f - ((1.0f - minAlpha) * (dist / ringdist));
			}
			else {
				dist = fabs (dist - ringdist);
				targetalpha = 1.0f - ((1.0f - minAlpha) * (dist / ringdist));
				targetalpha *= 1.45f;
			}

			direction = MathUtil::getVectorDirection ((x + 0.5f) - cx, (y + 0.5f) - cy);
			pct = 100.0f;
			if (percentFill <= 0.0f) {
				targetalpha = 0.0f;
			}
			else {
				if (direction <= 90.0f) {
					pct = ((90.0f - direction) / 90.0f) * 25.0f;
				}
				else if (direction <= 180.0f) {
					pct = 75.0f + (((180.0f - direction) / 90.0f) * 25.0f);
				}
				else if (direction <= 270.0f) {
					pct = 50.0f + (((270.0f - direction) / 90.0f) * 25.0f);
				}
				else {
					pct = 25.0f + (((360.0f - direction) / 90.0f) * 25.0f);
				}
				if (pct > percentFill) {
					targetalpha = 0.0f;
				}
			}

			if (targetalpha <= 0.0f) {
				targetalpha = 0.0f;
			}
			else {
				targetalpha *= opacity;
				if (targetalpha > 1.0f) {
					targetalpha = 1.0f;
				}
			}
			alpha = (uint8_t) (targetalpha * 255.0f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			color = 0xFFFFFF00 | (alpha & 0xFF);
#else
			color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
			*dest = color;
			++dest;
			x += 1.0f;
		}
		y += 1.0f;
	}
}
