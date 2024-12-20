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
#include "IntList.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "Json.h"
#include "RecordStore.h"
#include "MediaControl.h"
#include "Font.h"
#include "Label.h"
#include "LabelWindow.h"
#include "IconLabelWindow.h"
#include "Image.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Button.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "PlayerWindow.h"
#include "MediaPlaylistViewWindow.h"
#include "MediaPlaylistWindow.h"

constexpr const double windowWidthScale = 0.46f;
constexpr const double itemViewHeightScale = 0.48f;
constexpr const double unexpandedWidthScale = 0.66f;
constexpr const double sliderTrackWidthScale = 0.6f;

MediaPlaylistWindow::MediaPlaylistWindow ()
: Panel ()
, windowWidth (0.0f)
, isExpanded (false)
, isExecuting (false)
, isShowingSettings (false)
, playerHandle (&player)
, isLoadingRecords (false)
, currentPlayItemIndex (-1)
, currentPlayDuration (0)
, nextPlayIndex (-1)
, shouldResetPlayItemIds (false)
, shouldPlayNext (false)
, shouldPlayNextReverse (false)
, shouldScrollOnPlay (false)
{
	classId = ClassId::MediaPlaylistWindow;
	SdlUtil::createMutex (&loadMutex);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;

	headerIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playlistIcon)));
	headerIcon->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::headerIconClicked, this);
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	headerIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::ClickRenameTooltip));

	nameLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	nameLabel->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::nameLabelClicked, this);
	nameLabel->setFixedPadding (true, 0.0f, 0.0f);
	nameLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::ClickRenameTooltip));

	topItemCountLabel = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playlistItemIcon), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	topItemCountLabel->setFixedPadding (true, 0.0f, 0.0f);
	topItemCountLabel->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextId::MediaPlaylistItem, UiTextId::MediaPlaylistItems));
	topItemCountLabel->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);

	playStatusLabel = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playlistActiveIcon), StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->statusOkTextColor));
	playStatusLabel->setFixedPadding (true, 0.0f, 0.0f);
	playStatusLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Playing).capitalized ());
	playStatusLabel->setTextChangeHighlight (true, UiConfiguration::instance->lightPrimaryTextColor);
	playStatusLabel->isVisible = false;

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaPlaylistWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isInputSuspended = true;
	dividerPanel->isVisible = false;

	view = (MediaPlaylistViewWindow *) addWidget (new MediaPlaylistViewWindow (windowWidth, App::instance->drawableHeight * itemViewHeightScale));
	view->itemClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::viewItemClicked, this);
	view->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	view->isVisible = false;

	settingsPanel = add (new Panel (), 1);
	settingsPanel->isVisible = false;
	settingsPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	settingsPanel->setBorder (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	settingsPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	shuffleToggle = createShuffleToggle ();
	shuffleToggle->stateChangeCallback = Widget::EventCallbackContext (MediaPlaylistWindow::shuffleToggleStateChanged, this);
	shuffleToggle->setFixedPadding (true, 0.0f, 0.0f);
	shuffleToggle->setRightAligned (true);
	settingsPanel->add (shuffleToggle);

	startPositionSlider = createStartPositionSlider ();
	startPositionSlider->valueChangeCallback = Widget::EventCallbackContext (MediaPlaylistWindow::startPositionSliderValueChanged, this);
	startPositionSlider->setFixedPadding (true, 0.0f, 0.0f);
	startPositionSlider->setTrackWidthScale (sliderTrackWidthScale);
	settingsPanel->add (startPositionSlider);

	playDurationSlider = createPlayDurationSlider ();
	playDurationSlider->valueChangeCallback = Widget::EventCallbackContext (MediaPlaylistWindow::playDurationSliderValueChanged, this);
	playDurationSlider->setFixedPadding (true, 0.0f, 0.0f);
	playDurationSlider->setTrackWidthScale (sliderTrackWidthScale);
	settingsPanel->add (playDurationSlider);

	removeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::removeButtonClicked, this);
	removeButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::DeletePlaylist).capitalized ());
	removeButton->isVisible = false;

	bottomLeftPanel = add (new Panel (), 1);
	bottomLeftPanel->isVisible = false;
	bottomLeftPanel->setFixedPadding (true, 0.0f, 0.0f);
	bottomLeftPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	bottomItemCountLabel = bottomLeftPanel->add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playlistItemIcon), StdString ("0"), UiConfiguration::BodyFont, UiConfiguration::instance->lightPrimaryTextColor));
	bottomItemCountLabel->setFixedPadding (true, 0.0f, 0.0f);
	bottomItemCountLabel->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextId::MediaPlaylistItem, UiTextId::MediaPlaylistItems));
	bottomItemCountLabel->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);

	addItemButton = bottomLeftPanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_addPlaylistItemButton)));
	addItemButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::addItemButtonClicked, this);
	addItemButton->focusCallback = Widget::EventCallbackContext (MediaPlaylistWindow::addItemButtonFocused, this);
	addItemButton->unfocusCallback = Widget::EventCallbackContext (MediaPlaylistWindow::addItemButtonUnfocused, this);
	addItemButton->setImageColor (UiConfiguration::instance->buttonTextColor);

	editButton = bottomLeftPanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_editPlaylistButton)));
	editButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::editButtonClicked, this);
	editButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	editButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::EditPlaylist).capitalized ());

	bottomRightPanel = add (new Panel (), 1);
	bottomRightPanel->isVisible = false;
	bottomRightPanel->setFixedPadding (true, 0.0f, 0.0f);
	bottomRightPanel->setLayout (Panel::RightFlowLayoutOption);

	settingsButton = bottomRightPanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton)));
	settingsButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::settingsButtonClicked, this);
	settingsButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	settingsButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::MediaPlaylistWindowSettingsTooltip));

	skipPreviousButton = bottomRightPanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_skipPreviousButton)));
	skipPreviousButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::skipPreviousButtonClicked, this);
	skipPreviousButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	skipPreviousButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::MediaPlaylistWindowSkipPreviousTooltip));
	skipPreviousButton->setDisabled (true);

	skipNextButton = bottomRightPanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_skipNextButton)));
	skipNextButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::skipNextButtonClicked, this);
	skipNextButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	skipNextButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Play).capitalized ());
	skipNextButton->setDisabled (true);

	playButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playButton)));
	playButton->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistWindow::skipNextButtonClicked, this);
	playButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	playButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Play).capitalized ());
	playButton->setDisabled (true);

	reflow ();
}
MediaPlaylistWindow::~MediaPlaylistWindow () {
	if (player && (! player->isDestroyed)) {
		if (player->playlistId.equals (itemId)) {
			player->playlistId.assign ("");
		}
	}

	SDL_LockMutex (loadMutex);
	RecordStore::instance->remove (mediaItemIds);
	mediaItemIds.clear ();
	SDL_UnlockMutex (loadMutex);

	SdlUtil::destroyMutex (&loadMutex);
}

