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
#include "Int64List.h"
#include "Database.h"
#include "RecordStore.h"
#include "MediaItem.h"
#include "MediaPlaylist.h"

MediaPlaylist::MediaPlaylist ()
: isExpanded (false)
, isShuffle (false)
, startPosition (MediaPlaylist::ZeroStartPosition)
, playDuration (MediaPlaylist::FullPlayDuration)
{
}
MediaPlaylist::~MediaPlaylist () {
}

void MediaPlaylist::clear () {
	id.assign ("");
	name.assign ("");
	isExpanded = false;
	isShuffle = false;
	startPosition = MediaPlaylist::ZeroStartPosition;
	playDuration = MediaPlaylist::FullPlayDuration;
	items.clear ();
}

void MediaPlaylist::copyValues (const MediaPlaylist &source) {
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;

	id.assign (source.id);
	name.assign (source.name);
	isExpanded = source.isExpanded;
	isShuffle = source.isShuffle;
	startPosition = source.startPosition;
	playDuration = source.playDuration;

	items.clear ();
	i1 = source.items.cbegin ();
	i2 = source.items.cend ();
	while (i1 != i2) {
		items.push_back (MediaPlaylistItem (*i1));
		++i1;
	}
}

void MediaPlaylist::resetId () {
	id = RecordStore::instance->getRecordId (MediaPlaylist::idCommandType);
}

constexpr const char *selectPlaylistSql = "SELECT id, name, isExpanded, isShuffle, startPosition, playDuration FROM ";
constexpr const int selectPlaylistColumnCount = 6;
constexpr const char *selectPlaylistItemSql = "SELECT id, name, mediaPath, duration, isVideo, isAudio, hasAudioAlbumArt, width, height, playSeekTimestamp FROM ";
constexpr const int selectPlaylistItemColumnCount = 10;
bool MediaPlaylist::copyDatabaseRowValues (int columnCount, char **columnValues, char **columnNames) {
	char *val;
	int i;

	if (columnCount < selectPlaylistColumnCount) {
		return (false);
	}
	i = 0;
	val = columnValues[i];
	id.assign (val ? val : "");

	++i;
	val = columnValues[i];
	name.assign (val ? val : "");

	++i;
	val = columnValues[i];
	isExpanded = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	isShuffle = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	startPosition = val ? StdString (val).parsedInt ((int) 0) : 0;

	++i;
	val = columnValues[i];
	playDuration = val ? StdString (val).parsedInt ((int) 0) : 0;

	return (true);
}

bool MediaPlaylist::readDatabaseRows (const StdString &databasePath, const char *tableName, StdString *errorMessage, std::list<MediaPlaylist> *destList) {
	StdString sql;
	OpResult result;
	std::list<MediaPlaylist>::iterator i1, i2;

	destList->clear ();
	if (errorMessage) {
		errorMessage->assign ("");
	}
	sql.assign (selectPlaylistSql);
	sql.append (tableName);
	sql.append (";");
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaPlaylist::readDatabaseRows_playlistRow, destList);
	if (result != OpResult::Success) {
		return (false);
	}
	i1 = destList->begin ();
	i2 = destList->end ();
	while (i1 != i2) {
		sql.assign (selectPlaylistItemSql);
		sql.append (tableName);
		sql.append ("Item WHERE playlistId=");
		sql.append (Database::getColumnValueSql (i1->id));
		sql.append (" ORDER BY sortKey ASC;");
		result = Database::instance->exec (databasePath, sql, errorMessage, MediaPlaylist::readDatabaseRows_itemRow, &(i1->items));
		++i1;
	}
	return (true);
}
int MediaPlaylist::readDatabaseRows_playlistRow (void *destListPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaPlaylist playlist;

	if (! playlist.copyDatabaseRowValues (columnCount, columnValues, columnNames)) {
		return (-1);
	}
	((std::list<MediaPlaylist> *) destListPtr)->push_back (playlist);
	return (0);
}
int MediaPlaylist::readDatabaseRows_itemRow (void *itemVectorPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaPlaylistItem item;
	char *val;
	int i;

	if (columnCount < selectPlaylistItemColumnCount) {
		return (-1);
	}
	i = 0;
	val = columnValues[i];
	item.id.assign (val ? val : "");

	++i;
	val = columnValues[i];
	item.name.assign (val ? val : "");

	++i;
	val = columnValues[i];
	item.mediaPath.assign (val ? val : "");

	++i;
	val = columnValues[i];
	item.duration = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	item.isVideo = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	item.isAudio = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	item.hasAudioAlbumArt = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	item.width = val ? StdString (val).parsedInt ((int) 0) : 0;

	++i;
	val = columnValues[i];
	item.height = val ? StdString (val).parsedInt ((int) 0) : 0;

	++i;
	val = columnValues[i];
	item.playSeekTimestamp = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	((std::vector<MediaPlaylistItem> *) itemVectorPtr)->push_back (item);
	return (0);
}

