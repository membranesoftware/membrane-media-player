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
#include "MediaPlaylist.h"

const StdString MediaPlaylist::createTableSql = StdString ("CREATE TABLE IF NOT EXISTS MediaPlaylist(id TEXT, name TEXT, mediaIds TEXT, startTimestamps TEXT, isExpanded INTEGER, isShuffle INTEGER, startPosition INTEGER, playDuration INTEGER); CREATE UNIQUE INDEX IF NOT EXISTS MediaPlaylistId ON MediaPlaylist(id);");
constexpr const char *selectSql = "SELECT id, name, mediaIds, startTimestamps, isExpanded, isShuffle, startPosition, playDuration FROM MediaPlaylist";
constexpr const int selectColumnCount = 8;

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
		items.push_back (MediaPlaylistItem (i1->mediaId, i1->startTimestamp));
		++i1;
	}
}

bool MediaPlaylist::readDatabaseRows (const StdString &databasePath, StdString *errorMessage, std::list<MediaPlaylist> *destList) {
	StdString sql;
	OpResult result;

	destList->clear ();
	sql.assign (selectSql);
	sql.append (";");
	result = Database::instance->exec (databasePath, sql, errorMessage, MediaPlaylist::readDatabaseRows_row, destList);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
int MediaPlaylist::readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames) {
	MediaPlaylist playlist;

	if (! playlist.copyDatabaseRowValues (columnCount, columnValues, columnNames)) {
		return (-1);
	}
	((std::list<MediaPlaylist> *) destListPtr)->push_back (playlist);
	return (0);
}

bool MediaPlaylist::copyDatabaseRowValues (int columnCount, char **columnValues, char **columnNames) {
	char *val;
	StringList mediaids;
	StringList::const_iterator i;
	Int64List starttimestamps;
	Int64List::const_iterator j;
	int count;

	if (columnCount < selectColumnCount) {
		return (false);
	}
	items.clear ();

	val = columnValues[0];
	id.assign (val ? val : "");

	val = columnValues[1];
	name.assign (val ? val : "");

	val = columnValues[2];
	if ((! val) || (! mediaids.parseJsonString (StdString (val)))) {
		mediaids.clear ();
	}

	val = columnValues[3];
	if ((! val) || (! starttimestamps.parseJsonString (StdString (val)))) {
		starttimestamps.clear ();
	}

	count = (int) mediaids.size ();
	if (count < (int) starttimestamps.size ()) {
		count = (int) starttimestamps.size ();
	}
	i = mediaids.cbegin ();
	j = starttimestamps.cbegin ();
	while (count > 0) {
		items.push_back (MediaPlaylistItem (*i, *j));
		++i;
		++j;
		--count;
	}

	val = columnValues[4];
	isExpanded = val ? (*val != '0') : false;

	val = columnValues[5];
	isShuffle = val ? (*val != '0') : false;

	val = columnValues[6];
	startPosition = val ? StdString (val).parsedInt ((int) 0) : 0;

	val = columnValues[7];
	playDuration = val ? StdString (val).parsedInt ((int) 0) : 0;

	return (true);
}

StdString MediaPlaylist::getUpsertSql () const {
	StdString sql, update;
	StringList mediaids, fields;
	Int64List starttimestamps;
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;
	int j;

	if (name.empty ()) {
		return (StdString ());
	}
	i1 = items.cbegin ();
	i2 = items.cend ();
	while (i1 != i2) {
		mediaids.push_back (i1->mediaId);
		starttimestamps.push_back (i1->startTimestamp);
		++i1;
	}

	fields.push_back (StdString ("id"));
	fields.push_back (Database::getColumnValueSql (id));
	fields.push_back (StdString ("name"));
	fields.push_back (Database::getColumnValueSql (name));
	fields.push_back (StdString ("mediaIds"));
	fields.push_back (Database::getColumnValueSql (mediaids.toJsonString ()));
	fields.push_back (StdString ("startTimestamps"));
	fields.push_back (Database::getColumnValueSql (starttimestamps.toJsonString ()));
	fields.push_back (StdString ("isExpanded"));
	fields.push_back (Database::getColumnValueSql (isExpanded));
	fields.push_back (StdString ("isShuffle"));
	fields.push_back (Database::getColumnValueSql (isShuffle));
	fields.push_back (StdString ("startPosition"));
	fields.push_back (Database::getColumnValueSql (startPosition));
	fields.push_back (StdString ("playDuration"));
	fields.push_back (Database::getColumnValueSql (playDuration));
	sql.assign ("INSERT INTO ");
	sql.append (Database::getRowInsertSql (StdString ("MediaPlaylist"), fields));

	for (j = 0; j < 2; ++j) {
		fields.erase (fields.begin ());
	}
	update = Database::getRowUpdateSql (fields);

	sql.append (" ON CONFLICT(id) DO UPDATE SET ");
	sql.append (update);
	sql.append (";");
	return (sql);
}

StdString MediaPlaylist::getDeleteAllSql () {
	return (StdString ("DELETE FROM MediaPlaylist;"));
}

StdString MediaPlaylist::getDeleteExcludeSql (const std::list<MediaPlaylist> &excludeList) {
	StdString sql;
	std::list<MediaPlaylist>::const_iterator i1, i2;
	bool first;

	if (excludeList.empty ()) {
		return (MediaPlaylist::getDeleteAllSql ());
	}
	first = true;
	sql.assign ("DELETE FROM MediaPlaylist WHERE (");
	i1 = excludeList.cbegin ();
	i2 = excludeList.cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
		}
		else {
			sql.append (" AND ");
		}
		sql.append ("(id != ");
		sql.append (Database::getColumnValueSql (i1->id));
		sql.append (")");
		++i1;
	}
	sql.append (");");
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
: startTimestamp (0)
{
}
MediaPlaylistItem::MediaPlaylistItem (const StdString &mediaId, int64_t startTimestamp)
: mediaId (mediaId)
, startTimestamp (startTimestamp)
{
}
MediaPlaylistItem::~MediaPlaylistItem () {
}
