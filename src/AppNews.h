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
// Class that stores information received from application news requests
#ifndef APP_NEWS_H
#define APP_NEWS_H

class Json;

class AppNews {
public:
	AppNews ();
	~AppNews ();
	static AppNews *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Read-only data members
	bool isReady;
	bool isLoadFailed;
	StdString databasePath;

	// Set the news store to target the specified database path
	void configure (const StdString &databasePathValue);

	struct NewsPost {
		StdString body;
		int64_t publishTime;
		int64_t endTime;
		NewsPost ():
			publishTime (0),
			endTime (0) { }
	};
	struct NewsState {
		StdString updateBuildId;
		int64_t updatePublishTime;
		std::list<AppNews::NewsPost> posts;
		NewsState ():
			updatePublishTime (0) { }
	};
	// Parse news state from a GetApplicationNewsResult command string into the provided struct, and return true if state fields were successfully loaded
	bool parseCommand (const StdString &command, AppNews::NewsState *state);

	// Store news state from a GetApplicationNewsResult command string
	OpResult writeRecord (const StdString &command);

	// Read stored news state into the provided struct, and return true if state fields were successfully loaded
	bool readRecord (AppNews::NewsState *state);

private:
	// Task functions
	static void initialize (void *itPtr);
	OpResult executeInitialize ();

	// Row select callback
	static int readRecord_row (void *stringPtr, int columnCount, char **columnValues, char **columnNames);

	// Read state fields from a GetApplicationNewsResult params object
	void parseCommandParams (Json *params, AppNews::NewsState *state);

	bool isDatabaseOpen;
	SDL_mutex *databaseMutex;
};
#endif
