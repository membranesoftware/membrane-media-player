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
#include <string>
#include "Buffer.h"

StdString::StdString ()
: std::string ()
{
}

StdString::StdString (const char *s)
: std::string (s)
{
}

StdString::StdString (const char *s, const int sLength)
: std::string (s, sLength)
{
}

StdString::StdString (const std::string &s)
: std::string (s)
{
}

StdString::StdString (const Buffer &buffer)
: std::string ()
{
	assignBuffer (buffer);
}

StdString::StdString (const Buffer &buffer, int assignLength)
: std::string ()
{
	assignBuffer (buffer, assignLength);
}

StdString::StdString (Buffer *buffer)
: std::string ()
{
	assignBuffer (buffer);
}

StdString::StdString (Buffer *buffer, int assignLength)
: std::string ()
{
	assignBuffer (buffer, assignLength);
}

StdString::~StdString () {
}

void StdString::sprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	assign (buf);
}

void StdString::vsprintf (const char *str, va_list ap) {
	char buf[StdString::maxSprintfLength];

	vsnprintf (buf, sizeof (buf), str, ap);
	assign (buf);
}

void StdString::appendSprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	append (buf);
}

void StdString::appendVsprintf (const char *str, va_list ap) {
	char buf[StdString::maxSprintfLength];

	vsnprintf (buf, sizeof (buf), str, ap);
	append (buf);
}

bool StdString::equals (const StdString &value) const {
	return (! compare (value));
}
bool StdString::equals (const char *value) const {
	return (! compare (value));
}
bool StdString::equals (size_t pos, size_t len, const StdString &value) const {
	return (! compare (pos, len, value));
}
bool StdString::equals (size_t pos, size_t len, const char *value) const {
	return (! compare (pos, len, value));
}
bool StdString::equals (size_t pos, size_t len, const StdString &value, size_t subpos, size_t sublen) const {
	return (! compare (pos, len, value, subpos, sublen));
}

bool StdString::startsWith (const StdString &value) const {
	if (value.size () <= 0) {
		return (true);
	}
	return (find (value) == 0);
}
bool StdString::startsWith (const char *value) const {
	return (startsWith (StdString (value)));
}

bool StdString::endsWith (const StdString &value) const {
	if (value.size () <= 0) {
		return (true);
	}
	return ((value.length () <= length ()) && (rfind (value) == (length () - value.length ())));
}
bool StdString::endsWith (const char *value) const {
	return (endsWith (StdString (value)));
}

bool StdString::contains (const StdString &value) const {
	return (find (value) != StdString::npos);
}
bool StdString::contains (const char *value) const {
	return (find (value) != StdString::npos);
}

void StdString::lowercase () {
	assign (lowercased ());
}
StdString StdString::lowercased () const {
	StdString s;
	int len;
	char c, *ptr, *end;

	len = length ();
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		s.append ((size_t) 1, tolower (c));
		++ptr;
	}
	return (s);
}

void StdString::uppercase () {
	assign (uppercased ());
}
StdString StdString::uppercased () const {
	StdString s;
	int len;
	char c, *ptr, *end;

	len = length ();
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		s.append ((size_t) 1, toupper (c));
		++ptr;
	}
	return (s);
}

void StdString::capitalize () {
	char c;

	if (length () < 1) {
		return;
	}
	c = at (0);
	if (! islower (c)) {
		return;
	}
	std::string::replace (0, 1, 1, toupper (c));
}
StdString StdString::capitalized () const {
	StdString s;

	s.assign (c_str ());
	s.capitalize ();
	return (s);
}

void StdString::trim () {
	int pos1, pos2;
	char c;

	pos1 = 0;
	pos2 = (int) length () - 1;
	if (pos2 < 0) {
		return;
	}

	while (pos2 > pos1) {
		c = at (pos2);
		if (! isspace (c)) {
			break;
		}
		--pos2;
	}
	while (pos1 < pos2) {
		c = at (pos1);
		if (! isspace (c)) {
			break;
		}
		++pos1;
	}
	if (pos1 == pos2) {
		c = at (pos1);
		if (isspace (c)) {
			assign ("");
			return;
		}
	}

	assign (substr (pos1, pos2 - pos1 + 1));
}
StdString StdString::trimmed () const {
	StdString s;

	s.assign (c_str ());
	s.trim ();
	return (s);
}

