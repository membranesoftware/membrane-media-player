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
#include "MediaUtil.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "Input.h"
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
#include "MediaControl.h"
#include "ActionWindow.h"
#include "HelpWindow.h"
#include "Label.h"
#include "LabelWindow.h"
#include "CardLabelWindow.h"
#include "IconLabelWindow.h"
#include "TextFieldWindow.h"
#include "TextCardWindow.h"
#include "AppCardWindow.h"
#include "MediaControlWindow.h"
#include "MediaPlaylist.h"
#include "MediaPlaylistWindow.h"
#include "MediaControlSearch.h"
#include "MediaItemImageWindow.h"
#include "MediaItemDetailWindow.h"
#include "TagActionWindow.h"
#include "PlayerWindow.h"
#include "MediaItemUi.h"
#include "MediaPlaylistUi.h"
#include "PlayerUi.h"

constexpr const int UnexpandedMediaControlRow = 0;
constexpr const int ExpandedMediaControlRow = 1;
constexpr const int PlaylistToggleRow = 2;
constexpr const int UnexpandedPlaylistRow = 3;
constexpr const int ExpandedPlaylistRow = 4;
constexpr const int AudioDisabledAlertRow = 5;
constexpr const int EmptyMediaRow = 6;
constexpr const int MediaItemImageRow = 7;
constexpr const int MediaItemDetailRow = 8;
constexpr const int MediaLoadingRow = 9;
constexpr const int RowCount = 10;

constexpr const int ImageGridWindowMode = 0;
constexpr const int DetailLineWindowMode = 1;

constexpr const int EmptyMediaState = 0;
constexpr const int EmptySearchResultState = 1;

constexpr const SDL_Keycode selectAllKeycode = SDLK_a;
constexpr const double textTruncateWidthScale = 0.25f;
constexpr const double searchFieldWidthScale = 0.27f;
constexpr const double bottomPaddingHeightScale = 0.5f;
constexpr const char *playlistQueueId = "PlayerUiPlaylist";

PlayerUi::PlayerUi ()
: Ui ()
, appCardHandle (&appCard)
, searchPanelHandle (&searchPanel)
, searchFieldHandle (&searchField)
, searchStatusIconHandle (&searchStatusIcon)
, mediaControlWindowHandle (&mediaControlWindow)
, emptyStateWindowHandle (&emptyStateWindow)
, loadingIconWindowHandle (&loadingIconWindow)
, targetMediaItemWindowHandle (&targetMediaItemWindow)
, lastSelectedMediaItemWindowHandle (&lastSelectedMediaItemWindow)
, playlistHeaderPanelHandle (&playlistHeaderPanel)
, expandPlaylistsToggleHandle (&expandPlaylistsToggle)
, createPlaylistButtonHandle (&createPlaylistButton)
, audioDisabledAlertWindowHandle (&audioDisabledAlertWindow)
, emptyStateType (-1)
, mediaWindowMode (-1)
, mediaSortOrder (-1)
, mediaDisplayCount (0)
, mediaAvailableCount (0)
, isShowingPlaylists (false)
, isLoadingPlaylists (false)
, isLoadPlaylistsComplete (false)
, shouldWritePlaylists (false)
, isWritingPlaylists (false)
, isWritePlaylistsComplete (false)
, isLoadingMedia (false)
, mediaSearchUpdateTime (0)
, lastRecordSyncTime (0)
, searchRecordSyncClock (-1)
{
	classId = ClassId::PlayerUi;
	SdlUtil::createMutex (&mediaPlaylistMutex);
	SdlUtil::createMutex (&mediaSearchMutex);
}

PlayerUi::~PlayerUi () {
	clearSearch ();
	SdlUtil::destroyMutex (&mediaPlaylistMutex);
	SdlUtil::destroyMutex (&mediaSearchMutex);
}

void PlayerUi::clearSearch () {
	std::list<MediaSearch *>::iterator i1, i2;

	SDL_LockMutex (mediaSearchMutex);
	i1 = mediaSearchList.begin ();
	i2 = mediaSearchList.end ();
	while (i1 != i2) {
		(*i1)->release ();
		++i1;
	}
	mediaSearchList.clear ();
	SDL_UnlockMutex (mediaSearchMutex);
}

StdString PlayerUi::getSpritePath () {
	return (StdString ("ui/PlayerUi/sprite"));
}

Widget *PlayerUi::createBreadcrumbWidget () {
	return (new Chip (UiText::instance->getText (UiTextId::Player).capitalized (), sprites.getSprite (SpriteId::PlayerUi_breadcrumbIcon)));
}

