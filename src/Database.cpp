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
extern "C" {
#include "sqlite3.h"
}
#include "App.h"
#include "SdlUtil.h"
#include "Log.h"
#include "Database.h"

Database *Database::instance = NULL;

constexpr const char *metadataSchemaSql = "(version INTEGER)";

Database::Database ()
{
	SdlUtil::createMutex (&connectionMapMutex);
}
Database::~Database () {
	SdlUtil::destroyMutex (&connectionMapMutex);
}

void Database::createInstance () {
	if (! Database::instance) {
		Database::instance = new Database ();
	}
}
void Database::freeInstance () {
	if (Database::instance) {
		delete (Database::instance);
		Database::instance = NULL;
	}
}

OpResult Database::open (const StdString &dbFilePath) {
	std::map<StdString, Database::Connection>::iterator i;
	Database::Connection connection;
	int result;
	bool success;

	success = false;
	SDL_LockMutex (connectionMapMutex);
	i = connectionMap.find (dbFilePath);
	if (i != connectionMap.end ()) {
		++(i->second.refcount);
		success = true;
	}
	else {
		result = sqlite3_open (dbFilePath.c_str (), &(connection.sqlite));
		if (result != SQLITE_OK) {
			Log::debug ("Failed to open sqlite3 file; dbFilePath=\"%s\" err=%i", dbFilePath.c_str (), result);
			sqlite3_close (connection.sqlite);
		}
		else {
			success = true;
			connection.refcount = 1;
			connectionMap.insert (std::pair<StdString, Database::Connection> (dbFilePath, connection));
		}
	}
	SDL_UnlockMutex (connectionMapMutex);
	if (! success) {
		return (OpResult::SqliteOperationFailedError);
	}
	return (OpResult::Success);
}

void Database::close (const StdString &dbFilePath) {
	std::map<StdString, Database::Connection>::iterator i;

	SDL_LockMutex (connectionMapMutex);
	i = connectionMap.find (dbFilePath);
	if (i != connectionMap.end ()) {
		--(i->second.refcount);
		if (i->second.refcount <= 0) {
			sqlite3_close (i->second.sqlite);
			connectionMap.erase (dbFilePath);
		}
	}
	SDL_UnlockMutex (connectionMapMutex);
}

OpResult Database::exec (const StdString &dbFilePath, const StdString &sql, StdString *errorMessage, Database::ExecCallbackFunction callback, void *callbackData) {
	std::map<StdString, Database::Connection>::iterator i;
	sqlite3 *sqlite;
	char *err;
	OpResult result;
	int sqliteresult;

	sqlite = NULL;
	SDL_LockMutex (connectionMapMutex);
	i = connectionMap.find (dbFilePath);
	if (i != connectionMap.end ()) {
		sqlite = i->second.sqlite;
		++(i->second.refcount);
	}
	SDL_UnlockMutex (connectionMapMutex);
	if (! sqlite) {
		return (OpResult::KeyNotFoundError);
	}

	err = NULL;
	result = OpResult::Success;
	sqliteresult = sqlite3_exec (sqlite, sql.c_str (), callback, callbackData, &err);
	close (dbFilePath);
	if (sqliteresult != SQLITE_OK) {
		Log::debug ("sql exec failed; err=%i errmsg=%s", sqliteresult, err ? err : "");
		result = OpResult::SqliteOperationFailedError;
		if (errorMessage) {
			errorMessage->assign (err ? err : "db operation failed");
		}
	}
	else {
		if (errorMessage) {
			errorMessage->assign ("");
		}
	}
	if (err) {
		sqlite3_free (err);
		err = NULL;
	}
	return (result);
}

