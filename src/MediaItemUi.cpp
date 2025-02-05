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
#include "UiConfiguration.h"
#include "UiText.h"
#include "UiStack.h"
#include "AppUrl.h"
#include "MediaUtil.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "OsUtil.h"
#include "Json.h"
#include "RecordStore.h"
#include "MediaControl.h"
#include "Database.h"
#include "PlayMarker.h"
#include "Font.h"
#include "Image.h"
#include "Button.h"
#include "Toolbar.h"
#include "Chip.h"
#include "ImageWindow.h"
#include "TextFieldWindow.h"
#include "PlayerTimelineWindow.h"
#include "CardView.h"
#include "CardLabelWindow.h"
#include "HelpWindow.h"
#include "TextCardWindow.h"
#include "MediaItem.h"
#include "MediaItemWindow.h"
#include "MediaItemTagWindow.h"
#include "MediaThumbnailWindow.h"
#include "ActionWindow.h"
#include "IconLabelWindow.h"
#include "MediaItemUi.h"

constexpr const int InfoRow = 0;
constexpr const int TagWindowRow = 1;
constexpr const int MarkerRow = 2;
constexpr const int FrameThumbnailRow = 3;
constexpr const int RowCount = 4;

constexpr const double timelineWidthScale = 0.5f;
constexpr const double nameTextWidthScale = 0.7f;

MediaItemUi::MediaItemUi (MediaItemWindow *mediaItemWindow)
: Ui ()
, playTimestamp (-1)
, selectPlayPositionTimestamp (-1)
, mediaId (mediaItemWindow->mediaId)
, tagWindowHandle (&tagWindow)
, timelineWindowHandle (&timelineWindow)
, selectedThumbnailHandle (&selectedThumbnail)
, timelinePopupImageWindowHandle (&timelinePopupImageWindow)
, timelineHoverPosition (-1.0f)
, timelineHoverTimestamp (-1)
, timelineHoverClock (-1)
, shouldLoadMarkers (false)
, isTagWindowExpanded (false)
, shouldEnableTagWindow (false)
{
	SdlUtil::createMutex (&loadThumbnailsMutex);
	mediaItem.copyValues (mediaItemWindow->mediaItem);
}
MediaItemUi::~MediaItemUi () {
	SdlUtil::destroyMutex (&loadThumbnailsMutex);
}

StdString MediaItemUi::getSpritePath () {
	return (StdString ("ui/MediaItemUi/sprite"));
}

Widget *MediaItemUi::createBreadcrumbWidget () {
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (mediaItem.name, App::instance->drawableWidth * 0.5f, Font::dotTruncateSuffix)));
}

void MediaItemUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextId::MediaItemUiHelpTitle), UiText::instance->getText (UiTextId::MediaItemUiHelpText));
	if (! mediaItem.isVideo) {
		helpWindow->addAction (UiText::instance->getText (UiTextId::MediaItemUiHelpAction1Text));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextId::MediaItemUiHelpAction2Text));
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextId::MediaInspectorInterface).capitalized (), StdString (AppUrl::MediaInspectorInterface));
}

OpResult MediaItemUi::doLoad () {
	StdString text;
	HashMap *prefs;
	int imagesize;

	prefs = App::instance->lockPrefs ();
	imagesize = prefs->find (MediaItemUi::imageSizeKey, (int) Ui::MediumSize);
	App::instance->unlockPrefs ();

	setDetailImageSize (imagesize);
	cardView->setRowCount (RowCount);
	cardView->setRowItemMarginSize (FrameThumbnailRow, 0.0f);
	cardView->setRowSelectionAnimated (FrameThumbnailRow, true);
	cardView->setRowRepositionAnimated (FrameThumbnailRow, true);
	cardView->setRowLabeled (FrameThumbnailRow, true, MediaItemUi::thumbnailCardViewItemLabel, this);

	if ((! mediaItem.isVideo) && mediaItem.isAudio && mediaItem.hasAudioAlbumArt) {
		text = UiText::instance->getText (UiTextId::AlbumArt).capitalized ();
	}
	else {
		text = UiText::instance->getText (UiTextId::Frames).capitalized ();
	}
	cardView->setRowHeader (FrameThumbnailRow, createRowHeaderPanel (text));

	cardView->setRowSelectionAnimated (MarkerRow, true);
	cardView->setRowRepositionAnimated (MarkerRow, true);
	cardView->setRowLabeled (MarkerRow, true, MediaItemUi::thumbnailCardViewItemLabel, this);
	cardView->setRowHeader (MarkerRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::TimeMarkers).capitalized ()));

	cardView->reflow ();
	return (OpResult::Success);
}

void MediaItemUi::doUnload () {
	std::list<MediaThumbnailWindow *>::iterator i1, i2;

	eventCallback (endCallback);

	SDL_LockMutex (loadThumbnailsMutex);
	i1 = loadThumbnails.begin ();
	i2 = loadThumbnails.end ();
	while (i1 != i2) {
		(*i1)->release ();
		++i1;
	}
	loadThumbnails.clear ();
	SDL_UnlockMutex (loadThumbnailsMutex);

	tagWindowHandle.clear ();
	timelineWindowHandle.clear ();
	selectedThumbnailHandle.clear ();
	timelinePopupImageWindowHandle.clear ();
}

