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
#include "MediaUtil.h"
#include "AppUrl.h"
#include "Font.h"
#include "Network.h"
#include "SharedBuffer.h"
#include "TaskGroup.h"
#include "SequenceList.h"
#include "HashMap.h"
#include "Json.h"
#include "PrefsKey.h"
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
#include "VideoCycleWindow.h"
#include "ProgressBar.h"
#include "ProgressRing.h"
#include "HyperlinkWindow.h"
#include "AppCardWindow.h"

constexpr const double windowWidthScale = 0.38f;
constexpr const double unexpandedTextWidthScale = 0.55f;
constexpr const double progressBarScale = 0.38f;
constexpr const int newsPostInfoTextPosition = 2;
constexpr const char *primeVideoPath = "ui/PlayerUi/video/";
constexpr const double primeVideoWidthScale = 0.715f;
constexpr const int primeVideoCount = 6;

AppCardWindow::AppCardWindow (SpriteGroup *playerUiSpriteGroup, bool startupPrimePanel)
: Panel ()
, isExpanded (false)
, isInitializing (false)
, playerUiSpriteGroup (playerUiSpriteGroup)
, startupPrimePanel (startupPrimePanel)
, currentTextId (-1)
, nextTextId (-1)
, isTextCrawlEnabled (true)
, isShowingInfoText (false)
, isShowingPrimePanel (false)
, isShowingUpdateRow (false)
, isShowingUpdateLink (false)
, isCheckingForUpdates (false)
, isUpdateNewsReceived (false)
, isLoadNewsReceived (false)
, isShowingNewsPosts (false)
, currentNewsPostId (-1)
, isInitializeShowingNewsPostsFirst (false)
, isInitializeCheckingForUpdates (false)
, updateAppNews (NULL)
, primeVideoHandle (&primeVideo)
{
	ProgressRing *ring;

	classId = ClassId::AppCardWindow;
	SdlUtil::createMutex (&newsPostMutex);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;

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
	expandToggle->widgetName.assign ("appInfoExpandToggle");
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (AppCardWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isInputSuspended = true;

	versionIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_versionIcon)));
	versionIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	versionText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	versionText->setFixedPadding (true, 0.0f, 0.0f);
	versionText->setTextColor (UiConfiguration::instance->lightPrimaryTextColor);
	versionText->setText (StdString::createSprintf ("%s %s\n%s %s", UiText::instance->getText (UiTextId::Version).capitalized ().c_str (), BUILD_ID, UiText::instance->getText (UiTextId::BuildDateText).c_str (), StdString (BUILD_DATE).replaced ("  ", " ").c_str ()));

	updateIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_updateIcon)));
	updateIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	updateProgressBar = add (new ProgressBar ());
	updateProgressBar->setSize (1.0f, UiConfiguration::instance->progressBarHeight);
	updateProgressBar->setIndeterminate (true);

	updateText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	updateText->setFixedPadding (true, 0.0f, 0.0f);
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);

	updateLink = (HyperlinkWindow *) addWidget (new HyperlinkWindow ());
	updateLink->linkOpenCallback = Widget::EventCallbackContext (AppCardWindow::updateLinkOpened, this);
	updateLink->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);

	unexpandedUpdateIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_updateIcon)), 1);
	unexpandedUpdateIcon->setDrawColor (true, UiConfiguration::instance->statusOkTextColor);
	unexpandedUpdateIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::UpdateFoundPrompt));
	unexpandedUpdateIcon->isVisible = false;

	infoIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_messageIcon)));
	infoIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	infoText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	infoText->setFixedPadding (true, 0.0f, 0.0f);
	infoText->setTextColor (UiConfiguration::instance->primaryTextColor);

	updateButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_updateButton), Widget::EventCallbackContext (AppCardWindow::updateButtonClicked, this), UiText::instance->getText (UiTextId::CheckForUpdates).capitalized (), "appInfoUpdateButton"));
	aboutButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_aboutButton), Widget::EventCallbackContext (AppCardWindow::aboutButtonClicked, this), UiText::instance->getText (UiTextId::About).capitalized (), "appInfoAboutButton"));
	nextButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_nextItemButton), Widget::EventCallbackContext (AppCardWindow::nextButtonClicked, this), UiText::instance->getText (UiTextId::Next).capitalized (), "appInfoNextButton"));
	nextButton->setDisabled (true);

	primePanel = add (new Panel ());
	primePanel->setFixedPadding (true, 0.0f, 0.0f);

	primeButton = add (Ui::createIconButton (playerUiSpriteGroup->getSprite (SpriteId::PlayerUi_primeButton), Widget::EventCallbackContext (AppCardWindow::primeButtonClicked, this), UiText::instance->getText (UiTextId::AppInfoPrimeTooltip), "appInfoPrimeButton"));

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (AppCardWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::AppInfoBackTooltip), "appInfoBackButton"));

	resetInfoText ();
	resetUnexpandedInfoText ();
	isShowingPrimePanel = startupPrimePanel;
	resetControlsVisible ();
	reflow ();
}
AppCardWindow::~AppCardWindow () {
	if (primeVideo) {
		primeVideo->stop ();
		primeVideoHandle.destroyAndClear ();
	}
	if (updateAppNews) {
		delete (updateAppNews);
		updateAppNews = NULL;
	}
	SdlUtil::destroyMutex (&newsPostMutex);
}

