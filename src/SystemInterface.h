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
#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

#include <stdint.h>
#include <map>
#include <list>
#include <vector>
#include "StdString.h"
#include "Json.h"

class SystemInterface {
public:
	SystemInterface ();
	~SystemInterface ();
	static SystemInterface *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	struct Prefix {
		StdString agentId;
		StdString userId;
		int priority;
		int64_t createTime;
		int64_t startTime;
		int64_t duration;
		Prefix (): priority (0), createTime (0), startTime (0), duration (0) { }
	};

	struct Command {
		int id;
		StdString name;
		StdString paramType;
		Command (): id (0), name (""), paramType ("") { }
		Command (int id, const StdString &name, const StdString &paramType): id (id), name (name), paramType (paramType) { }
	};

	typedef void *(*HashCreateFunction) ();
	typedef void (*HashUpdateFunction) (void *contextPtr, unsigned char *data, int dataLength);
	typedef StdString (*HashDigestFunction) (void *contextPtr);
	typedef void (*PopulateDefaultFieldsFunction) (Json *destObject);
	typedef void (*TransformInputFieldsFunction) (Json *destObject);
	typedef void (*TransformOutputFieldsFunction) (Json *destObject);
	typedef void (*HashFieldsFunction) (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);

	// Read-write data members
	HashCreateFunction hashCreate;
	HashUpdateFunction hashUpdate;
	HashDigestFunction hashDigest;

	// Read-only data members
	StdString lastError;
	std::map<StdString, SystemInterface::Command> commandMap;
	std::map<int, StdString> commandIdMap;
	std::map<StdString, SystemInterface::PopulateDefaultFieldsFunction> populateDefaultFieldsMap;
	std::map<StdString, SystemInterface::TransformInputFieldsFunction> transformInputFieldsMap;
	std::map<StdString, SystemInterface::TransformOutputFieldsFunction> transformOutputFieldsMap;
	std::map<StdString, SystemInterface::HashFieldsFunction> hashFieldsMap;

	void populate ();
	void populate_commandMap ();
	void populate_populateDefaultFieldsMap ();
	void populate_transformInputFieldsMap ();
	void populate_transformOutputFieldsMap ();
	void populate_hashFieldsMap ();