OpResult PlayerUi::doLoad () {
	HashMap *prefs;
	MediaControlSearch *search;
	int imagesize, soundmixvolume, visualizertype;
	bool mediacontrolexpanded, appcardexpanded, shownews, soundmuted, subtitleenabled;

	mediaDisplayCount = 0;
	mediaAvailableCount = 0;
	searchMediaItemIds.clear ();
	isLoadingMedia = false;
	emptyStateType = -1;

	prefs = App::instance->lockPrefs ();
	mediaWindowMode = prefs->find (PlayerUi::windowModeKey, ImageGridWindowMode);
	mediaSortOrder = prefs->find (PlayerUi::sortOrderKey, (int) SystemInterface::Constant_NameSort);
	soundmixvolume = prefs->find (PlayerUi::soundMixVolumeKey, SoundMixer::maxMixVolume);
	soundmuted = prefs->find (PlayerUi::soundMutedKey, false);
	visualizertype = prefs->find (PlayerUi::visualizerTypeKey, PlayerWindow::NoVisualizer);
	subtitleenabled = prefs->find (PlayerUi::subtitleEnabledKey, true);
	imagesize = prefs->find (PlayerUi::imageSizeKey, (int) Ui::MediumSize);
	mediacontrolexpanded = prefs->find (PlayerUi::mediaControlWindowExpandedKey, false);
	appcardexpanded = prefs->find (PlayerUi::appCardExpandedKey, false);
	isShowingPlaylists = prefs->find (PlayerUi::showPlaylistsKey, false);
	shownews = prefs->find (PlayerUi::showAppNewsKey, false);
	App::instance->unlockPrefs ();
	if (! MediaControl::instance->isConfigured) {
		mediacontrolexpanded = true;
		appcardexpanded = false;
		shownews = false;
	}
	UiStack::instance->setPlayerControlOptions (soundmixvolume, soundmuted, visualizertype, subtitleenabled);

	setDetailImageSize (imagesize);
	cardView->setRowCount (RowCount);

	cardView->setRowReverseSorted (ExpandedPlaylistRow, true);

	cardView->setRowHeader (EmptyMediaRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::Media).capitalized ()));
	cardView->setRowHeader (MediaItemImageRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::Media).capitalized ()));
	cardView->setRowHeader (MediaItemDetailRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::Media).capitalized ()));
	cardView->setRowHeader (MediaLoadingRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::Media).capitalized ()));
	cardView->setRowItemMarginSize (MediaItemImageRow, 0.0f);
	cardView->setRowSelectionAnimated (MediaItemImageRow, true);
	cardView->setRowItemMarginSize (MediaItemDetailRow, UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowLabeled (MediaItemImageRow, true, PlayerUi::mediaItemWindowCardViewItemLabel, this);
	cardView->setRowItemMarginSize (MediaLoadingRow, UiConfiguration::instance->marginSize);
	cardView->setBottomPadding (App::instance->drawableHeight * bottomPaddingHeightScale);

	mediaControlWindowHandle.assign (new MediaControlWindow ());
	mediaControlWindow->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaControlWindowExpandStateChanged, this);
	mediaControlWindow->layoutChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaControlWindowLayoutChanged, this);
	mediaControlWindow->itemId = cardView->getAvailableItemId ();
	mediaControlWindow->sortKey.assign ("a");
	mediaControlWindow->setExpanded (mediacontrolexpanded, true);
	cardView->addItem (mediaControlWindow, mediaControlWindow->itemId, mediaControlWindow->isExpanded ? ExpandedMediaControlRow : UnexpandedMediaControlRow);

	appCardHandle.assign (new AppCardWindow ());
	appCard->itemId = cardView->getAvailableItemId ();
	appCard->sortKey.assign ("b");
	appCard->expandStateChangeCallback = Widget::EventCallbackContext (PlayerUi::appCardExpandStateChanged, this);
	appCard->layoutChangeCallback = Widget::EventCallbackContext (PlayerUi::appCardLayoutChanged, this);
	appCard->setExpanded (appcardexpanded, true);
	appCard->initialize (shownews, App::instance->isStartUpdateEnabled);
	cardView->addItem (appCard, appCard->itemId, appCard->isExpanded ? ExpandedMediaControlRow : UnexpandedMediaControlRow);

	search = new MediaControlSearch ();
	search->addRecordsCallback = MediaSearch::EventCallbackContext (PlayerUi::mediaSearchRecordsAdded, this);
	search->removeRecordsCallback = MediaSearch::EventCallbackContext (PlayerUi::mediaSearchRecordsRemoved, this);
	search->resetSearch (StdString (), mediaSortOrder);
	search->retain ();
	SDL_LockMutex (mediaSearchMutex);
	mediaSearchList.push_back (search);
	SDL_UnlockMutex (mediaSearchMutex);

	retain ();
	App::instance->addUpdateTask (PlayerUi::awaitMediaControlReady, this);

	return (OpResult::Success);
}