MediaPlaylistWindow *MediaPlaylistWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::MediaPlaylistWindow) ? (MediaPlaylistWindow *) widget : NULL);
}

StdString MediaPlaylistWindow::toStringDetail () {
	StdString s;
	return (s);
}

void MediaPlaylistWindow::resetPlaylistId () {
	playlist.id = RecordStore::instance->getRecordId (MediaPlaylist::idCommandType);
}

void MediaPlaylistWindow::setPlaylistName (const StdString &name) {
	StdString s;

	s = name.trimmed ();
	if (s.empty () || playlist.name.equals (s)) {
		return;
	}
	playlist.name.assign (s);
	headerIcon->widgetName.sprintf ("%sHeaderIcon", s.c_str ());
	nameLabel->widgetName.sprintf ("%sNameLabel", s.c_str ());
	removeButton->widgetName.sprintf ("%sRemoveButton", s.c_str ());
	addItemButton->widgetName.sprintf ("%sAddItemButton", s.c_str ());
	editButton->widgetName.sprintf ("%sEditButton", s.c_str ());
	settingsButton->widgetName.sprintf ("%sSettingsButton", s.c_str ());
	playButton->widgetName.sprintf ("%sPlayButton", s.c_str ());
	skipPreviousButton->widgetName.sprintf ("%sSkipPreviousButton", s.c_str ());
	skipNextButton->widgetName.sprintf ("%sSkipNextButton", s.c_str ());
	resetNameLabel ();
	reflow ();
}

