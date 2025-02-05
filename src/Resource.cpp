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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "SDL2/SDL_image.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "App.h"
#include "SdlUtil.h"
#include "UiText.h"
#include "UiTextId.h"
#include "Font.h"
#include "Log.h"
#include "Resource.h"

Resource *Resource::instance = NULL;

Resource::Resource ()
: dataPath ("")
, freetype (NULL)
, isBundleFile (false)
, isOpen (false)
{
	SdlUtil::createMutex (&fileMapMutex);
	SdlUtil::createMutex (&textureMapMutex);
	SdlUtil::createMutex (&fontMapMutex);
}
Resource::~Resource () {
	close ();
	clearFontMap ();
	clearFileMap ();
	clearTextureMap ();
	SdlUtil::destroyMutex (&fileMapMutex);
	SdlUtil::destroyMutex (&textureMapMutex);
	SdlUtil::destroyMutex (&fontMapMutex);
}

void Resource::createInstance () {
	if (! Resource::instance) {
		Resource::instance = new Resource ();
	}
}
void Resource::freeInstance () {
	if (Resource::instance) {
		delete (Resource::instance);
		Resource::instance = NULL;
	}
}

void Resource::clearFileMap () {
	std::map<StdString, Resource::FileData>::iterator i1, i2;

	SDL_LockMutex (fileMapMutex);
	i1 = fileMap.begin ();
	i2 = fileMap.end ();
	while (i1 != i2) {
		if (i1->second.data) {
			delete (i1->second.data);
			i1->second.data = NULL;
		}
		++i1;
	}
	fileMap.clear ();
	SDL_UnlockMutex (fileMapMutex);
}

void Resource::clearTextureMap () {
	std::map<StdString, Resource::TextureData>::iterator i1, i2;

	SDL_LockMutex (textureMapMutex);
	i1 = textureMap.begin ();
	i2 = textureMap.end ();
	while (i1 != i2) {
		if (i1->second.texture) {
			SDL_DestroyTexture (i1->second.texture);
			i1->second.texture = NULL;
		}
		++i1;
	}
	textureMap.clear ();
	SDL_UnlockMutex (textureMapMutex);
}

void Resource::clearFontMap () {
	std::map<StdString, Resource::FontData>::iterator i1, i2;

	SDL_LockMutex (fontMapMutex);
	i1 = fontMap.begin ();
	i2 = fontMap.end ();
	while (i1 != i2) {
		if (i1->second.font) {
			delete (i1->second.font);
			i1->second.font = NULL;
		}
		++i1;
	}
	fontMap.clear ();
	SDL_UnlockMutex (fontMapMutex);
}

void Resource::setSource (const StdString &path) {
	dataPath.assign (path);
	isBundleFile = (dataPath.find (".dat") == (dataPath.length () - 4));
	Log::debug ("Set resource path; dataPath=\"%s\"", dataPath.c_str ());
}

OpResult Resource::open () {
	struct stat st;
	SDL_RWops *rw;
	Resource::ArchiveEntry ae;
	uint64_t id;
	OpResult result;

	if (FT_Init_FreeType (&(freetype))) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "Failed to initialize freetype library");
		return (OpResult::FreetypeOperationFailedError);
	}
	if (dataPath.empty ()) {
		return (OpResult::InvalidConfigurationError);
	}
	if (stat (dataPath.c_str (), &st) != 0) {
		failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("stat failed; path=\"%s\" err=\"%s\"", dataPath.c_str (), strerror (errno)).c_str ());
		return (OpResult::FileOperationFailedError);
	}

	if (! isBundleFile) {
		if (! S_ISDIR (st.st_mode)) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), "Invalid resource path");
			return (OpResult::MismatchedTypeError);
		}
		isOpen = true;
		return (OpResult::Success);
	}

	rw = SDL_RWFromFile (dataPath.c_str (), "r");
	if (! rw) {
		failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("Failed to open resource bundle file; path=\"%s\" error=\"%s\"", dataPath.c_str (), SDL_GetError ()).c_str ());
		return (OpResult::FileOperationFailedError);
	}

	result = OpResult::Success;
	archiveEntryMap.clear ();
	while (true) {
		result = Resource::readUint64 (rw, &id);
		if (result != OpResult::Success) {
			break;
		}
		if (id == 0) {
			break;
		}
		result = Resource::readUint64 (rw, &(ae.position));
		if (result != OpResult::Success) {
			break;
		}
		result = Resource::readUint64 (rw, &(ae.length));
		if (result != OpResult::Success) {
			break;
		}
		archiveEntryMap.insert (std::pair<uint64_t, Resource::ArchiveEntry> (id, ae));
	}

	SDL_RWclose (rw);
	if (result == OpResult::Success) {
		isOpen = true;
	}
	return (result);
}

