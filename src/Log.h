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
// Class that writes messages to the application log
#ifndef LOG_H
#define LOG_H

class Log {
public:
	Log ();
	~Log ();

	enum LogLevel {
		ErrLevel = 0,
		WarningLevel = 1,
		NoticeLevel = 2,
		InfoLevel = 3,
		DebugLevel = 4,
		Debug1Level = 5,
		Debug2Level = 6,
		Debug3Level = 7,
		Debug4Level = 8,
		NoLevel = 255
	};
	static constexpr const int levelCount = 9;
	static const char *levelNames[];

	// Read-write data members
	bool isStdoutWriteEnabled;

	// Read-only data members
	bool isFileWriteEnabled;
	StdString writeFilename;
	int writeLevel;

	// Set the log's level, causing it to write messages of that level and below
	void setLevel (Log::LogLevel level);

	// Set the log's level according to the value of the provided string. Returns a Result value.
	OpResult setLevelByName (const char *name);
	OpResult setLevelByName (const StdString &name);

	// Open the specified file for log message writing. If filename is empty, instead close any previously opened log file. Returns a Result value.
	OpResult openLogFile (const char *filename);
	OpResult openLogFile (const StdString &filename);

	// Write a message to the log using the provided format string and va_list
	void voutput (Log::LogLevel level, const char *str, va_list args);

	// Write a message to the default log instance using the specified parameters
	static void write (Log::LogLevel level, const char *str, ...) __attribute__((format(printf, 2, 3)));
	static void write (Log::LogLevel level, const char *str, va_list args);

	// Write a message to the default log instance without specifying a level
	static void printf (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the ERR level
	static void err (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the WARNING level
	static void warning (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the NOTICE level
	static void notice (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the INFO level
	static void info (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG level
	static void debug (const char *str, ...) __attribute__((format(printf, 1, 2)));
	static void debugSourcePosition (const char *fileName, const int fileLine, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Write a message to the default log instance at the DEBUG1 level
	static void debug1 (const char *str, ...) __attribute__((format(printf, 1, 2)));
	static void debug1SourcePosition (const char *fileName, const int fileLine, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Write a message to the default log instance at the DEBUG2 level
	static void debug2 (const char *str, ...) __attribute__((format(printf, 1, 2)));
	static void debug2SourcePosition (const char *fileName, const int fileLine, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Write a message to the default log instance at the DEBUG3 level
	static void debug3 (const char *str, ...) __attribute__((format(printf, 1, 2)));
	static void debug3SourcePosition (const char *fileName, const int fileLine, const char *str, ...) __attribute__((format(printf, 3, 4)));

	// Write a message to the default log instance at the DEBUG4 level
	static void debug4 (const char *str, ...) __attribute__((format(printf, 1, 2)));
	static void debug4SourcePosition (const char *fileName, const int fileLine, const char *str, ...) __attribute__((format(printf, 3, 4)));

protected:
	bool isFileErrorLogged;
	SDL_mutex *mutex;
};
#endif