void MediaPlaylistWindow::setShuffle (bool shuffle) {
	if (playlist.isShuffle == shuffle) {
		return;
	}
	playlist.isShuffle = shuffle;
	shuffleToggle->setChecked (playlist.isShuffle, true);
}

void MediaPlaylistWindow::setStartPosition (int startPosition) {
	if (playlist.startPosition == startPosition) {
		return;
	}
	playlist.startPosition = startPosition;
	startPositionSlider->setValue (playlist.startPosition, true);
}

void MediaPlaylistWindow::setPlayDuration (int playDuration) {
	if (playlist.playDuration == playDuration) {
		return;
	}
	playlist.playDuration = playDuration;
	playDurationSlider->setValue (playlist.playDuration, true);
}

void MediaPlaylistWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	playlist.isExpanded = isExpanded;
	if (isExpanded) {
		setPaddingScale (1.0f, 1.0f);
	}
	else {
		setPaddingScale (0.5f, 0.5f);
	}
	topItemCountLabel->isVisible = (! isExpanded) && (! isExecuting);
	playStatusLabel->isVisible = (! isExpanded) && isExecuting;
	playButton->isVisible = (! isExpanded);
	dividerPanel->isVisible = isExpanded;
	view->isVisible = isExpanded;
	settingsPanel->isVisible = isExpanded && isShowingSettings;
	removeButton->isVisible = isExpanded;
	bottomLeftPanel->isVisible = isExpanded;
	bottomRightPanel->isVisible = isExpanded;
	expandToggle->setChecked (isExpanded, true);
	resetNameLabel ();
	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void MediaPlaylistWindow::setShowingSettings (bool showing) {
	if (showing == isShowingSettings) {
		return;
	}
	isShowingSettings = showing;
	if (isShowingSettings) {
		settingsPanel->isVisible = isExpanded;
	}
	else {
		settingsPanel->isVisible = false;
	}
	reflow ();
}

void MediaPlaylistWindow::read (const MediaPlaylist &mediaPlaylist) {
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;

	SDL_LockMutex (loadMutex);
	RecordStore::instance->remove (mediaItemIds);
	mediaItemIds.clear ();
	SDL_UnlockMutex (loadMutex);

	playlist.clear ();
	playlist.id = mediaPlaylist.id;
	playlist.isExpanded = mediaPlaylist.isExpanded;
	playlist.isShuffle = mediaPlaylist.isShuffle;
	playlist.startPosition = mediaPlaylist.startPosition;
	playlist.playDuration = mediaPlaylist.playDuration;
	shuffleToggle->setChecked (playlist.isShuffle, true);
	startPositionSlider->setValue (playlist.startPosition, true);
	playDurationSlider->setValue (playlist.playDuration, true);
	setPlaylistName (mediaPlaylist.name);
	if (isExpanded != playlist.isExpanded) {
		setExpanded (playlist.isExpanded, true);
	}

	view->clearItems ();
	i1 = mediaPlaylist.items.cbegin ();
	i2 = mediaPlaylist.items.cend ();
	while (i1 != i2) {
		addItem (*i1);
		++i1;
	}
}

void MediaPlaylistWindow::addItem (const StdString &mediaId, int64_t startTimestamp) {
	addItem (MediaPlaylistItem (mediaId, startTimestamp));
}
void MediaPlaylistWindow::addItem (const MediaPlaylistItem &playlistItem) {
	bool found;

	view->addItem (playlistItem);
	playlist.items.push_back (playlistItem);
	resetItemCount ();
	reflow ();

	found = RecordStore::instance->exists (playlistItem.mediaId, true);
	SDL_LockMutex (loadMutex);
	if (found) {
		mediaItemIds.push_back (playlistItem.mediaId);
		App::instance->shouldSyncRecordStore = true;
	}
	else {
		loadIds.push_back (playlistItem.mediaId);
	}
	SDL_UnlockMutex (loadMutex);
}

