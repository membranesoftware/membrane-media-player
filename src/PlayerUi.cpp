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
#include "OsUtil.h"
#include "Log.h"
#include "PrefsKey.h"
#include "MediaUtil.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "AppUrl.h"
#include "Font.h"
#include "Button.h"
#include "Toggle.h"
#include "Chip.h"
#include "Toolbar.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Database.h"
#include "CardView.h"
#include "Menu.h"
#include "UiStack.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "PlayerControl.h"
#include "SoundMixer.h"
#include "MediaItem.h"
#include "MediaControl.h"
#include "MediaSearchGroup.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "CardLabelWindow.h"
#include "IconLabelWindow.h"
#include "TextFieldWindow.h"
#include "TextCardWindow.h"
#include "AppCardWindow.h"
#include "MediaOptionWindow.h"
#include "MediaFilescanWindow.h"
#include "MediaPlaylist.h"
#include "MediaPlaylistWindow.h"
#include "MediaItemImageWindow.h"
#include "MediaItemDetailWindow.h"
#include "TagActionWindow.h"
#include "PlayFileActionWindow.h"
#include "PlayerWindow.h"
#include "MediaItemUi.h"
#include "MediaPlaylistUi.h"
#include "PlayerUi.h"

constexpr const int UnexpandedControlRow = 0;
constexpr const int ExpandedControlRow = 1;
constexpr const int PlaylistHeaderRow = 2;
constexpr const int UnexpandedPlaylistRow = 3;
constexpr const int ExpandedPlaylistRow = 4;
constexpr const int AudioDisabledAlertRow = 5;
constexpr const int PlayHistoryHeaderRow = 6;
constexpr const int PlayHistoryImageRow = 7;
constexpr const int PlayHistoryDetailRow = 8;
constexpr const int MediaFilescanHeaderRow = 9;
constexpr const int MediaFilescanImageRow = 10;
constexpr const int MediaFilescanDetailRow = 11;
constexpr const int MediaFilescanEmptyCardRow = 12;
constexpr const int LoadingIconRow = 13;
constexpr const int RowCount = 14;

constexpr const int ImageGridWindowMode = 0;
constexpr const int DetailLineWindowMode = 1;

constexpr const int EmptyMediaState = 0;
constexpr const int EmptySearchResultState = 1;

constexpr const SDL_Keycode selectAllKeycode = SDLK_a;
constexpr const SDL_Keycode fileMediaOpenKeycode = SDLK_o;
constexpr const double textTruncateWidthScale = 0.25f;
constexpr const double searchFieldWidthScale = 0.27f;
constexpr const double bottomPaddingHeightScale = 0.5f;

PlayerUi::PlayerUi ()
: Ui ()
, appCardHandle (&appCard)
, searchPanelHandle (&searchPanel)
, searchFieldHandle (&searchField)
, searchStatusIconHandle (&searchStatusIcon)
, mediaOptionWindowHandle (&mediaOptionWindow)
, mediaFilescanWindowHandle (&mediaFilescanWindow)
, emptyStateWindowHandle (&emptyStateWindow)
, loadingIconWindowHandle (&loadingIconWindow)
, targetMediaItemWindowHandle (&targetMediaItemWindow)
, lastSelectedMediaItemWindowHandle (&lastSelectedMediaItemWindow)
, playlistHeaderPanelHandle (&playlistHeaderPanel)
, playHistoryHeaderPanelHandle (&playHistoryHeaderPanel)
, playHistoryCountLabelHandle (&playHistoryCountLabel)
, mediaFilescanHeaderPanelHandle (&mediaFilescanHeaderPanel)
, expandPlaylistsToggleHandle (&expandPlaylistsToggle)
, createPlaylistButtonHandle (&createPlaylistButton)
, fileMediaOpenButtonHandle (&fileMediaOpenButton)
, playFileActionWindowHandle (&playFileActionWindow)
, audioDisabledAlertWindowHandle (&audioDisabledAlertWindow)
, emptyStateType (-1)
, mediaWindowMode (-1)
, mediaSortOrder (-1)
, mediaDisplayCount (0)
, mediaAvailableCount (0)
, isLoadingMedia (false)
, mediaSearchUpdateTime (0)
, searchRecordSyncClock (-1)
, isLoadMediaPlaylistsComplete (false)
, mediaItemUiPlayTimestamp (0)
, isSearchSyncRecordWaiting (false)
{
	classId = ClassId::PlayerUi;
	spritePrefix.assign (SpriteId::PlayerUi_prefix);
	SdlUtil::createMutex (&syncRecordMutex);
}
PlayerUi::~PlayerUi () {
	SdlUtil::destroyMutex (&syncRecordMutex);
}

Widget *PlayerUi::createBreadcrumbWidget () {
	return (new Chip (UiText::instance->getText (UiTextId::Player).capitalized (), sprites.getSprite (SpriteId::PlayerUi_breadcrumbIcon)));
}

OpResult PlayerUi::doLoad () {
	HashMap *prefs;
	int imagesize, soundmixvolume, visualizertype;
	bool appcardexpanded, optionwindowexpanded, shownews, soundmuted, subtitleenabled, skipprime;

	mediaDisplayCount = 0;
	mediaAvailableCount = 0;
	searchSyncRecordIds.clear ();
	playHistorySyncRecordIds.clear ();
	isLoadingMedia = false;
	emptyStateType = -1;

	prefs = App::instance->lockPrefs ();
	mediaWindowMode = prefs->find (PrefsKey::playerUiWindowMode, ImageGridWindowMode);
	mediaSortOrder = prefs->find (PrefsKey::playerUiSortOrder, (int) SystemInterface::Constant_NameSort);
	soundmixvolume = prefs->find (PrefsKey::soundMixVolume, SoundMixer::maxMixVolume);
	soundmuted = prefs->find (PrefsKey::soundMuted, false);
	visualizertype = prefs->find (PrefsKey::visualizerType, PlayerWindow::NoVisualizer);
	subtitleenabled = prefs->find (PrefsKey::subtitleEnabled, true);
	imagesize = prefs->find (PrefsKey::playerUiImageSize, (int) Ui::MediumSize);
	appcardexpanded = prefs->find (PrefsKey::appCardExpanded, false);
	optionwindowexpanded = prefs->find (PrefsKey::mediaOptionWindowExpanded, false);
	playerUiOptions.showPlayHistory = prefs->find (PrefsKey::showPlayHistory, true);
	playerUiOptions.showPlaylists = prefs->find (PrefsKey::showPlaylists, true);
	shownews = prefs->find (PrefsKey::showAppNews, false);
	skipprime = prefs->find (PrefsKey::skipPrimePanel, false);
	App::instance->unlockPrefs ();
	if (! skipprime) {
		appcardexpanded = true;
		optionwindowexpanded = true;
		shownews = false;
	}

	UiStack::instance->setPlayerControlOptions (soundmixvolume, soundmuted, visualizertype, subtitleenabled);

	setDetailImageSize (imagesize);
	cardView->setRowCount (RowCount);

	cardView->setRowReverseSorted (ExpandedPlaylistRow, true);

	cardView->setRowReverseSorted (PlayHistoryImageRow, true);
	cardView->setRowItemMarginSize (PlayHistoryImageRow, 0.0f);
	cardView->setRowSelectionAnimated (PlayHistoryImageRow, true);
	cardView->setRowRepositionAnimated (PlayHistoryImageRow, true);
	cardView->setRowLabeled (PlayHistoryImageRow, true, PlayerUi::mediaItemWindowCardViewItemLabel, this);
	cardView->setRowReverseSorted (PlayHistoryDetailRow, true);
	cardView->setRowItemMarginSize (PlayHistoryDetailRow, UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowRepositionAnimated (PlayHistoryDetailRow, true);

	cardView->setRowItemMarginSize (MediaFilescanImageRow, 0.0f);
	cardView->setRowSelectionAnimated (MediaFilescanImageRow, true);
	cardView->setRowLabeled (MediaFilescanImageRow, true, PlayerUi::mediaItemWindowCardViewItemLabel, this);
	cardView->setRowItemMarginSize (MediaFilescanDetailRow, UiConfiguration::instance->marginSize / 2.0f);

	cardView->setRowItemMarginSize (LoadingIconRow, UiConfiguration::instance->marginSize);

	cardView->setBottomPadding (App::instance->drawableHeight * bottomPaddingHeightScale);

	appCardHandle.assign (new AppCardWindow (&sprites, ! skipprime));
	appCard->widgetName.assign ("appInfoWindow");
	appCard->itemId = cardView->getAvailableItemId ();
	appCard->sortKey.assign ("a");
	appCard->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::appCardExpandStateChanged, this);
	appCard->layoutChangeCallback = Widget::EventCallbackContext (PlayerUi::appCardLayoutChanged, this);
	appCard->setExpanded (appcardexpanded, true);
	appCard->initialize (shownews, App::instance->isStartUpdateEnabled);
	cardView->addItem (appCard, appCard->itemId, appCard->isExpanded ? ExpandedControlRow : UnexpandedControlRow);

	mediaOptionWindowHandle.assign (new MediaOptionWindow (&sprites, playerUiOptions));
	mediaOptionWindow->widgetName.assign ("mediaOptionWindow");
	mediaOptionWindow->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaOptionWindowExpandStateChanged, this);
	mediaOptionWindow->layoutChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaOptionWindowLayoutChanged, this);
	mediaOptionWindow->configureCallback = Widget::EventCallbackContext (PlayerUi::mediaOptionWindowConfigured, this);
	mediaOptionWindow->itemId = cardView->getAvailableItemId ();
	mediaOptionWindow->sortKey.assign ("b");
	mediaOptionWindow->setExpanded (optionwindowexpanded, true);
	cardView->addItem (mediaOptionWindow, mediaOptionWindow->itemId, mediaOptionWindow->isExpanded ? ExpandedControlRow : UnexpandedControlRow);

	App::instance->setUiActive ();
	retain ();
	App::instance->addUpdateTask (PlayerUi::awaitMediaControlReady, this);
	return (OpResult::Success);
}

