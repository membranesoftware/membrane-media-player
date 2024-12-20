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
#include "ClassId.h"
#include "Prng.h"
#include "OsUtil.h"
#include "MediaControl.h"
#include "AppUrl.h"
#include "Font.h"
#include "Network.h"
#include "SharedBuffer.h"
#include "TaskGroup.h"
#include "HashMap.h"
#include "Json.h"
#include "SystemInterface.h"
#include "Log.h"
#include "AppNews.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextFlow.h"
#include "Image.h"
#include "Button.h"
#include "Toggle.h"
#include "ProgressBar.h"
#include "ProgressRing.h"
#include "HyperlinkWindow.h"
#include "PlayerUi.h"
#include "AppCardWindow.h"

constexpr const double windowWidthScale = 0.38f;
constexpr const double unexpandedTextWidthScale = 0.21f;
constexpr const double progressBarScale = 0.38f;
constexpr const int newsPostInfoTextPosition = 2;

AppCardWindow::AppCardWindow ()
: Panel ()
, isExpanded (false)
, isInitializing (false)
, currentTextId (-1)
, nextTextId (-1)
, isTextCrawlEnabled (true)
, isShowingUpdateRow (false)
, isShowingUpdateLink (false)
, isCheckingForUpdates (false)
, isUpdateNewsReceived (false)
, isLoadNewsReceived (false)
, isShowingNewsPosts (false)
, currentNewsPostId (-1)
, isInitializeShowingNewsPostsFirst (false)
, isInitializeCheckingForUpdates (false)
, loadNewsState (NULL)
, updateNewsState (NULL)
{
	ProgressRing *ring;

	classId = ClassId::AppCardWindow;
	SdlUtil::createMutex (&newsPostMutex);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	unexpandedTextWidth = App::instance->drawableWidth * unexpandedTextWidthScale;

	headerIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_appInfoIcon)));
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	nameLabel = add (new Label (UiText::instance->getText (UiTextId::AppInfo).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	progressRingPanel = add (new Panel (), 2);
	progressRingPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	ring = progressRingPanel->add (new ProgressRing (UiConfiguration::instance->progressRingSize));
	ring->setIndeterminate (true);
	progressRingPanel->setPaddingScale (0.1f, 0.1f);
	progressRingPanel->setLayout (Panel::RightFlowLayoutOption);
	progressRingPanel->isVisible = false;

	unexpandedTextLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor)));
	unexpandedTextLabel->setFixedPadding (true, 0.0f, 0.0f);

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->widgetName.assign ("appWindowExpandToggle");
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (AppCardWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isInputSuspended = true;
	dividerPanel->isVisible = false;

	versionIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_versionIcon)));
	versionIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	versionIcon->isVisible = false;

	versionText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	versionText->setFixedPadding (true, 0.0f, 0.0f);
	versionText->setTextColor (UiConfiguration::instance->lightPrimaryTextColor);
	versionText->setText (StdString::createSprintf ("%s %s\n%s %s", UiText::instance->getText (UiTextId::Version).capitalized ().c_str (), BUILD_ID, UiText::instance->getText (UiTextId::BuildDateText).c_str (), StdString (BUILD_DATE).replaced ("  ", " ").c_str ()));
	versionText->isVisible = false;

	updateIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_updateIcon)));
	updateIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	updateIcon->isVisible = false;

	updateProgressBar = add (new ProgressBar ());
	updateProgressBar->setSize (1.0f, UiConfiguration::instance->progressBarHeight);
	updateProgressBar->setIndeterminate (true);
	updateProgressBar->isVisible = false;

	updateText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	updateText->setFixedPadding (true, 0.0f, 0.0f);
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);
	updateText->isVisible = false;

	updateLink = (HyperlinkWindow *) addWidget (new HyperlinkWindow ());
	updateLink->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	updateLink->isVisible = false;

	infoIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_messageIcon)));
	infoIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	infoIcon->isVisible = false;

	infoText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	infoText->setFixedPadding (true, 0.0f, 0.0f);
	infoText->setTextColor (UiConfiguration::instance->primaryTextColor);
	infoText->isVisible = false;

	updateButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_updateButton), Widget::EventCallbackContext (AppCardWindow::updateButtonClicked, this), UiText::instance->getText (UiTextId::CheckForUpdates).capitalized (), "appWindowUpdateButton"));
	updateButton->isVisible = false;
	aboutButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_aboutButton), Widget::EventCallbackContext (AppCardWindow::aboutButtonClicked, this), UiText::instance->getText (UiTextId::About).capitalized (), "appWindowAboutButton"));
	aboutButton->isVisible = false;
	nextButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_nextItemButton), Widget::EventCallbackContext (AppCardWindow::nextButtonClicked, this), UiText::instance->getText (UiTextId::Next).capitalized (), "appWindowNextButton"));
	nextButton->isVisible = false;

	resetInfoText ();
	reflow ();
}
AppCardWindow::~AppCardWindow () {
	if (loadNewsState) {
		delete (loadNewsState);
		loadNewsState = NULL;
	}
	if (updateNewsState) {
		delete (updateNewsState);
		updateNewsState = NULL;
	}
	SdlUtil::destroyMutex (&newsPostMutex);
}