void MediaPlaylistWindow::resetItems (const IntList &indexList) {
	IntList::const_iterator i1, i2;
	std::vector<MediaPlaylistItem> previtems;
	StringList previtemids;
	int index;

	SDL_LockMutex (loadMutex);
	previtemids.swap (mediaItemIds);
	SDL_UnlockMutex (loadMutex);

	previtems.swap (playlist.items);
	view->clearItems ();
	i1 = indexList.cbegin ();
	i2 = indexList.cend ();
	while (i1 != i2) {
		index = *i1;
		if ((index >= 0) && (index < (int) previtems.size ())) {
			addItem (previtems.at (index));
		}
		++i1;
	}

	RecordStore::instance->remove (previtemids);
	resetItemCount ();
	reflow ();
}

void MediaPlaylistWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerIcon->flowRight (&layoutFlow);
	if (isExpanded) {
		nameLabel->flowRight (&layoutFlow);
	}
	else {
		nameLabel->flowDown (&layoutFlow);
		layoutFlow.y -= (UiConfiguration::instance->marginSize / 2.0f);
		if (topItemCountLabel->isVisible) {
			topItemCountLabel->flowRight (&layoutFlow);
		}
		if (playStatusLabel->isVisible) {
			playStatusLabel->flowRight (&layoutFlow);
		}
	}

	nextColumnLayoutFlow ();
	if (removeButton->isVisible) {
		removeButton->flowRight (&layoutFlow);
	}
	if (playButton->isVisible) {
		playButton->flowRight (&layoutFlow);
	}
	expandToggle->flowRight (&layoutFlow);
	if (isExpanded) {
		nameLabel->centerVertical (&layoutFlow);
	}

	nextRowLayoutFlow ();
	if (isExpanded) {
		view->reflow ();
		settingsPanel->reflow ();
		bottomLeftPanel->reflow ();
		bottomRightPanel->reflow ();

		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);
		view->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		bottomLeftPanel->flowRight (&layoutFlow);
		if (bottomRightPanel->isVisible) {
			bottomRightPanel->flowRight (&layoutFlow);
		}

		setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
		dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);

		bottomRightLayoutFlow ();
		if (bottomRightPanel->isVisible) {
			bottomRightPanel->flowLeft (&layoutFlow);
		}

		if (settingsPanel->isVisible) {
			settingsPanel->position.assign (bottomLeftPanel->position.x, bottomLeftPanel->position.y - settingsPanel->height);
		}
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);
	if (isExpanded) {
		removeButton->flowLeft (&layoutFlow);
	}
}

void MediaPlaylistWindow::doResize () {
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	Panel::doResize ();
	view->setWindowSize (windowWidth, App::instance->drawableHeight * itemViewHeightScale);
	resetNameLabel ();
	MediaPlaylistWindow::resizeStartPositionSlider (startPositionSlider);
	MediaPlaylistWindow::resizePlayDurationSlider (playDurationSlider);
	reflow ();
}

void MediaPlaylistWindow::doUpdate (int msElapsed) {
	int count;

	playerHandle.compact ();
	updatePlay (msElapsed);
	Panel::doUpdate (msElapsed);

	if (! isLoadingRecords) {
		SDL_LockMutex (loadMutex);
		count = (int) loadIds.size ();
		SDL_UnlockMutex (loadMutex);
		if (count > 0) {
			isLoadingRecords = true;
			retain ();
			TaskGroup::instance->run (TaskGroup::RunContext (MediaPlaylistWindow::loadRecords, this));
		}
	}
}

void MediaPlaylistWindow::loadRecords (void *itPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;

	it->executeLoadRecords ();
	it->isLoadingRecords = false;
	it->release ();
}
void MediaPlaylistWindow::executeLoadRecords () {
	StringList ids;
	StringList::const_iterator i1, i2;
	StdString errmsg;
	MediaItem mediaitem;
	Json *record;

	SDL_LockMutex (loadMutex);
	ids.swap (loadIds);
	SDL_UnlockMutex (loadMutex);
	i1 = ids.cbegin ();
	i2 = ids.cend ();
	while (i1 != i2) {
		if (mediaitem.readDatabaseMediaIdRow (MediaControl::instance->databasePath, &errmsg, *i1)) {
			record = mediaitem.createRecord (MediaControl::instance->agentId);
			RecordStore::instance->insert (record, true);
			delete (record);

			SDL_LockMutex (loadMutex);
			mediaItemIds.push_back (*i1);
			SDL_UnlockMutex (loadMutex);
			App::instance->shouldSyncRecordStore = true;
		}
		++i1;
	}
}

