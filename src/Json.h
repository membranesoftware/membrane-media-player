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
// Class that handles JSON parsing functions
#ifndef JSON_H
#define JSON_H

#include "json-parser.h"
#include "json-builder.h"

class StringList;
class IntList;
class Int64List;
class FloatList;
class DoubleList;
class JsonList;

class Json {
public:
	Json ();
	~Json ();

	static const int intOverflow;

	// Free the provided Json object pointer
	static void freeObject (void *jsonPtr);

	// Return a boolean value indicating whether the Json object holds a value
	bool isAssigned ();

	// Clear any stored json_value, leaving the Json object with an unassigned value
	void unassign ();

	// Reassign the stored json_value to a newly created empty object, freeing any value that might already be present, and return the this pointer
	Json *setEmpty ();

	// Parse a JSON string and store the resulting data. Returns a boolean value indicating if the parse succeeded.
	bool parse (const StdString &data);
	bool parse (const char *data, const int dataLength);

	// Replace the Json object's content with fields parsed from a JSON string or an empty object if the parse failed, and return the this pointer
	Json *parseValue (const StdString &jsonString);
	Json *parseValue (const char *jsonString);

	// Return the object's contents as a JSON string
	StdString toString ();

	// Replace the Json object's content with another object's json pointer and free the other object
	void assign (Json *otherJson);

	// Replace the Json object's content with a copy of the provided source object and return the this pointer
	Json *copyValue (Json *sourceJson);

	// Return a newly created Json object with contents copied from this object
	Json *copy ();

	// Return a boolean value indicating if the object's content matches that of another
	bool deepEquals (Json *other);
	bool deepEquals (const StdString &jsonString);
	bool deepEquals (const char *jsonString);

	// Store a list of currently available map keys in the provided list, optionally clearing the list before doing so
	void getKeys (std::list<StdString> *keyVector, bool shouldClear = false);

