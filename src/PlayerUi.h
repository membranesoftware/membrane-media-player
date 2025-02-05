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
// UI that shows the main player interface
#ifndef PLAYER_UI_H
#define PLAYER_UI_H

#include "HashMap.h"
#include "StringList.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Ui.h"

class Json;
class Sprite;
class Button;
class Toggle;
class Panel;
class MediaSearch;
class HelpWindow;
class CardLabelWindow;
class IconLabelWindow;
class TextCardWindow;
class TextFieldWindow;
class AppCardWindow;
class MediaItemWindow;
class MediaControlWindow;
class MediaPlaylist;
class MediaPlaylistWindow;

class PlayerUi : public Ui {
public:
	PlayerUi ();
	~PlayerUi ();

	// Prefs keys
	static constexpr const char *appCardExpandedKey = "PlayerUiA";
	static constexpr const char *imageSizeKey = "PlayerUiB";
	static constexpr const char *sortOrderKey = "PlayerUiC";
	static constexpr const char *windowModeKey = "PlayerUiD";
	static constexpr const char *showPlaylistsKey = "PlayerUiE";
	static constexpr const char *soundMixVolumeKey = "PlayerUiF";
	static constexpr const char *soundMutedKey = "PlayerUiG";
	static constexpr const char *mediaControlWindowExpandedKey = "PlayerUiH";
	static constexpr const char *startUpdateKey = "PlayerUiI";
	static constexpr const char *showAppNewsKey = "PlayerUiJ";
	static constexpr const char *visualizerTypeKey = "PlayerUiK";
	static constexpr const char *subtitleEnabledKey = "PlayerUiL";

	// Superclass override methods
	void setHelpWindowContent (HelpWindow *helpWindow);

protected:
	// Superclass override methods
	StdString getSpritePath ();
	Widget *createBreadcrumbWidget ();
	OpResult doLoad ();
	void doUnload ();
	void doAddMainToolbarItems (Toolbar *toolbar);
	void doAddSecondaryToolbarItems (Toolbar *toolbar);
	void doResume ();
	void doPause ();
	void doUpdate (int msElapsed);
	void doResize ();
	void doSyncRecordStore ();
	Widget *findLuaOpenWidget (const char *targetName);
	Widget *findLuaTargetWidget (const char *targetName);
	void executeLuaOpen (Widget *targetWidget);
	void executeLuaTarget (Widget *targetWidget);
	void executeLuaUntarget (Widget *targetWidget);
	void handleDetailImageSizeChange ();
	static void handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget);

