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
#include "OsUtil.h"
#include "Json.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Database.h"
#include "MediaReader.h"
#include "MediaItem.h"

const StdString MediaItem::createTableSql = StdString ("CREATE TABLE IF NOT EXISTS MediaItem(id TEXT PRIMARY KEY, name TEXT, mediaPath TEXT, mediaDirname TEXT, thumbnailTimestamps TEXT, mtime INTEGER, duration INTEGER, mediaFileSize INTEGER, totalBitrate INTEGER, isVideo INTEGER, isAudio INTEGER, hasAudioAlbumArt INTEGER, frameRate REAL, videoBitrate INTEGER, width INTEGER, height INTEGER, audioSampleRate INTEGER, audioChannels INTEGER, audioBitrate INTEGER, tags TEXT, sortKey TEXT); CREATE UNIQUE INDEX IF NOT EXISTS MediaItemPath ON MediaItem(mediaPath); CREATE INDEX IF NOT EXISTS MediaItemSortKey ON MediaItem(sortKey); CREATE INDEX IF NOT EXISTS MediaItemDirname ON MediaItem(mediaDirname); CREATE INDEX IF NOT EXISTS MediaItemMtime ON MediaItem(mtime);");
const StdString MediaItem::sortKeyCharacters = StdString ("abcdefghijklmnopqrstuvwxyz0123456789");
constexpr const char *selectSql = "SELECT id, name, mediaPath, mediaDirname, thumbnailTimestamps, mtime, duration, mediaFileSize, totalBitrate, isVideo, isAudio, hasAudioAlbumArt, frameRate, videoBitrate, width, height, audioSampleRate, audioChannels, audioBitrate, tags, sortKey FROM MediaItem";
constexpr const int selectColumnCount = 20;

MediaItem::MediaItem ()
: mtime (0)
, duration (0)
, mediaFileSize (0)
, totalBitrate (0)
, isVideo (false)
, isAudio (false)
, hasAudioAlbumArt (false)
, frameRate (0.0f)
, videoBitrate (0)
, width (0)
, height (0)
, audioSampleRate (0)
, audioChannels (0)
, audioBitrate (0)
{
}
MediaItem::~MediaItem () {
}

StdString MediaItem::toString () const {
	StdString s;
	return (s);
}

void MediaItem::clear (const StdString &mediaIdValue) {
	mediaId.assign (mediaIdValue);
	agentId.assign ("");
	name.assign ("");
	mediaPath.assign ("");
	mediaDirname.assign ("");
	mtime = 0;
	duration = 0;
	mediaFileSize = 0;
	totalBitrate = 0;
	isVideo = false;
	isAudio = false;
	hasAudioAlbumArt = false;
	frameRate = 0.0f;
	videoBitrate = 0;
	width = 0;
	height = 0;
	audioSampleRate = 0;
	audioChannels = 0;
	audioBitrate = 0;
	thumbnailTimestamps.clear ();
	tags.clear ();
	sortKey.assign ("");
}

void MediaItem::copyValues (const MediaItem &source) {
	mediaId.assign (source.mediaId);
	agentId.assign (source.agentId);
	name.assign (source.name);
	mediaPath.assign (source.mediaPath);
	mediaDirname.assign (source.mediaDirname);
	mtime = source.mtime;
	duration = source.duration;
	mediaFileSize = source.mediaFileSize;
	totalBitrate = source.totalBitrate;
	isVideo = source.isVideo;
	isAudio = source.isAudio;
	hasAudioAlbumArt = source.hasAudioAlbumArt;
	frameRate = source.frameRate;
	videoBitrate = source.videoBitrate;
	width = source.width;
	height = source.height;
	audioSampleRate = source.audioSampleRate;
	audioChannels = source.audioChannels;
	audioBitrate = source.audioBitrate;
	thumbnailTimestamps.assign (source.thumbnailTimestamps);
	tags.assign (source.tags);
	sortKey.assign (source.sortKey);
}

bool MediaItem::isValid () const {
	if (name.empty () || mediaPath.empty () || (duration <= 0)) {
		return (false);
	}
	if (!(isVideo || isAudio)) {
		return (false);
	}
	if (isVideo) {
		if ((frameRate <= 0.0f) || (width <= 0) || (height <= 0)) {
			return (false);
		}
	}
	if (isAudio) {
		if ((audioSampleRate <= 0) || (audioChannels <= 0)) {
			return (false);
		}
	}
	return (true);
}

