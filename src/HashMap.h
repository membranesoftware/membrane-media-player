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
// Class that holds a map of key-value pairs
#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "StringList.h"

class Buffer;
class Json;
class JsonList;

class HashMap {
public:
	HashMap ();
	~HashMap ();

	struct Iterator {
		std::list<StdString>::iterator listIterator;
		std::map<StdString, StdString>::iterator mapIterator;
	};
	typedef bool (*SortFunction) (const StdString &a, const StdString &b);

	// Read-only data members
	bool isWriteDirty;

	// Remove all items from the map
	void clear ();

	// Return the number of items in the map
	int size () const;

	// Return a boolean value indicating if the map is empty
	bool empty () const;

	// Return a boolean value indicating if the map contains the same keys and values as another map
	bool equals (const HashMap &other) const;

	// Return a string representation of the map
	StdString toString () const;

	// Return a newly created Json object containing map values
	Json *toJson () const;

	// Clear map values and replace them with values from the provided Json object, as previously created by toJson
	void readJson (Json *json);

	// Read values from configuration file data and store the resulting items in the map, optionally clearing the map before doing so. Returns a Result value.
	OpResult read (const StdString &filename, bool shouldClear = false);
	OpResult read (Buffer *buffer, bool shouldClear = false);

	// Write values from the map to the specified file. Returns a Result value.
	OpResult write (const StdString &filename);

	// Set the sort function that should be used to order items in generated map traversals
	void sort (HashMap::SortFunction fn);

	// Insert the map's keys into the provided list, optionally clearing the list before doing so
	void getKeys (StringList *destList, bool shouldClear = false);

	// Set a key-value pair in the map. If removeValue is provided and matches value, instead remove the named key.
	void insert (const StdString &key, const StdString &value);
	void insert (const char *key, const StdString &value);
	void insert (const StdString &key, const char *value);
	void insert (const char *key, const char *value);
	void insert (const StdString &key, bool value);
	void insert (const char *key, bool value);
	void insert (const StdString &key, int value);
	void insert (const char *key, int value);
	void insert (const StdString &key, int64_t value);
	void insert (const char *key, int64_t value);
	void insert (const StdString &key, float value);
	void insert (const char *key, float value);
	void insert (const StdString &key, double value);
	void insert (const char *key, double value);
	void insert (const StdString &key, const StdString &value, const StdString &removeValue);
	void insert (const char *key, const StdString &value, const StdString &removeValue);
	void insert (const StdString &key, const char *value, const char *removeValue);
	void insert (const char *key, const char *value, const char *removeValue);
	void insert (const StdString &key, bool value, bool removeValue);
	void insert (const char *key, bool value, bool removeValue);
	void insert (const StdString &key, int value, int removeValue);
	void insert (const char *key, int value, int removeValue);
	void insert (const StdString &key, int64_t value, int64_t removeValue);
	void insert (const char *key, int64_t value, int64_t removeValue);
	void insert (const StdString &key, float value, float removeValue);
	void insert (const char *key, float value, float removeValue);
	void insert (const StdString &key, double value, double removeValue);
	void insert (const char *key, double value, double removeValue);

	// Set a key-value pair in the map to store a StringList. If value is an empty list, instead remove the named key.
	void insert (const StdString &key, const StringList &value);
	void insert (const char *key, const StringList &value);

	// Set key-value pairs in the map to store a JsonList, freeing all contained Json objects in the process. If value is an empty list, instead remove all key-value pairs for JsonList objects associated with the named base key.
	void insert (const StdString &key, JsonList *value);
	void insert (const char *key, JsonList *value);

	// Remove the specified key or list of keys from the map
	void remove (const StdString &key);
	void remove (const char *key);
	void remove (const StringList &keys);

	// Return a boolean value indicating if the provided key exists in the map
	bool exists (const StdString &key) const;
	bool exists (const char *key) const;

	// Return a string containing a value from the map, or the specified default if no such value exists
	StdString find (const StdString &key, const StdString &defaultValue) const;
	StdString find (const StdString &key, const char *defaultValue) const;
	StdString find (const char *key, const char *defaultValue) const;
	int find (const StdString &key, int defaultValue) const;
	int find (const char *key, int defaultValue) const;
	int64_t find (const StdString &key, int64_t defaultValue) const;
	int64_t find (const char *key, int64_t defaultValue) const;
	bool find (const StdString &key, bool defaultValue) const;
	bool find (const char *key, bool defaultValue) const;
	float find (const StdString &key, float defaultValue) const;
	float find (const char *key, float defaultValue) const;
	double find (const StdString &key, double defaultValue) const;
	double find (const char *key, double defaultValue) const;

	// Clear the provided StringList and fill it with items from the specified string list key if found
	void find (const StdString &key, StringList *destList) const;
	void find (const char *key, StringList *destList) const;

	// Clear the provided JsonList and free all contained Json objects, then fill it with items from the specified object list key if found
	void find (const StdString &key, JsonList *destList) const;
	void find (const char *key, JsonList *destList) const;

	// Return an iterator positioned at the map's first element
	HashMap::Iterator begin ();

	// Write an iteration's next key value to destString, advance the iterator, and return true if the operation succeeded
	bool next (HashMap::Iterator *i, StdString *destString);

	// Sort functions
	static bool sortAscending (const StdString &a, const StdString &b);
	static bool sortDescending (const StdString &a, const StdString &b);

private:
	// Clear keyList and populate it with key items in sorted order
	void doSort ();

	std::map<StdString, StdString> valueMap;
	StringList keyList;
	HashMap::SortFunction sortFunction;
	bool isSorted;
};
#endif