AppCardWindow *AppCardWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::AppCardWindow) ? (AppCardWindow *) widget : NULL);
}

void AppCardWindow::resetInfoText () {
	infoTextIds.clear ();
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
	infoTextIds.append (UiTextId::AppInfoText17);

	currentTextId = -1;
	nextTextId = Prng::instance->getRandomNumber (0, (int) infoTextIds.size () - 1);
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
	resetControlsVisible ();
	if (isTextCrawlEnabled) {
		isTextCrawlEnabled = false;
		resetUnexpandedInfoText ();
	}
	expandToggle->setChecked (isExpanded, true);
	if (isExpanded && isShowingPrimePanel) {
		populatePrimePanel ();
	}
	else {
		if (primeVideo) {
			primeVideo->stop ();
			primeVideoHandle.clear ();
		}
		primePanel->clear ();
	}
	reflow ();

	if (! isExpanded) {
		clearStartupPrimePanel ();
	}
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void AppCardWindow::resetUnexpandedInfoText () {
	Font::Metrics metrics;
	double w;

	w = windowWidth * unexpandedTextWidthScale;
	if (isShowingUpdateRow && isShowingUpdateLink) {
		w -= (unexpandedUpdateIcon->width + (UiConfiguration::instance->marginSize * 2.0f));
	}
	unexpandedTextLabel->setWindowWidth (w);
	if (isTextCrawlEnabled) {
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, infoText->text);
		unexpandedTextLabel->setText (infoText->text);
		if (metrics.textWidth > w) {
			unexpandedTextLabel->setCrawl (true);
		}
		else {
			unexpandedTextLabel->setCrawl (false);
		}
	}
	else {
		unexpandedTextLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (infoText->text, w, Font::dotTruncateSuffix));
		unexpandedTextLabel->setCrawl (false);
	}
}

void AppCardWindow::resetControlsVisible () {
	unexpandedTextLabel->isVisible = (! isExpanded);
	dividerPanel->isVisible = isExpanded;
	updateButton->isVisible = isExpanded && (! isShowingPrimePanel);
	aboutButton->isVisible = isExpanded && (! isShowingPrimePanel);
	nextButton->isVisible = isExpanded && (! isShowingPrimePanel);
	primeButton->isVisible = isExpanded && (! isShowingPrimePanel);
	backButton->isVisible = isExpanded && isShowingPrimePanel;
	versionIcon->isVisible = isExpanded && (! isShowingPrimePanel);
	versionText->isVisible = isExpanded && (! isShowingPrimePanel);
	infoIcon->isVisible = isExpanded && isShowingInfoText && (! isShowingPrimePanel);
	infoText->isVisible = isExpanded && isShowingInfoText && (! isShowingPrimePanel);
	primePanel->isVisible = isExpanded && isShowingPrimePanel;
	updateIcon->isVisible = isExpanded && isShowingUpdateRow && (! isShowingPrimePanel);
	updateProgressBar->isVisible = isExpanded && isShowingUpdateRow && isCheckingForUpdates && (! isShowingPrimePanel);
	updateText->isVisible = isExpanded && isShowingUpdateRow && (! isShowingPrimePanel);
	updateLink->isVisible = isExpanded && isShowingUpdateRow && isShowingUpdateLink && (! isShowingPrimePanel);
	unexpandedUpdateIcon->isVisible = (! isExpanded) && isShowingUpdateRow && isShowingUpdateLink;
}

