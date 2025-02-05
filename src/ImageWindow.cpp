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
#include "SDL2/SDL_image.h"
#include "App.h"
#include "ClassId.h"
#include "Log.h"
#include "SpriteId.h"
#include "UiConfiguration.h"
#include "Network.h"
#include "MediaUtil.h"
#include "TaskGroup.h"
#include "Resource.h"
#include "SharedBuffer.h"
#include "MediaReader.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "ImageWindow.h"

constexpr const int NoResize = 0;
constexpr const int ScaleResize = 1;
constexpr const int FitResize = 2;

ImageWindow::ImageWindow (Image *imageWidget)
: Panel ()
, isLoadingSpriteDisabled (false)
, isOffscreenUnloadEnabled (false)
, isLoadSuspended (false)
, loadingSprite (NULL)
, loadingSpriteColor (0.0f, 0.0f, 0.0f)
, imageLoadType (ImageWindow::NoLoadType)
, isImageFileExternal (false)
, isVideoFileExternal (false)
, videoFrameTimestamp (0)
, imageLoadSourceWidth (0.0f)
, imageLoadSourceHeight (0.0f)
, isLoadingImageData (false)
, isLoadFailed (false)
, imageHandle (&image)
, loadingProgressRingHandle (&loadingProgressRing)
, drawAlpha (1.0f)
, isWindowSizeEnabled (false)
, windowWidth (0.0f)
, windowHeight (0.0f)
, loadingWidth (0.0f)
, loadingHeight (0.0f)
, isImageDataLoaded (false)
, shouldInvokeLoadEndCallback (false)
, imageUrlData (NULL)
, mediaReader (NULL)
, videoFrameSeekPercent (0.0f)
, videoFrameSeekTimestamp (-1)
, onLoadResizeType (NoResize)
, onLoadWidth (0.0f)
, onLoadHeight (0.0f)
, nextImageLoadType (ImageWindow::NoLoadType)
, nextIsImageFileExternal (false)
, nextIsVideoFileExternal (false)
, nextVideoFrameSeekTimestamp (-1)
, nextVideoFrameSeekPercent (0.0f)
{
	classId = ClassId::ImageWindow;
	loadingSprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_loadingImageIcon);
	loadErrorSprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_loadingImageErrorIcon);
	if (imageWidget) {
		imageHandle.assign (imageWidget);
		addWidget (imageWidget);
	}
}
ImageWindow::~ImageWindow () {
	if (mediaReader) {
		mediaReader->release ();
		mediaReader = NULL;
	}
	if (imageUrlData) {
		imageUrlData->release ();
		imageUrlData = NULL;
	}
	imageHandle.destroyAndClear ();
}

ImageWindow *ImageWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::ImageWindow) ? (ImageWindow *) widget : NULL);
}

void ImageWindow::setWindowSize (bool enable, double windowSizeWidth, double windowSizeHeight) {
	if (! enable) {
		isWindowSizeEnabled = false;
	}
	else {
		if ((windowSizeWidth >= 1.0f) && (windowSizeHeight >= 1.0f)) {
			isWindowSizeEnabled = true;
			windowWidth = windowSizeWidth;
			windowHeight = windowSizeHeight;
		}
	}
	reflow ();
}

bool ImageWindow::isLoaded () {
	if (! image) {
		return (false);
	}
	return (isImageDataLoaded && (! shouldInvokeLoadEndCallback));
}

bool ImageWindow::isOffscreen () {
	double w, h;

	if (! isOffscreenUnloadEnabled) {
		return (false);
	}
	if (! hasScreenPosition) {
		return (true);
	}
	w = App::instance->drawableWidth * UiConfiguration::instance->imageWindowOffscreenAreaMultiplier;
	h = App::instance->drawableHeight * UiConfiguration::instance->imageWindowOffscreenAreaMultiplier;
	if ((screenX >= -w) && (screenX <= w)) {
		if ((screenY >= -h) && (screenY <= h)) {
			return (false);
		}
	}
	return (true);
}

void ImageWindow::setImage (Image *nextImage) {
	if (image) {
		image->zLevel = -1;
		image->setDestroyDelay (1);
	}
	imageHandle.assign (nextImage);
	if (image) {
		image->drawAlpha = drawAlpha;
		addWidget (image);
	}
	reflow ();
}

