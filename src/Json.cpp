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
#include "json-parser.h"
#include "json-builder.h"
#include "Log.h"
#include "StringList.h"
#include "IntList.h"
#include "Int64List.h"
#include "FloatList.h"
#include "DoubleList.h"
#include "Json.h"

constexpr const int Json::intOverflow = 0x7FFFFFFF;

Json::Json ()
: json (NULL)
, shouldFreeJson (false)
, isJsonBuilder (false)
{
}
Json::~Json () {
	unassign ();
}

void Json::freeObject (void *jsonPtr) {
	delete ((Json *) jsonPtr);
}

bool Json::isAssigned () {
	return (json ? true : false);
}

void Json::unassign () {
	if (json) {
		if (shouldFreeJson) {
			if (isJsonBuilder) {
				json_builder_free (json);
			}
			else {
				json_value_free (json);
			}
		}
		json = NULL;
	}
	shouldFreeJson = false;
	isJsonBuilder = false;
}

void Json::resetBuilder () {
	json_value *jsoncopy;

	if (json) {
		jsoncopy = copyJsonValue (json);
		unassign ();
		json = jsoncopy;
	}
	else {
		json = json_object_new (0);
	}
	shouldFreeJson = true;
	isJsonBuilder = true;
}

Json *Json::setEmpty () {
	unassign ();
	resetBuilder ();
	return (this);
}

void Json::setJsonValue (json_value *value, bool isJsonBuilder) {
	unassign ();
	json = value;
	this->isJsonBuilder = isJsonBuilder;
}

void Json::jsonObjectPush (const json_char *name, json_value *value) {
	if ((! json) || (! isJsonBuilder)) {
		resetBuilder ();
	}
	json_object_push (json, name, value);
}

json_object_entry *Json::findJsonObjectEntry (const StdString &key, json_type entryType) const {
	int i, len;
	json_object_entry *entry;

	if (! json) {
		return (NULL);
	}
	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		entry = &(json->u.object.values[i]);
		if (key.equals (0, key.length (), entry->name, 0, entry->name_length)) {
			if ((entryType != json_none) && (entry->value->type != entryType)) {
				return (NULL);
			}
			return (entry);
		}
	}
	return (NULL);
}

void Json::setArrayEntry (const StdString &key, json_value *arrayValue) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value) {
			json_builder_free (entry->value);
		}
		entry->value = arrayValue;
		entry->value->parent = json;
	}
	else {
		jsonObjectPush (key.c_str (), arrayValue);
	}
}

bool Json::exists (const StdString &key) const {
	return (findJsonObjectEntry (key) != NULL);
}
bool Json::exists (const char *key) const {
	return (exists (StdString (key)));
}

void Json::getKeys (std::list<StdString> *destList, bool shouldClear) {
	int i, len;

	if (shouldClear) {
		destList->clear ();
	}
	if (! json) {
		return;
	}
	len = json->u.object.length;
	for (i = 0; i < len; ++i) {
		destList->push_back (StdString (json->u.object.values[i].name, json->u.object.values[i].name_length));
	}
}

bool Json::isNull (const StdString &key) const {
	return (findJsonObjectEntry (key, json_null) != NULL);
}
bool Json::isNull (const char *key) const {
	return (isNull (StdString (key)));
}

bool Json::isNumber (const StdString &key) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	return (entry && ((entry->value->type == json_integer) || (entry->value->type == json_double)));
}
bool Json::isNumber (const char *key) const {
	return (isNumber (StdString (key)));
}

bool Json::isBoolean (const StdString &key) const {
	return (findJsonObjectEntry (key, json_boolean) != NULL);
}
bool Json::isBoolean (const char *key) const {
	return (isBoolean (StdString (key)));
}

bool Json::isString (const StdString &key) const {
	return (findJsonObjectEntry (key, json_string) != NULL);
}
bool Json::isString (const char *key) const {
	return (isString (StdString (key)));
}

