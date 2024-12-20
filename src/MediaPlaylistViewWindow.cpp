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
#include "ClassId.h"
#include "UiStack.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "MediaUtil.h"
#include "MediaPlaylist.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "ScrollViewWindow.h"
#include "MediaPlaylistWindow.h"
#include "MediaPlaylistViewWindow.h"

constexpr const double thumbnailHeightScale = 0.3f;
constexpr const double viewBottomPaddingScale = 0.3f;
constexpr const double thumbnailLoadSuspendWidthScale = 1.6f;

MediaPlaylistViewWindow::MediaPlaylistViewWindow (double windowWidth, double windowHeight)
: Panel ()
, clickItemIndex (-1)
, itemWidth (0.0f)
, highlightedThumbnailImageHandle (&highlightedThumbnailImage)
{
	classId = ClassId::MediaPlaylistViewWindow;
	SdlUtil::createMutex (&itemListMutex);

	thumbnailPanel = add (new Panel ());
	thumbnailPanel->setFixedPadding (true, 0.0f, 0.0f);
	thumbnailPanel->isVisible = false;

	scrollWindow = (ScrollViewWindow *) addWidget (new ScrollViewWindow ());
	scrollWindow->setScrollOptions (ScrollViewWindow::MouseWheelScrollOption);

	setFixedPadding (true, 0.0f, 0.0f);
	setWindowSize (windowWidth, windowHeight);
}
MediaPlaylistViewWindow::~MediaPlaylistViewWindow () {
	clearItems ();
	SdlUtil::destroyMutex (&itemListMutex);
}

void MediaPlaylistViewWindow::clearItems () {
	std::vector<MediaPlaylistViewWindowItem *>::iterator i1, i2;
	MediaPlaylistViewWindowItem *item;

	SDL_LockMutex (itemListMutex);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		item = *i1;
		item->itemLabelHandle.destroyAndClear ();
		item->thumbnailImageHandle.destroyAndClear ();
		delete (item);
		++i1;
	}
	itemList.clear ();
	SDL_UnlockMutex (itemListMutex);
}

void MediaPlaylistViewWindow::setWindowSize (double windowWidth, double windowHeight) {
	std::vector<MediaPlaylistViewWindowItem *>::iterator i1, i2;
	MediaPlaylistViewWindowItem *item;
	double ratio;

	setFixedSize (true, windowWidth, windowHeight);
	itemWidth = width - (UiConfiguration::instance->paddingSize * 2.0f);
	thumbnailPanel->setFixedSize (true, width, height * thumbnailHeightScale);
	scrollWindow->setViewBottomPadding (height * viewBottomPaddingScale);

	SDL_LockMutex (itemListMutex);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		item = *i1;
		if (item->isLoaded) {
			if (! item->mediaItem.thumbnailTimestamps.empty ()) {
				ratio = MediaUtil::defaultAspectRatio;
				if ((item->thumbnailWidth > 0) && (item->thumbnailHeight > 0)) {
					ratio = (double) item->thumbnailWidth / (double) item->thumbnailHeight;
				}
				item->thumbnailImage->setWindowSize (true, thumbnailPanel->height * ratio, thumbnailPanel->height);
				item->thumbnailImage->onLoadScale (0.0f, thumbnailPanel->height);
				item->thumbnailImage->reload ();
			}
			else if (item->mediaItem.isAudio && (! item->mediaItem.isVideo) && (! item->mediaItem.hasAudioAlbumArt)) {
				item->thumbnailImage->setWindowSize (true, thumbnailPanel->height, thumbnailPanel->height);
			}
		}
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);

	reflow ();
}

void MediaPlaylistViewWindow::addItem (const MediaPlaylistItem &playlistItem) {
	MediaPlaylistViewWindowItem *item;

	item = new MediaPlaylistViewWindowItem ();
	item->mediaId.assign (playlistItem.mediaId);
	item->startTimestamp = playlistItem.startTimestamp;
	item->thumbnailImageHandle.assign (createThumbnailImage ());
	item->itemLabelHandle.assign (createNameLabel ());
	SDL_LockMutex (itemListMutex);
	itemList.push_back (item);
	SDL_UnlockMutex (itemListMutex);
}