void AppCardWindow::resetInfoText () {
	bool configured;

	infoTextIds.clear ();
	configured = MediaControl::instance->isConfigured;
	if (! configured) {
		infoTextIds.append (UiTextId::AppInfoPrimeConfigureText);
	}
	infoTextIds.append (UiTextId::AppInfoText1);
	infoTextIds.append (UiTextId::AppInfoText2);
	infoTextIds.append (UiTextId::AppInfoText3);
	infoTextIds.append (UiTextId::AppInfoText4);
	infoTextIds.append (UiTextId::AppInfoText5);
	infoTextIds.append (UiTextId::AppInfoText6);
	infoTextIds.append (UiTextId::AppInfoText7);
	infoTextIds.append (UiTextId::AppInfoText8);
	infoTextIds.append (UiTextId::AppInfoText9);
	infoTextIds.append (UiTextId::AppInfoText10);
	infoTextIds.append (UiTextId::AppInfoText11);
	infoTextIds.append (UiTextId::AppInfoText12);
	infoTextIds.append (UiTextId::AppInfoText13);
	infoTextIds.append (UiTextId::AppInfoText14);
	infoTextIds.append (UiTextId::AppInfoText15);
	infoTextIds.append (UiTextId::AppInfoText16);

	currentTextId = -1;
	if (! configured) {
		nextTextId = 0;
	}
	else {
		nextTextId = Prng::instance->getRandomNumber (0, (int) infoTextIds.size () - 1);
	}
}

void AppCardWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPaddingScale (1.0f, 1.0f);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
	}
	else {
		setPaddingScale (0.5f, 0.5f);
		nameLabel->setFont (UiConfiguration::BodyFont);
	}
	if (isTextCrawlEnabled) {
		isTextCrawlEnabled = false;
		unexpandedTextLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (infoText->text, unexpandedTextWidth, Font::dotTruncateSuffix));
		unexpandedTextLabel->setCrawl (false);
	}

	unexpandedTextLabel->isVisible = (! isExpanded);
	dividerPanel->isVisible = isExpanded;
	versionIcon->isVisible = isExpanded;
	versionText->isVisible = isExpanded;
	infoIcon->isVisible = isExpanded;
	infoText->isVisible = isExpanded;
	updateButton->isVisible = isExpanded;
	aboutButton->isVisible = isExpanded;
	nextButton->isVisible = isExpanded;
	resetUpdateVisible ();
	expandToggle->setChecked (isExpanded, true);

	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void AppCardWindow::resetUpdateVisible () {
	updateIcon->isVisible = isExpanded && isShowingUpdateRow;
	updateProgressBar->isVisible = isExpanded && isShowingUpdateRow && isCheckingForUpdates;
	updateText->isVisible = isExpanded && isShowingUpdateRow;
	updateLink->isVisible = isExpanded && isShowingUpdateRow && isShowingUpdateLink;
}

