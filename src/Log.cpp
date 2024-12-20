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
#include <errno.h>
#include <unistd.h>
#endif
#if PLATFORM_WINDOWS
#include <io.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "App.h"
#include "SdlUtil.h"
#include "OsUtil.h"
#include "Log.h"

const char *Log::levelNames[Log::levelCount] = {
	"ERR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	"DEBUG1",
	"DEBUG2",
	"DEBUG3",
	"DEBUG4"
};

Log::Log ()
: isStdoutWriteEnabled (false)
, isFileWriteEnabled (false)
, writeLevel (Log::ErrLevel)
, isFileErrorLogged (false)
{
	SdlUtil::createMutex (&mutex);
}
Log::~Log () {
	SdlUtil::destroyMutex (&mutex);
}

void Log::setLevel (Log::LogLevel level) {
	if ((level >= 0) && ((int) level < Log::levelCount)) {
		writeLevel = level;
	}
}

OpResult Log::setLevelByName (const char *name) {
	StdString s;
	int i;
	OpResult result;

	result = OpResult::InvalidParamError;
	s.assign (StdString (name).uppercased ());
	for (i = 0; i < Log::levelCount; ++i) {
		if (s.equals (Log::levelNames[i])) {
			setLevel ((Log::LogLevel) i);
			result = OpResult::Success;
			break;
		}
	}
	return (result);
}

OpResult Log::setLevelByName (const StdString &name) {
	return (setLevelByName (name.c_str ()));
}

OpResult Log::openLogFile (const char *filename) {
	StdString fname;
	int fd;
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *c, *cwd;
	int sz;
#endif

	fname.assign (filename);
	if (fname.empty ()) {
		isFileWriteEnabled = false;
		writeFilename.assign ("");
		return (OpResult::Success);
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	if (filename[0] != '/') {
		c = NULL;
		cwd = NULL;
		sz = 0;
		do {
			sz += 1024;
			cwd = (char *) realloc (cwd, sz);
			c = getcwd (cwd, sz);
			if (c) {
				break;
			}
			if (errno != ERANGE) {
				::printf ("Failed to open log file %s - %s\n", filename, strerror (errno));
				break;
			}
		} while (sz < 8192);
		if (c && cwd) {
			fname.sprintf ("%s/%s", cwd, filename);
		}

		if (cwd) {
			free (cwd);
		}
		if (! c) {
			return (OpResult::FileOpenFailedError);
		}
	}
#endif
#if PLATFORM_WINDOWS
	fd = _open (fname.c_str (), _O_APPEND | _O_WRONLY | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
	fd = open (fname.c_str (), O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
	if (fd < 0) {
		::printf ("Failed to open log file %s - %s\n", fname.c_str (), strerror (errno));
		return (OpResult::FileOpenFailedError);
	}
	close (fd);

	isFileWriteEnabled = true;
	writeFilename.assign (fname);
	return (OpResult::Success);
}

OpResult Log::openLogFile (const StdString &filename) {
	return (openLogFile (filename.c_str ()));
}

void Log::voutput (Log::LogLevel level, const char *str, va_list args) {
	FILE *fp;
	int64_t now;
	va_list argscopy;
	StdString text;

	if (!(isStdoutWriteEnabled || isFileWriteEnabled)) {
		return;
	}
	if ((level < 0) || ((int) level >= Log::levelCount)) {
		level = Log::NoLevel;
	}
	if (level != Log::NoLevel) {
		if (level > writeLevel) {
			return;
		}
	}

	now = OsUtil::getTime ();
	text.appendSprintf ("[%s]", OsUtil::getTimestampString (now, true).c_str ());
	if (level != Log::NoLevel) {
		text.appendSprintf ("[%s]", Log::levelNames[level]);
	}
	text.append (" ");

	va_copy (argscopy, args);
	text.appendVsprintf (str, argscopy);
	va_end (argscopy);

	SDL_LockMutex (mutex);
	if (isStdoutWriteEnabled) {
		::printf ("%s%s", text.c_str (), CONFIG_NEWLINE);
	}
	if (isFileWriteEnabled) {
		fp = fopen (writeFilename.c_str (), "ab");
		if (! fp) {
			if (! isFileErrorLogged) {
				::printf ("Warning: could not open log file %s for writing - %s\n", writeFilename.c_str (), strerror (errno));
				isFileErrorLogged = true;
			}
		}
		else {
			fprintf (fp, "%s%s", text.c_str (), CONFIG_NEWLINE);
			fclose (fp);
		}
	}
	SDL_UnlockMutex (mutex);
}

void Log::write (Log::LogLevel level, const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (level, str, ap);
	va_end (ap);
}
void Log::write (Log::LogLevel level, const char *str, va_list args) {
	App::instance->log.voutput (level, str, args);
}

void Log::printf (const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	::printf ("[%s] %s\n", OsUtil::getTimestampString (OsUtil::getTime (), true).c_str (), text.c_str ());
	fflush (stdout);
	App::instance->writeConsoleOutput (text);
}

void Log::err (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::ErrLevel, str, ap);
	va_end (ap);
}

void Log::warning (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::WarningLevel, str, ap);
	va_end (ap);
}

void Log::notice (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::NoticeLevel, str, ap);
	va_end (ap);
}

void Log::info (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::InfoLevel, str, ap);
	va_end (ap);
}

static StdString getBaseFileName (const char *fileName) {
	StdString name;
	size_t pos;

	name.assign (fileName);
	pos = name.rfind ("/");
	if (pos != StdString::npos) {
		name.assign (name.substr (pos + 1));
	}
	return (name);
}

void Log::debug (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::DebugLevel, str, ap);
	va_end (ap);
}

void Log::debugSourcePosition (const char *fileName, const int fileLine, const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	Log::debug ("[%s:%i] %s", getBaseFileName (fileName).c_str (), fileLine, text.c_str ());
}

void Log::debug1 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug1Level, str, ap);
	va_end (ap);
}

void Log::debug1SourcePosition (const char *fileName, const int fileLine, const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	Log::debug1 ("[%s:%i] %s", getBaseFileName (fileName).c_str (), fileLine, text.c_str ());
}

void Log::debug2 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug2Level, str, ap);
	va_end (ap);
}

void Log::debug2SourcePosition (const char *fileName, const int fileLine, const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	Log::debug2 ("[%s:%i] %s", getBaseFileName (fileName).c_str (), fileLine, text.c_str ());
}

void Log::debug3 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug3Level, str, ap);
	va_end (ap);
}

void Log::debug3SourcePosition (const char *fileName, const int fileLine, const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	Log::debug3 ("[%s:%i] %s", getBaseFileName (fileName).c_str (), fileLine, text.c_str ());
}

void Log::debug4 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug4Level, str, ap);
	va_end (ap);
}

void Log::debug4SourcePosition (const char *fileName, const int fileLine, const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	Log::debug4 ("[%s:%i] %s", getBaseFileName (fileName).c_str (), fileLine, text.c_str ());
}
