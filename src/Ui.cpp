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
#include "SdlUtil.h"
#include "Log.h"
#include "Input.h"
#include "UiText.h"
#include "Widget.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Button.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ProgressBar.h"
#include "HelpWindow.h"
#include "IconLabelWindow.h"
#include "Toolbar.h"
#include "CardView.h"
#include "Menu.h"
#include "SpriteId.h"
#include "UiStack.h"
#include "Ui.h"

Ui::Ui ()
: classId (-1)
, rootPanel (NULL)
, isLoaded (false)
, isFirstResumeComplete (false)
, cardView (NULL)
, actionWidgetHandle (&actionWidget)
, actionTargetHandle (&actionTarget)
, actionSource (NULL)
, breadcrumbWidgetHandle (&breadcrumbWidget)
, toolPopupHandle (&toolPopup)
, toolPopupSourceHandle (&toolPopupSource)
, detailImageSize (-1)
, refcount (0)
, lastWindowCloseCount (0)
{
	SdlUtil::createMutex (&refcountMutex);

	rootPanel = new Panel ();
	rootPanel->keyEventCallback = Widget::KeyEventCallbackContext (Ui::keyEvent, this);
	rootPanel->retain ();
}
Ui::~Ui () {
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}
	SdlUtil::destroyMutex (&refcountMutex);
}

void Ui::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}
void Ui::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);
	if (isdestroyed) {
		delete (this);
	}
}

bool Ui::isUiClass (Ui *ui, int classIdValue) {
	return (ui && (ui->classId == classIdValue));
}

OpResult Ui::load () {
	StdString path;
	OpResult result;

	if (isLoaded) {
		return (OpResult::Success);
	}
	path = getSpritePath ();
	if (! path.empty ()) {
		result = sprites.load (path);
		if (result != OpResult::Success) {
			Log::err ("Failed to load sprite resources");
			return (result);
		}
	}

	setDetailImageSize (Ui::MediumSize);

	cardView = (CardView *) addWidget (new CardView (App::instance->drawableWidth, App::instance->drawableHeight - UiStack::instance->topBarHeight - UiStack::instance->bottomBarHeight));
	cardView->isKeyboardScrollEnabled = true;
	cardView->isMouseWheelScrollEnabled = true;
	cardView->isExitedMouseWheelScrollEnabled = true;

	result = doLoad ();
	if (result != OpResult::Success) {
		return (result);
	}
	cardView->position.assign (0.0f, UiStack::instance->topBarHeight);
	isLoaded = true;
	return (OpResult::Success);
}
OpResult Ui::doLoad () {
	// Default implementation does nothing
	return (OpResult::Success);
}

void Ui::unload () {
	if (! isLoaded) {
		return;
	}
	actionWidgetHandle.destroyAndClear ();
	actionTargetHandle.clear ();
	breadcrumbWidgetHandle.destroyAndClear ();
	cardView->removeAllItems ();
	clearPopupWidgets ();
	clearWidgets ();

	rootPanel->clear ();
	doUnload ();

	sprites.unload ();
	isLoaded = false;
}
void Ui::doUnload () {
	// Default implementation does nothing
}

StdString Ui::getSpritePath () {
	// Default implementation returns an empty path
	return (StdString ());
}

bool Ui::eventCallback (const Ui::EventCallbackContext &callback) {
	if (! callback.callback) {
		return (false);
	}
	callback.callback (callback.callbackData, this);
	return (true);
}

