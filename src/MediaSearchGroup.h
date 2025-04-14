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
// Class that runs a set of MediaSearch objects
#ifndef MEDIA_SEARCH_GROUP_H
#define MEDIA_SEARCH_GROUP_H

class StringList;
class MediaSearch;
class MediaControlSearch;

class MediaSearchGroup {
public:
	MediaSearchGroup ();
	~MediaSearchGroup ();
	static MediaSearchGroup *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Read-only data members
	StdString searchKey;
	int sortOrder;

	typedef void (*SearchRecordCallback) (void *data, const StringList &recordIds);
	// Set callback functions to be invoked when search result sets change
	void addListener (void *callbackData, MediaSearchGroup::SearchRecordCallback addRecordsCallback, MediaSearchGroup::SearchRecordCallback removeRecordsCallback);

	// Remove previously added search result callbacks
	void removeListener (void *callbackData);

	// Set configuration values for media control search
	void configureMediaControlSearch (bool enable);

	// Set search parameters and restart active search operations
	void resetSearch (const StdString &searchKeyValue, int sortOrderValue);

	// Load the next page of results for active searches
	void advanceSearch ();

	struct Status {
		int mediaAvailableCount;
		int searchReceiveCount;
		int searchSetSize;
		int64_t lastStatusUpdateTime;
		bool isLoading;
		Status ():
			mediaAvailableCount (0),
			searchReceiveCount (0),
			searchSetSize (0),
			lastStatusUpdateTime (0),
			isLoading (false) { }
	};
	// Copy search status fields into destStatus
	void getStatus (MediaSearchGroup::Status *destStatus);

	// Update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

private:
	// Remove all items from searchList
	void clearSearchList ();

	// Remove the specified item from searchList. This method must be invoked only while holding a lock on searchMutex.
	void removeSearch (MediaSearch *searchItem);

	SDL_mutex *searchMutex;
	std::list<MediaSearch *> searchList;
	StringList removedSearchRecordIds;
	MediaControlSearch *mediaControlSearch;

	struct SearchRecordCallbackContext {
		void *callbackData;
		MediaSearchGroup::SearchRecordCallback addRecordsCallback;
		MediaSearchGroup::SearchRecordCallback removeRecordsCallback;
		SearchRecordCallbackContext ():
			callbackData (NULL),
			addRecordsCallback (NULL),
			removeRecordsCallback (NULL) { }
		SearchRecordCallbackContext (void *callbackData, MediaSearchGroup::SearchRecordCallback addRecordsCallback, MediaSearchGroup::SearchRecordCallback removeRecordsCallback):
			callbackData (callbackData),
			addRecordsCallback (addRecordsCallback),
			removeRecordsCallback (removeRecordsCallback) { }
	};
	SDL_mutex *callbackListMutex;
	std::list<MediaSearchGroup::SearchRecordCallbackContext> callbackList;

	MediaSearchGroup::Status status;
	SDL_mutex *statusMutex;
};
#endif
