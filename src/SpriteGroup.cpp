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
#include "Log.h"
#include "StringList.h"
#include "Resource.h"
#include "SpriteId.h"
#include "SpriteGroup.h"

SpriteGroup *SpriteGroup::instance = NULL;

SpriteGroup::SpriteGroup ()
: isLoaded (false)
{
}
SpriteGroup::~SpriteGroup () {
	clearSprites ();
}

void SpriteGroup::createInstance () {
	if (! SpriteGroup::instance) {
		SpriteGroup::instance = new SpriteGroup ();
	}
}
void SpriteGroup::freeInstance () {
	if (SpriteGroup::instance) {
		delete (SpriteGroup::instance);
		SpriteGroup::instance = NULL;
	}
}

void SpriteGroup::clearSprites () {
	std::map<StdString, Sprite *>::const_iterator i1, i2;
	Sprite *sprite;

	i1 = spriteMap.cbegin ();
	i2 = spriteMap.cend ();
	while (i1 != i2) {
		sprite = i1->second;
		sprite->unload ();
		delete (sprite);
		++i1;
	}
	spriteMap.clear ();
}

OpResult SpriteGroup::load (const StdString &spriteIdPrefix, const StdString &imagePrefix) {
	Sprite *sprite;
	StdString prefix;
	StringList ids;
	StringList::const_iterator i1, i2;
	OpResult result;

	if (isLoaded) {
		if (! loadSpriteIdPrefix.equals (spriteIdPrefix)) {
			return (OpResult::AlreadyLoadedError);
		}
		return (OpResult::Success);
	}
	ids = SpriteId::getSpriteIds (spriteIdPrefix);
	if (ids.empty ()) {
		return (OpResult::KeyNotFoundError);
	}

	prefix.assign (imagePrefix);
	if (prefix.empty ()) {
		prefix.assign (App::instance->imagePrefix);
	}
	result = OpResult::Success;
	i1 = ids.cbegin ();
	i2 = ids.cend ();
	while (i1 != i2) {
		sprite = new Sprite ();
		result = sprite->load (*i1, prefix);
		if (result != OpResult::Success) {
			delete (sprite);
			break;
		}
		spriteMap.insert (std::pair<StdString, Sprite *> (*i1, sprite));
		++i1;
	}

	if (result == OpResult::Success) {
		loadSpriteIdPrefix.assign (spriteIdPrefix);
		isLoaded = true;
	}
	else {
		clearSprites ();
	}
	return (result);
}

void SpriteGroup::unload () {
	if (! isLoaded) {
		return;
	}
	isLoaded = false;
	clearSprites ();
}

void SpriteGroup::resize (const StdString &imagePrefix) {
	std::map<StdString, Sprite *>::const_iterator i1, i2;
	StdString prefix;
	Sprite *sprite;
	int result;

	prefix.assign (imagePrefix);
	if (prefix.empty ()) {
		prefix.assign (App::instance->imagePrefix);
	}
	i1 = spriteMap.cbegin ();
	i2 = spriteMap.cend ();
	while (i1 != i2) {
		sprite = i1->second;
		if (Resource::instance->fileExists (StdString::createSprintf ("%s/%s/000.png", i1->first.c_str (), prefix.c_str ()))) {
			sprite->unload ();
			result = sprite->load (i1->first, prefix);
			if (result != OpResult::Success) {
				Log::err ("Failed to reload textures; path=\"%s\" err=%i", i1->first.c_str (), result);
			}
		}
		++i1;
	}
}

Sprite *SpriteGroup::getSprite (const char *spriteId) {
	std::map<StdString, Sprite *>::iterator pos;

	if (! isLoaded) {
		return (NULL);
	}
	pos = spriteMap.find (StdString (spriteId));
	return ((pos != spriteMap.end ()) ? pos->second : NULL);
}
