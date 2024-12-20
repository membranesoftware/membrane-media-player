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
// Base class for execution of media search operations
#ifndef MEDIA_SEARCH_H
#define MEDIA_SEARCH_H

#include "StringList.h"

class MediaItem;

class MediaSearch {
public:
	MediaSearch ();
	virtual ~MediaSearch ();

	static constexpr const int defaultPageSize = 64;

	typedef void (*EventCallback) (void *data, MediaSearch *search);
	struct EventCallbackContext {
		MediaSearch::EventCallback callback;
		void *callbackData;
		EventCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		EventCallbackContext (MediaSearch::EventCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Read-write data members
	int pageSize;
	int sortOrder;
	MediaSearch::EventCallbackContext addRecordsCallback;
	MediaSearch::EventCallbackContext removeRecordsCallback;

	// Read-only data members
	StdString agentId;
	int64_t lastStatusUpdateTime;
	StringList eventRecordIds;
	int mediaAvailableCount;
	StdString searchKey;
	bool isLoading;
	bool isFindComplete;
	int resultOffset;
	int setSize;
	int searchReceiveCount;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Invoke any function contained in callback and return a boolean value indicating if a function executed
	bool eventCallback (const MediaSearch::EventCallbackContext &callback);

	// Execute operations appropriate for current search state
	virtual void update (int msElapsed);

	// Start a new search operation targeting searchKeyValue and sortOrderValue. If searchKeyValue or sortKeyValue are not provided, leave them unchanged from current values.
	void resetSearch (const StdString &searchKeyValue, int sortOrderValue);
	void resetSearch (const StdString &searchKeyValue);
	void resetSearch (int sortOrderValue);
	void resetSearch ();

	// Load the next page of results for the active search
	void advanceSearch ();

protected:
	// Start a new search operation targeting searchKeyValue and sortOrderValue
	virtual void doResetSearch (const StdString &searchKeyValue, int sortOrderValue);

	int stage;
	StringList foundRecordIds;
	StringList insertedRecordIds;
	bool shouldReloadSearch;
	bool shouldAdvanceSearch;
	StdString nextSearchKey;
	int nextSortOrder;

private:
	SDL_mutex *refcountMutex;
	int refcount;
};
#endif
