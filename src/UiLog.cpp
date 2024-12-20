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
#include "OsUtil.h"
#include "TaskGroup.h"
#include "Database.h"
#include "Log.h"
#include "UiLog.h"

UiLog *UiLog::instance = NULL;

constexpr const char *createTableSql = "CREATE TABLE IF NOT EXISTS Message(line INTEGER, createTime INTEGER, options INTEGER, messageText TEXT); CREATE INDEX IF NOT EXISTS MessageLine ON Message(line); CREATE INDEX IF NOT EXISTS MessageCreateTime ON Message(createTime);";
constexpr const char *selectSql = "SELECT line, createTime, options, messageText FROM Message";
constexpr const int selectColumnCount = 4;
constexpr const char *metadataTableName = "MessageMetadata";
constexpr const int metadataVersion = 1;

constexpr const int Uninitialized = 0;
constexpr const int InitializeWait1 = 1;
constexpr const int InitializeWait2 = 2;
constexpr const int Running = 3;
constexpr const int StoreMessageRecordsWait1 = 4;
constexpr const int StoreMessageRecordsWait2 = 5;
constexpr const int ClearWait1 = 6;
constexpr const int ClearWait2 = 7;

UiLog::UiLog ()
: isReady (false)
, maxMessageAge (-1)
, lastMessageLine (0)
, isClearing (false)
, stage (Uninitialized)
, isDatabaseOpen (false)
{
	SdlUtil::createMutex (&writeMessageListMutex);
}
UiLog::~UiLog () {
	if (isDatabaseOpen && (! databasePath.empty ())) {
		Database::instance->close (databasePath);
		databasePath.assign ("");
		isDatabaseOpen = false;
	}
	SdlUtil::destroyMutex (&writeMessageListMutex);
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

void UiLog::configure (const StdString &databasePathValue, int maxMessageAgeValue) {
	databasePath.assign (databasePathValue);
	maxMessageAge = maxMessageAgeValue;
}

void UiLog::clear () {
	isClearing = true;
}

void UiLog::update (int msElapsed) {
	int count;

	switch (stage) {
		case Uninitialized: {
			if (databasePath.empty ()) {
				break;
			}
			stage = InitializeWait1;
			TaskGroup::instance->run (TaskGroup::RunContext (UiLog::initialize, this));
			break;
		}
		case InitializeWait1: {
			break;
		}
		case InitializeWait2: {
			isReady = true;
			stage = Running;
			break;
		}
		case Running: {
			if (isClearing) {
				SDL_LockMutex (writeMessageListMutex);
				writeMessageList.clear ();
				SDL_UnlockMutex (writeMessageListMutex);
				lastMessageLine = 0;
				stage = ClearWait1;
				TaskGroup::instance->run (TaskGroup::RunContext (UiLog::removeAllRecords, this));
				break;
			}
			SDL_LockMutex (writeMessageListMutex);
			count = (int) writeMessageList.size ();
			SDL_UnlockMutex (writeMessageListMutex);
			if (count > 0) {
				stage = StoreMessageRecordsWait1;
				TaskGroup::instance->run (TaskGroup::RunContext (UiLog::storeMessageRecords, this));
			}
			break;
		}
		case StoreMessageRecordsWait1: {
			break;
		}
		case StoreMessageRecordsWait2: {
			stage = Running;
			break;
		}
		case ClearWait1: {
			break;
		}
		case ClearWait2: {
			SDL_LockMutex (writeMessageListMutex);
			writeMessageList.clear ();
			SDL_UnlockMutex (writeMessageListMutex);
			isClearing = false;
			stage = Running;
			break;
		}
	}
}

void UiLog::initialize (void *itPtr) {
	UiLog *it = (UiLog *) itPtr;
	OpResult result;

	result = it->executeInitialize ();
	if (result != OpResult::Success) {
		it->stage = Uninitialized;
	}
	else {
		it->stage = InitializeWait2;
	}
}
OpResult UiLog::executeInitialize () {
	OpResult result;
	StdString errmsg;

	result = openDatabase ();
	if (result == OpResult::Success) {
		if (! removeMaxAgeRecords (&errmsg)) {
			Log::debug ("Failed to open database; err=\"%s\"", errmsg.c_str ());
			return (OpResult::MalformedDataError);
		}
	}
	if (result == OpResult::Success) {
		lastMessageLine = readMaxMessageLine (&errmsg);
		if (lastMessageLine < 0) {
			Log::debug ("Failed to open database; err=\"%s\"", errmsg.c_str ());
			return (OpResult::MalformedDataError);
		}
	}
	return (result);
}

OpResult UiLog::setDatabasePath (const StdString &databasePathValue) {
	if (databasePathValue.empty ()) {
		return (OpResult::InvalidParamError);
	}
	if (databasePath.equals (databasePathValue)) {
		return (OpResult::Success);
	}
	if (! databasePath.empty ()) {
		Database::instance->close (databasePath);
	}
	databasePath.assign (databasePathValue);
	return (openDatabase ());
}

OpResult UiLog::openDatabase () {
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
	return (result);
}

void UiLog::storeMessageRecords (void *itPtr) {
	UiLog *it = (UiLog *) itPtr;

	it->executeStoreMessageRecords ();
	it->stage = StoreMessageRecordsWait2;
}
void UiLog::executeStoreMessageRecords () {
	std::list<UiLog::Message> msglist;
	std::list<UiLog::Message>::const_iterator i1, i2;
	StdString sql, errmsg;
	StringList fields;
	OpResult result;
	int maxline;

	SDL_LockMutex (writeMessageListMutex);
	msglist.swap (writeMessageList);
	SDL_UnlockMutex (writeMessageListMutex);
	maxline = lastMessageLine;
	i1 = msglist.cbegin ();
	i2 = msglist.cend ();
	while (i1 != i2) {
		fields.clear ();
		fields.push_back (StdString ("line"));
		fields.push_back (Database::getColumnValueSql (maxline + 1));
		fields.push_back (StdString ("createTime"));
		fields.push_back (Database::getColumnValueSql (i1->createTime));
		fields.push_back (StdString ("options"));
		fields.push_back (Database::getColumnValueSql (i1->options));
		fields.push_back (StdString ("messageText"));
		fields.push_back (Database::getColumnValueSql (i1->text));

		sql.assign ("INSERT INTO ");
		sql.append (Database::getRowInsertSql (StdString ("Message"), fields));
		result = Database::instance->exec (databasePath, sql, &errmsg);
		if (result != OpResult::Success) {
			Log::debug ("Failed to store message record; err=\"%s\"", errmsg.c_str ());
		}
		else {
			++maxline;
		}
		++i1;
	}

	lastMessageLine = maxline;
}

void UiLog::removeAllRecords (void *itPtr) {
	UiLog *it = (UiLog *) itPtr;

	it->executeRemoveAllRecords ();
	it->stage = ClearWait2;
}
void UiLog::executeRemoveAllRecords () {
	StdString sql;
	OpResult result;
	StdString errmsg;

	sql.assign ("DELETE FROM Message;");
	result = Database::instance->exec (databasePath, sql, &errmsg);
	if (result != OpResult::Success) {
		Log::debug ("Failed to write to database; err=\"%s\"", errmsg.c_str ());
		return;
	}
	lastMessageLine = 0;
}

void UiLog::voutput (int options, const char *str, va_list args) {
	UiLog::Message msg;

	if (isClearing) {
		return;
	}
	msg.createTime = OsUtil::getTime ();
	msg.options = options;
	msg.text.vsprintf (str, args);
	SDL_LockMutex (writeMessageListMutex);
	writeMessageList.push_back (msg);
	SDL_UnlockMutex (writeMessageListMutex);
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

bool UiLog::readRecords (StdString *errorMessage, std::list<UiLog::Message> *destList, int messageLine, int direction, int limit) {
	StdString sql;
	OpResult result;

	destList->clear ();
	if (!(isReady && isDatabaseOpen)) {
		if (errorMessage) {
			errorMessage->assign ("Database not available");
		}
		return (false);
	}

	sql.sprintf ("%s WHERE (line %s %i)", selectSql, (direction >= 0) ? ">" : "<", messageLine);
	if (limit > 0) {
		sql.appendSprintf (" LIMIT %i", limit);
	}
	sql.append (";");

	result = Database::instance->exec (databasePath, sql, errorMessage, UiLog::readRecords_row, destList);
	if (result != OpResult::Success) {
		return (false);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
int UiLog::readRecords_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames) {
	UiLog::Message msg;
	char *val;

	if (columnCount != selectColumnCount) {
		return (-1);
	}
	val = columnValues[0];
	msg.line = val ? StdString (val).parsedInt ((int) 0) : 0;

	val = columnValues[1];
	msg.createTime = val ? StdString (val).parsedInt ((int64_t) 0) : 0;

	val = columnValues[2];
	msg.options = val ? StdString (val).parsedInt ((int) 0) : 0;

	val = columnValues[3];
	msg.text.assign (val ? val : "");

	((std::list<UiLog::Message> *) destListPtr)->push_back (msg);
	return (0);
}

int UiLog::readMaxMessageLine (StdString *errorMessage) {
	StdString sql;
	OpResult result;
	int maxline;

	sql.assign ("SELECT MAX(line) FROM Message;");
	maxline = 0;
	result = Database::instance->exec (databasePath, sql, errorMessage, Database::selectAggregate_row, &maxline);
	if (result != OpResult::Success) {
		return (-1);
	}
	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (maxline);
}

bool UiLog::removeMaxAgeRecords (StdString *errorMessage) {
	StdString sql;
	OpResult result;
	int64_t t;
	int minline;

	if (maxMessageAge < 0) {
		return (true);
	}
	sql.assign ("DELETE FROM Message");
	if (maxMessageAge > 0) {
		t = OsUtil::getTime () - (((int64_t) maxMessageAge) * 1000);
		sql.appendSprintf (" WHERE (createTime < %lli)", (long long int) t);
	}
	sql.append (";");
	result = Database::instance->exec (databasePath, sql, errorMessage);
	if (result != OpResult::Success) {
		return (false);
	}

	sql.assign ("SELECT MIN(line) FROM Message;");
	minline = 0;
	result = Database::instance->exec (databasePath, sql, errorMessage, Database::selectAggregate_row, &minline);
	if (result != OpResult::Success) {
		return (false);
	}
	if (minline > 1) {
		sql.sprintf ("UPDATE Message SET line = (line - %i);", minline - 1);
		result = Database::instance->exec (databasePath, sql, errorMessage);
		if (result != OpResult::Success) {
			return (false);
		}
	}

	if (errorMessage) {
		errorMessage->assign ("");
	}
	return (true);
}