void MediaPlaylistViewWindow::setActiveItem (int itemIndex) {
	std::vector<MediaPlaylistViewWindowItem *>::iterator i1, i2;
	MediaPlaylistViewWindowItem *item;

	SDL_LockMutex (itemListMutex);
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		item = *i1;
		if ((item->listPosition - 1) == itemIndex) {
			item->itemLabel->setActive (true);
		}
		else {
			item->itemLabel->setActive (false);
		}
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);
}

void MediaPlaylistViewWindow::scrollToItem (int itemIndex) {
	std::vector<MediaPlaylistViewWindowItem *>::const_iterator i1, i2;
	MediaPlaylistViewWindowItem *item;
	MediaPlaylistViewWindowItemLabel *label;

	label = NULL;
	SDL_LockMutex (itemListMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		if ((item->listPosition - 1) == itemIndex) {
			label = item->itemLabel;
			label->retain ();
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);
	if (label) {
		scrollWindow->scrollToItem (label, ScrollViewWindow::NearestScrollPosition, UiConfiguration::instance->shortScrollPositionTranslateDuration);
		label->release ();
	}
}

void MediaPlaylistViewWindow::reflow () {
	std::vector<MediaPlaylistViewWindowItem *>::const_iterator i1, i2;
	MediaPlaylistViewWindowItem *item;
	double x, y, h, statusw, namew, startposw;

	statusw = 0.0f;
	namew = 0.0f;
	startposw = 0.0f;
	SDL_LockMutex (itemListMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		if (item->itemLabel->statusLabel->width > statusw) {
			statusw = item->itemLabel->statusLabel->width;
		}
		if (item->itemLabel->startPositionLabel->width > startposw) {
			startposw = item->itemLabel->startPositionLabel->width;
		}
		++i1;
	}
	namew = scrollWindow->scrollViewWidth - statusw - startposw - UiConfiguration::instance->marginSize;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		item->itemLabel->statusLabelWidth = statusw;
		item->itemLabel->startPositionLabelWidth = startposw;
		if (item->isLoaded && (! FLOAT_EQUALS (item->itemLabel->nameLabelWidth, namew))) {
			item->itemLabel->nameLabelWidth = namew;
			item->itemLabel->nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (item->mediaName, namew, Font::dotTruncateSuffix));
		}
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);

	h = height;
	if (thumbnailPanel->isVisible) {
		h -= thumbnailPanel->height;
	}
	scrollWindow->setViewSize (width, h);
	scrollWindow->reflow ();
	x = 0.0f;
	y = 0.0f;
	SDL_LockMutex (itemListMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		item->itemLabel->setWindowWidth (scrollWindow->scrollViewWidth);
		item->itemLabel->reflow ();
		item->itemLabel->position.assign (x, y);
		y += item->itemLabel->height;
		++i1;
	}

	x = 0.0f;
	y = 0.0f;
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		item->thumbnailImage->position.assign (x, y);
		if (item->thumbnailImage->isVisible) {
			x += item->thumbnailImage->width;
		}
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);

	x = 0.0f;
	y = 0.0f;
	if (thumbnailPanel->isVisible) {
		thumbnailPanel->position.assign (x, y);
		y += thumbnailPanel->height;
	}
	scrollWindow->position.assign (x, y);
}