void Ui::processShutdownEvent () {
	LabelWindow *label;
	ProgressBar *bar;
	Panel *panel;

	if (doProcessShutdownEvent ()) {
		return;
	}
	clearPopupWidgets ();
	panel = new Panel ();
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);

	label = new LabelWindow (new Label (StdString::createSprintf ("%s %s", UiText::instance->getText (UiTextId::ShuttingDown).capitalized ().c_str (), APPLICATION_NAME), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	label->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	label->reflow ();

	bar = new ProgressBar (label->width, UiConfiguration::instance->progressBarHeight);
	bar->setIndeterminate (true);

	App::instance->rootPanel->addWidget (panel, App::instance->rootPanel->maxWidgetZLevel + 1);
	App::instance->rootPanel->addWidget (label, App::instance->rootPanel->maxWidgetZLevel + 2);
	App::instance->rootPanel->addWidget (bar, App::instance->rootPanel->maxWidgetZLevel + 2);
	label->position.assign ((App::instance->rootPanel->width / 2.0f) - (label->width / 2.0f), (App::instance->rootPanel->height / 2.0f) - (label->height / 2.0f));
	bar->position.assign (label->position.x, label->position.y + label->height);
}
bool Ui::doProcessShutdownEvent () {
	// Default implementation does nothing
	return (false);
}

bool Ui::keyEvent (void *itPtr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	return (((Ui *) itPtr)->processKeyEvent (keycode, isShiftDown, isControlDown));
}

bool Ui::processKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Base class method takes no action
	return (doProcessKeyEvent (keycode, isShiftDown, isControlDown));
}
bool Ui::doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	// Default implementation does nothing
	return (false);
}

bool Ui::doProcessWindowCloseEvent () {
	// Default implementation does nothing
	return (false);
}

void Ui::resume () {
	StdString title;

	if (! isLoaded) {
		return;
	}
	if (rootPanel->id <= 0) {
		rootPanel->id = App::instance->getUniqueId ();
	}
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, App::instance->drawableWidth, App::instance->drawableHeight);
	rootPanel->resetInputState ();
	rootPanel->isInputSuspended = false;
	App::instance->rootPanel->addWidget (rootPanel);
	lastWindowCloseCount = Input::instance->windowCloseCount;
	cardView->setViewSize (App::instance->drawableWidth, App::instance->drawableHeight - UiStack::instance->topBarHeight - UiStack::instance->bottomBarHeight);
	cardView->position.assign (0.0f, UiStack::instance->topBarHeight);

	doResume ();
	isFirstResumeComplete = true;
}
void Ui::doResume () {
	// Default implementation does nothing
}

void Ui::pause () {
	if (! isLoaded) {
		return;
	}
	App::instance->rootPanel->removeWidget (rootPanel);
	lastWindowCloseCount = Input::instance->windowCloseCount;
	clearPopupWidgets ();
	doPause ();
}
void Ui::doPause () {
	// Default implementation does nothing
}

void Ui::update (int msElapsed) {
	int count;

	if (actionWidget) {
		actionWidgetHandle.compact ();
		if (! actionWidget) {
			actionTargetHandle.clear ();
		}
	}
	actionTargetHandle.compact ();
	breadcrumbWidgetHandle.compact ();
	toolPopupHandle.compact ();

	count = Input::instance->windowCloseCount;
	if (lastWindowCloseCount != count) {
		lastWindowCloseCount = count;
		if (! doProcessWindowCloseEvent ()) {
			App::instance->shutdown ();
		}
	}

	doUpdate (msElapsed);
}
void Ui::doUpdate (int msElapsed) {
	// Default implementation does nothing
}

void Ui::draw () {
	// Base class method takes no action
	doDraw ();
}
void Ui::doDraw () {
	// Default implementation does nothing
}

void Ui::reloadSprites () {
	sprites.resize ();
}

void Ui::resize () {
	rootPanel->position.assign (0.0f, 0.0f);
	rootPanel->setFixedSize (true, App::instance->drawableWidth, App::instance->drawableHeight);
	rootPanel->resize ();
	cardView->setViewSize (App::instance->drawableWidth, App::instance->drawableHeight - UiStack::instance->topBarHeight - UiStack::instance->bottomBarHeight);
	cardView->position.assign (0.0f, UiStack::instance->topBarHeight);
	doResize ();
	rootPanel->resetInputState ();
}
void Ui::doResize () {
	// Default implementation does nothing
}

void Ui::clearPopupWidgets () {
	actionWidgetHandle.destroyAndClear ();
	actionTargetHandle.clear ();
	actionSource = NULL;
	toolPopupHandle.destroyAndClear ();
	toolPopupSourceHandle.clear ();
	doClearPopupWidgets ();
}
void Ui::doClearPopupWidgets () {
	// Default implementation does nothing
}

Widget *Ui::addWidget (Widget *widget, int zLevel) {
	return (rootPanel->addWidget (widget, zLevel));
}

