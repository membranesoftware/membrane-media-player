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
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#endif
#if PLATFORM_WINDOWS
#include <time.h>
#include <windows.h>
#include <processthreadsapi.h>
#include <ShellAPI.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include "StringList.h"
#include "Buffer.h"
#include "OsUtil.h"

const char *OsUtil::monthNames[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int64_t OsUtil::getTime () {
	int64_t t;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct timeval now;

	gettimeofday (&now, NULL);
	t = ((int64_t) now.tv_sec) * 1000;
	t += (now.tv_usec / 1000);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;

	GetSystemTimeAsFileTime (&ft);
	t = ft.dwHighDateTime;
	t <<= 32;
	t |= ft.dwLowDateTime;
	t /= 10000;
	t -= 11644473600000ULL;
#endif
	return (t);
}

int64_t OsUtil::getHrtime () {
	int64_t t;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct timeval now;

	gettimeofday (&now, NULL);
	t = ((int64_t) now.tv_sec) * 1000 * 10000;
	t += (now.tv_usec * 10);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;

	GetSystemTimeAsFileTime (&ft);
	t = ft.dwHighDateTime;
	t <<= 32;
	t |= ft.dwLowDateTime;
	t -= 116444736000000000ULL;
#endif
	return (t);
}

int OsUtil::getProcessId () {
#if PLATFORM_WINDOWS
	return ((int) GetCurrentProcessId ());
#else
	return ((int) getpid ());
#endif
}

StdString OsUtil::getTimestampString (int64_t timestamp, bool isTimezoneEnabled) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
	int ms;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	ms = (int) (timestamp % 1000);
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", tv.tm_mday, OsUtil::monthNames[tv.tm_mon], tv.tm_year + 1900, tv.tm_hour, tv.tm_min, tv.tm_sec, ms);
	if (isTimezoneEnabled) {
		s.appendSprintf (" %+.2ld00", tv.tm_gmtoff / 3600);
	}
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;
	TIME_ZONE_INFORMATION tz;
	DWORD result;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", stlocal.wDay, OsUtil::monthNames[stlocal.wMonth - 1], stlocal.wYear, stlocal.wHour, stlocal.wMinute, stlocal.wSecond, stlocal.wMilliseconds);

			if (isTimezoneEnabled) {
				result = GetTimeZoneInformation (&tz);
				if (result != TIME_ZONE_ID_INVALID) {
					s.appendSprintf (" %+.2ld00", -(tz.Bias / 60));
				}
			}
		}
	}
#endif
	return (s);
}

StdString OsUtil::getJoinedPath (const char *basePath, const char *pathElement1, const char *pathElement2, const char *pathElement3, const char *pathElement4, const char *pathElement5, const char *pathElement6, const char *pathElement7) {
	return (OsUtil::getJoinedPath (StdString (basePath ? basePath : ""), StdString (pathElement1 ? pathElement1 : ""), StdString (pathElement2 ? pathElement2 : ""), StdString (pathElement3 ? pathElement3 : ""), StdString (pathElement4 ? pathElement4 : ""), StdString (pathElement5 ? pathElement5 : ""), StdString (pathElement6 ? pathElement6 : ""), StdString (pathElement7 ? pathElement7 : "")));
}

