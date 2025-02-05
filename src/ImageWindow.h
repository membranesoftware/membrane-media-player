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
// Panel that holds an Image widget and provides operations for loading image content from outside resources
#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#include "Color.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Image.h"
#include "ProgressRing.h"
#include "Panel.h"

class SharedBuffer;
class Sprite;
class MediaReader;

class ImageWindow : public Panel {
public:
	ImageWindow (Image *imageWidget = NULL);
	~ImageWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static ImageWindow *castWidget (Widget *widget);

	// imageLoadType values
	static constexpr const int NoLoadType = 0;
	static constexpr const int ImageFileLoadType = 1;
	static constexpr const int UrlLoadType = 2;
	static constexpr const int VideoFrameLoadType = 3;

	// Read-write data members
	Widget::EventCallbackContext loadStartCallback;
	Widget::EventCallbackContext loadEndCallback;
	bool isLoadingSpriteDisabled;
	bool isOffscreenUnloadEnabled;
	bool isLoadSuspended;
	Sprite *loadingSprite;
	Sprite *loadErrorSprite;
	StdString loadErrorMessage;
	Color loadingSpriteColor;

	// Read-only data members
	int imageLoadType;
	StdString imageUrl;
	StdString imageFilePath;
	bool isImageFileExternal;
	StdString videoFilePath;
	bool isVideoFileExternal;
	int64_t videoFrameTimestamp;
	double imageLoadSourceWidth, imageLoadSourceHeight;
	bool isLoadingImageData;
	bool isLoadFailed;

	// Set window's fixed size option. If window size is enabled and the display image does not fill the provided size, it is centered inside a larger background space.
	void setWindowSize (bool enable, double windowSizeWidth = 0.0f, double windowSizeHeight = 0.0f);

	// Clear the window's content and replace it with the provided image
	void setImage (Image *nextImage);

	// Set window size values to apply while image loading is in progress
	void setLoadingSize (double loadingWidthValue = 0.0f, double loadingHeightValue = 0.0f);

	// Set a source URL that should be used to load the image window's content
	void loadImageUrl (const StdString &loadUrl);

	// Set a path that should be used to load the image window's content from file data. If isExternalPath is true, read data from a filesystem path instead of application resources.
	void loadImageFile (const StdString &filePath, bool isExternalPath = false);

	// Set a path that should be used to load the image window's content from video frame data in a media file. If isExternalPath is true, read data from a filesystem path instead of application resources.
	void loadSeekPercentVideoFrame (const StdString &filePath, double seekPercent = 0.0f, bool isExternalPath = false);
	void loadSeekTimestampVideoFrame (const StdString &filePath, int64_t seekTimestamp = 0, bool isExternalPath = false);

	// Set the image window to load content from the same source loaded by another image window
	void loadSourceCopy (ImageWindow *sourceImage);

	// Return a boolean value indicating if the image window's load URL is empty
	bool isImageUrlEmpty ();

	// Return a boolean value indicating if the image window is loaded with content
	bool isLoaded ();

	// Reload image content from the window's source URL
	void reload ();

	// Set the sprite frame for use in drawing the image
	void setFrame (int frame);

	// Set the draw scale factor for the window image
	void setScale (double scale);

	// Set the window image's draw color option. If enabled, the image renders its texture with a filter of the specified color.
	void setDrawColor (bool enable, const Color &color = Color ());

	// Set the window image's draw alpha. If alphaValue is less than zero, disable alpha blending.
	void setDrawAlpha (double alphaValue);

	// Set a scale size to apply after image content loads. If scaleWidth or scaleHeight are zero or less, choose a value that preserves the source aspect ratio.
	void onLoadScale (double scaleWidth = 0.0f, double scaleHeight = 0.0f);

	// Set a fit size to apply after image content loads, with the final image size computed as the largest width and height values that preserve the source aspect ratio while fitting inside targetWidth and targetHeight
	void onLoadFit (double targetWidth, double targetHeight);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Set the image content to the loading sprite
	void showLoadingSprite ();

	// Set the image content to the load error sprite
	void showLoadErrorSprite ();

	// Assign destWidth and destHeight to target size values for configured onLoad settings and return a boolean value indicating if the operation succeeded
	bool getOnLoadScaleSize (double *destWidth, double *destHeight);

	// Return a boolean value indicating if the image window is configured with isOffscreenUnloadEnabled and holds state indicating that it should unload content
	bool isOffscreen ();

	// Execute operations to load content using the value stored in imageFilePath
	static void createImageFromImageFile (void *itPtr);
	void executeCreateImageFromImageFile ();

	// Execute operations appropriate after an imageFilePath load completes. If errorMessage is non-empty, disable subsequent load attempts of that path.
	void endCreateImageFromImageFile (const StdString &errorMessage = StdString ());

	// Execute operations to load content using the value stored in imageUrl
	static void createImageFromUrlHttpGetComplete (void *itPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void createImageFromUrlResponseData (void *itPtr);
	void executeCreateImageFromUrlResponseData ();

	// Execute operations appropriate after an imageUrl load completes. If errorMessage is non-empty, disable subsequent load attempts of that URL.
	void endCreateImageFromUrl (const StdString &errorMessage = StdString ());

	// Execute operations to load content from a video frame
	static void readVideoFrame (void *itPtr);
	void executeReadVideoFrame ();
	static bool createVideoFrameTextureComplete (void *itPtr, MediaReader *reader, SDL_Texture *texture, const StdString &texturePath);

	// Execute operations appropriate after a videoFilePath load completes. If errorMessage is non-empty, disable subsequent load attempts of that path.
	void endCreateImageFromVideoFrame (const StdString &errorMessage = StdString ());

	Image *image;
	WidgetHandle<Image> imageHandle;
	ProgressRing *loadingProgressRing;
	WidgetHandle<ProgressRing> loadingProgressRingHandle;
	double drawAlpha;
	bool isWindowSizeEnabled;
	double windowWidth;
	double windowHeight;
	double loadingWidth;
	double loadingHeight;
	bool isImageDataLoaded;
	bool shouldInvokeLoadEndCallback;
	SharedBuffer *imageUrlData;
	MediaReader *mediaReader;
	double videoFrameSeekPercent;
	int64_t videoFrameSeekTimestamp;
	int onLoadResizeType;
	double onLoadWidth;
	double onLoadHeight;
	int nextImageLoadType;
	StdString nextImageUrl;
	StdString nextImageFilePath;
	bool nextIsImageFileExternal;
	StdString nextVideoFilePath;
	bool nextIsVideoFileExternal;
	int64_t nextVideoFrameSeekTimestamp;
	double nextVideoFrameSeekPercent;
};
#endif