void MediaItemUi::doAddMainToolbarItems (Toolbar *toolbar) {
	toolbar->addRightItem (createImageSizeButton ());
}

void MediaItemUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	ImageWindow *image;

	timelineWindowHandle.destroyAndClear ();
	if (mediaItem.isVideo) {
		image = new ImageWindow (new Image (sprites.getSprite (SpriteId::MediaItemUi_timeIcon)));
		image->setFixedPadding (true, 0.0f, 0.0f);
		image->setDrawColor (true, UiConfiguration::instance->lightSecondaryColor);
		toolbar->setLeftCorner (image);

		timelineWindowHandle.assign (new PlayerTimelineWindow (App::instance->drawableWidth * timelineWidthScale));
		timelineWindow->positionHoverCallback = Widget::EventCallbackContext (MediaItemUi::timelinePositionHovered, this);
		timelineWindow->positionClickCallback = Widget::EventCallbackContext (MediaItemUi::timelinePositionClicked, this);
		toolbar->addLeftItem (timelineWindow);

		toolbar->addLeftItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaItemUi_viewBeforeButton), MediaItemUi::viewBeforeButtonClicked, MediaItemUi::viewBeforeButtonFocused, UiText::instance->getText (UiTextId::MediaItemUiViewBeforeTooltip), "viewBeforeButton"));
		toolbar->addLeftItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaItemUi_viewAfterButton), MediaItemUi::viewAfterButtonClicked, MediaItemUi::viewAfterButtonFocused, UiText::instance->getText (UiTextId::MediaItemUiViewAfterTooltip), "viewAfterButton"));
	}

	toolbar->addRightItem (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playButton), MediaItemUi::playButtonClicked, MediaItemUi::playButtonFocused, UiText::instance->getText (UiTextId::MediaItemUiPlayTooltip), "playButton"));
	if (mediaItem.isVideo) {
		toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaItemUi_selectPlayPositionButton), MediaItemUi::selectPlayPositionButtonClicked, MediaItemUi::selectPlayPositionButtonFocused, UiText::instance->getText (UiTextId::MediaItemUiSelectPlayPositionTooltip), "selectPlayPositionButton"));
		toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaItemUi_playMarkerButton), MediaItemUi::addMarkerButtonClicked, MediaItemUi::addMarkerButtonFocused, UiText::instance->getText (UiTextId::MediaItemUiAddMarkerTooltip), "addMarkerButton"));
	}
}

