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
// Panel that shows console output and a text field for input
#ifndef CONSOLE_WINDOW_H
#define CONSOLE_WINDOW_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class IconLabelWindow;
class Button;
class TextArea;
class TextFieldWindow;

class ConsoleWindow : public Panel {
public:
	ConsoleWindow (double windowWidth, double windowHeight);
	~ConsoleWindow ();

	static constexpr const int maxTextLines = 1024;
	
	// Read-write data members
	Widget::EventCallbackContext textEnterCallback;
	Widget::EventCallbackContext fileRunCallback;

	// Read-only data members
	double windowWidth;
	double windowHeight;
	StdString textEnterValue;
	StdString fileRunPath;

	// Set the window size
	void setWindowSize (double windowWidthValue, double windowHeightValue);

	// Assign the widgetName value for the window and its component widgets
	void setWidgetNames (const StdString &name);

	// Set the window title text
	void setTitleText (const StdString &text);

	// Set the empty value prompt for the window's text field
	void setTextFieldPrompt (const StdString &text);

	// Show the window's run button. When the run action is executed, set fileRunPath and invoke fileRunCallback.
	void showRunButton (const StdString &fileSelectPromptText = StdString (), const StdString &runButtonTooltipText = StdString ());

	// Assign keypress edit focus to the console's text field
	void assignKeyFocus ();

	// Append text as console window output
	void appendText (const StdString &text);

	// Superclass override methods
	void reflow ();

	// Return the value of the console's text field
	StdString getTextFieldValue () const;

private:
	// Callback functions
	static void closeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void textFieldWindowValueEdited (void *itPtr, Widget *widgetPtr);
	static void runButtonClicked (void *itPtr, Widget *widgetPtr);
	static void fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr);

	IconLabelWindow *titleIcon;
	Button *runButton;
	StdString runFileSelectPromptText;
	Button *closeButton;
	TextArea *textArea;
	TextFieldWindow *textField;
	WidgetHandle<Panel> fsBrowserPanelHandle;
	Panel *fsBrowserPanel;
};
#endif
