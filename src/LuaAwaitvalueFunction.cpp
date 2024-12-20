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
#include "UiText.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "IconLabelWindow.h"
#include "ProgressBar.h"
#include "ProgressRing.h"
#include "Button.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "ImageWindow.h"
#include "LuaAwaitvalueFunction.h"

static bool awaitResult_exec (void *data) {
	char **args, *targetname, *expectedtext;
	Widget *target;
	Label *label;
	LabelWindow *labelwindow;
	IconLabelWindow *iconlabelwindow;
	ProgressBar *progressbar;
	ProgressRing *progressring;
	Button *button;
	Toggle *toggle;
	ToggleWindow *togglewindow;
	ImageWindow *imagewindow;
	bool result, found;

	if (! App::instance->rootPanel) {
		return (false);
	}
	args = (char **) data;
	targetname = args[0];
	expectedtext = args[1];
	target = App::instance->rootPanel->findWidget (StdString (targetname), true);
	if (! target) {
		return (false);
	}
	found = false;
	result = false;
	if (target->isDestroyed || (! target->isVisible)) {
		found = true;
	}
	if (! found) {
		label = Label::castWidget (target);
		if (label) {
			found = true;
			result = label->text.equals (expectedtext);
		}
	}
	if (! found) {
		labelwindow = LabelWindow::castWidget (target);
		if (labelwindow) {
			found = true;
			result = labelwindow->getText ().equals (expectedtext);
		}
	}
	if (! found) {
		iconlabelwindow = IconLabelWindow::castWidget (target);
		if (iconlabelwindow) {
			found = true;
			result = iconlabelwindow->getText ().equals (expectedtext);
		}
	}
	if (! found) {
		progressbar = ProgressBar::castWidget (target);
		if (progressbar) {
			found = true;
			result = (progressbar->progressValue >= StdString (expectedtext).parsedFloat (0.0f));
		}
	}
	if (! found) {
		progressring = ProgressRing::castWidget (target);
		if (progressring) {
			found = true;
			result = (progressring->progressValue >= StdString (expectedtext).parsedFloat (0.0f));
		}
	}
	if (! found) {
		button = Button::castWidget (target);
		if (button) {
			found = true;
			result = true;
		}
	}
	if (! found) {
		toggle = Toggle::castWidget (target);
		if (toggle) {
			found = true;
			result = true;
		}
	}
	if (! found) {
		togglewindow = ToggleWindow::castWidget (target);
		if (togglewindow) {
			found = true;
			result = true;
		}
	}
	if (! found) {
		imagewindow = ImageWindow::castWidget (target);
		if (imagewindow) {
			found = true;
			result = imagewindow->isLoaded ();
		}
	}
	target->release ();
	return (result);
}
static int exec (lua_State *L) {
	char buf[1024];
	char *args[2];
	bool result;

	LuaFunction::argvString (L, 1, &(args[0]));
	LuaFunction::argvString (L, 2, &(args[1]), true);
	result = LuaFunction::awaitResult (awaitResult_exec, args);
	if (! result) {
		snprintf (buf, sizeof (buf), "awaitvalue: control value not found; controlName=\"%s\"", args[0]);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

LuaAwaitvalueFunction::LuaAwaitvalueFunction ()
: LuaFunction ()
{
	name.assign ("awaitvalue");
	parameters.assign ("controlName, text");
	fn = exec;
}
LuaAwaitvalueFunction::~LuaAwaitvalueFunction () {
}

StdString LuaAwaitvalueFunction::getHelpText () {
	return (UiText::instance->getText (UiTextId::LuaScriptAwaitvalueHelpText));
}