void StdString::truncate (int maxLength, const StdString &suffix) {
	int len;

	len = (int) length ();
	if (len <= maxLength) {
		return;
	}
	len = maxLength;
	len -= suffix.length ();
	if (len <= 0) {
		assign (suffix);
		return;
	}
	assign (substr (0, len));
	append (suffix);
}
StdString StdString::truncated (int maxLength, const StdString &suffix) const {
	StdString s;

	s.assign (c_str ());
	s.truncate (maxLength, suffix);
	return (s);
}

void StdString::replace (const StdString &oldText, const StdString &newText) {
	size_t curpos, pos, oldtextlen, newtextlen;

	oldtextlen = oldText.length ();
	if (oldtextlen <= 0) {
		return;
	}
	newtextlen = newText.length ();
	curpos = 0;
	while (true) {
		pos = find (oldText, curpos);
		if (pos == StdString::npos) {
			break;
		}
		std::string::replace (pos, oldtextlen, newText);
		curpos = pos + newtextlen;
	}
}
void StdString::replace (size_t pos, size_t len, const StdString &str) {
	std::string::replace (pos, len, str);
}
void StdString::replace (size_t pos, size_t len, size_t n, char c) {
	std::string::replace (pos, len, n, c);
}
StdString StdString::replaced (const StdString &oldText, const StdString &newText) const {
	StdString s;

	s.assign (c_str ());
	s.replace (oldText, newText);
	return (s);
}
StdString StdString::replaced (size_t pos, size_t len, const StdString &str) const {
	StdString s;

	s.assign (c_str ());
	s.replace (pos, len, str);
	return (s);
}
StdString StdString::replaced (size_t pos, size_t len, size_t n, char c) const {
	StdString s;

	s.assign (c_str ());
	s.replace (pos, len, n, c);
	return (s);
}

void StdString::filter (const StdString &allowedCharacters) {
	std::map<char, bool> charmap;
	int i, len;
	StdString s;
	char c;

	len = allowedCharacters.length ();
	for (i = 0; i < len; ++i) {
		charmap.insert (std::pair<char, bool> (allowedCharacters.at (i), true));
	}

	s.assign (c_str ());
	assign ("");
	len = s.length ();
	for (i = 0; i < len; ++i) {
		c = s.at (i);
		if (charmap.count (c) > 0) {
			append ((size_t) 1, c);
		}
	}
}
StdString StdString::filtered (const StdString &allowedCharacters) const {
	StdString s;

	s.assign (c_str ());
	s.filter (allowedCharacters);
	return (s);
}

bool StdString::urlDecode () {
	StdString s;
	char *d, *end, c;
	int code, codechars;
	bool result;

	result = true;
	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	code = 0;
	codechars = 0;
	while (d < end) {
		c = *d;
		if (codechars <= 0) {
			if (c == '%') {
				codechars = 1;
			}
			else {
				s.append ((size_t) 1, c);
			}
		}
		else {
			code <<= 4;
			if ((c >= '0') && (c <= '9')) {
				code |= ((c - '0') & 0x0F);
				++codechars;
			}
			else if ((c >= 'a') && (c <= 'f')) {
				code |= ((c - 'a' + 10) & 0x0F);
				++codechars;
			}
			else if ((c >= 'A') && (c <= 'F')) {
				code |= ((c - 'A' + 10) & 0x0F);
				++codechars;
			}
			else {
				result = false;
				break;
			}

			if (codechars >= 3) {
				s.append ((size_t) 1, (char) (code & 0xFF));
				codechars = 0;
			}
		}
		++d;
	}

	if (result) {
		assign (s.c_str ());
	}
	return (result);
}
StdString StdString::urlDecoded () const {
	StdString s;

	s.assign (c_str ());
	if (! s.urlDecode ()) {
		return (StdString ());
	}
	return (s);
}