static void getJoinedPath_append (const StdString &pathElement, StdString *result) {
	StringList parts;
	StringList::const_iterator i1, i2;
	StdString delim;
	bool first;

	if (pathElement.empty ()) {
		return;
	}
#if PLATFORM_WINDOWS
	delim.assign ("\\");
#else
	delim.assign ("/");
#endif
	pathElement.split (delim, &parts);
	if (parts.empty ()) {
		return;
	}
	first = true;
	i1 = parts.cbegin ();
	i2 = parts.cend ();
	while (i1 != i2) {
		if (i1->empty ()) {
			if (first && result->empty ()) {
				result->append (delim);
			}
		}
		else {
			if ((! result->empty ()) && (! result->endsWith (delim))) {
				result->append (delim);
			}
			result->append (*i1);
		}
		first = false;
		++i1;
	}
}
StdString OsUtil::getJoinedPath (const StdString &basePath, const StdString &pathElement1, const StdString &pathElement2, const StdString &pathElement3, const StdString &pathElement4, const StdString &pathElement5, const StdString &pathElement6, const StdString &pathElement7) {
	StdString result;

	getJoinedPath_append (basePath, &result);
	getJoinedPath_append (pathElement1, &result);
	getJoinedPath_append (pathElement2, &result);
	getJoinedPath_append (pathElement3, &result);
	getJoinedPath_append (pathElement4, &result);
	getJoinedPath_append (pathElement5, &result);
	getJoinedPath_append (pathElement6, &result);
	getJoinedPath_append (pathElement7, &result);
	return (result);
}

StdString OsUtil::getPathBasename (const StdString &path) {
	StringList parts;
	StringList::const_iterator i;

	OsUtil::splitPath (path, &parts);
	if (parts.size () < 1) {
		return (StdString ());
	}
	i = parts.cend ();
	--i;
	return (*i);
}

StdString OsUtil::getPathDirname (const StdString &path) {
	StringList parts;
	StringList::const_iterator i1, i2;
	StdString dirname;
#if PLATFORM_WINDOWS
	const char *delimchar = "\\";
#else
	const char *delimchar = "/";
#endif

	OsUtil::splitPath (path, &parts);
	if (parts.size () < 2) {
		return (StdString ());
	}
	i1 = parts.cbegin ();
	i2 = parts.cend ();
	dirname.assign (*i1);
	if (dirname.empty ()) {
		dirname.assign (delimchar);
	}
	++i1;
	--i2;
	while (i1 != i2) {
		if (! i1->empty ()) {
			dirname = OsUtil::getJoinedPath (dirname, *i1);
		}
		++i1;
	}
	return (dirname);
}

StdString OsUtil::getPathExtension (const StdString &path) {
	size_t pos;

	pos = path.find_last_of ('.');
	if ((pos == StdString::npos) || (pos == (path.length () - 1))) {
		return (StdString ());
	}
	return (StdString (path.substr (pos + 1)));
}

StdString OsUtil::getAppendExtensionPath (const StdString &path, const StdString &extension) {
	StdString s;

	if (path.empty () && extension.empty ()) {
		return (StdString ());
	}
	s.assign (path);
	s.append (".");
	s.append (extension);
	return (s);
}

StdString OsUtil::getReplaceExtensionPath (const StdString &path, const StdString &extension) {
	StdString s;
	size_t pos;

	if (path.empty () && extension.empty ()) {
		return (StdString ());
	}
	s.assign (path);
	pos = s.find_last_of ('.');
	if (pos != StdString::npos) {
		s = s.substr (0, pos);
	}
	s.append (".");
	s.append (extension);
	return (s);
}

StdString OsUtil::getTrailingSeparatorPath (const StdString &path) {
	StdString delim, s;

#if PLATFORM_WINDOWS
	delim.assign ("\\");
#else
	delim.assign ("/");
#endif
	s.assign (path);
	if (! s.endsWith (delim)) {
		s.append (delim);
	}
	return (s);
}

void OsUtil::splitPath (const StdString &path, StringList *destList) {
	StdString delim;
	StringList parts;
	StringList::const_iterator i1, i2;
	bool skipempty;

#if PLATFORM_WINDOWS
	delim.assign ("\\");
#else
	delim.assign ("/");
#endif
	destList->clear ();
	path.split (delim, &parts);
	skipempty = false;
	i1 = parts.cbegin ();
	i2 = parts.cend ();
	while (i1 != i2) {
		if (i1->empty ()) {
			if (! skipempty) {
				destList->push_back (*i1);
			}
		}
		else {
			destList->push_back (*i1);
		}
		skipempty = true;
		++i1;
	}
}