	// Return a boolean value indicating if the specified key exists in the object
	bool exists (const StdString &key) const;
	bool exists (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a null value
	bool isNull (const StdString &key) const;
	bool isNull (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a number
	bool isNumber (const StdString &key) const;
	bool isNumber (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a boolean
	bool isBoolean (const StdString &key) const;
	bool isBoolean (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is a string
	bool isString (const StdString &key) const;
	bool isString (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is an array
	bool isArray (const StdString &key) const;
	bool isArray (const char *key) const;

	// Return a boolean value indicating if the specified key exists and is an object
	bool isObject (const StdString &key) const;
	bool isObject (const char *key) const;

	// Return the value of the named number key, or the provided default if no such key was found
	int getNumber (const StdString &key, int defaultValue) const;
	int getNumber (const char *key, int defaultValue) const;
	int64_t getNumber (const StdString &key, int64_t defaultValue) const;
	int64_t getNumber (const char *key, int64_t defaultValue) const;
	double getNumber (const StdString &key, double defaultValue) const;
	double getNumber (const char *key, double defaultValue) const;
	float getNumber (const StdString &key, float defaultValue) const;
	float getNumber (const char *key, float defaultValue) const;

	// Return the value of the named boolean key, or the provided default if no such key was found
	bool getBoolean (const StdString &key, bool defaultValue) const;
	bool getBoolean (const char *key, bool defaultValue) const;

	// Return the value of the named string key, or the provided default if no such key was found
	StdString getString (const StdString &key, const StdString &defaultValue) const;
	StdString getString (const char *key, const StdString &defaultValue) const;
	StdString getString (const StdString &key, const char *defaultValue) const;
	StdString getString (const char *key, const char *defaultValue) const;

	// Get the named string array key and store its items into destList, clearing the list first. Returns a boolean value indicating if the array was found. Non-string items in the targeted array are entered into destList as empty strings.
	bool getStringList (const StdString &key, StringList *destList) const;
	bool getStringList (const char *key, StringList *destList) const;

	// Get the named number array key and store its items into destList, clearing the list first. Returns a boolean value indicating if the array was found. Non-number items in the targeted array are entered into destList as defaultValue.
	bool getIntList (const StdString &key, IntList *destList, int defaultValue) const;
	bool getIntList (const char *key, IntList *destList, int defaultValue) const;
	bool getInt64List (const StdString &key, Int64List *destList, int64_t defaultValue) const;
	bool getInt64List (const char *key, Int64List *destList, int64_t defaultValue) const;
	bool getFloatList (const StdString &key, FloatList *destList, float defaultValue) const;
	bool getFloatList (const char *key, FloatList *destList, float defaultValue) const;
	bool getDoubleList (const StdString &key, DoubleList *destList, double defaultValue) const;
	bool getDoubleList (const char *key, DoubleList *destList, double defaultValue) const;

	// Get the named object array key and store its items into destList, clearing the list first. Returns a boolean value indicating if the array was found. Non-object items in the targeted array are entered into destList as empty objects.
	bool getObjectList (const StdString &key, JsonList *destList) const;
	bool getObjectList (const char *key, JsonList *destList) const;

	// Get the named object key and store its value in the provided Json object. Returns a boolean value indicating if the object was found.
	bool getObject (const StdString &key, Json *destJson);
	bool getObject (const char *key, Json *destJson);

	// Return the length of the named array key. A return value of zero indicates an empty or nonexistent array.
	int getArrayLength (const StdString &key) const;
	int getArrayLength (const char *key) const;

	// Return an item from a number array, or the provided default if no such item was found
	int getArrayNumber (const StdString &key, int index, int defaultValue) const;
	int getArrayNumber (const char *key, int index, int defaultValue) const;
	int64_t getArrayNumber (const StdString &key, int index, int64_t defaultValue) const;
	int64_t getArrayNumber (const char *key, int index, int64_t defaultValue) const;
	double getArrayNumber (const StdString &key, int index, double defaultValue) const;
	double getArrayNumber (const char *key, int index, double defaultValue) const;
	float getArrayNumber (const StdString &key, int index, float defaultValue) const;
	float getArrayNumber (const char *key, int index, float defaultValue) const;

	// Get a string item from the named array key return its value, or the specified default value if no such item was found
	StdString getArrayString (const StdString &key, int index, const StdString &defaultValue) const;
	StdString getArrayString (const char *key, int index, const StdString &defaultValue) const;

	// Get a boolean item from the named array key return its value, or the specified default value if no such item was found
	bool getArrayBoolean (const StdString &key, int index, bool defaultValue) const;
	bool getArrayBoolean (const char *key, int index, bool defaultValue) const;

	// Get an object item from the named array key and store its value in the provided Json object. Returns a boolean value indicating if the object was found.
	bool getArrayObject (const StdString &key, int index, Json *destJson) const;
	bool getArrayObject (const char *key, int index, Json *destJson) const;

	// Set a key-value pair in the map and return the this pointer
	Json *set (const StdString &key, const char *value);
	Json *set (const char *key, const char *value);
	Json *set (const StdString &key, const StdString &value);
	Json *set (const char *key, const StdString &value);
	Json *set (const StdString &key, const int value);
	Json *set (const char *key, const int value);
	Json *set (const StdString &key, const int64_t value);
	Json *set (const char *key, const int64_t value);
	Json *set (const StdString &key, const float value);
	Json *set (const char *key, const float value);
	Json *set (const StdString &key, const double value);
	Json *set (const char *key, const double value);
	Json *set (const StdString &key, const bool value);
	Json *set (const char *key, const bool value);
	Json *set (const StdString &key, Json *value);
	Json *set (const char *key, Json *value);
	Json *set (const StdString &key, const IntList &value);
	Json *set (const char *key, const IntList &value);
	Json *set (const StdString &key, const Int64List &value);
	Json *set (const char *key, const Int64List &value);
	Json *set (const StdString &key, const FloatList &value);
	Json *set (const char *key, const FloatList &value);
	Json *set (const StdString &key, const DoubleList &value);
	Json *set (const char *key, const DoubleList &value);
	Json *set (const StdString &key, const std::list<int> &value);
	Json *set (const char *key, const std::list<int> &value);
	Json *set (const StdString &key, const std::list<int64_t> &value);
	Json *set (const char *key, const std::list<int64_t> &value);
	Json *set (const StdString &key, const std::list<float> &value);
	Json *set (const char *key, const std::list<float> &value);
	Json *set (const StdString &key, const std::list<double> &value);
	Json *set (const char *key, const std::list<double> &value);
	Json *set (const StdString &key, const std::list<bool> &value);
	Json *set (const char *key, const std::list<bool> &value);
	Json *set (const StdString &key, const std::list<StdString> &value);
	Json *set (const char *key, const std::list<StdString> &value);
	Json *set (const StdString &key, std::vector<Json *> *value);
	Json *set (const char *key, std::vector<Json *> *value);
	Json *set (const StdString &key, std::list<Json *> *value);
	Json *set (const char *key, std::list<Json *> *value);
	Json *set (const StdString &key, JsonList *value);
	Json *set (const char *key, JsonList *value);

	// Set a string value in the map using a format string and return the this pointer
	Json *setSprintf (const StdString &key, const char *str, ...) __attribute__((format(printf, 3, 4)));
	Json *setSprintf (const char *key, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Set a key in the map to a null value and return the this pointer
	Json *setNull (const StdString &key);
	Json *setNull (const char *key);

private:
	// Set the json value to a newly created builder object
	void resetBuilder ();

	// Use the json_object_push method to set a key in the json object, creating the object if needed
	void jsonObjectPush (const json_char *name, json_value *value);

	// Return the json_object_entry matching key, or NULL if no such entry was found. If entryType is provided, return an entry only if it matches that type.
	json_object_entry *findJsonObjectEntry (const StdString &key, json_type entryType = json_none) const;

	// Set a key in the json object to a value with an array type
	void setArrayEntry (const StdString &key, json_value *arrayValue);

	// Reassign the json pointer, clearing any pointer that might already be present
	void setJsonValue (json_value *value, bool isJsonBuilder);

	// Return a newly created json_value object containing a copy of the provided source value's data
	json_value *copyJsonValue (json_value *sourceValue);

	// Return a boolean value indicating if a value's content matches that of a value from another object
	bool deepEqualsValue (json_value *thisValue, json_value *otherValue);

	json_value *json;
	bool shouldFreeJson;
	bool isJsonBuilder;
};

// Json list class that extends std::list<Json *> and frees all contained Json objects when destroyed
class JsonList : public std::list<Json *> {
public:
	JsonList ();
	virtual ~JsonList ();

	// Remove all elements from the list and free all contained Json objects
	void clear ();

	// Return a string containing the list as a JSON array
	StdString toString ();

	// Remove all elements from the list and add copies of all objects in sourceList
	void copyValues (JsonList *sourceList);

	// Return a newly created JsonList object with contents copied from this object
	JsonList *copy ();

	// Return the Json object at the specified list index, or NULL if no object was found
	Json *at (int index);
};
#endif