void MediaItemUi::doResume () {
	StdString cardid, text, rationame, framesizename, attributedesc;
	Int64List::const_iterator i1, i2;
	MediaThumbnailWindow *thumbnail;
	TextCardWindow *textcard;
	IconLabelWindow *iconlabel;
	StringList attributes;
	HashMap *prefs;
	int row;
	int64_t timestamp;

	UiStack::instance->setNextBackgroundTexturePath ("ui/MediaItemUi/bg");
	if (timelineWindow) {
		timelineWindow->readRecord (mediaId);
	}

	if (mediaItem.isVideo || mediaItem.hasAudioAlbumArt) {
		if ((! mediaItem.thumbnailTimestamps.empty ()) && (mediaItem.width > 0) && (mediaItem.height > 0)) {
			i1 = mediaItem.thumbnailTimestamps.cbegin ();
			i2 = mediaItem.thumbnailTimestamps.cend ();
			while (i1 != i2) {
				timestamp = *i1;
				cardid = getFrameThumbnailItemId (timestamp);
				if (! cardView->contains (cardid)) {
					thumbnail = createFrameThumbnailWindow ();
					thumbnail->itemId.assign (cardid);
					thumbnail->sortKey = getTimestampString (timestamp);
					thumbnail->setSourceImageFile (MediaControl::instance->getThumbnailPath (mediaId, timestamp), timestamp);
					thumbnailCardTimestamps.insertInOrder (getTimestampString (timestamp));
					cardView->addItem (thumbnail, thumbnail->itemId, FrameThumbnailRow, true);
					if (timelineWindow) {
						timelineWindow->addTimestampFill (timestamp);
					}
					shouldLoadMarkers = true;
				}
				++i1;
			}
		}
	}

	textcard = new TextCardWindow ();
	textcard->setTitleIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeMediaIcon));
	textcard->setCornerRadius (UiConfiguration::instance->cornerRadius);
	textcard->setTitleText (UiConfiguration::instance->fonts[TextCardWindow::titleTextFont]->truncatedText (mediaItem.name, App::instance->drawableWidth * nameTextWidthScale, Font::dotTruncateSuffix));
	textcard->setSubtitleText (UiConfiguration::instance->fonts[TextCardWindow::subtitleTextFont]->truncatedText (OsUtil::getPathDirname (mediaItem.mediaPath), App::instance->drawableWidth * nameTextWidthScale, Font::dotTruncateSuffix), UiConfiguration::instance->lightPrimaryTextColor);
	textcard->setMouseHoverTooltip (UiText::instance->getText (UiTextId::MediaName).capitalized ());
	cardView->addItem (textcard, InfoRow);

	if (mediaItem.isVideo) {
		if ((mediaItem.width > 0) && (mediaItem.height > 0)) {
			text.sprintf ("%ix%i", mediaItem.width, mediaItem.height);
			rationame = MediaUtil::getAspectRatioDisplayString (mediaItem.width, mediaItem.height);
			framesizename = MediaUtil::getFrameSizeName (mediaItem.width, mediaItem.height);
			if ((! rationame.empty ()) || (! framesizename.empty ())) {
				text.append (" (");
				if (! rationame.empty ()) {
					text.append (rationame);
				}
				if (! framesizename.empty ()) {
					if (! rationame.empty ()) {
						text.append (", ");
					}
					text.append (framesizename);
				}
				text.append (")");
			}
			attributes.push_back (text);
		}
		if (mediaItem.frameRate > 0.0f) {
			attributes.push_back (StdString::createSprintf ("%.2ffps", mediaItem.frameRate));
		}
		if (mediaItem.videoBitrate > 0) {
			attributes.push_back (MediaUtil::getBitrateDisplayString (mediaItem.videoBitrate));
		}
		else if (mediaItem.totalBitrate > 0) {
			attributes.push_back (MediaUtil::getBitrateDisplayString (mediaItem.totalBitrate));
		}
		attributedesc.assign (UiText::instance->getText (UiTextId::VideoAttributes).capitalized ());
	}
	else if (mediaItem.isAudio) {
		if (mediaItem.audioBitrate > 0) {
			attributes.push_back (MediaUtil::getBitrateDisplayString (mediaItem.audioBitrate));
		}
		else if (mediaItem.totalBitrate > 0) {
			attributes.push_back (MediaUtil::getBitrateDisplayString (mediaItem.totalBitrate));
		}
		attributedesc.assign (UiText::instance->getText (UiTextId::AudioAttributes).capitalized ());
	}

	if (! attributes.empty ()) {
		iconlabel = new IconLabelWindow (sprites.getSprite (SpriteId::MediaItemUi_attributesIcon), StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
		iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
		iconlabel->setPaddingScale (1.0f, 1.0f);
		iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
		iconlabel->setMouseHoverTooltip (attributedesc);
		iconlabel->setText (attributes.join (", "));
		cardView->addItem (iconlabel, InfoRow);
	}

	if (mediaItem.mediaFileSize > 0) {
		iconlabel = new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_storageIcon), StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
		iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
		iconlabel->setPaddingScale (1.0f, 1.0f);
		iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
		iconlabel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FileSize).capitalized ());
		iconlabel->setText (StdString::createSprintf ("%s (%lli)", UiText::instance->getByteCountText (mediaItem.mediaFileSize).c_str (), (long long int) mediaItem.mediaFileSize));
		cardView->addItem (iconlabel, InfoRow);
	}

	if (mediaItem.duration > 0) {
		iconlabel = new IconLabelWindow (sprites.getSprite (SpriteId::MediaItemUi_durationIcon), StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
		iconlabel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
		iconlabel->setPaddingScale (1.0f, 1.0f);
		iconlabel->setCornerRadius (UiConfiguration::instance->cornerRadius);
		iconlabel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Duration).capitalized ());
		iconlabel->setText (UiText::instance->getDurationText (mediaItem.duration));
		cardView->addItem (iconlabel, InfoRow);
	}

	prefs = App::instance->lockPrefs ();
	isTagWindowExpanded = prefs->find (MediaItemUi::tagWindowExpandedKey, false);
	App::instance->unlockPrefs ();
	tagWindowHandle.destroyAndAssign (new MediaItemTagWindow (mediaItem.tags));
	tagWindow->itemId.assign (cardView->getAvailableItemId ());
	tagWindow->expandStateChangeCallback = Widget::EventCallbackContext (MediaItemUi::tagWindowExpandStateChanged, this);
	tagWindow->addClickCallback = Widget::EventCallbackContext (MediaItemUi::tagWindowAddClicked, this);
	tagWindow->itemDeleteCallback = Widget::EventCallbackContext (MediaItemUi::tagWindowItemDeleted, this);
	row = InfoRow;
	if (isTagWindowExpanded) {
		tagWindow->setExpanded (true, true);
		row = TagWindowRow;
	}
	cardView->addItem (tagWindow, tagWindow->itemId, row);

	cardView->reflow ();
}