void PlayerUi::doUnload () {
	MediaSearchGroup::instance->removeListener (this);
	MediaSearchGroup::instance->configureMediaControlSearch (false);

	appCardHandle.clear ();
	searchPanelHandle.clear ();
	searchFieldHandle.clear ();
	searchStatusIconHandle.clear ();
	emptyStateWindowHandle.clear ();
	playlistHeaderPanelHandle.clear ();
	playHistoryHeaderPanelHandle.clear ();
	playHistoryCountLabelHandle.clear ();
	mediaFilescanHeaderPanelHandle.clear ();
	expandPlaylistsToggleHandle.clear ();
	createPlaylistButtonHandle.clear ();
	fileMediaOpenButtonHandle.clear ();
	playFileActionWindowHandle.clear ();
	mediaOptionWindowHandle.clear ();
	mediaFilescanWindowHandle.clear ();
	loadingIconWindowHandle.clear ();
	targetMediaItemWindowHandle.clear ();
	lastSelectedMediaItemWindowHandle.clear ();
	selectedMediaMap.clear ();
	audioDisabledAlertWindowHandle.clear ();

	playHistoryRecordIds.clear ();
	RecordStore::instance->remove (mediaOpenRecordIds);
	mediaOpenRecordIds.clear ();
	RecordStore::instance->remove (loadedRecordIds);
	loadedRecordIds.clear ();
}

void PlayerUi::doAddMainToolbarItems (Toolbar *toolbar) {
	Button *button;

	toolbar->addRightItem (createImageSizeButton ());

	button = new Button (sprites.getSprite (SpriteId::PlayerUi_navigateButton));
	button->widgetName.assign ("mainToolbarViewTargetButton");
	button->mouseClickCallback = Widget::EventCallbackContext (PlayerUi::navigateButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerUiNavigateTooltip));
	toolbar->addRightItem (button);
}

void PlayerUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	Button *button;

	searchPanelHandle.destroyAndAssign (new Panel ());
	searchPanel->add (Ui::createToolbarIconButton (sprites.getSprite (SpriteId::PlayerUi_sortButton), Widget::EventCallbackContext (PlayerUi::playerMenuButtonClicked, this), UiText::instance->getText (UiTextId::PlayerUiMenuTooltip), "playerMenuButton"));

	searchFieldHandle.destroyAndAssign (new TextFieldWindow (App::instance->drawableWidth * searchFieldWidthScale, UiText::instance->getText (UiTextId::EnterSearchKeyPrompt)));
	searchField->widgetName.assign ("searchText");
	searchField->setPaddingScale (1.0f, 0.0f);
	searchField->setButtonsEnabled (TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesAppendSeparatorOption | TextFieldWindow::ClearButtonOption);
	searchField->valueEditCallback = Widget::EventCallbackContext (PlayerUi::searchFieldEdited, this);
	searchPanel->add (searchField);

	searchPanel->add (Ui::createToolbarIconButton (sprites.getSprite (SpriteId::PlayerUi_searchButton), Widget::EventCallbackContext (PlayerUi::searchButtonClicked, this), UiText::instance->getText (UiTextId::PlayerUiSearchTooltip), "searchButton"));

	searchStatusIconHandle.destroyAndAssign (new IconLabelWindow (sprites.getSprite (SpriteId::PlayerUi_searchStatusIcon), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor));
	searchStatusIcon->widgetName.assign ("searchStatus");
	searchStatusIcon->setPaddingScale (1.0f, 0.0f);
	searchStatusIcon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
	searchStatusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchStatusIconTooltip));
	searchPanel->add (searchStatusIcon);

	searchPanel->setFixedPadding (true, 0.0f, 0.0f);
	searchPanel->layoutSpacing = 0.0f;
	searchPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);
	toolbar->setLeftCorner (searchPanel);

	toolbar->addRightItem (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playButton), PlayerUi::playButtonClicked, PlayerUi::playButtonFocused, UiText::instance->getText (UiTextId::PlayerUiPlayTooltip).capitalized (), "playButton", SDLK_F4));
	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_fullscreenButton), PlayerUi::fullscreenButtonClicked, PlayerUi::fullscreenButtonFocused, UiText::instance->getText (UiTextId::PlayerUiFullscreenTooltip).capitalized (), "fullscreenButton", SDLK_F3));
	toolbar->addRightItem (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_pauseButton), PlayerUi::pauseButtonClicked, PlayerUi::pauseButtonFocused, UiText::instance->getText (UiTextId::PlayerUiPauseTooltip).capitalized (), "pauseButton", SDLK_F2));
	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_stopButton), PlayerUi::stopButtonClicked, PlayerUi::stopButtonFocused, UiText::instance->getText (UiTextId::PlayerUiStopTooltip).capitalized (), "stopButton", SDLK_F1));

	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_tagButton), PlayerUi::tagButtonClicked, PlayerUi::tagButtonFocused, UiText::instance->getText (UiTextId::PlayerUiTagActionTooltip), "tagButton"));

	button = createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_starHalfButton), PlayerUi::selectAllButtonClicked, PlayerUi::selectAllButtonFocused, UiText::instance->getText (UiTextId::SelectAllTooltip), "selectAllButton");
	button->shortcutKey = selectAllKeycode;
	button->isShortcutKeyControlPress = true;
	toolbar->addRightItem (button);

	fileMediaOpenButtonHandle.destroyAndAssign (Ui::createToolbarIconButton (sprites.getSprite (SpriteId::PlayerUi_playMediaButton), Widget::EventCallbackContext (PlayerUi::fileMediaOpenButtonClicked, this), UiText::instance->getText (UiTextId::PlayerUiPlayFileTooltip), "fileMediaOpenButton"));
	toolbar->addRightItem (fileMediaOpenButton);
}

void PlayerUi::doResume () {
	UiStack::instance->setNextBackgroundTexturePath ("ui/PlayerUi/bg");
	mediaSearchUpdateTime = 0;
	searchField->setValue (searchKey, true, true);
	cardView->reflow ();
	resetExpandToggles ();
}

void PlayerUi::doPause () {
	HashMap *prefs;
	int soundmixvolume, visualizertype;
	bool soundmuted, subtitleenabled;

	UiStack::instance->getPlayerControlOptions (&soundmixvolume, &soundmuted, &visualizertype, &subtitleenabled);
	prefs = App::instance->lockPrefs ();
	if (mediaOptionWindow) {
		prefs->insert (PrefsKey::mediaOptionWindowExpanded, mediaOptionWindow->isExpanded, false);
	}
	if (mediaFilescanWindow) {
		prefs->insert (PrefsKey::mediaFilescanWindowExpanded, mediaFilescanWindow->isExpanded, false);
	}
	if (appCard) {
		prefs->insert (PrefsKey::appCardExpanded, appCard->isExpanded, false);
	}
	prefs->insert (PrefsKey::playerUiImageSize, detailImageSize, (int) Ui::MediumSize);
	prefs->insert (PrefsKey::playerUiSortOrder, mediaSortOrder, (int) SystemInterface::Constant_NameSort);
	prefs->insert (PrefsKey::playerUiWindowMode, mediaWindowMode, ImageGridWindowMode);
	prefs->insert (PrefsKey::soundMixVolume, soundmixvolume, SoundMixer::maxMixVolume);
	prefs->insert (PrefsKey::soundMuted, soundmuted, false);
	prefs->insert (PrefsKey::visualizerType, visualizertype, PlayerWindow::NoVisualizer);
	prefs->insert (PrefsKey::subtitleEnabled, subtitleenabled, true);
	prefs->insert (PrefsKey::showPlayHistory, playerUiOptions.showPlayHistory, true);
	prefs->insert (PrefsKey::showPlaylists, playerUiOptions.showPlaylists, true);
	App::instance->unlockPrefs ();
}

void PlayerUi::doUpdate (int msElapsed) {
	emptyStateWindowHandle.compact ();
	playlistHeaderPanelHandle.compact ();
	playHistoryHeaderPanelHandle.compact ();
	playHistoryCountLabelHandle.compact ();
	mediaFilescanHeaderPanelHandle.compact ();
	expandPlaylistsToggleHandle.compact ();
	createPlaylistButtonHandle.compact ();
	fileMediaOpenButtonHandle.compact ();
	playFileActionWindowHandle.compact ();
	loadingIconWindowHandle.compact ();
	targetMediaItemWindowHandle.compact ();
	lastSelectedMediaItemWindowHandle.compact ();
	audioDisabledAlertWindowHandle.compact ();
	updateSearch (msElapsed);
}

void PlayerUi::doResize () {
	cardView->processItems (PlayerUi::doResize_processItems, this, true);
	if (searchField) {
		searchField->setWindowWidth (App::instance->drawableWidth * searchFieldWidthScale);
	}
}
void PlayerUi::doResize_processItems (void *itPtr, Widget *itemWidget) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaItemWindow *mediaitem;

	mediaitem = MediaItemWindow::castWidget (itemWidget);
	if (mediaitem) {
		mediaitem->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}

void PlayerUi::awaitMediaControlReady (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (App::instance->isShuttingDown || App::instance->isShutdown) {
		it->release ();
		return;
	}
	if (! MediaControl::instance->isReady) {
		App::instance->addUpdateTask (PlayerUi::awaitMediaControlReady, it);
		return;
	}
	if (it->searchField) {
		it->searchField->setValue (StdString (), true, true);
		it->searchKey.assign (StdString ());
		it->resetSearch ();
	}
	MediaSearchGroup::instance->addListener (it, PlayerUi::mediaSearchRecordsAdded, PlayerUi::mediaSearchRecordsRemoved);
	MediaSearchGroup::instance->configureMediaControlSearch (true);
	MediaSearchGroup::instance->resetSearch (it->searchKey, it->mediaSortOrder);

	if (MediaControl::instance->mainOptions.savePlayHistory) {
		MediaControl::instance->getPlayHistoryRecordIds (&(it->playHistoryRecordIds));
		if (it->playerUiOptions.showPlayHistory) {
			if (! it->playHistoryRecordIds.empty ()) {
				SDL_LockMutex (it->syncRecordMutex);
				it->playHistorySyncRecordIds.append (it->playHistoryRecordIds);
				SDL_UnlockMutex (it->syncRecordMutex);
				App::instance->shouldSyncRecordStore = true;
			}
			it->resetPlayHistoryCount ();
		}
	}
	MediaControl::instance->addPlayHistoryListener (it, PlayerUi::mediaControlPlayHistoryRecordAdded, PlayerUi::mediaControlPlayHistoryRecordRemoved);

	if (MediaControl::instance->mainOptions.savePlaylists && it->playerUiOptions.showPlaylists) {
		it->isLoadMediaPlaylistsComplete = false;
		it->retain ();
		TaskGroup::instance->run (TaskGroup::RunContext (PlayerUi::loadMediaPlaylists, it, App::databaseWriteQueueId));

		App::instance->setUiActive ();
		it->retain ();
		App::instance->addUpdateTask (PlayerUi::awaitLoadMediaPlaylistsComplete, it);
	}

	App::instance->unsetUiActive ();
	it->resetMediaOptionRows ();
	it->release ();
}