bool OsUtil::isFilename (const StdString &text) {
#if PLATFORM_WINDOWS
	const char *delimchar = "\\";
#else
	const char *delimchar = "/";
#endif
	if (text.empty () || text.equals (".") || text.equals ("..") || text.contains (delimchar)) {
		return (false);
	}
	return (true);
}

StdString OsUtil::getWorkingPath () {
#if PLATFORM_LINUX || PLATFORM_MACOS
	char data[8192];

	if (! getcwd (data, sizeof (data))) {
		return (StdString ());
	}
	return (StdString (data));
#endif
#if PLATFORM_WINDOWS
	DWORD result, bufsize;
	LPTSTR buf;
	StdString val;
	std::basic_string<TCHAR> k;

	bufsize = 8192;
	buf = (LPTSTR) malloc (bufsize * sizeof (TCHAR));
	if (! buf) {
		return (StdString ());
	}
	val.assign ("");
	result = GetCurrentDirectory (bufsize, buf);
	if (result > bufsize) {
		bufsize = result;
		buf = (LPTSTR) realloc (buf, bufsize * sizeof (TCHAR));
		if (! buf) {
			return (StdString ());
		}
		result = GetCurrentDirectory (bufsize, buf);
	}
	if ((result != 0) && (result <= bufsize)) {
		k.assign (buf);
		val.assign (k);
	}

	free (buf);
	return (val);
#endif
}

StdString OsUtil::getUserHomePath () {
	StdString path;

#if PLATFORM_LINUX
	path = OsUtil::getEnvValue (OsUtil::homeEnvKey, "");
#endif
#if PLATFORM_MACOS
	path = OsUtil::getEnvValue (OsUtil::homeEnvKey, "");
#endif
#if PLATFORM_WINDOWS
	path = OsUtil::getEnvValue (OsUtil::localAppDataEnvKey, "");
#endif
	return (path);
}

StdString OsUtil::getAppDataPath () {
	StdString path;

#if PLATFORM_LINUX
	path = OsUtil::getEnvValue (OsUtil::homeEnvKey, "");
	if (! path.empty ()) {
		path = OsUtil::getJoinedPath (path.c_str (), CONFIG_APPDATA_DIRNAME);
	}
#endif
#if PLATFORM_MACOS
	path = OsUtil::getEnvValue (OsUtil::homeEnvKey, "");
	if (! path.empty ()) {
		path = OsUtil::getJoinedPath (path.c_str (), "Library", "Application Support", CONFIG_APPDATA_DIRNAME);
	}
#endif
#if PLATFORM_WINDOWS
	path = OsUtil::getEnvValue (OsUtil::localAppDataEnvKey, "");
	if (! path.empty ()) {
		path = OsUtil::getJoinedPath (path.c_str (), CONFIG_APPDATA_DIRNAME);
	}
#endif
	return (path);
}

OpResult OsUtil::createDirectory (const StdString &path) {
	OpResult result;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;
	int cresult;

	cresult = stat (path.c_str (), &st);
	if (cresult != 0) {
		if (errno != ENOENT) {
			return (OpResult::FileOperationFailedError);
		}
	}
	if ((cresult == 0) && (st.st_mode & S_IFDIR)) {
		return (OpResult::Success);
	}
	cresult = mkdir (path.c_str (), S_IRWXU);
	if (cresult != 0) {
		return (OpResult::FileOperationFailedError);
	}
	result = OpResult::Success;
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if (a != INVALID_FILE_ATTRIBUTES) {
		if (a & FILE_ATTRIBUTE_DIRECTORY) {
			return (OpResult::Success);
		}
		else {
			return (OpResult::FileOperationFailedError);
		}
	}
	if (! CreateDirectory (path.c_str (), NULL)) {
		return (OpResult::FileOperationFailedError);
	}
	result = OpResult::Success;
#endif
	return (result);
}

