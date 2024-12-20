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
// String values that reference sprite assets
#ifndef SPRITE_ID_H
#define SPRITE_ID_H

class StringList;

class SpriteId {
public:
	static constexpr const char *MediaItemUi_prefix = "ui/MediaItemUi/";
	static constexpr const char *MediaItemUi_attributesIcon = "ui/MediaItemUi/sprite/attributesIcon";
	static constexpr const char *MediaItemUi_durationIcon = "ui/MediaItemUi/sprite/durationIcon";
	static constexpr const char *MediaItemUi_playMarkerButton = "ui/MediaItemUi/sprite/playMarkerButton";
	static constexpr const char *MediaItemUi_selectPlayPositionButton = "ui/MediaItemUi/sprite/selectPlayPositionButton";
	static constexpr const char *MediaItemUi_timeIcon = "ui/MediaItemUi/sprite/timeIcon";
	static constexpr const char *MediaItemUi_viewAfterButton = "ui/MediaItemUi/sprite/viewAfterButton";
	static constexpr const char *MediaItemUi_viewBeforeButton = "ui/MediaItemUi/sprite/viewBeforeButton";
	static constexpr const char *MediaPlaylistUi_prefix = "ui/MediaPlaylistUi/";
	static constexpr const char *MediaPlaylistUi_breadcrumbIcon = "ui/MediaPlaylistUi/sprite/breadcrumbIcon";
	static constexpr const char *MediaPlaylistUi_topButton = "ui/MediaPlaylistUi/sprite/topButton";
	static constexpr const char *MediaPlaylistUi_upButton = "ui/MediaPlaylistUi/sprite/upButton";
	static constexpr const char *PlayerUi_prefix = "ui/PlayerUi/";
	static constexpr const char *PlayerUi_breadcrumbIcon = "ui/PlayerUi/sprite/breadcrumbIcon";
	static constexpr const char *PlayerUi_createPlaylistButton = "ui/PlayerUi/sprite/createPlaylistButton";
	static constexpr const char *PlayerUi_fullscreenButton = "ui/PlayerUi/sprite/fullscreenButton";
	static constexpr const char *PlayerUi_layoutButton = "ui/PlayerUi/sprite/layoutButton";
	static constexpr const char *PlayerUi_mediaScanIcon = "ui/PlayerUi/sprite/mediaScanIcon";
	static constexpr const char *PlayerUi_searchButton = "ui/PlayerUi/sprite/searchButton";
	static constexpr const char *PlayerUi_searchStatusIcon = "ui/PlayerUi/sprite/searchStatusIcon";
	static constexpr const char *PlayerUi_sortButton = "ui/PlayerUi/sprite/sortButton";
	static constexpr const char *PlayerUi_stopButton = "ui/PlayerUi/sprite/stopButton";
	static constexpr const char *PlayerUi_tagButton = "ui/PlayerUi/sprite/tagButton";
	static constexpr const char *SpriteGroup_prefix = "ui/SpriteGroup/";
	static constexpr const char *SpriteGroup_aboutButton = "ui/SpriteGroup/sprite/aboutButton";
	static constexpr const char *SpriteGroup_addButton = "ui/SpriteGroup/sprite/addButton";
	static constexpr const char *SpriteGroup_addPlaylistItemButton = "ui/SpriteGroup/sprite/addPlaylistItemButton";
	static constexpr const char *SpriteGroup_appInfoIcon = "ui/SpriteGroup/sprite/appInfoIcon";
	static constexpr const char *SpriteGroup_appLogo = "ui/SpriteGroup/sprite/appLogo";
	static constexpr const char *SpriteGroup_appMenuButton = "ui/SpriteGroup/sprite/appMenuButton";
	static constexpr const char *SpriteGroup_audioIcon = "ui/SpriteGroup/sprite/audioIcon";
	static constexpr const char *SpriteGroup_backButton = "ui/SpriteGroup/sprite/backButton";
	static constexpr const char *SpriteGroup_browserPlayButton = "ui/SpriteGroup/sprite/browserPlayButton";
	static constexpr const char *SpriteGroup_cancelButton = "ui/SpriteGroup/sprite/cancelButton";
	static constexpr const char *SpriteGroup_checkmark = "ui/SpriteGroup/sprite/checkmark";
	static constexpr const char *SpriteGroup_cleanButton = "ui/SpriteGroup/sprite/cleanButton";
	static constexpr const char *SpriteGroup_clearListButton = "ui/SpriteGroup/sprite/clearListButton";
	static constexpr const char *SpriteGroup_clearTextButton = "ui/SpriteGroup/sprite/clearTextButton";
	static constexpr const char *SpriteGroup_consoleIcon = "ui/SpriteGroup/sprite/consoleIcon";
	static constexpr const char *SpriteGroup_deleteButton = "ui/SpriteGroup/sprite/deleteButton";
	static constexpr const char *SpriteGroup_detachPlayerButton = "ui/SpriteGroup/sprite/detachPlayerButton";
	static constexpr const char *SpriteGroup_directoryButton = "ui/SpriteGroup/sprite/directoryButton";
	static constexpr const char *SpriteGroup_directoryIcon = "ui/SpriteGroup/sprite/directoryIcon";
	static constexpr const char *SpriteGroup_durationIcon = "ui/SpriteGroup/sprite/durationIcon";
	static constexpr const char *SpriteGroup_editPlaylistButton = "ui/SpriteGroup/sprite/editPlaylistButton";
	static constexpr const char *SpriteGroup_enterTextButton = "ui/SpriteGroup/sprite/enterTextButton";
	static constexpr const char *SpriteGroup_exitButton = "ui/SpriteGroup/sprite/exitButton";
	static constexpr const char *SpriteGroup_expandAllLessButton = "ui/SpriteGroup/sprite/expandAllLessButton";
	static constexpr const char *SpriteGroup_expandAllMoreButton = "ui/SpriteGroup/sprite/expandAllMoreButton";
	static constexpr const char *SpriteGroup_expandLessButton = "ui/SpriteGroup/sprite/expandLessButton";
	static constexpr const char *SpriteGroup_expandMoreButton = "ui/SpriteGroup/sprite/expandMoreButton";
	static constexpr const char *SpriteGroup_feedbackButton = "ui/SpriteGroup/sprite/feedbackButton";
	static constexpr const char *SpriteGroup_fileIcon = "ui/SpriteGroup/sprite/fileIcon";
	static constexpr const char *SpriteGroup_forwardButton = "ui/SpriteGroup/sprite/forwardButton";
	static constexpr const char *SpriteGroup_helpButton = "ui/SpriteGroup/sprite/helpButton";
	static constexpr const char *SpriteGroup_historyButton = "ui/SpriteGroup/sprite/historyButton";
	static constexpr const char *SpriteGroup_infoIcon = "ui/SpriteGroup/sprite/infoIcon";
	static constexpr const char *SpriteGroup_keyboardButton = "ui/SpriteGroup/sprite/keyboardButton";
	static constexpr const char *SpriteGroup_largeComputerIcon = "ui/SpriteGroup/sprite/largeComputerIcon";
	static constexpr const char *SpriteGroup_largeErrorIcon = "ui/SpriteGroup/sprite/largeErrorIcon";
	static constexpr const char *SpriteGroup_largeMediaIcon = "ui/SpriteGroup/sprite/largeMediaIcon";
	static constexpr const char *SpriteGroup_largeSizeButton = "ui/SpriteGroup/sprite/largeSizeButton";
	static constexpr const char *SpriteGroup_loadingImageErrorIcon = "ui/SpriteGroup/sprite/loadingImageErrorIcon";
	static constexpr const char *SpriteGroup_loadingImageIcon = "ui/SpriteGroup/sprite/loadingImageIcon";
	static constexpr const char *SpriteGroup_mediaScanButton = "ui/SpriteGroup/sprite/mediaScanButton";
	static constexpr const char *SpriteGroup_mediumSizeButton = "ui/SpriteGroup/sprite/mediumSizeButton";
	static constexpr const char *SpriteGroup_messageIcon = "ui/SpriteGroup/sprite/messageIcon";
	static constexpr const char *SpriteGroup_networkActivityIcon = "ui/SpriteGroup/sprite/networkActivityIcon";
	static constexpr const char *SpriteGroup_nextItemButton = "ui/SpriteGroup/sprite/nextItemButton";
	static constexpr const char *SpriteGroup_okButton = "ui/SpriteGroup/sprite/okButton";
	static constexpr const char *SpriteGroup_openButton = "ui/SpriteGroup/sprite/openButton";
	static constexpr const char *SpriteGroup_pasteButton = "ui/SpriteGroup/sprite/pasteButton";
	static constexpr const char *SpriteGroup_pauseButton = "ui/SpriteGroup/sprite/pauseButton";
	static constexpr const char *SpriteGroup_playButton = "ui/SpriteGroup/sprite/playButton";
	static constexpr const char *SpriteGroup_playlistActiveIcon = "ui/SpriteGroup/sprite/playlistActiveIcon";
	static constexpr const char *SpriteGroup_playlistIcon = "ui/SpriteGroup/sprite/playlistIcon";
	static constexpr const char *SpriteGroup_playlistItemIcon = "ui/SpriteGroup/sprite/playlistItemIcon";
	static constexpr const char *SpriteGroup_pointerIcon = "ui/SpriteGroup/sprite/pointerIcon";
	static constexpr const char *SpriteGroup_randomizeButton = "ui/SpriteGroup/sprite/randomizeButton";
	static constexpr const char *SpriteGroup_renameButton = "ui/SpriteGroup/sprite/renameButton";
	static constexpr const char *SpriteGroup_rewindButton = "ui/SpriteGroup/sprite/rewindButton";
	static constexpr const char *SpriteGroup_scrollDownArrow = "ui/SpriteGroup/sprite/scrollDownArrow";
	static constexpr const char *SpriteGroup_scrollUpArrow = "ui/SpriteGroup/sprite/scrollUpArrow";
	static constexpr const char *SpriteGroup_selectImageSizeButton = "ui/SpriteGroup/sprite/selectImageSizeButton";
	static constexpr const char *SpriteGroup_settingsBoxButton = "ui/SpriteGroup/sprite/settingsBoxButton";
	static constexpr const char *SpriteGroup_settingsGearButton = "ui/SpriteGroup/sprite/settingsGearButton";
	static constexpr const char *SpriteGroup_shuffleIcon = "ui/SpriteGroup/sprite/shuffleIcon";
	static constexpr const char *SpriteGroup_skipNextButton = "ui/SpriteGroup/sprite/skipNextButton";
	static constexpr const char *SpriteGroup_skipPreviousButton = "ui/SpriteGroup/sprite/skipPreviousButton";
	static constexpr const char *SpriteGroup_smallErrorIcon = "ui/SpriteGroup/sprite/smallErrorIcon";
	static constexpr const char *SpriteGroup_smallLoadingIcon = "ui/SpriteGroup/sprite/smallLoadingIcon";
	static constexpr const char *SpriteGroup_smallMediaIcon = "ui/SpriteGroup/sprite/smallMediaIcon";
	static constexpr const char *SpriteGroup_smallPlaylistIcon = "ui/SpriteGroup/sprite/smallPlaylistIcon";
	static constexpr const char *SpriteGroup_smallSizeButton = "ui/SpriteGroup/sprite/smallSizeButton";
	static constexpr const char *SpriteGroup_smallStreamIcon = "ui/SpriteGroup/sprite/smallStreamIcon";
	static constexpr const char *SpriteGroup_soundOffButton = "ui/SpriteGroup/sprite/soundOffButton";
	static constexpr const char *SpriteGroup_soundOnButton = "ui/SpriteGroup/sprite/soundOnButton";
	static constexpr const char *SpriteGroup_starButton = "ui/SpriteGroup/sprite/starButton";
	static constexpr const char *SpriteGroup_starHalfButton = "ui/SpriteGroup/sprite/starHalfButton";
	static constexpr const char *SpriteGroup_starOutlineButton = "ui/SpriteGroup/sprite/starOutlineButton";
	static constexpr const char *SpriteGroup_startPositionIcon = "ui/SpriteGroup/sprite/startPositionIcon";
	static constexpr const char *SpriteGroup_storageIcon = "ui/SpriteGroup/sprite/storageIcon";
	static constexpr const char *SpriteGroup_tagIcon = "ui/SpriteGroup/sprite/tagIcon";
	static constexpr const char *SpriteGroup_taskInProgressIcon = "ui/SpriteGroup/sprite/taskInProgressIcon";
	static constexpr const char *SpriteGroup_toggleCheckbox = "ui/SpriteGroup/sprite/toggleCheckbox";
	static constexpr const char *SpriteGroup_toggleCheckboxIndeterminate = "ui/SpriteGroup/sprite/toggleCheckboxIndeterminate";
	static constexpr const char *SpriteGroup_toggleCheckboxOutline = "ui/SpriteGroup/sprite/toggleCheckboxOutline";
	static constexpr const char *SpriteGroup_updateButton = "ui/SpriteGroup/sprite/updateButton";
	static constexpr const char *SpriteGroup_updateIcon = "ui/SpriteGroup/sprite/updateIcon";
	static constexpr const char *SpriteGroup_versionIcon = "ui/SpriteGroup/sprite/versionIcon";
	static constexpr const char *SpriteGroup_visibilityOffButton = "ui/SpriteGroup/sprite/visibilityOffButton";
	static constexpr const char *SpriteGroup_visibilityOnButton = "ui/SpriteGroup/sprite/visibilityOnButton";
	static constexpr const char *SpriteGroup_webLinkIcon = "ui/SpriteGroup/sprite/webLinkIcon";

	// Return a list of all spriteId const values, with an optional prefix to filter results
	static StringList getSpriteIds ();
	static StringList getSpriteIds (const StdString &prefix);
};
#endif