StdString Database::getColumnValueSql (const StdString &value) {
	StdString s;

	s.assign ("'");
	s.append (value.replaced (StdString ("'"), StdString ("''")));
	s.append ("'");
	return (s);
}
StdString Database::getColumnValueSql (int value) {
	return (StdString::createSprintf ("%i", value));
}
StdString Database::getColumnValueSql (int64_t value) {
	return (StdString::createSprintf ("%lli", (long long int) value));
}
StdString Database::getColumnValueSql (float value) {
	return (StdString::createSprintf ("%.8f", value));
}
StdString Database::getColumnValueSql (double value) {
	return (StdString::createSprintf ("%.8f", value));
}
StdString Database::getColumnValueSql (bool value) {
	return (StdString (value ? "1": "0"));
}

StdString Database::getRowInsertSql (const StdString &tableName, const StringList &fields) {
	StdString s1, s2, colname;
	StringList::const_iterator i1, i2;
	bool first;

	if (tableName.empty () || (fields.size () < 2)) {
		return (StdString ());
	}
	s1.assign (tableName);
	s1.append ("(");
	s2.assign (" VALUES(");
	first = true;
	i1 = fields.cbegin ();
	i2 = fields.cend ();
	while (i1 != i2) {
		if (colname.empty ()) {
			colname.assign (*i1);
		}
		else {
			if (first) {
				first = false;
			}
			else {
				s1.append (",");
				s2.append (",");
			}
			s1.append (colname);
			s2.append (*i1);
			colname.assign ("");
		}
		++i1;
	}
	s1.append (")");
	s2.append (")");
	s1.append (s2);
	return (s1);
}

StdString Database::getRowUpdateSql (const StringList &fields) {
	StringList::const_iterator i1, i2;
	StdString s, colname;
	bool first;

	if (fields.size () < 2) {
		return (StdString ());
	}
	first = true;
	i1 = fields.cbegin ();
	i2 = fields.cend ();
	while (i1 != i2) {
		if (colname.empty ()) {
			colname.assign (*i1);
		}
		else {
			if (first) {
				first = false;
			}
			else {
				s.append (",");
			}
			s.append (colname);
			s.append ("=");
			s.append (*i1);
			colname.assign ("");
		}
		++i1;
	}
	return (s);
}

int Database::selectAggregate_row (void *intPtr, int columnCount, char **columnValues, char **columnNames) {
	char *val;

	if (columnCount < 1) {
		return (-1);
	}
	val = columnValues[0];
	*((int *) intPtr) = val ? StdString (val).parsedInt (0) : 0;
	return (0);
}

OpResult Database::createMetadataTable (const StdString &dbFilePath, const StdString &tableName) {
	StdString sql;

	sql.assign ("CREATE TABLE IF NOT EXISTS ");
	sql.append (tableName);
	sql.append (metadataSchemaSql);
	sql.append (";");
	return (exec (dbFilePath, sql));
}

int Database::readMetadataVersion (const StdString &dbFilePath, const StdString &tableName) {
	StdString sql, errmsg;
	OpResult result;
	int version;

	sql.assign ("SELECT version FROM ");
	sql.append (tableName);
	sql.append (" ORDER BY version DESC LIMIT 1;");
	version = 0;
	result = exec (dbFilePath, sql, &errmsg, Database::selectAggregate_row, &version);
	if (result != OpResult::Success) {
		Log::debug ("Failed to read database; err=%s", errmsg.c_str ());
		return (0);
	}
	return (version);
}

OpResult Database::writeMetadataVersion (const StdString &dbFilePath, const StdString &tableName, int metadataVersion) {
	StdString sql, errmsg;
	OpResult result;

	if (metadataVersion < 1) {
		return (OpResult::InvalidParamError);
	}
	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append (";");
	result = exec (dbFilePath, sql, &errmsg);
	if (result != OpResult::Success) {
		Log::debug ("Failed to write database; err=%s", errmsg.c_str ());
		return (result);
	}
	sql.assign ("INSERT INTO ");
	sql.append (getRowInsertSql (tableName, StringList (StdString ("version"), StdString::createSprintf ("%i", metadataVersion))));
	sql.append (";");
	result = exec (dbFilePath, sql, &errmsg);
	if (result != OpResult::Success) {
		Log::debug ("Failed to write database; err=%s", errmsg.c_str ());
		return (result);
	}
	return (OpResult::Success);
}