Json *MediaItem::createRecord (const StdString &agentIdValue) const {
	SystemInterface::Prefix prefix;
	Json *params;

	prefix.createTime = OsUtil::getTime ();
	prefix.agentId.assign ((! agentIdValue.empty ()) ? agentIdValue : agentId);

	params = new Json ();
	params->set (SystemInterface::Field_id, mediaId);
	params->set (SystemInterface::Field_name, name);
	params->set (SystemInterface::Field_mediaPath, mediaPath);
	params->set (SystemInterface::Field_mediaDirname, mediaDirname);
	params->set (SystemInterface::Field_thumbnailTimestamps, thumbnailTimestamps);
	params->set (SystemInterface::Field_mtime, mtime);
	params->set (SystemInterface::Field_duration, duration);
	params->set (SystemInterface::Field_mediaFileSize, mediaFileSize);
	params->set (SystemInterface::Field_totalBitrate, totalBitrate);
	params->set (SystemInterface::Field_isVideo, isVideo);
	params->set (SystemInterface::Field_isAudio, isAudio);
	params->set (SystemInterface::Field_hasAudioAlbumArt, hasAudioAlbumArt);
	params->set (SystemInterface::Field_frameRate, frameRate);
	params->set (SystemInterface::Field_videoBitrate, videoBitrate);
	params->set (SystemInterface::Field_width, width);
	params->set (SystemInterface::Field_height, height);
	params->set (SystemInterface::Field_audioSampleRate, audioSampleRate);
	params->set (SystemInterface::Field_audioChannels, audioChannels);
	params->set (SystemInterface::Field_audioBitrate, audioBitrate);
	params->set (SystemInterface::Field_tags, tags);
	params->set (SystemInterface::Field_sortKey, sortKey);
	return (SystemInterface::instance->createCommand (prefix, SystemInterface::Command_MediaItem, params));
}

bool MediaItem::readRecord (Json *record) {
	clear ();
	if (! record) {
		return (false);
	}
	agentId = SystemInterface::instance->getCommandAgentId (record);
	mediaId = SystemInterface::instance->getCommandStringParam (record, SystemInterface::Field_id, "");
	name = SystemInterface::instance->getCommandStringParam (record, SystemInterface::Field_name, "");
	mediaPath = SystemInterface::instance->getCommandStringParam (record, SystemInterface::Field_mediaPath, "");
	mediaDirname = SystemInterface::instance->getCommandStringParam (record, SystemInterface::Field_mediaDirname, "");
	mtime = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_mtime, (int64_t) 0);
	duration = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_duration, (int64_t) 0);
	mediaFileSize = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_mediaFileSize, (int64_t) 0);
	totalBitrate = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_totalBitrate, (int64_t) 0);
	isVideo = SystemInterface::instance->getCommandBooleanParam (record, SystemInterface::Field_isVideo, false);
	isAudio = SystemInterface::instance->getCommandBooleanParam (record, SystemInterface::Field_isAudio, false);
	hasAudioAlbumArt = SystemInterface::instance->getCommandBooleanParam (record, SystemInterface::Field_hasAudioAlbumArt, false);
	frameRate = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_frameRate, (double) 0.0f);
	videoBitrate = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_videoBitrate, (int64_t) 0);
	width = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_width, (int) 0);
	height = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_height, (int) 0);
	audioSampleRate = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_audioSampleRate, (int) 0);
	audioChannels = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_audioChannels, (int) 0);
	audioBitrate = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_audioBitrate, (int64_t) 0);
	SystemInterface::instance->getCommandNumberArrayParam (record, SystemInterface::Field_thumbnailTimestamps, &thumbnailTimestamps, 0, true);
	SystemInterface::instance->getCommandStringArrayParam (record, SystemInterface::Field_tags, &tags, true);
	sortKey = SystemInterface::instance->getCommandStringParam (record, SystemInterface::Field_sortKey, "");
	if (! isValid ()) {
		return (false);
	}
	return (true);
}