void ImageWindow::setFrame (int frame) {
	if (! image) {
		return;
	}
	image->setFrame (frame);
	reflow ();
}

void ImageWindow::setScale (double scale) {
	if (! image) {
		return;
	}
	image->setScale (scale);
	reflow ();
}

void ImageWindow::setDrawColor (bool enable, const Color &color) {
	if (! image) {
		return;
	}
	image->setDrawColor (enable, color);
	reflow ();
}

void ImageWindow::setDrawAlpha (double alphaValue) {
	if (alphaValue > 1.0f) {
		alphaValue = 1.0f;
	}
	drawAlpha = alphaValue;
	if (image) {
		image->drawAlpha = drawAlpha;
	}
}

void ImageWindow::setLoadingSize (double loadingWidthValue, double loadingHeightValue) {
	loadingWidth = loadingWidthValue;
	loadingHeight = loadingHeightValue;
	reflow ();
}

void ImageWindow::showLoadingSprite () {
	if (isLoadingSpriteDisabled) {
		return;
	}
	loadingProgressRingHandle.destroyAndAssign (new ProgressRing (UiConfiguration::instance->progressRingSize));
	loadingProgressRing->setIndeterminate (true);
	add (loadingProgressRing, -1);
	setImage (new Image (loadingSprite));
	image->setDrawColor (true, loadingSpriteColor);
}

void ImageWindow::showLoadErrorSprite () {
	loadingProgressRingHandle.destroyAndClear ();
	setImage (new Image (loadErrorSprite));
	image->setDrawColor (true, loadingSpriteColor);
}

void ImageWindow::loadImageFile (const StdString &filePath, bool isExternalPath) {
	if (filePath.empty ()) {
		return;
	}
	if ((imageLoadType == ImageWindow::ImageFileLoadType) && imageFilePath.equals (filePath) && (isImageFileExternal == isExternalPath)) {
		return;
	}
	if (isLoadingImageData) {
		nextImageFilePath.assign (filePath);
		nextIsImageFileExternal = isExternalPath;
		nextImageLoadType = ImageWindow::ImageFileLoadType;
		return;
	}
	imageLoadType = ImageWindow::ImageFileLoadType;
	imageFilePath.assign (filePath);
	isImageFileExternal = isExternalPath;
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
	isLoadFailed = false;
	isImageDataLoaded = false;
}

void ImageWindow::loadImageUrl (const StdString &loadUrl) {
	if (loadUrl.empty ()) {
		return;
	}
	if ((imageLoadType == ImageWindow::UrlLoadType) && imageUrl.equals (loadUrl)) {
		return;
	}
	if (isLoadingImageData) {
		nextImageUrl.assign (loadUrl);
		nextImageLoadType = ImageWindow::UrlLoadType;
		return;
	}
	imageLoadType = ImageWindow::UrlLoadType;
	imageUrl.assign (loadUrl);
	showLoadingSprite ();
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
	isLoadFailed = false;
	isImageDataLoaded = false;
}

void ImageWindow::loadSeekPercentVideoFrame (const StdString &filePath, double seekPercent, bool isExternalPath) {
	if (filePath.empty ()) {
		return;
	}
	if (seekPercent < 0.0f) {
		seekPercent = 0.0f;
	}
	if (seekPercent > 100.0f) {
		seekPercent = 100.0f;
	}
	if ((imageLoadType == ImageWindow::VideoFrameLoadType) && videoFilePath.equals (filePath) && (isVideoFileExternal == isExternalPath) && FLOAT_EQUALS (videoFrameSeekPercent, seekPercent) && (videoFrameSeekTimestamp < 0)) {
		return;
	}
	if (isLoadingImageData) {
		nextVideoFilePath.assign (filePath);
		nextIsVideoFileExternal = isExternalPath;
		nextVideoFrameSeekTimestamp = -1;
		nextVideoFrameSeekPercent = seekPercent;
		nextImageLoadType = ImageWindow::VideoFrameLoadType;
		return;
	}
	imageLoadType = ImageWindow::VideoFrameLoadType;
	videoFilePath.assign (filePath);
	videoFrameSeekPercent = seekPercent;
	videoFrameSeekTimestamp = -1;
	isVideoFileExternal = isExternalPath;
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
	isLoadFailed = false;
	isImageDataLoaded = false;
}

