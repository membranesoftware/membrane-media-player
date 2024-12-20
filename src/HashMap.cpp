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
#include "Buffer.h"
#include "OsUtil.h"
#include "Json.h"
#include "HashMap.h"

HashMap::HashMap ()
: isWriteDirty (false)
, sortFunction (NULL)
, isSorted (false)
{
}
HashMap::~HashMap () {
}

void HashMap::clear () {
	valueMap.clear ();
	keyList.clear ();
}

int HashMap::size () const {
	return (valueMap.size ());
}

bool HashMap::empty () const {
	return (valueMap.empty ());
}

bool HashMap::equals (const HashMap &other) const {
	std::map<StdString, StdString>::const_iterator i1, i2;

	if (size () != other.size ()) {
		return (false);
	}
	i1 = valueMap.cbegin ();
	i2 = valueMap.cend ();
	while (i1 != i2) {
		if (! other.find (i1->first, "").equals (i1->second)) {
			return (false);
		}
		++i1;
	}
	return (true);
}

StdString HashMap::toString () const {
	StdString s;
	std::map<StdString, StdString>::const_iterator i1, i2;

	s.assign ("{");
	i1 = valueMap.cbegin ();
	i2 = valueMap.cend ();
	while (i1 != i2) {
		s.append (" ");
		s.append (i1->first);
		s.append ("=\"");
		s.append (i1->second);
		s.append ("\"");
		++i1;
	}
	s.append (" }");
	return (s);
}

Json *HashMap::toJson () const {
	Json *json;
	std::map<StdString, StdString>::const_iterator i1, i2;

	json = new Json ();
	json->setEmpty ();
	i1 = valueMap.cbegin ();
	i2 = valueMap.cend ();
	while (i1 != i2) {
		json->set (i1->first, i1->second);
		++i1;
	}
	return (json);
}

void HashMap::readJson (Json *json) {
	StringList keys;
	StringList::iterator i1, i2;
	StdString s;

	clear ();
	json->getKeys (&keys);
	i1 = keys.begin ();
	i2 = keys.end ();
	while (i1 != i2) {
		s = json->getString (*i1, "");
		if (! s.empty ()) {
			insert (*i1, s);
		}
		++i1;
	}
}

void HashMap::sort (HashMap::SortFunction fn) {
	sortFunction = fn;
	isSorted = false;
}

void HashMap::doSort () {
	std::map<StdString, StdString>::const_iterator i1, i2;

	keyList.clear ();
	if (sortFunction) {
		i1 = valueMap.cbegin ();
		i2 = valueMap.cend ();
		while (i1 != i2) {
			keyList.push_back (i1->first);
			++i1;
		}
		keyList.sort (sortFunction);
	}
	isSorted = true;
}

void HashMap::getKeys (StringList *destList, bool shouldClear) {
	HashMap::Iterator i;
	StdString key;

	if (shouldClear) {
		destList->clear ();
	}
	i = begin ();
	while (next (&i, &key)) {
		destList->push_back (key);
	}
}

bool HashMap::sortAscending (const StdString &a, const StdString &b) {
	if (a.lowercased ().compare (b.lowercased ()) < 0) {
		return (true);
	}
	return (false);
}

bool HashMap::sortDescending (const StdString &a, const StdString &b) {
	if (a.lowercased ().compare (b.lowercased ()) > 0) {
		return (true);
	}
	return (false);
}

OpResult HashMap::read (const StdString &filename, bool shouldClear) {
	Buffer *buffer;
	OpResult result;

	buffer = OsUtil::readFile (filename);
	if (! buffer) {
		return (OpResult::FileOpenFailedError);
	}
	result = read (buffer, shouldClear);
	delete (buffer);
	return (result);
}

OpResult HashMap::read (Buffer *buffer, bool shouldClear) {
	uint8_t *data, *end, *key1, *key2, *val1, *val2;
	char c;
	bool iscomment;
	StdString key, val;

	if (shouldClear) {
		clear ();
	}
	data = buffer->data;
	end = data + buffer->length;
	iscomment = false;
	key1 = NULL;
	key2 = NULL;
	val1 = NULL;
	val2 = NULL;
	while (data < end) {
		c = (char) *data;
		++data;

		if (iscomment) {
			if (c == '\n') {
				iscomment = false;
			}
			continue;
		}
		if (! key1) {
			if (c == '#') {
				iscomment = true;
				key1 = NULL;
				key2 = NULL;
				val1 = NULL;
				val2 = NULL;
				continue;
			}
			if (! isspace (c)) {
				key1 = (data - 1);
			}
		}
		else if (! key2) {
			if (isspace (c)) {
				key2 = (data - 2);
			}
		}
		else if (! val1) {
			if (! isspace (c)) {
				val1 = (data - 1);
			}
		}
		else {
			if ((c == '\r') || (c == '\n')) {
				val2 = (data - 2);
			}
		}

		if (key1 && key2 && val1 && val2) {
			key.assign ((char *) key1, key2 - key1 + 1);
			val.assign ((char *) val1, val2 - val1 + 1);
			if ((key.length () > 0) && (val.length () > 0)) {
				valueMap.insert (std::pair<StdString, StdString> (key, val));
			}
		}

		if ((c == '\r') || (c == '\n')) {
			key1 = NULL;
			key2 = NULL;
			val1 = NULL;
			val2 = NULL;
		}
	}
	return (OpResult::Success);
}

