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
// Object that holds fields from a stored MediaItem record
#ifndef MEDIA_ITEM_H
#define MEDIA_ITEM_H

#include "StringList.h"
#include "Int64List.h"

class Json;
class MediaReader;

class MediaItem {
public:
	MediaItem ();
	~MediaItem ();

	static const StdString createTableSql;
	static const StdString sortKeyCharacters;

	StdString mediaId;
	StdString agentId;
	StdString name;
	StdString mediaPath;
	StdString mediaDirname;
	int64_t mtime;
	int64_t duration;
	int64_t mediaFileSize;
	int64_t totalBitrate;
	bool isVideo;
	bool isAudio;
	bool hasAudioAlbumArt;
	double frameRate;
	int64_t videoBitrate;
	int width;
	int height;
	int audioSampleRate;
	int audioChannels;
	int64_t audioBitrate;
	Int64List thumbnailTimestamps;
	StringList tags;
	StdString sortKey;

	// Return a string representation of the MediaItem
	StdString toString () const;

	// Clear all fields and optionally reset mediaId to a provided value
	void clear (const StdString &mediaIdValue = StdString ());

	// Copy field values from a source object
	void copyValues (const MediaItem &source);

	// Return true if fields are populated with a complete and valid MediaItem record
	bool isValid () const;

	// Return a newly created Json object containing a MediaItem record populated from current field values
	Json *createRecord (const StdString &agentIdValue = StdString ()) const;

	// Read fields from a record object and return true if the operation succeeded
	bool readRecord (Json *record);

	// Read fields from the RecordStore record matching mediaIdValue and return true if the operation succeeded
	bool readRecordStore (const StdString &mediaIdValue);

	// Read fields from a MediaReader object and return true if the operation succeeded
	bool readMediaReader (const MediaReader &reader);

	// Read fields from a database row and return true if the operation succeeded
	bool readDatabaseMediaPathRow (const StdString &databasePath, StdString *errorMessage, const StdString &mediaPathValue);
	bool readDatabaseMediaIdRow (const StdString &databasePath, StdString *errorMessage, const StdString &mediaIdValue);
	static int readDatabaseRow_row (void *itPtr, int columnCount, char **columnValues, char **columnNames);

	// Read MediaItem records from the database and add them to destList, clearing the list before doing so. Returns true if the operation succeeded.
	static bool readDatabaseRows (const StdString &databasePath, StdString *errorMessage, std::list<MediaItem> *destList, const StdString &searchKey = StdString (), int offset = 0, int limit = 0, int sortOrder = -1);
	static int readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames);

	// Compute metadata fields from database records and store them into the provided pointers. Returns true if the operation succeeded.
	static bool readDatabaseMetadata (const StdString &databasePath, StdString *errorMessage, int64_t *mediaSizeTotal, int64_t *mediaDurationTotal);
	static int readDatabaseMetadata_row (void *int64Ptr, int columnCount, char **columnValues, char **columnNames);

	// Return database upsert SQL generated from item fields
	StdString getUpsertSql () const;

	// Return an SQL SELECT WHERE clause generated from searchKey, or an empty string if no WHERE clause applies
	static StdString getSelectWhereSql (const StdString &searchKey);

	// Return an SQL UPDATE statement that modifies a record's tags field
	static StdString getUpdateTagsSql (const StdString &mediaId, const StringList &tags);

	// Return an SQL DELETE statement targeting a record matching mediaId
	static StdString getDeleteSql (const StdString &mediaId);

	// Return an SQL DELETE statement that clears all records
	static StdString getDeleteAllSql ();

	// Return the number of MediaItem database records, or -1 if a database error occurred
	static int countDatabaseRecords (const StdString &databasePath, StdString *errorMessage, const StdString &searchKey = StdString ());
	static int countDatabaseRecords_row (void *intPtr, int columnCount, char **columnValues, char **columnNames);

	// Set fields by reading values from a database row and return true if the operation succeeded
	bool copyDatabaseRowValues (int columnCount, char **columnValues, char **columnNames);
};
#endif