void MediaPlaylistWindow::resetNameLabel () {
	double w;
	UiConfiguration::FontType fonttype;
	Font::Metrics metrics;

	w = windowWidth;
	if (isExpanded) {
		w -= (headerIcon->width + topItemCountLabel->width + removeButton->width + expandToggle->width + (UiConfiguration::instance->marginSize * 4.0f) + (widthPadding * 2.0f));
		fonttype = UiConfiguration::HeadlineFont;
		UiConfiguration::instance->fonts[fonttype]->resetMetrics (&metrics, playlist.name);
		if (metrics.textWidth >= w) {
			fonttype = UiConfiguration::BodyFont;
		}
	}
	else {
		w *= unexpandedWidthScale;
		w -= (headerIcon->width + expandToggle->width + (UiConfiguration::instance->marginSize * 2.0f) + (widthPadding * 2.0f));
		fonttype = UiConfiguration::BodyFont;
	}
	nameLabel->setFont (fonttype);
	nameLabel->setText (UiConfiguration::instance->fonts[fonttype]->truncatedText (playlist.name, w, Font::dotTruncateSuffix));
}

void MediaPlaylistWindow::resetItemCount () {
	int itemcount;

	itemcount = (int) playlist.items.size ();
	topItemCountLabel->setText (StdString::createSprintf ("%i", itemcount));
	topItemCountLabel->tooltipText.assign (UiText::instance->getCountText (itemcount, UiTextId::MediaPlaylistItem, UiTextId::MediaPlaylistItems));
	bottomItemCountLabel->setText (StdString::createSprintf ("%i", itemcount));
	bottomItemCountLabel->tooltipText.assign (UiText::instance->getCountText (itemcount, UiTextId::MediaPlaylistItem, UiTextId::MediaPlaylistItems));
	playButton->setDisabled (itemcount <= 0);
	skipNextButton->setDisabled (itemcount <= 0);
	skipPreviousButton->setDisabled (itemcount <= 0);
}

Widget::Rectangle MediaPlaylistWindow::getRemoveButtonScreenRect () {
	return (removeButton->getScreenRect ());
}

ToggleWindow *MediaPlaylistWindow::createShuffleToggle () {
	ToggleWindow *toggle;

	toggle = new ToggleWindow (new Toggle ());
	toggle->setIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_shuffleIcon));
	toggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	toggle->setMouseHoverTooltip (UiText::instance->getText (UiTextId::ShuffleTooltip));
	toggle->setChecked (playlist.isShuffle);
	return (toggle);
}

SliderWindow *MediaPlaylistWindow::createStartPositionSlider () {
	Slider *slider;
	SliderWindow *window;
	int i;

	slider = new Slider (0.0f, (double) (MediaPlaylist::StartPositionCount - 1));
	for (i = 0; i < MediaPlaylist::StartPositionCount; ++i) {
		slider->addSnapValue ((double) i);
	}
	window = new SliderWindow (slider);
	window->setIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_startPositionIcon));
	window->setMouseHoverTooltip (UiText::instance->getText (UiTextId::StartPosition).capitalized ());
	window->setValueNameFunction (MediaPlaylistWindow::startPositionSliderValueName, MediaPlaylistWindow::getStartPositionSliderValueMaxTextWidth ());
	window->setValue (playlist.startPosition);
	return (window);
}

SliderWindow *MediaPlaylistWindow::createPlayDurationSlider () {
	Slider *slider;
	SliderWindow *window;
	int i;

	slider = new Slider (0.0f, (double) (MediaPlaylist::PlayDurationCount - 1));
	for (i = 0; i < MediaPlaylist::PlayDurationCount; ++i) {
		slider->addSnapValue ((double) i);
	}
	window = new SliderWindow (slider);
	window->setIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_durationIcon));
	window->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayDuration).capitalized ());
	window->setValueNameFunction (MediaPlaylistWindow::playDurationSliderValueName, MediaPlaylistWindow::getPlayDurationSliderValueNameMaxTextWidth ());
	window->setValue (playlist.playDuration);
	return (window);
}

