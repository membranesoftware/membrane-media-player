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
#include "Database.h"
#include "PlayMarker.h"

const StdString PlayMarker::createTableSql = StdString ("CREATE TABLE IF NOT EXISTS PlayMarker(recordId TEXT PRIMARY KEY, markerTimestamps TEXT);");
constexpr const char *selectSql = "SELECT recordId, markerTimestamps FROM PlayMarker";
constexpr const int selectColumnCount = 2;

PlayMarker::PlayMarker () {
}
PlayMarker::~PlayMarker () {
}

bool PlayMarker::readDatabaseRow (const StdString &databasePath, StdString *errorMessage) {
	StdString sql;
	OpResult result;

	if (recordId.empty ()) {
		return (false);
	}
	sql.sprintf ("%s WHERE recordId=", selectSql);
	sql.append (Database::getColumnValueSql (recordId));
	sql.append (";");

	result = Database::instance->exec (databasePath, sql, errorMessage, PlayMarker::readDatabaseRow_row, this);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
int PlayMarker::readDatabaseRow_row (void *itPtr, int columnCount, char **columnValues, char **columnNames) {
	PlayMarker *it = (PlayMarker *) itPtr;
	char *val;
	
	if (columnCount < selectColumnCount) {
		return (-1);
	}
	val = columnValues[1];
	if ((! val) || (! it->markerTimestamps.parseJsonString (StdString (val)))) {
		it->markerTimestamps.clear ();
	}
	return (0);
}

StdString PlayMarker::getUpdateSql () const {
	StdString s;
	StringList fields;
	int i;

	if (recordId.empty ()) {
		return (StdString ());
	}
	if (markerTimestamps.empty ()) {
		s.assign ("DELETE FROM PlayMarker WHERE recordId=");
		s.append (Database::getColumnValueSql (recordId));
		s.append (";");
	}
	else {
		fields.push_back (StdString ("recordId"));
		fields.push_back (Database::getColumnValueSql (recordId));
		fields.push_back (StdString ("markerTimestamps"));
		fields.push_back (Database::getColumnValueSql (markerTimestamps.toJsonString ()));
		s.assign ("INSERT INTO ");
		s.append (Database::getRowInsertSql (StdString ("PlayMarker"), fields));

		for (i = 0; i < 2; ++i) {
			fields.erase (fields.begin ());
		}
		s.append (" ON CONFLICT(recordId) DO UPDATE SET ");
		s.append (Database::getRowUpdateSql (fields));
		s.append (";");
	}
	return (s);
}

StdString PlayMarker::getDeleteAllSql () {
	return (StdString ("DELETE FROM PlayMarker;"));
}
