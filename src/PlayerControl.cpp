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
#include "UiStack.h"
#include "Input.h"
#include "SoundMixer.h"
#include "PlayerWindow.h"
#include "MediaPlaylistWindow.h"
#include "PlayerControl.h"

constexpr const double mainPlayerWidthScale = 0.34f;
constexpr const double detachedPlayerWidthScale = 0.18f;
constexpr const double playerAspectRatio = 16.0f / 9.0f;

constexpr const int maxFullscreenPlayerRows = 4;
static const int fullscreenPlayerRowSizes[PlayerControl::maxPlayerCount][maxFullscreenPlayerRows] = {
	{ 1, 0, 0, 0 },
	{ 2, 0, 0, 0 },
	{ 2, 1, 0, 0 },
	{ 2, 2, 0, 0 },
	{ 2, 2, 1, 0 },
	{ 2, 2, 2, 0 },
	{ 3, 2, 2, 0 },
	{ 3, 2, 3, 0 },
	{ 3, 3, 3, 0 },
	{ 3, 4, 3, 0 },
	{ 4, 3, 4, 0 },
	{ 4, 4, 4, 0 },
	{ 4, 3, 3, 3 },
	{ 4, 4, 3, 3 },
	{ 4, 4, 4, 3 },
	{ 4, 4, 4, 4 },
	{ 5, 4, 4, 4 },
	{ 5, 4, 4, 5 },
	{ 5, 5, 5, 4 },
	{ 5, 5, 5, 5 }
};

PlayerControl::PlayerControl ()
: playerSoundMixVolume (SoundMixer::maxMixVolume)
, isPlayerSoundMuted (false)
, isFullscreenPlaying (false)
, mainPlayerHandle (&mainPlayer)
, lastZLevelStart (0)
, fullscreenKeyDownCount (0)
, fullscreenMouseLeftDownCount (0)
, fullscreenMouseRightDownCount (0)
{
	SdlUtil::createMutex (&playersMutex);
	SdlUtil::createMutex (&playlistsMutex);
}

PlayerControl::~PlayerControl () {
	clearPlayers ();
	clearPlaylists ();
	SdlUtil::destroyMutex (&playersMutex);
	SdlUtil::destroyMutex (&playlistsMutex);
}

void PlayerControl::clearPlayers () {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	PlayerWindow *player;

	SDL_LockMutex (playersMutex);
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		player->stop ();
		player->isDestroyed = true;
		player->release ();
		++i1;
	}
	players.clear ();
	SDL_UnlockMutex (playersMutex);
	mainPlayerHandle.clear ();
}

void PlayerControl::clearPlaylists () {
	std::list<MediaPlaylistWindow *>::const_iterator i1, i2;
	MediaPlaylistWindow *playlist;

	SDL_LockMutex (playlistsMutex);
	i1 = playlists.cbegin ();
	i2 = playlists.cend ();
	while (i1 != i2) {
		playlist = *i1;
		playlist->stop ();
		playlist->release ();
		++i1;
	}
	playlists.clear ();
	SDL_UnlockMutex (playlistsMutex);
}

int PlayerControl::getPlayerCount () {
	int count;

	SDL_LockMutex (playersMutex);
	count = players.size ();
	SDL_UnlockMutex (playersMutex);
	return (count);
}

int PlayerControl::getUnpausedPlayerCount () {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	int count;

	count = 0;
	SDL_LockMutex (playersMutex);
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		if (!((*i1)->isPaused ())) {
			++count;
		}
		++i1;
	}
	SDL_UnlockMutex (playersMutex);
	return (count);
}

int PlayerControl::getPausedPlayerCount () {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	int count;

	count = 0;
	SDL_LockMutex (playersMutex);
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		if ((*i1)->isPaused ()) {
			++count;
		}
		++i1;
	}
	SDL_UnlockMutex (playersMutex);
	return (count);
}

void PlayerControl::playMedia (const StdString &mediaId, int64_t seekTimestamp, bool isDetached) {
	PlayerWindow *player;

	if (getPlayerCount () >= PlayerControl::maxPlayerCount) {
		return;
	}
	SDL_LockMutex (playersMutex);
	unmaximizePlayers ();
	SDL_UnlockMutex (playersMutex);
	if (! isDetached) {
		if ((! mainPlayer) || mainPlayer->isDestroyed) {
			mainPlayerHandle.assign (createPlayerWindow (false));
		}
		player = mainPlayer;
	}
	else {
		player = createPlayerWindow (true);
	}
	player->setPlayMedia (mediaId);
	player->setPlaySeekTimestamp (seekTimestamp);
	player->play ();
	assignPlayerPositions (true);
}