void AppCardWindow::readNewsState (AppNews::NewsState *state) {
	int64_t now;
	std::list<AppNews::NewsPost>::const_iterator i1, i2;

	now = OsUtil::getTime ();
	SDL_LockMutex (newsPostMutex);
	newsPosts.clear ();
	i1 = state->posts.cbegin ();
	i2 = state->posts.cend ();
	while (i1 != i2) {
		if ((i1->endTime <= 0) || (now < i1->endTime)) {
			newsPosts.push_back (i1->body);
		}
		++i1;
	}
	SDL_UnlockMutex (newsPostMutex);

	if ((! state->updateBuildId.empty ()) && (state->updatePublishTime > 0)) {
		updateText->setText (UiText::instance->getText (UiTextId::UpdateFoundPrompt));
		updateLink->setLink (StdString::createSprintf ("%s, %s", state->updateBuildId.c_str (), UiText::instance->getDateText (state->updatePublishTime).c_str ()), AppUrl::instance->update (StdString::createSprintf ("%s_%s", BUILD_ID, PLATFORM_ID)));
		isShowingUpdateLink = true;
		isShowingUpdateRow = true;
	}
	else {
		updateText->setText (UiText::instance->getText (UiTextId::NoUpdatesPrompt));
		isShowingUpdateLink = false;
		isShowingUpdateRow = false;
	}
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);
	resetUpdateVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void AppCardWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerIcon->flowRight (&layoutFlow);
	progressRingPanel->position.assign (headerIcon->position.x + (headerIcon->width / 2.0f) - (progressRingPanel->width / 2.0f), headerIcon->position.y + (headerIcon->height / 2.0f) - (progressRingPanel->height / 2.0f));

	if (isExpanded) {
		nameLabel->flowDown (&layoutFlow);
		layoutFlow.y = heightPadding;
		nameLabel->centerVertical (&layoutFlow);
	}
	else {
		nameLabel->flowDown (&layoutFlow);
		unexpandedTextLabel->setWindowWidth (unexpandedTextWidth);
		unexpandedTextLabel->flowRight (&layoutFlow);
	}

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);

	nextRowLayoutFlow ();
	if (isExpanded) {
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		versionIcon->flowRight (&layoutFlow);
		versionText->setViewWidth (windowWidth - layoutFlow.xExtent - (widthPadding * 2.0f) - UiConfiguration::instance->marginSize);
		versionText->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		infoIcon->flowRight (&layoutFlow);
		infoText->setViewWidth (windowWidth - layoutFlow.xExtent - (widthPadding * 2.0f) - UiConfiguration::instance->marginSize);
		infoText->flowDown (&layoutFlow);

		if (updateIcon->isVisible) {
			nextRowLayoutFlow ();
			updateIcon->flowRight (&layoutFlow);
			updateText->setViewWidth (windowWidth - layoutFlow.xExtent - (widthPadding * 2.0f) - UiConfiguration::instance->marginSize);
			updateText->flowDown (&layoutFlow);

			if (updateProgressBar->isVisible) {
				layoutFlow.y -= (UiConfiguration::instance->marginSize / 2.0f);
				updateProgressBar->flowDown (&layoutFlow);
			}
			if (updateLink->isVisible) {
				updateLink->flowDown (&layoutFlow);
			}
		}

		updateButton->flowRight (&layoutFlow);
		aboutButton->flowRight (&layoutFlow);
		nextButton->flowRight (&layoutFlow);

		setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
		dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);

		bottomRightLayoutFlow ();
		nextButton->flowLeft (&layoutFlow);
		aboutButton->flowLeft (&layoutFlow);
		updateButton->flowLeft (&layoutFlow);
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	if (updateProgressBar->isVisible) {
		updateProgressBar->setSize (width * progressBarScale, UiConfiguration::instance->progressBarHeight);
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);
}

void AppCardWindow::doResize () {
	StdString text;

	Panel::doResize ();
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	unexpandedTextWidth = App::instance->drawableWidth * unexpandedTextWidthScale;
	unexpandedTextLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (infoText->text, unexpandedTextWidth, Font::dotTruncateSuffix));
	unexpandedTextLabel->setCrawl (false);
	isTextCrawlEnabled = false;
	reflow ();
}

void AppCardWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

