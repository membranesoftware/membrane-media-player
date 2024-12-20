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
#include <stdlib.h>
#include "Config.h"
#include "StdString.h"
#include "Json.h"
#include "SystemInterface.h"

SystemInterface *SystemInterface::instance = NULL;

SystemInterface::SystemInterface ()
: hashCreate (SystemInterface::defaultHashCreate)
, hashUpdate (SystemInterface::defaultHashUpdate)
, hashDigest (SystemInterface::defaultHashDigest)
, lastError ("")
{
	populate ();
}
SystemInterface::~SystemInterface () {
}

void SystemInterface::createInstance () {
	if (! SystemInterface::instance) {
		SystemInterface::instance = new SystemInterface ();
	}
}
void SystemInterface::freeInstance () {
	if (SystemInterface::instance) {
		delete (SystemInterface::instance);
		SystemInterface::instance = NULL;
	}
}

void SystemInterface::populate () {
	populate_commandMap ();
	populate_populateDefaultFieldsMap ();
	populate_transformInputFieldsMap ();
	populate_transformOutputFieldsMap ();
	populate_hashFieldsMap ();
}

void SystemInterface::populate_commandMap () {
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CommandResult"), SystemInterface::Command (0, StdString ("CommandResult"), StdString ("CommandResult"))));
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("Fixture1Record"), SystemInterface::Command (328, StdString ("Fixture1Record"), StdString ("Fixture1Record"))));
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("Fixture2Record"), SystemInterface::Command (329, StdString ("Fixture2Record"), StdString ("Fixture2Record"))));
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetApplicationNews"), SystemInterface::Command (318, StdString ("GetApplicationNews"), StdString ("GetApplicationNews"))));
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetApplicationNewsResult"), SystemInterface::Command (319, StdString ("GetApplicationNewsResult"), StdString ("GetApplicationNewsResult"))));
	commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaItem"), SystemInterface::Command (16, StdString ("MediaItem"), StdString ("MediaItem"))));
	commandIdMap.insert (std::pair<int, StdString> (0, StdString ("CommandResult")));
	commandIdMap.insert (std::pair<int, StdString> (328, StdString ("Fixture1Record")));
	commandIdMap.insert (std::pair<int, StdString> (329, StdString ("Fixture2Record")));
	commandIdMap.insert (std::pair<int, StdString> (318, StdString ("GetApplicationNews")));
	commandIdMap.insert (std::pair<int, StdString> (319, StdString ("GetApplicationNewsResult")));
	commandIdMap.insert (std::pair<int, StdString> (16, StdString ("MediaItem")));
}