private:
	// Callback functions
	static void doResize_processItems (void *itPtr, Widget *itemWidget);
	static void expandPlaylistsToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void mediaItemWindowImageClicked (void *itPtr, Widget *widgetPtr);
	static void mediaItemWindowViewButtonClicked (void *itPtr, Widget *widgetPtr);
	static void mediaItemWindowSelectStateChanged (void *itPtr, Widget *widgetPtr);
	static void mediaItemWindowCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel);
	static void mediaItemUiEnded (void *itPtr, Ui *uiPtr);
	static void searchFieldEdited (void *itPtr, Widget *widgetPtr);
	static void searchButtonClicked (void *itPtr, Widget *widgetPtr);
	static void playerMenuButtonClicked (void *itPtr, Widget *widgetPtr);
	static void boxLayoutActionClicked (void *itPtr, Widget *widgetPtr);
	static void lineLayoutActionClicked (void *itPtr, Widget *widgetPtr);
	static void sortByNameActionClicked (void *itPtr, Widget *widgetPtr);
	static void sortByNewestActionClicked (void *itPtr, Widget *widgetPtr);
	static void sortByFilePathActionClicked (void *itPtr, Widget *widgetPtr);
	static void showPlaylistsActionClicked (void *itPtr, Widget *widgetPtr);
	static void mediaControlWindowExpandStateChanged (void *itPtr, Widget *widgetPtr);
	static void mediaControlWindowLayoutChanged (void *itPtr, Widget *widgetPtr);
	static void appCardExpandStateChanged (void *itPtr, Widget *widgetPtr);
	static void appCardLayoutChanged (void *itPtr, Widget *widgetPtr);
	static void playButtonFocused (void *itPtr, Widget *widgetPtr);
	static void playButtonClicked (void *itPtr, Widget *widgetPtr);
	static void pauseButtonFocused (void *itPtr, Widget *widgetPtr);
	static void pauseButtonClicked (void *itPtr, Widget *widgetPtr);
	static void stopButtonFocused (void *itPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *itPtr, Widget *widgetPtr);
	static void fullscreenButtonFocused (void *itPtr, Widget *widgetPtr);
	static void fullscreenButtonClicked (void *itPtr, Widget *widgetPtr);
	static void selectAllButtonFocused (void *itPtr, Widget *widgetPtr);
	static void selectAllButtonClicked (void *itPtr, Widget *widgetPtr);
	static void tagButtonFocused (void *itPtr, Widget *widgetPtr);
	static void tagButtonClicked (void *itPtr, Widget *widgetPtr);
	static void tagActionWindowAddClicked (void *itPtr, Widget *widgetPtr);
	static void tagActionWindowRemoveClicked (void *itPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *itPtr, Widget *widgetPtr);
	static void playlistExpandStateChanged (void *itPtr, Widget *widgetPtr);
	static void playlistRenameActionClicked (void *itPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *itPtr, Widget *widgetPtr);
	static void playlistNameEditEnterButtonClicked (void *itPtr, Widget *widgetPtr);
	static void playlistRemoveActionClicked (void *itPtr, Widget *widgetPtr);
	static void removePlaylistActionClosed (void *itPtr, Widget *widgetPtr);
	static void playlistAddItemActionClicked (void *itPtr, Widget *widgetPtr);
	static void playlistAddItemFocused (void *itPtr, Widget *widgetPtr);
	static void playlistOptionChanged (void *itPtr, Widget *widgetPtr);
	static void playlistEditActionClicked (void *itPtr, Widget *widgetPtr);
	static void playlistPlayActionClicked (void *itPtr, Widget *widgetPtr);
	static void mediaPlaylistUiEnded (void *itPtr, Ui *uiPtr);
	static void mediaSearchRecordsAdded (void *itPtr, MediaSearch *search);
	static void mediaSearchRecordsRemoved (void *itPtr, MediaSearch *search);

	struct TagTask {
		PlayerUi *ui;
		StdString tag;
		StringList mediaItemIds;
		TagTask ():
			ui (NULL) { }
	};
	// Task functions
	static void awaitMediaControlReady (void *itPtr);
	static void addTags (void *taskPtr);
	void executeAddTags (PlayerUi::TagTask *task);
	static void removeTags (void *taskPtr);
	void executeRemoveTags (PlayerUi::TagTask *task);
	static void loadMediaPlaylists (void *itPtr);
	void executeLoadMediaPlaylists ();
	static void writeMediaPlaylists (void *itPtr);
	void executeWriteMediaPlaylists ();

	// Set the media item window display mode
	void setMediaItemWindowMode (int mode);

	// Set the sortKey value for a view item. If sequenceValue is not provided, use the current time.
	void setSortKey (MediaPlaylistWindow *mediaPlaylist, int64_t sequenceValue = 0);

	// Clear search state data
	void clearSearch ();

	// Apply reset state for all active searches
	void resetSearch ();

	// Execute operations as needed for any active search state
	void updateSearch (int msElapsed);

	// Return a string containing the set of selected media item names, appropriate for use in a command popup, or an empty string if no media items are selected
	StdString getSelectedMediaNames (bool isPlayableMediaRequired = false);

	// Clear selected state from all media items
	void unselectAllMedia ();

	// Return a newly created MediaPlaylistWindow widget, suitable for use as a card view item
	MediaPlaylistWindow *createMediaPlaylistWindow ();

	// Return the provided base value, after appending suffixes as needed to generate an unused playlist name
	StdString getAvailablePlaylistName (const StdString &baseName = StdString ());

	// Execute operations as needed for playlist state
	void updatePlaylists ();

	// Copy changes to stored playlist state and queue a write operation
	void updatePlaylistRecord (const MediaPlaylist &playlist);

	// Remove the specified playlist record and queue a write operation
	void removePlaylistRecord (const StdString &playlistId);

	// Start video playback windows for each selected stream
	void playSelectedStreams ();

	// Reset checked states for row expand toggles, as appropriate for item expand state
	void resetExpandToggles ();

	// Set the time of the next record sync if it isn't already assigned
	void syncSearchRecords ();

	// Return the number of selected media items
	int getSelectedMediaCount ();

	WidgetHandle<AppCardWindow> appCardHandle;
	AppCardWindow *appCard;
	WidgetHandle<Panel> searchPanelHandle;
	Panel *searchPanel;
	WidgetHandle<TextFieldWindow> searchFieldHandle;
	TextFieldWindow *searchField;
	WidgetHandle<IconLabelWindow> searchStatusIconHandle;
	IconLabelWindow *searchStatusIcon;
	WidgetHandle<MediaControlWindow> mediaControlWindowHandle;
	MediaControlWindow *mediaControlWindow;
	WidgetHandle<TextCardWindow> emptyStateWindowHandle;
	TextCardWindow *emptyStateWindow;
	WidgetHandle<IconLabelWindow> loadingIconWindowHandle;
	IconLabelWindow *loadingIconWindow;
	WidgetHandle<MediaItemWindow> targetMediaItemWindowHandle;
	MediaItemWindow *targetMediaItemWindow;
	WidgetHandle<MediaItemWindow> lastSelectedMediaItemWindowHandle;
	MediaItemWindow *lastSelectedMediaItemWindow;
	WidgetHandle<Panel> playlistHeaderPanelHandle;
	Panel *playlistHeaderPanel;
	WidgetHandle<Toggle> expandPlaylistsToggleHandle;
	Toggle *expandPlaylistsToggle;
	WidgetHandle<Button> createPlaylistButtonHandle;
	Button *createPlaylistButton;
	WidgetHandle<TextCardWindow> audioDisabledAlertWindowHandle;
	TextCardWindow *audioDisabledAlertWindow;
	int emptyStateType;
	int mediaWindowMode;
	int mediaSortOrder;
	int mediaDisplayCount;
	int mediaAvailableCount;
	std::list<MediaPlaylist> mediaPlaylists;
	HashMap mediaPlaylistWriteMap;
	SDL_mutex *mediaPlaylistMutex;
	bool isShowingPlaylists;
	bool isLoadingPlaylists;
	bool isLoadPlaylistsComplete;
	bool shouldWritePlaylists;
	bool isWritingPlaylists;
	bool isWritePlaylistsComplete;
	bool isLoadingMedia;
	StringList loadedRecordIds;
	StdString searchKey;
	StringList searchMediaItemIds;
	SDL_mutex *mediaSearchMutex;
	std::list<MediaSearch *> mediaSearchList;
	int64_t mediaSearchUpdateTime;
	int64_t lastRecordSyncTime;
	int searchRecordSyncClock;
	HashMap selectedMediaMap;
};
#endif