OpResult OsUtil::removeDirectory (const StdString &path, bool isRecursiveRemove) {
	StringList files;
	StringList::const_iterator i1, i2;
	StdString filepath;
	OpResult result;
	int type;

	if (! isRecursiveRemove) {
#if PLATFORM_LINUX || PLATFORM_MACOS
		if (rmdir (path.c_str ()) != 0) {
			return (OpResult::FileOperationFailedError);
		}
#endif
#if PLATFORM_WINDOWS
		if (! RemoveDirectory (path.c_str ())) {
			return (OpResult::FileOperationFailedError);
		}
#endif
		return (OpResult::Success);
	}
	type = OsUtil::getFileType (path);
	if (type == OsUtil::DirectoryFile) {
		result = OsUtil::readDirectory (path, &files);
		if (result != OpResult::Success) {
			return (result);
		}
		i1 = files.cbegin ();
		i2 = files.cend ();
		while (i1 != i2) {
			filepath = *i1;
			++i1;
			if (! OsUtil::isFilename (filepath)) {
				continue;
			}
			filepath = OsUtil::getJoinedPath (path, filepath);
			type = OsUtil::getFileType (filepath);
			if (type == OsUtil::DirectoryFile) {
				result = OsUtil::removeDirectory (filepath, true);
				if (result != OpResult::Success) {
					return (result);
				}
			}
			else if (type == OsUtil::RegularFile) {
				result = OsUtil::removeFile (filepath);
				if (result != OpResult::Success) {
					return (result);
				}
			}
			else {
				return (OpResult::FileOperationFailedError);
			}
		}
		return (OsUtil::removeDirectory (path));
	}
	else if (type == OsUtil::RegularFile) {
		return (OsUtil::removeFile (path));
	}
	return (OpResult::FileOperationFailedError);
}

bool OsUtil::directoryExists (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	if (stat (path.c_str (), &st) != 0) {
		return (false);
	}
	return (st.st_mode & S_IFDIR);
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if ((a == INVALID_FILE_ATTRIBUTES) || (!(a & FILE_ATTRIBUTE_DIRECTORY))) {
		return (false);
	}
	return (true);
#endif
}

OpResult OsUtil::readDirectory (const StdString &path, StringList *destList) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	DIR *d;
	struct dirent *ent;

	d = opendir (path.c_str ());
	if (! d) {
		return (OpResult::FileOperationFailedError);
	}
	if (destList) {
		destList->clear ();
	}
	while (true) {
		ent = readdir (d);
		if (! ent) {
			break;
		}
		if (destList) {
			destList->push_back (StdString (ent->d_name));
		}
	}
	closedir (d);
#endif
#if PLATFORM_WINDOWS
	HANDLE h;
	WIN32_FIND_DATA f;
	StdString s;

	s.assign (path);
	s.append ("\\*");
	h = FindFirstFile (s.c_str (), &f);
	if (h == INVALID_HANDLE_VALUE) {
		if (GetLastError () != ERROR_FILE_NOT_FOUND) {
			return (OpResult::FileOperationFailedError);
		}
		if (destList) {
			destList->clear ();
		}
		return (OpResult::Success);
	}
	if (destList) {
		destList->clear ();
		if (!(f.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_REPARSE_POINT))) {
			destList->push_back (StdString (f.cFileName));
		}
	}
	while (true) {
		if (! FindNextFile (h, &f)) {
			break;
		}
		if (destList) {
			if (!(f.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_REPARSE_POINT))) {
				destList->push_back (StdString (f.cFileName));
			}
		}
	}
	FindClose (h);
#endif

	return (OpResult::Success);
}