void PlayerUi::doUnload () {
	clearSearch ();
	appCardHandle.clear ();
	searchPanelHandle.clear ();
	searchFieldHandle.clear ();
	searchStatusIconHandle.clear ();
	emptyStateWindowHandle.clear ();
	playlistHeaderPanelHandle.clear ();
	expandPlaylistsToggleHandle.clear ();
	createPlaylistButtonHandle.clear ();
	mediaControlWindowHandle.clear ();
	loadingIconWindowHandle.clear ();
	targetMediaItemWindowHandle.clear ();
	lastSelectedMediaItemWindowHandle.clear ();
	selectedMediaMap.clear ();
	audioDisabledAlertWindowHandle.clear ();

	RecordStore::instance->remove (loadedRecordIds);
	loadedRecordIds.clear ();
}

void PlayerUi::doAddMainToolbarItems (Toolbar *toolbar) {
	toolbar->addRightItem (createImageSizeButton ());
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
	searchStatusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchEmptyResultTooltip));
	searchPanel->add (searchStatusIcon);

	searchPanel->setFixedPadding (true, 0.0f, 0.0f);
	searchPanel->layoutSpacing = 0.0f;
	searchPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);
	toolbar->setLeftCorner (searchPanel);

	toolbar->addRightItem (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_playButton), PlayerUi::playButtonClicked, PlayerUi::playButtonFocused, UiText::instance->getText (UiTextId::PlayerUiPlayTooltip).capitalized (), "playButton", SDLK_F4));
	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_fullscreenButton), PlayerUi::fullscreenButtonClicked, PlayerUi::fullscreenButtonFocused, UiText::instance->getText (UiTextId::PlayerUiFullscreenTooltip).capitalized (), "fullscreenButton", SDLK_F3));
	toolbar->addRightItem (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_pauseButton), PlayerUi::pauseButtonClicked, PlayerUi::pauseButtonFocused, UiText::instance->getText (UiTextId::PlayerUiPauseTooltip).capitalized (), "pauseButton", SDLK_F2));
	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_stopButton), PlayerUi::stopButtonClicked, PlayerUi::stopButtonFocused, UiText::instance->getText (UiTextId::PlayerUiStopTooltip).capitalized (), "stopButton", SDLK_F1));

	button = createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_starHalfButton), PlayerUi::selectAllButtonClicked, PlayerUi::selectAllButtonFocused, UiText::instance->getText (UiTextId::SelectAllTooltip), "selectAllButton");
	button->shortcutKey = selectAllKeycode;
	button->isShortcutKeyControlPress = true;
	toolbar->addRightItem (button);

	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::PlayerUi_tagButton), PlayerUi::tagButtonClicked, PlayerUi::tagButtonFocused, UiText::instance->getText (UiTextId::PlayerUiTagMenuTooltip), "tagButton"));
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
	if (mediaControlWindow) {
		prefs->insert (PlayerUi::mediaControlWindowExpandedKey, mediaControlWindow->isExpanded, false);
	}
	if (appCard) {
		prefs->insert (PlayerUi::appCardExpandedKey, appCard->isExpanded, false);
	}
	prefs->insert (PlayerUi::imageSizeKey, detailImageSize, (int) Ui::MediumSize);
	prefs->insert (PlayerUi::sortOrderKey, mediaSortOrder, (int) SystemInterface::Constant_NameSort);
	prefs->insert (PlayerUi::windowModeKey, mediaWindowMode, ImageGridWindowMode);
	prefs->insert (PlayerUi::soundMixVolumeKey, soundmixvolume, SoundMixer::maxMixVolume);
	prefs->insert (PlayerUi::soundMutedKey, soundmuted, false);
	prefs->insert (PlayerUi::visualizerTypeKey, visualizertype, PlayerWindow::NoVisualizer);
	prefs->insert (PlayerUi::subtitleEnabledKey, subtitleenabled, true);
	prefs->insert (PlayerUi::showPlaylistsKey, isShowingPlaylists, false);
	App::instance->unlockPrefs ();
}

void PlayerUi::doUpdate (int msElapsed) {
	emptyStateWindowHandle.compact ();
	playlistHeaderPanelHandle.compact ();
	expandPlaylistsToggleHandle.compact ();
	createPlaylistButtonHandle.compact ();
	loadingIconWindowHandle.compact ();
	targetMediaItemWindowHandle.compact ();
	lastSelectedMediaItemWindowHandle.compact ();
	audioDisabledAlertWindowHandle.compact ();
	updatePlaylists ();
	updateSearch (msElapsed);
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
	it->release ();
}

