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
#include "StringList.h"
#include "SpriteId.h"

StringList SpriteId::getSpriteIds () {
	StringList s;

	s.push_back (SpriteId::MediaItemUi_attributesIcon);
	s.push_back (SpriteId::MediaItemUi_durationIcon);
	s.push_back (SpriteId::MediaItemUi_playMarkerButton);
	s.push_back (SpriteId::MediaItemUi_selectPlayPositionButton);
	s.push_back (SpriteId::MediaItemUi_timeIcon);
	s.push_back (SpriteId::MediaItemUi_viewAfterButton);
	s.push_back (SpriteId::MediaItemUi_viewBeforeButton);
	s.push_back (SpriteId::MediaPlaylistUi_breadcrumbIcon);
	s.push_back (SpriteId::MediaPlaylistUi_topButton);
	s.push_back (SpriteId::MediaPlaylistUi_upButton);
	s.push_back (SpriteId::PlayerUi_breadcrumbIcon);
	s.push_back (SpriteId::PlayerUi_createPlaylistButton);
	s.push_back (SpriteId::PlayerUi_fullscreenButton);
	s.push_back (SpriteId::PlayerUi_layoutButton);
	s.push_back (SpriteId::PlayerUi_mediaScanIcon);
	s.push_back (SpriteId::PlayerUi_searchButton);
	s.push_back (SpriteId::PlayerUi_searchStatusIcon);
	s.push_back (SpriteId::PlayerUi_sortButton);
	s.push_back (SpriteId::PlayerUi_stopButton);
	s.push_back (SpriteId::PlayerUi_tagButton);
	s.push_back (SpriteId::SpriteGroup_aboutButton);
	s.push_back (SpriteId::SpriteGroup_addButton);
	s.push_back (SpriteId::SpriteGroup_addPlaylistItemButton);
	s.push_back (SpriteId::SpriteGroup_appInfoIcon);
	s.push_back (SpriteId::SpriteGroup_appLogo);
	s.push_back (SpriteId::SpriteGroup_appMenuButton);
	s.push_back (SpriteId::SpriteGroup_audioIcon);
	s.push_back (SpriteId::SpriteGroup_backButton);
	s.push_back (SpriteId::SpriteGroup_browserPlayButton);
	s.push_back (SpriteId::SpriteGroup_cancelButton);
	s.push_back (SpriteId::SpriteGroup_checkmark);
	s.push_back (SpriteId::SpriteGroup_cleanButton);
	s.push_back (SpriteId::SpriteGroup_clearListButton);
	s.push_back (SpriteId::SpriteGroup_clearTextButton);
	s.push_back (SpriteId::SpriteGroup_consoleIcon);
	s.push_back (SpriteId::SpriteGroup_deleteButton);
	s.push_back (SpriteId::SpriteGroup_detachPlayerButton);
	s.push_back (SpriteId::SpriteGroup_directoryButton);
	s.push_back (SpriteId::SpriteGroup_directoryIcon);
	s.push_back (SpriteId::SpriteGroup_durationIcon);
	s.push_back (SpriteId::SpriteGroup_editPlaylistButton);
	s.push_back (SpriteId::SpriteGroup_enterTextButton);
	s.push_back (SpriteId::SpriteGroup_exitButton);
	s.push_back (SpriteId::SpriteGroup_expandAllLessButton);
	s.push_back (SpriteId::SpriteGroup_expandAllMoreButton);
	s.push_back (SpriteId::SpriteGroup_expandLessButton);
	s.push_back (SpriteId::SpriteGroup_expandMoreButton);
	s.push_back (SpriteId::SpriteGroup_feedbackButton);
	s.push_back (SpriteId::SpriteGroup_fileIcon);
	s.push_back (SpriteId::SpriteGroup_forwardButton);
	s.push_back (SpriteId::SpriteGroup_helpButton);
	s.push_back (SpriteId::SpriteGroup_historyButton);
	s.push_back (SpriteId::SpriteGroup_infoIcon);
	s.push_back (SpriteId::SpriteGroup_keyboardButton);
	s.push_back (SpriteId::SpriteGroup_largeComputerIcon);
	s.push_back (SpriteId::SpriteGroup_largeErrorIcon);
	s.push_back (SpriteId::SpriteGroup_largeMediaIcon);
	s.push_back (SpriteId::SpriteGroup_largeSizeButton);
	s.push_back (SpriteId::SpriteGroup_loadingImageErrorIcon);
	s.push_back (SpriteId::SpriteGroup_loadingImageIcon);
	s.push_back (SpriteId::SpriteGroup_mediaScanButton);
	s.push_back (SpriteId::SpriteGroup_mediumSizeButton);
	s.push_back (SpriteId::SpriteGroup_messageIcon);
	s.push_back (SpriteId::SpriteGroup_networkActivityIcon);
	s.push_back (SpriteId::SpriteGroup_nextItemButton);
	s.push_back (SpriteId::SpriteGroup_okButton);
	s.push_back (SpriteId::SpriteGroup_openButton);
	s.push_back (SpriteId::SpriteGroup_pasteButton);
	s.push_back (SpriteId::SpriteGroup_pauseButton);
	s.push_back (SpriteId::SpriteGroup_playButton);
	s.push_back (SpriteId::SpriteGroup_playlistActiveIcon);
	s.push_back (SpriteId::SpriteGroup_playlistIcon);
	s.push_back (SpriteId::SpriteGroup_playlistItemIcon);
	s.push_back (SpriteId::SpriteGroup_pointerIcon);
	s.push_back (SpriteId::SpriteGroup_randomizeButton);
	s.push_back (SpriteId::SpriteGroup_renameButton);
	s.push_back (SpriteId::SpriteGroup_rewindButton);
	s.push_back (SpriteId::SpriteGroup_scrollDownArrow);
	s.push_back (SpriteId::SpriteGroup_scrollUpArrow);
	s.push_back (SpriteId::SpriteGroup_selectImageSizeButton);
	s.push_back (SpriteId::SpriteGroup_settingsBoxButton);
	s.push_back (SpriteId::SpriteGroup_settingsGearButton);
	s.push_back (SpriteId::SpriteGroup_shuffleIcon);
	s.push_back (SpriteId::SpriteGroup_skipNextButton);
	s.push_back (SpriteId::SpriteGroup_skipPreviousButton);
	s.push_back (SpriteId::SpriteGroup_smallErrorIcon);
	s.push_back (SpriteId::SpriteGroup_smallLoadingIcon);
	s.push_back (SpriteId::SpriteGroup_smallMediaIcon);
	s.push_back (SpriteId::SpriteGroup_smallPlaylistIcon);
	s.push_back (SpriteId::SpriteGroup_smallSizeButton);
	s.push_back (SpriteId::SpriteGroup_smallStreamIcon);
	s.push_back (SpriteId::SpriteGroup_soundOffButton);
	s.push_back (SpriteId::SpriteGroup_soundOnButton);
	s.push_back (SpriteId::SpriteGroup_starButton);
	s.push_back (SpriteId::SpriteGroup_starHalfButton);
	s.push_back (SpriteId::SpriteGroup_starOutlineButton);
	s.push_back (SpriteId::SpriteGroup_startPositionIcon);
	s.push_back (SpriteId::SpriteGroup_storageIcon);
	s.push_back (SpriteId::SpriteGroup_tagIcon);
	s.push_back (SpriteId::SpriteGroup_taskInProgressIcon);
	s.push_back (SpriteId::SpriteGroup_toggleCheckbox);
	s.push_back (SpriteId::SpriteGroup_toggleCheckboxIndeterminate);
	s.push_back (SpriteId::SpriteGroup_toggleCheckboxOutline);
	s.push_back (SpriteId::SpriteGroup_updateButton);
	s.push_back (SpriteId::SpriteGroup_updateIcon);
	s.push_back (SpriteId::SpriteGroup_versionIcon);
	s.push_back (SpriteId::SpriteGroup_visibilityOffButton);
	s.push_back (SpriteId::SpriteGroup_visibilityOnButton);
	s.push_back (SpriteId::SpriteGroup_webLinkIcon);
	return (s);
}

