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
#include "OsUtil.h"
#include "Resource.h"
#include "UiText.h"

UiText *UiText::instance = NULL;
const StdString UiText::defaultLanguage = StdString ("en");

UiText::UiText () {
}
UiText::~UiText () {
}

void UiText::createInstance () {
	if (! UiText::instance) {
		UiText::instance = new UiText ();
	}
}
void UiText::freeInstance () {
	if (UiText::instance) {
		delete (UiText::instance);
		UiText::instance = NULL;
	}
}

OpResult UiText::load (const StdString &language) {
	Buffer *buffer;
	StdString path, text, s;
	size_t curpos, pos1, pos2;

	path.sprintf ("text/%s.txt", language.c_str ());
	buffer = Resource::instance->loadFile (path);
	if ((! buffer) && (! language.equals (UiText::defaultLanguage))) {
		path.sprintf ("text/%s.txt", UiText::defaultLanguage.c_str ());
		buffer = Resource::instance->loadFile (path);
	}
	if (! buffer) {
		return (OpResult::FileOpenFailedError);
	}
	text.assignBuffer (buffer);
	textStrings.clear ();
	curpos = 0;
	while (true) {
		pos1 = text.find (' ', curpos);
		pos2 = text.find ('\n', curpos);
		if ((pos1 == StdString::npos) || (pos2 == StdString::npos) || (pos1 >= pos2)) {
			break;
		}

		s = text.substr (pos1 + 1, pos2 - pos1 - 1);
		s.replace (StdString ("\\n"), StdString ("\n"));
		textStrings.push_back (s);
		curpos = pos2 + 1;
	}

	Resource::instance->unloadFile (path);
	return (OpResult::Success);
}

StdString UiText::getText (int stringIndex) {
	int pos;

	pos = ((int) stringIndex) - 1;
	if ((pos < 0) || (pos >= (int) textStrings.size ())) {
		return (StdString ());
	}
	return (textStrings.at (pos));
}

StdString UiText::getSdlWindowFlagsText (Uint32 flags) {
	StdString s;

	s.sprintf ("0x%x", (unsigned int) flags);
	if (flags & SDL_WINDOW_FULLSCREEN) {
		s.append ("/FULLSCREEN");
	}
	if (flags & SDL_WINDOW_OPENGL) {
		s.append ("/OPENGL");
	}
	if (flags & SDL_WINDOW_SHOWN) {
		s.append ("/SHOWN");
	}
	if (flags & SDL_WINDOW_HIDDEN) {
		s.append ("/HIDDEN");
	}
	if (flags & SDL_WINDOW_BORDERLESS) {
		s.append ("/BORDERLESS");
	}
	if (flags & SDL_WINDOW_RESIZABLE) {
		s.append ("/RESIZABLE");
	}
	if (flags & SDL_WINDOW_MINIMIZED) {
		s.append ("/MINIMIZED");
	}
	if (flags & SDL_WINDOW_MAXIMIZED) {
		s.append ("/MAXIMIZED");
	}
	if (flags & SDL_WINDOW_INPUT_GRABBED) {
		s.append ("/INPUT_GRABBED");
	}
	if (flags & SDL_WINDOW_INPUT_FOCUS) {
		s.append ("/INPUT_FOCUS");
	}
	if (flags & SDL_WINDOW_MOUSE_FOCUS) {
		s.append ("/MOUSE_FOCUS");
	}
	if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		s.append ("/FULLSCREEN_DESKTOP");
	}
	if (flags & SDL_WINDOW_FOREIGN) {
		s.append ("/FOREIGN");
	}
	if (flags & SDL_WINDOW_ALLOW_HIGHDPI) {
		s.append ("/ALLOW_HIGHDPI");
	}
	if (flags & SDL_WINDOW_MOUSE_CAPTURE) {
		s.append ("/MOUSE_CAPTURE");
	}
	if (flags & SDL_WINDOW_ALWAYS_ON_TOP) {
		s.append ("/ALWAYS_ON_TOP");
	}
	if (flags & SDL_WINDOW_SKIP_TASKBAR) {
		s.append ("/SKIP_TASKBAR");
	}
	if (flags & SDL_WINDOW_UTILITY) {
		s.append ("/UTILITY");
	}
	if (flags & SDL_WINDOW_TOOLTIP) {
		s.append ("/TOOLTIP");
	}
	if (flags & SDL_WINDOW_POPUP_MENU) {
		s.append ("/POPUP_MENU");
	}
	if (flags & SDL_WINDOW_VULKAN) {
		s.append ("/VULKAN");
	}
	if (flags & SDL_WINDOW_METAL) {
		s.append ("/METAL");
	}
	return (s);
}