void PlayerUi::updatePlaylists () {
	StdString id;
	Panel *panel;
	std::list<MediaPlaylist>::const_iterator i1, i2;
	MediaPlaylistWindow *playlist;
	std::list<MediaPlaylistWindow *> playlistwindows;
	std::list<MediaPlaylistWindow *>::const_iterator j1, j2;
	int64_t now;

	if (isShowingPlaylists) {
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
			cardView->addItem (playlistHeaderPanel, PlaylistToggleRow);

			if (! isLoadingPlaylists) {
				isLoadingPlaylists = true;
				isLoadPlaylistsComplete = false;
				retain ();
				TaskGroup::instance->run (TaskGroup::RunContext (PlayerUi::loadMediaPlaylists, this, playlistQueueId));
			}
		}

		if (isLoadingPlaylists) {
			if (isLoadPlaylistsComplete) {
				now = OsUtil::getTime ();
				SDL_LockMutex (mediaPlaylistMutex);
				i1 = mediaPlaylists.cbegin ();
				i2 = mediaPlaylists.cend ();
				while (i1 != i2) {
					playlist = createMediaPlaylistWindow ();
					playlist->read (*i1);
					playlistwindows.push_back (playlist);
					++i1;
				}
				SDL_UnlockMutex (mediaPlaylistMutex);

				j1 = playlistwindows.cbegin ();
				j2 = playlistwindows.cend ();
				while (j1 != j2) {
					playlist = *j1;
					id = cardView->getAvailableItemId ();
					playlist->itemId.assign (id);
					setSortKey (playlist, now);
					cardView->addItem (playlist, id, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow, true);
					cardView->animateItemScaleBump (id);
					++j1;
				}
				cardView->reflow ();
				isLoadingPlaylists = false;
			}
		}

		if (createPlaylistButton) {
			createPlaylistButton->setDisabled (isLoadingPlaylists || (! MediaControl::instance->isReady));
		}
	}
	else {
		shouldWritePlaylists = false;
		if (playlistHeaderPanel) {
			UiStack::instance->stopPlaylists ();
			cardView->removeRowItems (ExpandedPlaylistRow);
			cardView->removeRowItems (UnexpandedPlaylistRow);

			id = cardView->findItemId (CardView::matchPointerValue, playlistHeaderPanel);
			if (! id.empty ()) {
				cardView->removeItem (id);
			}
			playlistHeaderPanelHandle.clear ();
		}
	}

	if (shouldWritePlaylists) {
		isWritingPlaylists = true;
		isWritePlaylistsComplete = false;
		retain ();
		TaskGroup::instance->run (TaskGroup::RunContext (PlayerUi::writeMediaPlaylists, this, playlistQueueId));
		shouldWritePlaylists = false;
	}
	if (isWritingPlaylists) {
		shouldWritePlaylists = false;
		if (isWritePlaylistsComplete) {
			isWritingPlaylists = false;
		}
	}
}

void PlayerUi::updatePlaylistRecord (const MediaPlaylist &playlist) {
	std::list<MediaPlaylist>::iterator i1, i2;
	MediaPlaylist item;
	bool found;

	found = false;
	SDL_LockMutex (mediaPlaylistMutex);
	i1 = mediaPlaylists.begin ();
	i2 = mediaPlaylists.end ();
	while (i1 != i2) {
		if (i1->id.equals (playlist.id)) {
			i1->copyValues (playlist);
			found = true;
			break;
		}
		++i1;
	}
	if (! found) {
		item.copyValues (playlist);
		mediaPlaylists.push_back (item);
	}
	mediaPlaylistWriteMap.insert (playlist.id, true);
	SDL_UnlockMutex (mediaPlaylistMutex);
	shouldWritePlaylists = true;
}

