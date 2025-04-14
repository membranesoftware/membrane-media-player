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
#include "Log.h"
#include "StringList.h"
#include "Json.h"
#include "Database.h"
#include "SystemInterface.h"
#include "AppNews.h"

AppNews::AppNews ()
: updatePublishTime (0)
{
}
AppNews::~AppNews () {
}

bool AppNews::parseCommand (const StdString &command) {
	bool result;
	Json *cmd, params;

	cmd = NULL;
	if ((! SystemInterface::instance->parseCommand (command, &cmd)) || (! cmd)) {
		return (false);
	}
	result = false;
	if (SystemInterface::instance->getCommandId (cmd) == SystemInterface::CommandId_GetApplicationNewsResult) {
		if (SystemInterface::instance->getCommandParams (cmd, &params)) {
			parseCommandParams (&params);
			result = true;
		}
	}
	delete (cmd);
	return (result);
}

void AppNews::parseCommandParams (Json *params) {
	Json obj;
	AppNews::NewsPost post;
	int i, count;

	updateBuildId = params->getString (SystemInterface::Field_applicationUpdateId, "");
	updatePublishTime = params->getNumber (SystemInterface::Field_applicationUpdateTime, (int64_t) 0);
	posts.clear ();
	count = params->getArrayLength (SystemInterface::Field_applicationPosts);
	for (i = 0; i < count; ++i) {
		if (params->getArrayObject (SystemInterface::Field_applicationPosts, i, &obj)) {
			post.body = obj.getString (SystemInterface::Field_body, "");
			post.publishTime = obj.getNumber (SystemInterface::Field_publishTime, (int64_t) 0);
			post.endTime = obj.getNumber (SystemInterface::Field_endTime, (int64_t) 0);
			if (! post.body.empty ()) {
				posts.push_back (post);
			}
		}
	}
}

bool AppNews::getInsertCommandSql (const StdString &command, const char *tableName, StringList *destList) {
	StringList fields;
	StdString sql, recordtext;
	Json *cmd, params;

	if (! destList) {
		return (false);
	}
	cmd = NULL;
	if (SystemInterface::instance->parseCommand (command, &cmd)) {
		if (cmd) {
			if (SystemInterface::instance->getCommandId (cmd) == SystemInterface::CommandId_GetApplicationNewsResult) {
				if (SystemInterface::instance->getCommandParams (cmd, &params)) {
					recordtext.assign (params.toString ());
				}
			}
			delete (cmd);
		}
	}
	if (recordtext.empty ()) {
		return (false);
	}

	sql.assign ("DELETE FROM ");
	sql.append (tableName);
	sql.append (";");
	destList->push_back (sql);

	fields.push_back ("record");
	fields.push_back (Database::getColumnValueSql (recordtext));
	fields.push_back ("buildId");
	fields.push_back (Database::getColumnValueSql (StdString (BUILD_ID)));
	sql.assign ("INSERT INTO ");
	sql.append (Database::getRowInsertSql (StdString (tableName), fields));
	sql.append (";");
	destList->push_back (sql);

	return (true);
}

bool AppNews::readRecord (const StdString &databasePath, const char *tableName, StdString *errorMessage) {
	StdString sql;
	StringList cols;
	OpResult result;
	Json params;

	sql.assign ("SELECT record, buildId FROM ");
	sql.append (tableName);
	sql.append (" LIMIT 1;");
	result = Database::instance->exec (databasePath, sql, errorMessage, AppNews::readRecord_row, &cols);
	if (result != OpResult::Success) {
		return (false);
	}
	if ((cols.size () < 2) || (! params.parse (cols.at (0)))) {
		if (errorMessage) {
			errorMessage->assign ("Invalid record fields");
		}
		return (false);
	}
	parseCommandParams (&params);
	recordBuildId.assign (cols.at (1));
	return (true);
}
int AppNews::readRecord_row (void *stringListPtr, int columnCount, char **columnValues, char **columnNames) {
	StringList *s = (StringList *) stringListPtr;

	if (columnCount < 2) {
		return (-1);
	}
	s->push_back (columnValues[0]);
	s->push_back (columnValues[1]);
	return (0);
}

StdString AppNews::getCreateTableSql (const char *tableName) {
	StdString sql;

	sql.assign ("CREATE TABLE IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("(record TEXT, buildId TEXT);");

	return (sql);
}
