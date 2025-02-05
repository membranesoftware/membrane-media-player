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
#include "ClassId.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "MediaUtil.h"
#include "Ui.h"
#include "UiStack.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Sprite.h"
#include "Color.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Button.h"
#include "MediaThumbnailWindow.h"

// imageSourceType values
static const constexpr int FileThumbnailImageSource = 0;
static const constexpr int UrlThumbnailImageSource = 1;
static const constexpr int VideoFrameThumbnailImageSource = 2;
static const constexpr int SpriteThumbnailImageSource = 3;

MediaThumbnailWindow::MediaThumbnailWindow (int sourceWidth, int sourceHeight)
: Panel ()
, listPosition (0)
, itemIndex (-1)
, sourceWidth (sourceWidth)
, sourceHeight (sourceHeight)
, imageSourceType (-1)
, imageSprite (NULL)
, isImageLoaded (false)
, thumbnailTimestamp (-1)
, isSelected (false)
, isMarkerControlEnabled (false)
{
	classId = ClassId::MediaThumbnailWindow;
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	thumbnailImage = add (new ImageWindow ());

	deleteButton = add (Ui::createScrimIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton), Widget::EventCallbackContext (MediaThumbnailWindow::deleteButtonClicked, this), UiText::instance->getText (UiTextId::RemoveTimeMarker).capitalized ()), 1);
	deleteButton->isVisible = false;

	reflow ();
}
MediaThumbnailWindow::~MediaThumbnailWindow () {
}

MediaThumbnailWindow *MediaThumbnailWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::MediaThumbnailWindow) ? (MediaThumbnailWindow *) widget : NULL);
}

void MediaThumbnailWindow::setMarkerControl (bool enable) {
	if (isMarkerControlEnabled == enable) {
		return;
	}
	isMarkerControlEnabled = enable;
	reflow ();
}

void MediaThumbnailWindow::setSourceImageUrl (const StdString &url, int64_t timestamp) {
	imageSourceType = UrlThumbnailImageSource;
	thumbnailTimestamp = timestamp;
	sourcePath.assign (url);
	thumbnailImage->isOffscreenUnloadEnabled = true;
	thumbnailImage->loadEndCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLoaded, this);
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLongPressed, this);
	thumbnailImage->setDrawAlpha (-1.0f);
	thumbnailImage->setDrawColor (false);
	thumbnailImage->loadImageUrl (sourcePath);
}

void MediaThumbnailWindow::setSourceImageFile (const StdString &path, int64_t timestamp) {
	imageSourceType = FileThumbnailImageSource;
	thumbnailTimestamp = timestamp;
	sourcePath.assign (path);
	thumbnailImage->isOffscreenUnloadEnabled = true;
	thumbnailImage->loadEndCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLoaded, this);
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLongPressed, this);
	thumbnailImage->setDrawAlpha (-1.0f);
	thumbnailImage->setDrawColor (false);
	thumbnailImage->loadImageFile (sourcePath, true);
}

void MediaThumbnailWindow::setSourceVideoFrame (const StdString &path, int64_t timestamp) {
	imageSourceType = VideoFrameThumbnailImageSource;
	thumbnailTimestamp = timestamp;
	sourcePath.assign (path);
	thumbnailImage->isOffscreenUnloadEnabled = true;
	thumbnailImage->loadEndCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLoaded, this);
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaThumbnailWindow::thumbnailImageLongPressed, this);
	thumbnailImage->setDrawAlpha (-1.0f);
	thumbnailImage->setDrawColor (false);
	thumbnailImage->loadSeekTimestampVideoFrame (sourcePath, timestamp, true);
}

void MediaThumbnailWindow::setSourceSprite (const StdString &path, Sprite *sprite, const Color &drawColor) {
	imageSourceType = SpriteThumbnailImageSource;
	imageSprite = sprite;
	imageSpriteDrawColor.assign (drawColor);
	sourcePath.assign (path);
	thumbnailImage->isOffscreenUnloadEnabled = false;
	thumbnailImage->loadEndCallback = Widget::EventCallbackContext ();
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext ();
	thumbnailImage->setImage (new Image (imageSprite));
	thumbnailImage->setDrawAlpha (1.0f);
	thumbnailImage->setDrawColor (true, imageSpriteDrawColor);
}