StringList SpriteId::getSpriteIds (const StdString &prefix) {
	StringList s;

	if (StdString (SpriteId::MediaItemUi_attributesIcon).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_attributesIcon);
	}
	if (StdString (SpriteId::MediaItemUi_durationIcon).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_durationIcon);
	}
	if (StdString (SpriteId::MediaItemUi_playMarkerButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_playMarkerButton);
	}
	if (StdString (SpriteId::MediaItemUi_selectPlayPositionButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_selectPlayPositionButton);
	}
	if (StdString (SpriteId::MediaItemUi_timeIcon).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_timeIcon);
	}
	if (StdString (SpriteId::MediaItemUi_viewAfterButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_viewAfterButton);
	}
	if (StdString (SpriteId::MediaItemUi_viewBeforeButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaItemUi_viewBeforeButton);
	}
	if (StdString (SpriteId::MediaPlaylistUi_breadcrumbIcon).startsWith (prefix)) {
		s.push_back (SpriteId::MediaPlaylistUi_breadcrumbIcon);
	}
	if (StdString (SpriteId::MediaPlaylistUi_topButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaPlaylistUi_topButton);
	}
	if (StdString (SpriteId::MediaPlaylistUi_upButton).startsWith (prefix)) {
		s.push_back (SpriteId::MediaPlaylistUi_upButton);
	}
	if (StdString (SpriteId::PlayerUi_breadcrumbIcon).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_breadcrumbIcon);
	}
	if (StdString (SpriteId::PlayerUi_createPlaylistButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_createPlaylistButton);
	}
	if (StdString (SpriteId::PlayerUi_fullscreenButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_fullscreenButton);
	}
	if (StdString (SpriteId::PlayerUi_layoutButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_layoutButton);
	}
	if (StdString (SpriteId::PlayerUi_mediaScanIcon).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_mediaScanIcon);
	}
	if (StdString (SpriteId::PlayerUi_searchButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_searchButton);
	}
	if (StdString (SpriteId::PlayerUi_searchStatusIcon).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_searchStatusIcon);
	}
	if (StdString (SpriteId::PlayerUi_sortButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_sortButton);
	}
	if (StdString (SpriteId::PlayerUi_stopButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_stopButton);
	}
	if (StdString (SpriteId::PlayerUi_tagButton).startsWith (prefix)) {
		s.push_back (SpriteId::PlayerUi_tagButton);
	}
	if (StdString (SpriteId::SpriteGroup_aboutButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_aboutButton);
	}
	if (StdString (SpriteId::SpriteGroup_addButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_addButton);
	}
	if (StdString (SpriteId::SpriteGroup_addPlaylistItemButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_addPlaylistItemButton);
	}
	if (StdString (SpriteId::SpriteGroup_appInfoIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_appInfoIcon);
	}
	if (StdString (SpriteId::SpriteGroup_appLogo).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_appLogo);
	}
	if (StdString (SpriteId::SpriteGroup_appMenuButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_appMenuButton);
	}
	if (StdString (SpriteId::SpriteGroup_audioIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_audioIcon);
	}
	if (StdString (SpriteId::SpriteGroup_backButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_backButton);
	}
	if (StdString (SpriteId::SpriteGroup_browserPlayButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_browserPlayButton);
	}
	if (StdString (SpriteId::SpriteGroup_cancelButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_cancelButton);
	}
	if (StdString (SpriteId::SpriteGroup_checkmark).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_checkmark);
	}
	if (StdString (SpriteId::SpriteGroup_cleanButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_cleanButton);
	}
	if (StdString (SpriteId::SpriteGroup_clearListButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_clearListButton);
	}
	if (StdString (SpriteId::SpriteGroup_clearTextButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_clearTextButton);
	}
	if (StdString (SpriteId::SpriteGroup_consoleIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_consoleIcon);
	}
	if (StdString (SpriteId::SpriteGroup_deleteButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_deleteButton);
	}
	if (StdString (SpriteId::SpriteGroup_detachPlayerButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_detachPlayerButton);
	}
	if (StdString (SpriteId::SpriteGroup_directoryButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_directoryButton);
	}
	if (StdString (SpriteId::SpriteGroup_directoryIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_directoryIcon);
	}
	if (StdString (SpriteId::SpriteGroup_durationIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_durationIcon);
	}
	if (StdString (SpriteId::SpriteGroup_editPlaylistButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_editPlaylistButton);
	}
	if (StdString (SpriteId::SpriteGroup_enterTextButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_enterTextButton);
	}
	if (StdString (SpriteId::SpriteGroup_exitButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_exitButton);
	}
	if (StdString (SpriteId::SpriteGroup_expandAllLessButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_expandAllLessButton);
	}
	if (StdString (SpriteId::SpriteGroup_expandAllMoreButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_expandAllMoreButton);
	}
	if (StdString (SpriteId::SpriteGroup_expandLessButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_expandLessButton);
	}
	if (StdString (SpriteId::SpriteGroup_expandMoreButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_expandMoreButton);
	}
	if (StdString (SpriteId::SpriteGroup_feedbackButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_feedbackButton);
	}
	if (StdString (SpriteId::SpriteGroup_fileIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_fileIcon);
	}
	if (StdString (SpriteId::SpriteGroup_forwardButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_forwardButton);
	}
	if (StdString (SpriteId::SpriteGroup_helpButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_helpButton);
	}
	if (StdString (SpriteId::SpriteGroup_historyButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_historyButton);
	}
	if (StdString (SpriteId::SpriteGroup_infoIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_infoIcon);
	}
	if (StdString (SpriteId::SpriteGroup_keyboardButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_keyboardButton);
	}
	if (StdString (SpriteId::SpriteGroup_largeComputerIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_largeComputerIcon);
	}
	if (StdString (SpriteId::SpriteGroup_largeErrorIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_largeErrorIcon);
	}
	if (StdString (SpriteId::SpriteGroup_largeMediaIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_largeMediaIcon);
	}
	if (StdString (SpriteId::SpriteGroup_largeSizeButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_largeSizeButton);
	}
	if (StdString (SpriteId::SpriteGroup_loadingImageErrorIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_loadingImageErrorIcon);
	}
	if (StdString (SpriteId::SpriteGroup_loadingImageIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_loadingImageIcon);
	}
	if (StdString (SpriteId::SpriteGroup_mediaScanButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_mediaScanButton);
	}
	if (StdString (SpriteId::SpriteGroup_mediumSizeButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_mediumSizeButton);
	}
	if (StdString (SpriteId::SpriteGroup_messageIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_messageIcon);
	}
	if (StdString (SpriteId::SpriteGroup_networkActivityIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_networkActivityIcon);
	}
	if (StdString (SpriteId::SpriteGroup_nextItemButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_nextItemButton);
	}
	if (StdString (SpriteId::SpriteGroup_okButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_okButton);
	}
	if (StdString (SpriteId::SpriteGroup_openButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_openButton);
	}
	if (StdString (SpriteId::SpriteGroup_pasteButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_pasteButton);
	}
	if (StdString (SpriteId::SpriteGroup_pauseButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_pauseButton);
	}
	if (StdString (SpriteId::SpriteGroup_playButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_playButton);
	}
	if (StdString (SpriteId::SpriteGroup_playlistActiveIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_playlistActiveIcon);
	}
	if (StdString (SpriteId::SpriteGroup_playlistIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_playlistIcon);
	}
	if (StdString (SpriteId::SpriteGroup_playlistItemIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_playlistItemIcon);
	}
	if (StdString (SpriteId::SpriteGroup_pointerIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_pointerIcon);
	}
	if (StdString (SpriteId::SpriteGroup_randomizeButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_randomizeButton);
	}
	if (StdString (SpriteId::SpriteGroup_renameButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_renameButton);
	}
	if (StdString (SpriteId::SpriteGroup_rewindButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_rewindButton);
	}
	if (StdString (SpriteId::SpriteGroup_scrollDownArrow).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_scrollDownArrow);
	}
	if (StdString (SpriteId::SpriteGroup_scrollUpArrow).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_scrollUpArrow);
	}
	if (StdString (SpriteId::SpriteGroup_selectImageSizeButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_selectImageSizeButton);
	}
	if (StdString (SpriteId::SpriteGroup_settingsBoxButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_settingsBoxButton);
	}
	if (StdString (SpriteId::SpriteGroup_settingsGearButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_settingsGearButton);
	}
	if (StdString (SpriteId::SpriteGroup_shuffleIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_shuffleIcon);
	}
	if (StdString (SpriteId::SpriteGroup_skipNextButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_skipNextButton);
	}
	if (StdString (SpriteId::SpriteGroup_skipPreviousButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_skipPreviousButton);
	}
	if (StdString (SpriteId::SpriteGroup_smallErrorIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallErrorIcon);
	}
	if (StdString (SpriteId::SpriteGroup_smallLoadingIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallLoadingIcon);
	}
	if (StdString (SpriteId::SpriteGroup_smallMediaIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallMediaIcon);
	}
	if (StdString (SpriteId::SpriteGroup_smallPlaylistIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallPlaylistIcon);
	}
	if (StdString (SpriteId::SpriteGroup_smallSizeButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallSizeButton);
	}
	if (StdString (SpriteId::SpriteGroup_smallStreamIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_smallStreamIcon);
	}
	if (StdString (SpriteId::SpriteGroup_soundOffButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_soundOffButton);
	}
	if (StdString (SpriteId::SpriteGroup_soundOnButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_soundOnButton);
	}
	if (StdString (SpriteId::SpriteGroup_starButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_starButton);
	}
	if (StdString (SpriteId::SpriteGroup_starHalfButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_starHalfButton);
	}
	if (StdString (SpriteId::SpriteGroup_starOutlineButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_starOutlineButton);
	}
	if (StdString (SpriteId::SpriteGroup_startPositionIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_startPositionIcon);
	}
	if (StdString (SpriteId::SpriteGroup_storageIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_storageIcon);
	}
	if (StdString (SpriteId::SpriteGroup_tagIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_tagIcon);
	}
	if (StdString (SpriteId::SpriteGroup_taskInProgressIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_taskInProgressIcon);
	}
	if (StdString (SpriteId::SpriteGroup_toggleCheckbox).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_toggleCheckbox);
	}
	if (StdString (SpriteId::SpriteGroup_toggleCheckboxIndeterminate).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_toggleCheckboxIndeterminate);
	}
	if (StdString (SpriteId::SpriteGroup_toggleCheckboxOutline).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_toggleCheckboxOutline);
	}
	if (StdString (SpriteId::SpriteGroup_updateButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_updateButton);
	}
	if (StdString (SpriteId::SpriteGroup_updateIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_updateIcon);
	}
	if (StdString (SpriteId::SpriteGroup_versionIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_versionIcon);
	}
	if (StdString (SpriteId::SpriteGroup_visibilityOffButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_visibilityOffButton);
	}
	if (StdString (SpriteId::SpriteGroup_visibilityOnButton).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_visibilityOnButton);
	}
	if (StdString (SpriteId::SpriteGroup_webLinkIcon).startsWith (prefix)) {
		s.push_back (SpriteId::SpriteGroup_webLinkIcon);
	}
	return (s);
}
