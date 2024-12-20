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

Buffer::Buffer ()
: data (NULL)
, length (0)
, readPosition (0)
, readLength (0)
, size (0)
, sizeIncrement (Buffer::defaultSizeIncrement)
{
}
Buffer::~Buffer () {
	if (data) {
		free (data);
		data = NULL;
	}
}

Buffer *Buffer::copy () const {
	Buffer *buffer;

	buffer = new Buffer ();
	if (! empty ()) {
		buffer->add (data, length);
	}
	return (buffer);
}

void Buffer::reset () {
	if (data) {
		free (data);
		data = NULL;
	}
	length = 0;
	readPosition = 0;
	readLength = 0;
	size = 0;
}

bool Buffer::empty () const {
	if ((! data) || (length <= 0) || (readPosition >= length)) {
		return (true);
	}
	return (false);
}

OpResult Buffer::add (uint8_t *dataPtr, int dataLength) {
	int sz, diff, blocks, sz2;

	if (dataLength <= 0) {
		return (OpResult::Success);
	}
	sz = length + dataLength;
	diff = sz - size;
	if (diff > 0) {
		blocks = (diff / sizeIncrement);
		if (diff % sizeIncrement) {
			++blocks;
		}

		sz2 = size + (blocks * sizeIncrement);
		data = (uint8_t *) realloc (data, sz2);
		if (! data) {
			return (OpResult::OutOfMemoryError);
		}
		size = sz2;
	}

	memcpy (data + length, dataPtr, dataLength);
	length += dataLength;
	readLength += dataLength;
	return (OpResult::Success);
}

OpResult Buffer::add (const char *str) {
	return (add ((uint8_t *) str, (int) strlen (str)));
}

OpResult Buffer::expand (int expandSize) {
	int sz, diff, blocks, sz2;

	if (expandSize <= 0) {
		return (OpResult::Success);
	}
	sz = length + expandSize;
	diff = sz - size;
	if (diff > 0) {
		blocks = (diff / sizeIncrement);
		if (diff % sizeIncrement) {
			++blocks;
		}

		sz2 = size + (blocks * sizeIncrement);
		data = (uint8_t *) realloc (data, sz2);
		if (! data) {
			return (OpResult::OutOfMemoryError);
		}
		size = sz2;
	}

	memset (data + length, 0, expandSize);
	length += expandSize;
	readLength += expandSize;
	return (OpResult::Success);
}

void Buffer::truncate (int dataLength) {
	if ((dataLength < 0) || (dataLength >= readLength)) {
		return;
	}
	length = readPosition + dataLength;
	readLength = length - readPosition;
}

void Buffer::advanceRead (int advanceSize) {
	if (advanceSize <= 0) {
		return;
	}
	readPosition += advanceSize;
	if (readPosition > length) {
		readPosition = length;
	}
	readLength = length - readPosition;
}

int Buffer::find (char charValue) const {
	char *pos;

	if (empty ()) {
		return (-1);
	}
	pos = (char *) memchr (data + readPosition, charValue, readLength);
	if (! pos) {
		return (-1);
	}
	return (((uint8_t *) pos) - (data + readPosition));
}

OpResult Buffer::compact () {
	int endlen, blocks, sz;

	endlen = length - readPosition;
	blocks = (endlen / sizeIncrement);
	if (endlen % sizeIncrement) {
		++blocks;
	}
	if (blocks < 1) {
		blocks = 1;
	}
	sz = blocks * sizeIncrement;

	if (readPosition > 0) {
		memmove (data, data + readPosition, endlen);
	}
	data = (uint8_t *) realloc (data, sz);
	if (! data) {
		return (OpResult::OutOfMemoryError);
	}
	size = sz;
	readPosition = 0;
	length = endlen;
	readLength = endlen;
	return (OpResult::Success);
}