bool OsUtil::fileExists (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	if (stat (path.c_str (), &st) != 0) {
		return (false);
	}
	return (st.st_mode & S_IFREG);
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if ((a == INVALID_FILE_ATTRIBUTES) || (a & FILE_ATTRIBUTE_DIRECTORY)) {
		return (false);
	}
	return (true);
#endif
}

Buffer *OsUtil::readFile (const StdString &path) {
	Buffer *buf;
	FILE *fp;
	uint8_t data[8192];
	int len;

	fp = fopen (path.c_str (), "rb");
	if (! fp) {
		return (NULL);
	}
	buf = new Buffer ();
	while (true) {
		len = (int) fread (data, 1, sizeof (data), fp);
		if (len > 0) {
			buf->add (data, len);
		}
		if (len < (int) sizeof (data)) {
			break;
		}
	}
	fclose (fp);
	return (buf);
}

OpResult OsUtil::readFileLines (const StdString &path, OsUtil::ReadFileLinesCallback callback, void *callbackData, int maxLineLength) {
	FILE *fp;
	uint8_t data[8192], *d, *end, *linestart;
	char c, lastchar;
	uint8_t bom1, bom2, bom3;
	int readlen, linelen;
	bool firstline;
	StdString line;
	OpResult result;

	if (maxLineLength < 0) {
		return (OpResult::InvalidParamError);
	}
	line.reserve (maxLineLength);

	fp = fopen (path.c_str (), "rb");
	if (! fp) {
		return (OpResult::FileOpenFailedError);
	}
	result = OpResult::Success;
	firstline = true;
	linelen = 0;
	lastchar = 0;
	while (true) {
		readlen = (int) fread (data, 1, sizeof (data), fp);
		if (readlen > 0) {
			linestart = NULL;
			d = data;
			end = data + readlen;
			while (d < end) {
				c = *d;
				if (c == '\n') {
					if (linestart) {
						line.append ((char *) linestart, d - linestart - ((lastchar == '\r') ? 1 : 0));
					}
					if (firstline) {
						firstline = false;
						linelen = line.length ();
						bom1 = 0;
						bom2 = 0;
						bom3 = 0;
						if (linelen >= 2) {
							bom1 = line.at (0) & 0xFF;
							bom2 = line.at (1) & 0xFF;
						}
						if (linelen >= 3) {
							bom3 = line.at (2) & 0xFF;
						}
						if ((bom1 == 0xEF) && (bom2 == 0xBB) && (bom3 == 0xBF)) {
							line = line.substr (3);
						}
					}
					result = callback (callbackData, line);
					line.assign ("");
					linelen = 0;
					linestart = NULL;
					lastchar = 0;
					if (result != OpResult::Success) {
						break;
					}
				}
				else {
					if (! linestart) {
						linestart = d;
					}
					++linelen;
					lastchar = c;
					if (linelen > maxLineLength) {
						result = OpResult::MalformedDataError;
						break;
					}
				}
				++d;
			}
			if (result == OpResult::Success) {
				if (linestart) {
					line.append ((char *) linestart, end - linestart - ((lastchar == '\r') ? 1 : 0));
				}
			}
		}
		if ((readlen < (int) sizeof (data)) || (result != OpResult::Success)) {
			break;
		}
	}
	fclose (fp);
	if ((result == OpResult::Success) && (! line.empty ())) {
		result = callback (callbackData, line);
	}
	return (result);
}

OpResult OsUtil::writeFile (const StdString &path, Buffer *writeData, bool freeWriteData) {
	OpResult result;
	FILE *fp;
	size_t wlen, datalen;
	uint8_t *pos;

	if (path.empty () || (! writeData)) {
		return (OpResult::InvalidParamError);
	}
	result = OpResult::Success;
	fp = fopen (path.c_str (), "wb");
	if (! fp) {
		result = OpResult::FileOpenFailedError;
	}
	else {
		pos = writeData->data;
		datalen = (size_t) writeData->length;
		while (datalen > 0) {
			wlen = fwrite (pos, 1, datalen, fp);
			if (wlen >= datalen) {
				break;
			}
			datalen -= wlen;
			pos += wlen;
		}
		fflush (fp);
		fclose (fp);
	}
	if (freeWriteData) {
		delete (writeData);
	}
	return (result);
}

