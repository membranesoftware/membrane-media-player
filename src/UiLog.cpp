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
#include "SdlUtil.h"
#include "Log.h"
#include "OsUtil.h"
#include "StringList.h"
#include "Database.h"
#include "UiLog.h"

UiLog *UiLog::instance = NULL;

constexpr const int64_t defaultMaxLogSize = 128 * 1024;
constexpr const char *metadataTableName = "MessageMetadata";
constexpr const int metadataVersion = 1;

UiLog::UiLog ()
: nextMessageLine (1)
, logSize (0)
, maxLogSize (defaultMaxLogSize)
, loadMessageMaxLine (0)
, loadMessageTrimLine (0)
, loadMessageLogSize (0)
, loadMessageMaxLogSize (0)
{
	SdlUtil::createMutex (&messageListMutex);
	SdlUtil::createMutex (&callbackListMutex);
}
UiLog::~UiLog () {
	SdlUtil::destroyMutex (&messageListMutex);
	SdlUtil::destroyMutex (&callbackListMutex);
}

void UiLog::createInstance () {
	if (! UiLog::instance) {
		UiLog::instance = new UiLog ();
	}
}
void UiLog::freeInstance () {
	if (UiLog::instance) {
		delete (UiLog::instance);
		UiLog::instance = NULL;
	}
}

void UiLog::clear () {
	SDL_LockMutex (messageListMutex);
	messageList.clear ();
	nextMessageLine = 1;
	logSize = 0;
	SDL_UnlockMutex (messageListMutex);
}

void UiLog::setNextMessageLine (int line) {
	SDL_LockMutex (messageListMutex);
	nextMessageLine = line;
	SDL_UnlockMutex (messageListMutex);
}

void UiLog::setMaxLogSize (int64_t maxLogSizeValue) {
	SDL_LockMutex (messageListMutex);
	maxLogSize = maxLogSizeValue;
	SDL_UnlockMutex (messageListMutex);
}

void UiLog::voutput (int options, const char *str, va_list args) {
	UiLog::Message msg;
	std::list<UiLog::MessageCallbackContext>::const_iterator i1, i2;
	std::list<UiLog::Message>::iterator j;
	bool trim;

	msg.text.vsprintf (str, args);
	if (msg.text.empty ()) {
		return;
	}
	trim = false;
	msg.createTime = OsUtil::getTime ();
	msg.options = options;
	SDL_LockMutex (messageListMutex);
	msg.line = nextMessageLine;
	messageList.push_back (msg);
	++nextMessageLine;
	logSize += msg.text.size ();
	if ((maxLogSize > 0) && (logSize > maxLogSize)) {
		trim = true;
	}
	SDL_UnlockMutex (messageListMutex);

	SDL_LockMutex (callbackListMutex);
	i1 = callbackList.cbegin ();
	i2 = callbackList.cend ();
	while (i1 != i2) {
		i1->addMessageCallback (i1->callbackData, msg);
		++i1;
	}
	SDL_UnlockMutex (callbackListMutex);

	if (trim) {
		SDL_LockMutex (messageListMutex);
		while (logSize > maxLogSize) {
			if (messageList.empty ()) {
				break;
			}
			j = messageList.begin ();
			logSize -= j->text.size ();
			messageList.erase (j);
		}
		SDL_UnlockMutex (messageListMutex);
	}
}

void UiLog::write (int options, const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	UiLog::instance->voutput (options, str, ap);
	va_end (ap);
}
void UiLog::write (int options, const char *str, va_list args) {
	UiLog::instance->voutput (options, str, args);
}

void UiLog::processMessages (UiLog::ProcessMessageFunction processFn, void *processFnData) {
	std::list<UiLog::Message>::const_iterator i1, i2;

	SDL_LockMutex (messageListMutex);
	i1 = messageList.cbegin ();
	i2 = messageList.cend ();
	while (i1 != i2) {
		processFn (processFnData, *i1);
		++i1;
	}
	SDL_UnlockMutex (messageListMutex);
}

void UiLog::addListener (void *callbackData, UiLog::ProcessMessageFunction addMessageCallback) {
	std::list<UiLog::MessageCallbackContext>::iterator i1, i2;
	bool found;

	SDL_LockMutex (callbackListMutex);
	found = false;
	i1 = callbackList.begin ();
	i2 = callbackList.end ();
	while (i1 != i2) {
		if (i1->callbackData == callbackData) {
			found = true;
			i1->addMessageCallback = addMessageCallback;
			break;
		}
		++i1;
	}
	if (! found) {
		callbackList.push_back (UiLog::MessageCallbackContext (callbackData, addMessageCallback));
	}
	SDL_UnlockMutex (callbackListMutex);
}

