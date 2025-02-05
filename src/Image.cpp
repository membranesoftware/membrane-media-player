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
#include "Sprite.h"
#include "Resource.h"
#include "Image.h"

Image::Image (Sprite *sprite, int spriteFrame, bool shouldDestroySprite)
: Widget ()
, drawAlpha (1.0f)
, isResizeDisabled (false)
, sprite (sprite)
, spriteTexture (NULL)
, spriteTextureWidth (0)
, spriteTextureHeight (0)
, spriteFrame (spriteFrame)
, maxSpriteWidth (0.0f)
, maxSpriteHeight (0.0f)
, drawScale (1.0f)
, isDrawColorEnabled (false)
, shouldDestroySprite (shouldDestroySprite)
{
	SdlUtil::createMutex (&spriteTextureMutex);
	maxSpriteWidth = (double) sprite->maxWidth;
	maxSpriteHeight = (double) sprite->maxHeight;
	spriteTexture = sprite->getTexture (spriteFrame, &spriteTextureWidth, &spriteTextureHeight, &spriteTexturePath);
	if (! Resource::instance->loadTexture (spriteTexturePath, true)) {
		spriteTexturePath.assign ("");
	}
	resetSize ();
}
Image::~Image () {
	spriteTexture = NULL;
	if (! spriteTexturePath.empty ()) {
		Resource::instance->unloadTexture (spriteTexturePath);
		spriteTexturePath.assign ("");
	}
	if (shouldDestroySprite) {
		sprite->unload ();
		delete (sprite);
	}
	sprite = NULL;
	SdlUtil::destroyMutex (&spriteTextureMutex);
}

void Image::setSprite (Sprite *targetSprite, int frame) {
	SDL_Texture *texture;
	StdString path;
	int w, h;

	if ((! targetSprite) || (frame < 0) || (frame >= targetSprite->frameCount)) {
		return;
	}
	if (shouldDestroySprite) {
		sprite->unload ();
		delete (sprite);
	}
	shouldDestroySprite = false;
	sprite = targetSprite;
	spriteFrame = frame;

	texture = sprite->getTexture (spriteFrame, &w, &h, &path);
	if (! texture) {
		return;
	}
	if (! spriteTexturePath.empty ()) {
		Resource::instance->unloadTexture (spriteTexturePath);
	}
	spriteTexturePath.assign (path);
	if (! Resource::instance->loadTexture (spriteTexturePath, true)) {
		spriteTexturePath.assign ("");
	}
	SDL_LockMutex (spriteTextureMutex);
	spriteTexture = texture;
	spriteTextureWidth = w;
	spriteTextureHeight = h;
	SDL_UnlockMutex (spriteTextureMutex);
	maxSpriteWidth = (double) sprite->maxWidth;
	maxSpriteHeight = (double) sprite->maxHeight;
	resetSize ();
}

void Image::setScale (double scale) {
	if (scale <= 0.0f) {
		return;
	}
	drawScale = scale;
	resetSize ();
}

void Image::setFrame (int frame) {
	SDL_Texture *texture;
	StdString path;
	int w, h;

	if ((spriteFrame == frame) || (frame < 0) || (frame >= sprite->frameCount)) {
		return;
	}
	spriteFrame = frame;

	texture = sprite->getTexture (spriteFrame, &w, &h, &path);
	if (! texture) {
		return;
	}
	if (! spriteTexturePath.empty ()) {
		Resource::instance->unloadTexture (spriteTexturePath);
	}
	spriteTexturePath.assign (path);
	if (! Resource::instance->loadTexture (spriteTexturePath, true)) {
		spriteTexturePath.assign ("");
	}
	SDL_LockMutex (spriteTextureMutex);
	spriteTexture = texture;
	spriteTextureWidth = w;
	spriteTextureHeight = h;
	SDL_UnlockMutex (spriteTextureMutex);
	resetSize ();
}

void Image::setSpriteDestroy (bool enable) {
	shouldDestroySprite = enable;
}

void Image::setDrawColor (bool enable, const Color &color) {
	isDrawColorEnabled = enable;
	if (isDrawColorEnabled) {
		drawColor.assign (color);
	}
}

void Image::translateAlpha (double startAlpha, double targetAlpha, int durationMs) {
	if (startAlpha < 0.0f) {
		startAlpha = 0.0f;
	}
	else if (startAlpha > 1.0f) {
		startAlpha = 1.0f;
	}
	if (targetAlpha < 0.0f) {
		targetAlpha = 0.0f;
	}
	else if (targetAlpha > 1.0f) {
		targetAlpha = 1.0f;
	}

	drawAlpha = startAlpha;
	translateAlphaValue.translateX (startAlpha, targetAlpha, durationMs);
}

void Image::resetSize () {
	SDL_LockMutex (spriteTextureMutex);
	width = ((double) spriteTextureWidth) * drawScale;
	height = ((double) spriteTextureHeight) * drawScale;
	SDL_UnlockMutex (spriteTextureMutex);
}

void Image::doUpdate (int msElapsed) {
	if (translateAlphaValue.isTranslating) {
		translateAlphaValue.update (msElapsed);
		drawAlpha = translateAlphaValue.x;
	}
	if (isDrawColorEnabled) {
		drawColor.update (msElapsed);
	}
}

void Image::doDraw (double originX, double originY) {
	SDL_Rect rect;

	SDL_LockMutex (spriteTextureMutex);
	if (spriteTexture) {
		rect.x = (int) (originX + position.x);
		rect.y = (int) (originY + position.y);
		rect.w = (int) width;
		rect.h = (int) height;

		if (drawAlpha < 0.0f) {
			SDL_SetTextureBlendMode (spriteTexture, SDL_BLENDMODE_NONE);
		}
		else {
			SDL_SetTextureAlphaMod (spriteTexture, (Uint8) (drawAlpha * 255.0f));
			SDL_SetTextureBlendMode (spriteTexture, SDL_BLENDMODE_BLEND);
		}
		if (isDrawColorEnabled) {
			SDL_SetTextureColorMod (spriteTexture, drawColor.rByte, drawColor.gByte, drawColor.bByte);
		}
		SDL_RenderCopy (App::instance->render, spriteTexture, NULL, &rect);
		if (isDrawColorEnabled) {
			SDL_SetTextureColorMod (spriteTexture, 255, 255, 255);
		}
	}
	SDL_UnlockMutex (spriteTextureMutex);
}

void Image::doResize () {
	if ((! sprite) || shouldDestroySprite || isResizeDisabled) {
		return;
	}
	setSprite (sprite, spriteFrame);
}