void MediaPlaylistViewWindow::syncRecordStore () {
	std::vector<MediaPlaylistViewWindowItem *>::iterator i1, i2;
	MediaPlaylistViewWindowItem *item;
	int listcount, thumbnailcount;
	double ratio;
	bool shouldreflow;

	shouldreflow = false;
	SDL_LockMutex (itemListMutex);
	listcount = 1;
	thumbnailcount = 0;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		item = *i1;
		if (! item->isLoaded) {
			if (item->mediaItem.readRecordStore (item->mediaId)) {
				item->mediaName.assign (item->mediaItem.name);
				item->thumbnailWidth = item->mediaItem.width;
				item->thumbnailHeight = item->mediaItem.height;
				item->itemLabel->setStartTimestamp (item->startTimestamp, item->mediaItem.duration);
				if (! item->mediaItem.thumbnailTimestamps.empty ()) {
					ratio = MediaUtil::defaultAspectRatio;
					if ((item->thumbnailWidth > 0) && (item->thumbnailHeight > 0)) {
						ratio = (double) item->thumbnailWidth / (double) item->thumbnailHeight;
					}
					item->thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaPlaylistViewWindow::thumbnailImageLongPressed, this);
					item->thumbnailImage->loadEndCallback = Widget::EventCallbackContext (MediaPlaylistViewWindow::thumbnailImageLoaded, this);
					item->thumbnailImage->setWindowSize (true, thumbnailPanel->height * ratio, thumbnailPanel->height);
					item->thumbnailImage->onLoadScale (0.0f, thumbnailPanel->height);
					item->thumbnailImage->setDrawAlpha (-1.0f);
					item->thumbnailImage->loadSeekTimestampVideoFrame (item->mediaItem.mediaPath, item->startTimestamp, true);
					item->thumbnailImage->isVisible = true;
				}
				else if (item->mediaItem.isAudio && (! item->mediaItem.isVideo) && (! item->mediaItem.hasAudioAlbumArt)) {
					item->thumbnailImage->setWindowSize (true, thumbnailPanel->height, thumbnailPanel->height);
					item->thumbnailImage->setImage (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_audioIcon)));
					item->thumbnailImage->setDrawColor (true, UiConfiguration::instance->darkPrimaryColor);
					item->thumbnailImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
					item->thumbnailImage->isVisible = true;
				}
				else {
					item->thumbnailImage->isVisible = false;
				}
				item->isLoaded = true;
				shouldreflow = true;
			}
		}
		if (item->isLoaded) {
			if (! item->mediaItem.thumbnailTimestamps.empty ()) {
				++thumbnailcount;
			}
		}

		item->setListPosition (listcount);
		++listcount;
		++i1;
	}
	SDL_UnlockMutex (itemListMutex);

	if ((! thumbnailPanel->isVisible) && (thumbnailcount > 0)) {
		thumbnailPanel->isVisible = true;
		shouldreflow = true;
	}
	else if (thumbnailPanel->isVisible && (thumbnailcount <= 0)) {
		thumbnailPanel->isVisible = false;
		shouldreflow = true;
	}

	if (shouldreflow) {
		reflow ();
	}
}