void ImageWindow::loadSeekTimestampVideoFrame (const StdString &filePath, int64_t seekTimestamp, bool isExternalPath) {
	if (filePath.empty ()) {
		return;
	}
	if (seekTimestamp < 0) {
		seekTimestamp = 0;
	}
	if ((imageLoadType == ImageWindow::VideoFrameLoadType) && videoFilePath.equals (filePath) && (isVideoFileExternal == isExternalPath) && (videoFrameSeekTimestamp == seekTimestamp)) {
		return;
	}
	if (isLoadingImageData) {
		nextVideoFilePath.assign (filePath);
		nextIsVideoFileExternal = isExternalPath;
		nextVideoFrameSeekTimestamp = seekTimestamp;
		nextImageLoadType = ImageWindow::VideoFrameLoadType;
		return;
	}
	imageLoadType = ImageWindow::VideoFrameLoadType;
	videoFilePath.assign (filePath);
	videoFrameSeekTimestamp = seekTimestamp;
	isVideoFileExternal = isExternalPath;
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
	isLoadFailed = false;
	isImageDataLoaded = false;
}

void ImageWindow::loadSourceCopy (ImageWindow *sourceImage) {
	switch (sourceImage->imageLoadType) {
		case ImageWindow::ImageFileLoadType: {
			loadImageFile (sourceImage->imageFilePath, sourceImage->isImageFileExternal);
			break;
		}
		case ImageWindow::UrlLoadType: {
			loadImageUrl (sourceImage->imageUrl);
			break;
		}
		case ImageWindow::VideoFrameLoadType: {
			if (sourceImage->videoFrameSeekTimestamp >= 0) {
				loadSeekTimestampVideoFrame (sourceImage->videoFilePath, sourceImage->videoFrameSeekTimestamp, sourceImage->isVideoFileExternal);
			}
			else {
				loadSeekPercentVideoFrame (sourceImage->videoFilePath, sourceImage->videoFrameSeekPercent, sourceImage->isVideoFileExternal);
			}
			break;
		}
	}
}

void ImageWindow::onLoadScale (double scaleWidth, double scaleHeight) {
	onLoadResizeType = ScaleResize;
	onLoadWidth = scaleWidth;
	onLoadHeight = scaleHeight;
}

void ImageWindow::onLoadFit (double targetWidth, double targetHeight) {
	if ((targetWidth <= 0.0f) || (targetHeight <= 0.0f)) {
		return;
	}
	onLoadResizeType = FitResize;
	onLoadWidth = targetWidth;
	onLoadHeight = targetHeight;
}

bool ImageWindow::isImageUrlEmpty () {
	return (imageUrl.empty ());
}

void ImageWindow::reload () {
	if (isLoadingImageData) {
		return;
	}
	if (isImageDataLoaded) {
		showLoadingSprite ();
		isImageDataLoaded = false;
	}
}

void ImageWindow::reflow () {
	double w, h;

	if (isWindowSizeEnabled) {
		setFixedSize (true, windowWidth, windowHeight);
		if (image) {
			image->position.assign ((width / 2.0f) - (image->width / 2.0f), (height / 2.0f) - (image->height / 2.0f));
		}
	}
	else if (image && (image->sprite == loadingSprite) && (loadingWidth >= 1.0f) && (loadingHeight >= 1.0f)) {
		setFixedSize (true, loadingWidth, loadingHeight);
		image->position.assign ((width / 2.0f) - (image->width / 2.0f), (height / 2.0f) - (image->height / 2.0f));
	}
	else {
		w = widthPadding * 2.0f;
		h = heightPadding * 2.0f;
		if (image) {
			image->position.assign (widthPadding, heightPadding);
			w += image->width;
			h += image->height;
		}
		setFixedSize (true, w, h);
	}

	if (loadingProgressRing) {
		loadingProgressRing->position.assign ((width / 2.0f) - (loadingProgressRing->width / 2.0f), (height / 2.0f) - (loadingProgressRing->height / 2.0f));
	}
}

