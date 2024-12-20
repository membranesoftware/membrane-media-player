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
// Class providing access to a sqlite database
#ifndef DATABASE_H
#define DATABASE_H

extern "C" {
#include "sqlite3.h"
}

class StringList;

class Database {
public:
	Database ();
	~Database ();
	static Database *instance;

	// Callback function executed for each row of an exec result. The return value is zero for successful row processing or non-zero to abort the query.
	typedef int (*ExecCallbackFunction) (void *itPtr, int columnCount, char **columnValues, char **columnNames);

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Open a connection to the specified database file and return a Result value
	OpResult open (const StdString &dbFilePath);

	// Close a previously opened database connection
	void close (const StdString &dbFilePath);

	// Execute sql as a command targeting an opened database connection. If errorMessage is provided, set its content to any generated error string.
	OpResult exec (const StdString &dbFilePath, const StdString &sql, StdString *errorMessage = NULL, Database::ExecCallbackFunction callback = NULL, void *callbackData = NULL);

	// Return a string representation of a column value for use in an SQL statement
	static StdString getColumnValueSql (const StdString &value);
	static StdString getColumnValueSql (int value);
	static StdString getColumnValueSql (int64_t value);
	static StdString getColumnValueSql (float value);
	static StdString getColumnValueSql (double value);
	static StdString getColumnValueSql (bool value);

	// Return SQL text for use as a row in an INSERT statement targeting tableName and using each two consecutive items in fields as column name/value pairs
	static StdString getRowInsertSql (const StdString &tableName, const StringList &fields);

	// Return SQL text for use as a row in an UPDATE statement using each two consecutive items in fields as column name/value pairs
	static StdString getRowUpdateSql (const StringList &fields);

	// Row callback that writes the first column to the provided int pointer, for use with SELECT queries reading a single aggregate value
	static int selectAggregate_row (void *intPtr, int columnCount, char **columnValues, char **columnNames);

	// Execute SQL to create a metadata table
	OpResult createMetadataTable (const StdString &dbFilePath, const StdString &tableName);

	// Execute SQL to read the metadata version value and return the result number, or zero if no metadata version was found.
	int readMetadataVersion (const StdString &dbFilePath, const StdString &tableName);

	// Execute SQL to write the metadata version value
	OpResult writeMetadataVersion (const StdString &dbFilePath, const StdString &tableName, int metadataVersion);

private:
	struct Connection {
		sqlite3 *sqlite;
		int refcount;
		Connection ():
			sqlite (NULL),
			refcount (0) { }
	};
	std::map<StdString, Database::Connection> connectionMap;
	SDL_mutex *connectionMapMutex;
};
#endif