OpResult HashMap::write (const StdString &filename) {
	std::map<StdString, StdString>::iterator i1, i2;
	StdString out;

	isWriteDirty = false;
	out.assign ("");
	i1 = valueMap.begin ();
	i2 = valueMap.end ();
	while (i1 != i2) {
		out.append (i1->first);
		out.append (" ");
		out.append (i1->second);
		out.append ("\n");
		++i1;
	}
	return (OsUtil::writeFile (filename, out.createBuffer ()));
}

bool HashMap::exists (const StdString &key) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	return (i != valueMap.cend ());
}

bool HashMap::exists (const char *key) const {
	return (exists (StdString (key)));
}

void HashMap::insert (const StdString &key, const StdString &value) {
	std::map<StdString, StdString>::iterator i;

	i = valueMap.find (key);
	if (i != valueMap.end ()) {
		if (! i->second.equals (value)) {
			i->second.assign (value);
			isWriteDirty = true;
		}
	}
	else {
		valueMap.insert (std::pair<StdString, StdString> (key, value));
		isWriteDirty = true;
		isSorted = false;
	}
}

void HashMap::insert (const char *key, const StdString &value) {
	insert (StdString (key), value);
}

void HashMap::insert (const StdString &key, const char *value) {
	insert (key, StdString (value));
}

void HashMap::insert (const char *key, const char *value) {
	insert (StdString (key), StdString (value));
}

void HashMap::insert (const StdString &key, bool value) {
	insert (key, value ? StdString ("true") : StdString ("false"));
}

void HashMap::insert (const char *key, bool value) {
	insert (StdString (key), value ? StdString ("true") : StdString ("false"));
}

void HashMap::insert (const StdString &key, int value) {
	insert (key, StdString::createSprintf ("%i", value));
}

void HashMap::insert (const char *key, int value) {
	insert (StdString (key), StdString::createSprintf ("%i", value));
}

void HashMap::insert (const StdString &key, int64_t value) {
	insert (key, StdString::createSprintf ("%lli", (long long int) value));
}

void HashMap::insert (const char *key, int64_t value) {
	insert (StdString (key), StdString::createSprintf ("%lli", (long long int) value));
}

void HashMap::insert (const StdString &key, float value) {
	insert (key, StdString::createSprintf ("%f", value));
}

void HashMap::insert (const char *key, float value) {
	insert (StdString (key), StdString::createSprintf ("%f", value));
}

void HashMap::insert (const StdString &key, double value) {
	insert (key, StdString::createSprintf ("%f", value));
}

void HashMap::insert (const char *key, double value) {
	insert (StdString (key), StdString::createSprintf ("%f", value));
}

void HashMap::insert (const StdString &key, const StringList &value) {
	if (value.empty ()) {
		remove (key);
	}
	else {
		insert (key, value.toJsonString ());
	}
}

void HashMap::insert (const char *key, const StringList &value) {
	insert (StdString (key), value);
}

void HashMap::insert (const StdString &key, JsonList *value) {
	std::map<StdString, StdString>::iterator i1, i2;
	JsonList::iterator j1, j2;
	Json *j;
	StringList keys;
	StdString s;
	bool match;
	char *c;
	int index;

	i1 = valueMap.begin ();
	i2 = valueMap.end ();
	while (i1 != i2) {
		s.assign (i1->first);
		match = false;
		if ((s.length () > (key.length () + 1)) && s.startsWith (key)) {
			c = (char *) s.c_str ();
			c += key.length ();
			if ((*c) == '_') {
				while (true) {
					++c;
					if (*c == '\0') {
						match = true;
						break;
					}
					if ((*c < '0') || (*c > '9')) {
						break;
					}
				}
			}
		}
		if (match) {
			keys.push_back (s);
		}
		++i1;
	}
	remove (keys);

	index = 1;
	j1 = value->begin ();
	j2 = value->end ();
	while (j1 != j2) {
		j = *j1;
		if (! j->isAssigned ()) {
			j->setEmpty ();
		}
		insert (StdString::createSprintf ("%s_%i", key.c_str (), index), j->toString ());
		++index;
		++j1;
	}
	value->clear ();
}

void HashMap::insert (const char *key, JsonList *value) {
	insert (StdString (key), value);
}