OpResult OsUtil::removeFile (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	if (unlink (path.c_str ()) != 0) {
		return (OpResult::FileOperationFailedError);
	}
#endif
#if PLATFORM_WINDOWS
	if (! DeleteFile (path.c_str ())) {
		return (OpResult::FileOperationFailedError);
	}
#endif
	return (OpResult::Success);
}

int OsUtil::getFileType (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	if (stat (path.c_str (), &st) != 0) {
		if (errno == ENOENT) {
			return (OsUtil::FileNotFound);
		}
		return (OsUtil::FileOpenFailed);
	}
	if (st.st_mode & S_IFREG) {
		return (OsUtil::RegularFile);
	}
	if (st.st_mode & S_IFDIR) {
		return (OsUtil::DirectoryFile);
	}
	return (OsUtil::UnknownFileType);
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if (a == INVALID_FILE_ATTRIBUTES) {
		if (GetLastError () == ERROR_FILE_NOT_FOUND) {
			return (OsUtil::FileNotFound);
		}
		return (OsUtil::FileOpenFailed);
	}
	if (a & FILE_ATTRIBUTE_REPARSE_POINT) {
		return (OsUtil::LinkFile);
	}
	if (a & FILE_ATTRIBUTE_DIRECTORY) {
		return (OsUtil::DirectoryFile);
	}
	return (OsUtil::RegularFile);
#endif
}

int64_t OsUtil::getFileSize (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	if (stat (path.c_str (), &st) != 0) {
		return (-1);
	}
	if (!(st.st_mode & S_IFREG)) {
		return (-1);
	}
	return ((int64_t) st.st_size);
#endif
#if PLATFORM_WINDOWS
	HANDLE hfile;
	FILE_STANDARD_INFO st;
	int64_t result;

	hfile = CreateFile (path.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return (-1);
	}
	result = -1;
	if (GetFileInformationByHandleEx (hfile, FileStandardInfo, &st, sizeof (st))) {
		result = (int64_t) st.EndOfFile.QuadPart;
	}
	CloseHandle (hfile);
	return (result);
#endif
}

int64_t OsUtil::getDirectorySize (const StdString &path) {
	StringList files;
	StringList::const_iterator i1, i2;
	StdString filepath;
	OpResult result;
	int64_t total, filesize;
	int type;

	if (! OsUtil::directoryExists (path)) {
		return (-1);
	}
	result = OsUtil::readDirectory (path, &files);
	if (result != OpResult::Success) {
		return (-1);
	}
	total = 0;
	i1 = files.cbegin ();
	i2 = files.cend ();
	while (i1 != i2) {
		filepath = *i1;
		++i1;
		if (! OsUtil::isFilename (filepath)) {
			continue;
		}
		filepath = OsUtil::getJoinedPath (path, filepath);
		type = OsUtil::getFileType (filepath);
		if (type == OsUtil::DirectoryFile) {
			filesize = OsUtil::getDirectorySize (filepath);
			if (filesize > 0) {
				total += filesize;
			}
		}
		else if (type == OsUtil::RegularFile) {
			filesize = OsUtil::getFileSize (filepath);
			if (filesize > 0) {
				total += filesize;
			}
		}
	}
	return (total);
}

int64_t OsUtil::getFileMtime (const StdString &path) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	if (stat (path.c_str (), &st) != 0) {
		return (-1);
	}
	if (!(st.st_mode & S_IFREG)) {
		return (-1);
	}
	return ((int64_t) (st.st_mtime * 1000));