void PlayerUi::navigateButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	Menu *menu;

	UiStack::instance->suspendMouseHover ();
	if (it->clearActionPopup (widgetPtr, PlayerUi::navigateButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextId::MediaOptions).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_smallMediaOptionIcon), Widget::EventCallbackContext (PlayerUi::navigateMediaOptionsClicked, it));
	if (it->playerUiOptions.showPlaylists && MediaControl::instance->mainOptions.savePlaylists && it->playlistHeaderPanel) {
		menu->addItem (UiText::instance->getText (UiTextId::Playlists).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallPlaylistIcon), Widget::EventCallbackContext (PlayerUi::navigatePlaylistsClicked, it));
	}
	if (it->playerUiOptions.showPlayHistory && it->playHistoryHeaderPanel) {
		menu->addItem (UiText::instance->getText (UiTextId::PlayedItems).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), Widget::EventCallbackContext (PlayerUi::navigatePlayHistoryClicked, it));
	}
	if (it->mediaFilescanWindow) {
		menu->addItem (UiText::instance->getText (UiTextId::MediaScan).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallComputerIcon), Widget::EventCallbackContext (PlayerUi::navigateMediaScanClicked, it));
	}
	it->showActionPopup (menu, widgetPtr, PlayerUi::navigateButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}
void PlayerUi::navigateMediaOptionsClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (! it->mediaOptionWindow) {
		return;
	}
	if (it->appCard) {
		it->appCard->setExpanded (false);
	}
	if (it->mediaFilescanWindow) {
		it->mediaFilescanWindow->setExpanded (true);
	}
	it->mediaOptionWindow->setExpanded (true);
	it->cardView->scrollToItem (it->mediaOptionWindow->itemId);
}
void PlayerUi::navigatePlaylistsClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->playlistHeaderPanel) {
		it->cardView->scrollToRow (PlaylistHeaderRow);
	}
}
void PlayerUi::navigatePlayHistoryClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->playHistoryHeaderPanel) {
		it->cardView->scrollToRow (PlayHistoryHeaderRow);
	}
}
void PlayerUi::navigateMediaScanClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->mediaFilescanHeaderPanel) {
		it->cardView->scrollToRow (MediaFilescanHeaderRow);
	}
}

static void updateSearch_matchLoadWaitingMediaItem (void *boolPtr, Widget *itemWidget) {
	MediaItemWindow *mediaitem;
	bool *b;

	b = (bool *) boolPtr;
	if (*b) {
		return;
	}
	mediaitem = MediaItemWindow::castWidget (itemWidget);
	if (mediaitem && mediaitem->isLoadWaiting) {
		*b = true;
	}
}
void PlayerUi::updateSearch (int msElapsed) {
	MediaSearchGroup::Status status;
	bool advance, waiting, loading;

	advance = false;
	SDL_LockMutex (syncRecordMutex);
	waiting = isSearchSyncRecordWaiting;
	SDL_UnlockMutex (syncRecordMutex);
	if (! waiting) {
		advance = cardView->isScrolledToBottom (App::instance->drawableHeight * bottomPaddingHeightScale);
	}
	if (advance) {
		loading = false;
		cardView->processItems (updateSearch_matchLoadWaitingMediaItem, &loading);
		if (loading) {
			advance = false;
		}
	}
	if (advance) {
		MediaSearchGroup::instance->advanceSearch ();
	}

	MediaSearchGroup::instance->getStatus (&status);
	if (searchStatusIcon && (mediaSearchUpdateTime != status.lastStatusUpdateTime)) {
		if (status.searchSetSize <= 0) {
			searchStatusIcon->setText (StdString ("0"));
			searchStatusIcon->setTextColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchStatusIconTooltip));
		}
		else if (status.searchReceiveCount >= status.searchSetSize) {
			searchStatusIcon->setText (StdString::createSprintf ("%i", status.searchSetSize));
			searchStatusIcon->setTextColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextId::SearchStatusIconTooltip).c_str (), UiText::instance->getText (UiTextId::SearchCompleteTooltip).c_str ()));
		}
		else {
			searchStatusIcon->setText (StdString::createSprintf ("%i / %i", status.searchReceiveCount, status.searchSetSize));
			searchStatusIcon->setTextColor (UiConfiguration::instance->mediumSecondaryColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->mediumSecondaryColor);
			searchStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextId::SearchStatusIconTooltip).c_str (), UiText::instance->getText (UiTextId::SearchInProgressTooltip).c_str ()));
		}
		searchPanel->reflow ();
		UiStack::instance->secondaryToolbar->reflow ();
		mediaSearchUpdateTime = status.lastStatusUpdateTime;
		syncSearchRecords ();
	}
	if (status.isLoading) {
		setLoadingIconVisible (true);
		if (emptyStateWindow) {
			cardView->removeItem (emptyStateWindow->itemId);
			emptyStateWindowHandle.clear ();
			emptyStateType = -1;
		}
	}
	else {
		setLoadingIconVisible (false);
		if (isLoadingMedia) {
			searchRecordSyncClock = 0;
		}
	}
	isLoadingMedia = status.isLoading;
	mediaAvailableCount = status.mediaAvailableCount;

	if (searchRecordSyncClock >= 0) {
		searchRecordSyncClock -= msElapsed;
		if (searchRecordSyncClock <= 0) {
			App::instance->shouldSyncRecordStore = true;
			searchRecordSyncClock = -1;
		}
	}
}

void PlayerUi::mediaOptionWindowExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaOptionWindow *window = (MediaOptionWindow *) widgetPtr;

	window->resetInputState ();
	it->cardView->animateItemScaleBump (window->itemId);
	it->clearPopupWidgets ();
	it->cardView->setItemRow (window->itemId, window->isExpanded ? ExpandedControlRow : UnexpandedControlRow, true);
	it->cardView->reflow ();
}
void PlayerUi::mediaOptionWindowLayoutChanged (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->cardView->reflow ();
}
void PlayerUi::mediaOptionWindowConfigured (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaOptionWindow *window = (MediaOptionWindow *) widgetPtr;

	it->clearPopupWidgets ();
	it->playerUiOptions = window->playerUiOptions;
	App::instance->showNotification (UiText::instance->getText (UiTextId::MediaOptionConfiguredText));
	App::instance->setUiActive ();
	it->retain ();
	App::instance->addUpdateTask (PlayerUi::awaitMediaControlReady, it);
}

void PlayerUi::resetMediaOptionRows () {
	Panel *panel, *sidepanel;
	Button *button;
	HashMap *prefs;
	bool expanded;

	if (! playerUiOptions.showPlayHistory) {
		if (playHistoryHeaderPanel) {
			cardView->removeItem (playHistoryHeaderPanel);
			playHistoryHeaderPanelHandle.destroyAndClear ();
		}
		cardView->removeRowItems (PlayHistoryImageRow);
		cardView->removeRowItems (PlayHistoryDetailRow);
	}
	else {
		if (! playHistoryHeaderPanel) {
			panel = new Panel ();

			playHistoryCountLabelHandle.destroyAndAssign (new LabelWindow (new Label (StdString (), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
			playHistoryCountLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
			panel->add (playHistoryCountLabel);

			sidepanel = panel->add (new Panel ());
			sidepanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
			sidepanel->setFixedPadding (true, 0.0f, 0.0f);
			button = sidepanel->add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton)));
			button->widgetName.assign ("clearPlayHistoryButton");
			button->mouseClickCallback = Widget::EventCallbackContext (PlayerUi::clearPlayHistoryButtonClicked, this);
			button->setInverseColor (true);
			button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerUiClearPlayHistoryTooltip));

			panel->setLayout (Panel::RightFlowLayoutOption);
			playHistoryHeaderPanelHandle.assign (panel);
			cardView->addItem (playHistoryHeaderPanel, PlayHistoryHeaderRow);
			resetPlayHistoryCount ();
		}
		SDL_LockMutex (syncRecordMutex);
		playHistorySyncRecordIds.append (playHistoryRecordIds);
		SDL_UnlockMutex (syncRecordMutex);
		App::instance->shouldSyncRecordStore = true;
	}

	if (! MediaControl::instance->mainOptions.mediaScan) {
		if (mediaFilescanHeaderPanel) {
			cardView->removeItem (mediaFilescanHeaderPanel);
			mediaFilescanHeaderPanelHandle.destroyAndClear ();
		}
		if (mediaFilescanWindow) {
			cardView->removeItem (mediaFilescanWindow->itemId);
			mediaFilescanWindowHandle.destroyAndClear ();
		}
		MediaSearchGroup::instance->configureMediaControlSearch (false);
	}
	else {
		if (! mediaFilescanHeaderPanel) {
			mediaFilescanHeaderPanelHandle.assign (createRowHeaderPanel (UiText::instance->getText (UiTextId::MediaScan).capitalized ()));
			cardView->addItem (mediaFilescanHeaderPanel, MediaFilescanHeaderRow);
		}
		if (! mediaFilescanWindow) {
			if (! MediaControl::instance->isFilescanConfigured) {
				expanded = true;
			}
			else {
				prefs = App::instance->lockPrefs ();
				expanded = prefs->find (PrefsKey::mediaFilescanWindowExpanded, false);
				App::instance->unlockPrefs ();
			}

			mediaFilescanWindowHandle.assign (new MediaFilescanWindow ());
			mediaFilescanWindow->widgetName.assign ("mediaScanWindow");
			mediaFilescanWindow->layoutChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaFilescanWindowLayoutChanged, this);
			mediaFilescanWindow->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaFilescanWindowExpandStateChanged, this);
			mediaFilescanWindow->itemId = cardView->getAvailableItemId ();
			mediaFilescanWindow->sortKey.assign ("c");
			mediaFilescanWindow->setExpanded (expanded, true);
			cardView->addItem (mediaFilescanWindow, mediaFilescanWindow->itemId, mediaFilescanWindow->isExpanded ? ExpandedControlRow : UnexpandedControlRow);
		}
	}

	if (!(MediaControl::instance->mainOptions.savePlaylists && playerUiOptions.showPlaylists)) {
		if (playlistHeaderPanel) {
			cardView->removeItem (playlistHeaderPanel);
			playlistHeaderPanelHandle.clear ();
		}
		UiStack::instance->stopPlaylists ();
		cardView->removeRowItems (ExpandedPlaylistRow);
		cardView->removeRowItems (UnexpandedPlaylistRow);
	}
	else {
		if (! playlistHeaderPanel) {
			panel = new Panel ();
			panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
			panel->setFixedPadding (true, 0.0f, 0.0f);
			expandPlaylistsToggleHandle.assign (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandAllLessButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandAllMoreButton)));
			expandPlaylistsToggle->stateChangeCallback = Widget::EventCallbackContext (PlayerUi::expandPlaylistsToggleStateChanged, this);
			expandPlaylistsToggle->setInverseColor (true);
			expandPlaylistsToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::MinimizeAll).capitalized (), UiText::instance->getText (UiTextId::ExpandAll).capitalized ());
			panel->add (expandPlaylistsToggle);

			createPlaylistButtonHandle.assign (new Button (sprites.getSprite (SpriteId::PlayerUi_createPlaylistButton)));
			createPlaylistButton->widgetName.assign ("createPlaylistButton");
			createPlaylistButton->mouseClickCallback = Widget::EventCallbackContext (PlayerUi::createPlaylistButtonClicked, this);
			createPlaylistButton->setInverseColor (true);
			createPlaylistButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerUiCreatePlaylistTooltip));
			panel->add (createPlaylistButton);

			panel->setLayout (Panel::RightFlowLayoutOption);
			playlistHeaderPanelHandle.assign (createRowHeaderPanel (UiText::instance->getText (UiTextId::Playlists).capitalized (), panel));
			cardView->addItem (playlistHeaderPanel, PlaylistHeaderRow);
		}
	}
}

