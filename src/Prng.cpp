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
#include <stdint.h>
#include "Prng.h"

Prng *Prng::instance = NULL;
const int Prng::randMax = RAND_MAX;
constexpr const char *randomStringCharset = "abcdefghijklmnopqrstuvwxyz0123456789";
constexpr const int randomStringCharsetLength = 36;
constexpr const char *uuidStringCharset = "0123456789abcdef";
constexpr const int uuidCharsetLength = 16;

// Return a uint32_t resulting from performing a left circular shift on the provided value
static uint32_t lshift (uint32_t value, int shift);

// Return a uint32_t resulting from performing a right circular shift on the provided value
static uint32_t rshift (uint32_t value, int shift);

Prng::Prng ()
: sequenceIndex (0)
{
	memset (&MT, 0, sizeof (MT));
}
Prng::~Prng () {
}

void Prng::createInstance () {
	if (! Prng::instance) {
		Prng::instance = new Prng ();
	}
}
void Prng::freeInstance () {
	if (Prng::instance) {
		delete (Prng::instance);
		Prng::instance = NULL;
	}
}

void Prng::seed (uint32_t seedValue) {
	int i;
	uint32_t *a;
	uint64_t val;

	a = MT;
	a[0] = seedValue;
	for (i = 1; i < Prng::mersenneTwisterN; ++i) {
		val = 0x6C078965;
		val *= (a[i - 1] ^ rshift (a[i - 1], 30));
		val += i;
		a[i] = (uint32_t) (val & 0xFFFFFFFF);
	}
}

int Prng::getRandomNumber (int i1, int i2) {
	int64_t diff, i;

	diff = (int64_t) i2;
	diff -= i1;
	diff += 1;
	i = i1;
	if (diff > 1) {
		i += extract () % diff;
	}
	return ((int) i);
}

int64_t Prng::getRandomNumber (int64_t i1, int64_t i2) {
	int64_t diff, i;
	uint64_t n;

	diff = i2;
	diff -= i1;
	diff += 1;
	i = i1;
	if (diff > 1) {
		n = (uint64_t) extract ();
		n <<= 32;
		n |= (uint64_t) extract ();
		i += llabs ((long long int) n) % diff;
	}
	return (i);
}

float Prng::getRandomNumber (float f1, float f2) {
	float diff, fraction;

	diff = f2 - f1;
	if (diff < CONFIG_FLOAT_EPSILON) {
		return (f1);
	}
	fraction = (float) getRandomNumber ((int) 0, Prng::randMax - 1);
	fraction /= (float) (Prng::randMax - 1);
	return (f1 + (diff * fraction));
}

double Prng::getRandomNumber (double d1, double d2) {
	double diff, fraction;

	diff = d2 - d1;
	if (diff < CONFIG_FLOAT_EPSILON) {
		return (d1);
	}
	fraction = (double) getRandomNumber ((int) 0, Prng::randMax - 1);
	fraction /= (double) (Prng::randMax - 1);
	return (d1 + (diff * fraction));
}

StdString Prng::getRandomString (int randomStringLength, const char *charSet, int charSetLength) {
	StdString s;
	int i;

	if ((! charSet) || (charSetLength <= 0)) {
		charSet = randomStringCharset;
		charSetLength = randomStringCharsetLength;
	}
	for (i = 0; i < randomStringLength; ++i) {
		s.append ((size_t) 1, charSet[getRandomNumber ((int) 0, charSetLength - 1)]);
	}
	return (s);
}

StdString Prng::getUuid () {
	StdString s;

	s.assign (getRandomString (8, uuidStringCharset, uuidCharsetLength));
	s.append ("-");
	s.append (getRandomString (4, uuidStringCharset, uuidCharsetLength));
	s.append ("-");
	s.append (getRandomString (4, uuidStringCharset, uuidCharsetLength));
	s.append ("-");
	s.append (getRandomString (4, uuidStringCharset, uuidCharsetLength));
	s.append ("-");
	s.append (getRandomString (12, uuidStringCharset, uuidCharsetLength));
	return (s);
}

uint32_t Prng::extract () {
	uint32_t *a, y;
	int i;

	a = MT;
	if (sequenceIndex == 0) {
		for (i = 0; i < Prng::mersenneTwisterN; ++i) {
			y = a[i] & 0x80000000;
			y += (a[(i + 1) % Prng::mersenneTwisterN] & 0x7FFFFFFF);
			a[i] = a[(i + 397) % Prng::mersenneTwisterN] ^ (rshift (y, 1));
			if (y % 2) {
				a[i] ^= 0x9908B0DF;
			}
		}
	}
	y = a[sequenceIndex];
	y ^= rshift (y, 11);
	y ^= (lshift (y, 7)) & 0x9D2C5680;
	y ^= (lshift (y, 15)) & 0xEFC60000;
	y ^= rshift (y, 18);

	++sequenceIndex;
	if (sequenceIndex > (Prng::mersenneTwisterN - 1)) {
		sequenceIndex = 0;
	}
	return (y);
}

uint32_t lshift (uint32_t value, int shift) {
	shift &= 0x1F;
	if (! shift) {
		return (value);
	}
	return ((value << shift) | (value >> (32 - shift)));
}

uint32_t rshift (uint32_t value, int shift) {
	shift &= 0x1F;
	if (! shift) {
		return (value);
	}
	return ((value >> shift) | (value << (32 - shift)));
}

void Prng::writeState (uint8_t *state) {
	uint8_t *d;
	int n;

	d = state;
	memcpy (d, MT, sizeof (MT));
	d += sizeof (MT);

	n = sequenceIndex;
	*(d + 3) = (uint8_t) (n & 0xFF);
	n >>= 8;
	*(d + 2) = (uint8_t) (n & 0xFF);
	n >>= 8;
	*(d + 1) = (uint8_t) (n & 0xFF);
	n >>= 8;
	*d = (uint8_t) (n & 0xFF);
}

void Prng::readState (uint8_t *state) {
	uint8_t *d;
	int n;

	d = state;
	memcpy (MT, d, sizeof (MT));
	d += sizeof (MT);

	n = 0;
	n |= *d;
	++d;
	n <<= 8;
	n |= *d;
	++d;
	n <<= 8;
	n |= *d;
	++d;
	n <<= 8;
	n |= *d;
	sequenceIndex = n;
}