bool Json::isArray (const StdString &key) const {
	return (findJsonObjectEntry (key, json_array) != NULL);
}
bool Json::isArray (const char *key) const {
	return (isArray (StdString (key)));
}

bool Json::isObject (const StdString &key) const {
	return (findJsonObjectEntry (key, json_object) != NULL);
}
bool Json::isObject (const char *key) const {
	return (isObject (StdString (key)));
}

bool Json::parse (const char *data, const int dataLength) {
	json_settings settings;
	json_value *value;
	char buf[json_error_max];

	memset (&settings, 0, sizeof (settings));
	settings.value_extra = json_builder_extra;
	value = json_parse_ex (&settings, data, dataLength, buf);
	if (! value) {
		return (false);
	}

	unassign ();
	json = value;
	shouldFreeJson = true;
	isJsonBuilder = false;
	return (true);
}
bool Json::parse (const StdString &data) {
	return (parse (data.c_str (), data.length ()));
}

Json *Json::parseValue (const StdString &jsonString) {
	bool result;

	result = parse (jsonString);
	if (! result) {
		setEmpty ();
	}
	return (this);
}
Json *Json::parseValue (const char *jsonString) {
	return (parseValue (StdString (jsonString)));
}

void Json::assign (Json *otherJson) {
	if (! otherJson) {
		unassign ();
		return;
	}
	if (otherJson->json) {
		setJsonValue (otherJson->json, otherJson->isJsonBuilder);
		otherJson->json = NULL;
		shouldFreeJson = true;
	}
	else {
		unassign ();
	}
	delete (otherJson);
}

Json *Json::copyValue (Json *sourceJson) {
	setEmpty ();
	if (sourceJson && sourceJson->json) {
		setJsonValue (copyJsonValue (sourceJson->json), true);
		shouldFreeJson = true;
	}
	return (this);
}

Json *Json::copy () {
	Json *j;

	j = new Json ();
	j->copyValue (this);
	return (j);
}

json_value *Json::copyJsonValue (json_value *sourceValue) {
	json_value *value;
	json_object_entry *entry;
	StdString s;
	int i, len;

	switch (sourceValue->type) {
		case json_integer: {
			value = json_integer_new (sourceValue->u.integer);
			break;
		}
		case json_double: {
			value = json_double_new (sourceValue->u.dbl);
			break;
		}
		case json_boolean: {
			value = json_boolean_new (sourceValue->u.boolean);
			break;
		}
		case json_string: {
			s.assign (sourceValue->u.string.ptr, sourceValue->u.string.length);
			value = json_string_new (s.c_str ());
			break;
		}
		case json_array: {
			value = json_array_new (0);
			len = sourceValue->u.array.length;
			for (i = 0; i < len; ++i) {
				json_array_push (value, copyJsonValue (sourceValue->u.array.values[i]));
			}
			break;
		}
		case json_object: {
			value = json_object_new (0);
			len = sourceValue->u.object.length;
			for (i = 0; i < len; ++i) {
				entry = &(sourceValue->u.object.values[i]);
				s.assign (entry->name, entry->name_length);
				json_object_push (value, s.c_str (), copyJsonValue (entry->value));
			}
			break;
		}
		default: {
			value = json_null_new ();
			break;
		}
	}
	return (value);
}

bool Json::deepEquals (Json *other) {
	if ((! json) && (! other->json)) {
		return (true);
	}
	if ((! json) || (! other->json)) {
		return (false);
	}
	return (deepEqualsValue (json, other->json));
}
bool Json::deepEquals (const StdString &jsonString) {
	Json other;

	if (! other.parse (jsonString)) {
		return (false);
	}
	return (deepEquals (&other));
}
bool Json::deepEquals (const char *jsonString) {
	return (deepEquals (StdString (jsonString)));
}