void MediaItemUi::doPause () {
	HashMap *prefs;
	PlayMarker marker;
	StdString errmsg, sql;
	OpResult result;

	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaItemUi::imageSizeKey, detailImageSize, (int) Ui::MediumSize);
	prefs->insert (MediaItemUi::tagWindowExpandedKey, isTagWindowExpanded, false);
	App::instance->unlockPrefs ();

	marker.recordId.assign (mediaId);
	cardView->processRowItems (MarkerRow, MediaItemUi::doPause_processItems, &marker);
	sql = marker.getUpdateSql ();
	result = Database::instance->exec (MediaControl::instance->databasePath, sql, &errmsg);
	if (result != OpResult::Success) {
		Log::debug ("Failed to store play marker record; id=\"%s\" err=\"%s\"", marker.recordId.c_str (), errmsg.c_str ());
	}
}
void MediaItemUi::doPause_processItems (void *markerPtr, Widget *itemWidget) {
	PlayMarker *marker = (PlayMarker *) markerPtr;
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		marker->markerTimestamps.push_back (thumbnail->thumbnailTimestamp);
	}
}

void MediaItemUi::doUpdate (int msElapsed) {
	std::list<MediaThumbnailWindow *>::iterator i1, i2;
	std::list<MediaThumbnailWindow *> items;
	MediaThumbnailWindow *thumbnail;
	StdString cardid, scrolltargetid, errmsg;
	PlayMarker marker;
	Int64List::const_iterator j1, j2;
	double x, y, w, h;
	bool loaderror;

	selectedThumbnailHandle.compact ();
	timelinePopupImageWindowHandle.compact ();

	SDL_LockMutex (loadThumbnailsMutex);
	items.swap (loadThumbnails);
	SDL_UnlockMutex (loadThumbnailsMutex);

	if (! items.empty ()) {
		loaderror = false;
		i1 = items.begin ();
		i2 = items.end ();
		while (i1 != i2) {
			thumbnail = *i1;
			rootPanel->removeWidget (thumbnail);
			if (! thumbnail->isImageLoaded) {
				loaderror = true;
			}
			else {
				cardid = getFrameThumbnailItemId (thumbnail->thumbnailTimestamp);
				if (! cardView->contains (cardid)) {
					thumbnail->itemId.assign (cardid);
					thumbnail->sortKey = getTimestampString (thumbnail->thumbnailTimestamp);
					thumbnail->isVisible = true;
					thumbnailCardTimestamps.insertInOrder (getTimestampString (thumbnail->thumbnailTimestamp));
					cardView->addItem (thumbnail, thumbnail->itemId, FrameThumbnailRow, true);
					cardView->animateItemScaleBump (thumbnail->itemId);
					scrolltargetid.assign (thumbnail->itemId);
					if (timelineWindow) {
						timelineWindow->addTimestampFill (thumbnail->thumbnailTimestamp);
					}
				}
				else {
					scrolltargetid.assign (cardid);
				}
			}
			thumbnail->release ();
			++i1;
		}
		items.clear ();
		cardView->reflow ();
		if (! scrolltargetid.empty ()) {
			cardView->scrollToItem (scrolltargetid);
		}
		if (loaderror) {
			App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiLoadThumbnailError));
		}
	}

	if (shouldLoadMarkers) {
		shouldLoadMarkers = false;
		marker.recordId.assign (mediaId);
		if (! marker.readDatabaseRow (MediaControl::instance->databasePath, &errmsg)) {
			Log::debug ("Failed to read play marker record; id=\"%s\" err=\"%s\"", marker.recordId.c_str (), errmsg.c_str ());
		}
		else {
			j1 = marker.markerTimestamps.cbegin ();
			j2 = marker.markerTimestamps.cend ();
			while (j1 != j2) {
				cardid = getMarkerThumbnailItemId (*j1);
				if (! cardView->contains (cardid)) {
					thumbnail = createMarkerThumbnailWindow ();
					thumbnail->setSourceVideoFrame (mediaItem.mediaPath, *j1);
					thumbnail->itemId.assign (cardid);
					thumbnail->sortKey = getTimestampString (*j1);
					cardView->addItem (thumbnail, thumbnail->itemId, MarkerRow);
				}
				++j1;
			}
		}
	}

	if (timelineWindow && (timelineHoverClock >= 0)) {
		timelineHoverClock -= msElapsed;
		if ((timelineHoverClock < 0) && (timelineHoverPosition >= 0.0f) && (mediaItem.height > 0) && (mediaItem.width > 0)) {
			w = App::instance->drawableWidth * UiConfiguration::instance->popupThumbnailImageScale;
			h = w * ((double) mediaItem.height) / ((double) mediaItem.width);
			x = timelineWindow->screenX + timelineHoverPosition - (w / 2.0f);
			y = timelineWindow->screenY - h - (UiConfiguration::instance->marginSize / 2.0f);
			timelineHoverTimestamp = (int64_t) (timelineHoverPosition * (double) mediaItem.duration / timelineWindow->width);
			timelinePopupImageWindowHandle.destroyAndAssign (new ImageWindow ());
			timelinePopupImageWindow->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
			timelinePopupImageWindow->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
			timelinePopupImageWindow->setLoadingSize (w, h);
			timelinePopupImageWindow->onLoadScale (w);
			timelinePopupImageWindow->loadSeekTimestampVideoFrame (mediaItem.mediaPath, timelineHoverTimestamp, true);
			timelinePopupImageWindow->isInputSuspended = true;
			timelinePopupImageWindow->position.assignBounded (x, y, 0.0f, y, App::instance->drawableWidth - w, y);
			App::instance->rootPanel->add (timelinePopupImageWindow, App::instance->rootPanel->maxWidgetZLevel + 1);
		}
	}

	if (shouldEnableTagWindow) {
		if (tagWindow) {
			tagWindow->setTags (mediaItem.tags);
			tagWindow->setDisabled (false);
			cardView->reflow ();
		}
		shouldEnableTagWindow = false;
	}
}

