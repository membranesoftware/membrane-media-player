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
// List template providing functions for repeated traversal in natural or randomized order (extends std::vector<T>)
#ifndef SEQUENCE_LIST_H
#define SEQUENCE_LIST_H

#include "Prng.h"

template<class T> class SequenceList : public std::vector<T> {
public:
	SequenceList<T> ():
		std::vector<T> (),
		nextItemIndex (-1),
		sequenceSize (0),
		prng (NULL),
		lastItemIndex (-1) { }
	~SequenceList<T> () { }

	// Read-write data members
	int nextItemIndex;

	// Remove all items from the list
	void clear () {
		std::vector<T>::clear ();
		sequenceList.clear ();
		sequenceSize = 0;
	}

	// Set the list to return items in a randomized order, as generated using the provided Prng
	void randomizeOrder (Prng *p) {
		if (p) {
			prng = p;
			sequenceList.clear ();
			sequenceSize = 0;
		}
	}

	// Set the list to return items in natural order
	void unrandomizeOrder () {
		if (prng) {
			prng = NULL;
			sequenceList.clear ();
			sequenceSize = 0;
		}
	}

	// Get the next item in the list's sequence and write it to the provided pointer. Returns a boolean value indicating if an item was found.
	bool next (T *destItem) {
		int i, count, pos, itemindex;

		count = (int) std::vector<T>::size ();
		if (count > sequenceSize) {
			for (i = sequenceSize; i < count; ++i) {
				if (prng) {
					sequenceList.insert (sequenceList.begin () + prng->getRandomNumber (0, (int) sequenceList.size ()), i);
				}
				else {
					sequenceList.push_back (i);
				}
			}
			sequenceSize = count;
		}
		else if (count < sequenceSize) {
			sequenceSize = count;
			i = 0;
			while (i < (int) sequenceList.size ()) {
				if (sequenceList.at (i) >= sequenceSize) {
					sequenceList.erase (sequenceList.begin () + i);
				}
				else {
					++i;
				}
			}
		}

		if (sequenceList.empty ()) {
			if (count <= 0) {
				return (false);
			}
			for (i = 0; i < count; ++i) {
				if (prng) {
					sequenceList.insert (sequenceList.begin () + prng->getRandomNumber (0, i), i);
				}
				else {
					sequenceList.push_back (i);
				}
			}

			if (prng && (count > 1) && (lastItemIndex >= 0)) {
				i = sequenceList.at (0);
				if (i == lastItemIndex) {
					pos = prng->getRandomNumber (1, ((int) sequenceList.size ()) - 1);
					sequenceList.at (0) = sequenceList.at (pos);
					sequenceList.at (pos) = i;
				}
			}
		}

		if ((nextItemIndex >= 0) && (nextItemIndex < count)) {
			itemindex = nextItemIndex;
			i = 0;
			while (i < (int) sequenceList.size ()) {
				if (sequenceList.at (i) == itemindex) {
					sequenceList.erase (sequenceList.begin () + i);
					break;
				}
				++i;
			}
		}
		else {
			itemindex = sequenceList.at (0);
			sequenceList.erase (sequenceList.begin ());
		}
		nextItemIndex = -1;

		if (destItem) {
			*destItem = std::vector<T>::at (itemindex);
		}
		lastItemIndex = itemindex;
		return (true);
	}

private:
	std::vector<int> sequenceList;
	int sequenceSize;
	Prng *prng;
	int lastItemIndex;
};
#endif
