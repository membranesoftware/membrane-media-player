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
#include "ClassId.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "HashMap.h"
#include "OsUtil.h"
#include "Ui.h"
#include "Label.h"
#include "Button.h"
#include "IconLabelWindow.h"
#include "TextArea.h"
#include "TextFieldWindow.h"
#include "FsBrowserWindow.h"
#include "ConsoleWindow.h"

constexpr const double fsBrowserWindowScale = 0.83f;

ConsoleWindow::ConsoleWindow (double windowWidth, double windowHeight)
: Panel ()
, windowWidth (windowWidth)
, windowHeight (windowHeight)
, runButton (NULL)
, fsBrowserPanelHandle (&fsBrowserPanel)
{
	classId = ClassId::ConsoleWindow;
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setFixedSize (true, windowWidth, windowHeight);

	titleIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_consoleIcon)));
	titleIcon->setFixedPadding (true, 0.0f, 0.0f);

	closeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton)), 1);
	closeButton->mouseClickCallback = Widget::EventCallbackContext (ConsoleWindow::closeButtonClicked, this);
	closeButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);

	textArea = add (new TextArea (0.0f, 0.0f, UiConfiguration::ConsoleFont, true));
	textArea->setMaxLineCount (ConsoleWindow::maxTextLines);
	textArea->setInverseColor (true);

	textField = add (new TextFieldWindow (0.0f));
	textField->setFixedPadding (true, 0.0f, 0.0f);
	textField->setButtonsEnabled (TextFieldWindow::PasteButtonOption | TextFieldWindow::ClearButtonOption);
	textField->setRetainFocusOnReturnKey (true);
	textField->valueEditCallback = Widget::EventCallbackContext (ConsoleWindow::textFieldWindowValueEdited, this);

	reflow ();
}
ConsoleWindow::~ConsoleWindow () {
	fsBrowserPanelHandle.destroyAndClear ();
}

StdString ConsoleWindow::getTextFieldValue () const {
	return (textField->getValue ());
}

void ConsoleWindow::setWindowSize (double windowWidthValue, double windowHeightValue) {
	windowWidth = windowWidthValue;
	windowHeight = windowHeightValue;
	setFixedSize (true, windowWidth, windowHeight);
	reflow ();
}

void ConsoleWindow::setWidgetNames (const StdString &name) {
	widgetName.assign (name);
	if (runButton) {
		runButton->widgetName.sprintf ("%sRunButton", name.c_str ());
	}
	closeButton->widgetName.sprintf ("%sCloseButton", name.c_str ());
	textArea->widgetName.sprintf ("%sTextArea", name.c_str ());
	textField->widgetName.sprintf ("%sTextField", name.c_str ());
}

void ConsoleWindow::setTitleText (const StdString &text) {
	titleIcon->setText (text);
	reflow ();
}

void ConsoleWindow::setTextFieldPrompt (const StdString &text) {
	textField->setPromptText (text);
}

void ConsoleWindow::showRunButton (const StdString &fileSelectPromptText, const StdString &runButtonTooltipText) {
	if (! runButton) {
		runButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_directoryButton)), 1);
		runButton->mouseClickCallback = Widget::EventCallbackContext (ConsoleWindow::runButtonClicked, this);
	}
	runFileSelectPromptText.assign (fileSelectPromptText);
	if (! runButtonTooltipText.empty ()) {
		runButton->setMouseHoverTooltip (runButtonTooltipText);
	}
	reflow ();
}

void ConsoleWindow::assignKeyFocus () {
	textField->assignKeyFocus ();
}