static void clearWidgets_processWidget (void *itPtr, Widget *widgetPtr) {
	widgetPtr->isDestroyed = true;
}
void Ui::clearWidgets () {
	rootPanel->processWidgets (clearWidgets_processWidget, NULL);
	doClearWidgets ();
}
void Ui::doClearWidgets () {
	// Default implementation does nothing
}

Widget *Ui::createBreadcrumbWidget () {
	return (NULL);
}

void Ui::addMainToolbarItems (Toolbar *toolbar) {
	Widget *widget;

	if (! breadcrumbWidget) {
		widget = createBreadcrumbWidget ();
		if (widget) {
			breadcrumbWidgetHandle.assign (widget);
			toolbar->addLeftItem (breadcrumbWidget);
		}
	}

	doAddMainToolbarItems (toolbar);
}

void Ui::addSecondaryToolbarItems (Toolbar *toolbar) {
	// Base class method takes no action
	doAddSecondaryToolbarItems (toolbar);
}

void Ui::doAddMainToolbarItems (Toolbar *toolbar) {
	// Default implementation does nothing
}

void Ui::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	// Default implementation does nothing
}

void Ui::handleSecondaryToolbarModeChange (int modeValue) {
	// Default implementation does nothing
}

void Ui::setHelpWindowContent (HelpWindow *helpWindow) {
	// Default implementation does nothing
}

Button *Ui::createIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText, const char *widgetName) {
	Button *button;

	button = new Button (buttonSprite);
	if (widgetName) {
		button->widgetName.assign (widgetName);
	}
	button->mouseClickCallback = clickCallback;
	button->setImageColor (UiConfiguration::instance->buttonTextColor);
	if (! mouseHoverTooltipText.empty ()) {
		button->setMouseHoverTooltip (mouseHoverTooltipText);
	}
	return (button);
}

