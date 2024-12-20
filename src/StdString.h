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
// String class (extends std::string)
#ifndef STD_STRING_H
#define STD_STRING_H

#include <string>

class Buffer;

class StdString : public std::string {
public:
	// Default constructor that creates an empty string
	StdString ();

	// Constructor that initializes the string to the provided value
	StdString (const char *s);
	StdString (const char *s, const int sLength);
	StdString (const std::string &s);
	StdString (const Buffer &buffer);
	StdString (const Buffer &buffer, int assignLength);
	StdString (Buffer *buffer);
	StdString (Buffer *buffer, int assignLength);

	virtual ~StdString ();

	static constexpr const int maxSprintfLength = (64 * 1024); // bytes

	// Set the string's content using a format string. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void sprintf (const char *str, ...) __attribute__((format(printf, 2, 3)));

	// Set the string's content using a format string and an arg list. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void vsprintf (const char *str, va_list ap);

	// Append to the string's content using a format string. If the final length of the string to append is greater than MaxSprintfLength, the content is truncated to fit.
	void appendSprintf (const char *str, ...) __attribute__((format(printf, 2, 3)));

	// Append to the string's content using a format string and an arg list. If the final length is greater than MaxSprintfLength, the content is truncated to fit.
	void appendVsprintf (const char *str, va_list ap);

	// Return a boolean value indicating if the string is equal to the provided value
	bool equals (const StdString &value) const;
	bool equals (const char *value) const;
	bool equals (size_t pos, size_t len, const StdString &value) const;
	bool equals (size_t pos, size_t len, const char *value) const;
	bool equals (size_t pos, size_t len, const StdString &value, size_t subpos, size_t sublen) const;

	// Return a boolean value indicating if the string starts with the provided value
	bool startsWith (const StdString &value) const;
	bool startsWith (const char *value) const;

	// Return a boolean value indicating if the string ends with the provided value
	bool endsWith (const StdString &value) const;
	bool endsWith (const char *value) const;

	// Return a boolean value indicating if the string contains the provided value
	bool contains (const StdString &value) const;
	bool contains (const char *value) const;

	// Change all uppercase characters in the string's content to their matching lowercase values
	void lowercase ();

	// Return the string resulting from a lowercase operation
	StdString lowercased () const;

	// Change all lowercase characters in the string's content to their matching uppercase values
	void uppercase ();

	// Return the string resulting from an uppercase operation
	StdString uppercased () const;

	// If the first character of the string's content is a lowercase letter, change it to the matching uppercase value
	void capitalize ();

	// Return the string resulting from a capitalize operation
	StdString capitalized () const;

	// Remove leading and trailing space characters from the string
	void trim ();

	// Return the string resulting from a trim operation
	StdString trimmed () const;

	// Reduce the string's length to the specified value, appending a truncation suffix if needed
	void truncate (int maxLength, const StdString &suffix = StdString ("..."));

	// Return the string resulting from a truncate operation
	StdString truncated (int maxLength, const StdString &suffix = StdString ("...")) const;

	// Remove all instances of oldText and replace them with newText
	void replace (const StdString &oldText, const StdString &newText);

	// Replace the portion of the string that starts at pos and spans len
	void replace (size_t pos, size_t len, const StdString &str);
	void replace (size_t pos, size_t len, size_t n, char c);

	// Return the string resulting from a replace operation
	StdString replaced (const StdString &oldText, const StdString &newText) const;
	StdString replaced (size_t pos, size_t len, const StdString &str) const;
	StdString replaced (size_t pos, size_t len, size_t n, char c) const;

	// Remove all characters from the string except those appearing in allowedCharacters
	void filter (const StdString &allowedCharacters);

	// Return the string resulting from a filter operation
	StdString filtered (const StdString &allowedCharacters) const;

	// URL decode the string's content and assign it to the resulting value. Returns a boolean value indicating if the decode succeeded.
	bool urlDecode ();

	// Return the string resulting from a urlDecode operation, or an empty string if the URL decode failed
	StdString urlDecoded () const;

	// URL encode the string's content and assign it to the resulting value
	void urlEncode ();

	// Return the string resulting from a urlEncode operation
	StdString urlEncoded () const;

	// Base64 encode the provided data and assign the string to the resulting value
	void assignBase64 (const unsigned char *data, int dataLength);

	// Base64 encode the string's content and assign it to the resulting value
	void base64Encode ();

	// Return the string resulting from a base64Encode operation
	StdString base64Encoded () const;

	// Escape the string's content for use as a JSON string value and assign it to the resulting value
	void jsonEscape ();

	// Return the string resulting from a jsonEscape operation
	StdString jsonEscaped () const;

	// Replace the string's content as needed for use as an identifier and assign it to the resulting value
	void idTranslate ();

	// Return the string resulting from an idTranslate operation
	StdString idTranslated () const;

	// Return a newly created Buffer object containing the string's value
	Buffer *createBuffer ();

	// Assign the string's value from data held in a Buffer object
	void assignBuffer (Buffer *buffer);
	void assignBuffer (Buffer *buffer, int assignLength);
	void assignBuffer (const Buffer &buffer);
	void assignBuffer (const Buffer &buffer, int assignLength);

	// Assign the string's value to a set of hex numbers, as read from the provided data
	void assignHex (const unsigned char *hexData, int hexDataLength);

	// Split the string by the specified delimiter and store the resulting parts into the provided list, clearing the list before doing so
	void split (const char *delimiter, std::list<StdString> *destList) const;
	void split (const StdString &delimiter, std::list<StdString> *destList) const;

	// Overwrite all characters in the string and clear its contents
	void wipe ();

	// Parse the string's content as an integer and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseInt (int *value) const;
	bool parseInt (int64_t *value) const;

	// Parse the string's content as an integer and return the resulting int value, or defaultValue if the parse failed
	int parsedInt (int defaultValue) const;
	int64_t parsedInt (int64_t defaultValue) const;

	// Parse the string's content as a hex integer and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseHex (int *value) const;
	bool parseHex (int64_t *value) const;

	// Parse the string's content as a hex integer and return the resulting int value, or defaultValue if the parse failed
	int parsedHex (int defaultValue) const;
	int64_t parsedHex (int64_t defaultValue) const;

	// Parse the string's content as a float and store its value in the provided pointer. Returns a boolean value indicating if the parse was successful.
	bool parseFloat (float *value) const;
	bool parseFloat (double *value) const;

	// Parse the string's content as a float and return the resulting float or double value, or defaultValue if the parse failed
	float parsedFloat (float defaultValue) const;
	double parsedFloat (double defaultValue) const;

	// Parse the string's content as a network address and store its component values in the provided pointers. Returns a boolean value indicating if the parse was successful. If no port value is found in the address, assign the specified default port value to portValue.
	bool parseAddress (StdString *hostnameValue = NULL, int *portValue = NULL, int defaultPortValue = 0) const;

	// Return a boolean value indicating if the string contains a UUID value
	bool isUuid () const;

	// Return a newly created string with content generated from a format string
	static StdString createSprintf (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Return a newly created string with hex number content generated from the provided data
	static StdString createHex (const unsigned char *hexData, int hexDataLength);
};
#endif