void MediaThumbnailWindow::setSourceCopy (MediaThumbnailWindow *sourceThumbnail) {
	switch (sourceThumbnail->imageSourceType) {
		case UrlThumbnailImageSource: {
			setSourceImageUrl (sourceThumbnail->sourcePath, sourceThumbnail->thumbnailTimestamp);
			break;
		}
		case FileThumbnailImageSource: {
			setSourceImageFile (sourceThumbnail->sourcePath, sourceThumbnail->thumbnailTimestamp);
			break;
		}
		case VideoFrameThumbnailImageSource: {
			setSourceVideoFrame (sourceThumbnail->sourcePath, sourceThumbnail->thumbnailTimestamp);
			break;
		}
		case SpriteThumbnailImageSource: {
			setSourceSprite (sourceThumbnail->sourcePath, sourceThumbnail->imageSprite, sourceThumbnail->imageSpriteDrawColor);
			break;
		}
	}
}

void MediaThumbnailWindow::refreshDetailSize () {
	double w, h;

	w = floor (detailMaxWidth * getDetailThumbnailScale ());
	if ((sourceWidth > 0) && (sourceHeight > 0)) {
		h = floor (w * (double) sourceHeight / (double) sourceWidth);
	}
	else {
		h = floor (w / MediaUtil::defaultAspectRatio);
	}
	thumbnailImage->setWindowSize (true, w, h);
	if (imageSourceType != SpriteThumbnailImageSource) {
		thumbnailImage->setLoadingSize (w, h);
		thumbnailImage->onLoadScale (w, h);
		thumbnailImage->reload ();
	}
	reflow ();
}

void MediaThumbnailWindow::reflow () {
	double x, y;

	x = 0.0f;
	y = 0.0f;
	thumbnailImage->position.assign (x, y);
	if (deleteButton->isVisible) {
		deleteButton->position.assign (thumbnailImage->width - deleteButton->width, 0.0f);
	}
	resetSize ();
}

void MediaThumbnailWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	if (isSelected == selected) {
		return;
	}
	isSelected = selected;
	if (isSelected) {
		setBorder (true, UiConfiguration::instance->mediumSecondaryColor.copy (UiConfiguration::instance->selectionBorderAlpha), UiConfiguration::instance->selectionBorderWidth);
	}
	else {
		setBorder (false);
	}
	reflow ();
	if (! shouldSkipStateChangeCallback) {
		eventCallback (selectStateChangeCallback);
	}
	shouldComposeRender = true;
}

void MediaThumbnailWindow::doProcessComposeDrawChange () {
	if (isComposeDrawEnabled) {
		deleteButton->isVisible = false;
	}
	else {
		deleteButton->isVisible = isMarkerControlEnabled;
	}
	reflow ();
}

void MediaThumbnailWindow::thumbnailImageLoaded (void *itPtr, Widget *widgetPtr) {
	MediaThumbnailWindow *it = (MediaThumbnailWindow *) itPtr;
	ImageWindow *image = (ImageWindow *) widgetPtr;

	it->isImageLoaded = image->isLoaded ();
	if (it->isImageLoaded) {
		if (it->imageSourceType == VideoFrameThumbnailImageSource) {
			it->thumbnailTimestamp = image->videoFrameTimestamp;
		}
		it->shouldComposeRender = true;
	}
	it->eventCallback (it->imageLoadCallback);
}

void MediaThumbnailWindow::thumbnailImageLongPressed (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->showImageDialog ((ImageWindow *) widgetPtr);
}

void MediaThumbnailWindow::deleteButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaThumbnailWindow *) itPtr)->eventCallback (((MediaThumbnailWindow *) itPtr)->deleteClickCallback);
}