void MediaPlaylistWindow::resizeStartPositionSlider (SliderWindow *slider) {
	slider->setValueNameFunction (MediaPlaylistWindow::startPositionSliderValueName, MediaPlaylistWindow::getStartPositionSliderValueMaxTextWidth ());
}

void MediaPlaylistWindow::resizePlayDurationSlider (SliderWindow *slider) {
	slider->setValueNameFunction (MediaPlaylistWindow::playDurationSliderValueName, MediaPlaylistWindow::getPlayDurationSliderValueNameMaxTextWidth ());
}

StdString MediaPlaylistWindow::startPositionSliderValueName (double sliderValue) {
	switch ((int) sliderValue) {
		case MediaPlaylist::ZeroStartPosition: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistZeroStartPositionDescription));
		}
		case MediaPlaylist::NearBeginningStartPosition: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistNearBeginningStartPositionDescription));
		}
		case MediaPlaylist::MiddleStartPosition: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistMiddleStartPositionDescription));
		}
		case MediaPlaylist::NearEndStartPosition: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistNearEndPositionDescription));
		}
		case MediaPlaylist::FullRangeStartPosition: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistFullRangePositionDescription));
		}
	}
	return (StdString ());
}
double MediaPlaylistWindow::getStartPositionSliderValueMaxTextWidth () {
	Font::Metrics metrics;
	double max;
	int i;
	int s[5] = { UiTextId::MediaPlaylistZeroStartPositionDescription, UiTextId::MediaPlaylistNearBeginningStartPositionDescription, UiTextId::MediaPlaylistMiddleStartPositionDescription, UiTextId::MediaPlaylistNearEndPositionDescription, UiTextId::MediaPlaylistFullRangePositionDescription };

	max = 0.0f;
	for (i = 0; i < 5; ++i) {
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, UiText::instance->getText (s[i]));
		if (metrics.textWidth > max) {
			max = metrics.textWidth;
		}
	}
	return (max);
}

StdString MediaPlaylistWindow::playDurationSliderValueName (double sliderValue) {
	switch ((int) sliderValue) {
		case MediaPlaylist::VeryShortPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistVeryShortPlayDurationDescription));
		}
		case MediaPlaylist::ShortPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistShortPlayDurationDescription));
		}
		case MediaPlaylist::MediumPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistMediumPlayDurationDescription));
		}
		case MediaPlaylist::LongPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistLongPlayDurationDescription));
		}
		case MediaPlaylist::VeryLongPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistVeryLongPlayDurationDescription));
		}
		case MediaPlaylist::FullPlayDuration: {
			return (UiText::instance->getText (UiTextId::MediaPlaylistFullPlayDurationDescription));
		}
	}
	return (StdString ());
}
double MediaPlaylistWindow::getPlayDurationSliderValueNameMaxTextWidth () {
	Font::Metrics metrics;
	double max;
	int i;
	int s[6] = { UiTextId::MediaPlaylistVeryShortPlayDurationDescription, UiTextId::MediaPlaylistShortPlayDurationDescription, UiTextId::MediaPlaylistMediumPlayDurationDescription, UiTextId::MediaPlaylistLongPlayDurationDescription, UiTextId::MediaPlaylistVeryLongPlayDurationDescription, UiTextId::MediaPlaylistFullPlayDurationDescription };

	max = 0.0f;
	for (i = 0; i < 6; ++i) {
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, UiText::instance->getText (s[i]));
		if (metrics.textWidth > max) {
			max = metrics.textWidth;
		}
	}
	return (max);
}

void MediaPlaylistWindow::startPositionSliderValueChanged (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;
	SliderWindow *slider = (SliderWindow *) widgetPtr;

	it->playlist.startPosition = (int) slider->value;
	it->eventCallback (it->optionChangeCallback);
}

void MediaPlaylistWindow::playDurationSliderValueChanged (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;
	SliderWindow *slider = (SliderWindow *) widgetPtr;

	it->playlist.playDuration = (int) slider->value;
	it->eventCallback (it->optionChangeCallback);
}