void MediaPlaylist::getUpsertSql (const char *tableName, StringList *destList) const {
	StringList fields;
	StdString sql, update;
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;
	int i, sortkey;

	destList->clear ();
	if (id.empty () || name.empty ()) {
		return;
	}
	fields.push_back (StdString ("id"));
	fields.push_back (Database::getColumnValueSql (id));
	fields.push_back (StdString ("name"));
	fields.push_back (Database::getColumnValueSql (name));
	fields.push_back (StdString ("isExpanded"));
	fields.push_back (Database::getColumnValueSql (isExpanded));
	fields.push_back (StdString ("isShuffle"));
	fields.push_back (Database::getColumnValueSql (isShuffle));
	fields.push_back (StdString ("startPosition"));
	fields.push_back (Database::getColumnValueSql (startPosition));
	fields.push_back (StdString ("playDuration"));
	fields.push_back (Database::getColumnValueSql (playDuration));

	sql.assign ("INSERT INTO ");
	sql.append (Database::getRowInsertSql (StdString (tableName), fields));
	for (i = 0; i < 2; ++i) {
		fields.erase (fields.begin ());
	}
	update = Database::getRowUpdateSql (fields);

	sql.append (" ON CONFLICT(id) DO UPDATE SET ");
	sql.append (update);
	sql.append (";");
	destList->push_back (sql);

	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append ("Item WHERE playlistId=");
	sql.append (Database::getColumnValueSql (id));
	sql.append (";");
	destList->push_back (sql);
	sortkey = 1;
	i1 = items.cbegin ();
	i2 = items.cend ();
	while (i1 != i2) {
		fields.clear ();
		fields.push_back (StdString ("id"));
		fields.push_back (Database::getColumnValueSql (i1->id));
		fields.push_back (StdString ("playlistId"));
		fields.push_back (Database::getColumnValueSql (id));
		fields.push_back (StdString ("sortKey"));
		fields.push_back (Database::getColumnValueSql (sortkey));
		fields.push_back (StdString ("name"));
		fields.push_back (Database::getColumnValueSql (i1->name));
		fields.push_back (StdString ("mediaPath"));
		fields.push_back (Database::getColumnValueSql (i1->mediaPath));
		fields.push_back (StdString ("duration"));
		fields.push_back (Database::getColumnValueSql (i1->duration));
		fields.push_back (StdString ("isVideo"));
		fields.push_back (Database::getColumnValueSql (i1->isVideo));
		fields.push_back (StdString ("isAudio"));
		fields.push_back (Database::getColumnValueSql (i1->isAudio));
		fields.push_back (StdString ("hasAudioAlbumArt"));
		fields.push_back (Database::getColumnValueSql (i1->hasAudioAlbumArt));
		fields.push_back (StdString ("width"));
		fields.push_back (Database::getColumnValueSql (i1->width));
		fields.push_back (StdString ("height"));
		fields.push_back (Database::getColumnValueSql (i1->height));
		fields.push_back (StdString ("playSeekTimestamp"));
		fields.push_back (Database::getColumnValueSql (i1->playSeekTimestamp));
		sql.assign ("INSERT INTO ");
		sql.append (Database::getRowInsertSql (StdString::createSprintf ("%sItem", tableName), fields));
		sql.append (";");
		destList->push_back (sql);
		++sortkey;
		++i1;
	}
}

void MediaPlaylist::getDeleteSql (const char *tableName, StringList *destList) const {
	StdString sql;

	destList->clear ();
	if (id.empty ()) {
		return;
	}
	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append (" WHERE id=");
	sql.append (Database::getColumnValueSql (id));
	sql.append (";");
	destList->push_back (sql);

	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append ("Item WHERE playlistId=");
	sql.append (Database::getColumnValueSql (id));
	sql.append (";");
	destList->push_back (sql);
}

