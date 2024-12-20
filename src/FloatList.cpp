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
#include "FloatList.h"

FloatList::FloatList ()
: std::vector<float> ()
{
}
FloatList::FloatList (const FloatList &copySource)
: FloatList ()
{
	append (copySource);
}
FloatList::FloatList (float n)
: FloatList ()
{
	append (n);
}
FloatList::FloatList (float n1, float n2)
: FloatList ()
{
	append (n1, n2);
}
FloatList::FloatList (float n1, float n2, float n3)
: FloatList ()
{
	append (n1, n2, n3);
}
FloatList::FloatList (float n1, float n2, float n3, float n4)
: FloatList ()
{
	append (n1, n2, n3, n4);
}

FloatList::~FloatList () {
}

void FloatList::assign (float n) {
	clear ();
	append (n);
}
void FloatList::assign (float n1, float n2) {
	clear ();
	append (n1, n2);
}
void FloatList::assign (float n1, float n2, float n3) {
	clear ();
	append (n1, n2, n3);
}
void FloatList::assign (float n1, float n2, float n3, float n4) {
	clear ();
	append (n1, n2, n3, n4);
}
void FloatList::assign (const FloatList &sourceList) {
	clear ();
	append (sourceList);
}

void FloatList::append (float n) {
	push_back (n);
}
void FloatList::append (float n1, float n2) {
	push_back (n1);
	push_back (n2);
}
void FloatList::append (float n1, float n2, float n3) {
	push_back (n1);
	push_back (n2);
	push_back (n3);
}
void FloatList::append (float n1, float n2, float n3, float n4) {
	push_back (n1);
	push_back (n2);
	push_back (n3);
	push_back (n4);
}
void FloatList::append (const FloatList &sourceList) {
	FloatList::const_iterator i1, i2;

	i1 = sourceList.cbegin ();
	i2 = sourceList.cend ();
	while (i1 != i2) {
		push_back (*i1);
		++i1;
	}
}

bool FloatList::equals (const FloatList &otherList) const {
	FloatList::const_iterator i1, i2, j1, j2;

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
		if (! FLOAT_EQUALS (*i1, *j1)) {
			return (false);
		}
		++i1;
		++j1;
	}
	return (true);
}

StdString FloatList::toString () const {
	StdString s;
	FloatList::const_iterator i1, i2;
	bool first;

	first = true;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
		}
		else {
			s.append (",");
		}
		s.append (StdString::createSprintf ("%.5f", *i1));
		++i1;
	}
	return (s);
}

StdString FloatList::toJsonString () const {
	StdString s;
	FloatList::const_iterator i1, i2;
	bool first;

	s.assign ("[");
	first = true;
	i1 = cbegin ();
	i2 = cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
		}
		else {
			s.append (",");
		}
		s.append (StdString::createSprintf ("%.5f", *i1));
		++i1;
	}
	s.append ("]");
	return (s);
}

bool FloatList::parseJsonString (const StdString &jsonString) {
	int i, len, stage;
	char c;
	bool success, item, err, ended;
	float n;
	StdString s;

	clear ();
	if (jsonString.empty ()) {
		return (true);
	}
	success = false;
	err = false;
	item = false;
	ended = false;
	stage = 0;
	len = jsonString.length ();
	i = 0;
	while (i < len) {
		c = jsonString.at (i);
		switch (stage) {
			case 0: {
				if (c == '[') {
					stage = 1;
				}
				else {
					err = true;
				}
				break;
			}
			case 1: {
				if (c == ']') {
					ended = true;
				}
				else if (isdigit (c) || (c == '-') || (c == '.')) {
					item = true;
					s.append (1, c);
					stage = 2;
				}
				else if (! isspace (c)) {
					err = true;
				}
				break;
			}
			case 2: {
				if (c == ']') {
					if ((! item) || s.empty ()) {
						err = true;
					}
					else {
						if (! s.parseFloat (&n)) {
							err = true;
						}
						else {
							push_back (n);
							s.assign ("");
							item = false;
						}
					}
					ended = true;
				}
				else if (c == ',') {
					if ((! item) || s.empty ()) {
						err = true;
					}
					else {
						if (! s.parseFloat (&n)) {
							err = true;
						}
						else {
							push_back (n);
							s.assign ("");
							stage = 3;
						}
					}
				}
				else if (isspace (c)) {
					if ((! item) || s.empty ()) {
						err = true;
					}
					else {
						if (! s.parseFloat (&n)) {
							err = true;
						}
						else {
							push_back (n);
							s.assign ("");
							item = false;
							stage = 4;
						}
					}
				}
				else if (isdigit (c) || (c == '-') || (c == '.')) {
					s.append (1, c);
				}
				else {
					err = true;
				}
				break;
			}
			case 3: {
				if (isdigit (c) || (c == '-') || (c == '.')) {
					item = true;
					s.append (1, c);
					stage = 2;
				}
				else if (! isspace (c)) {
					err = true;
				}
				break;
			}
			case 4: {
				if (c == ']') {
					ended = true;
				}
				else if (! isspace (c)) {
					err = true;
				}
				break;
			}
		}
		if (err || ended) {
			break;
		}
		++i;
	}

	if (! err) {
		if (item || (! ended)) {
			err = true;
		}
	}
	if (err) {
		clear ();
	}
	else {
		success = true;
	}
	return (success);
}
