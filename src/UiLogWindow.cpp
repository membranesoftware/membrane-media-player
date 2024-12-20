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
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "Ui.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "UiLog.h"
#include "Button.h"
#include "Label.h"
#include "LabelWindow.h"
#include "IconLabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "TextFlow.h"
#include "ScrollViewWindow.h"
#include "UiLogWindow.h"

constexpr const double headerImageAspectRatio = 25.0f / 9.0f;

constexpr const int Uninitialized = 0;
constexpr const int Running = 1;
constexpr const int LoadMessagesWait1 = 2;
constexpr const int LoadMessagesWait2 = 3;

UiLogWindow::UiLogWindow (double windowWidth, double windowHeight)
: Panel ()
, windowWidth (windowWidth)
, windowHeight (windowHeight)
, stage (Uninitialized)
, isLoadDisabled (false)
, firstMessageLine (0)
, lastMessageLine (0)
, loadErrorIcon (NULL)
{
	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	headerImage = add (new ImageWindow ());
	headerImage->widgetName.assign ("uiLogWindowHeaderImage");
	headerImage->onLoadScale (windowWidth, windowWidth / headerImageAspectRatio);
	headerImage->setWindowSize (true, windowWidth, windowWidth / headerImageAspectRatio);
	headerImage->loadImageFile (StdString::createSprintf ("bg/uilog/%s/000.png", App::instance->imagePrefix.c_str ()));

	titleLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::ApplicationLog).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
	titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));

	closeButton = add (Ui::createScrimIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton), Widget::EventCallbackContext (UiLogWindow::closeButtonClicked, this), UiText::instance->getText (UiTextId::Close).capitalized (), "uiLogWindowCloseButton"), 1);

	clearButton = add (Ui::createScrimIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton), Widget::EventCallbackContext (UiLogWindow::clearButtonClicked, this), UiText::instance->getText (UiTextId::ClearLog).capitalized (), "uiLogWindowClearButton"), 1);

	view = (ScrollViewWindow *) addWidget (new ScrollViewWindow ());
	view->setViewPaddingScale (0.0f, 0.0f);
	view->setViewBottomPadding (UiConfiguration::instance->paddingSize);
	view->setViewLayout (Panel::DownFlowLayoutOption | Panel::LeftGravityLayoutOption, 0.0f);
	view->setScrollOptions (ScrollViewWindow::KeyboardScrollOption | ScrollViewWindow::MouseWheelScrollOption);
	view->setScrollBarPosition (ScrollViewWindow::RightEdgeScrollBarPosition);
	scrollViewWidth = view->scrollViewWidth;

	reflow ();
}
UiLogWindow::~UiLogWindow () {
}

void UiLogWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerImage->position.assign (0.0f, 0.0f);
	layoutFlow.y = headerImage->height - titleLabel->height - heightPadding;
	titleLabel->flowRight (&layoutFlow);
	clearButton->flowRight (&layoutFlow);
	closeButton->flowRight (&layoutFlow);

	layoutFlow.y = headerImage->height + heightPadding;
	view->setViewSize (windowWidth, height - layoutFlow.y);
	view->reflow ();
	view->position.assign (0.0f, layoutFlow.y);
	if (! FLOAT_EQUALS (scrollViewWidth, view->scrollViewWidth)) {
		scrollViewWidth = view->scrollViewWidth;
		view->processViewItems (UiLogWindow::reflow_processItem, this);
		view->reflow ();
	}
	if (loadErrorIcon) {
		loadErrorIcon->position.assign (view->position.x + (view->width / 2.0f) - (loadErrorIcon->width / 2.0f), view->position.y + (view->height / 2.0f) - (loadErrorIcon->height / 2.0f));
	}

	bottomRightLayoutFlow ();
	closeButton->flowLeft (&layoutFlow);
	clearButton->flowLeft (&layoutFlow);
}
void UiLogWindow::reflow_processItem (void *itPtr, Widget *widgetPtr) {
	UiLogWindow *it = (UiLogWindow *) itPtr;
	TextFlow *textflow = (TextFlow *) widgetPtr;

	textflow->setViewWidth (it->scrollViewWidth);
}

void UiLogWindow::closeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((UiLogWindow *) itPtr)->isDestroyed = true;
}

void UiLogWindow::clearButtonClicked (void *itPtr, Widget *widgetPtr) {
	UiLogWindow *it = (UiLogWindow *) itPtr;

	UiLog::instance->clear ();
	it->view->clearViewItems ();
	it->firstMessageLine = 0;
	it->lastMessageLine = 0;
}

void UiLogWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	switch (stage) {
		case Uninitialized: {
			stage = Running;
			break;
		}
		case Running: {
			if ((! isLoadDisabled) && (! UiLog::instance->isClearing) && view->isScrolledToBottom ()) {
				if (UiLog::instance->lastMessageLine > lastMessageLine) {
					stage = LoadMessagesWait1;
					retain ();
					TaskGroup::instance->run (TaskGroup::RunContext (UiLogWindow::loadMessages, this));
				}
			}
			break;
		}
		case LoadMessagesWait1: {
			break;
		}
		case LoadMessagesWait2: {
			if (isLoadDisabled) {
				if (loadErrorIcon) {
					loadErrorIcon->isDestroyed = true;
				}
				loadErrorIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon), UiText::instance->getText (UiTextId::DataLoadError).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->errorTextColor), 1);
			}
			reflow ();
			view->scrollToBottom ();
			stage = Running;
			break;
		}
	}
}

void UiLogWindow::loadMessages (void *itPtr) {
	UiLogWindow *it = (UiLogWindow *) itPtr;

	it->executeLoadMessages ();
	it->stage = LoadMessagesWait2;
	it->release ();
}
void UiLogWindow::executeLoadMessages () {
	StdString errmsg;
	std::list<UiLog::Message> msglist;
	std::list<UiLog::Message>::const_iterator i1, i2;
	std::list<TextFlow *> msgitems;
	std::list<TextFlow *>::const_iterator j1, j2;
	StdString text;
	TextFlow *textflow;

	if (! UiLog::instance->readRecords (&errmsg, &msglist, lastMessageLine, 1)) {
		isLoadDisabled = true;
		return;
	}
	i1 = msglist.cbegin ();
	i2 = msglist.cend ();
	while (i1 != i2) {
		textflow = new TextFlow (scrollViewWidth, UiConfiguration::ConsoleFont);
		textflow->setPaddingScale (1.0f, 0.0f);
		text.sprintf ("[%s] ", UiText::instance->getTimestampText (i1->createTime).c_str ());
		text.append (i1->text);
		textflow->setText (text);

		msgitems.push_back (textflow);
		if ((firstMessageLine <= 0) || (i1->line < firstMessageLine)) {
			firstMessageLine = i1->line;
		}
		if ((lastMessageLine <= 0) || (i1->line > lastMessageLine)) {
			lastMessageLine = i1->line;
		}
		++i1;
	}
	j1 = msgitems.cbegin ();
	j2 = msgitems.cend ();
	while (j1 != j2) {
		view->addViewItem (*j1);
		++j1;
	}
}