void MediaItemUi::doClearPopupWidgets () {
	if (timelinePopupImageWindow) {
		timelineWindow->setHighlightedPosition (-1);
		timelineHoverClock = -1;
		timelineHoverTimestamp = -1;
		timelinePopupImageWindowHandle.destroyAndClear ();
	}
}

void MediaItemUi::doResize () {
	if (timelineWindow) {
		timelineWindow->setBarWidth (App::instance->drawableWidth * timelineWidthScale);
	}
	cardView->processItems (MediaItemUi::doResize_processItems, this, true);
	UiStack::instance->secondaryToolbar->reflow ();
}
void MediaItemUi::doResize_processItems (void *itPtr, Widget *itemWidget) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		thumbnail->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}

void MediaItemUi::handleDetailImageSizeChange () {
	cardView->processItems (MediaItemUi::handleDetailImageSizeChange_processItems, this, true);
}
void MediaItemUi::handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		thumbnail->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}

MediaThumbnailWindow *MediaItemUi::createFrameThumbnailWindow () {
	MediaThumbnailWindow *thumbnail;

	thumbnail = new MediaThumbnailWindow (mediaItem.width, mediaItem.height);
	if (mediaItem.isVideo) {
		thumbnail->mouseEnterCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseEntered, this);
		thumbnail->mouseExitCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseExited, this);
		thumbnail->mouseClickCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailClicked, this);
		thumbnail->selectStateChangeCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailSelectStateChanged, this);
	}
	thumbnail->setDetailSize (detailImageSize, cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	return (thumbnail);
}

MediaThumbnailWindow *MediaItemUi::createMarkerThumbnailWindow () {
	MediaThumbnailWindow *thumbnail;

	thumbnail = new MediaThumbnailWindow (mediaItem.width, mediaItem.height);
	thumbnail->mouseEnterCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseEntered, this);
	thumbnail->mouseExitCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailMouseExited, this);
	thumbnail->mouseClickCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailClicked, this);
	thumbnail->selectStateChangeCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailSelectStateChanged, this);
	thumbnail->deleteClickCallback = Widget::EventCallbackContext (MediaItemUi::markerDeleteClicked, this);
	thumbnail->setDetailSize (detailImageSize, cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	thumbnail->setMarkerControl (true);
	return (thumbnail);
}

StdString MediaItemUi::getFrameThumbnailItemId (int64_t timestamp) {
	return (StdString::createSprintf ("frame%lli", (long long int) timestamp));
}

StdString MediaItemUi::getMarkerThumbnailItemId (int64_t timestamp) {
	return (StdString::createSprintf ("marker%lli", (long long int) timestamp));
}

void MediaItemUi::thumbnailMouseEntered (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	if (it->timelineWindow) {
		it->timelineWindow->setHighlightedPosition (thumbnail->thumbnailTimestamp);
	}
}

void MediaItemUi::thumbnailMouseExited (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	if (it->timelineWindow && (it->timelineWindow->highlightedPosition == thumbnail->thumbnailTimestamp)) {
		it->timelineWindow->setHighlightedPosition (-1);
	}
}

void MediaItemUi::thumbnailClicked (void *itPtr, Widget *widgetPtr) {
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	thumbnail->setSelected (! thumbnail->isSelected);
}

void MediaItemUi::thumbnailSelectStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	if (thumbnail->isSelected) {
		if (it->selectedThumbnail) {
			it->selectedThumbnail->setSelected (false, true);
		}
		it->selectedThumbnailHandle.assign (thumbnail);
	}
	else {
		if (it->selectedThumbnail == thumbnail) {
			it->selectedThumbnailHandle.clear ();
		}
	}
}

void MediaItemUi::thumbnailVideoFrameLoaded (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	thumbnail->imageLoadCallback = Widget::EventCallbackContext ();
	SDL_LockMutex (it->loadThumbnailsMutex);
	it->loadThumbnails.push_back (thumbnail);
	SDL_UnlockMutex (it->loadThumbnailsMutex);
}

void MediaItemUi::thumbnailCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		cardLabel->setText (UiText::instance->getTimespanText (thumbnail->thumbnailTimestamp, UiText::HoursUnit));
	}
}

void MediaItemUi::markerDeleteClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	it->cardView->removeItem (thumbnail->itemId);
}

void MediaItemUi::markerPlayClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	it->playTimestamp = thumbnail->thumbnailTimestamp;
	UiStack::instance->popUi ();
}