StdString MediaPlaylist::getCreateTableSql (const char *tableName) {
	StdString sql;

	sql.assign ("CREATE TABLE IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("(id TEXT PRIMARY KEY, name TEXT, isExpanded INTEGER, isShuffle INTEGER, startPosition INTEGER, playDuration INTEGER);");

	sql.append ("CREATE TABLE IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("Item(id TEXT PRIMARY KEY, playlistId TEXT, sortKey INTEGER, name TEXT, mediaPath TEXT, duration INTEGER, isVideo INTEGER, isAudio INTEGER, hasAudioAlbumArt INTEGER, width INTEGER, height INTEGER, playSeekTimestamp INTEGER);");

	sql.append ("CREATE INDEX IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("PlaylistId ON ");
	sql.append (tableName);
	sql.append ("Item(playlistId);");

	return (sql);
}

StdString MediaPlaylist::getDeleteAllSql (const char *tableName) {
	StdString sql;

	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append (";");

	sql.append ("DELETE FROM ");
	sql.append (tableName);
	sql.append ("Item;");

	return (sql);
}

void MediaPlaylist::getStartPositionRange (int *minStartPosition, int *maxStartPosition) {
	int min, max;

	min = 0;
	max = 0;
	switch (startPosition) {
		case MediaPlaylist::NearBeginningStartPosition: {
			min = 10;
			max = 20;
			break;
		}
		case MediaPlaylist::MiddleStartPosition: {
			min = 35;
			max = 60;
			break;
		}
		case MediaPlaylist::NearEndStartPosition: {
			min = 75;
			max = 90;
			break;
		}
		case MediaPlaylist::FullRangeStartPosition: {
			min = 0;
			max = 99;
			break;
		}
	}
	if (minStartPosition) {
		*minStartPosition = min;
	}
	if (maxStartPosition) {
		*maxStartPosition = max;
	}
}

void MediaPlaylist::getPlayDurationRange (int64_t *minPlayDuration, int64_t *maxPlayDuration) {
	int64_t min, max;

	min = 0;
	max = 0;
	switch (playDuration) {
		case MediaPlaylist::VeryShortPlayDuration: {
			min = 15000;
			max = 45000;
			break;
		}
		case MediaPlaylist::ShortPlayDuration: {
			min = 60000;
			max = 180000;
			break;
		}
		case MediaPlaylist::MediumPlayDuration: {
			min = 300000;
			max = 900000;
			break;
		}
		case MediaPlaylist::LongPlayDuration: {
			min = 1800000;
			max = 3600000;
			break;
		}
		case MediaPlaylist::VeryLongPlayDuration: {
			min = 7200000;
			max = 14400000;
			break;
		}
		case MediaPlaylist::FullPlayDuration: {
			min = 0;
			max = 0;
			break;
		}
	}
	if (minPlayDuration) {
		*minPlayDuration = min;
	}
	if (maxPlayDuration) {
		*maxPlayDuration = max;
	}
}

MediaPlaylistItem::MediaPlaylistItem ()
: duration (0)
, isVideo (false)
, isAudio (false)
, hasAudioAlbumArt (false)
, width (0)
, height (0)
, playSeekTimestamp (0)
{
}
MediaPlaylistItem::MediaPlaylistItem (const MediaPlaylistItem &mediaPlaylistItem)
: MediaPlaylistItem ()
{
	id.assign (mediaPlaylistItem.id);
	name.assign (mediaPlaylistItem.name);
	mediaPath.assign (mediaPlaylistItem.mediaPath);
	duration = mediaPlaylistItem.duration;
	isVideo = mediaPlaylistItem.isVideo;
	isAudio = mediaPlaylistItem.isAudio;
	hasAudioAlbumArt = mediaPlaylistItem.hasAudioAlbumArt;
	width = mediaPlaylistItem.width;
	height = mediaPlaylistItem.height;
	playSeekTimestamp = mediaPlaylistItem.playSeekTimestamp;
}
MediaPlaylistItem::MediaPlaylistItem (const MediaItem &mediaItem)
: MediaPlaylistItem ()
{
	readMediaItem (mediaItem);
}
MediaPlaylistItem::~MediaPlaylistItem () {
}

void MediaPlaylistItem::readMediaItem (const MediaItem &mediaItem) {
	id.assign (mediaItem.id);
	name.assign (mediaItem.name);
	mediaPath.assign (mediaItem.mediaPath);
	duration = mediaItem.duration;
	isVideo = mediaItem.isVideo;
	isAudio = mediaItem.isAudio;
	hasAudioAlbumArt = mediaItem.hasAudioAlbumArt;
	width = mediaItem.width;
	height = mediaItem.height;
	playSeekTimestamp = mediaItem.playSeekTimestamp;
}

void MediaPlaylistItem::resetId () {
	id = RecordStore::instance->getRecordId (MediaPlaylist::idCommandType);
}