void UiLog::removeListener (void *callbackData) {
	std::list<UiLog::MessageCallbackContext>::iterator i1, i2;

	SDL_LockMutex (callbackListMutex);
	i1 = callbackList.begin ();
	i2 = callbackList.end ();
	while (i1 != i2) {
		if (i1->callbackData == callbackData) {
			callbackList.erase (i1);
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (callbackListMutex);
}

constexpr const char *selectSql = "SELECT line, createTime, options, messageText FROM ";
constexpr const int selectColumnCount = 4;
bool UiLog::loadMessages (const StdString &databasePath, const char *tableName, StdString *errorMessage) {
	StdString sql, errmsg;

	if (errorMessage) {
		errorMessage->assign ("");
	}
	clear ();
	loadMessageList.clear ();
	loadMessageMaxLine = 0;
	loadMessageTrimLine = -1;
	loadMessageLogSize = 0;
	SDL_LockMutex (messageListMutex);
	loadMessageMaxLogSize = maxLogSize;
	SDL_UnlockMutex (messageListMutex);

	sql.assign (selectSql);
	sql.append (tableName);
	sql.append (" ORDER BY line ASC;");
	if (Database::instance->exec (databasePath, sql, errorMessage, UiLog::loadMessages_row, this) != OpResult::Success) {
		loadMessageList.clear ();
		return (false);
	}
	SDL_LockMutex (messageListMutex);
	messageList.swap (loadMessageList);
	nextMessageLine = loadMessageMaxLine + 1;
	logSize = loadMessageLogSize;
	SDL_UnlockMutex (messageListMutex);

	if (loadMessageTrimLine >= 0) {
		sql.assign ("DELETE FROM ");
		sql.append (tableName);
		sql.appendSprintf (" WHERE (line <= %i);", loadMessageTrimLine);
		if (Database::instance->exec (databasePath, sql, &errmsg) != OpResult::Success) {
			Log::debug ("Failed to store application data; err=%s", errmsg.c_str ());
		}
	}
	return (true);
}
int UiLog::loadMessages_row (void *itPtr, int columnCount, char **columnValues, char **columnNames) {
	UiLog *it = (UiLog *) itPtr;
	UiLog::Message msg;

	if (! UiLog::copyDatabaseRowValues (&msg, columnCount, columnValues, columnNames)) {
		return (-1);
	}
	it->appendLoadMessage (msg);
	return (0);
}
void UiLog::appendLoadMessage (const UiLog::Message &msg) {
	std::list<UiLog::Message>::iterator i;

	loadMessageList.push_back (msg);
	if (msg.line > loadMessageMaxLine) {
		loadMessageMaxLine = msg.line;
	}
	loadMessageLogSize += msg.text.size ();
	if (loadMessageMaxLogSize > 0) {
		while (loadMessageLogSize > loadMessageMaxLogSize) {
			if (loadMessageList.empty ()) {
				break;
			}
			i = loadMessageList.begin ();
			if ((loadMessageTrimLine < 0) || (i->line > loadMessageTrimLine)) {
				loadMessageTrimLine = i->line;
			}
			loadMessageLogSize -= i->text.size ();
			loadMessageList.erase (i);
		}
	}
}

StdString UiLog::getCreateTableSql (const char *tableName) {
	StdString sql;

	sql.assign ("CREATE TABLE IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("(line INTEGER, createTime INTEGER, options INTEGER, messageText TEXT);");

	sql.append ("CREATE INDEX IF NOT EXISTS ");
	sql.append (tableName);
	sql.append ("Line ON ");
	sql.append (tableName);
	sql.append ("(line);");

	return (sql);
}

void UiLog::getInsertMessageSql (const UiLog::Message &message, const char *tableName, StringList *destList) {
	StringList fields;
	StdString sql;

	if (! destList) {
		return;
	}
	fields.push_back (StdString ("line"));
	fields.push_back (Database::getColumnValueSql (message.line));
	fields.push_back (StdString ("createTime"));
	fields.push_back (Database::getColumnValueSql (message.createTime));
	fields.push_back (StdString ("options"));
	fields.push_back (Database::getColumnValueSql (message.options));
	fields.push_back (StdString ("messageText"));
	fields.push_back (Database::getColumnValueSql (message.text));

	sql.assign ("INSERT INTO ");
	sql.append (Database::getRowInsertSql (StdString (tableName), fields));
	sql.append (";");
	destList->push_back (sql);
}

bool UiLog::readDatabaseRows (const StdString &databasePath, const char *tableName, StdString *errorMessage, std::list<UiLog::Message> *destList, int offset, int limit) {
	StdString sql;
	OpResult result;

	destList->clear ();
	if (errorMessage) {
		errorMessage->assign ("");
	}
	sql.assign (selectSql);
	sql.append (tableName);
	sql.append (" ORDER BY line ASC");
	if (limit > 0) {
		sql.appendSprintf (" LIMIT %i", limit);
		if (offset > 0) {
			sql.appendSprintf (" OFFSET %i", offset);
		}
	}
	sql.append (";");
	result = Database::instance->exec (databasePath, sql, errorMessage, UiLog::readDatabaseRows_row, destList);
	if (result != OpResult::Success) {
		return (false);
	}
	return (true);
}
int UiLog::readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames) {
	UiLog::Message msg;

	if (! UiLog::copyDatabaseRowValues (&msg, columnCount, columnValues, columnNames)) {
		return (-1);
	}
	((std::list<UiLog::Message> *) destListPtr)->push_back (msg);
	return (0);
}
bool UiLog::copyDatabaseRowValues (UiLog::Message *destMessage, int columnCount, char **columnValues, char **columnNames) {
	int i;
	char *val;

	if (columnCount < selectColumnCount) {
		return (false);
	}
	i = 0;
	val = columnValues[i];
	destMessage->line = val ? StdString (val).parsedInt ((int) 0) : 0;

	++i;
	val = columnValues[i];
	destMessage->createTime = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	++i;
	val = columnValues[i];
	destMessage->options = val ? StdString (val).parsedInt ((int) 0) : 0;

	++i;
	val = columnValues[i];
	destMessage->text.assign (val ? val : "");

	return (true);
}
