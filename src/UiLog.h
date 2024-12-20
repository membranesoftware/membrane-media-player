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

class UiLog {
public:
	UiLog ();
	~UiLog ();
	static UiLog *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static constexpr const char *databaseName = "log.db";

	// Read-only data members
	bool isReady;
	StdString databasePath;
	int maxMessageAge;
	int lastMessageLine;
	bool isClearing;

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

	// Update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Change the log configuration. maxMessageAgeValue is specified as a number of seconds, with a negative value indicating that no max message age should apply.
	void configure (const StdString &databasePathValue, int maxMessageAgeValue = -1);

	// Change the log database path, reopen the database connection, and return a result value
	OpResult setDatabasePath (const StdString &databasePathValue);

	// Remove all stored log messages
	void clear ();

	// Write a message to the log using the provided format string and va_list
	void voutput (int options, const char *str, va_list args);

	// Write a message to the log
	static void write (int options, const char *str, ...) __attribute__((format(printf, 2, 3)));
	static void write (int options, const char *str, va_list args);

	// Read message records from the database and add them to destList, clearing the list before doing so. Returns true if the operation succeeded.
	bool readRecords (StdString *errorMessage, std::list<UiLog::Message> *destList, int messageLine, int direction, int limit = -1);
	static int readRecords_row (void *destListPtr, int columnCount, char **columnValues, char **columnNames);

	// Return the maximum line value among stored records, or -1 if a database error occurred
	int readMaxMessageLine (StdString *errorMessage);

	// Remove records with age exceeding the configured maxMessageAge value
	bool removeMaxAgeRecords (StdString *errorMessage);

private:
	// Task functions
	static void initialize (void *itPtr);
	OpResult executeInitialize ();
	static void storeMessageRecords (void *itPtr);
	void executeStoreMessageRecords ();
	static void removeAllRecords (void *itPtr);
	void executeRemoveAllRecords ();

	// Open the log's database connection and return a result value
	OpResult openDatabase ();

	int stage;
	bool isDatabaseOpen;
	SDL_mutex *writeMessageListMutex;
	std::list<UiLog::Message> writeMessageList;
};
#endif
