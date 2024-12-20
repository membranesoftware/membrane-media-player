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
#include "LuaFunction.h"
#include "LuaAwaitloadimagesFunction.h"
#include "LuaAwaitvalueFunction.h"
#include "LuaCapturescreenFunction.h"
#include "LuaCapturescreenpathFunction.h"
#include "LuaClickFunction.h"
#include "LuaDofileFunction.h"
#include "LuaGetenvFunction.h"
#include "LuaHelpFunction.h"
#include "LuaKeypressFunction.h"
#include "LuaMenuselectFunction.h"
#include "LuaMouseleftclickFunction.h"
#include "LuaMouseoverFunction.h"
#include "LuaMousepointerFunction.h"
#include "LuaMousewarpFunction.h"
#include "LuaOpenFunction.h"
#include "LuaPrintcontrolsFunction.h"
#include "LuaPrintFunction.h"
#include "LuaQuitFunction.h"
#include "LuaSetvalueFunction.h"
#include "LuaSleepFunction.h"
#include "LuaStartstreamFunction.h"
#include "LuaStopstreamFunction.h"
#include "LuaStreampathFunction.h"
#include "LuaTargetFunction.h"
#include "LuaTimeoutFunction.h"
#include "LuaUntargetFunction.h"
#include "LuaFunctionList.h"

LuaFunctionList *LuaFunctionList::instance = NULL;

static bool compareFunctions (LuaFunction *a, LuaFunction *b) {
	return (a->name.compare (b->name) < 0);
}

LuaFunctionList::LuaFunctionList ()
{
	push_back (new LuaAwaitloadimagesFunction ());
	push_back (new LuaAwaitvalueFunction ());
	push_back (new LuaCapturescreenFunction ());
	push_back (new LuaCapturescreenpathFunction ());
	push_back (new LuaClickFunction ());
	push_back (new LuaDofileFunction ());
	push_back (new LuaGetenvFunction ());
	push_back (new LuaHelpFunction ());
	push_back (new LuaKeypressFunction ());
	push_back (new LuaMenuselectFunction ());
	push_back (new LuaMouseleftclickFunction ());
	push_back (new LuaMouseoverFunction ());
	push_back (new LuaMousepointerFunction ());
	push_back (new LuaMousewarpFunction ());
	push_back (new LuaOpenFunction ());
	push_back (new LuaPrintcontrolsFunction ());
	push_back (new LuaPrintFunction ());
	push_back (new LuaQuitFunction ());
	push_back (new LuaSetvalueFunction ());
	push_back (new LuaSleepFunction ());
	push_back (new LuaStartstreamFunction ());
	push_back (new LuaStopstreamFunction ());
	push_back (new LuaStreampathFunction ());
	push_back (new LuaTargetFunction ());
	push_back (new LuaTimeoutFunction ());
	push_back (new LuaUntargetFunction ());
	sort (compareFunctions);
}
LuaFunctionList::~LuaFunctionList () {
	LuaFunctionList::iterator i1, i2;

	i1 = begin ();
	i2 = end ();
	while (i1 != i2) {
		delete (*i1);
		++i1;
	}
	clear ();
}

void LuaFunctionList::createInstance () {
	if (! LuaFunctionList::instance) {
		LuaFunctionList::instance = new LuaFunctionList ();
	}
}
void LuaFunctionList::freeInstance () {
	if (LuaFunctionList::instance) {
		delete (LuaFunctionList::instance);
		LuaFunctionList::instance = NULL;
	}
}
