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
#include "Json.h"
#include "StringList.h"

StringList::StringList ()
: std::list<StdString> ()
{
}
StringList::StringList (const StringList &copySource)
: StringList ()
{
	append (copySource);
}
StringList::StringList (const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, const char *s8)
: StringList ()
{
	append (s1, s2, s3, s4, s5, s6, s7, s8);
}
StringList::StringList (const StdString &s1, const StdString &s2, const StdString &s3, const StdString &s4, const StdString &s5, const StdString &s6, const StdString &s7, const StdString &s8)
: StringList ()
{
	append (s1, s2, s3, s4, s5, s6, s7, s8);
}

StringList::~StringList () {
}

StdString StringList::toString () const {
	return (join (","));
}

StdString StringList::toJsonString () const {
	StdString s, item;
	StringList::const_iterator i1, i2;
	bool first;

	s.assign ("[");
	first = true;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		item = *i1;
		if (first) {
			first = false;
		}
		else {
			s.append (",");
		}
		s.append ("\"");
		s.append (item.jsonEscaped ());
		s.append ("\"");
		++i1;
	}
	s.append ("]");
	return (s);
}

StringList *StringList::copy () const {
	StringList *stringlist;

	stringlist = new StringList ();
	stringlist->append (*this);
	return (stringlist);
}

bool StringList::parseJsonString (const StdString &jsonString) {
	Json j;
	StdString s, json;
	constexpr const char *key = "a";
	int i, len;

	s.assign (jsonString.trimmed ());
	if (s.empty ()) {
		clear ();
		return (true);
	}
	if (! s.startsWith ("[")) {
		return (false);
	}
	json.sprintf ("{\"%s\":", key);
	json.append (s);
	json.append ("}");
	if (! j.parse (json)) {
		return (false);
	}
	clear ();
	len = j.getArrayLength (key);
	for (i = 0; i < len; ++i) {
		push_back (j.getArrayString (key, i, StdString ()));
	}
	return (true);
}

void StringList::insertInOrder (const StdString &item) {
	StringList::iterator i1, i2;
	bool found;

	found = false;
	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		if (item.compare (*i1) < 0) {
			found = true;
			insert (i1, item);
			break;
		}
		++i1;
	}
	if (! found) {
		push_back (item);
	}
}

void StringList::append (const StringList &sourceList) {
	StringList::const_iterator i1, i2;

	i1 = sourceList.cbegin ();
	i2 = sourceList.cend ();
	while (i1 != i2) {
		push_back (*i1);
		++i1;
	}
}

void StringList::append (const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, const char *s8) {
	if (s1 && (*s1 != '\0')) {
		push_back (StdString (s1));
	}
	if (s2 && (*s2 != '\0')) {
		push_back (StdString (s2));
	}
	if (s3 && (*s3 != '\0')) {
		push_back (StdString (s3));
	}
	if (s4 && (*s4 != '\0')) {
		push_back (StdString (s4));
	}
	if (s5 && (*s5 != '\0')) {
		push_back (StdString (s5));
	}
	if (s6 && (*s6 != '\0')) {
		push_back (StdString (s6));
	}
	if (s7 && (*s7 != '\0')) {
		push_back (StdString (s7));
	}
	if (s8 && (*s8 != '\0')) {
		push_back (StdString (s8));
	}
}

void StringList::append (const StdString &s1, const StdString &s2, const StdString &s3, const StdString &s4, const StdString &s5, const StdString &s6, const StdString &s7, const StdString &s8) {
	if (! s1.empty ()) {
		push_back (s1);
	}
	if (! s2.empty ()) {
		push_back (s2);
	}
	if (! s3.empty ()) {
		push_back (s3);
	}
	if (! s4.empty ()) {
		push_back (s4);
	}
	if (! s5.empty ()) {
		push_back (s5);
	}
	if (! s6.empty ()) {
		push_back (s6);
	}
	if (! s7.empty ()) {
		push_back (s7);
	}
	if (! s8.empty ()) {
		push_back (s8);
	}
}

void StringList::assign (const char *s1, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, const char *s8) {
	clear ();
	append (s1, s2, s3, s4, s5, s6, s7, s8);
}

void StringList::assign (const StdString &s1, const StdString &s2, const StdString &s3, const StdString &s4, const StdString &s5, const StdString &s6, const StdString &s7, const StdString &s8) {
	clear ();
	append (s1, s2, s3, s4, s5, s6, s7, s8);
}

void StringList::assign (const StringList &sourceList) {
	clear ();
	append (sourceList);
}

void StringList::remove (const StdString &item) {
	StringList::iterator i1, i2;
	bool found;

	while (true) {
		found = false;
		i1 = begin ();
		i2 = end ();
		while (i1 != i2) {
			if (i1->equals (item)) {
				found = true;
				erase (i1);
				break;
			}
			++i1;
		}
		if (! found) {
			break;
		}
	}
}

bool StringList::contains (const StdString &item) const {
	StringList::const_iterator i1, i2;
	bool found;

	found = false;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (item.equals (*i1)) {
			found = true;
			break;
		}
		++i1;
	}
	return (found);
}

int StringList::indexOf (const StdString &item) const {
	StringList::const_iterator i1, i2;
	int result, count;

	result = -1;
	count = 0;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (item.equals (*i1)) {
			result = count;
			break;
		}
		++count;
		++i1;
	}
	return (result);
}

int StringList::indexOf (const char *item) const {
	return (indexOf (StdString (item)));
}

StdString StringList::at (int index) const {
	StringList::const_iterator i1, i2;
	int listindex;

	if ((index < 0) || (index >= (int) size ())) {
		return (StdString ());
	}
	listindex = 0;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (index == listindex) {
			return (*i1);
		}
		++listindex;
		++i1;
	}
	return (StdString ());
}

bool StringList::equals (const StringList &otherList) const {
	StringList::const_iterator i1, i2, j1, j2;

	if (size () != otherList.size ()) {
		return (false);
	}
	i1 = cbegin ();
	i2 = cend ();
	j1 = otherList.cbegin ();
	j2 = otherList.cend ();
	while (i1 != i2) {
		if (j1 == j2) {
			return (false);
		}
		if (! (*i1).equals (*j1)) {
			return (false);
		}
		++i1;
		++j1;
	}
	return (true);
}

void StringList::sort (StringList::SortFunction sortFunction) {
	std::list<StdString>::sort (sortFunction);
}

bool StringList::compareAscending (const StdString &a, const StdString &b) {
	return (a.compare (b) < 0);
}

bool StringList::compareDescending (const StdString &a, const StdString &b) {
	return (a.compare (b) > 0);
}

bool StringList::compareCaseInsensitiveAscending (const StdString &a, const StdString &b) {
	return (a.lowercased ().compare (b.lowercased ()) < 0);
}

bool StringList::compareCaseInsensitiveDescending (const StdString &a, const StdString &b) {
	return (a.lowercased ().compare (b.lowercased ()) > 0);
}

StdString StringList::join (const StdString &delimiter) const {
	StdString s;
	StringList::const_iterator i1, i2;
	bool first;

	first = true;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
		}
		else {
			s.append (delimiter);
		}
		s.append (*i1);
		++i1;
	}
	return (s);
}

StdString StringList::join (const char *delimiter) const {
	return (join (StdString (delimiter)));
}

StdString StringList::loopNext (StringList::const_iterator *pos) const {
	StdString s;

	if (*pos == cend ()) {
		*pos = cbegin ();
	}
	if (*pos != cend ()) {
		s.assign (**pos);
		++(*pos);
	}
	return (s);
}