bool Json::deepEqualsValue (json_value *thisValue, json_value *otherValue) {
	std::map<StdString, int> keymap;
	std::map<StdString, int>::iterator pos;
	int i, len;
	bool result;

	if (thisValue->type != otherValue->type) {
		return (false);
	}
	result = false;
	switch (thisValue->type) {
		case json_integer: {
			result = (thisValue->u.integer == otherValue->u.integer);
			break;
		}
		case json_double: {
			result = FLOAT_EQUALS (thisValue->u.dbl, otherValue->u.dbl);
			break;
		}
		case json_boolean: {
			result = (thisValue->u.boolean == otherValue->u.boolean);
			break;
		}
		case json_string: {
			result = StdString (thisValue->u.string.ptr, thisValue->u.string.length).equals (StdString (otherValue->u.string.ptr, otherValue->u.string.length));
			break;
		}
		case json_array: {
			len = thisValue->u.array.length;
			if (len != (int) otherValue->u.array.length) {
				result = false;
				break;
			}

			result = true;
			for (i = 0; i < len; ++i) {
				if (! deepEqualsValue (thisValue->u.array.values[i], otherValue->u.array.values[i])) {
					result = false;
					break;
				}
			}
			break;
		}
		case json_object: {
			len = thisValue->u.object.length;
			if (len != (int) otherValue->u.object.length) {
				result = false;
				break;
			}
			if (len == 0) {
				result = true;
				break;
			}

			keymap.clear ();
			for (i = 0; i < len; ++i) {
				keymap.insert (std::pair<StdString, int> (StdString (thisValue->u.object.values[i].name, thisValue->u.object.values[i].name_length), i));
			}

			result = true;
			for (i = 0; i < len; ++i) {
				pos = keymap.find (StdString (otherValue->u.object.values[i].name, otherValue->u.object.values[i].name_length));
				if (pos == keymap.end ()) {
					result = false;
					break;
				}
				if (! deepEqualsValue (thisValue->u.object.values[pos->second].value, otherValue->u.object.values[i].value)) {
					result = false;
					break;
				}
			}
			break;
		}
		default: {
			result = true;
			break;
		}
	}
	return (result);
}

int Json::getNumber (const StdString &key, int defaultValue) const {
	json_object_entry *entry;
	double n;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value->type == json_integer) {
			return (entry->value->u.integer);
		}
		else if (entry->value->type == json_double) {
			n = entry->value->u.dbl;
			if (n > (double) Json::intOverflow) {
				n = (double) Json::intOverflow;
			}
			else if (n < -((double) Json::intOverflow)) {
				n = -((double) Json::intOverflow);
			}
			return ((int) n);
		}
	}
	return (defaultValue);
}
int Json::getNumber (const char *key, int defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

int64_t Json::getNumber (const StdString &key, int64_t defaultValue) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value->type == json_integer) {
			return ((int64_t) entry->value->u.integer);
		}
		else if (entry->value->type == json_double) {
			return ((int64_t) entry->value->u.dbl);
		}
	}
	return (defaultValue);
}
int64_t Json::getNumber (const char *key, int64_t defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

double Json::getNumber (const StdString &key, double defaultValue) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value->type == json_integer) {
			return ((double) entry->value->u.integer);
		}
		else if (entry->value->type == json_double) {
			return (entry->value->u.dbl);
		}
	}
	return (defaultValue);
}
double Json::getNumber (const char *key, double defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

float Json::getNumber (const StdString &key, float defaultValue) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value->type == json_integer) {
			return ((float) entry->value->u.integer);
		}
		else if (entry->value->type == json_double) {
			return ((float) entry->value->u.dbl);
		}
	}
	return (defaultValue);
}
float Json::getNumber (const char *key, float defaultValue) const {
	return (getNumber (StdString (key), defaultValue));
}

bool Json::getBoolean (const StdString &key, bool defaultValue) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key, json_boolean);
	if (entry) {
		return (entry->value->u.boolean);
	}
	return (defaultValue);
}
bool Json::getBoolean (const char *key, bool defaultValue) const {
	return (getBoolean (StdString (key), defaultValue));
}

