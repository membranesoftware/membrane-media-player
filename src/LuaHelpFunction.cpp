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
#include "Log.h"
#include "OsUtil.h"
#include "UiText.h"
#include "LuaScript.h"
#include "LuaFunctionList.h"
#include "LuaHelpFunction.h"

static int exec (lua_State *L) {
	LuaFunctionList::const_iterator i1, i2;
	StdString format, s;
	int w, maxw;

	maxw = 0;
	i1 = LuaFunctionList::instance->cbegin ();
	i2 = LuaFunctionList::instance->cend ();
	while (i1 != i2) {
		w = (*i1)->name.length () + (*i1)->parameters.length ();
		if (w > maxw) {
			maxw = w;
		}
		++i1;
	}

	maxw += 4;
	format.sprintf ("%%-%is%%s", maxw);
	i1 = LuaFunctionList::instance->cbegin ();
	i2 = LuaFunctionList::instance->cend ();
	while (i1 != i2) {
		s.sprintf ("%s(%s)", (*i1)->name.c_str (), (*i1)->parameters.c_str ());
		Log::printf (format.c_str (), s.c_str (), (*i1)->getHelpText ().c_str ());
		++i1;
	}
	Log::printf (" ");
	if (LuaScript::scriptTimeout > 0) {
		Log::printf ("* %s: %ims", UiText::instance->getText (UiTextId::FunctionTimeout).capitalized ().c_str (), LuaScript::scriptTimeout);
	}
	else {
		Log::printf ("* %s: %s", UiText::instance->getText (UiTextId::FunctionTimeout).capitalized ().c_str (), UiText::instance->getText (UiTextId::Disabled).c_str ());
	}
	Log::printf ("* %s", UiText::instance->getText (UiTextId::EnvironmentVariables).capitalized ().c_str ());
	format.sprintf ("%%-%is%%s", maxw);
	Log::printf (format.c_str (), OsUtil::runScriptEnvKey, UiText::instance->getText (UiTextId::LuaScriptRunScriptHelpText).c_str ());
	Log::printf (format.c_str (), OsUtil::runFileEnvKey, UiText::instance->getText (UiTextId::LuaScriptRunFileHelpText).c_str ());
	return (0);
}

LuaHelpFunction::LuaHelpFunction ()
: LuaFunction ()
{
	name.assign ("help");
	fn = exec;
}
LuaHelpFunction::~LuaHelpFunction () {
}

StdString LuaHelpFunction::getHelpText () {
	return (UiText::instance->getText (UiTextId::LuaScriptHelpHelpText));
}