bool MediaItem::readRecordStore (const StdString &mediaIdValue) {
	Json record;

	clear ();
	if (! RecordStore::instance->find (&record, mediaIdValue, SystemInterface::CommandId_MediaItem)) {
		return (false);
	}
	return (readRecord (&record));
}

bool MediaItem::readMediaReader (const MediaReader &reader) {
	duration = reader.duration;
	mediaFileSize = reader.mediaFileSize;
	totalBitrate = reader.totalBitrate;
	isVideo = reader.isVideo;
	isAudio = reader.isAudio;
	hasAudioAlbumArt = reader.hasAudioAlbumArt;
	frameRate = reader.videoFrameRate;
	videoBitrate = reader.videoBitrate;
	width = reader.videoFrameWidth;
	height = reader.videoFrameHeight;
	audioChannels = reader.audioChannelCount;
	audioSampleRate = reader.audioSampleRate;
	audioBitrate = reader.audioBitrate;
	return (isValid ());
}

bool MediaItem::copyDatabaseRowValues (int columnCount, char **columnValues, char **columnNames) {
	int i;
	char *val;

	if (columnCount < selectColumnCount) {
		return (false);
	}
	i = 0;
	val = columnValues[i];
	mediaId.assign (val ? val : "");

	++i;
	val = columnValues[i];
	name.assign (val ? val : "");

	++i;
	val = columnValues[i];
	mediaPath.assign (val ? val : "");

	++i;
	val = columnValues[i];
	mediaDirname.assign (val ? val : "");

	++i;
	val = columnValues[i];
	if ((! val) || (! thumbnailTimestamps.parseJsonString (StdString (val)))) {
		thumbnailTimestamps.clear ();
	}

	++i;
	val = columnValues[i];
	mtime = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	duration = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	mediaFileSize = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	totalBitrate = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	isVideo = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	isAudio = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	hasAudioAlbumArt = val ? (*val != '0') : false;

	++i;
	val = columnValues[i];
	frameRate = val ? StdString (val).parsedFloat ((double) 0.0f) : 0.0f;

	++i;
	val = columnValues[i];
	videoBitrate = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	width = val ? StdString (val).parsedInt (0) : 0;

	++i;
	val = columnValues[i];
	height = val ? StdString (val).parsedInt (0) : 0;

	++i;
	val = columnValues[i];
	audioSampleRate = val ? StdString (val).parsedInt (0) : 0;

	++i;
	val = columnValues[i];
	audioChannels = val ? StdString (val).parsedInt (0) : 0;

	++i;
	val = columnValues[i];
	audioBitrate = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	if ((! val) || (! tags.parseJsonString (StdString (val)))) {
		tags.clear ();
	}

	++i;
	val = columnValues[i];
	sortKey.assign (val ? val : "");

	return (true);
}

bool MediaItem::readDatabaseMediaPathRow (const StdString &databasePath, StdString *errorMessage, const StdString &mediaPathValue) {
	StdString sql;
	int result;

	clear ();
	sql.sprintf ("%s WHERE mediaPath=", selectSql);
	sql.append (Database::getColumnValueSql (mediaPathValue));
	sql.append (";");

	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::readDatabaseRow_row, this);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return ((! mediaId.empty ()) && isValid ());
}
bool MediaItem::readDatabaseMediaIdRow (const StdString &databasePath, StdString *errorMessage, const StdString &mediaIdValue) {
	StdString sql;
	int result;

	clear ();
	sql.sprintf ("%s WHERE id=", selectSql);
	sql.append (Database::getColumnValueSql (mediaIdValue));
	sql.append (";");

	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::readDatabaseRow_row, this);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return ((! mediaId.empty ()) && isValid ());
}
int MediaItem::readDatabaseRow_row (void *itPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaItem *it = (MediaItem *) itPtr;

	return (it->copyDatabaseRowValues (columnCount, columnValues, columnNames) ? 0 : -1);
}