StdString Json::getString (const StdString &key, const StdString &defaultValue) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key, json_string);
	if (entry) {
		return (StdString (entry->value->u.string.ptr, entry->value->u.string.length));
	}
	return (defaultValue);
}
StdString Json::getString (const char *key, const StdString &defaultValue) const {
	return (getString (StdString (key), defaultValue));
}
StdString Json::getString (const StdString &key, const char *defaultValue) const {
	return (getString (key, StdString (defaultValue)));
}
StdString Json::getString (const char *key, const char *defaultValue) const {
	return (getString (StdString (key), StdString (defaultValue)));
}

bool Json::getStringList (const StdString &key, StringList *destList) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayString (key, i, StdString ()));
	}
	return (true);
}
bool Json::getStringList (const char *key, StringList *destList) const {
	return (getStringList (StdString (key), destList));
}

bool Json::getIntList (const StdString &key, IntList *destList, int defaultValue) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayNumber (key, i, defaultValue));
	}
	return (true);
}
bool Json::getIntList (const char *key, IntList *destList, int defaultValue) const {
	return (getIntList (StdString (key), destList, defaultValue));
}

bool Json::getInt64List (const StdString &key, Int64List *destList, int64_t defaultValue) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayNumber (key, i, defaultValue));
	}
	return (true);
}
bool Json::getInt64List (const char *key, Int64List *destList, int64_t defaultValue) const {
	return (getInt64List (StdString (key), destList, defaultValue));
}

bool Json::getFloatList (const StdString &key, FloatList *destList, float defaultValue) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayNumber (key, i, defaultValue));
	}
	return (true);
}
bool Json::getFloatList (const char *key, FloatList *destList, float defaultValue) const {
	return (getFloatList (StdString (key), destList, defaultValue));
}

bool Json::getDoubleList (const StdString &key, DoubleList *destList, double defaultValue) const {
	int i, len;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		destList->push_back (getArrayNumber (key, i, defaultValue));
	}
	return (true);
}
bool Json::getDoubleList (const char *key, DoubleList *destList, double defaultValue) const {
	return (getDoubleList (StdString (key), destList, defaultValue));
}

bool Json::getObjectList (const StdString &key, JsonList *destList) const {
	int i, len;
	Json *item;

	destList->clear ();
	if (! isArray (key)) {
		return (false);
	}
	len = getArrayLength (key);
	if (len <= 0) {
		return (true);
	}
	for (i = 0; i < len; ++i) {
		item = new Json ();
		if (! getArrayObject (key, i, item)) {
			item->setEmpty ();
		}
		destList->push_back (item);
	}
	return (true);
}
bool Json::getObjectList (const char *key, JsonList *destList) const {
	return (getObjectList (StdString (key), destList));
}

bool Json::getObject (const StdString &key, Json *destJson) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key, json_object);
	if (entry) {
		if (destJson) {
			destJson->setJsonValue (entry->value, isJsonBuilder);
		}
		return (true);
	}
	return (false);
}
bool Json::getObject (const char *key, Json *destJson) {
	return (getObject (StdString (key), destJson));
}

int Json::getArrayLength (const StdString &key) const {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		return (entry->value->u.array.length);
	}
	return (0);
}
int Json::getArrayLength (const char *key) const {
	return (getArrayLength (StdString (key)));
}

