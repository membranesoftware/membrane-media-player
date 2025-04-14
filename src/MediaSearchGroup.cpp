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
#include "SdlUtil.h"
#include "SystemInterface.h"
#include "MediaSearch.h"
#include "MediaControlSearch.h"
#include "MediaSearchGroup.h"

MediaSearchGroup *MediaSearchGroup::instance = NULL;

MediaSearchGroup::MediaSearchGroup ()
: sortOrder (SystemInterface::Constant_NameSort)
, mediaControlSearch (NULL)
{
	SdlUtil::createMutex (&searchMutex);
	SdlUtil::createMutex (&callbackListMutex);
	SdlUtil::createMutex (&statusMutex);

	status.mediaAvailableCount = -1;
}
MediaSearchGroup::~MediaSearchGroup () {
	clearSearchList ();
	SdlUtil::destroyMutex (&searchMutex);
	SdlUtil::destroyMutex (&callbackListMutex);
	SdlUtil::destroyMutex (&statusMutex);
}

void MediaSearchGroup::createInstance () {
	if (! MediaSearchGroup::instance) {
		MediaSearchGroup::instance = new MediaSearchGroup ();
	}
}
void MediaSearchGroup::freeInstance () {
	if (MediaSearchGroup::instance) {
		delete (MediaSearchGroup::instance);
		MediaSearchGroup::instance = NULL;
	}
}

void MediaSearchGroup::clearSearchList () {
	SDL_LockMutex (searchMutex);
	if (mediaControlSearch) {
		mediaControlSearch->release ();
		mediaControlSearch = NULL;
	}
	searchList.clear ();
	SDL_UnlockMutex (searchMutex);
}

void MediaSearchGroup::removeSearch (MediaSearch *searchItem) {
	std::list<MediaSearch *>::iterator i1, i2;
	MediaSearch *search;

	i1 = searchList.begin ();
	i2 = searchList.end ();
	while (i1 != i2) {
		search = *i1;
		if (search == searchItem) {
			search->getInsertedRecordIds (&removedSearchRecordIds);
			searchList.erase (i1);
			break;
		}
		++i1;
	}
}

void MediaSearchGroup::addListener (void *callbackData, MediaSearchGroup::SearchRecordCallback addRecordsCallback, MediaSearchGroup::SearchRecordCallback removeRecordsCallback) {
	std::list<MediaSearchGroup::SearchRecordCallbackContext>::iterator i1, i2;
	bool found;

	SDL_LockMutex (callbackListMutex);
	found = false;
	i1 = callbackList.begin ();
	i2 = callbackList.end ();
	while (i1 != i2) {
		if (i1->callbackData == callbackData) {
			found = true;
			i1->addRecordsCallback = addRecordsCallback;
			i1->removeRecordsCallback = removeRecordsCallback;
			break;
		}
		++i1;
	}
	if (! found) {
		callbackList.push_back (MediaSearchGroup::SearchRecordCallbackContext (callbackData, addRecordsCallback, removeRecordsCallback));
	}
	SDL_UnlockMutex (callbackListMutex);
}
void MediaSearchGroup::removeListener (void *callbackData) {
	std::list<MediaSearchGroup::SearchRecordCallbackContext>::iterator i1, i2;

	SDL_LockMutex (callbackListMutex);
	i1 = callbackList.begin ();
	i2 = callbackList.end ();
	while (i1 != i2) {
		if (i1->callbackData == callbackData) {
			callbackList.erase (i1);
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (callbackListMutex);
}

void MediaSearchGroup::configureMediaControlSearch (bool enable) {
	SDL_LockMutex (searchMutex);
	if (enable) {
		if (! mediaControlSearch) {
			mediaControlSearch = new MediaControlSearch ();
			mediaControlSearch->retain ();
			searchList.push_back (mediaControlSearch);
		}
	}
	else {
		if (mediaControlSearch) {
			removeSearch (mediaControlSearch);
			mediaControlSearch->release ();
			mediaControlSearch = NULL;
		}
	}
	SDL_UnlockMutex (searchMutex);
}

void MediaSearchGroup::resetSearch (const StdString &searchKeyValue, int sortOrderValue) {
	std::list<MediaSearch *>::const_iterator i1, i2;

	searchKey.assign (searchKeyValue);
	sortOrder = sortOrderValue;
	SDL_LockMutex (searchMutex);
	i1 = searchList.cbegin ();
	i2 = searchList.cend ();
	while (i1 != i2) {
		(*i1)->getInsertedRecordIds (&removedSearchRecordIds);
		++i1;
	}
	i1 = searchList.cbegin ();
	i2 = searchList.cend ();
	while (i1 != i2) {
		(*i1)->resetSearch (searchKey, sortOrder);
		++i1;
	}
	SDL_UnlockMutex (searchMutex);
}

void MediaSearchGroup::advanceSearch () {
	std::list<MediaSearch *>::const_iterator i1, i2;

	SDL_LockMutex (searchMutex);
	i1 = searchList.cbegin ();
	i2 = searchList.cend ();
	while (i1 != i2) {
		(*i1)->advanceSearch ();
		++i1;
	}
	SDL_UnlockMutex (searchMutex);
}

void MediaSearchGroup::getStatus (MediaSearchGroup::Status *destStatus) {
	SDL_LockMutex (statusMutex);
	*destStatus = status;
	SDL_UnlockMutex (statusMutex);
}

void MediaSearchGroup::update (int msElapsed) {
	std::list<MediaSearch *>::const_iterator i1, i2;
	std::list<MediaSearchGroup::SearchRecordCallbackContext>::const_iterator j1, j2;
	MediaSearch *search;
	MediaSearchGroup::Status curstatus;
	StringList resultrecordids, removedrecordids;

	SDL_LockMutex (searchMutex);
	i1 = searchList.cbegin ();
	i2 = searchList.cend ();
	while (i1 != i2) {
		search = *i1;
		search->update (msElapsed);
		search->getSearchResultRecordIds (&resultrecordids);
		curstatus.mediaAvailableCount += search->mediaAvailableCount;
		curstatus.searchReceiveCount += search->searchReceiveCount;
		curstatus.searchSetSize += search->setSize;
		if (search->lastStatusUpdateTime > curstatus.lastStatusUpdateTime) {
			curstatus.lastStatusUpdateTime = search->lastStatusUpdateTime;
		}
		if (search->isLoading) {
			curstatus.isLoading = true;
		}
		++i1;
	}
	removedSearchRecordIds.swap (removedrecordids);
	SDL_UnlockMutex (searchMutex);

	SDL_LockMutex (statusMutex);
	status = curstatus;
	SDL_UnlockMutex (statusMutex);

	if (! removedrecordids.empty ()) {
		SDL_LockMutex (callbackListMutex);
		j1 = callbackList.cbegin ();
		j2 = callbackList.cend ();
		while (j1 != j2) {
			j1->removeRecordsCallback (j1->callbackData, removedrecordids);
			++j1;
		}
		SDL_UnlockMutex (callbackListMutex);
	}

	if (! resultrecordids.empty ()) {
		SDL_LockMutex (callbackListMutex);
		j1 = callbackList.cbegin ();
		j2 = callbackList.cend ();
		while (j1 != j2) {
			j1->addRecordsCallback (j1->callbackData, resultrecordids);
			++j1;
		}
		SDL_UnlockMutex (callbackListMutex);
	}
}