void MediaItemUi::timelinePositionHovered (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	PlayerTimelineWindow *timeline = (PlayerTimelineWindow *) widgetPtr;

	if (timeline->hoverPosition >= 0.0f) {
		if (! FLOAT_EQUALS (it->timelineHoverPosition, timeline->hoverPosition)) {
			it->timelineHoverClock = UiConfiguration::instance->mouseHoverThreshold;
			timeline->setHighlightedPosition ((int64_t) (timeline->hoverPosition * (double) it->mediaItem.duration / timeline->width));
		}
	}
	else {
		if (it->timelineHoverPosition >= 0.0f) {
			timeline->setHighlightedPosition (-1);
			it->timelineHoverClock = -1;
			it->timelineHoverTimestamp = -1;
			it->timelinePopupImageWindowHandle.destroyAndClear ();
		}
	}
	it->timelineHoverPosition = timeline->hoverPosition;
}

void MediaItemUi::timelinePositionClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	PlayerTimelineWindow *timeline = (PlayerTimelineWindow *) widgetPtr;
	MediaThumbnailWindow *thumbnail;
	int64_t seekpos;

	if (it->timelineHoverTimestamp >= 0) {
		seekpos = it->timelineHoverTimestamp;
	}
	else {
		seekpos = (int64_t) (timeline->clickPosition * (double) it->mediaItem.duration / timeline->width);
	}
	thumbnail = it->createFrameThumbnailWindow ();
	thumbnail->retain ();
	thumbnail->imageLoadCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailVideoFrameLoaded, it);
	thumbnail->setSourceVideoFrame (it->mediaItem.mediaPath, seekpos);
	thumbnail->isVisible = false;
	it->rootPanel->addWidget (thumbnail);
}

StdString MediaItemUi::getTimestampString (int64_t timestamp) {
	return (StdString::createSprintf ("%020lli", (long long int) timestamp));
}

void MediaItemUi::viewBeforeButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	StdString text;
	Color color;

	if (! it->selectedThumbnail) {
		text.assign (UiText::instance->getText (UiTextId::MediaItemUiNoFrameOrMarkerSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getTimespanText (it->selectedThumbnail->thumbnailTimestamp, UiText::HoursUnit));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MediaItemUiViewBeforePrompt), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void MediaItemUi::viewBeforeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail;
	int itempos;
	int64_t t1, t2, seekpos;

	if (! it->selectedThumbnail) {
		return;
	}
	t2 = it->selectedThumbnail->thumbnailTimestamp;
	itempos = it->thumbnailCardTimestamps.indexOf (it->getTimestampString (t2));
	if (itempos < 0) {
		return;
	}
	if (itempos == 0) {
		t1 = 0;
	}
	else {
		t1 = it->thumbnailCardTimestamps.at (itempos - 1).parsedInt ((int64_t) 0);
	}
	seekpos = (t1 + t2) / 2;
	thumbnail = it->createFrameThumbnailWindow ();
	thumbnail->retain ();
	thumbnail->imageLoadCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailVideoFrameLoaded, it);
	thumbnail->setSourceVideoFrame (it->mediaItem.mediaPath, seekpos);
	thumbnail->isVisible = false;
	it->rootPanel->addWidget (thumbnail);
}

void MediaItemUi::viewAfterButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	StdString text;
	Color color;

	if (! it->selectedThumbnail) {
		text.assign (UiText::instance->getText (UiTextId::MediaItemUiNoFrameOrMarkerSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getTimespanText (it->selectedThumbnail->thumbnailTimestamp, UiText::HoursUnit));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MediaItemUiViewAfterPrompt), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void MediaItemUi::viewAfterButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail;
	int itempos;
	int64_t t1, t2, seekpos;

	if (! it->selectedThumbnail) {
		return;
	}
	t1 = it->selectedThumbnail->thumbnailTimestamp;
	itempos = it->thumbnailCardTimestamps.indexOf (it->getTimestampString (t1));
	if (itempos < 0) {
		return;
	}
	if (itempos >= (int) (it->thumbnailCardTimestamps.size () - 1)) {
		t2 = it->mediaItem.duration;
	}
	else {
		t2 = it->thumbnailCardTimestamps.at (itempos + 1).parsedInt ((int64_t) 0);
	}
	seekpos = (t1 + t2) / 2;
	thumbnail = it->createFrameThumbnailWindow ();
	thumbnail->retain ();
	thumbnail->imageLoadCallback = Widget::EventCallbackContext (MediaItemUi::thumbnailVideoFrameLoaded, it);
	thumbnail->setSourceVideoFrame (it->mediaItem.mediaPath, seekpos);
	thumbnail->isVisible = false;
	it->rootPanel->addWidget (thumbnail);
}