void MediaPlaylistWindow::headerIconClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->renameClickCallback);
}

void MediaPlaylistWindow::nameLabelClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->renameClickCallback);
}

void MediaPlaylistWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

void MediaPlaylistWindow::settingsButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;

	it->setShowingSettings (! it->isShowingSettings);
}

void MediaPlaylistWindow::addItemButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->addItemClickCallback);
}

void MediaPlaylistWindow::addItemButtonFocused (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->addItemFocusCallback);
}

void MediaPlaylistWindow::addItemButtonUnfocused (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->addItemUnfocusCallback);
}

void MediaPlaylistWindow::editButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->editClickCallback);
}

void MediaPlaylistWindow::skipNextButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->skipNext ();
}
void MediaPlaylistWindow::skipNext () {
	if (isExecuting) {
		shouldScrollOnPlay = true;
		shouldPlayNextReverse = false;
		shouldPlayNext = true;
	}
	else {
		shouldScrollOnPlay = true;
		eventCallback (playClickCallback);
	}
}

void MediaPlaylistWindow::skipPreviousButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->skipPrevious ();
}
void MediaPlaylistWindow::skipPrevious () {
	if (isExecuting) {
		shouldScrollOnPlay = true;
		if (! playlist.isShuffle) {
			nextPlayIndex = currentPlayItemIndex;
		}
		shouldPlayNextReverse = true;
		shouldPlayNext = true;
		shouldResetPlayItemIds = true;
	}
	else {
		shouldScrollOnPlay = true;
		eventCallback (playClickCallback);
	}
}

void MediaPlaylistWindow::stop () {
	if (player) {
		player->stop ();
		player->isDestroyed = true;
	}
}

void MediaPlaylistWindow::removeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) itPtr)->eventCallback (((MediaPlaylistWindow *) itPtr)->removeClickCallback);
}

void MediaPlaylistWindow::shuffleToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->playlist.isShuffle = toggle->isChecked;
	it->eventCallback (it->optionChangeCallback);
}

void MediaPlaylistWindow::viewItemClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *it = (MediaPlaylistWindow *) itPtr;
	MediaPlaylistViewWindow *view = (MediaPlaylistViewWindow *) widgetPtr;

	it->nextPlayIndex = view->clickItemIndex;
	if (! it->isExecuting) {
		it->eventCallback (it->playClickCallback);
	}
	else {
		it->shouldPlayNext = true;
	}
}

void MediaPlaylistWindow::play (PlayerWindow *playerWindow) {
	playerHandle.assign (playerWindow);
	player->playlistId.assign (itemId);
	isExecuting = true;
	if (! isExpanded) {
		topItemCountLabel->isVisible = (! isExecuting);
		playStatusLabel->isVisible = isExecuting;
	}
	shouldResetPlayItemIds = true;
	shouldPlayNext = true;
	shouldPlayNextReverse = false;
	currentPlayItemIndex = -1;
	reflow ();
}