	static constexpr const char *Command_CommandResult = "CommandResult";
	static constexpr const char *Command_Fixture1Record = "Fixture1Record";
	static constexpr const char *Command_Fixture2Record = "Fixture2Record";
	static constexpr const char *Command_GetApplicationNews = "GetApplicationNews";
	static constexpr const char *Command_GetApplicationNewsResult = "GetApplicationNewsResult";
	static constexpr const char *Command_MediaItem = "MediaItem";
	static constexpr const int CommandId_CommandResult = 0;
	static constexpr const int CommandId_Fixture1Record = 328;
	static constexpr const int CommandId_Fixture2Record = 329;
	static constexpr const int CommandId_GetApplicationNews = 318;
	static constexpr const int CommandId_GetApplicationNewsResult = 319;
	static constexpr const int CommandId_MediaItem = 16;
	static constexpr const char *Field_applicationPosts = "applicationPosts";
	static constexpr const char *Field_applicationUpdateId = "applicationUpdateId";
	static constexpr const char *Field_applicationUpdateTime = "applicationUpdateTime";
	static constexpr const char *Field_audioBitrate = "audioBitrate";
	static constexpr const char *Field_audioChannels = "audioChannels";
	static constexpr const char *Field_audioSampleRate = "audioSampleRate";
	static constexpr const char *Field_body = "body";
	static constexpr const char *Field_buildId = "buildId";
	static constexpr const char *Field_duration = "duration";
	static constexpr const char *Field_endTime = "endTime";
	static constexpr const char *Field_error = "error";
	static constexpr const char *Field_frameRate = "frameRate";
	static constexpr const char *Field_hasAudioAlbumArt = "hasAudioAlbumArt";
	static constexpr const char *Field_height = "height";
	static constexpr const char *Field_id = "id";
	static constexpr const char *Field_isAudio = "isAudio";
	static constexpr const char *Field_isVideo = "isVideo";
	static constexpr const char *Field_item = "item";
	static constexpr const char *Field_itemId = "itemId";
	static constexpr const char *Field_mediaDirname = "mediaDirname";
	static constexpr const char *Field_mediaFileSize = "mediaFileSize";
	static constexpr const char *Field_mediaPath = "mediaPath";
	static constexpr const char *Field_mtime = "mtime";
	static constexpr const char *Field_n1 = "n1";
	static constexpr const char *Field_name = "name";
	static constexpr const char *Field_publishTime = "publishTime";
	static constexpr const char *Field_s1 = "s1";
	static constexpr const char *Field_sortKey = "sortKey";
	static constexpr const char *Field_stringResult = "stringResult";
	static constexpr const char *Field_success = "success";
	static constexpr const char *Field_tags = "tags";
	static constexpr const char *Field_taskId = "taskId";
	static constexpr const char *Field_thumbnailTimestamps = "thumbnailTimestamps";
	static constexpr const char *Field_totalBitrate = "totalBitrate";
	static constexpr const char *Field_videoBitrate = "videoBitrate";
	static constexpr const char *Field_width = "width";
	static constexpr const char *Constant_AgentIdPrefixField = "b";
	static constexpr const char *Constant_AuthorizationHashPrefixField = "g";
	static constexpr const char *Constant_AuthorizationTokenPrefixField = "h";
	static constexpr const char *Constant_CommandPostFormKey = "json";
	static constexpr const char *Constant_CreateTimePrefixField = "a";
	static constexpr const char *Constant_DurationPrefixField = "f";
	static constexpr const int Constant_FilePathSort = 2;
	static constexpr const int Constant_NameSort = 0;
	static constexpr const int Constant_NewestSort = 1;
	static constexpr const char *Constant_PriorityPrefixField = "d";
	static constexpr const char *Constant_StartTimePrefixField = "e";
	static constexpr const char *Constant_UserIdPrefixField = "c";
	static void populateDefaultFields_CommandResult (Json *destObject);
	static void transformInputFields_CommandResult (Json *destObject);
	static void transformOutputFields_CommandResult (Json *destObject);
	static void hashFields_CommandResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_Fixture1Record (Json *destObject);
	static void transformInputFields_Fixture1Record (Json *destObject);
	static void transformOutputFields_Fixture1Record (Json *destObject);
	static void hashFields_Fixture1Record (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_Fixture2Record (Json *destObject);
	static void transformInputFields_Fixture2Record (Json *destObject);
	static void transformOutputFields_Fixture2Record (Json *destObject);
	static void hashFields_Fixture2Record (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_GetApplicationNews (Json *destObject);
	static void transformInputFields_GetApplicationNews (Json *destObject);
	static void transformOutputFields_GetApplicationNews (Json *destObject);
	static void hashFields_GetApplicationNews (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_GetApplicationNewsResult (Json *destObject);
	static void transformInputFields_GetApplicationNewsResult (Json *destObject);
	static void transformOutputFields_GetApplicationNewsResult (Json *destObject);
	static void hashFields_GetApplicationNewsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_GetApplicationNewsResultPost (Json *destObject);
	static void transformInputFields_GetApplicationNewsResultPost (Json *destObject);
	static void transformOutputFields_GetApplicationNewsResultPost (Json *destObject);
	static void hashFields_GetApplicationNewsResultPost (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	static void populateDefaultFields_MediaItem (Json *destObject);
	static void transformInputFields_MediaItem (Json *destObject);
	static void transformOutputFields_MediaItem (Json *destObject);
	static void hashFields_MediaItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
	// Default implementations of hash functions
	static void *defaultHashCreate ();
	static void defaultHashUpdate (void *contextPtr, unsigned char *data, int dataLength);
	static StdString defaultHashDigest (void *contextPtr);

	// Return a newly created Json object containing a command item, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the resulting command to contain empty parameter fields. If commandParams is not NULL, this method becomes responsible for freeing the object when it's no longer needed.
	Json *createCommand (const SystemInterface::Prefix &prefix, const char *commandName, Json *commandParams = NULL);
	Json *createCommand (const SystemInterface::Prefix &prefix, int commandId, Json *commandParams = NULL);

	// Populate a command's authorization prefix field using the provided values and hash functions. Returns a boolean value indicating if the field was successfully generated.
	bool setCommandAuthorization (Json *command, const StdString &authSecret, const StdString &authToken);

	// Return the authorization hash generated from the provided values and functions. If authToken is not provided, any available prefix auth token is used.
	StdString getCommandAuthorizationHash (Json *command, const StdString &authSecret, const StdString &authToken);

	// Find command data for the specified name and store fields into the provided struct. Returns a boolean value indicating if the command was found.
	bool getCommand (const StdString &name, SystemInterface::Command *command);

	// Populate default fields in an object, as appropriate for the specified type name. Returns a boolean value indicating if the type was found.
	bool populateDefaultFields (const StdString &typeName, Json *destObject);

	// Change parameter fields as needed after parsing a command of the specified type name. Returns a boolean value indicating if the type was found.
	bool transformInputFields (const StdString &typeName, Json *destObject);

	// Change parameter fields as needed before encoding a command of the specified type name. Returns a boolean value indicating if the type was found.
	bool transformOutputFields (const StdString &typeName, Json *destObject);

	// Update a hash digest using fields in an object, as appropriate for the specified type name
	void hashFields (const StdString &typeName, Json *commandParams, void *hashContextPtr);

	// Parse a command JSON string and store the resulting Json object using the provided pointer. Returns a boolean value indicating if the parse was successful. If the parse fails, this method sets the lastError value.
	bool parseCommand (const StdString &commandString, Json **commandJson);

	// Return the command ID value appearing in the provided command object, or -1 if no such ID was found
	int getCommandId (Json *command);

	// Return the command name value appearing in the provided command object, or an empty string if no such name was found
	StdString getCommandName (Json *command);

	// Return the prefix.agentId value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandAgentId (Json *command);

	// Return the params.id value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandRecordId (Json *command);

	// Return a boolean value indicating if the provided string matches a Windows platform identifier
	bool isWindowsPlatform (const StdString &platform);

	// Return a SystemInterface::Prefix structure containing prefix fields from the provided command
	SystemInterface::Prefix getCommandPrefix (Json *command);

	// Get the provided command's params object and store it using the provided Json pointer. Returns a boolean value indicating if the params object was found.
	bool getCommandParams (Json *command, Json *params);

	// Return a string value from params in the provided command, or the default value if the named field wasn't found
	StdString getCommandStringParam (Json *command, const StdString &paramName, const StdString &defaultValue);
	StdString getCommandStringParam (Json *command, const char *paramName, const char *defaultValue);

	// Return a bool value from params in the provided command, or the default value if the named field wasn't found
	bool getCommandBooleanParam (Json *command, const StdString &paramName, bool defaultValue);
	bool getCommandBooleanParam (Json *command, const char *paramName, bool defaultValue);

	// Return an int number value from params in the provided command, or the default value if the named field wasn't found
	int getCommandNumberParam (Json *command, const StdString &paramName, const int defaultValue);
	int getCommandNumberParam (Json *command, const char *paramName, const int defaultValue);
	int64_t getCommandNumberParam (Json *command, const StdString &paramName, const int64_t defaultValue);
	int64_t getCommandNumberParam (Json *command, const char *paramName, const int64_t defaultValue);
	float getCommandNumberParam (Json *command, const StdString &paramName, const float defaultValue);
	float getCommandNumberParam (Json *command, const char *paramName, const float defaultValue);
	double getCommandNumberParam (Json *command, const StdString &paramName, const double defaultValue);
	double getCommandNumberParam (Json *command, const char *paramName, const double defaultValue);

	// Find the specified object item and store it in the provided Json object. Returns a boolean value indicating if the item was found.
	bool getCommandObjectParam (Json *command, const StdString &paramName, Json *destJson);
	bool getCommandObjectParam (Json *command, const char *paramName, Json *destJson);

	// Return the length of the specified array, or 0 if the array was empty or non-existent
	int getCommandArrayLength (Json *command, const StdString &paramName);
	int getCommandArrayLength (Json *command, const char *paramName);

	// Return the specified number array item, or the default value if the item wasn't found
	int getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int defaultValue);
	int64_t getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int64_t defaultValue);
	double getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, double defaultValue);
	float getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, float defaultValue);