void MediaPlaylistViewWindow::doUpdate (int msElapsed) {
	std::vector<MediaPlaylistViewWindowItem *>::const_iterator i1, i2;
	MediaPlaylistViewWindowItem *item;
	bool found, highlighted;
	double x, dx;

	Panel::doUpdate (msElapsed);
	highlightedThumbnailImageHandle.compact ();
	found = false;
	highlighted = false;
	x = 0.0f;
	SDL_LockMutex (itemListMutex);
	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		item = *i1;
		if (item->itemLabel->isHighlighted) {
			found = true;
			highlighted = true;
			x = item->thumbnailImage->position.x - (thumbnailPanel->width / 2.0f) + (item->thumbnailImage->width / 2.0f);
			if (highlightedThumbnailImage != item->thumbnailImage) {
				if (highlightedThumbnailImage) {
					highlightedThumbnailImage->setBorder (false);
				}
				highlightedThumbnailImageHandle.assign (item->thumbnailImage);
				highlightedThumbnailImage->setBorder (true, UiConfiguration::instance->lightPrimaryColor.copy (UiConfiguration::instance->selectionBorderAlpha), UiConfiguration::instance->selectionBorderWidth);
				highlightedThumbnailImage->borderColor.animate (UiConfiguration::instance->lightPrimaryColor.copy (UiConfiguration::instance->selectionBorderAlpha), UiConfiguration::instance->lightSecondaryColor, UiConfiguration::instance->longColorAnimateDuration);
			}
			break;
		}
		++i1;
	}
	if (! found) {
		i1 = itemList.cbegin ();
		i2 = itemList.cend ();
		while (i1 != i2) {
			item = *i1;
			if (item->itemLabel->isActive) {
				found = true;
				x = item->thumbnailImage->position.x - (thumbnailPanel->width / 2.0f) + (item->thumbnailImage->width / 2.0f);
				break;
			}
			++i1;
		}
	}
	if (! found) {
		i1 = itemList.cbegin ();
		i2 = itemList.cend ();
		while (i1 != i2) {
			item = *i1;
			if ((item->itemLabel->position.y + item->itemLabel->height) >= scrollWindow->scrollViewOriginY) {
				x = item->thumbnailImage->position.x;
				break;
			}
			++i1;
		}
	}
	if (isVisible) {
		i1 = itemList.cbegin ();
		i2 = itemList.cend ();
		while (i1 != i2) {
			item = *i1;
			dx = item->thumbnailImage->position.x + (item->thumbnailImage->width / 2.0f) - x - (width / 2.0f);
			if (fabs (dx) >= (width * thumbnailLoadSuspendWidthScale)) {
				item->thumbnailImage->isLoadSuspended = true;
			}
			else {
				item->thumbnailImage->isLoadSuspended = false;
			}
			++i1;
		}
	}
	SDL_UnlockMutex (itemListMutex);

	if (highlightedThumbnailImage && (! highlighted)) {
		highlightedThumbnailImage->setBorder (false);
		highlightedThumbnailImageHandle.clear ();
	}
	thumbnailPanelViewOrigin.translate (x, 0.0f, UiConfiguration::instance->shortScrollPositionTranslateDuration);
	thumbnailPanelViewOrigin.update (msElapsed);
	thumbnailPanel->setViewOrigin (thumbnailPanelViewOrigin.x, 0.0f);
}

ImageWindow *MediaPlaylistViewWindow::createThumbnailImage () {
	ImageWindow *image;

	image = thumbnailPanel->add (new ImageWindow ());
	image->isLoadSuspended = true;
	image->isVisible = false;
	image->setWindowSize (true, thumbnailPanel->height * MediaUtil::defaultAspectRatio, thumbnailPanel->height);
	image->reflow ();
	return (image);
}

MediaPlaylistViewWindowItemLabel *MediaPlaylistViewWindow::createNameLabel () {
	MediaPlaylistViewWindowItemLabel *label;

	label = (MediaPlaylistViewWindowItemLabel *) scrollWindow->addViewItem (new MediaPlaylistViewWindowItemLabel (scrollWindow->width));
	label->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistViewWindow::itemNameLabelClicked, this);
	label->mouseEnterCallback = Widget::EventCallbackContext (MediaPlaylistViewWindow::itemNameLabelMouseEntered, this);
	label->mouseExitCallback = Widget::EventCallbackContext (MediaPlaylistViewWindow::itemNameLabelMouseExited, this);
	return (label);
}

void MediaPlaylistViewWindow::thumbnailImageLongPressed (void *itPtr, Widget *widgetPtr) {
	UiStack::instance->showImageDialog ((ImageWindow *) widgetPtr);
}

void MediaPlaylistViewWindow::thumbnailImageLoaded (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistViewWindow *) itPtr)->reflow ();
}

void MediaPlaylistViewWindow::itemNameLabelClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistViewWindow *it = (MediaPlaylistViewWindow *) itPtr;
	MediaPlaylistViewWindowItemLabel *label = (MediaPlaylistViewWindowItemLabel *) widgetPtr;

	it->clickItemIndex = label->itemIndex;
	it->eventCallback (it->itemClickCallback);
}

void MediaPlaylistViewWindow::itemNameLabelMouseEntered (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistViewWindowItemLabel *) widgetPtr)->setHighlighted (true);
}

void MediaPlaylistViewWindow::itemNameLabelMouseExited (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistViewWindowItemLabel *) widgetPtr)->setHighlighted (false);
}

