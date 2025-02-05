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
// Class that manages state for a set of player windows
#ifndef PLAYER_CONTROL_H
#define PLAYER_CONTROL_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class PlayerWindow;
class MediaPlaylistWindow;

class PlayerControl {
public:
	PlayerControl ();
	~PlayerControl ();

	static constexpr const int maxPlayerCount = 20;

	// Read-write data members
	int playerSoundMixVolume;
	bool isPlayerSoundMuted;
	int playerVisualizerType;
	bool playerSubtitleEnabled;

	// Read-only data members
	bool isFullscreenPlaying;

	// Return the number of players in the view
	int getPlayerCount ();

	// Return the number of players in the unpaused state
	int getUnpausedPlayerCount ();

	// Return the number of players in the paused state
	int getPausedPlayerCount ();

	// Start playback of a media item
	void playMedia (const StdString &mediaId, int64_t seekTimestamp = 0, bool isDetached = false);

	// Start playback of a media playlist
	void playPlaylist (MediaPlaylistWindow *playlist);

	// Execute the stop operation for all players
	void stopPlayers ();

	// Execute the pause/resume operation for all players
	void pausePlayers ();

	// Reposition players into fullscreen view
	void fullscreenPlayers ();

	// Execute the stop operation for all playlist players
	void stopPlaylists ();

	// Execute the stop operation for the specified playlist player
	void stopPlaylist (const StdString &playlistId);

	// Update player state as appropriate for elapsed time
	void update (int msElapsed);

	// Update player state as appropriate after a window resize event
	void resize ();

	// Assign positions to all players in the view
	void assignPlayerPositions (bool disableAnimation = false);

	// Assign zLevel values to all players in the view and return the number of zLevels spanned
	int assignPlayerZLevels (int zLevelStart);

private:
	// Callback functions
	static void playerDetachClicked (void *itPtr, Widget *widgetPtr);
	static void playerMaximizeClicked (void *itPtr, Widget *widgetPtr);
	static void playerSettingsChanged (void *itPtr, Widget *widgetPtr);

	// Return a newly created PlayerWindow widget that has been added to the view
	PlayerWindow *createPlayerWindow (bool isDetached);

	// Execute the unmaximize operation for all players. This method must only be invoked while holding a lock on playersMutex.
	void unmaximizePlayers ();

	// Remove all items from the players list
	void clearPlayers ();

	// Remove all items from the playlists list
	void clearPlaylists ();

	std::list<PlayerWindow *> players;
	SDL_mutex *playersMutex;
	WidgetHandle<PlayerWindow> mainPlayerHandle;
	PlayerWindow *mainPlayer;
	std::list<MediaPlaylistWindow *> playlists;
	SDL_mutex *playlistsMutex;
	int lastZLevelStart;
	int fullscreenKeyDownCount;
	int fullscreenMouseLeftDownCount;
	int fullscreenMouseRightDownCount;
};
#endif