void MediaItemUi::addMarkerButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	StdString text;
	Color color;

	if ((! it->selectedThumbnail) || it->selectedThumbnail->isMarkerControlEnabled) {
		text.assign (UiText::instance->getText (UiTextId::MediaItemUiNoFrameSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getTimespanText (it->selectedThumbnail->thumbnailTimestamp, UiText::HoursUnit));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MediaItemUiAddMarkerPrompt), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void MediaItemUi::addMarkerButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaThumbnailWindow *thumbnail;
	StdString cardid;

	if ((! it->selectedThumbnail) || it->selectedThumbnail->isMarkerControlEnabled) {
		return;
	}
	cardid = it->getMarkerThumbnailItemId (it->selectedThumbnail->thumbnailTimestamp);
	if (it->cardView->contains (cardid)) {
		return;
	}
	thumbnail = it->createMarkerThumbnailWindow ();
	thumbnail->setSourceCopy (it->selectedThumbnail);
	thumbnail->itemId.assign (cardid);
	thumbnail->sortKey = it->getTimestampString (it->selectedThumbnail->thumbnailTimestamp);
	it->cardView->addItem (thumbnail, thumbnail->itemId, MarkerRow);
}

void MediaItemUi::selectPlayPositionButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	StdString text;
	Color color;

	if (! it->selectedThumbnail) {
		text.assign (UiText::instance->getText (UiTextId::MediaItemUiNoFrameOrMarkerSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getTimespanText (it->selectedThumbnail->thumbnailTimestamp, UiText::HoursUnit));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MediaItemUiSelectPlayPositionPrompt), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void MediaItemUi::selectPlayPositionButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;

	if (! it->selectedThumbnail) {
		return;
	}
	it->selectPlayPositionTimestamp = it->selectedThumbnail->thumbnailTimestamp;
	UiStack::instance->popUi ();
}

void MediaItemUi::playButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	StdString text;
	Color color;

	text.assign (UiText::instance->getTimespanText (it->selectedThumbnail ? it->selectedThumbnail->thumbnailTimestamp : 0, UiText::HoursUnit));
	color.assign (UiConfiguration::instance->primaryTextColor);
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::Play).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void MediaItemUi::playButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;

	it->playTimestamp = it->selectedThumbnail ? it->selectedThumbnail->thumbnailTimestamp : 0;
	UiStack::instance->popUi ();
}

void MediaItemUi::tagWindowExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaItemTagWindow *tagwindow = (MediaItemTagWindow *) widgetPtr;

	it->cardView->setItemRow (tagwindow->itemId, tagwindow->isExpanded ? TagWindowRow : InfoRow);
	it->cardView->animateItemScaleBump (tagwindow->itemId);
	it->isTagWindowExpanded = tagwindow->isExpanded;
	it->clearPopupWidgets ();
}

void MediaItemUi::tagWindowAddClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaItemTagWindow *tagwindow = (MediaItemTagWindow *) widgetPtr;
	ActionWindow *action;
	TextFieldWindow *textfield;

	if (it->clearActionPopup (tagwindow, MediaItemUi::tagWindowAddClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiText::instance->getText (UiTextId::AddSearchKey).capitalized ());
	action->setDescriptionText (UiText::instance->getText (UiTextId::AddMediaTagPrompt));
	textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
	action->addOption (UiText::instance->getText (UiTextId::SearchKey), textfield);
	action->setOptionNameText (UiText::instance->getText (UiTextId::SearchKey), StdString ());
	action->setOptionNotEmptyString (UiText::instance->getText (UiTextId::SearchKey));
	action->closeCallback = Widget::EventCallbackContext (MediaItemUi::addTagActionClosed, it);
	action->setWidgetNames ("addTagActionWindow");

	it->showActionPopup (action, tagwindow, MediaItemUi::tagWindowAddClicked, tagwindow->getAddButtonScreenRect (), Ui::RightOfAlignment, Ui::YCenteredAlignment);
	textfield->assignKeyFocus ();
}

void MediaItemUi::addTagActionClosed (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	ActionWindow *action = (ActionWindow *) widgetPtr;
	StdString tag;
	MediaItemUi::TagTask *task;

	if ((! action->isConfirmed) || it->mediaId.empty ()) {
		return;
	}
	tag = action->getStringValue (UiText::instance->getText (UiTextId::SearchKey), StdString ()).lowercased ().filtered (MediaItem::sortKeyCharacters);
	if (tag.empty ()) {
		return;
	}
	if (it->tagWindow) {
		it->tagWindow->setDisabled (true);
	}
	task = new MediaItemUi::TagTask ();
	task->ui = it;
	task->tag.assign (tag);
	task->ui->retain ();
	App::instance->setUiActive ();
	TaskGroup::instance->run (TaskGroup::RunContext (MediaItemUi::addMediaItemTag, task, App::databaseWriteQueueId));
}