void StdString::urlEncode () {
	StdString s;
	char *d, *end, c;

	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	while (d < end) {
		c = *d;
		switch (c) {
			case '!':
			case '#':
			case '$':
			case '&':
			case '\'':
			case '(':
			case ')':
			case '*':
			case '+':
			case ',':
			case '/':
			case ':':
			case ';':
			case '=':
			case '?':
			case '@':
			case '[':
			case ']':
			case '%':
			case '"':
			case '{':
			case '}':
			case ' ':
				s.appendSprintf ("%%%02X", c);
				break;
			default:
				s.append ((size_t) 1, c);
				break;
		}
		++d;
	}
	assign (s.c_str ());
}
StdString StdString::urlEncoded () const {
	StdString s;

	s.assign (c_str ());
	s.urlEncode ();
	return (s);
}

void StdString::jsonEscape () {
	StdString s;
	char *d, *end, c;

	s.assign ("");
	d = (char *) c_str ();
	end = d + length ();
	while (d < end) {
		c = *d;
		if (c == '"') {
			s.append ("\\\"");
		}
		else if (c == '\\') {
			s.append ("\\\\");
		}
		else if (c == '\n') {
			s.append ("\\n");
		}
		else if (c == '\t') {
			s.append ("\\t");
		}
		else if (c == '\b') {
			s.append ("\\b");
		}
		else if (c == '\r') {
			s.append ("\\r");
		}
		else if (c == '\f') {
			s.append ("\\f");
		}
		else if (c == '/') {
			s.append ("\\/");
		}
		else if (c <= 0x1F) {
			s.appendSprintf ("\\u%04X", (int) c);
		}
		else {
			s.append ((size_t) 1, c);
		}
		++d;
	}
	assign (s.c_str ());
}
StdString StdString::jsonEscaped () const {
	StdString s;

	s.assign (c_str ());
	s.jsonEscape ();
	return (s);
}

void StdString::idTranslate () {
	assign (idTranslated ());
}
StdString StdString::idTranslated () const {
	StdString s;
	char c, *ptr, *end;
	int len;

	len = (int) length ();
	s.reserve (len);
	ptr = (char *) c_str ();
	end = ptr + len;
	while (ptr < end) {
		c = *ptr;
		if (! isalnum (c)) {
			c = '_';
		}
		else {
			c = tolower (c);
		}
		s.append ((size_t) 1, c);
		++ptr;
	}
	return (s);
}

StdString StdString::createSprintf (const char *str, ...) {
	va_list ap;
	char buf[StdString::maxSprintfLength];

	va_start (ap, str);
	vsnprintf (buf, sizeof (buf), str, ap);
	va_end (ap);
	return (StdString (buf));
}

StdString StdString::createHex (const unsigned char *hexData, int hexDataLength) {
	StdString s;

	s.assignHex (hexData, hexDataLength);
	return (s);
}

bool StdString::parseInt (int *value) const {
	char *s, c;
	bool first;

	if (length () <= 0) {
		return (false);
	}
	first = true;
	s = (char *) c_str ();
	c = *s;
	while (c) {
		if (! isdigit (c)) {
			if ((! first) || (c != '-')) {
				return (false);
			}
		}
		first = false;
		++s;
		c = *s;
	}
	if (value) {
		*value = atoi (c_str ());
	}
	return (true);
}

bool StdString::parseInt (int64_t *value) const {
	char *s, c;
	bool first;

	if (length () <= 0) {
		return (false);
	}
	first = true;
	s = (char *) c_str ();
	c = *s;
	while (c) {
		if (! isdigit (c)) {
			if ((! first) || (c != '-')) {
				return (false);
			}
		}
		first = false;
		++s;
		c = *s;
	}
	if (value) {
		*value = atoll (c_str ());
	}
	return (true);
}

int StdString::parsedInt (int defaultValue) const {
	int n;

	if (parseInt (&n)) {
		return (n);
	}
	return (defaultValue);
}

int64_t StdString::parsedInt (int64_t defaultValue) const {
	int64_t n;

	if (parseInt (&n)) {
		return (n);
	}
	return (defaultValue);
}