void HashMap::insert (const StdString &key, const StdString &value, const StdString &removeValue) {
	if (value.equals (removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, const StdString &value, const StdString &removeValue) {
	if (value.equals (removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, const char *value, const char *removeValue) {
	if (StdString (value).equals (StdString (removeValue))) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, const char *value, const char *removeValue) {
	if (StdString (value).equals (StdString (removeValue))) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, bool value, bool removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, bool value, bool removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, int value, int removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, int value, int removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, int64_t value, int64_t removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, int64_t value, int64_t removeValue) {
	if (value == removeValue) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, float value, float removeValue) {
	if (FLOAT_EQUALS (value, removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, float value, float removeValue) {
	if (FLOAT_EQUALS (value, removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const StdString &key, double value, double removeValue) {
	if (FLOAT_EQUALS (value, removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::insert (const char *key, double value, double removeValue) {
	if (FLOAT_EQUALS (value, removeValue)) {
		remove (key);
	}
	else {
		insert (key, value);
	}
}

void HashMap::remove (const StdString &key) {
	std::map<StdString, StdString>::iterator i;

	i = valueMap.find (key);
	if (i != valueMap.end ()) {
		valueMap.erase (i);
		isWriteDirty = true;
		isSorted = false;
	}
}

void HashMap::remove (const char *key) {
	remove (StdString (key));
}

void HashMap::remove (const StringList &keys) {
	StringList::const_iterator i1, i2;

	i1 = keys.cbegin ();
	i2 = keys.cend ();
	while (i1 != i2) {
		remove (*i1);
		++i1;
	}
}

StdString HashMap::find (const StdString &key, const StdString &defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (i->second);
}

StdString HashMap::find (const StdString &key, const char *defaultValue) const {
	return (find (key, StdString (defaultValue)));
}

StdString HashMap::find (const char *key, const char *defaultValue) const {
	return (find (StdString (key), StdString (defaultValue)));
}

int HashMap::find (const StdString &key, int defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atoi (i->second.c_str ()));
}

int HashMap::find (const char *key, int defaultValue) const {
	return (find (StdString (key), defaultValue));
}

int64_t HashMap::find (const StdString &key, int64_t defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atoll (i->second.c_str ()));
}

int64_t HashMap::find (const char *key, int64_t defaultValue) const {
	return (find (StdString (key), defaultValue));
}

bool HashMap::find (const StdString &key, bool defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (i->second.lowercased ().equals ("true") || i->second.lowercased ().equals ("yes"));
}

bool HashMap::find (const char *key, bool defaultValue) const {
	return (find (StdString (key), defaultValue));
}

float HashMap::find (const StdString &key, float defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return ((float) atof (i->second.c_str ()));
}

float HashMap::find (const char *key, float defaultValue) const {
	return (find (StdString (key), defaultValue));
}

double HashMap::find (const StdString &key, double defaultValue) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		return (defaultValue);
	}
	return (atof (i->second.c_str ()));
}

double HashMap::find (const char *key, double defaultValue) const {
	return (find (StdString (key), defaultValue));
}

void HashMap::find (const StdString &key, StringList *destList) const {
	std::map<StdString, StdString>::const_iterator i;

	i = valueMap.find (key);
	if (i == valueMap.end ()) {
		destList->clear ();
		return;
	}
	if (! destList->parseJsonString (i->second)) {
		destList->clear ();
	}
}

void HashMap::find (const char *key, StringList *destList) const {
	find (StdString (key), destList);
}

void HashMap::find (const StdString &key, JsonList *destList) const {
	Json *json;
	StdString s;
	int index;

	destList->clear ();
	index = 1;
	while (true) {
		s = find (StdString::createSprintf ("%s_%i", key.c_str (), index), "");
		if (s.empty ()) {
			break;
		}
		json = new Json ();
		if (! json->parse (s)) {
			delete (json);
			break;
		}
		destList->push_back (json);
		++index;
	}
}

void HashMap::find (const char *key, JsonList *destList) const {
	find (StdString (key), destList);
}

HashMap::Iterator HashMap::begin () {
	HashMap::Iterator i;

	if (sortFunction) {
		if (! isSorted) {
			doSort ();
		}
		i.listIterator = keyList.begin ();
	}
	else {
		i.mapIterator = valueMap.begin ();
	}
	return (i);
}

bool HashMap::next (HashMap::Iterator *i, StdString *destString) {
	if (sortFunction) {
		if (i->listIterator != keyList.end ()) {
			if (destString) {
				destString->assign (*(i->listIterator));
			}
			++(i->listIterator);
			return (true);
		}
	}
	else {
		if (i->mapIterator != valueMap.end ()) {
			if (destString) {
				destString->assign (i->mapIterator->first);
			}
			++(i->mapIterator);
			return (true);
		}
	}
	return (false);
}
