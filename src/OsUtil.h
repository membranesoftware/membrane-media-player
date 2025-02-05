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
// Utility methods for operating system functions
#ifndef OS_UTIL_H
#define OS_UTIL_H

class Buffer;
class StringList;

class OsUtil {
public:
	static const char *monthNames[];
	static constexpr const char *homeEnvKey = "HOME";
	static constexpr const char *localAppDataEnvKey = "LOCALAPPDATA";
	static constexpr const char *browserEnvKey = "BROWSER";
	static constexpr const char *pathEnvKey = "PATH";
	static constexpr const char *langEnvKey = "LANG";
	static constexpr const char *logLevelEnvKey = "LOG_LEVEL";
	static constexpr const char *logConsoleEnvKey = "LOG_CONSOLE";
	static constexpr const char *logFilenameEnvKey = "LOG_FILENAME";
	static constexpr const char *resourcePathEnvKey = "RESOURCE_PATH";
	static constexpr const char *appDataPathEnvKey = "APPDATA_PATH";
	static constexpr const char *minDrawFrameDelayEnvKey = "MIN_DRAW_FRAME_DELAY";
	static constexpr const char *minUpdateFrameDelayEnvKey = "MIN_UPDATE_FRAME_DELAY";
	static constexpr const char *displayModeEnvKey = "DISPLAY_MODE";
	static constexpr const char *runScriptEnvKey = "RUN_SCRIPT";
	static constexpr const char *runFileEnvKey = "RUN_FILE";

	// Return the current time in milliseconds since the epoch
	static int64_t getTime ();

	// Return the current high-resolution time, as measured in 100-nanosecond intervals since the epoch
	static int64_t getHrtime ();

	// Return the application's process ID
	static int getProcessId ();

	// Return a string containing formatted text representing the time and date of the specified millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	static StdString getTimestampString (int64_t timestamp = 0, bool isTimezoneEnabled = false);

	// Return a string containing the joined path composed from basePath and one or more path elements
	static StdString getJoinedPath (const char *basePath, const char *pathElement1 = NULL, const char *pathElement2 = NULL, const char *pathElement3 = NULL, const char *pathElement4 = NULL, const char *pathElement5 = NULL, const char *pathElement6 = NULL, const char *pathElement7 = NULL);
	static StdString getJoinedPath (const StdString &basePath, const StdString &pathElement1 = StdString (), const StdString &pathElement2 = StdString (), const StdString &pathElement3 = StdString (), const StdString &pathElement4 = StdString (), const StdString &pathElement5 = StdString (), const StdString &pathElement6 = StdString (), const StdString &pathElement7 = StdString ());

	// Return a string containing the basename value of path, or an empty string for a path with no basename
	static StdString getPathBasename (const StdString &path);

	// Return a string containing the dirname value of path, or an empty string for a path with no dirname
	static StdString getPathDirname (const StdString &path);

	// Return a string containing the extension value of path, or an empty string for a path with no extension
	static StdString getPathExtension (const StdString &path);

	// Return a string containing path with extension appended
	static StdString getAppendExtensionPath (const StdString &path, const StdString &extension);

	// Return a string containing path with extension appended after removing any other extension present
	static StdString getReplaceExtensionPath (const StdString &path, const StdString &extension);

	// Return a string containing path after appending a trailing path separator if one is not already present
	static StdString getTrailingSeparatorPath (const StdString &path);

	// Parse path into path elements and set destList contents to the resulting strings
	static void splitPath (const StdString &path, StringList *destList);

	// Return a boolean value indicating if text contains a valid filename string
	static bool isFilename (const StdString &text);

	// Return a string containing the current working path, or an empty string if not known
	static StdString getWorkingPath ();

	// Return a string containing the system user home path, or an empty string if not known
	static StdString getUserHomePath ();

	// Return a string containing the application data path, or an empty string if not known
	static StdString getAppDataPath ();

	// Create the named directory if it doesn't already exist and return a Result value
	static OpResult createDirectory (const StdString &path);

	// Remove the named empty directory and return a Result value. If isRecursiveRemove is true, remove directory contents recursively to make it empty before removing it.
	static OpResult removeDirectory (const StdString &path, bool isRecursiveRemove = false);

	// Return a boolean value indicating if the provided path names a directory that exists
	static bool directoryExists (const StdString &path);

	// Clear destList and add filename list items from path, returning a Result value
	static OpResult readDirectory (const StdString &path, StringList *destList);

	// Return a boolean value indicating if the provided path names a file that exists
	static bool fileExists (const StdString &path);

	// Read the file at the specified path and return a newly created Buffer object holding the resulting data, or NULL if the file read failed. If a Buffer is returned by this method, the caller must delete it when no longer needed.
	static Buffer *readFile (const StdString &path);

	typedef OpResult (*ReadFileLinesCallback) (void *data, const StdString &line);
	// Read the file at the specified path, invoke the provided callback with text preceding each newline, and return a result value
	static OpResult readFileLines (const StdString &path, OsUtil::ReadFileLinesCallback callback, void *callbackData, int maxLineLength = 65536);

	// Write file data to the specified path and return a Result value. If freeWriteData is true, free the writeData buffer.
	static OpResult writeFile (const StdString &path, Buffer *writeData, bool freeWriteData = true);

	// Remove the named file and return a Result value
	static OpResult removeFile (const StdString &path);

	static constexpr const int FileNotFound = 0;
	static constexpr const int FileOpenFailed = 1;
	static constexpr const int RegularFile = 2;
	static constexpr const int DirectoryFile = 3;
	static constexpr const int LinkFile = 4;
	static constexpr const int UnknownFileType = 5;
	// Return the file type value found at the specified path
	static int getFileType (const StdString &path);

	// Return the size of the regular file at the specified path, or -1 if no such file was found
	static int64_t getFileSize (const StdString &path);

	// Return the total size of files in the directory at the specified path, or -1 if no such directory was found
	static int64_t getDirectorySize (const StdString &path);

	// Return the mtime value of the regular file at the specified path, or -1 if no such file was found
	static int64_t getFileMtime (const StdString &path);

	// Return the path prefix value for the platform
	static StdString getPathPrefix ();

	// Return true if the provided path value matches the root filesystem path
	static bool isRootPath (const StdString &path);

	// Clear destList and add filename list items from the root filesystem path, returning a Result value
	static OpResult readRootPath (StringList *destList);

	// Return the value of a variable from the environment, or the specified default if no such variable exists
	static StdString getEnvValue (const StdString &key, const StdString &defaultValue);
	static StdString getEnvValue (const StdString &key, const char *defaultValue);
	static bool getEnvValue (const StdString &key, bool defaultValue);
	static int getEnvValue (const StdString &key, int defaultValue);

	// Set an environment variable and return a Result value
	static OpResult setEnvValue (const StdString &key, const StdString &value);

	// Launch the system's default browser to open the specified URL and return a Result value
	static OpResult openUrl (const StdString &url);
};
#endif