void MediaPlaylistWindow::updatePlay (int msElapsed) {
	MediaPlaylistItem playlistitem;
	MediaItem mediaitem;
	int playindex, minpct, maxpct;
	int64_t minduration, maxduration, seekts;
	double pct, delta;

	if (! isExecuting) {
		return;
	}
	if ((! player) || player->isDestroyed) {
		endPlay ();
		return;
	}
	if (! player->playlistId.equals (itemId)) {
		endPlay ();
		return;
	}

	if ((currentPlayDuration > 0) && player->isPlaying () && (! player->isPaused ())) {
		currentPlayDuration -= msElapsed;
		if (currentPlayDuration <= 0) {
			currentPlayDuration = 0;
			shouldPlayNext = true;
		}
	}
	if (playItemIndexes.empty ()) {
		shouldResetPlayItemIds = true;
		shouldPlayNext = true;
	}
	if (nextPlayIndex >= 0) {
		shouldResetPlayItemIds = true;
	}
	if (shouldResetPlayItemIds) {
		resetPlayItemIds ();
		shouldResetPlayItemIds = false;
		if (playItemIndexes.empty ()) {
			endPlay ();
			return;
		}
	}
	if (! player->isPlaying ()) {
		if (! shouldPlayNext) {
			shouldPlayNext = true;
			shouldScrollOnPlay = true;
		}
	}
	if (! shouldPlayNext) {
		return;
	}
	shouldPlayNext = false;

	if (! playItemIndexes.next (&playindex)) {
		view->setActiveItem (-1);
		shouldResetPlayItemIds = true;
		return;
	}
	currentPlayItemIndex = playindex;
	view->setActiveItem (currentPlayItemIndex);
	if (shouldScrollOnPlay) {
		view->scrollToItem (currentPlayItemIndex);
		shouldScrollOnPlay = false;
	}
	if ((currentPlayItemIndex >= 0) && (currentPlayItemIndex < (int) playlist.items.size ())) {
		playlist.getPlayDurationRange (&minduration, &maxduration);
		if ((minduration <= 0) && (maxduration <= 0)) {
			currentPlayDuration = 0;
		}
		else {
			currentPlayDuration = Prng::instance->getRandomNumber (minduration, maxduration);
		}

		playlistitem = playlist.items.at (currentPlayItemIndex);
		seekts = playlistitem.startTimestamp;
		pct = 0.0f;
		playlist.getStartPositionRange (&minpct, &maxpct);
		if ((minpct > 0) || (maxpct > 0)) {
			if (mediaitem.readRecordStore (playlistitem.mediaId) && (mediaitem.duration > 0)) {
				delta = (double) (mediaitem.duration - playlistitem.startTimestamp);
				pct = Prng::instance->getRandomNumber ((double) minpct, (double) maxpct);
				seekts += (int64_t) (pct / 100.0f * delta);
			}
		}
		playStatusLabel->setText (StdString::createSprintf ("%i/%i", currentPlayItemIndex + 1, (int) playlist.items.size ()));
		player->setPlayMedia (playlistitem.mediaId);
		player->setPlaySeekTimestamp (seekts);
		player->play ();
	}
}
void MediaPlaylistWindow::endPlay () {
	if (player && (! player->isDestroyed)) {
		if (player->playlistId.equals (itemId)) {
			player->playlistId.assign ("");
		}
	}
	playerHandle.clear ();
	isExecuting = false;
	if (! isExpanded) {
		topItemCountLabel->isVisible = (! isExecuting);
		playStatusLabel->isVisible = isExecuting;
	}
	playStatusLabel->setText (StdString ());
	shouldPlayNext = false;
	shouldPlayNextReverse = false;
	currentPlayItemIndex = -1;
	view->setActiveItem (-1);
	reflow ();
}

void MediaPlaylistWindow::resetPlayItemIds () {
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;
	bool found;
	int index, nextindex;

	playItemIndexes.clear ();
	playItemIndexes.nextItemIndex = -1;
	index = 0;
	nextindex = -1;
	i1 = playlist.items.cbegin ();
	i2 = playlist.items.cend ();
	while (i1 != i2) {
		found = false;
		SDL_LockMutex (loadMutex);
		if (mediaItemIds.contains (i1->mediaId)) {
			found = true;
		}
		SDL_UnlockMutex (loadMutex);
		if (found) {
			playItemIndexes.push_back (index);
			if ((nextindex < 0) && (nextPlayIndex >= 0) && (index == nextPlayIndex)) {
				nextindex = ((int) playItemIndexes.size ()) - 1;
			}
		}
		++index;
		++i1;
	}

	if (playlist.isShuffle) {
		playItemIndexes.randomizeOrder (Prng::instance);
		playItemIndexes.nextItemIndex = nextindex;
	}
	else {
		playItemIndexes.unrandomizeOrder ();
		if (nextindex >= 0) {
			if (shouldPlayNextReverse) {
				--nextindex;
				if (nextindex < 0) {
					nextindex = ((int) playItemIndexes.size ()) - 1;
				}
			}
			--nextindex;
			while (nextindex >= 0) {
				playItemIndexes.next (NULL);
				--nextindex;
			}
		}
	}
	shouldPlayNextReverse = false;
	nextPlayIndex = -1;
}
