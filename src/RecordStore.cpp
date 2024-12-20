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
#include "SystemInterface.h"
#include "Prng.h"
#include "RecordStore.h"

RecordStore *RecordStore::instance = NULL;

RecordStore::RecordStore ()
{
	SdlUtil::createMutex (&mutex);
}
RecordStore::~RecordStore () {
	clear ();
	SdlUtil::destroyMutex (&mutex);
}

void RecordStore::createInstance () {
	if (! RecordStore::instance) {
		RecordStore::instance = new RecordStore ();
	}
}
void RecordStore::freeInstance () {
	if (RecordStore::instance) {
		delete (RecordStore::instance);
		RecordStore::instance = NULL;
	}
}

void RecordStore::clear () {
	std::map<StdString, RecordStore::RecordEntry>::iterator i1, i2;

	SDL_LockMutex (mutex);
	i1 = recordMap.begin ();
	i2 = recordMap.end ();
	while (i1 != i2) {
		if (i1->second.json) {
			delete (i1->second.json);
		}
		++i1;
	}
	recordMap.clear ();
	SDL_UnlockMutex (mutex);
}

bool RecordStore::insert (Json *sourceRecord, bool retainOverwrite) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;
	StdString id;
	RecordStore::RecordEntry entry;
	Json *json;

	if (! sourceRecord) {
		return (false);
	}
	id = SystemInterface::instance->getCommandRecordId (sourceRecord);
	if (id.empty ()) {
		return (false);
	}

	json = sourceRecord->copy ();
	SDL_LockMutex (mutex);
	pos = recordMap.find (id);
	if (pos != recordMap.end ()) {
		if (pos->second.json) {
			delete (pos->second.json);
		}
		pos->second.json = json;
		if (retainOverwrite) {
			++(pos->second.refcount);
		}
	}
	else {
		entry.json = json;
		entry.refcount = 1;
		recordMap.insert (std::pair<StdString, RecordStore::RecordEntry> (id, entry));
	}
	SDL_UnlockMutex (mutex);
	return (true);
}

void RecordStore::remove (const StdString &recordId) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;

	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		--(pos->second.refcount);
		if (pos->second.refcount <= 0) {
			delete (pos->second.json);
			recordMap.erase (pos);
		}
	}
	SDL_UnlockMutex (mutex);
}
void RecordStore::remove (const StringList &recordIds) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;
	StringList::const_iterator i1, i2;

	if (recordIds.empty ()) {
		return;
	}
	SDL_LockMutex (mutex);
	i1 = recordIds.cbegin ();
	i2 = recordIds.cend ();
	while (i1 != i2) {
		pos = recordMap.find (*i1);
		if (pos != recordMap.end ()) {
			--(pos->second.refcount);
			if (pos->second.refcount <= 0) {
				delete (pos->second.json);
				recordMap.erase (pos);
			}
		}
		++i1;
	}
	SDL_UnlockMutex (mutex);
}

bool RecordStore::exists (const StdString &recordId, bool retainRecord) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		result = true;
		if (retainRecord) {
			++(pos->second.refcount);
		}
	}
	SDL_UnlockMutex (mutex);
	return (result);
}

Json *RecordStore::find (const StdString &recordId, int recordType, bool retainRecord) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;
	Json *result;

	result = NULL;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		if ((recordType < 0) || (SystemInterface::instance->getCommandId (pos->second.json) == recordType)) {
			result = pos->second.json->copy ();
			if (retainRecord) {
				++(pos->second.refcount);
			}
		}
	}
	SDL_UnlockMutex (mutex);
	return (result);
}
bool RecordStore::find (Json *destJson, const StdString &recordId, int recordType, bool retainRecord) {
	std::map<StdString, RecordStore::RecordEntry>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (mutex);
	pos = recordMap.find (recordId);
	if (pos != recordMap.end ()) {
		if ((recordType < 0) || (SystemInterface::instance->getCommandId (pos->second.json) == recordType)) {
			result = true;
			if (destJson) {
				destJson->copyValue (pos->second.json);
			}
			if (retainRecord) {
				++(pos->second.refcount);
			}
		}
	}
	SDL_UnlockMutex (mutex);
	return (result);
}