void ImageWindow::doUpdate (int msElapsed) {
	bool shouldload;

	Panel::doUpdate (msElapsed);
	if (! isLoadingImageData) {
		switch (nextImageLoadType) {
			case ImageWindow::ImageFileLoadType: {
				loadImageFile (nextImageFilePath, nextIsImageFileExternal);
				nextImageFilePath.assign ("");
				nextImageLoadType = ImageWindow::NoLoadType;
				break;
			}
			case ImageWindow::UrlLoadType: {
				loadImageUrl (nextImageUrl);
				nextImageUrl.assign ("");
				nextImageLoadType = ImageWindow::NoLoadType;
				break;
			}
			case ImageWindow::VideoFrameLoadType: {
				if (nextVideoFrameSeekTimestamp >= 0) {
					loadSeekTimestampVideoFrame (nextVideoFilePath, nextVideoFrameSeekTimestamp, nextIsVideoFileExternal);
				}
				else {
					loadSeekPercentVideoFrame (nextVideoFilePath, nextVideoFrameSeekPercent, nextIsVideoFileExternal);
				}
				nextVideoFilePath.assign ("");
				nextVideoFrameSeekTimestamp = -1;
				nextImageLoadType = ImageWindow::NoLoadType;
				break;
			}
		}
	}
	if (imageLoadType != ImageWindow::NoLoadType) {
		shouldload = !(isLoadSuspended || isOffscreen () || isLoadFailed || isImageDataLoaded || isLoadingImageData);
		if (! shouldload) {
			if ((isLoadSuspended || isOffscreen ()) && isImageDataLoaded && (! isLoadingImageData)) {
				setImage (NULL);
				isImageDataLoaded = false;
			}
		}
		else {
			switch (imageLoadType) {
				case ImageWindow::ImageFileLoadType: {
					if (imageFilePath.empty ()) {
						break;
					}
					isLoadingImageData = true;
					eventCallback (loadStartCallback);
					retain ();
					App::instance->addPredrawTask (ImageWindow::createImageFromImageFile, this);
					break;
				}
				case ImageWindow::UrlLoadType: {
					if (imageUrl.empty ()) {
						break;
					}
					showLoadingSprite ();
					isLoadingImageData = true;
					eventCallback (loadStartCallback);
					retain ();
					Network::instance->sendHttpGet (imageUrl, Network::HttpRequestCallbackContext (ImageWindow::createImageFromUrlHttpGetComplete, this));
					break;
				}
				case ImageWindow::VideoFrameLoadType: {
					if (videoFilePath.empty ()) {
						break;
					}
					showLoadingSprite ();
					isLoadingImageData = true;
					eventCallback (loadStartCallback);
					retain ();
					TaskGroup::instance->run (TaskGroup::RunContext (ImageWindow::readVideoFrame, this));
					break;
				}
			}
		}
	}

	if (loadingProgressRing) {
		if (! isLoadingImageData) {
			loadingProgressRingHandle.destroyAndClear ();
		}
	}

	if (shouldInvokeLoadEndCallback) {
		shouldInvokeLoadEndCallback = false;
		eventCallback (loadEndCallback);
	}
}

void ImageWindow::createImageFromImageFile (void *itPtr) {
	((ImageWindow *) itPtr)->executeCreateImageFromImageFile ();
}
void ImageWindow::executeCreateImageFromImageFile () {
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;
	double scaledw, scaledh;

	if (isDestroyed) {
		imageFilePath.assign ("");
		endCreateImageFromImageFile ();
		return;
	}
	surface = NULL;
	if (isImageFileExternal) {
		rw = SDL_RWFromFile (imageFilePath.c_str (), "r");
		if (! rw) {
			endCreateImageFromImageFile (StdString::createSprintf ("File open failed, SDL_RWFromFile: %s", SDL_GetError ()));
			return;
		}
		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			endCreateImageFromImageFile (StdString::createSprintf ("File data parse failed, IMG_Load_RW: %s", SDL_GetError ()));
			return;
		}
	}
	else {
		surface = Resource::instance->loadSurface (imageFilePath);
		if (! surface) {
			endCreateImageFromImageFile (StdString::createSprintf ("Failed to load image data from resources, %s", Resource::instance->lastErrorMessage.c_str ()));
			return;
		}
	}

	imageLoadSourceWidth = (double) surface->w;
	imageLoadSourceHeight = (double) surface->h;
	if (getOnLoadScaleSize (&scaledw, &scaledh)) {
		scaledsurface = SDL_CreateRGBSurface (0, (int) floor (scaledw), (int) floor (scaledh), surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (scaledsurface) {
			SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
			SDL_FreeSurface (surface);
			surface = scaledsurface;
		}
	}

	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		endCreateImageFromImageFile (StdString::createSprintf ("Failed to create render texture, %s", Resource::instance->lastErrorMessage.c_str ()));
		return;
	}
	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	setImage (new Image (sprite, 0, true));
	isImageDataLoaded = true;
	reflow ();
	endCreateImageFromImageFile ();
}
void ImageWindow::endCreateImageFromImageFile (const StdString &errorMessage) {
	loadErrorMessage.assign (errorMessage);
	if (! loadErrorMessage.empty ()) {
		isLoadFailed = true;
		showLoadErrorSprite ();
		Log::debug ("Image load from file failed; path=\"%s\" err=%s", imageFilePath.c_str (), errorMessage.c_str ());
	}
	if (loadEndCallback.callback) {
		shouldInvokeLoadEndCallback = true;
	}
	isLoadingImageData = false;
	release ();
}