void PlayerUi::fileMediaOpenButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->clearActionPopup (widgetPtr, PlayerUi::fileMediaOpenButtonClicked)) {
		return;
	}
	it->playFileActionWindowHandle.destroyAndAssign (new PlayFileActionWindow (&(it->sprites)));
	it->playFileActionWindow->mediaReadCallback = Widget::EventCallbackContext (PlayerUi::fileMediaReadComplete, it);
	it->showActionPopup (it->playFileActionWindow, widgetPtr, PlayerUi::fileMediaOpenButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
	it->playFileActionWindow->assignKeyFocus ();
	if (it->appCard) {
		it->appCard->clearStartupPrimePanel ();
	}
}
void PlayerUi::fileMediaReadComplete (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	PlayFileActionWindow *window = (PlayFileActionWindow *) widgetPtr;
	Json *record;

	if (window->mediaItem.id.empty ()) {
		if (! window->loadErrorMessage.empty ()) {
			App::instance->showNotification (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::PlayFileError).c_str (), window->loadErrorMessage.c_str ()));
		}
		else {
			App::instance->showNotification (UiText::instance->getText (UiTextId::InternalError));
		}
	}
	else if (UiStack::instance->getPlayerCount () >= PlayerControl::maxPlayerCount) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiMaxPlayerCountError));
	}
	else {
		record = window->mediaItem.createRecord (MediaControl::instance->agentId);
		RecordStore::instance->insert (record, true);
		delete (record);
		it->mediaOpenRecordIds.push_back (window->mediaItem.id);
		UiStack::instance->playMediaItem (window->mediaItem.id, 0, false);
	}
	it->clearPopupWidgets ();
}

void PlayerUi::doSyncRecordStore () {
	StringList ids;
	StringList::const_iterator i1, i2;
	MediaItem mediaitem;
	bool match, applysearchkey;
	int type;

	SDL_LockMutex (syncRecordMutex);
	while (true) {
		ids.clear ();
		searchSyncRecordIds.swap (ids);
		if (ids.empty ()) {
			break;
		}
		SDL_UnlockMutex (syncRecordMutex);

		i1 = ids.cbegin ();
		i2 = ids.cend ();
		while (i1 != i2) {
			showMediaItem (*i1, MediaFilescanImageRow);
			++i1;
		}

		SDL_LockMutex (syncRecordMutex);
	}
	isSearchSyncRecordWaiting = false;

	applysearchkey = (! searchKey.empty ());
	while (true) {
		ids.clear ();
		playHistorySyncRecordIds.swap (ids);
		if (ids.empty ()) {
			break;
		}
		SDL_UnlockMutex (syncRecordMutex);

		i1 = ids.cbegin ();
		i2 = ids.cend ();
		while (i1 != i2) {
			match = true;
			if (applysearchkey) {
				if (mediaitem.readRecordStore (*i1, true)) {
					if (! mediaitem.match (searchKey)) {
						match = false;
					}
				}
			}
			if (match) {
				showMediaItem (*i1, PlayHistoryImageRow);
			}
			++i1;
		}

		SDL_LockMutex (syncRecordMutex);
	}
	SDL_UnlockMutex (syncRecordMutex);
	resetPlayHistoryCount ();

	type = -1;
	if (MediaControl::instance->mainOptions.mediaScan && MediaControl::instance->isReady && (! isLoadingMedia) && (mediaDisplayCount <= 0)) {
		if (mediaAvailableCount <= 0) {
			type = EmptyMediaState;
		}
		else {
			type = EmptySearchResultState;
		}
	}
	if (type != emptyStateType) {
		if (emptyStateWindow) {
			cardView->removeItem (emptyStateWindow->itemId);
			emptyStateWindowHandle.clear ();
		}
		switch (type) {
			case EmptyMediaState: {
				emptyStateWindowHandle.assign (new TextCardWindow ());
				emptyStateWindow->setTitleIcon (sprites.getSprite (SpriteId::PlayerUi_mediaScanIcon));
				emptyStateWindow->setTitleText (UiText::instance->getText (UiTextId::PlayerUiEmptyMediaStatusTitle));
				emptyStateWindow->setDetailText (UiText::instance->getText (UiTextId::PlayerUiEmptyMediaStatusText));
				break;
			}
			case EmptySearchResultState: {
				emptyStateWindowHandle.assign (new TextCardWindow ());
				emptyStateWindow->setTitleIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeMediaIcon));
				emptyStateWindow->setTitleText (UiText::instance->getText (UiTextId::PlayerUiEmptySearchResultStatusTitle));
				emptyStateWindow->setDetailText (UiText::instance->getText (UiTextId::PlayerUiEmptySearchResultStatusText));
				break;
			}
		}
		if (emptyStateWindow) {
			emptyStateWindow->itemId.assign (cardView->getAvailableItemId ());
			cardView->addItem (emptyStateWindow, emptyStateWindow->itemId, MediaFilescanEmptyCardRow);
		}
	}
	emptyStateType = type;

	if (! SoundMixer::instance->isActive) {
		if (! audioDisabledAlertWindow) {
			audioDisabledAlertWindowHandle.assign (new TextCardWindow ());
			audioDisabledAlertWindow->setTitleIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon));
			audioDisabledAlertWindow->setTitleText (UiText::instance->getText (UiTextId::PlayerUiAudioDisabledAlertWindowTitle), UiConfiguration::instance->errorTextColor);
			audioDisabledAlertWindow->setDetailText (UiText::instance->getText (UiTextId::PlayerUiAudioDisabledAlertWindowText));
			cardView->addItem (audioDisabledAlertWindow, StdString (), AudioDisabledAlertRow);
		}
	}
	else {
		if (audioDisabledAlertWindow) {
			cardView->removeRowItems (AudioDisabledAlertRow);
			audioDisabledAlertWindowHandle.clear ();
		}
	}

	cardView->syncRecordStore ();
	cardView->reflow ();
	resetExpandToggles ();
}
void PlayerUi::showMediaItem (const StdString &recordId, int baseRow) {
	Json record;
	MediaItemWindow *item;
	MediaItemDetailWindow *mediaitemdetail;
	MediaItemImageWindow *mediaitemimage;
	int row;

	if (cardView->contains (recordId)) {
		return;
	}
	if (RecordStore::instance->find (&record, recordId, SystemInterface::CommandId_MediaItem, true)) {
		if (mediaWindowMode == DetailLineWindowMode) {
			mediaitemdetail = new MediaItemDetailWindow (&record);
			item = mediaitemdetail;
		}
		else {
			mediaitemimage = new MediaItemImageWindow (&record);
			item = mediaitemimage;
		}
		row = baseRow + mediaWindowMode;

		item->mediaImageClickCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowImageClicked, this);
		item->viewButtonClickCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowViewButtonClicked, this);
		item->selectStateChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowSelectStateChanged, this);
		item->setDetailSize (detailImageSize, cardView->cardAreaWidth / CardView::reducedSizeItemScale);
		if (selectedMediaMap.exists (recordId)) {
			item->setSelected (true, true);
		}
		if (baseRow == MediaFilescanImageRow) {
			item->isTagEnabled = true;
			item->isPlayMarkerEnabled = true;
		}
		else if (baseRow == PlayHistoryImageRow) {
			item->sortKey.sprintf ("%016llx", (long long int) item->mediaItem.mtime);
		}
		cardView->addItem (item, recordId, row);
		if (baseRow == MediaFilescanImageRow) {
			cardView->animateItemScaleBump (recordId);
		}
		++mediaDisplayCount;
		loadedRecordIds.push_back (recordId);
	}
}

void PlayerUi::setLoadingIconVisible (bool show) {
	if (show) {
		if (! loadingIconWindow) {
			loadingIconWindowHandle.assign (createLoadingIconWindow ());
			cardView->addItem (loadingIconWindow, LoadingIconRow);
		}
	}
	else {
		if (loadingIconWindow) {
			cardView->removeRowItems (LoadingIconRow);
			loadingIconWindowHandle.clear ();
		}
	}
}

void PlayerUi::mediaSearchRecordsAdded (void *itPtr, const StringList &recordIds) {
	PlayerUi *it = (PlayerUi *) itPtr;

	SDL_LockMutex (it->syncRecordMutex);
	it->searchSyncRecordIds.append (recordIds);
	it->isSearchSyncRecordWaiting = true;
	SDL_UnlockMutex (it->syncRecordMutex);
	App::instance->shouldSyncRecordStore = true;
}
void PlayerUi::mediaSearchRecordsRemoved (void *itPtr, const StringList &recordIds) {
	PlayerUi *it = (PlayerUi *) itPtr;
	StringList::const_iterator i1, i2;
	StdString id;

	i1 = recordIds.cbegin ();
	i2 = recordIds.cend ();
	while (i1 != i2) {
		id = *i1;
		if (it->cardView->contains (id)) {
			it->cardView->removeItem (id);
			it->loadedRecordIds.remove (id);
			it->selectedMediaMap.remove (id);
			RecordStore::instance->remove (id);
		}
		++i1;
	}
	it->cardView->reflow ();
}

void PlayerUi::resetSearch () {
	MediaSearchGroup::instance->resetSearch (searchKey, mediaSortOrder);
	mediaDisplayCount = 0;

	if (playerUiOptions.showPlayHistory) {
		cardView->removeRowItems (PlayHistoryDetailRow);
		cardView->removeRowItems (PlayHistoryImageRow);
		SDL_LockMutex (syncRecordMutex);
		playHistorySyncRecordIds.append (playHistoryRecordIds);
		SDL_UnlockMutex (syncRecordMutex);
		App::instance->shouldSyncRecordStore = true;
	}
}

