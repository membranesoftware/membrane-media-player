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
#include "Log.h"
#include "App.h"
#include "Resource.h"
#include "RenderResource.h"
#include "Sprite.h"

Sprite::Sprite ()
: frameCount (0)
, maxWidth (0)
, maxHeight (0)
, pixelCount (0)
, minRenderScale (0.0f)
, maxRenderScale (0.0f)
, renderScaleIncrement (0.0f)
{
}
Sprite::~Sprite () {
	unload ();
}

Sprite *Sprite::copy () {
	Sprite *sprite;
	SDL_Texture *texture;
	std::vector<Sprite::TextureData>::iterator i1, i2;
	int result;

	sprite = new Sprite ();
	i1 = textureList.begin ();
	i2 = textureList.end ();
	while (i1 != i2) {
		texture = Resource::instance->loadTexture (i1->loadPath);
		if (! texture) {
			delete (sprite);
			sprite = NULL;
			break;
		}
		result = sprite->addTexture (texture, i1->loadPath);
		if (result != OpResult::Success) {
			Resource::instance->unloadTexture (i1->loadPath);
			delete (sprite);
			sprite = NULL;
			break;
		}
		++i1;
	}
	sprite->minRenderScale = minRenderScale;
	sprite->maxRenderScale = maxRenderScale;
	sprite->renderScaleIncrement = renderScaleIncrement;
	return (sprite);
}

OpResult Sprite::load (const StdString &path, const StdString &imagePrefix) {
	StdString loadpath;
	SDL_Texture *texture;
	OpResult result;
	int i;
	bool found;

	result = OpResult::Success;
	i = 0;
	maxWidth = 0;
	maxHeight = 0;
	while (true) {
		found = false;
		if (! imagePrefix.empty ()) {
			loadpath.sprintf ("%s/%s/%03i.png", path.c_str (), imagePrefix.c_str (), i);
			if (Resource::instance->fileExists (loadpath)) {
				found = true;
			}
		}
		if (! found) {
			loadpath.sprintf ("%s/%s/%03i.png", path.c_str (), App::instance->imagePrefix.c_str (), i);
			if (Resource::instance->fileExists (loadpath)) {
				found = true;
			}
		}
		if (! found) {
			loadpath.sprintf ("%s/%s/%03i.png", path.c_str (), App::defaultImagePrefix, i);
			if (Resource::instance->fileExists (loadpath)) {
				found = true;
			}
		}
		if (! found) {
			if (i <= 0) {
				result = OpResult::FileOperationFailedError;
			}
			break;
		}
		texture = Resource::instance->loadTexture (loadpath);
		if (! texture) {
			result = OpResult::SdlOperationFailedError;
			break;
		}
		result = addTexture (texture, loadpath);
		if (result != OpResult::Success) {
			Resource::instance->unloadTexture (loadpath);
			break;
		}
		++i;
	}
	return (result);
}