Button *Ui::createScrimIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText, const char *widgetName) {
	Button *button;

	button = new Button (buttonSprite);
	if (widgetName) {
		button->widgetName.assign (widgetName);
	}
	button->mouseClickCallback = clickCallback;
	button->setColorBackground (Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	button->setInverseColor (true);
	if (! mouseHoverTooltipText.empty ()) {
		button->setMouseHoverTooltip (mouseHoverTooltipText);
	}
	return (button);
}

Panel *Ui::createDarkWindowOverlayPanel () {
	Panel *panel;

	panel = new Panel ();
	panel->setFixedSize (true, App::instance->drawableWidth, App::instance->drawableHeight);
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	return (panel);
}

Button *Ui::createToolbarIconButton (Sprite *buttonSprite, Widget::EventCallbackContext clickCallback, const StdString &mouseHoverTooltipText, const char *widgetName, SDL_Keycode shortcutKey) {
	Button *button;

	button = new Button (buttonSprite);
	if (widgetName) {
		button->widgetName.assign (widgetName);
	}
	button->mouseClickCallback = clickCallback;
	button->setInverseColor (true);
	if (! mouseHoverTooltipText.empty ()) {
		button->setMouseHoverTooltip (mouseHoverTooltipText);
	}
	button->shortcutKey = shortcutKey;
	return (button);
}

bool Ui::luaOpen (const char *targetName, int64_t *updateTaskId) {
	Widget *widget;
	Ui::LuaWidgetContext *ctx;
	int64_t id;

	widget = findLuaOpenWidget (targetName);
	if (! widget) {
		return (false);
	}
	ctx = new Ui::LuaWidgetContext ();
	ctx->ui = this;
	ctx->widget = widget;

	ctx->ui->retain ();
	id = App::instance->addUpdateTask (Ui::executeLuaOpenTask, ctx);
	if (updateTaskId) {
		*updateTaskId = id;
	}
	return (true);
}
Widget *Ui::findLuaOpenWidget (const char *targetName) {
	// Default implementation returns no widget
	return (NULL);
}
void Ui::executeLuaOpenTask (void *ctxPtr) {
	Ui::LuaWidgetContext *ctx;

	ctx = (Ui::LuaWidgetContext *) ctxPtr;
	ctx->ui->executeLuaOpen (ctx->widget);
	ctx->widget->release ();
	ctx->ui->release ();
	delete (ctx);
}
void Ui::executeLuaOpen (Widget *targetWidget) {
	// Default implementation does nothing
}

bool Ui::luaTarget (const char *targetName, int64_t *updateTaskId) {
	Widget *widget;
	Ui::LuaWidgetContext *ctx;
	int64_t id;

	widget = findLuaTargetWidget (targetName);
	if (! widget) {
		return (false);
	}
	ctx = new Ui::LuaWidgetContext ();
	ctx->ui = this;
	ctx->widget = widget;

	ctx->ui->retain ();
	id = App::instance->addUpdateTask (Ui::executeLuaTargetTask, ctx);
	if (updateTaskId) {
		*updateTaskId = id;
	}
	return (true);
}
Widget *Ui::findLuaTargetWidget (const char *targetName) {
	// Default implementation returns no widget
	return (NULL);
}
void Ui::executeLuaTargetTask (void *ctxPtr) {
	Ui::LuaWidgetContext *ctx;

	ctx = (Ui::LuaWidgetContext *) ctxPtr;
	ctx->ui->executeLuaTarget (ctx->widget);
	ctx->widget->release ();
	ctx->ui->release ();
	delete (ctx);
}
void Ui::executeLuaTarget (Widget *targetWidget) {
	// Default implementation does nothing
}

bool Ui::luaUntarget (const char *targetName, int64_t *updateTaskId) {
	Widget *widget;
	Ui::LuaWidgetContext *ctx;
	int64_t id;

	widget = findLuaTargetWidget (targetName);
	if (! widget) {
		return (false);
	}
	ctx = new Ui::LuaWidgetContext ();
	ctx->ui = this;
	ctx->widget = widget;

	ctx->ui->retain ();
	id = App::instance->addUpdateTask (Ui::executeLuaUntargetTask, ctx);
	if (updateTaskId) {
		*updateTaskId = id;
	}
	return (true);
}
void Ui::executeLuaUntargetTask (void *ctxPtr) {
	Ui::LuaWidgetContext *ctx;

	ctx = (Ui::LuaWidgetContext *) ctxPtr;
	ctx->ui->executeLuaUntarget (ctx->widget);
	ctx->widget->release ();
	ctx->ui->release ();
	delete (ctx);
}
void Ui::executeLuaUntarget (Widget *targetWidget) {
	// Default implementation does nothing
}

void Ui::syncRecordStore () {
	// Superclass method takes no action
	doSyncRecordStore ();
}
void Ui::doSyncRecordStore () {
	// Default implementation does nothing
}

void Ui::showActionPopup (Widget *action, Widget *target, Widget::EventCallback sourceFn, const Widget::Rectangle &sourceRect, int xAlignment, int yAlignment) {
	actionWidgetHandle.assign (action);
	actionTargetHandle.assign (target);
	actionSource = sourceFn;

	App::instance->rootPanel->addWidget (action, App::instance->rootPanel->maxWidgetZLevel + 1);
	assignPopupPosition (action, sourceRect, xAlignment, yAlignment);
}

bool Ui::clearActionPopup (Widget *target, Widget::EventCallback sourceFn) {
	bool match;

	match = actionWidget && (actionTarget == target) && (actionSource == sourceFn);
	clearPopupWidgets ();
	return (match);
}

Button *Ui::createToolPopupButton (Sprite *sprite, Widget::EventCallback clickCallback, Widget::EventCallback focusCallback, const StdString &tooltipText, const char *widgetName, SDL_Keycode shortcutKey) {
	Button *button;

	button = new Button (sprite);
	button->mouseClickCallback = Widget::EventCallbackContext (clickCallback, this);
	button->focusCallback = Widget::EventCallbackContext (focusCallback, this);
	button->unfocusCallback = Widget::EventCallbackContext (Ui::toolPopupSourceUnfocused, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (tooltipText, Widget::LeftAlignment);
	if (widgetName) {
		button->widgetName.assign (widgetName);
	}
	button->shortcutKey = shortcutKey;
	return (button);
}

void Ui::toolPopupSourceUnfocused (void *itPtr, Widget *widgetPtr) {
	Ui *it;

	it = (Ui *) itPtr;
	if (it->toolPopupSourceHandle.equals (widgetPtr)) {
		it->toolPopupHandle.destroyAndClear ();
		it->toolPopupSourceHandle.clear ();
	}
}

void Ui::showToolPopup (Widget *sourceWidget, const StdString &titleText, Widget *detailWidget1, Widget *detailWidget2) {
	Panel *panel;
	LabelWindow *label;

	panel = new Panel ();
	panel->isInputSuspended = true;
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	panel->setBorder (true, Color (UiConfiguration::instance->darkBackgroundColor.r, UiConfiguration::instance->darkBackgroundColor.g, UiConfiguration::instance->darkBackgroundColor.b, UiConfiguration::instance->overlayWindowAlpha));
	label = panel->add (new LabelWindow (new Label (titleText, UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
	label->setFixedPadding (true, 0.0f, 0.0f);
	label->reflow ();
	if (detailWidget1) {
		panel->addWidget (detailWidget1);
	}
	if (detailWidget2) {
		panel->addWidget (detailWidget2);
	}
	panel->setLayout (Panel::DownFlowLayoutOption | Panel::RightGravityLayoutOption);
	panel->position.assign (App::instance->drawableWidth - panel->width - UiConfiguration::instance->paddingSize, App::instance->drawableHeight - UiStack::instance->bottomBarHeight - panel->height - UiConfiguration::instance->marginSize);

	toolPopupHandle.destroyAndAssign (panel);
	toolPopupSourceHandle.assign (sourceWidget);
	App::instance->rootPanel->add (panel, App::instance->rootPanel->maxWidgetZLevel + 1);
}

IconLabelWindow *Ui::createToolPopupLabel (Sprite *sprite, const StdString &labelText, const Color &textColor) {
	IconLabelWindow *icon;

	icon = new IconLabelWindow (sprite, labelText, UiConfiguration::CaptionFont, textColor);
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setRightAligned (true);
	return (icon);
}

void Ui::assignPopupPosition (Widget *popupWidget, const Widget::Rectangle &popupSourceRect, int xAlignment, int yAlignment) {
	double x, y;

	switch (xAlignment) {
		case Ui::LeftOfAlignment: {
			x = popupSourceRect.x - popupWidget->width;
			if (x <= UiConfiguration::instance->marginSize) {
				x = popupSourceRect.x + popupSourceRect.w;
			}
			break;
		}
		case Ui::LeftEdgeAlignment: {
			x = popupSourceRect.x;
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) >= (App::instance->drawableWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->drawableWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
		case Ui::RightEdgeAlignment: {
			x = popupSourceRect.x + popupSourceRect.w - popupWidget->width;
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) >= (App::instance->drawableWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->drawableWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
		case Ui::RightOfAlignment: {
			x = popupSourceRect.x + popupSourceRect.w;
			if ((x + popupWidget->width) >= (App::instance->drawableWidth - UiConfiguration::instance->marginSize)) {
				x = popupSourceRect.x - popupWidget->width;
			}
			break;
		}
		default: {
			x = popupSourceRect.x + ((popupSourceRect.w / 2.0f) - (popupWidget->width / 2.0f));
			if (x < UiConfiguration::instance->marginSize) {
				x = UiConfiguration::instance->marginSize;
			}
			if ((x + popupWidget->width) > (App::instance->drawableWidth - UiConfiguration::instance->marginSize)) {
				x = App::instance->drawableWidth - UiConfiguration::instance->marginSize - popupWidget->width;
			}
			break;
		}
	}

	switch (yAlignment) {
		case Ui::TopOfAlignment: {
			y = popupSourceRect.y - popupWidget->height;
			if (y <= UiConfiguration::instance->marginSize) {
				y = popupSourceRect.y + popupSourceRect.h;
			}
			break;
		}
		case Ui::TopEdgeAlignment: {
			y = popupSourceRect.y;
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) > (App::instance->drawableHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->drawableHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
		case Ui::BottomEdgeAlignment: {
			y = popupSourceRect.y + popupSourceRect.h - popupWidget->height;
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) >= (App::instance->drawableHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->drawableHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
		case Ui::BottomOfAlignment: {
			y = popupSourceRect.y + popupSourceRect.h;
			if ((y + popupWidget->height) >= (App::instance->drawableHeight - UiConfiguration::instance->marginSize)) {
				y = popupSourceRect.y - popupWidget->height;
			}
			break;
		}
		default: {
			y = popupSourceRect.y + ((popupSourceRect.h / 2.0f) - (popupWidget->height / 2.0f));
			if (y < UiConfiguration::instance->marginSize) {
				y = UiConfiguration::instance->marginSize;
			}
			if ((y + popupWidget->height) > (App::instance->drawableHeight - UiConfiguration::instance->marginSize)) {
				y = App::instance->drawableHeight - UiConfiguration::instance->marginSize - popupWidget->height;
			}
			break;
		}
	}

	popupWidget->position.assign (x, y);
	popupWidget->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
}

Panel *Ui::createRowHeaderPanel (const StdString &headerText, Panel *sidePanel) {
	Panel *panel;
	LabelWindow *label;

	panel = new Panel ();
	panel->setLayout (Panel::RightFlowLayoutOption);
	if (! headerText.empty ()) {
		label = (LabelWindow *) panel->addWidget (new LabelWindow (new Label (headerText, UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor)));
		label->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	}
	if (sidePanel) {
		panel->addWidget (sidePanel);
	}
	panel->reflow ();

	return (panel);
}

IconLabelWindow *Ui::createLoadingIconWindow () {
	IconLabelWindow *icon;

	icon = new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallLoadingIcon), UiText::instance->getText (UiTextId::Loading).capitalized (), UiConfiguration::CaptionFont);
	icon->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	icon->setProgressBar (true);
	return (icon);
}

Button *Ui::createImageSizeButton () {
	Button *button;

	button = new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_selectImageSizeButton));
	button->widgetName.assign ("mainToolbarImageSizeButton");
	button->mouseClickCallback = Widget::EventCallbackContext (Ui::imageSizeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::ThumbnailImageSizeTooltip));
	return (button);
}

void Ui::imageSizeButtonClicked (void *itPtr, Widget *widgetPtr) {
	Ui *it;
	Menu *menu;

	it = (Ui *) itPtr;
	UiStack::instance->suspendMouseHover ();
	if (it->clearActionPopup (widgetPtr, Ui::imageSizeButtonClicked)) {
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	menu->addItem (UiText::instance->getText (UiTextId::Small).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallSizeButton), Widget::EventCallbackContext (Ui::smallImageSizeActionClicked, it), 0, it->detailImageSize == Ui::SmallSize);
	menu->addItem (UiText::instance->getText (UiTextId::Medium).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_mediumSizeButton), Widget::EventCallbackContext (Ui::mediumImageSizeActionClicked, it), 0, it->detailImageSize == Ui::MediumSize);
	menu->addItem (UiText::instance->getText (UiTextId::Large).capitalized (), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeSizeButton), Widget::EventCallbackContext (Ui::largeImageSizeActionClicked, it), 0, it->detailImageSize == Ui::LargeSize);
	it->showActionPopup (menu, widgetPtr, Ui::imageSizeButtonClicked, widgetPtr->getScreenRect (), Ui::RightEdgeAlignment, Ui::BottomOfAlignment);
}

void Ui::smallImageSizeActionClicked (void *itPtr, Widget *widgetPtr) {
	((Ui *) itPtr)->setDetailImageSize (Ui::SmallSize);
}

void Ui::mediumImageSizeActionClicked (void *itPtr, Widget *widgetPtr) {
	((Ui *) itPtr)->setDetailImageSize (Ui::MediumSize);
}

void Ui::largeImageSizeActionClicked (void *itPtr, Widget *widgetPtr) {
	((Ui *) itPtr)->setDetailImageSize (Ui::LargeSize);
}

void Ui::setDetailImageSize (int detailImageSizeValue) {
	if (detailImageSize == detailImageSizeValue) {
		return;
	}
	detailImageSize = detailImageSizeValue;
	if (isLoaded) {
		handleDetailImageSizeChange ();
	}
}

void Ui::handleDetailImageSizeChange () {
	// Default implementation does nothing
}