void PlayerUi::searchFieldEdited (void *itPtr, Widget *widgetPtr) {
	PlayerUi::searchButtonClicked (itPtr, NULL);
}
void PlayerUi::searchButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (! MediaControl::instance->isReady) {
		return;
	}
	it->unselectAllMedia ();
	it->searchKey.assign (it->searchField->getValue ());
	it->resetSearch ();
	it->clearPopupWidgets ();
}

void PlayerUi::mediaControlPlayHistoryRecordAdded (void *itPtr, const StdString &recordId) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->playHistoryRecordIds.push_back (recordId);
	if (it->playerUiOptions.showPlayHistory) {
		SDL_LockMutex (it->syncRecordMutex);
		it->playHistorySyncRecordIds.append (recordId);
		SDL_UnlockMutex (it->syncRecordMutex);
		App::instance->shouldSyncRecordStore = true;
		it->resetPlayHistoryCount ();
	}
}
void PlayerUi::mediaControlPlayHistoryRecordRemoved (void *itPtr, const StdString &recordId) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->playHistoryRecordIds.remove (recordId);
	it->cardView->removeItem (recordId);
	it->selectedMediaMap.remove (recordId);
	it->resetPlayHistoryCount ();
}

bool PlayerUi::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isControlDown && (keycode == fileMediaOpenKeycode)) {
		if (fileMediaOpenButton) {
			fileMediaOpenButton->mouseClick ();
			if (playFileActionWindow) {
				playFileActionWindow->mouseClickFsBrowseButton ();
			}
			return (true);
		}
	}
	return (false);
}

void PlayerUi::clearPlayHistoryButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->playHistoryRecordIds.empty ()) {
		return;
	}
	MediaControl::instance->clearPlayHistory ();
}

void PlayerUi::selectAllButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::SelectAllPrompt));
}

static void selectAllButtonClicked_processItems (void *boolPtr, Widget *itemWidget) {
	MediaItemWindow *media;

	media = MediaItemWindow::castWidget (itemWidget);
	if (media) {
		media->setSelected (*((bool *) boolPtr));
	}
}
void PlayerUi::selectAllButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	bool selected;

	selected = it->selectedMediaMap.empty ();
	it->cardView->processItems (selectAllButtonClicked_processItems, &selected);
	it->cardView->reflow ();
}

void PlayerUi::playerMenuButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	Menu *menu;

	UiStack::instance->suspendMouseHover ();
	if (it->clearActionPopup (widgetPtr, PlayerUi::playerMenuButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextId::ShowGrid).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_layoutButton), Widget::EventCallbackContext (PlayerUi::boxLayoutActionClicked, it), 1, it->mediaWindowMode == ImageGridWindowMode);
	menu->addItem (UiText::instance->getText (UiTextId::ShowLines).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_layoutButton), Widget::EventCallbackContext (PlayerUi::lineLayoutActionClicked, it), 1, it->mediaWindowMode == DetailLineWindowMode);
	menu->addItem (UiText::instance->getText (UiTextId::SortByName).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_sortButton), Widget::EventCallbackContext (PlayerUi::sortByNameActionClicked, it), 2, it->mediaSortOrder == SystemInterface::Constant_NameSort);
	menu->addItem (UiText::instance->getText (UiTextId::SortByNewest).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_sortButton), Widget::EventCallbackContext (PlayerUi::sortByNewestActionClicked, it), 2, it->mediaSortOrder == SystemInterface::Constant_NewestSort);
	menu->addItem (UiText::instance->getText (UiTextId::SortByFilePath).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_sortButton), Widget::EventCallbackContext (PlayerUi::sortByFilePathActionClicked, it), 2, it->mediaSortOrder == SystemInterface::Constant_FilePathSort);

	it->showActionPopup (menu, widgetPtr, PlayerUi::playerMenuButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
}

void PlayerUi::boxLayoutActionClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->setMediaItemWindowMode (ImageGridWindowMode);
}
void PlayerUi::lineLayoutActionClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->setMediaItemWindowMode (DetailLineWindowMode);
}
void PlayerUi::setMediaItemWindowMode (int mode) {
	if (mediaWindowMode == mode) {
		return;
	}
	mediaWindowMode = mode;
	if (mediaWindowMode == ImageGridWindowMode) {
		cardView->removeRowItems (MediaFilescanDetailRow);
	}
	else {
		cardView->removeRowItems (MediaFilescanImageRow);
	}
	resetSearch ();
}

void PlayerUi::sortByNameActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->mediaSortOrder == SystemInterface::Constant_NameSort) {
		return;
	}
	it->mediaSortOrder = SystemInterface::Constant_NameSort;
	it->resetSearch ();
}
void PlayerUi::sortByNewestActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->mediaSortOrder == SystemInterface::Constant_NewestSort) {
		return;
	}
	it->mediaSortOrder = SystemInterface::Constant_NewestSort;
	it->resetSearch ();
}
void PlayerUi::sortByFilePathActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (it->mediaSortOrder == SystemInterface::Constant_FilePathSort) {
		return;
	}
	it->mediaSortOrder = SystemInterface::Constant_FilePathSort;
	it->resetSearch ();
}

void PlayerUi::setSortKey (MediaPlaylistWindow *mediaPlaylist, int64_t sequenceValue) {
	if (mediaPlaylist->isExpanded) {
		mediaPlaylist->sortKey.sprintf ("%016llx%s", (long long int) ((sequenceValue > 0) ? sequenceValue : OsUtil::getTime ()), mediaPlaylist->playlist.name.lowercased ().c_str ());
	}
	else {
		mediaPlaylist->sortKey.assign (mediaPlaylist->playlist.name.lowercased ().c_str ());
	}
}

void PlayerUi::handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaItemWindow *mediaitem;

	mediaitem = MediaItemWindow::castWidget (itemWidget);
	if (mediaitem) {
		mediaitem->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}
void PlayerUi::handleDetailImageSizeChange () {
	cardView->processItems (PlayerUi::handleDetailImageSizeChange_processItems, this, true);
}

MediaPlaylistWindow *PlayerUi::createMediaPlaylistWindow () {
	MediaPlaylistWindow *playlist;

	playlist = new MediaPlaylistWindow ();
	playlist->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::playlistExpandStateChanged, this);
	playlist->renameClickCallback = Widget::EventCallbackContext (PlayerUi::playlistRenameActionClicked, this);
	playlist->removeClickCallback = Widget::EventCallbackContext (PlayerUi::playlistRemoveActionClicked, this);
	playlist->addItemClickCallback = Widget::EventCallbackContext (PlayerUi::playlistAddItemActionClicked, this);
	playlist->addItemFocusCallback = Widget::EventCallbackContext (PlayerUi::playlistAddItemFocused, this);
	playlist->addItemUnfocusCallback = Widget::EventCallbackContext (Ui::toolPopupSourceUnfocused, this);
	playlist->optionChangeCallback = Widget::EventCallbackContext (PlayerUi::playlistOptionChanged, this);
	playlist->editClickCallback = Widget::EventCallbackContext (PlayerUi::playlistEditActionClicked, this);
	playlist->playClickCallback = Widget::EventCallbackContext (PlayerUi::playlistPlayActionClicked, this);
	return (playlist);
}

static void getAvailablePlaylistName_matchName (void *stringPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;
	StdString *name;

	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		name = (StdString *) stringPtr;
		if (name->lowercased ().equals (playlist->playlist.name.lowercased ())) {
			name->assign ("");
		}
	}
}
StdString PlayerUi::getAvailablePlaylistName (const StdString &baseName) {
	StdString base, name;
	int i;

	if (baseName.empty ()) {
		base.assign (UiText::instance->getText (UiTextId::Playlist).capitalized ());
	}
	else {
		base.assign (baseName);
	}
	name.assign (base);
	cardView->processItems (getAvailablePlaylistName_matchName, &name);
	if (name.empty ()) {
		i = 2;
		while (true) {
			name.sprintf ("%s %i", base.c_str (), i);
			cardView->processItems (getAvailablePlaylistName_matchName, &name);
			if (! name.empty ()) {
				break;
			}
			++i;
		}
	}
	return (name);
}

void PlayerUi::mediaItemWindowImageClicked (void *itPtr, Widget *widgetPtr) {
	MediaItemWindow *mediaitem;

	mediaitem = (MediaItemWindow *) widgetPtr;
	mediaitem->setSelected (! mediaitem->isSelected);
}

void PlayerUi::mediaItemWindowViewButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaItemWindow *mediaitem = (MediaItemWindow *) widgetPtr;
	MediaItemUi *mediaitemui;

	it->targetMediaItemWindowHandle.assign (mediaitem);
	mediaitemui = new MediaItemUi (mediaitem);
	mediaitemui->endCallback = Ui::EventCallbackContext (PlayerUi::mediaItemUiEnded, it);
	UiStack::instance->pushUi (mediaitemui);
}
void PlayerUi::mediaItemUiEnded (void *itPtr, Ui *uiPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaItemUi *ui = (MediaItemUi *) uiPtr;

	if (it->targetMediaItemWindow) {
		if (ui->playTimestamp >= 0) {
			it->mediaItemUiPlayId.assign (ui->mediaId);
			it->mediaItemUiPlayTimestamp = ui->playTimestamp;
			it->retain ();
			App::instance->addUpdateTask (PlayerUi::playMediaItemUiTarget, it);
		}
		else if (ui->selectPlayPositionTimestamp >= 0) {
			it->targetMediaItemWindow->setPlayTimestamp (ui->selectPlayPositionTimestamp);
			it->cardView->resetItemLabels ();
			it->cardView->reflow ();
		}
		it->targetMediaItemWindowHandle.clear ();
	}
}
void PlayerUi::playMediaItemUiTarget (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if ((! it->mediaItemUiPlayId.empty ()) && (it->mediaItemUiPlayTimestamp >= 0)) {
		if (UiStack::instance->getPlayerCount () >= PlayerControl::maxPlayerCount) {
			App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiMaxPlayerCountError));
		}
		else {
			UiStack::instance->playMediaItem (it->mediaItemUiPlayId, it->mediaItemUiPlayTimestamp);
		}
	}
	it->mediaItemUiPlayId.assign ("");
	it->mediaItemUiPlayTimestamp = 0;
	it->release ();
}

void PlayerUi::mediaItemWindowCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel) {
	MediaItemWindow *mediaitem;

	mediaitem = MediaItemWindow::castWidget (itemWidget);
	if (mediaitem) {
		cardLabel->setMainText (mediaitem->mediaItem.name);
		if (mediaitem->mediaItem.playSeekTimestamp <= 0) {
			cardLabel->setLeftText (StdString ());
		}
		else {
			cardLabel->setLeftText (UiText::instance->getTimespanText (mediaitem->mediaItem.playSeekTimestamp, UiText::HoursUnit, true), UiConfiguration::instance->darkInverseTextColor, UiConfiguration::instance->mediumPrimaryColor);
		}
	}
}