#endif
#if PLATFORM_WINDOWS
	HANDLE hfile;
	FILE_BASIC_INFO st;
	int64_t t;

	hfile = CreateFile (path.c_str (), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		return (-1);
	}
	t = -1;
	if (GetFileInformationByHandleEx (hfile, FileBasicInfo, &st, sizeof (st))) {
		t = st.LastWriteTime.QuadPart;
		t /= 10000;
		t -= 11644473600000ULL;
	}
	CloseHandle (hfile);
	return (t);
#endif
}

StdString OsUtil::getPathPrefix () {
	StdString s;

#if PLATFORM_LINUX || PLATFORM_MACOS
	s.assign ("/");
#endif
	return (s);
}

bool OsUtil::isRootPath (const StdString &path) {
	bool result;

	result = false;
#if PLATFORM_LINUX || PLATFORM_MACOS
	result = path.equals ("/");
#endif
#if PLATFORM_WINDOWS
	result = path.equals ("");
#endif
	return (result);
}

#if PLATFORM_WINDOWS
static int assignWcharString (WCHAR *src, StdString *dest) {
	WCHAR *w, c;
	int len;

	dest->assign ("");
	len = 0;
	w = src;
	while (true) {
		c = *w;
		if (c == '\0') {
			break;
		}
		dest->append (1, (char) c);
		++len;
		++w;
	}
	return (len);
}
#endif
OpResult OsUtil::readRootPath (StringList *destList) {
	OpResult result;
#if PLATFORM_WINDOWS
	HANDLE h;
	WCHAR volname[MAX_PATH] = L"";
	WCHAR volpaths[MAX_PATH + 1] = L"";
	WCHAR *w;
	DWORD charcount;
	StdString drive;
	int drivelen;
#endif

	result = OpResult::NotImplementedError;
#if PLATFORM_LINUX || PLATFORM_MACOS
	result = OsUtil::readDirectory (StdString ("/"), destList);
#endif
#if PLATFORM_WINDOWS
	h = FindFirstVolumeW (volname, ARRAYSIZE (volname));
	if (h == INVALID_HANDLE_VALUE) {
		return (OpResult::SystemOperationFailedError);
	}
	while (true) {
		charcount = MAX_PATH + 1;
		if (GetVolumePathNamesForVolumeNameW (volname, volpaths, charcount, &charcount)) {
			w = volpaths;
			while (charcount > 0) {
				drivelen = assignWcharString (w, &drive);
				if (drivelen <= 0) {
					break;
				}
				if ((drive.length () == 3) && (drive.at (1) == ':') && (drive.at (2) == '\\')) {
					destList->push_back (drive.substr (0, 2));
				}

				++drivelen;
				w += drivelen;
				if (charcount >= drivelen) {
					charcount -= drivelen;
				}
				else {
					charcount = 0;
				}
			}
		}
		if (! FindNextVolumeW (h, volname, ARRAYSIZE (volname))) {
			if (GetLastError () != ERROR_NO_MORE_FILES) {
				result = OpResult::SystemOperationFailedError;
			}
			else {
				result = OpResult::Success;
			}
			break;
		}
	}
	FindVolumeClose (h);
#endif
	return (result);
}

StdString OsUtil::getEnvValue (const StdString &key, const StdString &defaultValue) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *val;

	val = getenv (key.c_str ());
	if (! val) {
		return (defaultValue);
	}
	return (StdString (val));
#endif
#if PLATFORM_WINDOWS
	DWORD result, bufsize;
	LPTSTR buf;
	StdString val;
	std::basic_string<TCHAR> k;

	bufsize = 4096;
	buf = (LPTSTR) malloc (bufsize * sizeof (TCHAR));
	if (! buf) {
		return (defaultValue);
	}
	val.assign (defaultValue);
	k.assign (key);
	result = GetEnvironmentVariable (k.c_str (), buf, bufsize);
	if (result > bufsize) {
		bufsize = result;
		buf = (LPTSTR) realloc (buf, bufsize * sizeof (TCHAR));
		if (! buf) {
			return (defaultValue);
		}
		result = GetEnvironmentVariable (k.c_str (), buf, bufsize);
	}
	if ((result != 0) && (result <= bufsize)) {
		k.assign (buf);
		val.assign (k);
	}

	free (buf);
	return (val);