void ImageWindow::createImageFromUrlHttpGetComplete (void *itPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	ImageWindow *it = (ImageWindow *) itPtr;

	if (it->isDestroyed || it->isOffscreen ()) {
		it->endCreateImageFromUrl ();
		return;
	}
	if (statusCode != Network::HttpOkCode) {
		it->endCreateImageFromUrl (StdString ("Non-success response from server"));
		return;
	}
	if ((! responseData) || responseData->empty ()) {
		it->endCreateImageFromUrl (StdString ("Empty response from server"));
		return;
	}
	if (it->imageUrlData) {
		it->imageUrlData->release ();
	}
	it->imageUrlData = responseData;
	it->imageUrlData->retain ();
	App::instance->addPredrawTask (ImageWindow::createImageFromUrlResponseData, it);
}
void ImageWindow::createImageFromUrlResponseData (void *itPtr) {
	((ImageWindow *) itPtr)->executeCreateImageFromUrlResponseData ();
}
void ImageWindow::executeCreateImageFromUrlResponseData () {
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;
	double scaledw, scaledh;

	if (isDestroyed || isOffscreen () || (! imageUrlData)) {
		endCreateImageFromUrl ();
		return;
	}
	rw = SDL_RWFromConstMem (imageUrlData->data, imageUrlData->length);
	if (! rw) {
		endCreateImageFromUrl (StdString::createSprintf ("Failed to create render texture, SDL_RWFromConstMem: %s", SDL_GetError ()));
		return;
	}
	surface = IMG_Load_RW (rw, 1);
	if (! surface) {
		endCreateImageFromUrl (StdString::createSprintf ("Failed to create render texture, IMG_Load_RW: %s", SDL_GetError ()));
		return;
	}
	imageLoadSourceWidth = (double) surface->w;
	imageLoadSourceHeight = (double) surface->h;
	if (getOnLoadScaleSize (&scaledw, &scaledh)) {
		scaledsurface = SDL_CreateRGBSurface (0, (int) floor (scaledw), (int) floor (scaledh), surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (scaledsurface) {
			SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
			SDL_FreeSurface (surface);
			surface = scaledsurface;
		}
	}

	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		endCreateImageFromUrl (StdString::createSprintf ("Failed to create render texture, %s", Resource::instance->lastErrorMessage.c_str ()));
		return;
	}
	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	setImage (new Image (sprite, 0, true));
	isImageDataLoaded = true;
	reflow ();
	endCreateImageFromUrl ();
}
void ImageWindow::endCreateImageFromUrl (const StdString &errorMessage) {
	loadErrorMessage.assign (errorMessage);
	if (! loadErrorMessage.empty ()) {
		isLoadFailed = true;
		showLoadErrorSprite ();
		Log::debug ("Image load from URL failed; url=\"%s\" err=%s", imageUrl.c_str (), errorMessage.c_str ());
	}
	if (imageUrlData) {
		imageUrlData->release ();
		imageUrlData = NULL;
	}
	if (loadEndCallback.callback) {
		shouldInvokeLoadEndCallback = true;
	}
	isLoadingImageData = false;
	release ();
}