void AppCardWindow::nextButtonClicked (void *itPtr, Widget *widgetPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	if (it->isInitializing) {
		return;
	}
	it->isTextCrawlEnabled = false;
	it->setNextInfoText ();
}
void AppCardWindow::setNextInfoText () {
	StdString text;
	Font::Metrics metrics;

	if (isShowingNewsPosts) {
		SDL_LockMutex (newsPostMutex);
		if (currentNewsPostId < 0) {
			currentNewsPostId = 0;
		}
		else {
			++currentNewsPostId;
		}
		if (currentNewsPostId >= (int) newsPosts.size ()) {
			isShowingNewsPosts = false;
		}
		else {
			text = newsPosts.at (currentNewsPostId);
		}
		SDL_UnlockMutex (newsPostMutex);
	}
	if (text.empty ()) {
		if (nextTextId >= 0) {
			currentTextId = nextTextId;
			nextTextId = -1;
		}
		else {
			if (currentTextId < 0) {
				currentTextId = 0;
			}
			else {
				++currentTextId;
				if (currentTextId >= (int) infoTextIds.size ()) {
					currentTextId = 0;
				}
			}
		}
		text = UiText::instance->getText (infoTextIds.at (currentTextId));

		if (currentTextId == newsPostInfoTextPosition) {
			SDL_LockMutex (newsPostMutex);
			if (! newsPosts.empty ()) {
				isShowingNewsPosts = true;
				currentNewsPostId = -1;
			}
			SDL_UnlockMutex (newsPostMutex);
		}
	}
	infoText->setText (text);

	if (isTextCrawlEnabled) {
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, text);
		unexpandedTextLabel->setText (text);
		if (metrics.textWidth > unexpandedTextWidth) {
			unexpandedTextLabel->setCrawl (true);
		}
	}
	else {
		unexpandedTextLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (text, unexpandedTextWidth, Font::dotTruncateSuffix));
		unexpandedTextLabel->setCrawl (false);
	}

	if (isShowingUpdateRow && (! isCheckingForUpdates) && (! isShowingUpdateLink)) {
		isShowingUpdateRow = false;
		resetUpdateVisible ();
	}
	reflow ();
	eventCallback (layoutChangeCallback);
}

void AppCardWindow::aboutButtonClicked (void *itPtr, Widget *widgetPtr) {
	int result;

	result = OsUtil::openUrl (AppUrl::instance->help (AppUrl::AboutApplication, true));
	if (result != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenAboutUrlError));
	}
	else {
		App::instance->showNotification (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextId::LaunchedWebBrowser).capitalized ().c_str (), AppUrl::instance->help (AppUrl::AboutApplication, false).c_str ()));
	}
}

void AppCardWindow::initialize (bool showNewsPostsFirst, bool executeCheckForUpdates) {
	isInitializeShowingNewsPostsFirst = showNewsPostsFirst;
	isInitializeCheckingForUpdates = executeCheckForUpdates;
	if (loadNewsState) {
		delete (loadNewsState);
	}
	loadNewsState = new AppNews::NewsState ();
	isLoadNewsReceived = false;
	isInitializing = true;
	updateButton->setDisabled (true);
	retain ();
	App::instance->addUpdateTask (AppCardWindow::awaitAppNewsReady, this);
}
void AppCardWindow::endInitialize () {
	isInitializeCheckingForUpdates = false;
	if (loadNewsState) {
		delete (loadNewsState);
		loadNewsState = NULL;
	}
	updateButton->setDisabled (false);
	isInitializing = false;
}
void AppCardWindow::awaitAppNewsReady (void *itPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->executeAwaitAppNewsReady ();
	it->release ();
}
void AppCardWindow::executeAwaitAppNewsReady () {
	if (AppNews::instance->isLoadFailed || App::instance->isShuttingDown) {
		endInitialize ();
		return;
	}
	if (! MediaControl::instance->isConfigured) {
		retain ();
		App::instance->addUpdateTask (AppCardWindow::showLoadResult, this);
		return;
	}
	if (! AppNews::instance->isReady) {
		retain ();
		App::instance->addUpdateTask (AppCardWindow::awaitAppNewsReady, this);
		return;
	}
	retain ();
	TaskGroup::instance->run (TaskGroup::RunContext (AppCardWindow::readAppNewsRecord, this));
}
void AppCardWindow::readAppNewsRecord (void *itPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->executeReadAppNewsRecord ();
	it->release ();
}
void AppCardWindow::executeReadAppNewsRecord () {
	HashMap *prefs;

	isLoadNewsReceived = AppNews::instance->readRecord (loadNewsState);
	if (isLoadNewsReceived) {
		prefs = App::instance->lockPrefs ();
		prefs->insert (PlayerUi::showAppNewsKey, (! isInitializeShowingNewsPostsFirst), false);
		App::instance->unlockPrefs ();
	}
	retain ();
	App::instance->addUpdateTask (AppCardWindow::showLoadResult, this);
}
void AppCardWindow::showLoadResult (void *itPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->executeShowLoadResult ();
	it->release ();
}
void AppCardWindow::executeShowLoadResult () {
	if (loadNewsState && isLoadNewsReceived) {
		readNewsState (loadNewsState);
		if (isInitializeShowingNewsPostsFirst) {
			SDL_LockMutex (newsPostMutex);
			if (! newsPosts.empty ()) {
				isShowingNewsPosts = true;
				currentNewsPostId = -1;
			}
			SDL_UnlockMutex (newsPostMutex);
		}
	}
	setNextInfoText ();
	if (isInitializeCheckingForUpdates) {
		isInitializeCheckingForUpdates = false;
		checkForUpdates ();
	}
	endInitialize ();
}

