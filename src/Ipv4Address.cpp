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
#include "Ipv4Address.h"

Ipv4Address::Ipv4Address ()
: isValid (false)
, netmaskLength (-1)
{
	memset (octets, 0, sizeof (octets));
	memset (netmaskOctets, 0, sizeof (netmaskOctets));
}
Ipv4Address::~Ipv4Address () {
}

StdString Ipv4Address::toString () const {
	if (! isValid) {
		return (StdString ());
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", octets[0] & 0xFF, octets[1] & 0xFF, octets[2] & 0xFF, octets[3] & 0xFF));
}

bool Ipv4Address::parse (const StdString &text) {
	char *ptr, *end, *startpos, c;
	StdString s;
	int i;
	bool result;

	result = false;
	ptr = (char *) text.c_str ();
	end = ptr + text.length ();
	startpos = NULL;
	i = 0;
	while (ptr < end) {
		c = *ptr;
		if (! startpos) {
			if (! isdigit (c)) {
				break;
			}
			startpos = ptr;
		}
		else {
			if (c == '.') {
				s.assign (startpos, ptr - startpos);
				if (! s.parseInt (&octets[i])) {
					break;
				}
				startpos = NULL;
				if ((octets[i] < 0) || (octets[i] > 255)) {
					break;
				}
				++i;
				if (i >= 4) {
					break;
				}
			}
			else if (! isdigit (c)) {
				break;
			}
		}
		++ptr;
	}

	if (startpos && (i == 3)) {
		s.assign (startpos);
		if (s.parseInt (&octets[i])) {
			if ((octets[i] >= 0) && (octets[i] <= 255)) {
				result = true;
			}
		}
	}

	isValid = result;
	return (result);
}

bool Ipv4Address::parse (const char *text) {
	return (parse (StdString (text)));
}

void Ipv4Address::setAddress (uint32_t address) {
	octets[0] = (int) ((address >> 24) & 0xFF);
	octets[1] = (int) ((address >> 16) & 0xFF);
	octets[2] = (int) ((address >> 8) & 0xFF);
	octets[3] = (int) (address & 0xFF);
	isValid = true;
}

void Ipv4Address::setNetmask (uint32_t netmask) {
	int len, i;
	uint32_t mask;

	len = 0;
	mask = 0x80000000;
	while (true) {
		if (! (mask & netmask)) {
			break;
		}
		++len;
		if (mask == 0x00000001) {
			break;
		}
		mask >>= 1;
	}

	netmaskLength = len;
	memset (netmaskOctets, 0, sizeof (netmaskOctets));
	mask = 0x80;
	i = 0;
	while (len > 0) {
		if (i >= 4) {
			break;
		}
		netmaskOctets[i] |= mask;
		if (mask == 0x01) {
			mask = 0x80;
			++i;
		}
		else {
			mask >>= 1;
		}
		--len;
	}
}

StdString Ipv4Address::getNetmask () const {
	if ((! isValid) || (netmaskLength < 0)) {
		return (StdString ());
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", netmaskOctets[0] & 0xFF, netmaskOctets[1] & 0xFF, netmaskOctets[2] & 0xFF, netmaskOctets[3] & 0xFF));
}

StdString Ipv4Address::getBroadcastAddress () const {
	int result[4], i;

	if ((! isValid) || (netmaskLength < 0)) {
		return (StdString ());
	}
	for (i = 0; i < 4; ++i) {
		result[i] = octets[i];
		result[i] &= netmaskOctets[i];
		result[i] |= ~(netmaskOctets[i]);
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", result[0] & 0xFF, result[1] & 0xFF, result[2] & 0xFF, result[3] & 0xFF));
}

bool Ipv4Address::parseNetmask (const StdString &text) {
	Ipv4Address mask;
	uint32_t netmask;

	mask.parse (text);
	if (! mask.isValid) {
		return (false);
	}
	netmask = 0;
	netmask |= (mask.octets[0] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[1] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[2] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[3] & 0xFF);
	setNetmask (netmask);
	return (true);
}

bool Ipv4Address::parseNetmask (const char *text) {
	return (parseNetmask (StdString (text)));
}

bool Ipv4Address::isLocalhost () const {
	if (! isValid) {
		return (false);
	}
	return ((octets[0] == 127) && (octets[1] == 0) && (octets[2] == 0) && (octets[3] == 1));
}