int Json::getArrayNumber (const StdString &key, int index, int defaultValue) const {
	json_object_entry *entry;
	json_value *item;
	double n;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_integer) {
				return (item->u.integer);
			}
			else if (item->type == json_double) {
				n = item->u.dbl;
				if (n > (double) Json::intOverflow) {
					n = (double) Json::intOverflow;
				}
				else if (n < -((double) Json::intOverflow)) {
					n = -((double) Json::intOverflow);
				}
				return ((int) n);
			}
		}
	}
	return (defaultValue);
}
int Json::getArrayNumber (const char *key, int index, int defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

int64_t Json::getArrayNumber (const StdString &key, int index, int64_t defaultValue) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_integer) {
				return ((int64_t) item->u.integer);
			}
			else if (item->type == json_double) {
				return ((int64_t) item->u.dbl);
			}
		}
	}
	return (defaultValue);
}
int64_t Json::getArrayNumber (const char *key, int index, int64_t defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

double Json::getArrayNumber (const StdString &key, int index, double defaultValue) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_integer) {
				return ((double) item->u.integer);
			}
			else if (item->type == json_double) {
				return (item->u.dbl);
			}
		}
	}
	return (defaultValue);
}
double Json::getArrayNumber (const char *key, int index, double defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

float Json::getArrayNumber (const StdString &key, int index, float defaultValue) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_integer) {
				return ((float) item->u.integer);
			}
			else if (item->type == json_double) {
				return ((float) item->u.dbl);
			}
		}
	}
	return (defaultValue);
}
float Json::getArrayNumber (const char *key, int index, float defaultValue) const {
	return (getArrayNumber (StdString (key), index, defaultValue));
}

StdString Json::getArrayString (const StdString &key, int index, const StdString &defaultValue) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_string) {
				return (StdString (item->u.string.ptr, item->u.string.length));
			}
		}
	}
	return (defaultValue);
}
StdString Json::getArrayString (const char *key, int index, const StdString &defaultValue) const {
	return (getArrayString (StdString (key), index, defaultValue));
}

bool Json::getArrayBoolean (const StdString &key, int index, bool defaultValue) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (defaultValue);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_boolean) {
				return (item->u.boolean);
			}
		}
	}
	return (defaultValue);
}
bool Json::getArrayBoolean (const char *key, int index, bool defaultValue) const {
	return (getArrayBoolean (StdString (key), index, defaultValue));
}

bool Json::getArrayObject (const StdString &key, int index, Json *destJson) const {
	json_object_entry *entry;
	json_value *item;

	if ((! json) || (index < 0)) {
		return (false);
	}
	entry = findJsonObjectEntry (key, json_array);
	if (entry) {
		if (index < (int) entry->value->u.array.length) {
			item = entry->value->u.array.values[index];
			if (item->type == json_object) {
				if (destJson) {
					destJson->setJsonValue (item, isJsonBuilder);
				}
				return (true);
			}
		}
	}
	return (false);
}
bool Json::getArrayObject (const char *key, int index, Json *destJson) const {
	return (getArrayObject (StdString (key), index, destJson));
}