void PlayerUi::mediaItemWindowSelectStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaItemWindow *mediaitem = (MediaItemWindow *) widgetPtr;
	StdString mediaid;
	HashMap::Iterator i;

	if (mediaitem->isSelected) {
		it->selectedMediaMap.insert (mediaitem->mediaId, mediaitem->mediaItem.name);
		it->lastSelectedMediaItemWindowHandle.assign (mediaitem);
	}
	else {
		it->selectedMediaMap.remove (mediaitem->mediaId);
		if (it->selectedMediaMap.empty ()) {
			it->lastSelectedMediaItemWindowHandle.clear ();
		}
		else {
			if (it->lastSelectedMediaItemWindowHandle.equals (mediaitem)) {
				i = it->selectedMediaMap.begin ();
				if (it->selectedMediaMap.next (&i, &mediaid)) {
					it->lastSelectedMediaItemWindowHandle.assign (MediaItemWindow::castWidget (it->cardView->getItem (mediaid)));
				}
				else {
					it->lastSelectedMediaItemWindowHandle.clear ();
				}
			}
		}
	}
	it->clearPopupWidgets ();
}

static void expandPlaylistsToggleStateChanged_appendPlaylistId (void *stringListPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;

	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		((StringList *) stringListPtr)->push_back (playlist->playlist.id);
	}
}
void PlayerUi::expandPlaylistsToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;
	MediaPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i1, i2;
	int64_t now;

	now = OsUtil::getTime ();
	it->cardView->processItems (expandPlaylistsToggleStateChanged_appendPlaylistId, &idlist);
	i1 = idlist.begin ();
	i2 = idlist.end ();
	while (i1 != i2) {
		playlist = MediaPlaylistWindow::castWidget (it->cardView->getItem (*i1));
		if (playlist) {
			playlist->setExpanded (! toggle->isChecked, true);
			it->setSortKey (playlist, now);
			it->cardView->setItemRow (playlist->playlist.id, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow, true);
			playlist->writeRecord ();
		}
		++i1;
	}
	it->cardView->reflow ();
}

void PlayerUi::appCardExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	AppCardWindow *window = (AppCardWindow *) widgetPtr;

	window->resetInputState ();
	it->cardView->animateItemScaleBump (window->itemId);
	it->clearPopupWidgets ();
	it->cardView->setItemRow (window->itemId, window->isExpanded ? ExpandedControlRow : UnexpandedControlRow, true);
	it->cardView->reflow ();
}
void PlayerUi::appCardLayoutChanged (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->cardView->reflow ();
}

void PlayerUi::mediaFilescanWindowExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaFilescanWindow *window = (MediaFilescanWindow *) widgetPtr;

	window->resetInputState ();
	it->cardView->animateItemScaleBump (window->itemId);
	it->resetExpandToggles ();
	it->clearPopupWidgets ();
	it->cardView->setItemRow (window->itemId, window->isExpanded ? ExpandedControlRow : UnexpandedControlRow, true);
	it->cardView->reflow ();
}
void PlayerUi::mediaFilescanWindowLayoutChanged (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->cardView->reflow ();
}

void PlayerUi::playlistExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;

	it->setSortKey (playlist);
	it->cardView->setItemRow (playlist->playlist.id, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow, true);
	playlist->resetInputState ();
	playlist->writeRecord ();
	it->cardView->animateItemScaleBump (playlist->playlist.id);
	it->resetExpandToggles ();
	it->clearPopupWidgets ();
	it->cardView->reflow ();
}

void PlayerUi::playlistOptionChanged (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistWindow *) widgetPtr)->writeRecord ();
}

void PlayerUi::playlistRenameActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	TextFieldWindow *textfield;

	it->clearPopupWidgets ();
	textfield = new TextFieldWindow (playlist->windowWidth, UiText::instance->getText (UiTextId::EnterPlaylistNamePrompt));
	textfield->widgetName.assign ("playlistRenameTextField");
	textfield->setValue (playlist->playlist.name);
	textfield->valueEditCallback = Widget::EventCallbackContext (PlayerUi::playlistNameEdited, it);
	textfield->enterButtonClickCallback = Widget::EventCallbackContext (PlayerUi::playlistNameEditEnterButtonClicked, it);
	textfield->setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
	textfield->setButtonsEnabled (TextFieldWindow::EnterButtonOption | TextFieldWindow::CancelButtonOption | TextFieldWindow::PasteButtonOption | TextFieldWindow::ClearButtonOption);
	textfield->shouldSkipTextClearCallbacks = true;
	textfield->assignKeyFocus ();

	it->showActionPopup (textfield, widgetPtr, PlayerUi::playlistRenameActionClicked, widgetPtr->getScreenRect (), Ui::LeftEdgeAlignment, Ui::TopEdgeAlignment);
}
void PlayerUi::playlistNameEdited (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist;
	TextFieldWindow *textfield;

	textfield = TextFieldWindow::castWidget (it->actionWidget);
	playlist = MediaPlaylistWindow::castWidget (it->actionTarget);
	if ((! textfield) || (! playlist)) {
		return;
	}
	playlist->setPlaylistName (textfield->getValue ());
	it->setSortKey (playlist);
	playlist->writeRecord ();
	it->clearPopupWidgets ();
	it->cardView->reflow ();
}

void PlayerUi::playlistNameEditEnterButtonClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->clearPopupWidgets ();
}

void PlayerUi::playlistRemoveActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	ActionWindow *action;

	if (it->clearActionPopup (playlist, PlayerUi::playlistRemoveActionClicked)) {
		return;
	}
	action = new ActionWindow ();
	action->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	action->setInverseColor (true);
	action->setTitleText (UiConfiguration::instance->fonts[ActionWindow::titleTextFont]->truncatedText (playlist->playlist.name, playlist->width * 0.34f, Font::dotTruncateSuffix));
	action->setDescriptionText (UiText::instance->getText (UiTextId::RemovePlaylistDescription));
	action->closeCallback = Widget::EventCallbackContext (PlayerUi::removePlaylistActionClosed, it);
	action->setWidgetNames (StdString ("playlistRemoveActionWindow"));

	it->showActionPopup (action, playlist, PlayerUi::playlistRemoveActionClicked, playlist->getRemoveButtonScreenRect (), Ui::LeftEdgeAlignment, Ui::TopOfAlignment);
}
void PlayerUi::removePlaylistActionClosed (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	ActionWindow *action = (ActionWindow *) widgetPtr;
	MediaPlaylistWindow *playlist;

	if (! action->isConfirmed) {
		return;
	}
	playlist = MediaPlaylistWindow::castWidget (it->actionTarget);
	if (! playlist) {
		return;
	}
	UiStack::instance->stopPlaylist (playlist->playlist.id);
	playlist->removeRecord ();
	it->cardView->removeItem (playlist->playlist.id);
	it->resetExpandToggles ();
}

void PlayerUi::playlistAddItemActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	MediaItemWindow *mediaitem;
	HashMap::Iterator i;
	StdString mediaid;
	int count;

	if (it->selectedMediaMap.empty () || it->getSelectedMediaNames ().empty ()) {
		return;
	}
	count = 0;
	i = it->selectedMediaMap.begin ();
	while (it->selectedMediaMap.next (&i, &mediaid)) {
		mediaitem = MediaItemWindow::castWidget (it->cardView->getItem (mediaid));
		if (mediaitem) {
			playlist->addItem (mediaitem->mediaItem);
			++count;
		}
	}
	if (count > 0) {
		playlist->writeRecord ();
	}

	it->clearPopupWidgets ();
	if (count <= 0) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::InternalError));
	}
	else {
		if (count == 1) {
			App::instance->showNotification (StdString::createSprintf ("%s: %s", playlist->playlist.name.c_str (), UiText::instance->getText (UiTextId::AddedPlaylistItem).c_str ()));
		}
		else {
			App::instance->showNotification (StdString::createSprintf ("%s: %s (%i)", playlist->playlist.name.c_str (), UiText::instance->getText (UiTextId::AddedPlaylistItems).c_str (), count));
		}
		it->cardView->reflow ();
	}
}

void PlayerUi::playlistAddItemFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	IconLabelWindow *icon1, *icon2;
	StdString text;
	Color color;

	text.assign (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (playlist->playlist.name, App::instance->rootPanel->width * 0.20f, Font::dotTruncateSuffix));
	icon1 = it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallPlaylistIcon), text, color);

	text.assign (it->getSelectedMediaNames ());
	color.assign (UiConfiguration::instance->primaryTextColor);
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::PlayerUiNoMediaSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	icon2 = it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallMediaIcon), text, color);

	it->showToolPopup (playlist, UiText::instance->getText (UiTextId::AddPlaylistItems).capitalized (), icon1, icon2);
}

void PlayerUi::playlistEditActionClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;
	MediaPlaylistUi *playlistui;

	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		playlistui = new MediaPlaylistUi (playlist);
		playlistui->endCallback = Ui::EventCallbackContext (PlayerUi::mediaPlaylistUiEnded, itPtr);
		UiStack::instance->pushUi (playlistui);
	}
}
void PlayerUi::mediaPlaylistUiEnded (void *itPtr, Ui *uiPtr) {
	((MediaPlaylistUi *) uiPtr)->playlist->writeRecord ();
}

void PlayerUi::playlistPlayActionClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;

	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		if (UiStack::instance->getPlayerCount () >= PlayerControl::maxPlayerCount) {
			App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiMaxPlayerCountError));
		}
		else {
			UiStack::instance->playPlaylist (playlist);
		}
	}
}

void PlayerUi::playButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	StdString text;
	Color color;

	text.assign (it->getSelectedMediaNames ());
	color.assign (UiConfiguration::instance->primaryTextColor);
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::PlayerUiNoMediaSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::Play).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallMediaIcon), text, color));
}