StdString UiText::getSdlRendererFlagsText (Uint32 flags) {
	StdString s;

	s.sprintf ("0x%x", (unsigned int) flags);
	if (flags & SDL_RENDERER_SOFTWARE) {
		s.append ("/SOFTWARE");
	}
	if (flags & SDL_RENDERER_ACCELERATED) {
		s.append ("/ACCELERATED");
	}
	if (flags & SDL_RENDERER_PRESENTVSYNC) {
		s.append ("/PRESENTVSYNC");
	}
	if (flags & SDL_RENDERER_TARGETTEXTURE) {
		s.append ("/TARGETTEXTURE");
	}
	return (s);
}

StdString UiText::getCountText (int64_t amount, int singularStringIndex, int pluralStringIndex) {
	StdString s;

	if ((amount == 1) || (pluralStringIndex < 0)) {
		s = getText (singularStringIndex);
	}
	else {
		s = getText (pluralStringIndex);
	}
	return (StdString::createSprintf ("%lli %s", (long long int) amount, s.c_str ()));
}

StdString UiText::getCountText (int amount, int singularStringIndex, int pluralStringIndex) {
	return (getCountText ((int64_t) amount, singularStringIndex, pluralStringIndex));
}

StdString UiText::getTimestampText (int64_t timestamp) {
	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
	return (StdString::createSprintf ("%s %s", getDateText (timestamp).c_str (), getTimeText (timestamp).c_str ()));
}

StdString UiText::getTimestampNumber (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%04d%02d%02d%02d%02d%02d%03d", tv.tm_year + 1900, tv.tm_mon + 1, tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec, (int) (timestamp % 1000));
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;
	int ms;

	ms = (int) (timestamp % 1000);
	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%04d%02d%02d%02d%02d%02d%03d", stlocal.wYear, stlocal.wMonth, stlocal.wDay, stlocal.wHour, stlocal.wMinute, stlocal.wSecond, ms);
		}
	}
#endif
	return (s);
}

StdString UiText::getDateText (int64_t timestamp) {
	StdString s, month;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	switch (tv.tm_mon) {
		case 0: {
			month.assign (getText (UiTextId::Month1Abbreviation));
			break;
		}
		case 1: {
			month.assign (getText (UiTextId::Month2Abbreviation));
			break;
		}
		case 2: {
			month.assign (getText (UiTextId::Month3Abbreviation));
			break;
		}
		case 3: {
			month.assign (getText (UiTextId::Month4Abbreviation));
			break;
		}
		case 4: {
			month.assign (getText (UiTextId::Month5Abbreviation));
			break;
		}
		case 5: {
			month.assign (getText (UiTextId::Month6Abbreviation));
			break;
		}
		case 6: {
			month.assign (getText (UiTextId::Month7Abbreviation));
			break;
		}
		case 7: {
			month.assign (getText (UiTextId::Month8Abbreviation));
			break;
		}
		case 8: {
			month.assign (getText (UiTextId::Month9Abbreviation));
			break;
		}
		case 9: {
			month.assign (getText (UiTextId::Month10Abbreviation));
			break;
		}
		case 10: {
			month.assign (getText (UiTextId::Month11Abbreviation));
			break;
		}
		case 11: {
			month.assign (getText (UiTextId::Month12Abbreviation));
			break;
		}
	}
	s.sprintf ("%02d/%s/%04d", tv.tm_mday, month.c_str (), tv.tm_year + 1900);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			switch (stlocal.wMonth) {
				case 1: {
					month.assign (getText (UiTextId::Month1Abbreviation));
					break;
				}
				case 2: {
					month.assign (getText (UiTextId::Month2Abbreviation));
					break;
				}
				case 3: {
					month.assign (getText (UiTextId::Month3Abbreviation));
					break;
				}
				case 4: {
					month.assign (getText (UiTextId::Month4Abbreviation));
					break;
				}
				case 5: {
					month.assign (getText (UiTextId::Month5Abbreviation));
					break;
				}
				case 6: {
					month.assign (getText (UiTextId::Month6Abbreviation));
					break;
				}
				case 7: {
					month.assign (getText (UiTextId::Month7Abbreviation));
					break;
				}
				case 8: {
					month.assign (getText (UiTextId::Month8Abbreviation));
					break;
				}
				case 9: {
					month.assign (getText (UiTextId::Month9Abbreviation));
					break;
				}
				case 10: {
					month.assign (getText (UiTextId::Month10Abbreviation));
					break;
				}
				case 11: {
					month.assign (getText (UiTextId::Month11Abbreviation));
					break;
				}
				case 12: {
					month.assign (getText (UiTextId::Month12Abbreviation));
					break;
				}
			}
			s.sprintf ("%02d/%s/%04d", stlocal.wDay, month.c_str (), stlocal.wYear);
		}
	}
#endif
	return (s);
}

StdString UiText::getTimeText (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d:%02d:%02d", tv.tm_hour, tv.tm_min, tv.tm_sec);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;
	DWORD result;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%02d:%02d:%02d", stlocal.wHour, stlocal.wMinute, stlocal.wSecond);
		}
	}
#endif
	return (s);
}