OpResult Sprite::addTexture (SDL_Texture *texture, const StdString &loadPath) {
	Sprite::TextureData item;

	item.texture = texture;
	item.loadPath.assign (loadPath);
	if (SDL_QueryTexture (item.texture, NULL, NULL, &(item.width), &(item.height)) != 0) {
		Log::err ("Failed to query sprite texture; path=\"%s\" err=\"%s\"", item.loadPath.c_str (), SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}
	textureList.push_back (item);
	frameCount = (int) textureList.size ();
	if (item.width > maxWidth) {
		maxWidth = item.width;
	}
	if (item.height > maxHeight) {
		maxHeight = item.height;
	}
	pixelCount += (item.width * item.height);
	return (OpResult::Success);
}

void Sprite::unload () {
	std::vector<Sprite::TextureData>::iterator i1, i2;

	i1 = textureList.begin ();
	i2 = textureList.end ();
	while (i1 != i2) {
		Resource::instance->unloadTexture (i1->loadPath);
		i1->texture = NULL;
		++i1;
	}

	textureList.clear ();
	frameCount = 0;
	maxWidth = 0;
	maxHeight = 0;
	minRenderScale = 0.0f;
	maxRenderScale = 0.0f;
	renderScaleIncrement = 0.0f;
}

SDL_Texture *Sprite::getTexture (int index, int *width, int *height, StdString *loadPath) const {
	Sprite::TextureData item;

	if ((index < 0) || (index >= (int) textureList.size ())) {
		return (NULL);
	}
	item = textureList.at (index);
	if (width) {
		*width = item.width;
	}
	if (height) {
		*height = item.height;
	}
	if (loadPath) {
		loadPath->assign (item.loadPath);
	}
	return (item.texture);
}

StdString Sprite::getLoadPath (int index) const {
	Sprite::TextureData item;

	if ((index < 0) || (index >= (int) textureList.size ())) {
		return (StdString ());
	}
	item = textureList.at (index);
	return (item.loadPath);
}

OpResult Sprite::load (double minScale, double maxScale, double scaleIncrement) {
	OpResult result;
	Uint32 *pixels;
	SDL_Surface *surface;
	SDL_Texture *texture;
	StdString path;
	double scale, lastscale, framew, frameh;
	int surfacew, surfaceh;

	if ((minScale <= 0.0f) || (minScale >= maxScale) || (scaleIncrement <= 0.0f)) {
		return (OpResult::InvalidParamError);
	}
	framew = 0.0f;
	frameh = 0.0f;
	getRenderFrameSize (minScale, &framew, &frameh);
	if ((framew < 1.0f) || (frameh < 1.0f)) {
		return (OpResult::NotImplementedError);
	}

	result = OpResult::Success;
	scale = minScale;
	lastscale = scale;
	while (scale <= maxScale) {
		framew = 0.0f;
		frameh = 0.0f;
		getRenderFrameSize (scale, &framew, &frameh);
		if ((framew < 1.0f) || (frameh < 1.0f)) {
			result = OpResult::MalformedDataError;
			break;
		}
		surfacew = (int) ceil (framew);
		surfaceh = (int) ceil (frameh);
		pixels = (Uint32 *) malloc (surfacew * surfaceh * sizeof (Uint32));
		if (! pixels) {
			Log::err ("Failed to create texture; err=\"Out of memory, dimensions %ix%i\"", surfacew, surfaceh);
			result = OpResult::OutOfMemoryError;
			break;
		}
		writeRenderPixels (scale, pixels, framew, frameh);
		surface = SDL_CreateRGBSurfaceFrom (pixels, surfacew, surfaceh, 32, surfacew * sizeof (Uint32), RenderResource::instance->pixelRMask, RenderResource::instance->pixelGMask, RenderResource::instance->pixelBMask, RenderResource::instance->pixelAMask);
		if (! surface) {
			free (pixels);
			Log::err ("Failed to create texture; err=\"SDL_CreateRGBSurfaceFrom, %s\"", SDL_GetError ());
			result = OpResult::SdlOperationFailedError;
			break;
		}
		path.sprintf ("*_Sprite::load_%llx", (long long int) App::instance->getUniqueId ());
		texture = Resource::instance->createTexture (path, surface);
		SDL_FreeSurface (surface);
		free (pixels);
		if (! texture) {
			result = OpResult::SdlOperationFailedError;
			break;
		}
		addTexture (texture, path);
		lastscale = scale;
		scale += scaleIncrement;
	}

	if (result == OpResult::Success) {
		minRenderScale = minScale;
		maxRenderScale = lastscale;
		renderScaleIncrement = scaleIncrement;
	}
	return (result);
}

SDL_Texture *Sprite::getScaleTexture (double scale, int *width, int *height, int *frameIndex) const {
	SDL_Texture *texture;
	int frame;

	if ((minRenderScale <= 0.0f) || (renderScaleIncrement <= 0.0f)) {
		return (NULL);
	}
	frame = (int) floor ((scale - minRenderScale) / renderScaleIncrement);
	if (frame < 0) {
		frame = 0;
	}
	if (frame > (((int) textureList.size ()) - 1)) {
		frame = ((int) textureList.size ()) - 1;
	}
	texture = getTexture (frame, width, height);
	if (texture) {
		if (frameIndex) {
			*frameIndex = frame;
		}
	}
	return (texture);
}

void Sprite::getRenderFrameSize (double scale, double *frameWidth, double *frameHeight) {
	// Superclass method sets zero values
	if (frameWidth) {
		*frameWidth = 0.0f;
	}
	if (frameHeight) {
		*frameHeight = 0.0f;
	}
}

void Sprite::writeRenderPixels (double scale, Uint32 *pixels, double frameWidth, double frameHeight) {
	// Superclass method takes no action
}
