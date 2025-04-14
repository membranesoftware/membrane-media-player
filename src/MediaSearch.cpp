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
#include "App.h"
#include "SdlUtil.h"
#include "OsUtil.h"
#include "StringList.h"
#include "Json.h"
#include "RecordStore.h"
#include "TaskGroup.h"
#include "SystemInterface.h"
#include "MediaSearch.h"

// Stage values
constexpr const int Uninitialized = 0;
constexpr const int Running = 1;
constexpr const int FindStart = 2;
constexpr const int FindWait = 3;

MediaSearch::MediaSearch ()
: pageSize (MediaSearch::defaultPageSize)
, sortOrder (SystemInterface::Constant_NameSort)
, lastStatusUpdateTime (0)
, mediaAvailableCount (0)
, isLoading (false)
, isFindComplete (false)
, resultOffset (0)
, setSize (0)
, searchReceiveCount (0)
, stage (Uninitialized)
, shouldReloadSearch (false)
, shouldAdvanceSearch (false)
, nextSortOrder (SystemInterface::Constant_NameSort)
, refcount (0)
{
	SdlUtil::createMutex (&refcountMutex);
	SdlUtil::createMutex (&recordIdMutex);
}
MediaSearch::~MediaSearch () {
	SDL_LockMutex (recordIdMutex);
	RecordStore::instance->remove (insertedRecordIds);
	insertedRecordIds.clear ();
	SDL_UnlockMutex (recordIdMutex);

	SdlUtil::destroyMutex (&refcountMutex);
	SdlUtil::destroyMutex (&recordIdMutex);
}

void MediaSearch::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}
void MediaSearch::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);
	if (isdestroyed) {
		delete (this);
	}
}

void MediaSearch::resetSearch (const StdString &searchKeyValue, int sortOrderValue) {
	nextSearchKey.assign (searchKeyValue);
	nextSortOrder = sortOrderValue;
	shouldReloadSearch = true;
}
void MediaSearch::resetSearch (const StdString &searchKeyValue) {
	resetSearch (searchKeyValue, sortOrder);
}
void MediaSearch::resetSearch (int sortOrderValue) {
	resetSearch (searchKey, sortOrderValue);
}
void MediaSearch::resetSearch () {
	resetSearch (searchKey, sortOrder);
}

void MediaSearch::advanceSearch () {
	if (isLoading || isFindComplete) {
		return;
	}
	shouldAdvanceSearch = true;
}

void MediaSearch::update (int msElapsed) {
	StringList ids;

	switch (stage) {
		case Uninitialized: {
			if (initialize ()) {
				shouldReloadSearch = false;
				shouldAdvanceSearch = false;
				stage = FindStart;
			}
			break;
		}
		case Running: {
			if (shouldReloadSearch) {
				SDL_LockMutex (recordIdMutex);
				insertedRecordIds.swap (ids);
				SDL_UnlockMutex (recordIdMutex);
				RecordStore::instance->remove (ids);
				shouldReloadSearch = false;
				stage = FindStart;
			}
			else if (shouldAdvanceSearch) {
				shouldAdvanceSearch = false;
				isLoading = true;
				resultOffset += pageSize;
				stage = FindWait;
				retain ();
				TaskGroup::instance->run (TaskGroup::RunContext (MediaSearch::loadSearchResults, this));
			}
			else {
				mediaAvailableCount = getMediaAvailableCount ();
			}
			break;
		}
		case FindStart: {
			searchKey.assign (nextSearchKey);
			sortOrder = nextSortOrder;
			isLoading = true;
			resultOffset = 0;
			setSize = 0;
			isFindComplete = false;
			shouldAdvanceSearch = false;
			searchReceiveCount = 0;
			lastStatusUpdateTime = OsUtil::getTime ();
			stage = FindWait;
			retain ();
			TaskGroup::instance->run (TaskGroup::RunContext (MediaSearch::loadSearchResults, this));
			break;
		}
		case FindWait: {
			if (! isLoading) {
				if (searchReceiveCount >= setSize) {
					isFindComplete = true;
				}
				shouldAdvanceSearch = false;
				stage = Running;
				break;
			}
			break;
		}
	}
}

bool MediaSearch::initialize () {
	// Default method takes no action
	return (true);
}

void MediaSearch::loadSearchResults (void *itPtr) {
	MediaSearch *it = (MediaSearch *) itPtr;
	JsonList records;

	it->findMediaItems (&records);
	it->processSearchResults (records);
	it->lastStatusUpdateTime = OsUtil::getTime ();
	it->isLoading = false;
	it->release ();
}
void MediaSearch::processSearchResults (const JsonList &records) {
	JsonList::const_iterator i1, i2;
	Json *record;
	StdString id;
	StringList ids;

	i1 = records.cbegin ();
	i2 = records.cend ();
	while (i1 != i2) {
		record = *i1;
		if (SystemInterface::instance->getCommandId (record) == SystemInterface::CommandId_MediaItem) {
			RecordStore::instance->insert (record, true);
			id = SystemInterface::instance->getCommandRecordId (record);
			ids.push_back (id);
			++searchReceiveCount;
		}
		++i1;
	}

	SDL_LockMutex (recordIdMutex);
	insertedRecordIds.append (ids);
	searchResultRecordIds.append (ids);
	SDL_UnlockMutex (recordIdMutex);
}
void MediaSearch::findMediaItems (JsonList *destList) {
	// Default method takes no action
}

void MediaSearch::getInsertedRecordIds (StringList *destList) {
	SDL_LockMutex (recordIdMutex);
	destList->append (insertedRecordIds);
	SDL_UnlockMutex (recordIdMutex);
}

void MediaSearch::getSearchResultRecordIds (StringList *destList) {
	SDL_LockMutex (recordIdMutex);
	destList->append (searchResultRecordIds);
	searchResultRecordIds.clear ();
	SDL_UnlockMutex (recordIdMutex);
}

int MediaSearch::getMediaAvailableCount () {
	// Default method takes no action
	return (-1);
}