StdString UiText::getTimespanText (int64_t duration, UiText::TimespanUnit minUnitType, bool hideMilliseconds) {
	StdString s;
	char separator[2];
	int days, hours, minutes, seconds, ms, mintype;

	days = (int) (duration / (86400 * 1000));
	duration %= (86400 * 1000);
	hours = (int) (duration / (3600 * 1000));
	duration %= (3600 * 1000);
	minutes = (int) (duration / (60 * 1000));
	duration %= (60 * 1000);
	seconds = (int) (duration / 1000);
	ms = (int) (duration % 1000);

	mintype = UiText::MillisecondsUnit;
	if (seconds > 0) {
		mintype = UiText::SecondsUnit;
	}
	if (minutes > 0) {
		mintype = UiText::MinutesUnit;
	}
	if (hours > 0) {
		mintype = UiText::HoursUnit;
	}
	if (days > 0) {
		mintype = UiText::DaysUnit;
	}
	if (mintype < minUnitType) {
		mintype = minUnitType;
	}

	separator[0] = '\0';
	separator[1] = '\0';
	if (mintype >= UiText::DaysUnit) {
		s.appendSprintf ("%id", days);
	}
	if (mintype >= UiText::HoursUnit) {
		s.appendSprintf ("%02i", hours);
	}
	if (mintype >= UiText::MinutesUnit) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, minutes);
	}
	if (mintype >= UiText::SecondsUnit) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, seconds);
	}
	if ((! hideMilliseconds) && (mintype >= UiText::MillisecondsUnit)) {
		s.appendSprintf (".%03i", ms);
	}
	if (s.empty ()) {
		s.assign ("0");
	}
	return (s);
}

UiText::TimespanUnit UiText::getMinTimespanUnit (int64_t duration) {
	if (duration >= (72 * 3600 * 1000)) {
		return (UiText::DaysUnit);
	}
	if (duration >= (3600 * 1000)) {
		return (UiText::HoursUnit);
	}
	if (duration >= (60 * 1000)) {
		return (UiText::MinutesUnit);
	}
	return (UiText::SecondsUnit);
}

StdString UiText::getDurationText (int64_t duration) {
	StdString result;
	int64_t t;
	int unit, d, h, m, s;

	unit = getMinTimespanUnit (duration);
	t = duration;
	t /= 1000;
	if (unit >= UiText::DaysUnit) {
		t /= 3600;
		h = (int) t;
		d = h / 24;
		h %= 24;
		result.sprintf ("%id", d);
		if (h > 0) {
			result.appendSprintf ("%ih", h);
		}
		return (result);
	}
	if (unit >= UiText::HoursUnit) {
		t /= 60;
		h = (int) (t / 60);
		t %= 60;
		m = (int) t;
		result.sprintf ("%ih", h);
		if (m > 0) {
			result.appendSprintf ("%im", m);
		}
		return (result);
	}
	if (unit >= UiText::MinutesUnit) {
		m = (int) (t / 60);
		s = (int) (t % 60);
		result.sprintf ("%im", m);
		if (s > 0) {
			result.appendSprintf ("%is", s);
		}
		return (result);
	}
	return (StdString::createSprintf ("%is", (int) t));
}

StdString UiText::getByteCountText (int64_t bytes) {
	double n;

	if (bytes <= 0) {
		return (StdString ("0B"));
	}
	if (bytes >= 1099511627776L) {
		n = (double) bytes;
		n /= (double) 1099511627776L;
		return (StdString::createSprintf ("%.2fTB", n));
	}
	if (bytes >= 1073741824L) {
		n = (double) bytes;
		n /= (double) 1073741824L;
		return (StdString::createSprintf ("%.2fGB", n));
	}
	if (bytes >= 1048576) {
		n = (double) bytes;
		n /= (double) 1048576;
		return (StdString::createSprintf ("%.2fMB", n));
	}
	if (bytes >= 1024) {
		n = (double) bytes;
		n /= (double) 1024;
		return (StdString::createSprintf ("%ikB", (int) n));
	}
	n = (double) bytes;
	n /= (double) 1024;
	if (n < 0.01f) {
		n = 0.01f;
	}
	return (StdString::createSprintf ("%.2fkB", n));
}

StdString UiText::getStorageAmountText (int64_t bytesFree, int64_t bytesTotal) {
	double pct;

	if ((bytesFree > bytesTotal) || (bytesTotal <= 0)) {
		return (StdString ("0B"));
	}
	pct = (double) bytesFree;
	pct /= (double) bytesTotal;
	pct *= 100.0f;
	if (pct < 0.0f) {
		pct = 0.0f;
	}
	return (StdString::createSprintf ("%s / %s (%i%%)", getByteCountText (bytesFree).c_str (), getByteCountText (bytesTotal).c_str (), (int) pct));
}

StdString UiText::getRootPathText () {
	StdString s;

#if PLATFORM_WINDOWS
	s = getText (UiTextId::WindowsRootPathName);
#endif
	if (s.empty ()) {
		s = getText (UiTextId::UnixRootPathName);
	}
	return (s);
}