#endif
}
StdString OsUtil::getEnvValue (const StdString &key, const char *defaultValue) {
	return (OsUtil::getEnvValue (key, StdString (defaultValue)));
}
bool OsUtil::getEnvValue (const StdString &key, bool defaultValue) {
	StdString val;

	val = OsUtil::getEnvValue (key, defaultValue ? "true" : "false");
	return (val.equals ("true"));
}
int OsUtil::getEnvValue (const StdString &key, int defaultValue) {
	StdString val;
	int result;

	val = OsUtil::getEnvValue (key, "");
	if (val.empty () || (! val.parseInt (&result))) {
		return (defaultValue);
	}
	return (result);
}

OpResult OsUtil::setEnvValue (const StdString &key, const StdString &value) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	int result;

	result = setenv (key.c_str (), value.c_str (), 1);
	if (result != 0) {
		return (OpResult::SystemOperationFailedError);
	}
	return (OpResult::Success);
#elif PLATFORM_WINDOWS
	BOOL result;

	result = SetEnvironmentVariable (key.c_str (), value.c_str ());
	if (! result) {
		return (OpResult::SystemOperationFailedError);
	}
	return (OpResult::Success);
#else
	return (OpResult::NotImplementedError);
#endif
}

OpResult OsUtil::openUrl (const StdString &url) {
	OpResult result;
#if PLATFORM_LINUX
	StdString execfile, execarg, path;
	StringList parts, execnames;
	StringList::const_iterator i1, i2, j1, j2;
#endif

	result = OpResult::NotImplementedError;
#if PLATFORM_LINUX
	execfile = OsUtil::getEnvValue (OsUtil::browserEnvKey, "");
	if (! execfile.empty ()) {
		execarg.assign (execfile);
	}
	if (execfile.empty ()) {
		execnames.push_back (StdString ("xdg-open"));
		execnames.push_back (StdString ("firefox"));
		execnames.push_back (StdString ("google-chrome"));
		execnames.push_back (StdString ("chromium"));
		execnames.push_back (StdString ("mozilla"));

		path = OsUtil::getEnvValue (StdString (OsUtil::pathEnvKey), "");
		path.split (":", &parts);

		i1 = parts.cbegin ();
		i2 = parts.cend ();
		while (i1 != i2) {
			j1 = execnames.cbegin ();
			j2 = execnames.cend ();
			while (j1 != j2) {
				path.sprintf ("%s/%s", i1->c_str (), j1->c_str ());
				if (OsUtil::fileExists (path)) {
					execfile.assign (path);
					execarg.assign (j1->c_str ());
					break;
				}
				++j1;
			}
			if (! execfile.empty ()) {
				break;
			}
			++i1;
		}
	}
	if (execfile.empty ()) {
		return (OpResult::ProgramNotFoundError);
	}

	if (!(fork ())) {
		execlp (execfile.c_str (), execarg.c_str (), url.c_str (), NULL);
		_Exit (1);
	}
	result = OpResult::Success;
#endif
#if PLATFORM_MACOS
	if (!(fork ())) {
		execlp ("open", "open", url.c_str (), NULL);
		_Exit (1);
	}
	result = OpResult::Success;
#endif
#if PLATFORM_WINDOWS
	HINSTANCE h;

	result = OpResult::Success;
	h = ShellExecute (NULL, "open", url.c_str (), NULL, NULL, SW_SHOWNORMAL);
	if (((INT_PTR) h) <= 32) {
		result = OpResult::SystemOperationFailedError;
	}
#endif
	return (result);
}