void PlayerUi::removePlaylistRecord (const StdString &playlistId) {
	std::list<MediaPlaylist>::iterator i1, i2;
	MediaPlaylist item;
	bool found;

	found = false;
	SDL_LockMutex (mediaPlaylistMutex);
	i1 = mediaPlaylists.begin ();
	i2 = mediaPlaylists.end ();
	while (i1 != i2) {
		if (i1->id.equals (playlistId)) {
			mediaPlaylists.erase (i1);
			found = true;
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (mediaPlaylistMutex);
	if (found) {
		shouldWritePlaylists = true;
	}
}

void PlayerUi::loadMediaPlaylists (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->executeLoadMediaPlaylists ();
	it->isLoadPlaylistsComplete = true;
	it->release ();
}
void PlayerUi::executeLoadMediaPlaylists () {
	std::list<MediaPlaylist> playlists;
	StdString errmsg;

	if (! MediaControl::instance->isReady) {
		return;
	}
	if (MediaPlaylist::readDatabaseRows (MediaControl::instance->databasePath, &errmsg, &playlists)) {
		SDL_LockMutex (mediaPlaylistMutex);
		mediaPlaylists.swap (playlists);
		mediaPlaylistWriteMap.clear ();
		SDL_UnlockMutex (mediaPlaylistMutex);
	}
}

void PlayerUi::writeMediaPlaylists (void *itPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->executeWriteMediaPlaylists ();
	it->isWritePlaylistsComplete = true;
	it->release ();
}
void PlayerUi::executeWriteMediaPlaylists () {
	std::list<MediaPlaylist>::const_iterator i1, i2;
	StringList sql;
	StringList::const_iterator j1, j2;
	StdString errmsg;
	OpResult result;

	SDL_LockMutex (mediaPlaylistMutex);
	sql.push_back (MediaPlaylist::getDeleteExcludeSql (mediaPlaylists));
	i1 = mediaPlaylists.cbegin ();
	i2 = mediaPlaylists.cend ();
	while (i1 != i2) {
		if (mediaPlaylistWriteMap.exists (i1->id)) {
			sql.push_back (i1->getUpsertSql ());
		}
		++i1;
	}
	mediaPlaylistWriteMap.clear ();
	SDL_UnlockMutex (mediaPlaylistMutex);

	j1 = sql.cbegin ();
	j2 = sql.cend ();
	while (j1 != j2) {
		result = Database::instance->exec (MediaControl::instance->databasePath, *j1, &errmsg);
		if (result != OpResult::Success) {
			Log::debug ("Failed to write playlist records; result=%i errmsg=\"%s\"", result, errmsg.c_str ());
		}
		++j1;
	}
}

void PlayerUi::updateSearch (int msElapsed) {
	std::list<MediaSearch *>::const_iterator i1, i2;
	MediaSearch *search;
	int64_t updatetime;
	int recordcount, setsize, availablecount;
	bool advance, loading;

	updatetime = 0;
	availablecount = 0;
	advance = cardView->isScrolledToBottom (App::instance->drawableHeight * bottomPaddingHeightScale);
	loading = false;
	SDL_LockMutex (mediaSearchMutex);
	i1 = mediaSearchList.cbegin ();
	i2 = mediaSearchList.cend ();
	while (i1 != i2) {
		search = *i1;
		search->update (msElapsed);
		if (search->lastStatusUpdateTime > updatetime) {
			updatetime = search->lastStatusUpdateTime;
		}
		if (advance && (! search->isFindComplete) && (! search->isLoading) && (lastRecordSyncTime >= search->lastStatusUpdateTime)) {
			search->advanceSearch ();
		}
		if (search->isLoading) {
			loading = true;
		}
		availablecount += search->mediaAvailableCount;
		++i1;
	}
	SDL_UnlockMutex (mediaSearchMutex);

	if (searchStatusIcon && (mediaSearchUpdateTime != updatetime)) {
		recordcount = 0;
		setsize = 0;
		SDL_LockMutex (mediaSearchMutex);
		i1 = mediaSearchList.cbegin ();
		i2 = mediaSearchList.cend ();
		while (i1 != i2) {
			search = *i1;
			recordcount += search->searchReceiveCount;
			setsize += search->setSize;
			++i1;
		}
		SDL_UnlockMutex (mediaSearchMutex);
		if (setsize <= 0) {
			searchStatusIcon->setText (StdString ("0"));
			searchStatusIcon->setTextColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchEmptyResultTooltip));
		}
		else if (recordcount >= setsize) {
			searchStatusIcon->setText (StdString::createSprintf ("%i", setsize));
			searchStatusIcon->setTextColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->inverseTextColor);
			searchStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::SearchComplete).capitalized ().c_str (), UiText::instance->getCountText (setsize, UiTextId::Result, UiTextId::Results).c_str ()));
		}
		else {
			searchStatusIcon->setText (StdString::createSprintf ("%i / %i", recordcount, setsize));
			searchStatusIcon->setTextColor (UiConfiguration::instance->mediumSecondaryColor);
			searchStatusIcon->setIconImageColor (UiConfiguration::instance->mediumSecondaryColor);
			searchStatusIcon->setMouseHoverTooltip (StdString::createSprintf ("%s: %s %s", UiText::instance->getText (UiTextId::SearchInProgress).capitalized ().c_str (), UiText::instance->getCountText (setsize, UiTextId::Result, UiTextId::Results).c_str (), UiText::instance->getText (UiTextId::SearchInProgressTooltip).c_str ()));
		}
		searchPanel->reflow ();
		UiStack::instance->secondaryToolbar->reflow ();
		mediaSearchUpdateTime = updatetime;
		syncSearchRecords ();
	}

	if (loading) {
		if (! loadingIconWindow) {
			loadingIconWindowHandle.assign (createLoadingIconWindow ());
			cardView->addItem (loadingIconWindow, MediaLoadingRow);
		}
		if (emptyStateWindow) {
			cardView->removeItem (emptyStateWindow->itemId);
			emptyStateWindowHandle.clear ();
			emptyStateType = -1;
		}
	}
	else {
		if (loadingIconWindow) {
			cardView->removeRowItems (MediaLoadingRow);
			loadingIconWindowHandle.clear ();
		}
		if (isLoadingMedia) {
			searchRecordSyncClock = 0;
		}
	}
	isLoadingMedia = loading;
	mediaAvailableCount = availablecount;

	if (searchRecordSyncClock >= 0) {
		searchRecordSyncClock -= msElapsed;
		if (searchRecordSyncClock <= 0) {
			App::instance->shouldSyncRecordStore = true;
			searchRecordSyncClock = -1;
		}
	}
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