Json *Json::set (const StdString &key, const char *value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value) {
			json_builder_free (entry->value);
		}
		entry->value = json_string_new (value);
		entry->value->parent = json;
	}
	else {
		jsonObjectPush (key.c_str (), json_string_new (value));
	}
	return (this);
}
Json *Json::set (const char *key, const char *value) {
	return (set (StdString (key), value));
}
Json *Json::set (const StdString &key, const StdString &value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value) {
			json_builder_free (entry->value);
		}
		entry->value = json_string_new (value.c_str ());
		entry->value->parent = json;
	}
	else {
		jsonObjectPush (key.c_str (), json_string_new (value.c_str ()));
	}
	return (this);
}
Json *Json::set (const char *key, const StdString &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const int value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value && (entry->value->type == json_integer)) {
			entry->value->u.integer = value;
		}
		else {
			if (entry->value) {
				json_builder_free (entry->value);
			}
			entry->value = json_integer_new (value);
			entry->value->parent = json;
		}
	}
	else {
		jsonObjectPush (key.c_str (), json_integer_new (value));
	}
	return (this);
}
Json *Json::set (const char *key, const int value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const int64_t value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value && (entry->value->type == json_double)) {
			entry->value->u.dbl = (double) value;
		}
		else {
			if (entry->value) {
				json_builder_free (entry->value);
			}
			entry->value = json_double_new ((double) value);
			entry->value->parent = json;
		}
	}
	else {
		jsonObjectPush (key.c_str (), json_double_new ((double) value));
	}
	return (this);
}
Json *Json::set (const char *key, const int64_t value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const float value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value && (entry->value->type == json_double)) {
			entry->value->u.dbl = (double) value;
		}
		else {
			if (entry->value) {
				json_builder_free (entry->value);
			}
			entry->value = json_double_new ((double) value);
			entry->value->parent = json;
		}
	}
	else {
		jsonObjectPush (key.c_str (), json_double_new ((double) value));
	}
	return (this);
}
Json *Json::set (const char *key, const float value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const double value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value && (entry->value->type == json_double)) {
			entry->value->u.dbl = value;
		}
		else {
			if (entry->value) {
				json_builder_free (entry->value);
			}
			entry->value = json_double_new (value);
			entry->value->parent = json;
		}
	}
	else {
		jsonObjectPush (key.c_str (), json_double_new (value));
	}
	return (this);
}
Json *Json::set (const char *key, const double value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const bool value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value && (entry->value->type == json_boolean)) {
			entry->value->u.boolean = value;
		}
		else {
			if (entry->value) {
				json_builder_free (entry->value);
			}
			entry->value = json_boolean_new (value);
			entry->value->parent = json;
		}
	}
	else {
		jsonObjectPush (key.c_str (), json_boolean_new (value));
	}
	return (this);
}
Json *Json::set (const char *key, const bool value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, Json *value) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value) {
			json_builder_free (entry->value);
		}
		if (value->json) {
			entry->value = value->json;
			value->json = NULL;
		}
		else {
			entry->value = json_object_new (0);
		}
		entry->value->parent = json;
	}
	else {
		if (value->json) {
			jsonObjectPush (key.c_str (), value->json);
			value->json = NULL;
		}
		else {
			jsonObjectPush (key.c_str (), json_object_new (0));
		}
	}
	delete (value);
	return (this);
}
Json *Json::set (const char *key, Json *value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const IntList &value) {
	json_value *a;
	IntList::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_integer_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const IntList &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const Int64List &value) {
	json_value *a;
	Int64List::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const Int64List &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const FloatList &value) {
	json_value *a;
	FloatList::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const FloatList &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const DoubleList &value) {
	json_value *a;
	DoubleList::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const DoubleList &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<int> &value) {
	json_value *a;
	std::list<int>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_integer_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<int> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<int64_t> &value) {
	json_value *a;
	std::list<int64_t>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new ((double) *i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<int64_t> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<float> &value) {
	json_value *a;
	std::list<float>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<float> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<double> &value) {
	json_value *a;
	std::list<double>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_double_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<double> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<bool> &value) {
	json_value *a;
	std::list<bool>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_boolean_new (*i1));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<bool> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, const std::list<StdString> &value) {
	json_value *a;
	std::list<StdString>::const_iterator i1, i2;

	a = json_array_new (0);
	i1 = value.cbegin ();
	i2 = value.cend ();
	while (i1 != i2) {
		json_array_push (a, json_string_new (i1->c_str ()));
		++i1;
	}
	setArrayEntry (key, a);
	return (this);
}
Json *Json::set (const char *key, const std::list<StdString> &value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, std::vector<Json *> *value) {
	json_value *a;
	std::vector<Json *>::iterator i1, i2;
	Json *item;

	a = json_array_new (0);
	i1 = value->begin ();
	i2 = value->end ();
	while (i1 != i2) {
		item = *i1;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		delete (item);
		++i1;
	}
	setArrayEntry (key, a);
	value->clear ();
	return (this);
}
Json *Json::set (const char *key, std::vector<Json *> *value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, std::list<Json *> *value) {
	json_value *a;
	std::list<Json *>::iterator i1, i2;
	Json *item;

	a = json_array_new (0);
	i1 = value->begin ();
	i2 = value->end ();
	while (i1 != i2) {
		item = *i1;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		delete (item);
		++i1;
	}
	setArrayEntry (key, a);
	value->clear ();
	return (this);
}
Json *Json::set (const char *key, std::list<Json *> *value) {
	return (set (StdString (key), value));
}

