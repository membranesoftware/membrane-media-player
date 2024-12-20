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
#include "Log.h"
#include "StringList.h"
#include "Json.h"
#include "TaskGroup.h"
#include "Database.h"
#include "SystemInterface.h"
#include "AppNews.h"

AppNews *AppNews::instance = NULL;

constexpr const char *createTableSql = "CREATE TABLE IF NOT EXISTS AppNews(record TEXT);";
constexpr const char *metadataTableName = "AppNewsMetadata";
constexpr const int metadataVersion = 1;

AppNews::AppNews ()
: isReady (false)
, isLoadFailed (false)
, isDatabaseOpen (false) {
	SdlUtil::createMutex (&databaseMutex);
}
AppNews::~AppNews () {
	if (isDatabaseOpen && (! databasePath.empty ())) {
		Database::instance->close (databasePath);
		databasePath.assign ("");
		isDatabaseOpen = false;
	}
	SdlUtil::destroyMutex (&databaseMutex);
}

void AppNews::createInstance () {
	if (! AppNews::instance) {
		AppNews::instance = new AppNews ();
	}
}
void AppNews::freeInstance () {
	if (AppNews::instance) {
		delete (AppNews::instance);
		AppNews::instance = NULL;
	}
}

void AppNews::configure (const StdString &databasePathValue) {
	if (databasePathValue.empty () || databasePath.equals (databasePathValue)) {
		return;
	}
	if (isDatabaseOpen && (! databasePath.empty ())) {
		Database::instance->close (databasePath);
		databasePath.assign ("");
		isDatabaseOpen = false;
	}
	databasePath.assign (databasePathValue);
	TaskGroup::instance->run (TaskGroup::RunContext (AppNews::initialize, this));
}

void AppNews::initialize (void *itPtr) {
	OpResult result;

	result = ((AppNews *) itPtr)->executeInitialize ();
	if (result != OpResult::Success) {
		Log::debug ("Failed to initialize app data; err=%i", result);
	}
}
OpResult AppNews::executeInitialize () {
	OpResult result;
	int version;

	if (databasePath.empty ()) {
		return (OpResult::InvalidConfigurationError);
	}
	result = Database::instance->open (databasePath);
	if (result == OpResult::Success) {
		isDatabaseOpen = true;
		result = Database::instance->exec (databasePath, createTableSql);
	}
	if (result == OpResult::Success) {
		result = Database::instance->createMetadataTable (databasePath, StdString (metadataTableName));
	}
	if (result == OpResult::Success) {
		version = Database::instance->readMetadataVersion (databasePath, StdString (metadataTableName));
		if (version < metadataVersion) {
			Database::instance->writeMetadataVersion (databasePath, StdString (metadataTableName), metadataVersion);
		}
	}

	if (result == OpResult::Success) {
		isReady = true;
		isLoadFailed = false;
	}
	else {
		isReady = false;
		isLoadFailed = true;
	}
	return (result);
}

bool AppNews::parseCommand (const StdString &command, AppNews::NewsState *state) {
	Json *cmd, params;

	cmd = NULL;
	if (! SystemInterface::instance->parseCommand (command, &cmd)) {
		return (false);
	}
	if ((! cmd) || (SystemInterface::instance->getCommandId (cmd) != SystemInterface::CommandId_GetApplicationNewsResult)) {
		return (false);
	}
	if (! SystemInterface::instance->getCommandParams (cmd, &params)) {
		return (false);
	}
	parseCommandParams (&params, state);
	delete (cmd);
	return (true);
}
void AppNews::parseCommandParams (Json *params, AppNews::NewsState *state) {
	Json obj;
	AppNews::NewsPost post;
	int i, count;

	state->updateBuildId = params->getString (SystemInterface::Field_applicationUpdateId, "");
	state->updatePublishTime = params->getNumber (SystemInterface::Field_applicationUpdateTime, (int64_t) 0);
	state->posts.clear ();
	count = params->getArrayLength (SystemInterface::Field_applicationPosts);
	for (i = 0; i < count; ++i) {
		if (params->getArrayObject (SystemInterface::Field_applicationPosts, i, &obj)) {
			post.body = obj.getString (SystemInterface::Field_body, "");
			post.publishTime = obj.getNumber (SystemInterface::Field_publishTime, (int64_t) 0);
			post.endTime = obj.getNumber (SystemInterface::Field_endTime, (int64_t) 0);
			if (! post.body.empty ()) {
				state->posts.push_back (post);
			}
		}
	}
}

OpResult AppNews::writeRecord (const StdString &command) {
	StringList sql;
	StringList::const_iterator i1, i2;
	StdString s, recordtext, errmsg;
	OpResult result;
	Json *cmd, params;

	if (! isReady) {
		return (OpResult::InvalidStateError);
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
		return (OpResult::MalformedDataError);
	}

	sql.push_back ("DELETE FROM AppNews;");
	s.assign ("INSERT INTO ");
	s.append (Database::getRowInsertSql (StdString ("AppNews"), StringList (StdString ("record"), Database::getColumnValueSql (recordtext))));
	s.append (";");
	sql.push_back (s);
	sql.push_back ("COMMIT;");

	SDL_LockMutex (databaseMutex);
	result = Database::instance->exec (databasePath, StdString ("BEGIN TRANSACTION;"), &errmsg);
	if (result == OpResult::Success) {
		i1 = sql.cbegin ();
		i2 = sql.cend ();
		while (i1 != i2) {
			result = Database::instance->exec (databasePath, *i1, &errmsg);
			if (result != OpResult::Success) {
				break;
			}
			++i1;
		}
		if (result != OpResult::Success) {
			Database::instance->exec (databasePath, StdString ("ROLLBACK;"));
		}
	}
	SDL_UnlockMutex (databaseMutex);
	if (result != OpResult::Success) {
		Log::debug ("Failed to write app data; err=%s", errmsg.c_str ());
	}
	return (result);
}

bool AppNews::readRecord (AppNews::NewsState *state) {
	StdString sql, errmsg, text;
	OpResult result;
	Json params;

	if (! isReady) {
		return (false);
	}
	state->posts.clear ();
	sql.assign ("SELECT record FROM AppNews LIMIT 1;");
	SDL_LockMutex (databaseMutex);
	result = Database::instance->exec (databasePath, sql, &errmsg, AppNews::readRecord_row, &text);
	SDL_UnlockMutex (databaseMutex);

	if (result != OpResult::Success) {
		Log::debug ("Failed to read app data; err=%s", errmsg.c_str ());
	}
	if (text.empty () || (! params.parse (text))) {
		return (false);
	}
	parseCommandParams (&params, state);
	return (true);
}
int AppNews::readRecord_row (void *stringPtr, int columnCount, char **columnValues, char **columnNames) {
	if (columnCount < 1) {
		return (-1);
	}
	((StdString *) stringPtr)->assign (columnValues[0]);
	return (0);
}