void PlayerControl::playPlaylist (MediaPlaylistWindow *playlist) {
	std::list<MediaPlaylistWindow *>::const_iterator i1, i2;
	PlayerWindow *player;
	bool found;

	if (getPlayerCount () >= PlayerControl::maxPlayerCount) {
		return;
	}
	found = false;
	SDL_LockMutex (playlistsMutex);
	i1 = playlists.cbegin ();
	i2 = playlists.cend ();
	while (i1 != i2) {
		if (*i1 == playlist) {
			found = true;
			break;
		}
		++i1;
	}
	if (! found) {
		playlist->retain ();
		playlists.push_back (playlist);
	}
	SDL_UnlockMutex (playlistsMutex);
	if (found) {
		return;
	}

	SDL_LockMutex (playersMutex);
	unmaximizePlayers ();
	SDL_UnlockMutex (playersMutex);

	player = createPlayerWindow (true);
	playlist->play (player);
	assignPlayerPositions (true);
}

void PlayerControl::stopPlayers () {
	clearPlaylists ();
	clearPlayers ();
}

void PlayerControl::pausePlayers () {
	PlayerWindow *player;
	std::list<PlayerWindow *>::const_iterator i1, i2;
	bool targetstate;

	if (getUnpausedPlayerCount () > 0) {
		targetstate = false;
	}
	else if (getPausedPlayerCount () > 0) {
		targetstate = true;
	}
	else {
		return;
	}
	SDL_LockMutex (playersMutex);
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if (player->isPaused () == targetstate) {
			player->pause ();
		}
		++i1;
	}
	SDL_UnlockMutex (playersMutex);
}

void PlayerControl::stopPlaylists () {
	clearPlaylists ();
	assignPlayerPositions ();
}

