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
#include "Panel.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextField.h"
#include "TextFieldWindow.h"
#include "IconLabelWindow.h"
#include "ComboBox.h"
#include "SliderWindow.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "LuaSetvalueFunction.h"

struct StateData {
	int64_t taskId;
	char *targetName;
	char *value;
	Widget *widget;
};

static void execUpdateTask_Label (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((Label *) data->widget)->setText (StdString (data->value));
	data->widget->release ();
}
static void execUpdateTask_LabelWindow (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((LabelWindow *) data->widget)->setText (StdString (data->value));
	data->widget->release ();
}
static void execUpdateTask_TextField (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((TextField *) data->widget)->setValue (StdString (data->value));
	data->widget->release ();
}
static void execUpdateTask_TextFieldWindow (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((TextFieldWindow *) data->widget)->setValue (StdString (data->value));
	data->widget->release ();
}
static void execUpdateTask_IconLabelWindow (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((IconLabelWindow *) data->widget)->setText (StdString (data->value));
	data->widget->release ();
}
static void execUpdateTask_SliderWindow (void *dataPtr) {
	StateData *data;
	double d;

	data = (StateData *) dataPtr;
	if (StdString (data->value).parseFloat (&d)) {
		((SliderWindow *) data->widget)->setValue (d);
	}
	data->widget->release ();
}
static void execUpdateTask_Toggle (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((Toggle *) data->widget)->setChecked (StdString (data->value).equals ("true"));
	data->widget->release ();
}
static void execUpdateTask_ToggleWindow (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((ToggleWindow *) data->widget)->setChecked (StdString (data->value).equals ("true"));
	data->widget->release ();
}
static void execUpdateTask_ComboBox (void *dataPtr) {
	StateData *data;

	data = (StateData *) dataPtr;
	((ComboBox *) data->widget)->setValue (StdString (data->value));
	data->widget->release ();
}
static bool awaitResult_exec (void *dataPtr) {
	Widget *target;
	StateData *data;
	ComboBox *combobox;
	double d;

	if (! App::instance->rootPanel) {
		return (false);
	}
	data = (StateData *) dataPtr;
	target = App::instance->rootPanel->findWidget (StdString (data->targetName), true);
	if (! target) {
		return (false);
	}
	if (target->isDestroyed || (! target->isVisible) || target->isInputSuspended) {
		data->widget = NULL;
	}
	else {
		data->widget = NULL;
		if (!(data->widget)) {
			if (Label::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_Label, data);
			}
		}
		if (!(data->widget)) {
			if (LabelWindow::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_LabelWindow, data);
			}
		}
		if (!(data->widget)) {
			if (TextField::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_TextField, data);
			}
		}
		if (!(data->widget)) {
			if (TextFieldWindow::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_TextFieldWindow, data);
			}
		}
		if (!(data->widget)) {
			if (IconLabelWindow::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_IconLabelWindow, data);
			}
		}
		if (!(data->widget)) {
			if (SliderWindow::castWidget (target) && StdString (data->value).parseFloat (&d)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_SliderWindow, data);
			}
		}
		if (!(data->widget)) {
			if (Toggle::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_Toggle, data);
			}
		}
		if (!(data->widget)) {
			if (ToggleWindow::castWidget (target)) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_ToggleWindow, data);
			}
		}
		if (!(data->widget)) {
			combobox = ComboBox::castWidget (target);
			if (combobox && combobox->containsItem (StdString (data->value))) {
				data->widget = target;
				data->taskId = App::instance->addUpdateTask (execUpdateTask_ComboBox, data);
			}
		}
	}
	if (!(data->widget)) {
		target->release ();
		return (false);
	}
	return (true);
}
static int exec (lua_State *L) {
	char buf[1024];
	bool result;
	StateData data;

	memset (&data, 0, sizeof (data));
	LuaFunction::argvString (L, 1, &(data.targetName));
	LuaFunction::argvString (L, 2, &(data.value), true);
	result = LuaFunction::awaitResult (awaitResult_exec, &data);
	if (! result) {
		snprintf (buf, sizeof (buf), "setvalue: failed to update widget; controlName=\"%s\"", data.targetName);
		return (luaL_error (L, "%s", buf));
	}
	result = LuaFunction::awaitUpdateTaskComplete (data.taskId);
	if (! result) {
		snprintf (buf, sizeof (buf), "setvalue: failed to update widget; controlName=\"%s\"", data.targetName);
		return (luaL_error (L, "%s", buf));
	}
	return (0);
}

LuaSetvalueFunction::LuaSetvalueFunction ()
: LuaFunction ()
{
	name.assign ("setvalue");
	parameters.assign ("controlName, value");
	fn = exec;
}
LuaSetvalueFunction::~LuaSetvalueFunction () {
}

StdString LuaSetvalueFunction::getHelpText () {
	return (UiText::instance->getText (UiTextId::LuaScriptSetvalueHelpText));
}