void PlayerUi::doSyncRecordStore () {
	StringList::const_iterator i1, i2;
	StdString mediaid;
	Json record;
	MediaItemWindow *item;
	MediaItemDetailWindow *mediaitemdetail;
	MediaItemImageWindow *mediaitemimage;
	int type, row;

	i1 = searchMediaItemIds.cbegin ();
	i2 = searchMediaItemIds.cend ();
	while (i1 != i2) {
		mediaid = *i1;
		if (! cardView->contains (mediaid)) {
			if (RecordStore::instance->find (&record, mediaid, SystemInterface::CommandId_MediaItem, true)) {
				if (mediaWindowMode == DetailLineWindowMode) {
					mediaitemdetail = new MediaItemDetailWindow (&record);
					item = mediaitemdetail;
					row = MediaItemDetailRow;
				}
				else {
					mediaitemimage = new MediaItemImageWindow (&record);
					item = mediaitemimage;
					row = MediaItemImageRow;
				}

				item->mediaImageClickCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowImageClicked, this);
				item->viewButtonClickCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowViewButtonClicked, this);
				item->selectStateChangeCallback = Widget::EventCallbackContext (PlayerUi::mediaItemWindowSelectStateChanged, this);
				item->setDetailSize (detailImageSize, cardView->cardAreaWidth / CardView::reducedSizeItemScale);
				if (selectedMediaMap.exists (mediaid)) {
					item->setSelected (true, true);
				}
				cardView->addItem (item, mediaid, row);
				cardView->animateItemScaleBump (mediaid);
				++mediaDisplayCount;
				loadedRecordIds.push_back (mediaid);
			}
		}
		++i1;
	}
	searchMediaItemIds.clear ();

	type = -1;
	if (MediaControl::instance->isReady && (! isLoadingMedia) && (mediaDisplayCount <= 0)) {
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
			cardView->addItem (emptyStateWindow, emptyStateWindow->itemId, EmptyMediaRow);
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
	lastRecordSyncTime = OsUtil::getTime ();
}

void PlayerUi::mediaSearchRecordsAdded (void *itPtr, MediaSearch *search) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->searchMediaItemIds.append (search->eventRecordIds);
}

void PlayerUi::mediaSearchRecordsRemoved (void *itPtr, MediaSearch *search) {
	PlayerUi *it = (PlayerUi *) itPtr;
	StringList::const_iterator i1, i2;
	StdString id;

	i1 = search->eventRecordIds.cbegin ();
	i2 = search->eventRecordIds.cend ();
	while (i1 != i2) {
		id = *i1;
		if (it->cardView->contains (id)) {
			it->cardView->removeItem (id);
			it->loadedRecordIds.remove (id);
			RecordStore::instance->remove (id);
		}
		++i1;
	}
	it->cardView->reflow ();
}

void PlayerUi::resetSearch () {
	std::list<MediaSearch *>::iterator i1, i2;

	SDL_LockMutex (mediaSearchMutex);
	i1 = mediaSearchList.begin ();
	i2 = mediaSearchList.end ();
	while (i1 != i2) {
		(*i1)->resetSearch (searchKey, mediaSortOrder);
		++i1;
	}
	SDL_UnlockMutex (mediaSearchMutex);

	mediaDisplayCount = 0;
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
	menu->addItem (UiText::instance->getText (UiTextId::ShowPlaylists).capitalized (), it->sprites.getSprite (SpriteId::PlayerUi_createPlaylistButton), Widget::EventCallbackContext (PlayerUi::showPlaylistsActionClicked, it), 3, it->isShowingPlaylists);
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
		cardView->removeRowItems (MediaItemDetailRow);
	}
	else {
		cardView->removeRowItems (MediaItemImageRow);
	}
	App::instance->shouldSyncRecordStore = true;
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

void PlayerUi::showPlaylistsActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;

	it->isShowingPlaylists = (! it->isShowingPlaylists);
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
			UiStack::instance->playMedia (ui->mediaId, ui->playTimestamp);
		}
		else if (ui->selectPlayPositionTimestamp >= 0) {
			it->targetMediaItemWindow->setPlayTimestamp (ui->selectPlayPositionTimestamp);
			it->cardView->resetItemLabels ();
			it->cardView->reflow ();
		}
		it->targetMediaItemWindowHandle.clear ();
	}
}