void AppCardWindow::readAppNews (AppNews *appNews) {
	int64_t now;
	std::list<AppNews::NewsPost>::const_iterator i1, i2;

	now = OsUtil::getTime ();
	SDL_LockMutex (newsPostMutex);
	newsPosts.clear ();
	i1 = appNews->posts.cbegin ();
	i2 = appNews->posts.cend ();
	while (i1 != i2) {
		if ((i1->endTime <= 0) || (now < i1->endTime)) {
			newsPosts.push_back (i1->body);
		}
		++i1;
	}
	SDL_UnlockMutex (newsPostMutex);

	if ((! appNews->updateBuildId.empty ()) && (appNews->updatePublishTime > 0) && appNews->recordBuildId.equals (BUILD_ID)) {
		updateText->setText (UiText::instance->getText (UiTextId::UpdateFoundPrompt));
		updateLink->setLink (StdString::createSprintf ("%s, %s", appNews->updateBuildId.c_str (), UiText::instance->getDateText (appNews->updatePublishTime).c_str ()), AppUrl::instance->update (StdString::createSprintf ("%s_%s", BUILD_ID, PLATFORM_ID)));
		isShowingUpdateLink = true;
		isShowingUpdateRow = true;
	}
	else {
		updateText->setText (UiText::instance->getText (UiTextId::NoUpdatesPrompt));
		isShowingUpdateLink = false;
		isShowingUpdateRow = false;
	}
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);
	resetControlsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void AppCardWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerIcon->flowRight (&layoutFlow);
	progressRingPanel->position.assign (headerIcon->position.x + (headerIcon->width / 2.0f) - (progressRingPanel->width / 2.0f), headerIcon->position.y + (headerIcon->height / 2.0f) - (progressRingPanel->height / 2.0f));
	if (unexpandedUpdateIcon->isVisible) {
		unexpandedUpdateIcon->flowRight (&layoutFlow);
	}

	nameLabel->flowDown (&layoutFlow);
	if (isExpanded) {
		layoutFlow.y = heightPadding;
		nameLabel->centerVertical (&layoutFlow);
	}
	else {
		unexpandedTextLabel->flowRight (&layoutFlow);
	}

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);

	nextRowLayoutFlow ();
	if (isExpanded) {
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);

		if (versionIcon->isVisible && versionText->isVisible) {
			nextRowLayoutFlow ();
			versionIcon->flowRight (&layoutFlow);
			versionText->setViewWidth (windowWidth - layoutFlow.xExtent - (widthPadding * 2.0f) - UiConfiguration::instance->marginSize);
			versionText->flowDown (&layoutFlow);
		}
		if (infoIcon->isVisible && infoText->isVisible) {
			nextRowLayoutFlow ();
			infoIcon->flowRight (&layoutFlow);
			infoText->setViewWidth (windowWidth - layoutFlow.xExtent - (widthPadding * 2.0f) - UiConfiguration::instance->marginSize);
			infoText->flowDown (&layoutFlow);
		}
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
		if (primePanel->isVisible) {
			nextRowLayoutFlow ();
			primePanel->reflow ();
			primePanel->flowDown (&layoutFlow);
		}

		nextRowLayoutFlow ();
		if (primeButton->isVisible) {
			primeButton->flowRight (&layoutFlow);
		}
		if (backButton->isVisible) {
			backButton->flowRight (&layoutFlow);
		}
		if (updateButton->isVisible) {
			updateButton->flowRight (&layoutFlow);
		}
		if (aboutButton->isVisible) {
			aboutButton->flowRight (&layoutFlow);
		}
		if (nextButton->isVisible) {
			nextButton->flowRight (&layoutFlow);
		}

		setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
		dividerPanel->setFixedSize (true, windowWidth, UiConfiguration::instance->headlineDividerLineWidth);

		bottomRightLayoutFlow ();
		if (nextButton->isVisible) {
			nextButton->flowLeft (&layoutFlow);
		}
		if (aboutButton->isVisible) {
			aboutButton->flowLeft (&layoutFlow);
		}
		if (updateButton->isVisible) {
			updateButton->flowLeft (&layoutFlow);
		}
		if (backButton->isVisible) {
			backButton->flowLeft (&layoutFlow);
		}
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	if (updateProgressBar->isVisible) {
		updateProgressBar->setSize (width * progressBarScale, UiConfiguration::instance->progressBarHeight);
	}
	if (unexpandedUpdateIcon->isVisible) {
		unexpandedUpdateIcon->position.assignY (headerIcon->position.y + (headerIcon->height / 2.0f) - (unexpandedUpdateIcon->height / 2.0f));
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);
}