void PlayerControl::stopPlaylist (const StdString &playlistId) {
	std::list<MediaPlaylistWindow *>::const_iterator i1, i2;
	MediaPlaylistWindow *playlist;
	bool reposition;

	reposition = false;
	SDL_LockMutex (playlistsMutex);
	i1 = playlists.cbegin ();
	i2 = playlists.cend ();
	while (i1 != i2) {
		playlist = *i1;
		if (playlist->playlist.id.equals (playlistId)) {
			reposition = true;
			playlist->stop ();
			playlist->release ();
			playlists.erase (i1);
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (playlistsMutex);
	if (reposition) {
		assignPlayerPositions ();
	}
}

PlayerWindow *PlayerControl::createPlayerWindow (bool isDetached) {
	PlayerWindow *player;
	double w, h;

	w = App::instance->drawableWidth;
	h = App::instance->drawableHeight;
	if (isDetached) {
		w *= detachedPlayerWidthScale;
		h *= detachedPlayerWidthScale;
	}
	else {
		w *= mainPlayerWidthScale;
		h *= mainPlayerWidthScale;
	}

	player = new PlayerWindow (floor (w), floor (h), playerSoundMixVolume, isPlayerSoundMuted);
	player->retain ();

	if (isDetached) {
		player->isDetached = true;
	}
	else {
		player->detachCallback = Widget::EventCallbackContext (PlayerControl::playerDetachClicked, this);
	}
	player->maximizeCallback = Widget::EventCallbackContext (PlayerControl::playerMaximizeClicked, this);

	SDL_LockMutex (playersMutex);
	players.push_back (player);
	SDL_UnlockMutex (playersMutex);

	App::instance->rootPanel->addWidget (player);
	return (player);
}

void PlayerControl::playerDetachClicked (void *itPtr, Widget *widgetPtr) {
	PlayerControl *it = (PlayerControl *) itPtr;
	PlayerWindow *player = (PlayerWindow *) widgetPtr;

	player->isDetached = true;
	player->setWindowSize (floor (App::instance->drawableWidth * detachedPlayerWidthScale), floor (App::instance->drawableWidth * detachedPlayerWidthScale / playerAspectRatio));
	if (it->mainPlayer == player) {
		it->mainPlayerHandle.clear ();
	}
	it->assignPlayerPositions ();
	it->assignPlayerZLevels (it->lastZLevelStart);
}

void PlayerControl::playerMaximizeClicked (void *itPtr, Widget *widgetPtr) {
	PlayerControl *it = (PlayerControl *) itPtr;
	PlayerWindow *player;

	player = (PlayerWindow *) widgetPtr;
	if (! player->isMaximized) {
		player->maximize (App::instance->drawableWidth, App::instance->drawableHeight - UiStack::instance->topBarHeight - UiStack::instance->bottomBarHeight, Position (0.0f, UiStack::instance->topBarHeight), false);
	}
	else {
		player->unmaximize ();
	}
	it->assignPlayerPositions (true);
	it->assignPlayerZLevels (it->lastZLevelStart);
}

void PlayerControl::update (int msElapsed) {
	std::list<PlayerWindow *>::iterator i1, i2;
	std::list<MediaPlaylistWindow *>::iterator j1, j2;
	PlayerWindow *player;
	MediaPlaylistWindow *playlist;
	bool reposition, found;

	mainPlayerHandle.compact ();
	if (mainPlayer) {
		playerSoundMixVolume = mainPlayer->soundMixVolume;
		isPlayerSoundMuted = mainPlayer->isSoundMuted;
	}

	SDL_LockMutex (playlistsMutex);
	j1 = playlists.begin ();
	j2 = playlists.end ();
	while (j1 != j2) {
		playlist = *j1;
		playlist->updatePlay (msElapsed);
		++j1;
	}
	while (true) {
		found = false;
		j1 = playlists.begin ();
		j2 = playlists.end ();
		while (j1 != j2) {
			playlist = *j1;
			if (! playlist->isExecuting) {
				playlist->release ();
				playlists.erase (j1);
				found = true;
				break;
			}
			++j1;
		}
		if (! found) {
			break;
		}
	}
	SDL_UnlockMutex (playlistsMutex);

	reposition = false;
	SDL_LockMutex (playersMutex);
	while (true) {
		found = false;
		i1 = players.begin ();
		i2 = players.end ();
		while (i1 != i2) {
			player = *i1;
			if ((isFullscreenPlaying || player->isDetached) && (! player->isPlaying ()) && player->playlistId.empty ()) {
				player->isDestroyed = true;
			}
			if (player->isDestroyed) {
				found = true;
				reposition = true;
				player->stop ();
				player->release ();
				players.erase (i1);
				break;
			}
			++i1;
		}
		if (! found) {
			break;
		}
	}
	SDL_UnlockMutex (playersMutex);

	if (isFullscreenPlaying) {
		SDL_LockMutex (playersMutex);
		if (players.empty ()) {
			isFullscreenPlaying = false;
		}
		else if ((fullscreenKeyDownCount != Input::instance->keyDownCount) || (fullscreenMouseLeftDownCount != Input::instance->mouseLeftDownCount) || (fullscreenMouseRightDownCount != Input::instance->mouseRightDownCount)) {
			unmaximizePlayers ();
			isFullscreenPlaying = false;
			reposition = true;
		}
		SDL_UnlockMutex (playersMutex);
	}

	if (reposition) {
		if (isFullscreenPlaying) {
			fullscreenPlayers ();
		}
		else {
			assignPlayerPositions ();
		}
	}
}

void PlayerControl::resize () {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	PlayerWindow *player;
	double w;

	SDL_LockMutex (playersMutex);
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if (player->isMaximized) {
			player->unmaximize ();
		}
		if (player == mainPlayer) {
			w = App::instance->drawableWidth * mainPlayerWidthScale;
		}
		else {
			w = App::instance->drawableWidth * detachedPlayerWidthScale;
		}
		player->setWindowSize (floor (w), floor (w / playerAspectRatio));
		++i1;
	}
	SDL_UnlockMutex (playersMutex);
	assignPlayerPositions (true);
}

void PlayerControl::fullscreenPlayers () {
	PlayerWindow *player;
	std::list<PlayerWindow *>::const_iterator i1, i2;
	int playercount, j, row, col, rowcount, colcount, z;
	int *rowsizes;
	double targetw, targeth, x, y, w, h;

	playercount = getPlayerCount ();
	if (playercount <= 0) {
		return;
	}
	if (playercount > PlayerControl::maxPlayerCount) {
		playercount = PlayerControl::maxPlayerCount;
	}
	isFullscreenPlaying = true;
	fullscreenKeyDownCount = Input::instance->keyDownCount;
	fullscreenMouseLeftDownCount = Input::instance->mouseLeftDownCount;
	fullscreenMouseRightDownCount = Input::instance->mouseRightDownCount;

	SDL_LockMutex (playersMutex);
	unmaximizePlayers ();
	rowsizes = (int *) fullscreenPlayerRowSizes[playercount - 1];
	rowcount = 0;
	for (j = 0; j < maxFullscreenPlayerRows; ++j) {
		if (rowsizes[j] <= 0) {
			break;
		}
		++rowcount;
	}
	colcount = rowsizes[0];
	targetw = floor (App::instance->drawableWidth / colcount);
	targeth = floor (App::instance->drawableHeight / rowcount);
	row = 0;
	col = 0;
	x = 0.0f;
	y = 0.0f;
	z = App::instance->rootPanel->maxWidgetZLevel + 1;
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		if (playercount <= 0) {
			break;
		}
		player = *i1;
		if (col >= colcount) {
			++row;
			if (row >= rowcount) {
				break;
			}
			col = 0;
			colcount = rowsizes[row];
			targetw = floor (App::instance->drawableWidth / colcount);
			x = 0.0f;
			y += targeth;
		}

		w = targetw;
		h = targeth;
		if (col >= (colcount - 1)) {
			w = App::instance->drawableWidth - x;
		}
		if (row >= (rowcount - 1)) {
			h = App::instance->drawableHeight - y;
		}
		if (player->isPaused ()) {
			player->pause ();
		}
		player->maximize (w, h, Position (x, y), true);
		player->zLevel = z;
		x += w;
		++z;
		++col;
		--playercount;
		++i1;
	}
	SDL_UnlockMutex (playersMutex);
}

