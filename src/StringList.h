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
// String list class (extends std::list<StdString>)
#ifndef STRING_LIST_H
#define STRING_LIST_H
class StringList : public std::list<StdString> {
public:
	StringList ();
	StringList (const StringList &copySource);

	// Constructor append variants only add non-empty string items
	StringList (const char *s1, const char *s2 = NULL, const char *s3 = NULL, const char *s4 = NULL, const char *s5 = NULL, const char *s6 = NULL, const char *s7 = NULL, const char *s8 = NULL);
	StringList (const StdString &s1, const StdString &s2 = StdString (), const StdString &s3 = StdString (), const StdString &s4 = StdString (), const StdString &s5 = StdString (), const StdString &s6 = StdString (), const StdString &s7 = StdString (), const StdString &s8 = StdString ());

	~StringList ();

	typedef bool (*SortFunction) (const StdString &a, const StdString &b);

	// Return a string containing the items in the list
	StdString toString () const;

	// Return a JSON array string containing the items in the list
	StdString toJsonString () const;

	// Return a newly created StringList object containing copies of all items in the list
	StringList *copy () const;

	// Parse the provided JSON array string and replace the list content with the resulting items. Returns a boolean value indicating if the parse was successful.
	bool parseJsonString (const StdString &jsonString);

	// Add the provided string to the list, choosing a position that maintains sorted order
	void insertInOrder (const StdString &item);

	// Clear the list and add one or more non-empty string items
	void assign (const char *s1, const char *s2 = NULL, const char *s3 = NULL, const char *s4 = NULL, const char *s5 = NULL, const char *s6 = NULL, const char *s7 = NULL, const char *s8 = NULL);
	void assign (const StdString &s1, const StdString &s2 = StdString (), const StdString &s3 = StdString (), const StdString &s4 = StdString (), const StdString &s5 = StdString (), const StdString &s6 = StdString (), const StdString &s7 = StdString (), const StdString &s8 = StdString ());

	// Clear the list and append all items from sourceList
	void assign (const StringList &sourceList);

	// Append non-empty string items to the list
	void append (const char *s1, const char *s2 = NULL, const char *s3 = NULL, const char *s4 = NULL, const char *s5 = NULL, const char *s6 = NULL, const char *s7 = NULL, const char *s8 = NULL);
	void append (const StdString &s1, const StdString &s2 = StdString (), const StdString &s3 = StdString (), const StdString &s4 = StdString (), const StdString &s5 = StdString (), const StdString &s6 = StdString (), const StdString &s7 = StdString (), const StdString &s8 = StdString ());

	// Append all items from a source StringList object to the list
	void append (const StringList &sourceList);

	// Remove all items matching the specified value from the list
	void remove (const StdString &item);

	// Return a boolean value indicating if the list contains an item matching the specified value
	bool contains (const StdString &item) const;

	// Return the index of the specified item in the list, or -1 if the item was not found
	int indexOf (const StdString &item) const;
	int indexOf (const char *item) const;

	// Return the string at the specified list index, or an empty string if no item was found
	StdString at (int index) const;

	// Return a boolean value indicating if the list contains all items from another list, in the same order
	bool equals (const StringList &otherList) const;

	// Sort the items in the list
	void sort (StringList::SortFunction sortFunction = StringList::compareAscending);

	// Return a string composed by joining all list items with the specified delimiter
	StdString join (const StdString &delimiter = StdString ()) const;
	StdString join (const char *delimiter) const;

	// Return the string value from the list at the provided position, or an empty string if no values are available. If a string value was found, advance pos and loop it to the list start if the end has been reached.
	StdString loopNext (StringList::const_iterator *pos) const;

	static bool compareAscending (const StdString &a, const StdString &b);
	static bool compareDescending (const StdString &a, const StdString &b);
	static bool compareCaseInsensitiveAscending (const StdString &a, const StdString &b);
	static bool compareCaseInsensitiveDescending (const StdString &a, const StdString &b);
};
#endif
