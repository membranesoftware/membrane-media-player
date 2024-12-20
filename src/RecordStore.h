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
// Class that maintains a set of records stored from SystemInterface commands
#ifndef RECORD_STORE_H
#define RECORD_STORE_H

class Json;
class JsonList;
class HashMap;
class StringList;

class RecordStore {
public:
	RecordStore ();
	~RecordStore ();
	static RecordStore *instance;

	typedef bool (*FindMatchFunction) (void *data, Json *record);

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Return a UUID value generated for use with a record of type commandId. If recordTime is not provided, use the current time.
	StdString getRecordId (int commandId, int64_t recordTime = 0);

	// Return the command type value appearing in a recordId string, or -1 if no command type was parsed
	int getRecordIdCommand (const StdString &recordId);

	// Return the time value appearing in a recordId string, or -1 if no time value was parsed
	int64_t getRecordIdTime (const StdString &recordId);

	// Remove all records from the store
	void clear ();

	// Copy sourceRecord, add the copy to the record store and return true if the operation succeeded. If retainOverwrite is true, increase the record's refcount when overwriting an existing entry.
	bool insert (Json *sourceRecord, bool retainOverwrite = false);

	// Release the record with the specified ID and delete it from the record store if its refcount is 0 or less
	void remove (const StdString &recordId);

	// Release records with the specified ID values and delete any records with refcount 0 or less
	void remove (const StringList &recordIds);

	// Return true if an item matching recordId exists in the record store. If retainRecord is true, increase the record's refcount if found.
	bool exists (const StdString &recordId, bool retainRecord = false);

	// Find a record matching the specified ID with an optional type and return a copy of the associated Json object, or NULL if no such record was found. If retainRecord is true, increase the record's refcount. If a Json object is returned by this method, it must be deleted when no longer needed.
	Json *find (const StdString &recordId, int recordType = -1, bool retainRecord = false);

	// Find a record matching the specified ID with an optional type, copy its value to destJson and return true if the record was found. If retainRecord is true, increase the record's refcount.
	bool find (Json *destJson, const StdString &recordId, int recordType = -1, bool retainRecord = false);

	// Find the first record passing a match predicate function and return a copy of the associated Json object, or NULL if no such record was found. If retainRecord is true, increase the record's refcount. If a Json object is returned by this method, it must be deleted when no longer needed.
	Json *find (RecordStore::FindMatchFunction matchFn, void *matchData, bool retainRecord = false);

	// Find the first record passing a match predicate function, copy its value to destJson and return true if the record was found. If retainRecord is true, increase the record's refcount.
	bool find (Json *destJson, RecordStore::FindMatchFunction matchFn, void *matchData, bool retainRecord = false);

	// Find records using a match predicate function and insert them into the provided list, optionally clearing the list before doing so
	void findRecords (RecordStore::FindMatchFunction matchFn, void *matchData, JsonList *destList, bool shouldClear = false);

	// Find records using a match predicate function and insert their ID values into the provided list, clearing the list before doing so
	void findRecordIds (RecordStore::FindMatchFunction matchFn, void *matchData, StringList *destList);

	// Return the number of records in the store
	int countRecords ();

	// Match functions for use with find methods
	static bool matchCommandId (void *intPtr, Json *record);
	static bool matchAgentId (void *agentIdStringPtr, Json *record);

private:
	struct RecordEntry {
		Json *json;
		int refcount;
		RecordEntry ():
			json (NULL),
			refcount (0) { }
	};
	std::map<StdString, RecordStore::RecordEntry> recordMap;
	SDL_mutex *mutex;
};
#endif
