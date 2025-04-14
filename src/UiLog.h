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
// Class that stores log messages for display in a Ui window
#ifndef UI_LOG_H
#define UI_LOG_H

class StringList;

class UiLog {
public:
	UiLog ();
	~UiLog ();
	static UiLog *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static constexpr const int NoOptions = 0;

	// Read-only data members
	int nextMessageLine;
	int64_t logSize;
	int64_t maxLogSize;

	// Remove all stored log messages
	void clear ();

	// Set the line number value to apply when writing the next message
	void setNextMessageLine (int line);

	// Set the maximum text size to hold in message entries, or disable max log size if maxLogSizeValue is zero or less.
	void setMaxLogSize (int64_t maxLogSizeValue);

	// Write a message to the log using the provided format string and va_list
	void voutput (int options, const char *str, va_list args);

	// Write a message to the static UiLog instance
	static void write (int options, const char *str, ...) __attribute__((format(printf, 2, 3)));
	static void write (int options, const char *str, va_list args);

	struct Message {
		int line;
		int64_t createTime;
		int options;
		StdString text;
		Message ():
			line (0),
			createTime (0),
			options (0) { }
	};
	typedef void (*ProcessMessageFunction) (void *data, const UiLog::Message &message);
	// Execute processFn for each stored message
	void processMessages (UiLog::ProcessMessageFunction processFn, void *processFnData);

	typedef void (*MessageCallback) (void *data, const UiLog::Message &message);
	// Set a callback function to be invoked when a new message is received
	void addListener (void *callbackData, UiLog::MessageCallback addMessageCallback);

	// Remove previously added message callbacks
	void removeListener (void *callbackData);

	// Read log contents from the specified database path and return true if the load succeeded
	bool loadMessages (const StdString &databasePath, const char *tableName, StdString *errorMessage);
	static int loadMessages_row (void *itPtr, int columnCount, char **columnValues, char **columnNames);
	void appendLoadMessage (const UiLog::Message &msg);

	// Return an SQL CREATE TABLE statement that creates a table of message records
	static StdString getCreateTableSql (const char *tableName);

	// Generate SQL statements to insert a message record and append them to destList
	static void getInsertMessageSql (const UiLog::Message &message, const char *tableName, StringList *destList);

	// Read Message records from the database and add them to destList, clearing the list before doing so. Return true if the operation succeeded.
	static bool readDatabaseRows (const StdString &databasePath, const char *tableName, StdString *errorMessage, std::list<UiLog::Message> *destList, int offset = 0, int limit = 0);
	static int readDatabaseRows_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames);

	// Set Message fields by reading values from a database row and return true if the operation succeeded
	static bool copyDatabaseRowValues (UiLog::Message *destMessage, int columnCount, char **columnValues, char **columnNames);

private:
	SDL_mutex *messageListMutex;
	std::list<UiLog::Message> messageList;

	struct MessageCallbackContext {
		void *callbackData;
		UiLog::MessageCallback addMessageCallback;
		MessageCallbackContext ():
			callbackData (NULL),
			addMessageCallback (NULL) { }
		MessageCallbackContext (void *callbackData, UiLog::MessageCallback addMessageCallback):
			callbackData (callbackData),
			addMessageCallback (addMessageCallback) { }
	};
	SDL_mutex *callbackListMutex;
	std::list<UiLog::MessageCallbackContext> callbackList;

	std::list<UiLog::Message> loadMessageList;
	int loadMessageMaxLine;
	int loadMessageTrimLine;
	int64_t loadMessageLogSize;
	int64_t loadMessageMaxLogSize;
};
#endif