Json *RecordStore::find (RecordStore::FindMatchFunction matchFn, void *matchData, bool retainRecord) {
	std::map<StdString, RecordStore::RecordEntry>::iterator i1, i2;
	Json *result;

	result = NULL;
	SDL_LockMutex (mutex);
	i1 = recordMap.begin ();
	i2 = recordMap.end ();
	while (i1 != i2) {
		if (matchFn (matchData, i1->second.json)) {
			result = i1->second.json->copy ();
			if (retainRecord) {
				++(i1->second.refcount);
			}
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (mutex);
	return (result);
}
bool RecordStore::find (Json *destJson, RecordStore::FindMatchFunction matchFn, void *matchData, bool retainRecord) {
	std::map<StdString, RecordStore::RecordEntry>::iterator i1, i2;
	bool result;

	result = false;
	SDL_LockMutex (mutex);
	i1 = recordMap.begin ();
	i2 = recordMap.end ();
	while (i1 != i2) {
		if (matchFn (matchData, i1->second.json)) {
			result = true;
			if (destJson) {
				destJson->copyValue (i1->second.json);
			}
			if (retainRecord) {
				++(i1->second.refcount);
			}
			break;
		}
		++i1;
	}
	SDL_UnlockMutex (mutex);
	return (result);
}

void RecordStore::findRecords (RecordStore::FindMatchFunction matchFn, void *matchData, JsonList *destList, bool shouldClear) {
	std::map<StdString, RecordStore::RecordEntry>::iterator i1, i2;

	if (shouldClear) {
		destList->clear ();
	}
	SDL_LockMutex (mutex);
	i1 = recordMap.begin ();
	i2 = recordMap.end ();
	while (i1 != i2) {
		if (matchFn (matchData, i1->second.json)) {
			destList->push_back (i1->second.json->copy ());
		}
		++i1;
	}
	SDL_UnlockMutex (mutex);
}

void RecordStore::findRecordIds (RecordStore::FindMatchFunction matchFn, void *matchData, StringList *destList) {
	std::map<StdString, RecordStore::RecordEntry>::iterator i1, i2;

	destList->clear ();
	SDL_LockMutex (mutex);
	i1 = recordMap.begin ();
	i2 = recordMap.end ();
	while (i1 != i2) {
		if (matchFn (matchData, i1->second.json)) {
			destList->push_back (i1->first);
		}
		++i1;
	}
	SDL_UnlockMutex (mutex);
}

int RecordStore::countRecords () {
	int result;

	SDL_LockMutex (mutex);
	result = (int) recordMap.size ();
	SDL_UnlockMutex (mutex);
	return (result);
}

StdString RecordStore::getRecordId (int commandId, int64_t recordTime) {
	StdString s, result;
	int n, i;

	if (commandId < 0) {
		commandId = 0;
	}
	if (commandId > 0xFFFF) {
		commandId = 0xFFFF;
	}
	if (recordTime <= 0) {
		recordTime = OsUtil::getHrtime ();
	}

	s.sprintf ("%014llx", (long long int) recordTime);
	result.assign (s.substr (0, 8));
	result.append ("-");
	result.append (s.substr (8, 4));
	result.append ("-");
	result.append (s.substr (12, 2));
	s.sprintf ("%04llx", (long long int) commandId);
	result.append (s.substr (0, 2));
	result.append ("-");
	result.append (s.substr (2, 2));

	n = Prng::instance->getRandomNumber (0, 255);
	result.append (StdString::createSprintf ("%02llx", (long long int) n));
	result.append ("-");
	for (i = 0; i < 6; ++i) {
		n = Prng::instance->getRandomNumber (0, 255);
		result.append (StdString::createSprintf ("%02llx", (long long int) n));
	}
	return (result);
}

int RecordStore::getRecordIdCommand (const StdString &recordId) {
	StdString s1, s2;
	int result, n1, n2;

	result = -1;
	if (recordId.isUuid ()) {
		s1 = StdString (recordId.substr (16, 2));
		s2 = StdString (recordId.substr (19, 2));
		if (s1.parseHex (&n1) && s2.parseHex (&n2)) {
			result = (n1 << 8) | n2;
		}
	}
	return (result);
}

int64_t RecordStore::getRecordIdTime (const StdString &recordId) {
	StdString s1, s2, s3;
	int64_t result, n1, n2, n3;

	result = -1;
	if (recordId.isUuid ()) {
		s1 = StdString (recordId.substr (0, 8));
		s2 = StdString (recordId.substr (9, 4));
		s3 = StdString (recordId.substr (14, 2));
		if (s1.parseHex (&n1) && s2.parseHex (&n2) && s3.parseHex (&n3)) {
			result = (n1 << 24) | (n2 << 8) | n3;
		}
	}
	return (result);
}

bool RecordStore::matchCommandId (void *intPtr, Json *record) {
	return (SystemInterface::instance->getCommandId (record) == *((int *) intPtr));
}

bool RecordStore::matchAgentId (void *agentIdStringPtr, Json *record) {
	StdString *agentid = (StdString *) agentIdStringPtr;

	return (agentid->equals (SystemInterface::instance->getCommandAgentId (record)));
}
