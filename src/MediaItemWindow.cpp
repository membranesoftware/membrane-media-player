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
#include "Ui.h"
#include "UiStack.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "MediaUtil.h"
#include "MediaControl.h"
#include "Json.h"
#include "Sprite.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Button.h"
#include "TextFlow.h"
#include "MediaItemWindow.h"

MediaItemWindow::MediaItemWindow (Json *mediaItemRecord)
: Panel ()
, isPlayable (false)
, isSelected (false)
, playTimestamp (0)
, mediaIconImageHandle (&mediaIconImage)
{
	classId = ClassId::MediaItemWindow;

	setFillBg (true, Color (0.5f, 0.5f, 0.5f));
	if (! mediaItem.readRecord (mediaItemRecord)) {
		mediaId.assign ("");
	}
	else {
		mediaId.assign (mediaItem.mediaId);
	}

	mediaImage = add (new ImageWindow ());
	mediaImage->isOffscreenUnloadEnabled = true;
	mediaImage->widgetName.assign (mediaItem.name);
	mediaImage->widgetName.append ("ThumbnailImage");
	mediaImage->loadEndCallback = Widget::EventCallbackContext (MediaItemWindow::mediaImageLoaded, this);
	mediaImage->mouseClickCallback = Widget::EventCallbackContext (MediaItemWindow::mediaImageClicked, this);
	mediaImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaItemWindow::mediaImageLongPressed, this);
	mediaImage->setDrawAlpha (-1.0f);

	descriptionLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)), 4);
	descriptionLabel->isInputSuspended = true;
	descriptionLabel->isVisible = false;

	viewButton = add (Ui::createScrimIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_openButton), Widget::EventCallbackContext (MediaItemWindow::viewButtonClicked, this), UiText::instance->getText (UiTextId::ViewMediaDetails).capitalized ()), 5);
	viewButton->isVisible = false;

	timestampLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)), 2);
	timestampLabel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	timestampLabel->setPaddingScale (1.0f, 0.5f);
	timestampLabel->isInputSuspended = true;
	timestampLabel->isVisible = false;
}
MediaItemWindow::~MediaItemWindow () {
}

MediaItemWindow *MediaItemWindow::castWidget (Widget *widget) {
	if (Widget::isWidgetClass (widget, ClassId::MediaItemImageWindow)) {
		return ((MediaItemWindow *) widget);
	}
	if (Widget::isWidgetClass (widget, ClassId::MediaItemDetailWindow)) {
		return ((MediaItemWindow *) widget);
	}
	return (NULL);
}

void MediaItemWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	if (selected == isSelected) {
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

void MediaItemWindow::setPlayTimestamp (int64_t timestamp) {
	if (timestamp < 0) {
		timestamp = 0;
	}
	if (timestamp > mediaItem.duration) {
		timestamp = mediaItem.duration;
	}
	if (playTimestamp == timestamp) {
		return;
	}
	playTimestamp = timestamp;
	mediaImage->loadSeekTimestampVideoFrame (mediaItem.mediaPath, playTimestamp, true);
}

bool MediaItemWindow::hasThumbnails () {
	if (mediaItem.thumbnailTimestamps.empty () || (mediaItem.width <= 0) || (mediaItem.height <= 0)) {
		return (false);
	}
	return (true);
}

void MediaItemWindow::setMediaIconImage (Sprite *iconSprite) {
	mediaIconImageHandle.destroyAndAssign (new ImageWindow (new Image (iconSprite)));
	mediaIconImage->isInputSuspended = true;
	mediaIconImage->setDrawColor (true, UiConfiguration::instance->darkPrimaryColor);
	mediaIconImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	mediaIconImage->setPaddingScale (0.5f, 0.0f);
	add (mediaIconImage, 3);
}

void MediaItemWindow::syncRecordStore () {
	if (! mediaItem.readRecordStore (mediaId)) {
		return;
	}
	if (mediaItem.isVideo) {
		syncVideoItem ();
	}
	else if (mediaItem.isAudio) {
		syncAudioItem ();
	}
	else {
		isPlayable = false;
		setMediaIconImage (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon));
	}
	shouldComposeRender = true;
	reflow ();
}

void MediaItemWindow::syncAudioItem () {
	if (hasThumbnails ()) {
		mediaImage->loadImageFile (MediaControl::instance->getThumbnailPath (mediaId, 0), true);
		mediaIconImageHandle.destroyAndClear ();
	}
	else {
		setMediaIconImage (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_audioIcon));
	}
	isPlayable = true;
}

void MediaItemWindow::syncVideoItem () {
	int64_t pos;

	if (mediaItem.thumbnailTimestamps.empty ()) {
		setMediaIconImage (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon));
	}
	else {
		if (playTimestamp <= 0) {
			pos = mediaItem.thumbnailTimestamps.at (mediaItem.thumbnailTimestamps.size () / 4);
			mediaImage->loadImageFile (MediaControl::instance->getThumbnailPath (mediaId, pos), true);
		}
		else {
			mediaImage->loadSeekTimestampVideoFrame (mediaItem.mediaPath, playTimestamp, true);
		}
		mediaIconImageHandle.destroyAndClear ();
	}
	isPlayable = true;
}

void MediaItemWindow::refreshDetailSize () {
	double w, h;

	w = floor (detailMaxWidth * getDetailThumbnailScale ());
	if ((mediaItem.width > 0) && (mediaItem.height > 0)) {
		h = floor (w * (double) mediaItem.height / (double) mediaItem.width);
	}
	else {
		h = floor (w / MediaUtil::defaultAspectRatio);
	}
	mediaImage->setLoadingSize (w, h);
	mediaImage->setWindowSize (true, w, h);
	mediaImage->onLoadScale (w, h);
	mediaImage->reload ();
	reflow ();
}

void MediaItemWindow::mediaImageLoaded (void *itPtr, Widget *widgetPtr) {
	((MediaItemWindow *) itPtr)->shouldComposeRender = true;
}

void MediaItemWindow::mediaImageClicked (void *itPtr, Widget *widgetPtr) {
	((MediaItemWindow *) itPtr)->eventCallback (((MediaItemWindow *) itPtr)->mediaImageClickCallback);
}

void MediaItemWindow::mediaImageLongPressed (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->showImageDialog ((ImageWindow *) widgetPtr);
}

void MediaItemWindow::viewButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaItemWindow *) itPtr)->eventCallback (((MediaItemWindow *) itPtr)->viewButtonClickCallback);
}