void PlayerUi::playButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->clearPopupWidgets ();
	it->playSelectedMedia ();
}
void PlayerUi::playSelectedMedia () {
	StringList keys;
	StringList::const_iterator i1, i2;
	StdString id;
	MediaItemWindow *mediaitem;
	int playercount;
	bool detached;

	playercount = UiStack::instance->getPlayerCount ();
	if (playercount >= PlayerControl::maxPlayerCount) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiMaxPlayerCountError));
		return;
	}
	detached = false;
	if (lastSelectedMediaItemWindow) {
		UiStack::instance->playMediaItem (lastSelectedMediaItemWindow->mediaId, lastSelectedMediaItemWindow->mediaItem.playSeekTimestamp > 0 ? lastSelectedMediaItemWindow->mediaItem.playSeekTimestamp : 0, false);
		selectedMediaMap.remove (lastSelectedMediaItemWindow->mediaId);
		lastSelectedMediaItemWindow->setSelected (false, true);
		lastSelectedMediaItemWindowHandle.clear ();
		detached = true;
		playercount = UiStack::instance->getPlayerCount ();
	}

	selectedMediaMap.getKeys (&keys, true);
	i1 = keys.cbegin ();
	i2 = keys.cend ();
	while (i1 != i2) {
		id = *i1;
		mediaitem = MediaItemWindow::castWidget (cardView->getItem (id));
		if (mediaitem) {
			if (playercount >= PlayerControl::maxPlayerCount) {
				break;
			}
			UiStack::instance->playMediaItem (mediaitem->mediaId, mediaitem->mediaItem.playSeekTimestamp > 0 ? mediaitem->mediaItem.playSeekTimestamp : 0, detached);
			selectedMediaMap.remove (id);
			mediaitem->setSelected (false, true);
			detached = true;
			playercount = UiStack::instance->getPlayerCount ();
		}
		++i1;
	}
}

void PlayerUi::pauseButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	int paused, unpaused;
	StdString title, text;
	Color color;

	paused = UiStack::instance->getPausedPlayerCount ();
	unpaused = UiStack::instance->getUnpausedPlayerCount ();
	if (unpaused > 0) {
		title = UiText::instance->getText (UiTextId::Pause).capitalized ();
		text = UiText::instance->getCountText (unpaused, UiTextId::VideoPlayer, UiTextId::VideoPlayers);
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	else if (paused > 0) {
		title = UiText::instance->getText (UiTextId::Resume).capitalized ();
		text = UiText::instance->getCountText (paused, UiTextId::VideoPlayer, UiTextId::VideoPlayers);
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	else {
		title.sprintf ("%s / %s", UiText::instance->getText (UiTextId::Pause).capitalized ().c_str (), UiText::instance->getText (UiTextId::Resume).c_str ());
		text = UiText::instance->getText (UiTextId::PlayerUiNoPlayersPrompt);
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	it->showToolPopup (widgetPtr, title, it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}
void PlayerUi::pauseButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->clearPopupWidgets ();
	UiStack::instance->pausePlayers ();
}

void PlayerUi::stopButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	int count;
	StdString text;
	Color color;

	count = UiStack::instance->getPlayerCount ();
	if (count <= 0) {
		text = UiText::instance->getText (UiTextId::PlayerUiNoPlayersPrompt);
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text = UiText::instance->getCountText (count, UiTextId::VideoPlayer, UiTextId::VideoPlayers);
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::Stop).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

void PlayerUi::stopButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->clearPopupWidgets ();
	UiStack::instance->stopPlayers ();
}

void PlayerUi::fullscreenButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	int count;
	StdString text;
	Color color;

	count = UiStack::instance->getPlayerCount ();
	if (count <= 0) {
		text = UiText::instance->getText (UiTextId::PlayerUiNoPlayersPrompt);
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text = UiText::instance->getCountText (count, UiTextId::VideoPlayer, UiTextId::VideoPlayers);
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::FillWindow).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}
void PlayerUi::fullscreenButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->clearPopupWidgets ();
	if (UiStack::instance->getPlayerCount () <= 0) {
		return;
	}
	UiStack::instance->fullscreenPlayers ();
}

void PlayerUi::tagButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	StdString text;
	Color color;

	text.assign (it->getSelectedMediaNames (true));
	color.assign (UiConfiguration::instance->primaryTextColor);
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::PlayerUiNoTagMediaSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::PlayerUiTagActionPrompt), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallMediaIcon), text, color));
}

void PlayerUi::tagButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	TagActionWindow *window;
	StdString text;

	if (it->clearActionPopup (widgetPtr, PlayerUi::tagButtonClicked)) {
		return;
	}
	text.assign (it->getSelectedMediaNames (true));
	if (text.empty ()) {
		return;
	}
	UiStack::instance->suspendMouseHover ();
	window = new TagActionWindow ();
	window->addClickCallback = Widget::EventCallbackContext (PlayerUi::tagActionWindowAddClicked, it);
	window->removeClickCallback = Widget::EventCallbackContext (PlayerUi::tagActionWindowRemoveClicked, it);
	window->setMediaText (text);
	it->showActionPopup (window, widgetPtr, PlayerUi::tagButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::TopOfAlignment);
	window->assignKeyFocus ();
}

void PlayerUi::tagActionWindowAddClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	TagActionWindow *window = (TagActionWindow *) widgetPtr;
	PlayerUi::TagTask *task;
	StringList keys;

	it->getSelectedTagEnabledMediaIds (&keys);
	if (keys.empty ()) {
		return;
	}
	task = new PlayerUi::TagTask ();
	task->ui = it;
	task->tag.assign (window->tag);
	task->mediaItemIds.assign (keys);
	task->ui->retain ();
	App::instance->setUiActive ();
	TaskGroup::instance->run (TaskGroup::RunContext (PlayerUi::addTags, task, App::databaseWriteQueueId));
	it->clearPopupWidgets ();
}

void PlayerUi::tagActionWindowRemoveClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	TagActionWindow *window = (TagActionWindow *) widgetPtr;
	PlayerUi::TagTask *task;
	StringList keys;

	it->getSelectedTagEnabledMediaIds (&keys);
	if (keys.empty ()) {
		return;
	}
	task = new PlayerUi::TagTask ();
	task->ui = it;
	task->tag.assign (window->tag);
	task->mediaItemIds.assign (keys);
	task->ui->retain ();
	App::instance->setUiActive ();
	TaskGroup::instance->run (TaskGroup::RunContext (PlayerUi::removeTags, task, App::databaseWriteQueueId));
	it->clearPopupWidgets ();
}

void PlayerUi::addTags (void *taskPtr) {
	PlayerUi::TagTask *task = (PlayerUi::TagTask *) taskPtr;

	task->ui->executeAddTags (task);
	task->ui->release ();
	delete (task);
	App::instance->unsetUiActive ();
}
void PlayerUi::executeAddTags (PlayerUi::TagTask *task) {
	StdString sql, dbpath, errmsg;
	OpResult result;
	StringList::const_iterator i1, i2;
	MediaItem mediaitem;
	Json *record;
	bool err;

	if (task->tag.empty () || task->mediaItemIds.empty ()) {
		return;
	}
	err = false;
	i1 = task->mediaItemIds.cbegin ();
	i2 = task->mediaItemIds.cend ();
	while (i1 != i2) {
		if (mediaitem.readRecordStore (*i1)) {
			if (! mediaitem.tags.contains (task->tag)) {
				mediaitem.tags.push_back (task->tag);
				sql = MediaItem::getUpdateTagsSql (MediaControl::filescanTableName, mediaitem.id, mediaitem.tags);
				result = MediaControl::instance->openDatabase (&dbpath);
				if (result == OpResult::Success) {
					result = Database::instance->exec (dbpath, sql, &errmsg);
					Database::instance->close (dbpath);
				}
				if (result != OpResult::Success) {
					err = true;
				}
				else {
					record = mediaitem.createRecord (MediaControl::instance->agentId);
					RecordStore::instance->insert (record);
					delete (record);
				}
			}
		}
		++i1;
	}

	if (err) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiTagRecordUpdateError));
	}
	else {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiAddTagCompleteText));
	}
	App::instance->shouldSyncRecordStore = true;
}

void PlayerUi::removeTags (void *taskPtr) {
	PlayerUi::TagTask *task = (PlayerUi::TagTask *) taskPtr;

	task->ui->executeRemoveTags (task);
	task->ui->release ();
	delete (task);
	App::instance->unsetUiActive ();
}
void PlayerUi::executeRemoveTags (PlayerUi::TagTask *task) {
	StdString sql, dbpath, errmsg;
	OpResult result;
	StringList::const_iterator i1, i2;
	MediaItem mediaitem;
	Json *record;
	bool err;

	if (task->tag.empty () || task->mediaItemIds.empty ()) {
		return;
	}
	err = false;
	i1 = task->mediaItemIds.cbegin ();
	i2 = task->mediaItemIds.cend ();
	while (i1 != i2) {
		if (mediaitem.readRecordStore (*i1)) {
			if (mediaitem.tags.contains (task->tag)) {
				mediaitem.tags.remove (task->tag);
				sql = MediaItem::getUpdateTagsSql (MediaControl::filescanTableName, mediaitem.id, mediaitem.tags);
				result = MediaControl::instance->openDatabase (&dbpath);
				if (result == OpResult::Success) {
					result = Database::instance->exec (dbpath, sql, &errmsg);
					Database::instance->close (dbpath);
				}
				if (result != OpResult::Success) {
					err = true;
				}
				else {
					record = mediaitem.createRecord (MediaControl::instance->agentId);
					RecordStore::instance->insert (record);
					delete (record);
				}
			}
		}
		++i1;
	}

	if (err) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiTagRecordUpdateError));
	}
	else {
		App::instance->showNotification (UiText::instance->getText (UiTextId::PlayerUiRemoveTagCompleteText));
	}
	App::instance->shouldSyncRecordStore = true;
}

void PlayerUi::createPlaylistButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist;

	playlist = it->createMediaPlaylistWindow ();
	playlist->resetPlaylistId ();
	playlist->setPlaylistName (it->getAvailablePlaylistName ());
	playlist->setExpanded (true);
	it->cardView->addItem (playlist, playlist->playlist.id, ExpandedPlaylistRow);
	it->cardView->animateItemScaleBump (playlist->playlist.id);

	it->cardView->scrollToItem (playlist->playlist.id);
	it->cardView->reflow ();
	it->resetExpandToggles ();
	App::instance->showNotification (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CreatedPlaylist).capitalized ().c_str (), playlist->playlist.name.c_str ()));

	playlist->writeRecord ();
}

void PlayerUi::loadMediaPlaylists (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->executeLoadMediaPlaylists ();
	it->isLoadMediaPlaylistsComplete = true;
	it->release ();
}
void PlayerUi::executeLoadMediaPlaylists () {
	StdString dbpath, errmsg;

	mediaPlaylists.clear ();
	if (MediaControl::instance->openDatabase (&dbpath) != OpResult::Success) {
		return;
	}
	if (! MediaPlaylist::readDatabaseRows (dbpath, MediaControl::playlistTableName, &errmsg, &mediaPlaylists)) {
		mediaPlaylists.clear ();
		Log::debug ("Failed to read playlist data; err=%s", errmsg.c_str ());
	}
	Database::instance->close (dbpath);
}