void ImageWindow::readVideoFrame (void *itPtr) {
	((ImageWindow *) itPtr)->executeReadVideoFrame ();
}
void ImageWindow::executeReadVideoFrame () {
	double w, h;

	if (mediaReader) {
		mediaReader->release ();
	}
	mediaReader = new MediaReader ();
	mediaReader->retain ();
	mediaReader->setMediaPath (videoFilePath, (! isVideoFileExternal));
	if (mediaReader->readMetadata () != OpResult::Success) {
		endCreateImageFromVideoFrame (mediaReader->lastErrorMessage);
		return;
	}
	imageLoadSourceWidth = mediaReader->videoFrameWidth;
	imageLoadSourceHeight = mediaReader->videoFrameHeight;
	if (! getOnLoadScaleSize (&w, &h)) {
		w = 0.0f;
		h = 0.0f;
	}
	if (videoFrameSeekTimestamp >= 0) {
		mediaReader->setVideoFrameSeekTimestamp (videoFrameSeekTimestamp);
	}
	else {
		mediaReader->setVideoFrameSeekPercent (videoFrameSeekPercent);
	}
	mediaReader->readVideoFrame ((int) w, (int) h);
	if (! mediaReader->videoFrameData) {
		endCreateImageFromVideoFrame (mediaReader->lastErrorMessage);
		return;
	}
	videoFrameTimestamp = mediaReader->videoFrameTimestamp;
	mediaReader->createVideoFrameTexture (ImageWindow::createVideoFrameTextureComplete, this);
}
bool ImageWindow::createVideoFrameTextureComplete (void *itPtr, MediaReader *reader, SDL_Texture *texture, const StdString &texturePath) {
	ImageWindow *it = (ImageWindow *) itPtr;
	Sprite *sprite;
	bool textureprocessed;

	textureprocessed = false;
	if (texture && (! texturePath.empty ())) {
		sprite = new Sprite ();
		sprite->addTexture (texture, texturePath);
		it->setImage (new Image (sprite, 0, true));
		it->isImageDataLoaded = true;
		it->reflow ();
		textureprocessed = true;
	}
	if (! textureprocessed) {
		if (it->mediaReader->lastErrorMessage.empty ()) {
			it->endCreateImageFromVideoFrame (StdString ("Failed to create image texture"));
		}
		else {
			it->endCreateImageFromVideoFrame (it->mediaReader->lastErrorMessage);
		}
	}
	else {
		it->endCreateImageFromVideoFrame ();
	}
	return (textureprocessed);
}
void ImageWindow::endCreateImageFromVideoFrame (const StdString &errorMessage) {
	loadErrorMessage.assign (errorMessage);
	if (! loadErrorMessage.empty ()) {
		isLoadFailed = true;
		showLoadErrorSprite ();
		Log::debug ("Image load from video file failed; path=\"%s\" err=%s", videoFilePath.c_str (), errorMessage.c_str ());
	}
	if (mediaReader) {
		mediaReader->release ();
		mediaReader = NULL;
	}
	if (loadEndCallback.callback) {
		shouldInvokeLoadEndCallback = true;
	}
	isLoadingImageData = false;
	release ();
}

bool ImageWindow::getOnLoadScaleSize (double *destWidth, double *destHeight) {
	double w, h;

	if ((imageLoadSourceWidth <= 0.0f) || (imageLoadSourceHeight <= 0.0f)) {
		return (false);
	}
	switch (onLoadResizeType) {
		case ScaleResize: {
			if ((onLoadWidth <= 0.0f) && (onLoadHeight <= 0.0f)) {
				w = imageLoadSourceWidth;
				h = imageLoadSourceHeight;
			}
			else if (onLoadHeight <= 0.0f) {
				w = onLoadWidth;
				h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
			}
			else if (onLoadWidth <= 0.0f) {
				h = onLoadHeight;
				w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
			}
			else {
				w = onLoadWidth;
				h = onLoadHeight;
			}
			break;
		}
		case FitResize: {
			if ((onLoadWidth <= 0.0f) || (onLoadHeight <= 0.0f)) {
				return (false);
			}
			if (onLoadWidth >= onLoadHeight) {
				w = onLoadWidth;
				h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
				if (h > onLoadHeight) {
					h = onLoadHeight;
					w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
				}
			}
			else {
				h = onLoadHeight;
				w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
				if (w > onLoadWidth) {
					w = onLoadWidth;
					h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
				}
			}
			break;
		}
		default: {
			return (false);
		}
	}

	if (w < 1.0f) {
		w = 1.0f;
	}
	if (h < 1.0f) {
		h = 1.0f;
	}
	if (destWidth) {
		*destWidth = w;
	}
	if (destHeight) {
		*destHeight = h;
	}
	return (true);
}