void MediaItemUi::addMediaItemTag (void *taskPtr) {
	MediaItemUi::TagTask *task = (MediaItemUi::TagTask *) taskPtr;

	task->ui->executeAddMediaItemTag (task->tag);
	task->ui->shouldEnableTagWindow = true;
	task->ui->release ();
	delete (task);
	App::instance->unsetUiActive ();
}
void MediaItemUi::executeAddMediaItemTag (const StdString &tag) {
	StdString sql, errmsg;
	OpResult result;
	StringList cmdtags;

	if (tag.empty ()) {
		return;
	}
	if (mediaItem.tags.contains (tag)) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiAddTagAlreadyExistsText));
		return;
	}
	cmdtags.assign (mediaItem.tags);
	cmdtags.push_back (tag);
	sql = MediaItem::getUpdateTagsSql (mediaId, cmdtags);
	result = Database::instance->exec (MediaControl::instance->databasePath, sql, &errmsg);
	if (result != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiTagRecordUpdateError));
		return;
	}
	mediaItem.tags.push_back (tag);
	storeRecord ();
	App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiAddTagCompleteText));
}

void MediaItemUi::tagWindowItemDeleted (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	MediaItemTagWindow *tagwindow = (MediaItemTagWindow *) widgetPtr;
	ActionWindow *action;

	it->clearActionPopup (tagwindow, MediaItemUi::tagWindowItemDeleted);
	if (it->lastRemoveTagValue.equals (tagwindow->itemDeleteTag)) {
		it->lastRemoveTagValue.assign ("");
		return;
	}
	it->lastRemoveTagValue.assign (tagwindow->itemDeleteTag);
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiConfiguration::instance->fonts[ActionWindow::titleTextFont]->truncatedText (tagwindow->itemDeleteTag, tagwindow->width * 0.8f, Font::dotTruncateSuffix));
	action->setDescriptionText (UiText::instance->getText (UiTextId::RemoveMediaTagPrompt));
	action->closeCallback = Widget::EventCallbackContext (MediaItemUi::removeTagActionClosed, it);

	it->showActionPopup (action, tagwindow, MediaItemUi::tagWindowItemDeleted, tagwindow->getDeleteButtonScreenRect (), Ui::RightOfAlignment, Ui::YCenteredAlignment);
}

void MediaItemUi::removeTagActionClosed (void *itPtr, Widget *widgetPtr) {
	MediaItemUi *it = (MediaItemUi *) itPtr;
	ActionWindow *action = (ActionWindow *) widgetPtr;
	StdString tag;
	MediaItemUi::TagTask *task;

	if ((! action->isConfirmed) || (! it->tagWindow)) {
		return;
	}
	tag = it->tagWindow->itemDeleteTag.lowercased ().filtered (MediaItem::sortKeyCharacters);
	if (tag.empty ()) {
		return;
	}
	it->tagWindow->setDisabled (true);
	task = new MediaItemUi::TagTask ();
	task->ui = it;
	task->tag.assign (tag);
	task->ui->retain ();
	App::instance->setUiActive ();
	TaskGroup::instance->run (TaskGroup::RunContext (MediaItemUi::removeMediaItemTag, task, App::databaseWriteQueueId));
}

void MediaItemUi::removeMediaItemTag (void *taskPtr) {
	MediaItemUi::TagTask *task = (MediaItemUi::TagTask *) taskPtr;

	task->ui->executeRemoveMediaItemTag (task->tag);
	task->ui->shouldEnableTagWindow = true;
	task->ui->release ();
	delete (task);
	App::instance->unsetUiActive ();
}
void MediaItemUi::executeRemoveMediaItemTag (const StdString &tag) {
	StdString sql, errmsg;
	OpResult result;
	StringList cmdtags;

	if (tag.empty ()) {
		return;
	}
	cmdtags.assign (mediaItem.tags);
	if (! cmdtags.contains (tag)) {
		return;
	}
	cmdtags.remove (tag);
	sql = MediaItem::getUpdateTagsSql (mediaId, cmdtags);
	result = Database::instance->exec (MediaControl::instance->databasePath, sql, &errmsg);
	if (result != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiTagRecordUpdateError));
		return;
	}
	mediaItem.tags.assign (cmdtags);
	storeRecord ();
	App::instance->showNotification (UiText::instance->getText (UiTextId::MediaItemUiRemoveTagCompleteText));
}

void MediaItemUi::storeRecord () {
	MediaItem item;
	StdString errmsg;
	Json *record;

	if (! item.readDatabaseMediaIdRow (MediaControl::instance->databasePath, &errmsg, mediaId)) {
		Log::debug ("Failed to read MediaItem record; id=\"%s\" err=\"%s\"", mediaId.c_str (), errmsg.c_str ());
		return;
	}
	record = item.createRecord (MediaControl::instance->agentId);
	RecordStore::instance->insert (record);
	delete (record);
	App::instance->shouldSyncRecordStore = true;
}

Widget *MediaItemUi::findLuaTargetWidget (const char *targetName) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (cardView->getItem (targetName, true));
	if (thumbnail) {
		return (thumbnail);
	}
	return (NULL);
}

void MediaItemUi::executeLuaTarget (Widget *targetWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (targetWidget);
	if (thumbnail) {
		thumbnail->setSelected (true);
	}
}

void MediaItemUi::executeLuaUntarget (Widget *targetWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (targetWidget);
	if (thumbnail) {
		thumbnail->setSelected (false);
	}
}