void Buffer::write (int value) {
	int32_t n;
	uint8_t b[4];

	n = (int32_t) value;
	b[3] = uint8_t (n & 0xFF);
	n >>= 8;
	b[2] = uint8_t (n & 0xFF);
	n >>= 8;
	b[1] = uint8_t (n & 0xFF);
	n >>= 8;
	b[0] = uint8_t (n & 0xFF);
	add (b, 4);
}

bool Buffer::read (int *value) {
	int32_t n;
	uint8_t *b;

	if ((! data) || (readLength < 4)) {
		return (false);
	}
	b = data + readPosition;
	n = 0;
	n |= b[0];
	n <<= 8;
	n |= b[1];
	n <<= 8;
	n |= b[2];
	n <<= 8;
	n |= b[3];
	advanceRead (4);

	if (value) {
		*value = (int) n;
	}
	return (true);
}

void Buffer::write (int64_t value) {
	int64_t n;
	uint8_t b[8];

	n = value;
	b[7] = uint8_t (n & 0xFF);
	n >>= 8;
	b[6] = uint8_t (n & 0xFF);
	n >>= 8;
	b[5] = uint8_t (n & 0xFF);
	n >>= 8;
	b[4] = uint8_t (n & 0xFF);
	n >>= 8;
	b[3] = uint8_t (n & 0xFF);
	n >>= 8;
	b[2] = uint8_t (n & 0xFF);
	n >>= 8;
	b[1] = uint8_t (n & 0xFF);
	n >>= 8;
	b[0] = uint8_t (n & 0xFF);
	add (b, 8);
}

bool Buffer::read (int64_t *value) {
	int64_t n;
	uint8_t *b;

	if ((! data) || (readLength < 8)) {
		return (false);
	}
	b = data + readPosition;
	n = 0;
	n |= b[0];
	n <<= 8;
	n |= b[1];
	n <<= 8;
	n |= b[2];
	n <<= 8;
	n |= b[3];
	n <<= 8;
	n |= b[4];
	n <<= 8;
	n |= b[5];
	n <<= 8;
	n |= b[6];
	n <<= 8;
	n |= b[7];
	advanceRead (8);

	if (value) {
		*value = n;
	}
	return (true);
}

void Buffer::write (bool value) {
	uint8_t n;

	n = value ? 1 : 0;
	add (&n, 1);
}

bool Buffer::read (bool *value) {
	bool n;

	if ((! data) || (readLength < 1)) {
		return (false);
	}
	n = *(data + readPosition) ? true : false;
	advanceRead (1);

	if (value) {
		*value = n;
	}
	return (true);
}

void Buffer::write (float value) {
	write (StdString::createSprintf ("%.10f", value));
}

bool Buffer::read (float *value) {
	StdString s;
	float n;

	if (! read (&s)) {
		return (false);
	}
	if (! s.parseFloat (&n)) {
		return (false);
	}
	if (value) {
		*value = n;
	}
	return (true);
}

void Buffer::write (double value) {
	write (StdString::createSprintf ("%.10f", value));
}

bool Buffer::read (double *value) {
	StdString s;
	double n;

	if (! read (&s)) {
		return (false);
	}
	if (! s.parseFloat (&n)) {
		return (false);
	}
	if (value) {
		*value = n;
	}
	return (true);
}

void Buffer::write (const StdString &value) {
	int len;

	len = (int) value.length ();
	if (len > Buffer::maxStringWriteSize) {
		len = Buffer::maxStringWriteSize;
	}
	write (len);
	if (len > 0) {
		add ((uint8_t *) value.c_str (), len);
	}
}

bool Buffer::read (StdString *value) {
	int len;

	if (! read (&len)) {
		return (false);
	}
	if ((len < 0) || (len > Buffer::maxStringWriteSize)) {
		return (false);
	}
	if (len == 0) {
		if (value) {
			value->assign ("");
		}
	}
	else if (len > 0) {
		if ((! data) || (readLength < len)) {
			return (false);
		}
		if (value) {
			value->assign ((char *) (data + readPosition), len);
			advanceRead (len);
		}
	}
	return (true);
}