void Resource::close () {
	if (! isOpen) {
		return;
	}
	clearFileMap ();
	clearTextureMap ();
	clearFontMap ();
	if (freetype) {
		FT_Done_FreeType (freetype);
		freetype = NULL;
	}
	isOpen = false;
}

void Resource::compact () {
	compactFontMap ();
	compactFileMap ();
	compactTextureMap ();
}

void Resource::compactFileMap () {
	std::vector<StdString>::iterator i1, i2;
	std::map<StdString, Resource::FileData>::iterator j;

	if (fileCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (fileMapMutex);
	i1 = fileCompactList.begin ();
	i2 = fileCompactList.end ();
	while (i1 != i2) {
		j = fileMap.find (*i1);
		if (j != fileMap.end ()) {
			if (j->second.refcount <= 0) {
				delete (j->second.data);
				j->second.data = NULL;
				fileMap.erase (j);
			}
		}
		++i1;
	}
	fileCompactList.clear ();
	SDL_UnlockMutex (fileMapMutex);
}

void Resource::compactTextureMap () {
	std::vector<StdString>::iterator i1, i2;
	std::map<StdString, Resource::TextureData>::iterator j;

	if (textureCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (textureMapMutex);
	i1 = textureCompactList.begin ();
	i2 = textureCompactList.end ();
	while (i1 != i2) {
		j = textureMap.find (*i1);
		if (j != textureMap.end ()) {
			if (j->second.refcount <= 0) {
				SDL_DestroyTexture (j->second.texture);
				j->second.texture = NULL;
				textureMap.erase (j);
			}
		}
		++i1;
	}
	textureCompactList.clear ();
	SDL_UnlockMutex (textureMapMutex);
}

void Resource::compactFontMap () {
	std::vector<StdString>::iterator i1, i2;
	std::map<StdString, Resource::FontData>::iterator j;

	if (fontCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (fontMapMutex);
	i1 = fontCompactList.begin ();
	i2 = fontCompactList.end ();
	while (i1 != i2) {
		j = fontMap.find (*i1);
		if (j != fontMap.end ()) {
			if (j->second.refcount <= 0) {
				delete (j->second.font);
				j->second.font = NULL;
				fontMap.erase (j);
			}
		}
		++i1;
	}
	fontCompactList.clear ();
	SDL_UnlockMutex (fontMapMutex);
}

bool Resource::fileExists (const StdString &path) {
	uint64_t id;
	std::map<uint64_t, Resource::ArchiveEntry>::iterator i;
	SDL_RWops *rw;
	StdString loadpath;
	bool exists;

	exists = false;
	if (isBundleFile) {
		id = Resource::getPathId (path);
		i = archiveEntryMap.find (id);
		if (i != archiveEntryMap.end ()) {
			exists = true;
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		rw = SDL_RWFromFile (loadpath.c_str (), "r");
		if (rw) {
			SDL_RWclose (rw);
			exists = true;
		}
	}
	return (exists);
}

void Resource::failLoad (const StdString &lastErrorMessageValue, const char *logErrorMessage) {
	lastErrorMessage.assign (lastErrorMessageValue);
	if (logErrorMessage) {
		Log::debug ("Failed to load resource: %s", logErrorMessage);
	}
}

SDL_RWops *Resource::openFile (const StdString &path, uint64_t *fileSize) {
	uint64_t id;
	std::map<uint64_t, Resource::ArchiveEntry>::iterator i;
	SDL_RWops *rw, *rwbundle;
	StdString loadpath;
	Sint64 pos;

	rw = NULL;
	if (isBundleFile) {
		id = Resource::getPathId (path);
		i = archiveEntryMap.find (id);
		if (i == archiveEntryMap.end ()) {
			failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("Failed to open file resource; path=\"%s\" error=\"Unknown path\"", path.c_str ()).c_str ());
			return (NULL);
		}
		rwbundle = SDL_RWFromFile (dataPath.c_str (), "r");
		if (! rwbundle) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("Failed to open file resource; path=\"%s\" error=\"bundle: %s\"", dataPath.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
		pos = SDL_RWseek (rwbundle, (Sint64) i->second.position, RW_SEEK_SET);
		if (pos < 0) {
			SDL_RWclose (rwbundle);
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("Failed to open file resource; path=\"%s\" error=\"seek: %s\"", dataPath.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
		rw = SDL_AllocRW ();
		if (! rw) {
			SDL_RWclose (rwbundle);
			failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("Failed to open file resource; path=\"%s\" error=\"bundle: %s\"", dataPath.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
		rw->type = SDL_RWOPS_UNKNOWN;
		rw->hidden.unknown.data1 = &(i->second);
		rw->hidden.unknown.data2 = rwbundle;
		rw->size = Resource::rwopsSize;
		rw->seek = Resource::rwopsSeek;
		rw->read = Resource::rwopsRead;
		rw->write = Resource::rwopsWrite;
		rw->close = Resource::rwopsClose;
		if (fileSize) {
			*fileSize = i->second.length;
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		rw = SDL_RWFromFile (loadpath.c_str (), "r");
		if (! rw) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("SDL_RWFromFile failed, path=\"%s\" error=\"%s\"", loadpath.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
		pos = SDL_RWsize (rw);
		if (pos < 0) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("SDL_RWsize failed, path=\"%s\" error=\"%s\"", loadpath.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
		if (fileSize) {
			*fileSize = (uint64_t) pos;
		}
	}
	return (rw);
}

Sint64 Resource::rwopsSize (SDL_RWops *rw) {
	Resource::ArchiveEntry *ae;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	return ((Sint64) ae->length);
}

Sint64 Resource::rwopsSeek (SDL_RWops *rw, Sint64 offset, int whence) {
	Resource::ArchiveEntry *ae;
	SDL_RWops *rwbundle;
	Sint64 result, pos;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	result = -1;
	switch (whence) {
		case RW_SEEK_SET: {
			if ((offset < 0) || (offset >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + offset, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
		case RW_SEEK_CUR: {
			pos = SDL_RWtell (rwbundle);
			if (pos < 0) {
				break;
			}
			if (offset == 0) {
				return (pos - (Sint64) ae->position);
			}
			pos += offset;
			pos -= (Sint64) ae->position;
			if ((pos < 0) || (pos >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + pos, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
		case RW_SEEK_END: {
			pos = ((Sint64) ae->length) - offset - 1;
			if ((pos < 0) || (pos >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + pos, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
	}
	return (result);
}

size_t Resource::rwopsRead (SDL_RWops *rw, void *ptr, size_t size, size_t maxnum) {
	Resource::ArchiveEntry *ae;
	SDL_RWops *rwbundle;
	uint64_t readbytes, filebytes;
	Sint64 pos;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	if ((size == 0) || (maxnum == 0)) {
		return (0);
	}
	pos = SDL_RWtell (rwbundle);
	filebytes = ae->length - (uint64_t) (pos - (Sint64) ae->position);
	readbytes = ((uint64_t) size) * maxnum;
	if (readbytes > filebytes) {
		maxnum = (filebytes / size);
	}
	return (SDL_RWread (rwbundle, ptr, size, maxnum));
}

size_t Resource::rwopsWrite (SDL_RWops *rw, const void *ptr, size_t size, size_t num) {
	// Write operations are not supported by this interface
	return (0);
}

int Resource::rwopsClose (SDL_RWops *rw) {
	SDL_RWops *rwbundle;
	int result;

	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	result = SDL_RWclose (rwbundle);
	if (result < 0) {
		return (result);
	}
	SDL_FreeRW (rw);
	return (0);
}

Buffer *Resource::loadFile (const StdString &path) {
	std::map<StdString, Resource::FileData>::iterator i;
	Resource::FileData data;
	Buffer *buffer;
	SDL_RWops *rw;
	uint64_t sz;
	uint8_t buf[8192];
	size_t len, rlen;

	buffer = NULL;
	SDL_LockMutex (fileMapMutex);
	i = fileMap.find (path);
	if (i != fileMap.end ()) {
		++(i->second.refcount);
		buffer = i->second.data;
	}
	SDL_UnlockMutex (fileMapMutex);
	if (buffer) {
		return (buffer);
	}
	rw = openFile (path, &sz);
	if (! rw) {
		return (NULL);
	}

	buffer = new Buffer ();
	while (sz > 0) {
		rlen = (size_t) sz;
		if (rlen > sizeof (buf)) {
			rlen = sizeof (buf);
		}
		len = SDL_RWread (rw, buf, 1, rlen);
		if (len <= 0) {
			break;
		}
		buffer->add (buf, len);
		sz -= len;
	}
	SDL_RWclose (rw);

	if (sz > 0) {
		failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("Failed to load file resource; path=\"%s\" error=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
		delete (buffer);
		return (NULL);
	}
	data.data = buffer;
	data.refcount = 1;
	SDL_LockMutex (fileMapMutex);
	fileMap.insert (std::pair<StdString, Resource::FileData> (path, data));
	SDL_UnlockMutex (fileMapMutex);
	return (buffer);
}

void Resource::unloadFile (const StdString &path) {
	std::map<StdString, Resource::FileData>::iterator i;

	SDL_LockMutex (fileMapMutex);
	i = fileMap.find (path);
	if (i != fileMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);
			if (i->second.refcount <= 0) {
				fileCompactList.push_back (i->first);
			}
		}
	}
	SDL_UnlockMutex (fileMapMutex);
}

SDL_Surface *Resource::loadSurface (const StdString &path) {
	StdString loadpath;
	SDL_RWops *rw;
	SDL_Surface *surface;

	surface = NULL;
	if (isBundleFile) {
		rw = openFile (path);
		if (! rw) {
			return (NULL);
		}
		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("bundle IMG_Load_RW failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		surface = IMG_Load (loadpath.c_str ());
		if (! surface) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("IMG_Load failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
	}
	return (surface);
}

SDL_Texture *Resource::loadTexture (const StdString &path, bool refcountOnly) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	StdString loadpath;
	SDL_RWops *rw;
	SDL_Surface *surface;
	SDL_Texture *texture;

	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}
	if (refcountOnly) {
		return (NULL);
	}

	surface = NULL;
	if (isBundleFile) {
		rw = openFile (path);
		if (! rw) {
			return (NULL);
		}
		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("bundle IMG_Load_RW failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		surface = IMG_Load (loadpath.c_str ());
		if (! surface) {
			failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("IMG_Load failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
			return (NULL);
		}
	}
	if (! surface) {
		return (NULL);
	}

	texture = SDL_CreateTextureFromSurface (App::instance->render, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
		return (NULL);
	}

	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);

	return (texture);
}

SDL_Texture *Resource::createTexture (const StdString &path, SDL_Surface *surface) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	SDL_Texture *texture;

	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}
	texture = SDL_CreateTextureFromSurface (App::instance->render, surface);
	if (! texture) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
		return (NULL);
	}
	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);
	return (texture);
}

SDL_Texture *Resource::createTexture (const StdString &path, int textureWidth, int textureHeight, bool isStreamingTextureAccess) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	SDL_Texture *texture;

	if ((textureWidth <= 0) || (textureHeight <= 0)) {
		return (NULL);
	}
	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}
	texture = SDL_CreateTexture (App::instance->render, SDL_PIXELFORMAT_RGBA32, isStreamingTextureAccess ? SDL_TEXTUREACCESS_STREAMING : SDL_TEXTUREACCESS_TARGET, textureWidth, textureHeight);
	if (! texture) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ()).c_str ());
		return (NULL);
	}

	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);

	return (texture);
}

void Resource::unloadTexture (const StdString &path) {
	std::map<StdString, Resource::TextureData>::iterator i;

	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);
			if (i->second.refcount <= 0) {
				textureCompactList.push_back (i->first);
			}
		}
	}
	SDL_UnlockMutex (textureMapMutex);
}

Font *Resource::loadFont (const StdString &path, int pointSize) {
	std::map<StdString, Resource::FontData>::iterator i;
	Resource::FontData data;
	StdString key;
	Buffer *buffer;
	Font *font;
	int result;

	font = NULL;
	key = Resource::getFontKey (path, pointSize);
	SDL_LockMutex (fontMapMutex);
	i = fontMap.find (key);
	if (i != fontMap.end ()) {
		++(i->second.refcount);
		font = i->second.font;
	}
	SDL_UnlockMutex (fontMapMutex);
	if (font) {
		return (font);
	}
	buffer = loadFile (path);
	if (! buffer) {
		return (NULL);
	}

	font = new Font (freetype, key);
	result = font->load (buffer, pointSize);
	if (result != OpResult::Success) {
		delete (font);
		unloadFile (path);
		failLoad (UiText::instance->getText (UiTextId::InvalidFileFormat).capitalized (), StdString::createSprintf ("Failed to load font resource; key=\"%s\" err=%i", key.c_str (), result).c_str ());
		return (NULL);
	}
	data.font = font;
	data.refcount = 1;
	SDL_LockMutex (fontMapMutex);
	fontMap.insert (std::pair<StdString, Resource::FontData> (key, data));
	SDL_UnlockMutex (fontMapMutex);
	return (font);
}

void Resource::unloadFont (const StdString &path, int pointSize) {
	std::map<StdString, Resource::FontData>::iterator i;
	StdString key;
	bool unloaded;

	unloaded = false;
	key = Resource::getFontKey (path, pointSize);
	SDL_LockMutex (fontMapMutex);
	i = fontMap.find (key);
	if (i != fontMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);
			if (i->second.refcount <= 0) {
				fontCompactList.push_back (i->first);
				unloaded = true;
			}
		}
	}
	SDL_UnlockMutex (fontMapMutex);
	if (unloaded) {
		unloadFile (path);
	}
}

StdString Resource::getFontKey (const StdString &key, int pointSize) {
	return (StdString::createSprintf ("%s:%i", key.c_str (), pointSize));
}

uint64_t Resource::getPathId (const StdString &path) {
	uint64_t id;
	char *s, c;

	id = 5381;
	s = (char *) path.c_str ();
	while (true) {
		c = *s;
		if (! c) {
			break;
		}
		id = ((id << 5) + id) + c;
		++s;
	}
	return (id);
}

OpResult Resource::readUint64 (SDL_RWops *src, Uint64 *value) {
	char buf[8];
	size_t rlen;
	Uint64 val;

	rlen = SDL_RWread (src, buf, 8, 1);
	if (rlen < 1) {
		return (OpResult::FileOperationFailedError);
	}
	val = 0;
	val |= (buf[0] & 0xFF);
	val <<= 8;
	val |= (buf[1] & 0xFF);
	val <<= 8;
	val |= (buf[2] & 0xFF);
	val <<= 8;
	val |= (buf[3] & 0xFF);
	val <<= 8;
	val |= (buf[4] & 0xFF);
	val <<= 8;
	val |= (buf[5] & 0xFF);
	val <<= 8;
	val |= (buf[6] & 0xFF);
	val <<= 8;
	val |= (buf[7] & 0xFF);

	if (value) {
		*value = val;
	}
	return (OpResult::Success);
}