void AppCardWindow::updateButtonClicked (void *itPtr, Widget *widgetPtr) {
	((AppCardWindow *) itPtr)->checkForUpdates ();
}
void AppCardWindow::checkForUpdates () {
	StdString url, postdata;
	StringList headers;
	Json *cmd;

	if (isCheckingForUpdates || isInitializeCheckingForUpdates) {
		return;
	}
	cmd = SystemInterface::instance->createCommand (SystemInterface::Prefix (), SystemInterface::Command_GetApplicationNews, (new Json ())->set (SystemInterface::Field_buildId, StdString::createSprintf ("%s_%s_%s", BUILD_ID, PLATFORM_ID, App::instance->language.c_str ())));
	if (! cmd) {
		Log::debug ("Check for update failed: error creating update request");
		return;
	}

	isShowingUpdateRow = true;
	isShowingUpdateLink = false;
	if (updateNewsState) {
		delete (updateNewsState);
	}
	updateNewsState = new AppNews::NewsState ();
	isCheckingForUpdates = true;
	isUpdateNewsReceived = false;
	updateText->setText (UiText::instance->getText (UiTextId::CheckingForUpdates).capitalized ());
	updateText->setTextColor (UiConfiguration::instance->statusOkTextColor);
	progressRingPanel->isVisible = true;
	resetUpdateVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);

	postdata.sprintf ("%s=", SystemInterface::Constant_CommandPostFormKey);
	postdata.append (cmd->toString ().urlEncoded ());
	delete (cmd);

	url = AppUrl::instance->news ();
	headers.append (Network::HttpContentTypeHeader, Network::WwwFormUrlencodedContentType);
	headers.append (Network::HttpAcceptHeader, Network::ApplicationJsonContentType);
	headers.append (Network::HttpHostHeader, AppUrl::instance->requestHost.c_str ());

	retain ();
	Network::instance->sendHttpPost (url, postdata, Network::HttpRequestCallbackContext (AppCardWindow::updateRequestComplete, this), headers);
}
void AppCardWindow::updateRequestComplete (void *itPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->receiveUpdateResponse (statusCode, responseData);
	it->release ();
}
void AppCardWindow::receiveUpdateResponse (int statusCode, SharedBuffer *responseData) {
	StdString cmd;

	if (responseData && (responseData->length > 0)) {
		cmd.assignBuffer (responseData);
		if (! AppNews::instance->parseCommand (cmd, updateNewsState)) {
			Log::debug ("Check for update failed: invalid server response");
		}
		else {
			AppNews::instance->writeRecord (cmd);
			isUpdateNewsReceived = true;
		}
	}
	retain ();
	App::instance->addUpdateTask (AppCardWindow::showUpdateResult, this);
}
void AppCardWindow::showUpdateResult (void *itPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->executeShowUpdateResult ();
	it->release ();
}
void AppCardWindow::executeShowUpdateResult () {
	if ((! isUpdateNewsReceived) || (! updateNewsState)) {
		updateText->setText (UiText::instance->getText (UiTextId::UpdateErrorPrompt));
	}
	else {
		readNewsState (updateNewsState);
	}

	if (updateNewsState) {
		delete (updateNewsState);
		updateNewsState = NULL;
	}
	isCheckingForUpdates = false;
	isShowingUpdateRow = true;
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);
	progressRingPanel->isVisible = false;
	resetUpdateVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}