void SystemInterface::populate_populateDefaultFieldsMap () {
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CommandResult"), SystemInterface::populateDefaultFields_CommandResult));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("Fixture1Record"), SystemInterface::populateDefaultFields_Fixture1Record));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("Fixture2Record"), SystemInterface::populateDefaultFields_Fixture2Record));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetApplicationNews"), SystemInterface::populateDefaultFields_GetApplicationNews));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetApplicationNewsResult"), SystemInterface::populateDefaultFields_GetApplicationNewsResult));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetApplicationNewsResultPost"), SystemInterface::populateDefaultFields_GetApplicationNewsResultPost));
	populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaItem"), SystemInterface::populateDefaultFields_MediaItem));
}
void SystemInterface::populate_transformInputFieldsMap () {
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("CommandResult"), SystemInterface::transformInputFields_CommandResult));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("Fixture1Record"), SystemInterface::transformInputFields_Fixture1Record));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("Fixture2Record"), SystemInterface::transformInputFields_Fixture2Record));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("GetApplicationNews"), SystemInterface::transformInputFields_GetApplicationNews));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("GetApplicationNewsResult"), SystemInterface::transformInputFields_GetApplicationNewsResult));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("GetApplicationNewsResultPost"), SystemInterface::transformInputFields_GetApplicationNewsResultPost));
	transformInputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformInputFieldsFunction> (StdString ("MediaItem"), SystemInterface::transformInputFields_MediaItem));
}
void SystemInterface::populate_transformOutputFieldsMap () {
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("CommandResult"), SystemInterface::transformOutputFields_CommandResult));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("Fixture1Record"), SystemInterface::transformOutputFields_Fixture1Record));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("Fixture2Record"), SystemInterface::transformOutputFields_Fixture2Record));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("GetApplicationNews"), SystemInterface::transformOutputFields_GetApplicationNews));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("GetApplicationNewsResult"), SystemInterface::transformOutputFields_GetApplicationNewsResult));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("GetApplicationNewsResultPost"), SystemInterface::transformOutputFields_GetApplicationNewsResultPost));
	transformOutputFieldsMap.insert (std::pair<StdString, SystemInterface::TransformOutputFieldsFunction> (StdString ("MediaItem"), SystemInterface::transformOutputFields_MediaItem));
}
void SystemInterface::populate_hashFieldsMap () {
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CommandResult"), SystemInterface::hashFields_CommandResult));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("Fixture1Record"), SystemInterface::hashFields_Fixture1Record));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("Fixture2Record"), SystemInterface::hashFields_Fixture2Record));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetApplicationNews"), SystemInterface::hashFields_GetApplicationNews));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetApplicationNewsResult"), SystemInterface::hashFields_GetApplicationNewsResult));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetApplicationNewsResultPost"), SystemInterface::hashFields_GetApplicationNewsResultPost));
	hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaItem"), SystemInterface::hashFields_MediaItem));
}
void SystemInterface::populateDefaultFields_CommandResult (Json *destObject) {
}
void SystemInterface::transformInputFields_CommandResult (Json *destObject) {
	StdString s;
	
	if (destObject->isString ("success")) {
		s = destObject->getString ("success", "");
		if (s.equals ("true")) {
			destObject->set ("success", true);
		}
		else if (s.equals ("false")) {
			destObject->set ("success", false);
		}
	}
}
void SystemInterface::transformOutputFields_CommandResult (Json *destObject) {
}
void SystemInterface::hashFields_CommandResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	
	s = commandParams->getString ("error", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s = commandParams->getString ("itemId", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s = commandParams->getString ("stringResult", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%s", commandParams->getBoolean ("success", false) ? "true" : "false");
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s = commandParams->getString ("taskId", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
}
void SystemInterface::populateDefaultFields_Fixture1Record (Json *destObject) {
}
void SystemInterface::transformInputFields_Fixture1Record (Json *destObject) {
	if (destObject->isString ("n1")) {
		destObject->set ("n1", destObject->getString ("n1", "").parsedFloat ((double) 0.0f));
	}
}
void SystemInterface::transformOutputFields_Fixture1Record (Json *destObject) {
}
void SystemInterface::hashFields_Fixture1Record (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	
	s = commandParams->getString ("id", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	if (commandParams->isNumber ("n1")) {
		s.sprintf ("%lli", (long long int) commandParams->getNumber ("n1", (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s = commandParams->getString ("s1", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
}
void SystemInterface::populateDefaultFields_Fixture2Record (Json *destObject) {
}
void SystemInterface::transformInputFields_Fixture2Record (Json *destObject) {
	if (destObject->isString ("n1")) {
		destObject->set ("n1", destObject->getString ("n1", "").parsedFloat ((double) 0.0f));
	}
}
void SystemInterface::transformOutputFields_Fixture2Record (Json *destObject) {
}
void SystemInterface::hashFields_Fixture2Record (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	
	s = commandParams->getString ("id", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	if (commandParams->isNumber ("n1")) {
		s.sprintf ("%lli", (long long int) commandParams->getNumber ("n1", (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s = commandParams->getString ("s1", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
}
void SystemInterface::populateDefaultFields_GetApplicationNews (Json *destObject) {
}
void SystemInterface::transformInputFields_GetApplicationNews (Json *destObject) {
}
void SystemInterface::transformOutputFields_GetApplicationNews (Json *destObject) {
}
void SystemInterface::hashFields_GetApplicationNews (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	
	s = commandParams->getString ("buildId", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
}
void SystemInterface::populateDefaultFields_GetApplicationNewsResult (Json *destObject) {
	Json j1, *j2;
	JsonList items;
	int i, len;
	
	items.clear ();
	len = destObject->getArrayLength ("applicationPosts");
	for (i = 0; i < len; ++i) {
		if (destObject->getArrayObject ("applicationPosts", i, &j1)) {
			j2 = j1.copy ();
			SystemInterface::populateDefaultFields_GetApplicationNewsResultPost (j2);
			items.push_back (j2);
		}
	}
	if (items.size () > 0) {
		destObject->set ("applicationPosts", &items);
	}
}
void SystemInterface::transformInputFields_GetApplicationNewsResult (Json *destObject) {
	Json j1, *j2;
	JsonList items;
	int i, len;
	
	len = destObject->getArrayLength ("applicationPosts");
	if (len > 0) {
		items.clear ();
		for (i = 0; i < len; ++i) {
			if (destObject->getArrayObject ("applicationPosts", i, &j1)) {
				j2 = j1.copy ();
				SystemInterface::transformInputFields_GetApplicationNewsResultPost (j2);
				items.push_back (j2);
			}
		}
		destObject->set ("applicationPosts", &items);
	}
	if (destObject->isString ("applicationUpdateTime")) {
		destObject->set ("applicationUpdateTime", destObject->getString ("applicationUpdateTime", "").parsedFloat ((double) 0.0f));
	}
}
void SystemInterface::transformOutputFields_GetApplicationNewsResult (Json *destObject) {
	Json j1, *j2;
	JsonList items;
	int i, len;
	
	len = destObject->getArrayLength ("applicationPosts");
	if (len > 0) {
		items.clear ();
		for (i = 0; i < len; ++i) {
			if (destObject->getArrayObject ("applicationPosts", i, &j1)) {
				j2 = j1.copy ();
				SystemInterface::transformOutputFields_GetApplicationNewsResultPost (j2);
				items.push_back (j2);
			}
		}
		destObject->set ("applicationPosts", &items);
	}
}
void SystemInterface::hashFields_GetApplicationNewsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	Json j1;
	StdString s;
	int i, len;
	
	len = commandParams->getArrayLength ("applicationPosts");
	for (i = 0; i < len; ++i) {
		if (commandParams->getArrayObject ("applicationPosts", i, &j1)) {
			SystemInterface::hashFields_GetApplicationNewsResultPost (&j1, hashUpdateFn, hashContextPtr);
		}
	}
	s = commandParams->getString ("applicationUpdateId", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	if (commandParams->isNumber ("applicationUpdateTime")) {
		s.sprintf ("%lli", (long long int) commandParams->getNumber ("applicationUpdateTime", (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
}
void SystemInterface::populateDefaultFields_GetApplicationNewsResultPost (Json *destObject) {
}
void SystemInterface::transformInputFields_GetApplicationNewsResultPost (Json *destObject) {
	if (destObject->isString ("endTime")) {
		destObject->set ("endTime", destObject->getString ("endTime", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("publishTime")) {
		destObject->set ("publishTime", destObject->getString ("publishTime", "").parsedFloat ((double) 0.0f));
	}
}
void SystemInterface::transformOutputFields_GetApplicationNewsResultPost (Json *destObject) {
}
void SystemInterface::hashFields_GetApplicationNewsResultPost (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	
	s = commandParams->getString ("body", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	if (commandParams->isNumber ("endTime")) {
		s.sprintf ("%lli", (long long int) commandParams->getNumber ("endTime", (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("publishTime", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}
void SystemInterface::populateDefaultFields_MediaItem (Json *destObject) {
	if (! destObject->exists ("hasAudioAlbumArt")) {
		destObject->set ("hasAudioAlbumArt", false);
	}
	if (! destObject->exists ("isAudio")) {
		destObject->set ("isAudio", false);
	}
	if (! destObject->exists ("isVideo")) {
		destObject->set ("isVideo", false);
	}
	if (! destObject->exists ("mtime")) {
		destObject->set ("mtime", 0);
	}
}
void SystemInterface::transformInputFields_MediaItem (Json *destObject) {
	StdString s;
	int i, len;
	std::list<double> n;
	
	if (destObject->isString ("audioBitrate")) {
		destObject->set ("audioBitrate", destObject->getString ("audioBitrate", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("audioChannels")) {
		destObject->set ("audioChannels", destObject->getString ("audioChannels", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("audioSampleRate")) {
		destObject->set ("audioSampleRate", destObject->getString ("audioSampleRate", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("duration")) {
		destObject->set ("duration", destObject->getString ("duration", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("frameRate")) {
		destObject->set ("frameRate", destObject->getString ("frameRate", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("hasAudioAlbumArt")) {
		s = destObject->getString ("hasAudioAlbumArt", "");
		if (s.equals ("true")) {
			destObject->set ("hasAudioAlbumArt", true);
		}
		else if (s.equals ("false")) {
			destObject->set ("hasAudioAlbumArt", false);
		}
	}
	if (destObject->isString ("height")) {
		destObject->set ("height", destObject->getString ("height", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("isAudio")) {
		s = destObject->getString ("isAudio", "");
		if (s.equals ("true")) {
			destObject->set ("isAudio", true);
		}
		else if (s.equals ("false")) {
			destObject->set ("isAudio", false);
		}
	}
	if (destObject->isString ("isVideo")) {
		s = destObject->getString ("isVideo", "");
		if (s.equals ("true")) {
			destObject->set ("isVideo", true);
		}
		else if (s.equals ("false")) {
			destObject->set ("isVideo", false);
		}
	}
	if (destObject->isString ("mediaFileSize")) {
		destObject->set ("mediaFileSize", destObject->getString ("mediaFileSize", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("mtime")) {
		destObject->set ("mtime", destObject->getString ("mtime", "").parsedFloat ((double) 0.0f));
	}
	len = destObject->getArrayLength ("thumbnailTimestamps");
	if (len > 0) {
		n.clear ();
		for (i = 0; i < len; ++i) {
			s = destObject->getArrayString ("thumbnailTimestamps", i, "");
			if (! s.empty ()) {
				n.push_back (s.parsedFloat ((double) 0.0f));
			}
			else {
				n.push_back (destObject->getArrayNumber ("thumbnailTimestamps", i, (double) 0.0f));
			}
		}
		destObject->set ("thumbnailTimestamps", n);
	}
	if (destObject->isString ("totalBitrate")) {
		destObject->set ("totalBitrate", destObject->getString ("totalBitrate", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("videoBitrate")) {
		destObject->set ("videoBitrate", destObject->getString ("videoBitrate", "").parsedFloat ((double) 0.0f));
	}
	if (destObject->isString ("width")) {
		destObject->set ("width", destObject->getString ("width", "").parsedFloat ((double) 0.0f));
	}
}
void SystemInterface::transformOutputFields_MediaItem (Json *destObject) {
}
void SystemInterface::hashFields_MediaItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	StdString s;
	int i, len;
	
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("audioBitrate", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("audioChannels", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("audioSampleRate", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("duration", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("frameRate", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%s", commandParams->getBoolean ("hasAudioAlbumArt", false) ? "true" : "false");
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s = commandParams->getString ("id", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%s", commandParams->getBoolean ("isAudio", false) ? "true" : "false");
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%s", commandParams->getBoolean ("isVideo", false) ? "true" : "false");
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s = commandParams->getString ("mediaDirname", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("mediaFileSize", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s = commandParams->getString ("mediaPath", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("mtime", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s = commandParams->getString ("name", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s = commandParams->getString ("sortKey", "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	len = commandParams->getArrayLength ("tags");
	for (i = 0; i < len; ++i) {
		s = commandParams->getArrayString ("tags", i, StdString ());
		if (! s.empty ()) {
			hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
		}
	}
	len = commandParams->getArrayLength ("thumbnailTimestamps");
	for (i = 0; i < len; ++i) {
		s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("thumbnailTimestamps", i, (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("totalBitrate", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("videoBitrate", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
	hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}
Json *SystemInterface::createCommand (const SystemInterface::Prefix &prefix, const char *commandName, Json *commandParams) {
	Json *cmd, *cmdprefix;
	SystemInterface::Command command;

	if (! commandParams) {
		commandParams = new Json ();
		commandParams->setEmpty ();
	}
	if (! getCommand (commandName, &command)) {
		lastError.sprintf ("Unknown command name \"%s\"", commandName);
		delete (commandParams);
		return (NULL);
	}
	if (! populateDefaultFields (command.paramType, commandParams)) {
		lastError.sprintf ("Command \"%s\" failed to populate fields for param type \"%s\"", commandName, command.paramType.c_str ());
		delete (commandParams);
		return (NULL);
	}
	transformOutputFields (command.paramType, commandParams);
	cmd = new Json ();
	cmd->set ("command", command.id);
	cmd->set ("commandName", commandName);

	cmdprefix = new Json ();
	if (prefix.createTime > 0) {
		cmdprefix->set (SystemInterface::Constant_CreateTimePrefixField, prefix.createTime);
	}
	if (! prefix.agentId.empty ()) {
		cmdprefix->set (SystemInterface::Constant_AgentIdPrefixField, prefix.agentId);
	}
	if (! prefix.userId.empty ()) {
		cmdprefix->set (SystemInterface::Constant_UserIdPrefixField, prefix.userId);
	}
	if (prefix.priority > 0) {
		cmdprefix->set (SystemInterface::Constant_PriorityPrefixField, prefix.priority);
	}
	if (prefix.startTime > 0) {
		cmdprefix->set (SystemInterface::Constant_StartTimePrefixField, prefix.startTime);
	}
	if (prefix.duration > 0) {
		cmdprefix->set (SystemInterface::Constant_DurationPrefixField, prefix.duration);
	}
	cmd->set ("prefix", cmdprefix);

	cmd->set ("params", commandParams);

	return (cmd);
}

Json *SystemInterface::createCommand (const SystemInterface::Prefix &prefix, int commandId, Json *commandParams) {
	std::map<int, StdString>::iterator i;

	i = commandIdMap.find (commandId);
	if (i == commandIdMap.end ()) {
		lastError.sprintf ("Unknown command ID %i", commandId);
		return (NULL);
	}
	return (createCommand (prefix, i->second.c_str (), commandParams));
}

bool SystemInterface::setCommandAuthorization (Json *command, const StdString &authSecret, const StdString &authToken) {
	StdString hash;
	Json prefix;
	bool result;

	result = false;
	hash = getCommandAuthorizationHash (command, authSecret, authToken);
	if (! hash.empty ()) {
		if (command->getObject ("prefix", &prefix)) {
			prefix.set (SystemInterface::Constant_AuthorizationHashPrefixField, hash);
			if ((! authToken.empty ()) && (! prefix.exists (SystemInterface::Constant_AuthorizationTokenPrefixField))) {
				prefix.set (SystemInterface::Constant_AuthorizationTokenPrefixField, authToken);
			}
			result = true;
		}
	}
	return (result);
}

StdString SystemInterface::getCommandAuthorizationHash (Json *command, const StdString &authSecret, const StdString &authToken) {
	SystemInterface::Command cmd;
	StdString token, cmdname, s;
	Json prefix, params;
	void *hashcontext;

	cmdname = command->getString ("commandName", "");
	if (! getCommand (cmdname, &cmd)) {
		return (StdString ());
	}
	if (! command->getObject ("prefix", &prefix)) {
		return (StdString ());
	}

	if (! authToken.empty ()) {
		token.assign (authToken);
	}
	else {
		token = prefix.getString (SystemInterface::Constant_AuthorizationTokenPrefixField, "");
	}

	hashcontext = hashCreate ();
	hashUpdate (hashcontext, (unsigned char *) authSecret.c_str (), authSecret.length ());
	hashUpdate (hashcontext, (unsigned char *) token.c_str (), token.length ());
	hashUpdate (hashcontext, (unsigned char *) cmdname.c_str (), cmdname.length ());

	if (prefix.isNumber (SystemInterface::Constant_CreateTimePrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_CreateTimePrefixField, (int64_t) 0));
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	s = prefix.getString (SystemInterface::Constant_AgentIdPrefixField, "");
	if (! s.empty ()) {
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	s = prefix.getString (SystemInterface::Constant_UserIdPrefixField, "");
	if (! s.empty ()) {
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_PriorityPrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_PriorityPrefixField, (int64_t) 0));
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_StartTimePrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_StartTimePrefixField, (int64_t) 0));
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_DurationPrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_DurationPrefixField, (int64_t) 0));
		hashUpdate (hashcontext, (unsigned char *) s.c_str (), s.length ());
	}

	if (command->getObject ("params", &params)) {
		hashFields (cmd.paramType, &params, hashcontext);
	}
	return (hashDigest (hashcontext));
}

bool SystemInterface::getCommand (const StdString &name, SystemInterface::Command *command) {
	std::map<StdString, SystemInterface::Command>::iterator i;

	i = commandMap.find (name);
	if (i == commandMap.end ()) {
		return (false);
	}
	*command = i->second;
	return (true);
}

bool SystemInterface::populateDefaultFields (const StdString &typeName, Json *destObject) {
	std::map<StdString, SystemInterface::PopulateDefaultFieldsFunction>::iterator i;

	i = populateDefaultFieldsMap.find (typeName);
	if (i == populateDefaultFieldsMap.end ()) {
		return (false);
	}
	i->second (destObject);
	return (true);
}

bool SystemInterface::transformInputFields (const StdString &typeName, Json *destObject) {
	std::map<StdString, SystemInterface::TransformInputFieldsFunction>::iterator i;

	i = transformInputFieldsMap.find (typeName);
	if (i == transformInputFieldsMap.end ()) {
		return (false);
	}
	i->second (destObject);
	return (true);
}

bool SystemInterface::transformOutputFields (const StdString &typeName, Json *destObject) {
	std::map<StdString, SystemInterface::TransformOutputFieldsFunction>::iterator i;

	i = transformOutputFieldsMap.find (typeName);
	if (i == transformOutputFieldsMap.end ()) {
		return (false);
	}
	i->second (destObject);
	return (true);
}

void SystemInterface::hashFields (const StdString &typeName, Json *commandParams, void *hashContextPtr) {
	std::map<StdString, SystemInterface::HashFieldsFunction>::iterator i;

	i = hashFieldsMap.find (typeName);
	if (i == hashFieldsMap.end ()) {
		return;
	}
	i->second (commandParams, hashUpdate, hashContextPtr);
}

bool SystemInterface::parseCommand (const StdString &commandString, Json **commandJson) {
	SystemInterface::Command command;
	Json parsedjson, parsedparams, *json, *params;
	StdString cmdname;

	if (! parsedjson.parse (commandString.c_str (), commandString.length ())) {
		lastError.assign ("JSON parse failed");
		return (false);
	}
	cmdname = parsedjson.getString ("commandName", "");
	if (! getCommand (cmdname, &command)) {
		lastError.sprintf ("Unknown command name \"%s\"", cmdname.c_str ());
		return (false);
	}
	if (! parsedjson.getObject ("params", &parsedparams)) {
		lastError.assign ("Missing params object");
		return (false);
	}
	params = parsedparams.copy ();
	transformInputFields (command.paramType, params);
	populateDefaultFields (command.paramType, params);
	json = parsedjson.copy ()->set ("params", params);
	if (commandJson) {
		*commandJson = json;
	}
	else {
		delete (json);
	}
	return (true);
}

int SystemInterface::getCommandId (Json *command) {
	if (! command) {
		return (-1);
	}
	return (command->getNumber ("command", -1));
}

StdString SystemInterface::getCommandName (Json *command) {
	if (! command) {
		return (StdString ());
	}
	return (command->getString ("commandName", ""));
}

StdString SystemInterface::getCommandAgentId (Json *command) {
	Json prefix;

	if ((! command) || (! command->getObject ("prefix", &prefix))) {
		return (StdString ());
	}
	return (prefix.getString (SystemInterface::Constant_AgentIdPrefixField, ""));
}

StdString SystemInterface::getCommandRecordId (Json *command) {
	return (getCommandStringParam (command, "id", ""));
}

bool SystemInterface::isWindowsPlatform (const StdString &platform) {
	return (platform.startsWith ("windows") || platform.equals ("win32") || platform.equals ("win64"));
}

SystemInterface::Prefix SystemInterface::getCommandPrefix (Json *command) {
	SystemInterface::Prefix result;
	Json prefix;

	if ((! command) || (! command->getObject ("prefix", &prefix))) {
		return (result);
	}
	result.agentId = prefix.getString (SystemInterface::Constant_AgentIdPrefixField, "");
	result.userId = prefix.getString (SystemInterface::Constant_UserIdPrefixField, "");
	result.priority = prefix.getNumber (SystemInterface::Constant_PriorityPrefixField, (int) 0);
	result.createTime = prefix.getNumber (SystemInterface::Constant_CreateTimePrefixField, (int64_t) 0);
	result.startTime = prefix.getNumber (SystemInterface::Constant_StartTimePrefixField, (int64_t) 0);
	result.duration = prefix.getNumber (SystemInterface::Constant_DurationPrefixField, (int64_t) 0);
	return (result);
}

bool SystemInterface::getCommandParams (Json *command, Json *params) {
	if (! command) {
		return (false);
	}
	return (command->getObject ("params", params));
}

StdString SystemInterface::getCommandStringParam (Json *command, const StdString &paramName, const StdString &defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getString (paramName, defaultValue));
}
StdString SystemInterface::getCommandStringParam (Json *command, const char *paramName, const char *defaultValue) {
	return (getCommandStringParam (command, StdString (paramName), StdString (defaultValue)));
}

bool SystemInterface::getCommandBooleanParam (Json *command, const StdString &paramName, bool defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getBoolean (paramName, defaultValue));
}
bool SystemInterface::getCommandBooleanParam (Json *command, const char *paramName, bool defaultValue) {
	return (getCommandBooleanParam (command, StdString (paramName), defaultValue));
}

int SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const int defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}
int SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const int defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

int64_t SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const int64_t defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}
int64_t SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const int64_t defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

float SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const float defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}
float SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const float defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

double SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const double defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}
double SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const double defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

bool SystemInterface::getCommandObjectParam (Json *command, const StdString &paramName, Json *destJson) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	return (params.getObject (paramName, destJson));
}
bool SystemInterface::getCommandObjectParam (Json *command, const char *paramName, Json *destJson) {
	return (getCommandObjectParam (command, StdString (paramName), destJson));
}

int SystemInterface::getCommandArrayLength (Json *command, const StdString &paramName) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (0);
	}
	return (params.getArrayLength (paramName));
}
int SystemInterface::getCommandArrayLength (Json *command, const char *paramName) {
	return (getCommandArrayLength (command, StdString (paramName)));
}

int SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

int64_t SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int64_t defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

double SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, double defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

StdString SystemInterface::getCommandStringArrayItem (Json *command, const StdString &paramName, int index, const StdString &defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getArrayString (paramName, index, defaultValue));
}
StdString SystemInterface::getCommandStringArrayItem (Json *command, const char *paramName, int index, const StdString &defaultValue) {
	return (getCommandStringArrayItem (command, StdString (paramName), index, defaultValue));
}

bool SystemInterface::getCommandObjectArrayItem (Json *command, const StdString &paramName, int index, Json *destJson) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	return (params.getArrayObject (paramName, index, destJson));
}
bool SystemInterface::getCommandObjectArrayItem (Json *command, const char *paramName, int index, Json *destJson) {
	return (getCommandObjectArrayItem (command, StdString (paramName), index, destJson));
}

float SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, float defaultValue) {
	Json params;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (defaultValue);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int> *destList, int defaultValue, bool shouldClear) {
	Json params;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, defaultValue));
	}
	return (true);
}
bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int> *destList, int defaultValue, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, defaultValue, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int64_t> *destList, int64_t defaultValue, bool shouldClear) {
	Json params;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, defaultValue));
	}
	return (true);
}
bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int64_t> *destList, int64_t defaultValue, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, defaultValue, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<float> *destList, float defaultValue, bool shouldClear) {
	Json params;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, defaultValue));
	}
	return (true);
}
bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<float> *destList, float defaultValue, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, defaultValue, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, double defaultValue, bool shouldClear) {
	Json params;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, defaultValue));
	}
	return (true);
}
bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, double defaultValue, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, defaultValue, shouldClear));
}

bool SystemInterface::getCommandStringArrayParam (Json *command, const StdString &paramName, std::list<StdString> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayString (paramName, i, StdString ()));
	}
	return (true);
}

bool SystemInterface::getCommandObjectArrayParam (Json *command, const StdString &paramName, JsonList *destList, bool shouldClear) {
	Json params;
	Json *item;
	int i, len;

	if ((! command) || (! command->getObject ("params", &params))) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}
	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		item = new Json ();
		if (! params.getArrayObject (paramName, i, item)) {
			item->setEmpty ();
		}
		destList->push_back (item);
	}
	return (true);
}

void *SystemInterface::defaultHashCreate ()  {
	return (NULL);
}

void SystemInterface::defaultHashUpdate (void *contextPtr, unsigned char *data, int dataLength) {
}

StdString SystemInterface::defaultHashDigest (void *contextPtr) {
	return (StdString ());
}
