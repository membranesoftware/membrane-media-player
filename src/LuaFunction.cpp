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
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "App.h"
#include "OsUtil.h"
#include "UiText.h"
#include "LuaScript.h"
#include "LuaFunction.h"

LuaFunction::LuaFunction ()
: fn (NULL)
{
}
LuaFunction::~LuaFunction () {
}

StdString LuaFunction::getHelpText () {
	return (StdString ());
}

void LuaFunction::argvInteger (lua_State *L, int position, int *value) {
	int argc;
	char buf[512];

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextId::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	if (value) {
		*value = lua_tointeger (L, position);
	}
}

void LuaFunction::argvString (lua_State *L, int position, char **value, bool allowEmptyValue) {
	int argc;
	char buf[512], *s;

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextId::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	s = (char *) lua_tostring (L, position);
	if (! allowEmptyValue) {
		if ((! s) || (s[0] == '\0')) {
			snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextId::LuaScriptEmptyStringArgumentErrorText).c_str ());
			luaL_error (L, "%s", buf);
			return;
		}
	}
	if (value) {
		*value = s;
	}
}

void LuaFunction::argvBoolean (lua_State *L, int position, bool *value) {
	int argc;
	char buf[512];

	argc = lua_gettop (L);
	if (argc < position) {
		snprintf (buf, sizeof (buf), "%s", UiText::instance->getText (UiTextId::LuaScriptMissingFunctionArgumentErrorText).c_str ());
		luaL_error (L, "%s", buf);
		return;
	}
	if (value) {
		*value = lua_toboolean (L, position);
	}
}

bool LuaFunction::awaitResult (LuaFunction::AwaitResultFn fn, void *fnData) {
	int64_t tmax;
	bool result;

	result = false;
	tmax = OsUtil::getTime () + LuaScript::scriptTimeout;
	while (! result) {
		result = fn (fnData);
		if (result) {
			return (true);
		}
		if (LuaScript::scriptTimeout <= 0) {
			break;
		}
		SDL_Delay (LuaScript::scriptWait);
		if (App::instance->isShutdown || App::instance->isShuttingDown) {
			return (true);
		}
		if (OsUtil::getTime () >= tmax) {
			break;
		}
	}
	return (false);
}

static bool awaitResult_updateTaskComplete (void *taskIdPtr) {
	return (!(App::instance->isTaskRunning (*(int64_t *) taskIdPtr)));
}
bool LuaFunction::awaitUpdateTaskComplete (int64_t taskId) {
	if (taskId <= 0) {
		return (true);
	}
	return (LuaFunction::awaitResult (awaitResult_updateTaskComplete, &taskId));
}