void PlayerUi::awaitLoadMediaPlaylistsComplete (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	if (App::instance->isShuttingDown || App::instance->isShutdown) {
		it->release ();
		return;
	}
	if (! it->isLoadMediaPlaylistsComplete) {
		App::instance->addUpdateTask (PlayerUi::awaitLoadMediaPlaylistsComplete, it);
		return;
	}
	it->populateMediaPlaylistWindows ();
	App::instance->unsetUiActive ();
	it->release ();
}
void PlayerUi::populateMediaPlaylistWindows () {
	std::list<MediaPlaylist>::const_iterator i1, i2;
	MediaPlaylistWindow *playlist;
	int64_t seq;

	cardView->removeRowItems (ExpandedPlaylistRow);
	cardView->removeRowItems (UnexpandedPlaylistRow);
	seq = 1;
	i1 = mediaPlaylists.cbegin ();
	i2 = mediaPlaylists.cend ();
	while (i1 != i2) {
		playlist = createMediaPlaylistWindow ();
		playlist->read (*i1);
		setSortKey (playlist, seq);
		cardView->addItem (playlist, playlist->playlist.id, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow);
		cardView->animateItemScaleBump (playlist->playlist.id);

		++seq;
		++i1;
	}
	mediaPlaylists.clear ();
}

StdString PlayerUi::getSelectedMediaNames (bool requireTagEnabled) {
	StdString id;
	HashMap::Iterator i;
	MediaItemWindow *mediaitem;
	StringList names;

	i = selectedMediaMap.begin ();
	while (selectedMediaMap.next (&i, &id)) {
		mediaitem = MediaItemWindow::castWidget (cardView->getItem (id));
		if (mediaitem) {
			if (requireTagEnabled && (! mediaitem->isTagEnabled)) {
				continue;
			}
			names.push_back (selectedMediaMap.find (id, ""));
		}
	}
	if (names.empty ()) {
		return (StdString ());
	}
	names.sort (StringList::compareCaseInsensitiveAscending);
	return (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (names.join (", "), App::instance->rootPanel->width * textTruncateWidthScale, StdString::createSprintf ("... (%i)", (int) names.size ())));
}

void PlayerUi::getSelectedTagEnabledMediaIds (StringList *destList) {
	StringList keys;
	StringList::const_iterator i1, i2;
	MediaItemWindow *mediaitem;
	StdString id;

	selectedMediaMap.getKeys (&keys);
	i1 = keys.cbegin ();
	i2 = keys.cend ();
	while (i1 != i2) {
		id = *i1;
		mediaitem = MediaItemWindow::castWidget (cardView->getItem (id));
		if (mediaitem && mediaitem->isTagEnabled) {
			destList->push_back (id);
		}
		++i1;
	}
}

void PlayerUi::unselectAllMedia () {
	StringList keys;
	StringList::const_iterator i1, i2;
	MediaItemWindow *mediaitem;
	StdString id;

	selectedMediaMap.getKeys (&keys);
	i1 = keys.cbegin ();
	i2 = keys.cend ();
	while (i1 != i2) {
		id = *i1;
		mediaitem = MediaItemWindow::castWidget (cardView->getItem (id));
		if (mediaitem) {
			mediaitem->setSelected (false, true);
		}
		++i1;
	}
	selectedMediaMap.clear ();
	lastSelectedMediaItemWindowHandle.clear ();
}

static void resetExpandToggles_countExpandedPlaylists (void *intPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;
	int *count;

	count = (int *) intPtr;
	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist && playlist->isExpanded) {
		++(*count);
	}
}
void PlayerUi::resetExpandToggles () {
	int count;

	if (expandPlaylistsToggle) {
		count = 0;
		cardView->processItems (resetExpandToggles_countExpandedPlaylists, &count);
		expandPlaylistsToggle->setChecked ((count <= 0), true);
	}
}

void PlayerUi::resetPlayHistoryCount () {
	int count, historysize;

	if (playHistoryHeaderPanel && playHistoryCountLabel) {
		historysize = MediaControl::instance->getPlayHistorySize ();
		if (historysize <= 0) {
			playHistoryCountLabel->setText (UiText::instance->getText (UiTextId::PlayerUiEmptyPlayHistoryPrompt));
		}
		else {
			count = cardView->getRowItemCount (PlayHistoryImageRow) + cardView->getRowItemCount (PlayHistoryDetailRow);
			if (count < historysize) {
				playHistoryCountLabel->setText (StdString::createSprintf ("%s (%i / %i)", UiText::instance->getText (UiTextId::PlayedItems).capitalized ().c_str (), count, historysize));
			}
			else {
				playHistoryCountLabel->setText (StdString::createSprintf ("%s (%i)", UiText::instance->getText (UiTextId::PlayedItems).capitalized ().c_str (), historysize));
			}
		}
		playHistoryHeaderPanel->reflow ();
		cardView->reflow ();
	}
}

void PlayerUi::syncSearchRecords () {
	if (searchRecordSyncClock < 0) {
		searchRecordSyncClock = UiConfiguration::instance->recordSyncDelayDuration;
	}
}

int PlayerUi::getSelectedMediaCount () {
	return ((int) selectedMediaMap.size ());
}

void PlayerUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextId::PlayerUiHelpTitle), UiText::instance->getText (UiTextId::PlayerUiHelpText));
	if (! MediaControl::instance->mainOptions.mediaScan) {
		helpWindow->addAction (UiText::instance->getText (UiTextId::PlayerUiHelpAction1Text), HelpWindow::InfoAction);
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextId::PlayerUiHelpAction2Text), HelpWindow::InfoAction);
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextId::MediaPlayerInterface).capitalized (), StdString (AppUrl::MediaPlayerInterface));
	helpWindow->addTopicLink (UiText::instance->getText (UiTextId::MediaPlayerWindow).capitalized (), StdString (AppUrl::MediaPlayerWindow));
}

static bool findItem_matchMediaName (void *data, Widget *widget) {
	MediaItemWindow *mediaitem;

	mediaitem = MediaItemWindow::castWidget (widget);
	return (mediaitem && mediaitem->mediaItem.name.lowercased ().equals ((char *) data));
}
static bool findItem_matchPlaylistName (void *data, Widget *widget) {
	MediaPlaylistWindow *playlist;

	playlist = MediaPlaylistWindow::castWidget (widget);
	return (playlist && playlist->playlist.name.lowercased ().equals ((char *) data));
}
Widget *PlayerUi::findLuaOpenWidget (const char *targetName) {
	StdString name;
	Widget *widget;

	name.assign (targetName);
	name.lowercase ();
	widget = cardView->findItem (findItem_matchMediaName, (char *) name.c_str (), true);
	if (widget) {
		return (widget);
	}
	widget = cardView->findItem (findItem_matchPlaylistName, (char *) name.c_str (), true);
	if (widget) {
		return (widget);
	}
	return (NULL);
}

Widget *PlayerUi::findLuaTargetWidget (const char *targetName) {
	StdString name;
	Widget *widget;

	if (appCard && appCard->widgetName.equals (targetName)) {
		appCard->retain ();
		return (appCard);
	}
	if (mediaOptionWindow && mediaOptionWindow->widgetName.equals (targetName)) {
		mediaOptionWindow->retain ();
		return (mediaOptionWindow);
	}
	if (mediaFilescanWindow && mediaFilescanWindow->widgetName.equals (targetName)) {
		mediaFilescanWindow->retain ();
		return (mediaFilescanWindow);
	}
	name.assign (targetName);
	name.lowercase ();
	widget = cardView->findItem (findItem_matchMediaName, (char *) name.c_str (), true);
	if (widget) {
		return (widget);
	}
	widget = cardView->findItem (findItem_matchPlaylistName, (char *) name.c_str (), true);
	if (widget) {
		return (widget);
	}
	return (NULL);
}

void PlayerUi::executeLuaOpen (Widget *targetWidget) {
	MediaItemWindow *mediaitem;
	MediaPlaylistWindow *playlist;

	mediaitem = MediaItemWindow::castWidget (targetWidget);
	if (mediaitem) {
		mediaitem->eventCallback (mediaitem->viewButtonClickCallback);
		return;
	}
	playlist = MediaPlaylistWindow::castWidget (targetWidget);
	if (playlist) {
		playlist->eventCallback (playlist->editClickCallback);
		return;
	}
}

void PlayerUi::executeLuaTarget (Widget *targetWidget) {
	MediaItemWindow *mediaitem;
	MediaPlaylistWindow *playlist;
	AppCardWindow *appcard;
	MediaOptionWindow *mediaoption;
	MediaFilescanWindow *mediafilescan;

	mediaitem = MediaItemWindow::castWidget (targetWidget);
	if (mediaitem) {
		mediaitem->setSelected (true);
		cardView->scrollToItem (mediaitem->mediaId);
		return;
	}
	playlist = MediaPlaylistWindow::castWidget (targetWidget);
	if (playlist) {
		playlist->setExpanded (true);
		cardView->scrollToItem (playlist->playlist.id);
		return;
	}
	appcard = AppCardWindow::castWidget (targetWidget);
	if (appcard) {
		appcard->setExpanded (true);
		cardView->scrollToItem (appcard->itemId);
		return;
	}
	mediaoption = MediaOptionWindow::castWidget (targetWidget);
	if (mediaoption) {
		mediaoption->setExpanded (true);
		cardView->scrollToItem (mediaoption->itemId);
		return;
	}
	mediafilescan = MediaFilescanWindow::castWidget (targetWidget);
	if (mediafilescan) {
		mediafilescan->setExpanded (true);
		cardView->scrollToItem (mediafilescan->itemId);
		return;
	}
}

void PlayerUi::executeLuaUntarget (Widget *targetWidget) {
	MediaItemWindow *mediaitem;
	MediaPlaylistWindow *playlist;
	AppCardWindow *appcard;
	MediaOptionWindow *mediaoption;
	MediaFilescanWindow *mediafilescan;

	mediaitem = MediaItemWindow::castWidget (targetWidget);
	if (mediaitem) {
		mediaitem->setSelected (false);
		return;
	}
	playlist = MediaPlaylistWindow::castWidget (targetWidget);
	if (playlist) {
		playlist->setExpanded (false);
		return;
	}
	appcard = AppCardWindow::castWidget (targetWidget);
	if (appcard) {
		appcard->setExpanded (false);
		return;
	}
	mediaoption = MediaOptionWindow::castWidget (targetWidget);
	if (mediaoption) {
		mediaoption->setExpanded (false);
		return;
	}
	mediafilescan = MediaFilescanWindow::castWidget (targetWidget);
	if (mediafilescan) {
		mediafilescan->setExpanded (false);
		return;
	}
}