void PlayerUi::mediaItemWindowCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel) {
	MediaItemWindow *mediaitem;
	StdString text;

	mediaitem = MediaItemWindow::castWidget (itemWidget);
	if (mediaitem) {
		if (mediaitem->playTimestamp <= 0) {
			text.assign (mediaitem->mediaItem.name);
		}
		else {
			text.sprintf ("<%s> ", UiText::instance->getTimespanText (mediaitem->playTimestamp, UiText::HoursUnit, true).c_str ());
			text.append (mediaitem->mediaItem.name);
		}
		cardLabel->setText (text);
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
		((StringList *) stringListPtr)->push_back (playlist->itemId);
	}
}
void PlayerUi::expandPlaylistsToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist;
	StringList idlist;
	StringList::iterator i1, i2;
	int64_t now;

	if (! it->expandPlaylistsToggle) {
		return;
	}
	now = OsUtil::getTime ();
	it->cardView->processItems (expandPlaylistsToggleStateChanged_appendPlaylistId, &idlist);
	i1 = idlist.begin ();
	i2 = idlist.end ();
	while (i1 != i2) {
		playlist = MediaPlaylistWindow::castWidget (it->cardView->getItem (*i1));
		if (playlist) {
			playlist->setExpanded (! it->expandPlaylistsToggle->isChecked, true);
			it->setSortKey (playlist, now);
			it->cardView->setItemRow (playlist->itemId, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow, true);
			it->updatePlaylistRecord (playlist->playlist);
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
	it->cardView->setItemRow (window->itemId, window->isExpanded ? ExpandedMediaControlRow : UnexpandedMediaControlRow, true);
	it->cardView->reflow ();
}

void PlayerUi::appCardLayoutChanged (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->cardView->reflow ();
}

void PlayerUi::mediaControlWindowExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaControlWindow *window = (MediaControlWindow *) widgetPtr;

	window->resetInputState ();
	it->cardView->animateItemScaleBump (window->itemId);
	it->resetExpandToggles ();
	it->clearPopupWidgets ();
	it->cardView->setItemRow (window->itemId, window->isExpanded ? ExpandedMediaControlRow : UnexpandedMediaControlRow, true);
	it->cardView->reflow ();
}

void PlayerUi::mediaControlWindowLayoutChanged (void *itPtr, Widget *widgetPtr) {
	((PlayerUi *) itPtr)->cardView->reflow ();
}

void PlayerUi::playlistExpandStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;

	it->setSortKey (playlist);
	it->cardView->setItemRow (playlist->itemId, playlist->isExpanded ? ExpandedPlaylistRow : UnexpandedPlaylistRow, true);
	playlist->resetInputState ();
	it->cardView->animateItemScaleBump (playlist->itemId);
	it->resetExpandToggles ();
	it->clearPopupWidgets ();
	it->cardView->reflow ();
	it->updatePlaylistRecord (playlist->playlist);
}

void PlayerUi::playlistOptionChanged (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;

	it->updatePlaylistRecord (playlist->playlist);
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
	it->clearPopupWidgets ();
	it->cardView->reflow ();
	it->updatePlaylistRecord (playlist->playlist);
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
	it->cardView->removeItem (playlist->itemId);
	it->resetExpandToggles ();
	it->removePlaylistRecord (playlist->playlist.id);
}

void PlayerUi::playlistAddItemActionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	MediaItemWindow *mediaitem;
	HashMap::Iterator i;
	StdString mediaid;
	int count;

	if (it->selectedMediaMap.empty () || it->getSelectedMediaNames (true).empty ()) {
		return;
	}
	count = 0;
	i = it->selectedMediaMap.begin ();
	while (it->selectedMediaMap.next (&i, &mediaid)) {
		mediaitem = MediaItemWindow::castWidget (it->cardView->getItem (mediaid));
		if (mediaitem) {
			playlist->addItem (mediaitem->mediaId, mediaitem->playTimestamp);
			++count;
		}
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
	it->updatePlaylistRecord (playlist->playlist);
}

void PlayerUi::playlistAddItemFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistWindow *playlist = (MediaPlaylistWindow *) widgetPtr;
	IconLabelWindow *icon1, *icon2;
	StdString text;
	Color color;

	text.assign (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (playlist->playlist.name, App::instance->rootPanel->width * 0.20f, Font::dotTruncateSuffix));
	icon1 = it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallPlaylistIcon), text, color);

	text.assign (it->getSelectedMediaNames (true));
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
	PlayerUi *it = (PlayerUi *) itPtr;
	MediaPlaylistUi *playlistui = (MediaPlaylistUi *) uiPtr;

	it->updatePlaylistRecord (playlistui->playlist->playlist);
}

void PlayerUi::playlistPlayActionClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistWindow *playlist;

	playlist = MediaPlaylistWindow::castWidget (widgetPtr);
	if (playlist) {
		UiStack::instance->playPlaylist (playlist);
	}
}

