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
// Object that holds a growable data buffer
#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

class Buffer {
public:
	Buffer ();
	virtual ~Buffer ();

	static constexpr const int defaultSizeIncrement = 1024;
	static constexpr const int maxStringWriteSize = (256 * 1024);

	// Read-only data members
	uint8_t *data;
	int length;
	int readPosition;
	int readLength;

	// Return a newly created Buffer object that has been populated with a copy of this buffer's data
	Buffer *copy () const;

	// Free the buffer's underlying memory and reset its size to zero
	void reset ();

	// Return a boolean value indicating if the buffer is empty
	bool empty () const;

	// Add data to the buffer and return a Result value
	OpResult add (uint8_t *dataPtr, int dataLength);
	OpResult add (const char *str);

	// Increase the buffer's allocated length by expandSize bytes and return a Result value
	OpResult expand (int expandSize);

	// Truncate the buffer's data length to the provided value, which must be less than the buffer's current length
	void truncate (int dataLength);

	// Advance the buffer read position
	void advanceRead (int advanceSize);

	// Find the first instance of charValue in the buffer and return its position index, or -1 if the character value wasn't found
	int find (char charValue) const;

	// Reduce buffer size to discard data before the read position
	OpResult compact ();

	// Append the provided value to the buffer
	void write (int value);
	void write (int64_t value);
	void write (bool value);
	void write (float value);
	void write (double value);
	void write (const StdString &value);

	// Read a value from the buffer, store it in the provided pointer, and return a boolean value indicating if the read was successful
	bool read (int *value);
	bool read (int64_t *value);
	bool read (bool *value);
	bool read (float *value);
	bool read (double *value);
	bool read (StdString *value);

protected:
	int size;
	int sizeIncrement;
};
#endif
