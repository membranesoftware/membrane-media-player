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
#include "OsUtil.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Ui.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "AppUrl.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextFlow.h"
#include "Button.h"
#include "Image.h"
#include "ImageWindow.h"
#include "ScrollViewWindow.h"
#include "HelpActionWindow.h"
#include "HyperlinkWindow.h"
#include "HelpWindow.h"

constexpr const double headerImageAspectRatio = 25.0f / 9.0f;

HelpWindow::HelpWindow (double windowWidth, double windowHeight)
: Panel ()
, windowWidth (windowWidth)
, windowHeight (windowHeight)
{
	setFixedSize (true, windowWidth, windowHeight);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	headerImage = add (new ImageWindow ());
	headerImage->widgetName.assign ("helpWindowHeaderImage");
	headerImage->onLoadScale (windowWidth, windowWidth / headerImageAspectRatio);
	headerImage->setWindowSize (true, windowWidth, windowWidth / headerImageAspectRatio);
	headerImage->loadImageFile (StdString::createSprintf ("bg/help/%s/000.png", App::instance->imagePrefix.c_str ()));

	titleLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::Help).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
	titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));

	closeButton = add (Ui::createScrimIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton), Widget::EventCallbackContext (HelpWindow::closeButtonClicked, this), UiText::instance->getText (UiTextId::Close).capitalized (), "helpWindowCloseButton"), 1);

	scrollView = (ScrollViewWindow *) addWidget (new ScrollViewWindow ());
	scrollView->setScrollOptions (ScrollViewWindow::KeyboardScrollOption | ScrollViewWindow::MouseWheelScrollOption | ScrollViewWindow::ExitedMouseWheelScrollOption);
	scrollView->setScrollBarPosition (ScrollViewWindow::RightEdgeScrollBarPosition);

	helpTitleLabel = (Label *) scrollView->addViewItem (new Label (StdString (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	helpText = (TextFlow *) scrollView->addViewItem (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	helpText->setTextColor (UiConfiguration::instance->primaryTextColor);
	helpText->setFillBg (true, UiConfiguration::instance->lightSecondaryColor.copy (UiConfiguration::instance->scrimBackgroundAlpha));

	linkTitleLabel = (Label *) scrollView->addViewItem (new Label (UiText::instance->getText (UiTextId::RelatedHelpTopics).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	linkTitleLabel->isVisible = false;
	linkIconImage = (Image *) scrollView->addViewItem (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_webLinkIcon)));
	linkIconImage->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	linkIconImage->isVisible = false;
	linkPanel = (Panel *) scrollView->addViewItem (new Panel ());
	linkPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	linkPanel->layoutSpacing = (UiConfiguration::instance->textLineHeightMargin * 2.0f);
	linkPanel->setLayout (Panel::DownFlowLayoutOption);
	linkPanel->isVisible = false;

	buttonPanel = add (new Panel ());
	feedbackButton = buttonPanel->add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_feedbackButton), Widget::EventCallbackContext (HelpWindow::feedbackButtonClicked, this), UiText::instance->getText (UiTextId::SendFeedback).capitalized (), "helpWindowFeedbackButton"));
	searchButton = buttonPanel->add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_helpButton), Widget::EventCallbackContext (HelpWindow::searchButtonClicked, this), UiText::instance->getText (UiTextId::SearchForHelp).capitalized (), "helpWindowSearchButton"));
	buttonPanel->setFixedPadding (true, 0.0f, 0.0f);
	buttonPanel->setLayout (Panel::RightFlowLayoutOption | Panel::DownGravityLayoutOption);

	reflow ();
}
HelpWindow::~HelpWindow () {
}

void HelpWindow::setHelpText (const StdString &title, const StdString &text) {
	helpTitleLabel->setText (title);
	helpText->setText (text);
	reflow ();
}

void HelpWindow::addAction (const StdString &actionText, int actionType, const StdString &linkText, const StdString &linkUrl, const StdString &linkDisplayUrl) {
	HelpActionWindow *action;
	Sprite *iconsprite;
	Color textcolor;

	switch (actionType) {
		case HelpWindow::ErrorAction: {
			iconsprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon);
			textcolor.assign (UiConfiguration::instance->errorTextColor);
			break;
		}
		default: {
			iconsprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_infoIcon);
			textcolor.assign (UiConfiguration::instance->primaryTextColor);
			break;
		}
	}

	action = (HelpActionWindow *) scrollView->addViewItem (new HelpActionWindow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), iconsprite, actionText, textcolor, linkText, linkUrl, linkDisplayUrl));
	actionList.push_back (action);
	reflow ();
}