void AppCardWindow::doResize () {
	StdString text;

	Panel::doResize ();
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	isTextCrawlEnabled = false;
	resetUnexpandedInfoText ();
	if (isExpanded && isShowingPrimePanel) {
		populatePrimePanel ();
	}
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
	if (isShowingUpdateRow && (! isCheckingForUpdates) && (! isShowingUpdateLink)) {
		isShowingUpdateRow = false;
		resetControlsVisible ();
	}
	resetUnexpandedInfoText ();
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
	isInitializing = true;
	isLoadNewsReceived = false;
	updateButton->setDisabled (true);
	retain ();
	App::instance->addUpdateTask (AppCardWindow::awaitMediaControlReady, this);
}
void AppCardWindow::awaitMediaControlReady (void *itPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	if (App::instance->isShuttingDown || it->isDestroyed) {
		it->release ();
		return;
	}
	if (! MediaControl::instance->isReady) {
		App::instance->addUpdateTask (AppCardWindow::awaitMediaControlReady, it);
		return;
	}
	it->endInitialize ();
	it->release ();
}
void AppCardWindow::endInitialize () {
	HashMap *prefs;

	if (! MediaControl::instance->appNews.recordBuildId.empty ()) {
		isLoadNewsReceived = true;
	}
	readAppNews (&(MediaControl::instance->appNews));
	if (isInitializeShowingNewsPostsFirst) {
		SDL_LockMutex (newsPostMutex);
		if (! newsPosts.empty ()) {
			isShowingNewsPosts = true;
			currentNewsPostId = -1;
		}
		SDL_UnlockMutex (newsPostMutex);
	}
	setNextInfoText ();

	if (isLoadNewsReceived) {
		prefs = App::instance->lockPrefs ();
		prefs->insert (PrefsKey::showAppNews, (! isInitializeShowingNewsPostsFirst), false);
		App::instance->unlockPrefs ();
	}

	if (isInitializeCheckingForUpdates) {
		isInitializeCheckingForUpdates = false;
		checkForUpdates ();
	}
	updateButton->setDisabled (false);
	nextButton->setDisabled (false);
	isShowingInfoText = true;
	resetControlsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
	isInitializing = false;
}

void AppCardWindow::populatePrimePanel () {
	Panel *panel;
	TextFlow *text;
	Image *image;
	double w;
	int i, firstindex;

	if (primeVideo) {
		primeVideo->stop ();
		primeVideoHandle.destroyAndClear ();
	}
	primePanel->clear ();

	w = windowWidth * primeVideoWidthScale;
	primeVideoHandle.assign (new VideoCycleWindow (floor (w), floor (w / MediaUtil::defaultAspectRatio)));
	panel = primePanel->add (new Panel ());
	panel->setFixedPadding (true, 0.0f, 0.0f);
	panel->addWidget (primeVideo);
	panel->setFixedSize (true, windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), primeVideo->height);
	primeVideo->position.assignX ((panel->width - primeVideo->width) / 2.0f);

	if (startupPrimePanel) {
		firstindex = 1;
	}
	else {
		firstindex = Prng::instance->getRandomNumber (1, primeVideoCount);
	}
	primeVideo->addPlayPath (StdString::createSprintf ("%s%i.mp4", primeVideoPath, firstindex));
	for (i = 1; i <= primeVideoCount; ++i) {
		if (i == firstindex) {
			continue;
		}
		primeVideo->addPlayPath (StdString::createSprintf ("%s%i.mp4", primeVideoPath, i));
	}
	primeVideo->play ();

	w = windowWidth - (UiConfiguration::instance->paddingSize * 2.0f);
	text = primePanel->add (new TextFlow (w, UiConfiguration::CaptionFont));
	text->setText (UiText::instance->getText (UiTextId::AppInfoPrimeText1));

	w = windowWidth - (UiConfiguration::instance->paddingSize * 3.0f);
	panel = primePanel->add (new Panel ());
	panel->setPaddingScale (0.5f, 0.0f);
	panel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	image = panel->add (new Image (playerUiSpriteGroup->getSprite (SpriteId::PlayerUi_playMediaButton)));
	image->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	text = panel->add (new TextFlow (w - image->width - UiConfiguration::instance->marginSize, UiConfiguration::CaptionFont));
	text->setText (UiText::instance->getText (UiTextId::AppInfoPrimeText2));
	panel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	panel = primePanel->add (new Panel ());
	panel->setPaddingScale (0.5f, 0.0f);
	panel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	image = panel->add (new Image (playerUiSpriteGroup->getSprite (SpriteId::PlayerUi_smallMediaOptionIcon)));
	image->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	text = panel->add (new TextFlow (w - image->width - UiConfiguration::instance->marginSize, UiConfiguration::CaptionFont));
	text->setText (UiText::instance->getText (UiTextId::AppInfoPrimeText3));
	panel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	primePanel->layoutSpacing = UiConfiguration::instance->marginSize / 2.0f;
	primePanel->setLayout (Panel::DownFlowLayoutOption);
}