Json *Json::set (const StdString &key, JsonList *value) {
	json_value *a;
	JsonList::iterator i1, i2;
	Json *item;

	a = json_array_new (0);
	i1 = value->begin ();
	i2 = value->end ();
	while (i1 != i2) {
		item = *i1;
		if (! item->json) {
			item->setEmpty ();
		}
		json_array_push (a, item->json);
		item->json = NULL;
		++i1;
	}
	setArrayEntry (key, a);
	value->clear ();
	return (this);
}
Json *Json::set (const char *key, JsonList *value) {
	return (set (StdString (key), value));
}

Json *Json::setSprintf (const StdString &key, const char *str, ...) {
	va_list ap;
	StdString s;

	va_start (ap, str);
	s.vsprintf (str, ap);
	va_end (ap);
	return (set (key, s));
}
Json *Json::setSprintf (const char *key, const char *str, ...) {
	va_list ap;
	StdString s;

	va_start (ap, str);
	s.vsprintf (str, ap);
	va_end (ap);
	return (set (key, s));
}

Json *Json::setNull (const StdString &key) {
	json_object_entry *entry;

	entry = findJsonObjectEntry (key);
	if (entry) {
		if (entry->value) {
			json_builder_free (entry->value);
		}
		entry->value = json_null_new ();
		entry->value->parent = json;
	}
	else {
		jsonObjectPush (key.c_str (), json_null_new ());
	}
	return (this);
}
Json *Json::setNull (const char *key) {
	return (setNull (StdString (key)));
}

StdString Json::toString () {
	StdString s;
	char *buf;
	json_serialize_opts opts;
	int len;

	if (! json) {
		return (StdString ());
	}

	// TODO: Possibly employ a locking mechanism here (json_measure_ex modifies json data structures while measuring)

	opts.mode = json_serialize_mode_packed;
	opts.opts = json_serialize_opt_no_space_after_colon | json_serialize_opt_no_space_after_comma | json_serialize_opt_pack_brackets;
	opts.indent_size = 0;
	len = json_measure_ex (json, opts);
	if (len <= 0) {
		return (StdString ());
	}

	buf = (char *) malloc (len);
	if (! buf) {
		Log::err ("Out of memory in Json::toString; len=%i", len);
		return (StdString ());
	}

	json_serialize_ex (buf, json, opts);
	s.assign (buf);
	free (buf);

	return (s);
}

JsonList::JsonList ()
: std::list<Json *> ()
{
}
JsonList::~JsonList () {
	clear ();
}

void JsonList::clear () {
	JsonList::iterator i1, i2;

	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		delete (*i1);
		++i1;
	}
	std::list<Json *>::clear ();
}

StdString JsonList::toString () {
	JsonList::iterator i1, i2;
	StdString s;
	bool first;

	s.assign ("[");
	first = true;
	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		if (first) {
			first = false;
		}
		else {
			s.append (",");
		}
		s.append ((*i1)->toString ());
		++i1;
	}
	s.append ("]");
	return (s);
}

void JsonList::copyValues (JsonList *sourceList) {
	JsonList::iterator i1, i2;

	clear ();
	i1 = sourceList->begin ();
	i2 = sourceList->end ();
	while (i1 != i2) {
		push_back ((*i1)->copy ());
		++i1;
	}
}

JsonList *JsonList::copy () {
	JsonList *j;

	j = new JsonList ();
	j->copyValues (this);
	return (j);
}

Json *JsonList::at (int index) {
	JsonList::iterator i1, i2;
	int listindex;

	if ((index < 0) || (index >= (int) size ())) {
		return (NULL);
	}
	listindex = 0;
	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		if (index == listindex) {
			return (*i1);
		}
		++listindex;
		++i1;
	}
	return (NULL);
}