void HelpWindow::addTopicLink (const StdString &linkText, const StdString &topicId) {
	HyperlinkWindow *linkwindow;

	linkwindow = (HyperlinkWindow *) linkPanel->addWidget (new HyperlinkWindow (linkText, AppUrl::instance->help (topicId, true)));
	linkwindow->linkOpenCallback = Widget::EventCallbackContext (HelpWindow::topicLinkOpened, this);
	linkwindow->itemId.assign (topicId);
	linkwindow->setFixedPadding (true, 0.0f, 0.0f);
	linkwindow->reflow ();
	linkTitleLabel->isVisible = true;
	linkIconImage->isVisible = true;
	linkPanel->isVisible = true;
	reflow ();
}

void HelpWindow::topicLinkOpened (void *itPtr, Widget *widgetPtr) {
	HyperlinkWindow *hyperlink = (HyperlinkWindow *) widgetPtr;

	if (hyperlink->linkOpenResult != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenHelpUrlError));
	}
	else {
		App::instance->showNotification (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextId::LaunchedWebBrowser).capitalized ().c_str (), AppUrl::instance->help (hyperlink->itemId, false).c_str ()));
	}
}

void HelpWindow::reflow () {
	std::list<HelpActionWindow *>::iterator i1, i2;
	HelpActionWindow *action;
	double y;

	resetPadding ();
	topLeftLayoutFlow ();

	headerImage->position.assign (0.0f, 0.0f);
	titleLabel->position.assign (widthPadding, headerImage->height - titleLabel->height - heightPadding);
	closeButton->position.assign (windowWidth - closeButton->width - widthPadding, headerImage->height - closeButton->height - heightPadding);

	layoutFlow.y = headerImage->height + heightPadding + UiConfiguration::instance->marginSize;
	buttonPanel->reflow ();
	scrollView->position.assign (0.0f, layoutFlow.y);
	scrollView->setViewSize (windowWidth, height - layoutFlow.y - buttonPanel->height - heightPadding - UiConfiguration::instance->marginSize);

	topLeftLayoutFlow ();
	helpTitleLabel->flowDown (&layoutFlow);
	helpText->flowDown (&layoutFlow);

	i1 = actionList.begin ();
	i2 = actionList.end ();
	while (i1 != i2) {
		action = *i1;
		action->flowDown (&layoutFlow);
		++i1;
	}

	layoutFlow.y += (UiConfiguration::instance->marginSize * 2.0f);
	if (linkTitleLabel->isVisible) {
		linkTitleLabel->flowDown (&layoutFlow);
	}
	if (linkIconImage->isVisible) {
		linkIconImage->flowDown (&layoutFlow);
	}
	if (linkPanel->isVisible) {
		linkPanel->reflow ();
		layoutFlow.y = linkIconImage->position.y;
		layoutFlow.x += linkIconImage->width + UiConfiguration::instance->marginSize;
		linkPanel->flowDown (&layoutFlow);
	}
	scrollView->reflow ();

	helpText->setViewWidth (scrollView->scrollViewWidth - (widthPadding * 2.0f));
	i1 = actionList.begin ();
	i2 = actionList.end ();
	while (i1 != i2) {
		action = *i1;
		action->setWindowWidth (scrollView->scrollViewWidth - (widthPadding * 2.0f));
		++i1;
	}

	y = windowHeight - heightPadding;
	y -= buttonPanel->height;
	buttonPanel->position.assign (windowWidth - widthPadding - buttonPanel->width, y);
}

void HelpWindow::closeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((HelpWindow *) itPtr)->isDestroyed = true;
}

void HelpWindow::feedbackButtonClicked (void *itPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (AppUrl::instance->feedback (true));
	if (result != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenFeedbackUrlError));
	}
	else {
		App::instance->showNotification (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextId::LaunchedWebBrowser).capitalized ().c_str (), AppUrl::instance->feedback ().c_str ()));
	}
}

void HelpWindow::searchButtonClicked (void *itPtr, Widget *widgetPtr) {
	StdString url;
	int result;

	url.assign (AppUrl::instance->help (AppUrl::HelpIndex));
	result = OsUtil::openUrl (url);
	if (result != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenAboutUrlError));
	}
	else {
		App::instance->showNotification (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextId::LaunchedWebBrowser).capitalized ().c_str (), url.c_str ()));
	}
}