void AppCardWindow::primeButtonClicked (void *itPtr, Widget *widgetPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->populatePrimePanel ();
	it->isShowingPrimePanel = true;
	it->resetControlsVisible ();
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void AppCardWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	AppCardWindow *it = (AppCardWindow *) itPtr;

	it->isShowingPrimePanel = false;
	it->resetControlsVisible ();
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
	if (it->primeVideo) {
		it->primeVideo->stop ();
		it->primeVideoHandle.destroyAndClear ();
	}
	it->primePanel->clear ();
	it->clearStartupPrimePanel ();
}

void AppCardWindow::clearStartupPrimePanel () {
	HashMap *prefs;

	if (startupPrimePanel) {
		prefs = App::instance->lockPrefs ();
		prefs->insert (PrefsKey::skipPrimePanel, true);
		App::instance->unlockPrefs ();
		startupPrimePanel = false;
	}
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
	if (updateAppNews) {
		delete (updateAppNews);
	}
	updateAppNews = new AppNews ();
	updateAppNews->recordBuildId.assign (BUILD_ID);
	isCheckingForUpdates = true;
	isUpdateNewsReceived = false;
	updateText->setText (UiText::instance->getText (UiTextId::CheckingForUpdates).capitalized ());
	updateText->setTextColor (UiConfiguration::instance->statusOkTextColor);
	progressRingPanel->isVisible = true;
	resetControlsVisible ();
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
	StringList sql;

	if (responseData && (responseData->length > 0) && updateAppNews) {
		cmd.assignBuffer (responseData);
		if (! updateAppNews->parseCommand (cmd)) {
			Log::debug ("Check for update failed: invalid server response");
		}
		else {
			if (AppNews::getInsertCommandSql (cmd, MediaControl::appNewsTableName, &sql)) {
				MediaControl::instance->execDatabase (sql);
			}
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
	if ((! isUpdateNewsReceived) || (! updateAppNews)) {
		updateText->setText (UiText::instance->getText (UiTextId::UpdateErrorPrompt));
	}
	else {
		readAppNews (updateAppNews);
	}

	if (updateAppNews) {
		delete (updateAppNews);
		updateAppNews = NULL;
	}
	isCheckingForUpdates = false;
	isShowingUpdateRow = true;
	updateText->setTextColor (UiConfiguration::instance->primaryTextColor);
	progressRingPanel->isVisible = false;
	resetControlsVisible ();
	resetUnexpandedInfoText ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void AppCardWindow::updateLinkOpened (void *itPtr, Widget *widgetPtr) {
	HyperlinkWindow *hyperlink = (HyperlinkWindow *) widgetPtr;

	if (hyperlink->linkOpenResult != OpResult::Success) {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenUpdateUrlError));
	}
	else {
		App::instance->showNotification (UiText::instance->getText (UiTextId::OpenUpdateUrlCompleteText));
	}
}