void PlayerControl::assignPlayerPositions (bool disableAnimation) {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	PlayerWindow *player, *maxplayer;
	double x, y, xmain, ymain, xmax, xpos, ypos;

	SDL_LockMutex (playersMutex);

	xmain = App::instance->drawableWidth;
	ymain = App::instance->drawableHeight;
	maxplayer = NULL;
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if (player->isMaximized) {
			maxplayer = player;
			break;
		}
		++i1;
	}
	if (mainPlayer && (mainPlayer != maxplayer)) {
		xpos = App::instance->drawableWidth - mainPlayer->width;
		ypos = App::instance->drawableHeight - mainPlayer->height - UiStack::instance->bottomBarHeight;
		if (disableAnimation || mainPlayer->position.equals (0.0f, 0.0f)) {
			mainPlayer->position.assign (xpos, ypos);
		}
		else {
			mainPlayer->position.translate (xpos, ypos, UiConfiguration::instance->cardViewRepositionTranslateDuration);
		}
		xmain = xpos - UiConfiguration::instance->marginSize;
		ymain = ypos - UiConfiguration::instance->marginSize;
	}

	x = 0.0f;
	xmax = xmain;
	y = App::instance->drawableHeight - UiStack::instance->bottomBarHeight;
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if ((player != mainPlayer) && (player != maxplayer)) {
			if (maxplayer) {
				player->position.assign (0.0f, y - player->height);
			}
			else {
				if ((x + player->width) >= xmax) {
					x = 0.0f;
					if (y < ymain) {
						xmax = App::instance->drawableWidth;
					}
					y -= player->height;
				}

				xpos = x;
				ypos = y - player->height;
				if (disableAnimation || player->position.equals (0.0f, 0.0f)) {
					player->position.assign (xpos, ypos);
				}
				else {
					player->position.translate (xpos, ypos, UiConfiguration::instance->cardViewRepositionTranslateDuration);
				}
				x += player->width;
			}
		}
		++i1;
	}

	SDL_UnlockMutex (playersMutex);
}

int PlayerControl::assignPlayerZLevels (int zLevelStart) {
	std::list<PlayerWindow *>::const_iterator i1, i2;
	PlayerWindow *player, *maxplayer;
	int z, result;

	result = 0;
	z = zLevelStart;
	SDL_LockMutex (playersMutex);
	maxplayer = NULL;
	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if (player->isMaximized) {
			maxplayer = player;
			break;
		}
		++i1;
	}

	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		player = *i1;
		if (player != maxplayer) {
			player->zLevel = z;
			++z;
			++result;
		}
		++i1;
	}
	if (maxplayer) {
		maxplayer->zLevel = z;
		++z;
		++result;
	}
	SDL_UnlockMutex (playersMutex);

	lastZLevelStart = zLevelStart;
	return (result);
}

void PlayerControl::unmaximizePlayers () {
	std::list<PlayerWindow *>::const_iterator i1, i2;

	i1 = players.cbegin ();
	i2 = players.cend ();
	while (i1 != i2) {
		(*i1)->unmaximize ();
		++i1;
	}
}