void ConsoleWindow::reflow () {
	double w, h;

	resetPadding ();
	topLeftLayoutFlow ();
	titleIcon->flowRight (&layoutFlow);
	closeButton->flowRight (&layoutFlow);
	titleIcon->centerVertical (&layoutFlow);
	closeButton->centerVertical (&layoutFlow);

	w = windowWidth - (widthPadding * 2.0f);
	if (runButton) {
		w -= (runButton->width + UiConfiguration::instance->marginSize);
	}
	textField->setWindowWidth (w);

	h = textField->height;
	if (runButton && (runButton->height > h)) {
		h = runButton->height;
	}
	nextRowLayoutFlow ();
	textArea->setViewSize (windowWidth - (widthPadding * 2.0f), windowHeight - layoutFlow.y - heightPadding - h - UiConfiguration::instance->marginSize);

	textArea->flowDown (&layoutFlow);
	if (runButton) {
		runButton->flowRight (&layoutFlow);
	}
	textField->flowDown (&layoutFlow);
	if (runButton) {
		runButton->position.assignY (runButton->position.y + (h / 2.0f) - (runButton->height / 2.0f));
	}
	textField->position.assignY (textField->position.y + (h / 2.0f) - (textField->height / 2.0f));

	bottomRightLayoutFlow ();
	closeButton->flowLeft (&layoutFlow);
}

void ConsoleWindow::appendText (const StdString &text) {
	textArea->appendText (text, true);
}

void ConsoleWindow::closeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((ConsoleWindow *) itPtr)->isDestroyed = true;
}

void ConsoleWindow::textFieldWindowValueEdited (void *itPtr, Widget *widgetPtr) {
	ConsoleWindow *it = (ConsoleWindow *) itPtr;
	TextFieldWindow *textfield = (TextFieldWindow *) widgetPtr;

	it->textEnterValue.assign (textfield->getValue ());
	if (it->textEnterValue.empty ()) {
		return;
	}
	it->textArea->appendText (StdString::createSprintf ("\n> %s", it->textEnterValue.c_str ()), true);
	textfield->setValue (StdString (), true, true);
	it->eventCallback (it->textEnterCallback);
	it->textEnterValue.assign ("");
}

void ConsoleWindow::runButtonClicked (void *itPtr, Widget *widgetPtr) {
	ConsoleWindow *it = (ConsoleWindow *) itPtr;
	FsBrowserWindow *fs;
	HashMap *prefs;
	StdString path;

	prefs = App::instance->lockPrefs ();
	path = prefs->find (App::fsBrowserPathKey, "");
	App::instance->unlockPrefs ();
	if (path.empty ()) {
		path = OsUtil::getUserHomePath ();
	}
	it->fsBrowserPanelHandle.destroyAndAssign (Ui::createDarkWindowOverlayPanel ());

	fs = (FsBrowserWindow *) it->fsBrowserPanel->add (new FsBrowserWindow (App::instance->drawableWidth * fsBrowserWindowScale, App::instance->drawableHeight * fsBrowserWindowScale, path));
	fs->sortOrder = FsBrowserWindow::DirectoriesFirstSort;
	fs->selectType = FsBrowserWindow::SelectFiles;
	fs->closeCallback = Widget::EventCallbackContext (ConsoleWindow::fsBrowserWindowClosed, it);
	if (! it->runFileSelectPromptText.empty ()) {
		fs->setSelectPromptText (it->runFileSelectPromptText);
	}
	fs->position.assign ((App::instance->drawableWidth - fs->width) / 2.0f, (App::instance->drawableHeight - fs->height) / 2.0f);

	App::instance->rootPanel->addWidget (it->fsBrowserPanel, App::instance->rootPanel->maxWidgetZLevel + 1);
}

void ConsoleWindow::fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr) {
	ConsoleWindow *it = (ConsoleWindow *) itPtr;
	FsBrowserWindow *fs = (FsBrowserWindow *) widgetPtr;
	HashMap *prefs;
	StdString path;

	prefs = App::instance->lockPrefs ();
	prefs->insert (App::fsBrowserPathKey, fs->browsePath, "");
	App::instance->unlockPrefs ();
	if (fs->isPathSelectionConfirmed) {
		path.assign (fs->selectedPath);
	}
	it->fsBrowserPanelHandle.destroyAndClear ();
	if (! path.empty ()) {
		it->fileRunPath.assign (path);
		it->eventCallback (it->fileRunCallback);
	}
}