void PlayerUi::playButtonFocused (void *itPtr, Widget *widgetPtr) {
	PlayerUi *it = (PlayerUi *) itPtr;
	StdString text;
	Color color;

	text.assign (it->getSelectedMediaNames (true));
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
	it->playSelectedStreams ();
}
void PlayerUi::playSelectedStreams () {
	StringList keys;
	StringList::const_iterator i1, i2;
	StdString id;
	MediaItemWindow *mediaitem;
	int playercount;
	bool detached;

	playercount = UiStack::instance->getPlayerCount ();
	if (playercount >= PlayerControl::maxPlayerCount) {
		return;
	}
	detached = false;
	if (lastSelectedMediaItemWindow) {
		UiStack::instance->playMedia (lastSelectedMediaItemWindow->mediaId, lastSelectedMediaItemWindow->playTimestamp > 0 ? lastSelectedMediaItemWindow->playTimestamp : 0, false);
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
			UiStack::instance->playMedia (mediaitem->mediaId, mediaitem->playTimestamp > 0 ? mediaitem->playTimestamp : 0, detached);
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

	text.assign (it->getSelectedMediaNames (false));
	color.assign (UiConfiguration::instance->primaryTextColor);
	if (text.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::PlayerUiNoMediaSelectedPrompt));
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
	text.assign (it->getSelectedMediaNames (false));
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

	it->selectedMediaMap.getKeys (&keys);
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

	it->selectedMediaMap.getKeys (&keys);
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
	StdString sql, errmsg;
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
				sql = MediaItem::getUpdateTagsSql (mediaitem.mediaId, mediaitem.tags);
				result = Database::instance->exec (MediaControl::instance->databasePath, sql, &errmsg);
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
	StdString sql, errmsg;
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
				sql = MediaItem::getUpdateTagsSql (mediaitem.mediaId, mediaitem.tags);
				result = Database::instance->exec (MediaControl::instance->databasePath, sql, &errmsg);
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
	playlist->itemId.assign (it->cardView->getAvailableItemId ());
	playlist->setPlaylistName (it->getAvailablePlaylistName ());
	playlist->setExpanded (true);
	it->cardView->addItem (playlist, playlist->itemId, ExpandedPlaylistRow);
	it->cardView->animateItemScaleBump (playlist->itemId);

	it->cardView->scrollToItem (playlist->itemId);
	it->cardView->reflow ();
	it->resetExpandToggles ();
	App::instance->showNotification (StdString::createSprintf ("%s: %s", UiText::instance->getText (UiTextId::CreatedPlaylist).capitalized ().c_str (), playlist->playlist.name.c_str ()));

	it->updatePlaylistRecord (playlist->playlist);
}

StdString PlayerUi::getSelectedMediaNames (bool isPlayableMediaRequired) {
	StdString id;
	HashMap::Iterator i;
	MediaItemWindow *mediaitem;
	StringList names;

	i = selectedMediaMap.begin ();
	while (selectedMediaMap.next (&i, &id)) {
		mediaitem = MediaItemWindow::castWidget (cardView->getItem (id));
		if (mediaitem) {
			if (isPlayableMediaRequired && (! mediaitem->isPlayable)) {
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

void PlayerUi::unselectAllMedia () {
	StringList keys;
	StringList::const_iterator i1, i2;
	MediaItemWindow *mediaitem;
	StdString id;

	selectedMediaMap.getKeys (&keys);
	i1 = keys.cbegin ();
	i2 = keys.cend ();
	while (i1 != i2) {
		id.assign (*i1);
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
	if (! MediaControl::instance->isConfigured) {
		helpWindow->addAction (UiText::instance->getText (UiTextId::PlayerUiHelpAction1Text), HelpWindow::InfoAction, UiText::instance->getText (UiTextId::LearnMore).capitalized (), AppUrl::instance->help (AppUrl::MediaPlayerStart, true), AppUrl::instance->help (AppUrl::MediaPlayerStart));
	}
	else if ((mediaDisplayCount <= 0) && (mediaAvailableCount <= 0)) {
		helpWindow->addAction (UiText::instance->getText (UiTextId::PlayerUiHelpAction2Text), HelpWindow::InfoAction, UiText::instance->getText (UiTextId::LearnMore).capitalized (), AppUrl::instance->help (AppUrl::MediaPlayerScan, true), AppUrl::instance->help (AppUrl::MediaPlayerScan));
	}
	else {
		helpWindow->addAction (UiText::instance->getText (UiTextId::PlayerUiHelpAction3Text), HelpWindow::InfoAction);
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

	mediaitem = MediaItemWindow::castWidget (targetWidget);
	if (mediaitem) {
		mediaitem->setSelected (true);
		cardView->scrollToItem (mediaitem->mediaId);
		return;
	}
	playlist = MediaPlaylistWindow::castWidget (targetWidget);
	if (playlist) {
		playlist->setExpanded (true);
		cardView->scrollToItem (playlist->itemId);
		return;
	}
}

void PlayerUi::executeLuaUntarget (Widget *targetWidget) {
	MediaItemWindow *mediaitem;
	MediaPlaylistWindow *playlist;

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
}
