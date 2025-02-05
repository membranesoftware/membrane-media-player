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
#include "OsUtil.h"
#include "Json.h"
#include "SystemInterface.h"
#include "TaskGroup.h"
#include "RecordStore.h"
#include "MediaItem.h"
#include "MediaReader.h"
#include "MediaControl.h"
#include "MediaControlSearch.h"

// Stage values
constexpr const int Uninitialized = 0;
constexpr const int Running = 1;
constexpr const int FindStart = 2;
constexpr const int FindWait = 3;

MediaControlSearch::MediaControlSearch ()
: MediaSearch ()
{
}
MediaControlSearch::~MediaControlSearch () {
}

void MediaControlSearch::update (int msElapsed) {
	MediaControl::Status status;

	switch (stage) {
		case Uninitialized: {
			if (! MediaControl::instance->isReady) {
				break;
			}
			if (agentId.empty ()) {
				agentId.assign (MediaControl::instance->agentId);
				if (agentId.empty ()) {
					break;
				}
			}
			shouldReloadSearch = false;
			shouldAdvanceSearch = false;
			stage = FindStart;
			break;
		}
		case Running: {
			if (shouldReloadSearch) {
				eventRecordIds.assign (insertedRecordIds);
				eventCallback (removeRecordsCallback);
				RecordStore::instance->remove (insertedRecordIds);
				eventRecordIds.clear ();
				insertedRecordIds.clear ();
				shouldReloadSearch = false;
				stage = FindStart;
			}
			else if (shouldAdvanceSearch) {
				shouldAdvanceSearch = false;
				isLoading = true;
				resultOffset += pageSize;
				stage = FindWait;
				retain ();
				TaskGroup::instance->run (TaskGroup::RunContext (MediaControlSearch::findMediaItems, this));
			}
			else {
				MediaControl::instance->getStatus (&status);
				if (status.mediaCount >= 0) {
					mediaAvailableCount = status.mediaCount;
				}
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
			stage = FindWait;
			retain ();
			TaskGroup::instance->run (TaskGroup::RunContext (MediaControlSearch::findMediaItems, this));
			break;
		}
		case FindWait: {
			if (! isLoading) {
				if (searchReceiveCount >= setSize) {
					isFindComplete = true;
				}
				eventRecordIds.clear ();
				eventRecordIds.swap (foundRecordIds);
				eventCallback (addRecordsCallback);
				eventRecordIds.clear ();
				shouldAdvanceSearch = false;
				stage = Running;
				break;
			}
			break;
		}
	}
}

void MediaControlSearch::findMediaItems (void *itPtr) {
	MediaControlSearch *it = (MediaControlSearch *) itPtr;

	it->executeFindMediaItems ();
	it->isLoading = false;
	it->release ();
}
void MediaControlSearch::executeFindMediaItems () {
	StdString errmsg;
	std::list<MediaItem>::const_iterator i1, i2;
	Json *record;

	foundRecordIds.clear ();
	mediaItemList.clear ();
	setSize = MediaItem::countDatabaseRecords (MediaControl::instance->databasePath, &errmsg, searchKey);
	if (setSize < 0) {
		return;
	}
	if (setSize > 0) {
		if (! MediaItem::readDatabaseRows (MediaControl::instance->databasePath, &errmsg, &mediaItemList, searchKey, resultOffset, pageSize, sortOrder)) {
			return;
		}
		i1 = mediaItemList.cbegin ();
		i2 = mediaItemList.cend ();
		while (i1 != i2) {
			record = i1->createRecord (agentId);
			RecordStore::instance->insert (record, true);
			delete (record);
			foundRecordIds.push_back (i1->mediaId);
			insertedRecordIds.push_back (i1->mediaId);
			++searchReceiveCount;
			++i1;
		}
		mediaItemList.clear ();
	}
	lastStatusUpdateTime = OsUtil::getTime ();
}

void MediaControlSearch::doResetSearch (const StdString &searchKeyValue, int sortOrderValue) {
	nextSearchKey.assign (searchKeyValue);
	nextSortOrder = sortOrderValue;
	shouldReloadSearch = true;
}