bool MediaItem::readDatabaseRows (const StdString &databasePath, StdString *errorMessage, std::list<MediaItem> *destList, const StdString &searchKey, int offset, int limit, int sortOrder) {
	StdString sql;
	OpResult result;

	destList->clear ();
	sql.assign (selectSql);
	sql.append (MediaItem::getSelectWhereSql (searchKey));
	if (sortOrder == SystemInterface::Constant_NewestSort) {
		sql.append (" ORDER BY mtime DESC");
	}
	else if (sortOrder == SystemInterface::Constant_FilePathSort) {
		sql.append (" ORDER BY mediaDirname ASC, sortKey ASC");
	}
	else {
		sql.append (" ORDER BY sortKey ASC");
	}
	if (limit > 0) {
		sql.appendSprintf (" LIMIT %i", limit);
		if (offset > 0) {
			sql.appendSprintf (" OFFSET %i", offset);
		}
	}
	sql.append (";");
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::readDatabaseRows_row, destList);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
int MediaItem::readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaItem item;

	if (! item.copyDatabaseRowValues (columnCount, columnValues, columnNames)) {
		return (-1);
	}
	((std::list<MediaItem> *) destListPtr)->push_back (item);
	return (0);
}

bool MediaItem::readDatabaseMetadata (const StdString &databasePath, StdString *errorMessage, int64_t *mediaSizeTotal, int64_t *mediaDurationTotal) {
	StdString sql;
	OpResult result;
	int64_t sizetotal, durationtotal;

	sizetotal = 0;
	durationtotal = 0;
	sql.assign ("SELECT SUM(mediaFileSize) FROM MediaItem;");
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::readDatabaseMetadata_row, &sizetotal);
	if (result != OpResult::Success) {
		return (false);
	}

	sql.assign ("SELECT SUM(duration) FROM MediaItem;");
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::readDatabaseMetadata_row, &durationtotal);
	if (result != OpResult::Success) {
		return (false);
	}

	if (mediaSizeTotal) {
		*mediaSizeTotal = sizetotal;
	}
	if (mediaDurationTotal) {
		*mediaDurationTotal = durationtotal;
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
int MediaItem::readDatabaseMetadata_row (void *int64Ptr, int columnCount, char **columnValues, char **columnNames) {
	char *val;

	if (columnCount < 1) {
		return (-1);
	}
	val = columnValues[0];
	*((int64_t *) int64Ptr) = val ? StdString (val).parsedInt ((int64_t) 0) : 0;
	return (0);
}

int MediaItem::countDatabaseRecords (const StdString &databasePath, StdString *errorMessage, const StdString &searchKey) {
	StdString sql;
	OpResult result;
	int count;

	sql.assign ("SELECT COUNT(*) FROM MediaItem");
	sql.append (MediaItem::getSelectWhereSql (searchKey));
	sql.append (";");
	count = 0;
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaItem::countDatabaseRecords_row, &count);
	if (result != OpResult::Success) {
		return (-1);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (count);
}
int MediaItem::countDatabaseRecords_row (void *intPtr, int columnCount, char **columnValues, char **columnNames) {
	char *val;

	if (columnCount < 1) {
		return (-1);
	}
	val = columnValues[0];
	*((int *) intPtr) = val ? StdString (val).parsedInt (0) : 0;
	return (0);
}

StdString MediaItem::getUpsertSql () const {
	StdString s, update;
	StringList fields;
	int i;

	if (mediaId.empty () || (! isValid ())) {
		return (StdString ());
	}
	fields.push_back (StdString ("id"));
	fields.push_back (Database::getColumnValueSql (mediaId));
	fields.push_back (StdString ("name"));
	fields.push_back (Database::getColumnValueSql (name));
	fields.push_back (StdString ("mediaPath"));
	fields.push_back (Database::getColumnValueSql (mediaPath));
	fields.push_back (StdString ("mediaDirname"));
	fields.push_back (Database::getColumnValueSql (mediaDirname));
	fields.push_back (StdString ("thumbnailTimestamps"));
	fields.push_back (Database::getColumnValueSql (thumbnailTimestamps.toJsonString ()));
	fields.push_back (StdString ("mtime"));
	fields.push_back (Database::getColumnValueSql (mtime));
	fields.push_back (StdString ("duration"));
	fields.push_back (Database::getColumnValueSql (duration));
	fields.push_back (StdString ("mediaFileSize"));
	fields.push_back (Database::getColumnValueSql (mediaFileSize));
	fields.push_back (StdString ("totalBitrate"));
	fields.push_back (Database::getColumnValueSql (totalBitrate));
	fields.push_back (StdString ("isVideo"));
	fields.push_back (Database::getColumnValueSql (isVideo));
	fields.push_back (StdString ("isAudio"));
	fields.push_back (Database::getColumnValueSql (isAudio));
	fields.push_back (StdString ("hasAudioAlbumArt"));
	fields.push_back (Database::getColumnValueSql (hasAudioAlbumArt));
	fields.push_back (StdString ("frameRate"));
	fields.push_back (Database::getColumnValueSql (frameRate));
	fields.push_back (StdString ("videoBitrate"));
	fields.push_back (Database::getColumnValueSql (videoBitrate));
	fields.push_back (StdString ("width"));
	fields.push_back (Database::getColumnValueSql (width));
	fields.push_back (StdString ("height"));
	fields.push_back (Database::getColumnValueSql (height));
	fields.push_back (StdString ("audioSampleRate"));
	fields.push_back (Database::getColumnValueSql (audioSampleRate));
	fields.push_back (StdString ("audioChannels"));
	fields.push_back (Database::getColumnValueSql (audioChannels));
	fields.push_back (StdString ("audioBitrate"));
	fields.push_back (Database::getColumnValueSql (audioBitrate));
	fields.push_back (StdString ("tags"));
	fields.push_back (Database::getColumnValueSql (tags.toJsonString ()));
	fields.push_back (StdString ("sortKey"));
	fields.push_back (Database::getColumnValueSql (sortKey));
	s.assign ("INSERT INTO ");
	s.append (Database::getRowInsertSql (StdString ("MediaItem"), fields));

	for (i = 0; i < 6; ++i) {
		fields.erase (fields.begin ());
	}
	update = Database::getRowUpdateSql (fields);

	s.append (" ON CONFLICT(id) DO UPDATE SET ");
	s.append (update);
	s.append (" ON CONFLICT(mediaPath) DO UPDATE SET ");
	s.append (update);
	s.append (";");
	return (s);
}

StdString MediaItem::getSelectWhereSql (const StdString &searchKey) {
	StdString sql, key, tag, chars;

	if (searchKey.empty ()) {
		return (StdString ());
	}
	if (searchKey.contains ("/") || searchKey.contains ("\\")) {
		key.assign (searchKey);
		key.replace (StdString ("*"), StdString ("%"));
		if (! key.endsWith ("%")) {
			key.append ("%");
		}
		sql.assign (" WHERE (mediaPath LIKE ");
		sql.append (Database::getColumnValueSql (key));
		sql.append (")");
		return (sql);
	}
	chars.assign (MediaItem::sortKeyCharacters);
	chars.append ("*");
	key = searchKey.lowercased ().filtered (chars);
	if (! key.empty ()) {
		key.replace (StdString ("*"), StdString ("%"));
		if (! key.endsWith ("%")) {
			key.append ("%");
		}
		sql.assign (" WHERE (sortKey LIKE ");
		sql.append (Database::getColumnValueSql (key));
		sql.append (") OR (tags LIKE ");
		tag.assign ("[\"");
		tag.append (key);
		tag.append ("\"%");
		sql.append (Database::getColumnValueSql (tag));
		sql.append (") OR (tags LIKE ");
		tag.assign ("%,\"");
		tag.append (key);
		tag.append ("\"%");
		sql.append (Database::getColumnValueSql (tag));
		sql.append (")");
	}
	return (sql);
}

StdString MediaItem::getUpdateTagsSql (const StdString &mediaId, const StringList &tags) {
	StdString sql;

	if (mediaId.empty ()) {
		return (StdString ());
	}
	sql.assign ("UPDATE MediaItem SET tags=");
	sql.append (Database::getColumnValueSql (tags.empty () ? StdString () : tags.toJsonString ()));
	sql.append (" WHERE id=");
	sql.append (Database::getColumnValueSql (mediaId));
	sql.append (";");
	return (sql);
}

StdString MediaItem::getDeleteSql (const StdString &mediaId) {
	StdString sql;

	if (mediaId.empty ()) {
		return (StdString ());
	}
	sql.assign ("DELETE FROM MediaItem WHERE id=");
	sql.append (Database::getColumnValueSql (mediaId));
	sql.append (";");
	return (sql);
}

StdString MediaItem::getDeleteAllSql () {
	return (StdString ("DELETE FROM MediaItem;"));
}