MediaPlaylistViewWindowItem::MediaPlaylistViewWindowItem ()
: listPosition (0)
, isLoaded (false)
, startTimestamp (0)
, thumbnailWidth (0)
, thumbnailHeight (0)
, itemLabelHandle (&itemLabel)
, thumbnailImageHandle (&thumbnailImage)
{
}
MediaPlaylistViewWindowItem::~MediaPlaylistViewWindowItem ()
{
}

void MediaPlaylistViewWindowItem::setListPosition (int position) {
	if (listPosition == position) {
		return;
	}
	listPosition = position;
	itemLabel->itemIndex = position - 1;
	itemLabel->statusLabel->setText (StdString::createSprintf ("#%i", listPosition));
	itemLabel->resetColors ();
}

MediaPlaylistViewWindowItemLabel::MediaPlaylistViewWindowItemLabel (double windowWidth)
: Panel ()
, statusLabelWidth (0.0f)
, nameLabelWidth (0.0f)
, startPositionLabelWidth (0.0f)
, isErrorStatus (false)
, windowWidth (windowWidth)
, isActive (false)
, isHighlighted (false)
{
	statusLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)));
	statusLabel->isInputSuspended = true;
	statusLabel->setPaddingScale (1.0f, 0.5f);

	nameLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)));
	nameLabel->isInputSuspended = true;
	nameLabel->setPaddingScale (1.0f, 0.5f);

	startPositionLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)));
	startPositionLabel->isInputSuspended = true;
	startPositionLabel->setPaddingScale (1.0f, 0.5f);

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
MediaPlaylistViewWindowItemLabel::~MediaPlaylistViewWindowItemLabel () {
}

void MediaPlaylistViewWindowItemLabel::setWindowWidth (double widthValue) {
	if (FLOAT_EQUALS (windowWidth, widthValue)) {
		return;
	}
	windowWidth = widthValue;
	reflow ();
}

void MediaPlaylistViewWindowItemLabel::setStartTimestamp (int64_t startTimestamp, int64_t streamDuration) {
	StdString text;

	text = UiText::instance->getTimespanText (startTimestamp, UiText::HoursUnit, true);
	if (streamDuration > 0) {
		text.appendSprintf ("/%s", UiText::instance->getTimespanText (streamDuration, UiText::HoursUnit, true).c_str ());
	}
	startPositionLabel->setText (text);
	reflow ();
}

void MediaPlaylistViewWindowItemLabel::setActive (bool active) {
	if (isActive == active) {
		return;
	}
	isActive = active;
	resetColors ();
}

void MediaPlaylistViewWindowItemLabel::setHighlighted (bool highlighted) {
	if (isHighlighted == highlighted) {
		return;
	}
	isHighlighted = highlighted;
	resetColors ();
}

void MediaPlaylistViewWindowItemLabel::resetColors () {
	if (isActive) {
		setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
		statusLabel->translateTextColor (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->translateTextColor (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
		startPositionLabel->translateTextColor (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else if (isHighlighted) {
		setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
		statusLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		startPositionLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else if (isErrorStatus) {
		setFillBg (false);
		statusLabel->translateTextColor (UiConfiguration::instance->errorTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->translateTextColor (UiConfiguration::instance->lightPrimaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		startPositionLabel->translateTextColor (UiConfiguration::instance->lightPrimaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		setFillBg (false);
		statusLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
		startPositionLabel->translateTextColor (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
}

void MediaPlaylistViewWindowItemLabel::reflow () {
	double h;

	statusLabel->position.assign (0.0f, 0.0f);
	h = statusLabel->height;

	nameLabel->position.assign (statusLabelWidth, 0.0f);
	if (nameLabel->height > h) {
		h = nameLabel->height;
	}

	startPositionLabel->position.assign (width - startPositionLabelWidth, 0.0f);
	if (startPositionLabel->height > h) {
		h = startPositionLabel->height;
	}
	setFixedSize (true, windowWidth, h);

	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = h;
	statusLabel->centerVertical (&layoutFlow);
	nameLabel->centerVertical (&layoutFlow);
	startPositionLabel->centerVertical (&layoutFlow);
}
