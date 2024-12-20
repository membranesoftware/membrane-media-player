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
// Class that manages media resources loaded from assets
#ifndef RESOURCE_H
#define RESOURCE_H

#include "ft2build.h"
#include FT_FREETYPE_H
#include "Buffer.h"
#include "Font.h"

class Resource {
public:
	Resource ();
	~Resource ();
	static Resource *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	// Read-only data members
	StdString dataPath;
	StdString lastErrorMessage;

	// Set the source path that should be used for loading file assets. If the path ends in ".dat", it is opened as a bundle file; otherwise, the path is treated as a directory prefix for direct file access.
	void setSource (const StdString &path);

	// Prepare the Resource object to execute file operations. Returns a Result value.
	OpResult open ();

	// Close the resource object and free all assets
	void close ();

	// Free objects associated with resources that are no longer referenced
	void compact ();

	// Return a boolean value indicating whether a resource file exists at the specified path
	bool fileExists (const StdString &path);

	// Open resource data at the specified path and return the resulting SDL_RWops object, or NULL if the file could not be opened. The caller is responsible for closing the SDL_RWops object when it's no longer needed. If fileSize is non-NULL, its value is set to the size of the opened file.
	SDL_RWops *openFile (const StdString &path, uint64_t *fileSize = NULL);

	// Load file data from the specified resource path. Returns a pointer to the resulting Buffer object, or NULL if the file load failed. If a pointer is returned by this method, the referenced path must be unloaded with the unloadFile method when the Buffer is no longer needed.
	Buffer *loadFile (const StdString &path);

	// Unload previously acquired file resources from the specified path
	void unloadFile (const StdString &path);

	// Load an SDL_surface asset from an image file at the specified resource path. Returns a pointer to the resulting SDL_surface, or NULL if the surface could not be loaded. If an SDL_surface is returned by this method, the caller is responsible for freeing it with SDL_FreeSurface when it's no longer needed.
	SDL_Surface *loadSurface (const StdString &path);

	// Load an SDL_Texture asset from an image file at the specified resource path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be loaded. If refcountOnly is true, return a texture only from any previous successful load. Otherwise, this method must be invoked only from the application's main thread.
	SDL_Texture *loadTexture (const StdString &path, bool refcountOnly = false);

	// Create a texture from a surface and associate it with a path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be created. The surface object is not modified or freed by this method. This method must be invoked only from the application's main thread.
	SDL_Texture *createTexture (const StdString &path, SDL_Surface *surface);

	// Create a render target texture of the specified size and associate it with a path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be created. This method must be invoked only from the application's main thread.
	SDL_Texture *createTexture (const StdString &path, int textureWidth, int textureHeight, bool isStreamingTextureAccess = false);

	// Unload previously acquired texture resources from the specified path
	void unloadTexture (const StdString &path);

	// Load a Font asset from a ttf file at the specified resource path. Returns a pointer to the resulting Font, or NULL if the font could not be loaded. This method must be invoked only from the application's main thread.
	Font *loadFont (const StdString &path, int pointSize);

	// Unload previously acquired font resources for the specified path and point size
	void unloadFont (const StdString &path, int pointSize);

	// Read a value from an SDL_RWops object and store it in the provided pointer. Returns a Result value.
	static OpResult readUint64 (SDL_RWops *src, Uint64 *value);

	// Interface functions for use in an SDL_RWops struct
	static Sint64 rwopsSize (SDL_RWops *rw);
	static Sint64 rwopsSeek (SDL_RWops *rw, Sint64 offset, int whence);
	static size_t rwopsRead (SDL_RWops *rw, void *ptr, size_t size, size_t maxnum);
	static size_t rwopsWrite (SDL_RWops *rw, const void *ptr, size_t size, size_t num);
	static int rwopsClose (SDL_RWops *rw);

private:
	struct ArchiveEntry {
		uint64_t position;
		uint64_t length;
	};
	struct FileData {
		Buffer *data;
		int refcount;
	};
	struct TextureData {
		SDL_Texture *texture;
		int refcount;
	};
	struct FontData {
		Font *font;
		int refcount;
	};

	FT_Library freetype;
	bool isBundleFile;
	bool isOpen;

	// A map of resource paths to FileData objects
	std::map<StdString, Resource::FileData> fileMap;
	std::vector<StdString> fileCompactList;
	SDL_mutex *fileMapMutex;

	// A map of resource paths to TextureData objects
	std::map<StdString, Resource::TextureData> textureMap;
	std::vector<StdString> textureCompactList;
	SDL_mutex *textureMapMutex;

	// A map of font keys to FontData objects
	std::map<StdString, Resource::FontData> fontMap;
	std::vector<StdString> fontCompactList;
	SDL_mutex *fontMapMutex;

	// A map of entry ID values to ArchiveEntry structs
	std::map<uint64_t, Resource::ArchiveEntry> archiveEntryMap;

	// Clear the file map
	void clearFileMap ();

	// Clear the texture map
	void clearTextureMap ();

	// Clear the font map
	void clearFontMap ();

	// Remove unreferenced items from the file map
	void compactFileMap ();

	// Remove unreferenced items from the texture map
	void compactTextureMap ();

	// Remove unreferenced items from the font map
	void compactFontMap ();

	// Set failure state for a load operation
	void failLoad (const StdString &lastErrorMessageValue, const char *logErrorMessage = NULL);

	// Return the key that should be used to store the specified font in the font map
	static StdString getFontKey (const StdString &key, int pointSize);

	// Return an ID value computed from the provided path, using the djb2 algorithm
	static uint64_t getPathId (const StdString &path);
};
#endif