	// Return the specified string array item, or the default value if the item wasn't found
	StdString getCommandStringArrayItem (Json *command, const StdString &paramName, int index, const StdString &defaultValue);
	StdString getCommandStringArrayItem (Json *command, const char *paramName, int index, const StdString &defaultValue);

	// Find the specified object array item and store it in the provided Json object. Returns a boolean value indicating if the item was found.
	bool getCommandObjectArrayItem (Json *command, const StdString &paramName, int index, Json *destJson);
	bool getCommandObjectArrayItem (Json *command, const char *paramName, int index, Json *destJson);

	// Fill the provided vector with items from the specified number array, optionally clearing the list before doing so. Returns a boolean value indicating if the array was found. Non-number items in the targeted array are entered into destList as defaultValue.
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int> *destList, int defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int> *destList, int defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int64_t> *destList, int64_t defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int64_t> *destList, int64_t defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<float> *destList, float defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<float> *destList, float defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, double defaultValue, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, double defaultValue, bool shouldClear = false);

	// Fill the provided list with items from the specified string array, optionally clearing the list before doing so. Returns a boolean value indicating if the array was found. Non-string items in the targeted array are entered into destList as empty strings.
	bool getCommandStringArrayParam (Json *command, const StdString &paramName, std::list<StdString> *destList, bool shouldClear = false);

	// Fill the provided list with items from the specified object array, optionally clearing the list before doing so. Returns a boolean value indicating if the array was found. Non-object items in the targeted array are entered into destList as empty objects.
	bool getCommandObjectArrayParam (Json *command, const StdString &paramName, JsonList *destList, bool shouldClear = false);
};
#endif
