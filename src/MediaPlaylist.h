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
// Object that holds fields from a stored MediaPlaylist record
#ifndef MEDIA_PLAYLIST_H
#define MEDIA_PLAYLIST_H

class MediaPlaylistItem;

class MediaPlaylist {
public:
	MediaPlaylist ();
	~MediaPlaylist ();

	static const StdString createTableSql;
	static constexpr const int idCommandType = 0xFFFF;

	// startPosition values
	static constexpr const int ZeroStartPosition = 0;
	static constexpr const int NearBeginningStartPosition = 1;
	static constexpr const int MiddleStartPosition = 2;
	static constexpr const int NearEndStartPosition = 3;
	static constexpr const int FullRangeStartPosition = 4;
	static constexpr const int StartPositionCount = 5;

	// playDuration values
	static constexpr const int VeryShortPlayDuration = 0;
	static constexpr const int ShortPlayDuration = 1;
	static constexpr const int MediumPlayDuration = 2;
	static constexpr const int LongPlayDuration = 3;
	static constexpr const int VeryLongPlayDuration = 4;
	static constexpr const int FullPlayDuration = 5;
	static constexpr const int PlayDurationCount = 6;

	StdString id;
	StdString name;
	std::vector<MediaPlaylistItem> items;
	bool isExpanded;
	bool isShuffle;
	int startPosition;
	int playDuration;

	// Reset playlist fields to empty state
	void clear ();

	// Copy playlist field values from a source object
	void copyValues (const MediaPlaylist &source);

	// Read MediaPlaylist records from the database and add them to destList, clearing the list before doing so. Returns true if the operation succeeded.
	static bool readDatabaseRows (const StdString &databasePath, StdString *errorMessage, std::list<MediaPlaylist> *destList);
	static int readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames);

	// Return database upsert SQL generated from playlist fields
	StdString getUpsertSql () const;

	// Return an SQL DELETE statement that clears all records
	static StdString getDeleteAllSql ();

	// Return an SQL DELETE statement that removes all records with id values not appearing in items from excludeList
	static StdString getDeleteExcludeSql (const std::list<MediaPlaylist> &excludeList);

	// Set fields by reading values from a database row and return true if the operation succeeded
	bool copyDatabaseRowValues (int columnCount, char **columnValues, char **columnNames);

	// Set the provided values to start position timestamps matching playlist settings
	void getStartPositionRange (int *minStartPosition, int *maxStartPosition);

	// Set the provided values to play duration times matching playlist settings
	void getPlayDurationRange (int64_t *minPlayDuration, int64_t *maxPlayDuration);
};

class MediaPlaylistItem {
public:
	MediaPlaylistItem ();
	MediaPlaylistItem (const StdString &mediaId, int64_t startTimestamp);
	~MediaPlaylistItem ();

	StdString mediaId;
	int64_t startTimestamp;
};
#endif