bool StdString::parseHex (int *value) const {
	int i;
	char *s, c;

	if (length () <= 0) {
		return (false);
	}
	i = 0;
	s = (char *) c_str ();
	while (1) {
		c = *s;
		if (! c) {
			break;
		}
		i <<= 4;
		if ((c >= '0') && (c <= '9')) {
			i |= ((c - '0') & 0x0F);
		}
		else if ((c >= 'a') && (c <= 'f')) {
			i |= ((c - 'a' + 10) & 0x0F);
		}
		else if ((c >= 'A') && (c <= 'F')) {
			i |= ((c - 'A' + 10) & 0x0F);
		}
		else {
			return (false);
		}
		++s;
	}
	if (value) {
		*value = i;
	}
	return (true);
}

int StdString::parsedHex (int defaultValue) const {
	int n;

	if (parseHex (&n)) {
		return (n);
	}
	return (defaultValue);
}

bool StdString::parseHex (int64_t *value) const {
	int64_t i;
	char *s, c;

	if (length () <= 0) {
		return (false);
	}
	i = 0;
	s = (char *) c_str ();
	while (1) {
		c = *s;
		if (! c) {
			break;
		}
		i <<= 4;
		if ((c >= '0') && (c <= '9')) {
			i |= ((c - '0') & 0x0F);
		}
		else if ((c >= 'a') && (c <= 'f')) {
			i |= ((c - 'a' + 10) & 0x0F);
		}
		else if ((c >= 'A') && (c <= 'F')) {
			i |= ((c - 'A' + 10) & 0x0F);
		}
		else {
			return (false);
		}
		++s;
	}
	if (value) {
		*value = i;
	}
	return (true);
}

int64_t StdString::parsedHex (int64_t defaultValue) const {
	int64_t n;

	if (parseHex (&n)) {
		return (n);
	}
	return (defaultValue);
}

bool StdString::parseFloat (float *value) const {
	char *s, c;
	bool first, point;

	if (length () <= 0) {
		return (false);
	}
	first = true;
	point = false;
	s = (char *) c_str ();
	c = *s;
	while (c) {
		if (c == '-') {
			if (! first) {
				return (false);
			}
		}
		else if (c == '.') {
			if (point) {
				return (false);
			}
			point = true;
		}
		else if (! isdigit (c)) {
			return (false);
		}
		first = false;
		++s;
		c = *s;
	}
	if (value) {
		*value = strtof (c_str (), NULL);
	}
	return (true);
}

bool StdString::parseFloat (double *value) const {
	char *s, c;
	bool first, point;

	if (length () <= 0) {
		return (false);
	}
	first = true;
	point = false;
	s = (char *) c_str ();
	c = *s;
	while (c) {
		if (c == '-') {
			if (! first) {
				return (false);
			}
		}
		else if (c == '.') {
			if (point) {
				return (false);
			}
			point = true;
		}
		else if (! isdigit (c)) {
			return (false);
		}
		first = false;
		++s;
		c = *s;
	}
	if (value) {
		*value = strtod (c_str (), NULL);
	}
	return (true);
}

float StdString::parsedFloat (float defaultValue) const {
	float n;

	if (parseFloat (&n)) {
		return (n);
	}
	return (defaultValue);
}

double StdString::parsedFloat (double defaultValue) const {
	double n;

	if (parseFloat (&n)) {
		return (n);
	}
	return (defaultValue);
}

bool StdString::parseAddress (StdString *hostnameValue, int *portValue, int defaultPortValue) const {
	StdString hostname;
	int port;
	size_t pos;

	if (length () <= 0) {
		return (false);
	}
	port = defaultPortValue;
	pos = find (":");
	if (pos == StdString::npos) {
		hostname.assign (c_str ());
	}
	else {
		if (! StdString (substr (pos + 1)).parseInt (&port)) {
			return (false);
		}
		if ((port <= 0) || (port > 65535)) {
			return (false);
		}
		hostname.assign (substr (0, pos));
	}

	if (hostnameValue) {
		hostnameValue->assign (hostname);
	}
	if (portValue) {
		*portValue = port;
	}
	return (true);
}

bool StdString::isUuid () const {
	int i, len;
	char c;

	len = (int) length ();
	if (len != 36) {
		return (false);
	}
	for (i = 0; i < len; ++i) {
		c = at (i);
		if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
			if (c != '-') {
				return (false);
			}
		}
		else {
			if ((c >= '0') && (c <= '9')) {
				continue;
			}
			if ((c >= 'a') && (c <= 'f')) {
				continue;
			}
			if ((c >= 'A') && (c <= 'F')) {
				continue;
			}
			return (false);
		}
	}
	return (true);
}

Buffer *StdString::createBuffer () {
	Buffer *buffer;

	buffer = new Buffer ();
	buffer->add ((uint8_t *) c_str (), length ());
	return (buffer);
}

void StdString::assignBuffer (Buffer *buffer) {
	assignBuffer (buffer, buffer->readLength);
}
void StdString::assignBuffer (Buffer *buffer, int assignLength) {
	if ((! buffer) || buffer->empty () || (assignLength <= 0)) {
		assign ("");
		return;
	}
	assign ((char *) (buffer->data + buffer->readPosition), assignLength < buffer->readLength ? assignLength : buffer->readLength);
}
void StdString::assignBuffer (const Buffer &buffer) {
	assignBuffer (buffer, buffer.readLength);
}
void StdString::assignBuffer (const Buffer &buffer, int assignLength) {
	if (buffer.empty () || (assignLength <= 0)) {
		assign ("");
		return;
	}
	assign ((char *) (buffer.data + buffer.readPosition), assignLength < buffer.readLength ? assignLength : buffer.readLength);
}

void StdString::assignHex (const unsigned char *hexData, int hexDataLength) {
	unsigned char *d, *end;

	if ((! hexData) || (hexDataLength <= 0)) {
		return;
	}
	assign ("");
	d = (unsigned char *) hexData;
	end = d + hexDataLength;
	while (d < end) {
		appendSprintf ("%02x", *d);
		++d;
	}
}

void StdString::wipe () {
	StdString::iterator i1, i2;

	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		*i1 = ' ';
		++i1;
	}
	clear ();
}

void StdString::split (const char *delimiter, std::list<StdString> *destList) const {
	split (StdString (delimiter), destList);
}
void StdString::split (const StdString &delimiter, std::list<StdString> *destList) const {
	size_t curpos, pos, delimlen, len;

	if (! destList) {
		return;
	}
	destList->clear ();
	curpos = 0;
	delimlen = delimiter.length ();
	if (delimlen <= 0) {
		destList->push_back (StdString (c_str ()));
		return;
	}

	len = length ();
	while (true) {
		if (curpos >= len) {
			break;
		}
		pos = find (delimiter, curpos);
		if (pos == StdString::npos) {
			break;
		}
		destList->push_back (StdString (substr (curpos, pos - curpos)));
		curpos = pos + delimlen;
	}

	if (curpos <= len) {
		destList->push_back (StdString (substr (curpos)));
	}
}

static const char *base64EncodeCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void StdString::assignBase64 (const unsigned char *data, int dataLength) {
	StdString result;
	unsigned char *d, i0, i1, i2;
	char o[5];
	int dlen;

	if ((! data) || (dataLength <= 0)) {
		assign ("");
		return;
	}
	result.reserve (((dataLength * 4) / 3) + 4);
	o[4] = '\0';
	d = (unsigned char *) data;
	dlen = dataLength;
	while (dlen > 0) {
		i0 = *d;
		i1 = (dlen > 1) ? *(d + 1) : 0;
		i2 = (dlen > 2) ? *(d + 2) : 0;

		o[0] = base64EncodeCharacters[(i0 & 0xFC) >> 2];
		o[1] = base64EncodeCharacters[((i0 & 0x03) << 4) | ((i1 & 0xF0) >> 4)];
		o[2] = (dlen > 1) ? base64EncodeCharacters[((i1 & 0x0F) << 2) | ((i2 & 0xC0) >> 6)] : '=';
		o[3] = (dlen > 2) ? base64EncodeCharacters[i2 & 0x3F] : '=';
		result.append (o);

		d += 3;
		dlen -= 3;
	}
	assign (result);
}

void StdString::base64Encode () {
	StdString s;

	s.assignBase64 ((unsigned char *) c_str (), (int) length ());
	assign (s);
}
StdString StdString::base64Encoded () const {
	StdString s;

	s.assignBase64 ((unsigned char *) c_str (), (int) length ());
	return (s);
}
