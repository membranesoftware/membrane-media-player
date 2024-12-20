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
#include "Log.h"
#include "Ui.h"
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "OsUtil.h"
#include "HashMap.h"
#include "TaskGroup.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "ProgressBar.h"
#include "ProgressRing.h"
#include "TextFlow.h"
#include "TextFieldWindow.h"
#include "ListView.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "IconLabelWindow.h"
#include "ActionWindow.h"
#include "StatsWindow.h"
#include "FsBrowserWindow.h"
#include "MediaItem.h"
#include "MediaControl.h"
#include "MediaControlWindow.h"

constexpr const double windowWidthScale = 0.32f;
constexpr const double progressBarScale = 0.38f;
constexpr const double fsBrowserWindowScale = 0.83f;
constexpr const double configureOptionWidthScale = 0.3f;

// Stage values
constexpr const int Uninitialized = 0;
constexpr const int Prime = 1;
constexpr const int MainControl = 2;
constexpr const int ScanControl = 3;
constexpr const int CleanControl = 4;
constexpr const int ConfigureControl = 5;
constexpr const int ConfigurationMatch = 6;
constexpr const int ConfigurationChange = 7;
constexpr const int ConfigurationChangeBlocked = 8;
constexpr const int ConfigurationChangeWaiting = 9;
constexpr const int ConfigurationInvalid = 10;

MediaControlWindow::MediaControlWindow ()
: Panel ()
, isExpanded (false)
, stage (Uninitialized)
, primeWindowHandle (&primeWindow)
, mainWindowHandle (&mainWindow)
, scanWindowHandle (&scanWindow)
, cleanWindowHandle (&cleanWindow)
, configureWindowHandle (&configureWindow)
{
	ProgressRing *ring;

	classId = ClassId::MediaControlWindow;
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	mediaControlStatus.updateCount = -1;

	headerIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeComputerIcon)));
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	nameLabel = add (new Label (UiText::instance->getText (UiTextId::MediaScan).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaControlWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	progressRingPanel = add (new Panel (), 2);
	progressRingPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	ring = progressRingPanel->add (new ProgressRing (UiConfiguration::instance->progressRingSize));
	ring->setIndeterminate (true);
	progressRingPanel->setPaddingScale (0.1f, 0.1f);
	progressRingPanel->setLayout (Panel::RightFlowLayoutOption);
	progressRingPanel->isVisible = false;

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;

	mediaCountIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallMediaIcon), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	mediaCountIcon->setFixedPadding (true, 0.0f, 0.0f);
	mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextId::MediaFile, UiTextId::MediaFiles));

	reflow ();
}
MediaControlWindow::~MediaControlWindow () {
}

void MediaControlWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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
	expandToggle->setChecked (isExpanded, true);

	resetWidgetsVisible ();
	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void MediaControlWindow::resetWidgetsVisible () {
	dividerPanel->isVisible = isExpanded;
	mediaCountIcon->isVisible = (! isExpanded);
	if (primeWindow) {
		primeWindow->isVisible = isExpanded;
	}
	if (mainWindow) {
		mainWindow->isVisible = isExpanded;
	}
	if (scanWindow) {
		scanWindow->isVisible = isExpanded;
	}
	if (cleanWindow) {
		cleanWindow->isVisible = isExpanded;
	}
	if (configureWindow) {
		configureWindow->isVisible = isExpanded;
	}
}

void MediaControlWindow::reflow () {
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
		mediaCountIcon->flowRight (&layoutFlow);
	}

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);

	if (isExpanded) {
		nextRowLayoutFlow ();
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);
		layoutFlow.x = widthPadding;

		nextRowLayoutFlow ();
		if (primeWindow) {
			primeWindow->reflow ();
			primeWindow->flowDown (&layoutFlow);
		}
		if (mainWindow) {
			mainWindow->reflow ();
			mainWindow->flowDown (&layoutFlow);
		}
		if (scanWindow) {
			scanWindow->reflow ();
			scanWindow->flowDown (&layoutFlow);
		}
		if (cleanWindow) {
			cleanWindow->reflow ();
			cleanWindow->flowDown (&layoutFlow);
		}
		if (configureWindow) {
			configureWindow->reflow ();
			configureWindow->flowDown (&layoutFlow);
		}

		if (layoutFlow.xExtent > windowWidth) {
			setFixedSize (false);
			resetSize ();
		}
		else {
			setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
		}
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);
}

void MediaControlWindow::doResize () {
	resetPadding ();
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	if (primeWindow) {
		primeWindow->windowWidth = windowWidth - (widthPadding * 2.0f);
	}
	if (mainWindow) {
		mainWindow->windowWidth = windowWidth - (widthPadding * 2.0f);
	}
	if (scanWindow) {
		scanWindow->windowWidth = windowWidth - (widthPadding * 2.0f);
	}
	if (cleanWindow) {
		cleanWindow->windowWidth = windowWidth - (widthPadding * 2.0f);
	}
	if (configureWindow) {
		configureWindow->windowWidth = windowWidth - (widthPadding * 2.0f);
	}
	Panel::doResize ();
	reflow ();
}

void MediaControlWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaControlWindow *it = (MediaControlWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

void MediaControlWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;

	if (stage == Uninitialized) {
		if (! MediaControl::instance->isConfigured) {
			setPrime ();
		}
		else {
			setMainControl ();
		}
	}

	MediaControl::instance->getStatus (&status);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		if (! progressRingPanel->isVisible) {
			if (mediaControlStatus.isTaskRunning) {
				progressRingPanel->isVisible = true;
				reflow ();
			}
		}
		else {
			if (! mediaControlStatus.isTaskRunning) {
				progressRingPanel->isVisible = false;
				reflow ();
			}
		}
		if (status.mediaCount >= 0) {
			mediaCountIcon->setText (StdString::createSprintf ("%i", status.mediaCount));
			mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (status.mediaCount, UiTextId::MediaFile, UiTextId::MediaFiles));
		}
	}

	Panel::doUpdate (msElapsed);
}

void MediaControlWindow::setNextStage (int stageValue) {
	stage = stageValue;
	primeWindowHandle.destroyAndClear ();
	mainWindowHandle.destroyAndClear ();
	scanWindowHandle.destroyAndClear ();
	cleanWindowHandle.destroyAndClear ();
	configureWindowHandle.destroyAndClear ();
}

void MediaControlWindow::setPrime () {
	setNextStage (Prime);

	primeWindowHandle.assign (new MediaControlPrimeWindow (windowWidth - (widthPadding * 2.0f)));
	primeWindow->layoutChangeCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowLayoutChanged, this);
	primeWindow->configureCompleteCallback = Widget::EventCallbackContext (MediaControlWindow::primeWindowConfigureComplete, this);
	addWidget (primeWindow);

	resetWidgetsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void MediaControlWindow::primeWindowConfigureComplete (void *itPtr, Widget *widgetPtr) {
	((MediaControlWindow *) itPtr)->setMainControl ();
}

void MediaControlWindow::setMainControl () {
	setNextStage (MainControl);

	mainWindowHandle.assign (new MediaControlMainWindow (windowWidth - (widthPadding * 2.0f)));
	mainWindow->layoutChangeCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowLayoutChanged, this);
	mainWindow->scanClickCallback = Widget::EventCallbackContext (MediaControlWindow::mainWindowScanButtonClicked, this);
	mainWindow->cleanClickCallback = Widget::EventCallbackContext (MediaControlWindow::mainWindowCleanButtonClicked, this);
	mainWindow->configureClickCallback = Widget::EventCallbackContext (MediaControlWindow::mainWindowConfigureButtonClicked, this);
	addWidget (mainWindow);

	resetWidgetsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void MediaControlWindow::mainWindowScanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlWindow *) itPtr)->setScanControl ();
}

void MediaControlWindow::mainWindowCleanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlWindow *) itPtr)->setCleanControl ();
}

void MediaControlWindow::mainWindowConfigureButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlWindow *) itPtr)->setConfigureControl ();
}

void MediaControlWindow::setScanControl () {
	setNextStage (ScanControl);

	scanWindowHandle.assign (new MediaControlScanWindow (windowWidth - (widthPadding * 2.0f)));
	scanWindow->layoutChangeCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowLayoutChanged, this);
	scanWindow->backClickCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowBackClicked, this);
	addWidget (scanWindow);

	resetWidgetsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void MediaControlWindow::setCleanControl () {
	setNextStage (CleanControl);

	cleanWindowHandle.assign (new MediaControlCleanWindow (windowWidth - (widthPadding * 2.0f)));
	cleanWindow->layoutChangeCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowLayoutChanged, this);
	cleanWindow->backClickCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowBackClicked, this);
	addWidget (cleanWindow);

	resetWidgetsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void MediaControlWindow::setConfigureControl () {
	setNextStage (ConfigureControl);

	configureWindowHandle.assign (new MediaControlConfigureWindow (windowWidth - (widthPadding * 2.0f)));
	configureWindow->layoutChangeCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowLayoutChanged, this);
	configureWindow->backClickCallback = Widget::EventCallbackContext (MediaControlWindow::controlWindowBackClicked, this);
	addWidget (configureWindow);

	resetWidgetsVisible ();
	reflow ();
	eventCallback (layoutChangeCallback);
}

void MediaControlWindow::controlWindowLayoutChanged (void *itPtr, Widget *widgetPtr) {
	MediaControlWindow *it = (MediaControlWindow *) itPtr;

	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaControlWindow::controlWindowBackClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlWindow *) itPtr)->setMainControl ();
}

MediaControlPrimeWindow::MediaControlPrimeWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
, isConfiguring (false)
{
	StdString prompt;
	TextFieldWindow *textfield;

	statusIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon), UiText::instance->getText (UiTextId::ConfigurationRequired).capitalized ()));
	statusIcon->setFixedPadding (true, 0.0f, 0.0f);
	statusIcon->setTextColor (UiConfiguration::instance->errorTextColor);

	progressBar = add (new ProgressBar ());
	progressBar->setSize (windowWidth * progressBarScale, UiConfiguration::instance->progressBarHeight);
	progressBar->setIndeterminate (true);
	progressBar->isVisible = false;

	promptText = add (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	promptText->setText (UiText::instance->getText (UiTextId::MediaControlPrimeWindowPrompt));

	actionWindow = (ActionWindow *) addWidget (new ActionWindow ());
	actionWindow->optionChangeCallback = Widget::EventCallbackContext (MediaControlPrimeWindow::actionWindowOptionChanged, this);
	actionWindow->setFixedPadding (true, 0.0f, 0.0f);
	actionWindow->setFillBg (false);
	actionWindow->setButtonsVisible (false);

#if PLATFORM_WINDOWS
	prompt = UiText::instance->getText (UiTextId::SourceMediaPathPromptWindows);
#else
	prompt = UiText::instance->getText (UiTextId::SourceMediaPathPrompt);
#endif
	textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
	textfield->setPaddingScale (1.0f, 0.0f);
	textfield->setButtonsEnabled (TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesOption | TextFieldWindow::ClearButtonOption);
	textfield->setPromptErrorColor (true);
	actionWindow->addOption (UiText::instance->getText (UiTextId::SourceMediaPath).capitalized (), textfield, UiText::instance->getText (UiTextId::SourceMediaPathDescription));
	actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextId::SourceMediaPath).capitalized ());

#if PLATFORM_WINDOWS
	prompt = UiText::instance->getText (UiTextId::MediaDataPathPromptWindows);
#else
	prompt = UiText::instance->getText (UiTextId::MediaDataPathPrompt);
#endif
	textfield = new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt);
	textfield->setPaddingScale (1.0f, 0.0f);
	textfield->setButtonsEnabled (TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesOption | TextFieldWindow::ClearButtonOption);
	textfield->setPromptErrorColor (true);
	actionWindow->addOption (UiText::instance->getText (UiTextId::MediaDataPath).capitalized (), textfield, UiText::instance->getText (UiTextId::MediaDataPathDescription));
	actionWindow->setOptionNotEmptyString (UiText::instance->getText (UiTextId::MediaDataPath).capitalized ());

	actionWindow->setWidgetNames (StdString ("mediaControlActionWindow"));
	actionWindow->isVisible = false;

	nextButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsBoxButton), Widget::EventCallbackContext (MediaControlPrimeWindow::nextButtonClicked, this), UiText::instance->getText (UiTextId::Configure).capitalized (), "mediaControlNextButton"));
	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaControlPrimeWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaControlBackButton"));
	backButton->isVisible = false;
	confirmButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (MediaControlPrimeWindow::confirmButtonClicked, this), UiText::instance->getText (UiTextId::Configure).capitalized (), "mediaControlConfirmButton"));
	confirmButton->setDisabled (true);
	confirmButton->isVisible = false;

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
MediaControlPrimeWindow::~MediaControlPrimeWindow () {
}

void MediaControlPrimeWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;

	if (isConfiguring) {
		if (! MediaControl::instance->isRunningTask (MediaControl::PrimeTask)) {
			MediaControl::instance->getStatus (&status, MediaControl::PrimeTask, &taskresult);
			isConfiguring = false;
			if (! MediaControl::instance->isConfigured) {
				statusIcon->setText (taskresult.text2);
				progressBar->isVisible = false;
				actionWindow->setOptionDisabled (UiText::instance->getText (UiTextId::SourceMediaPath).capitalized (), false);
				actionWindow->setOptionDisabled (UiText::instance->getText (UiTextId::MediaDataPath).capitalized (), false);
				backButton->setDisabled (false);
				confirmButton->setDisabled (false);
				reflow ();
				eventCallback (layoutChangeCallback);
			}
			else {
				eventCallback (configureCompleteCallback);
			}
		}
	}
	Panel::doUpdate (msElapsed);
}

void MediaControlPrimeWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	if (statusIcon->isVisible) {
		statusIcon->flowRight (&layoutFlow);
	}
	if (progressBar->isVisible) {
		progressBar->flowRight (&layoutFlow);
		progressBar->centerVertical (&layoutFlow);
	}

	nextRowLayoutFlow ();
	if (promptText->isVisible) {
		promptText->flowDown (&layoutFlow);
	}
	if (actionWindow->isVisible) {
		actionWindow->reflow ();
		actionWindow->flowDown (&layoutFlow);
	}
	if (nextButton->isVisible) {
		nextButton->flowRight (&layoutFlow);
	}
	if (backButton->isVisible) {
		backButton->flowRight (&layoutFlow);
	}
	if (confirmButton->isVisible) {
		confirmButton->flowRight (&layoutFlow);
	}
	resetSize ();

	bottomRightLayoutFlow ();
	if (confirmButton->isVisible) {
		confirmButton->flowLeft (&layoutFlow);
	}
	if (backButton->isVisible) {
		backButton->flowLeft (&layoutFlow);
	}
	if (nextButton->isVisible) {
		nextButton->flowLeft (&layoutFlow);
	}
}

void MediaControlPrimeWindow::doResize () {
	Panel::doResize ();
	resetPadding ();
	progressBar->setSize (windowWidth * progressBarScale, UiConfiguration::instance->progressBarHeight);
	promptText->setViewWidth (windowWidth - (widthPadding * 2.0f));
	reflow ();
}

void MediaControlPrimeWindow::nextButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControlPrimeWindow *it = (MediaControlPrimeWindow *) itPtr;

	it->nextButton->isVisible = false;
	it->promptText->isVisible = false;
	it->actionWindow->isVisible = true;
	it->backButton->isVisible = true;
	it->confirmButton->isVisible = true;
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaControlPrimeWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControlPrimeWindow *it = (MediaControlPrimeWindow *) itPtr;

	it->actionWindow->isVisible = false;
	it->backButton->isVisible = false;
	it->confirmButton->isVisible = false;
	it->nextButton->isVisible = true;
	it->promptText->isVisible = true;
	it->statusIcon->setText (UiText::instance->getText (UiTextId::ConfigurationRequired).capitalized ());
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaControlPrimeWindow::actionWindowOptionChanged (void *itPtr, Widget *widgetPtr) {
	MediaControlPrimeWindow *it = (MediaControlPrimeWindow *) itPtr;

	it->confirmButton->setDisabled (! it->actionWindow->isOptionDataValid);
}

void MediaControlPrimeWindow::confirmButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControlPrimeWindow *it = (MediaControlPrimeWindow *) itPtr;

	it->isConfiguring = true;
	it->actionWindow->setOptionDisabled (UiText::instance->getText (UiTextId::SourceMediaPath).capitalized (), true);
	it->actionWindow->setOptionDisabled (UiText::instance->getText (UiTextId::MediaDataPath).capitalized (), true);
	it->confirmButton->setDisabled (true);
	it->backButton->setDisabled (true);
	it->statusIcon->setText (UiText::instance->getText (UiTextId::Configuring).capitalized ());
	it->progressBar->isVisible = true;
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);

	MediaControl::instance->prime (it->actionWindow->getStringValue (UiText::instance->getText (UiTextId::SourceMediaPath).capitalized (), ""), it->actionWindow->getStringValue (UiText::instance->getText (UiTextId::MediaDataPath).capitalized (), ""));
}

MediaControlMainWindow::MediaControlMainWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
, mediaSizeTotal (-1)
, mediaDurationTotal (-1)
, isLoadingMetadata (false)
, isLoadMetadataComplete (false)
{
	mediaControlStatus.updateCount = -1;
	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPaddingScale (1.0f, 0.0f);
	statsWindow->setItem (UiText::instance->getText (UiTextId::Status).capitalized (), StdString (), StdString ("mediaControlStatsWindowStatus"));
	statsWindow->setItem (UiText::instance->getText (UiTextId::MediaFiles).capitalized (), StdString (), StdString ("mediaControlStatsWindowMediaFiles"));
	statsWindow->setItem (UiText::instance->getText (UiTextId::TotalFileSize).capitalized (), StdString ());
	statsWindow->setItem (UiText::instance->getText (UiTextId::TotalDuration).capitalized (), StdString ());

	scanButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_mediaScanButton), Widget::EventCallbackContext (MediaControlMainWindow::scanButtonClicked, this), UiText::instance->getText (UiTextId::ScanMediaTooltip), "mediaControlScanButton"));
	cleanButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cleanButton), Widget::EventCallbackContext (MediaControlMainWindow::cleanButtonClicked, this), UiText::instance->getText (UiTextId::CleanMediaTooltip), "mediaControlCleanButton"));
	configureButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton), Widget::EventCallbackContext (MediaControlMainWindow::configureButtonClicked, this), UiText::instance->getText (UiTextId::Configure).capitalized (), "mediaControlConfigureButton"));

	reflow ();
}
MediaControlMainWindow::~MediaControlMainWindow () {
}

void MediaControlMainWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	statsWindow->flowDown (&layoutFlow);
	configureButton->flowRight (&layoutFlow);
	cleanButton->flowRight (&layoutFlow);
	scanButton->flowRight (&layoutFlow);
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);

	bottomRightLayoutFlow ();
	scanButton->flowLeft (&layoutFlow);
	cleanButton->flowLeft (&layoutFlow);
}

void MediaControlMainWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;

	MediaControl::instance->getStatus (&status);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		if ((mediaControlStatus.taskType != MediaControl::NoTask) && mediaControlStatus.isTaskRunning) {
			statsWindow->setItemTextColor (UiText::instance->getText (UiTextId::Status).capitalized (), UiConfiguration::instance->statusOkTextColor);
		}
		else {
			statsWindow->setItemTextColor (UiText::instance->getText (UiTextId::Status).capitalized (), UiConfiguration::instance->lightPrimaryTextColor);
		}
		statsWindow->setItem (UiText::instance->getText (UiTextId::Status).capitalized (), mediaControlStatus.statusText);

		if (mediaControlStatus.mediaCount >= 0) {
			statsWindow->setItem (UiText::instance->getText (UiTextId::MediaFiles).capitalized (), StdString::createSprintf ("%i", mediaControlStatus.mediaCount));
		}
		else {
			statsWindow->setItem (UiText::instance->getText (UiTextId::MediaFiles).capitalized (), StdString ());
		}
	}

	if (! isLoadingMetadata) {
		if ((mediaSizeTotal < 0) && (! status.isTaskRunning)) {
			isLoadingMetadata = true;
			retain ();
			TaskGroup::instance->run (TaskGroup::RunContext (MediaControlMainWindow::loadMetadata, this));
		}
	}
	else {
		if (isLoadMetadataComplete) {
			isLoadingMetadata = false;
			if (mediaSizeTotal > 0) {
				statsWindow->setItem (UiText::instance->getText (UiTextId::TotalFileSize).capitalized (), UiText::instance->getByteCountText (mediaSizeTotal));
			}
			if (mediaDurationTotal > 0) {
				statsWindow->setItem (UiText::instance->getText (UiTextId::TotalDuration).capitalized (), UiText::instance->getTimespanText (mediaDurationTotal, UiText::SecondsUnit, true));
			}
			reflow ();
			eventCallback (layoutChangeCallback);
		}
	}
	Panel::doUpdate (msElapsed);
}

void MediaControlMainWindow::loadMetadata (void *itPtr) {
	MediaControlMainWindow *it = (MediaControlMainWindow *) itPtr;
	StdString errmsg;
	int64_t sizetotal, durationtotal;

	if (! MediaItem::readDatabaseMetadata (MediaControl::instance->databasePath, &errmsg, &sizetotal, &durationtotal)) {
		Log::debug ("Failed to read media metadata; err=%s", errmsg.c_str ());
		sizetotal = 0;
		durationtotal = 0;
	}
	it->mediaSizeTotal = sizetotal;
	it->mediaDurationTotal = durationtotal;
	it->isLoadMetadataComplete = true;
	it->release ();
}

void MediaControlMainWindow::scanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlMainWindow *) itPtr)->eventCallback (((MediaControlMainWindow *) itPtr)->scanClickCallback);
}

void MediaControlMainWindow::cleanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlMainWindow *) itPtr)->eventCallback (((MediaControlMainWindow *) itPtr)->cleanClickCallback);
}

void MediaControlMainWindow::configureButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlMainWindow *) itPtr)->eventCallback (((MediaControlMainWindow *) itPtr)->configureClickCallback);
}

MediaControlScanWindow::MediaControlScanWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
{
	mediaControlStatus.updateCount = -1;
	titleLabel = add (new Label (UiText::instance->getText (UiTextId::MediaControlScanWindowTitle), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	taskWindow = new MediaControlTaskStatusWindow (MediaControl::ScanTask, windowWidth, UiText::instance->getText (UiTextId::MediaControlScanWindowPrompt));
	addWidget (taskWindow);

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaControlScanWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaControlBackButton"));
	executeButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_mediaScanButton), Widget::EventCallbackContext (MediaControlScanWindow::executeButtonClicked, this), UiText::instance->getText (UiTextId::MediaControlScanWindowExecuteTooltip).capitalized (), "mediaControlExecuteButton"));
	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (MediaControlScanWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::Cancel).capitalized (), "mediaControlCancelButton"));
	cancelButton->isVisible = false;
	clearButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (MediaControlScanWindow::clearButtonClicked, this), UiText::instance->getText (UiTextId::Ok).capitalized (), "mediaControlClearButton"));
	clearButton->isVisible = false;

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
MediaControlScanWindow::~MediaControlScanWindow () {
}

void MediaControlScanWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;

	MediaControl::instance->getStatus (&status, MediaControl::ScanTask, &taskresult);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		taskWindow->updateStatus (mediaControlStatus, taskresult);
		if (MediaControl::instance->isRunningTask (MediaControl::ScanTask)) {
			clearButton->isVisible = false;
			cancelButton->isVisible = true;
		}
		else {
			cancelButton->isVisible = false;
			clearButton->isVisible = (! taskresult.text1.empty ());
		}

		reflow ();
		eventCallback (layoutChangeCallback);
	}
	Panel::doUpdate (msElapsed);
}

void MediaControlScanWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	titleLabel->flowDown (&layoutFlow);
	taskWindow->flowDown (&layoutFlow);
	if (clearButton->isVisible) {
		clearButton->flowRight (&layoutFlow);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowRight (&layoutFlow);
	}
	backButton->flowRight (&layoutFlow);
	executeButton->flowRight (&layoutFlow);
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);

	bottomRightLayoutFlow ();
	executeButton->flowLeft (&layoutFlow);
	backButton->flowLeft (&layoutFlow);
}

void MediaControlScanWindow::doResize () {
	taskWindow->windowWidth = windowWidth;
	Panel::doResize ();
}

void MediaControlScanWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlScanWindow *) itPtr)->eventCallback (((MediaControlScanWindow *) itPtr)->backClickCallback);
}

void MediaControlScanWindow::executeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->scan ();
	((MediaControlScanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

void MediaControlScanWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->cancelTask (MediaControl::ScanTask);
	((MediaControlScanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

void MediaControlScanWindow::clearButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->clearTaskResult (MediaControl::ScanTask);
	((MediaControlScanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

MediaControlCleanWindow::MediaControlCleanWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
{
	mediaControlStatus.updateCount = -1;
	titleLabel = add (new Label (UiText::instance->getText (UiTextId::MediaControlCleanWindowTitle), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	taskWindow = new MediaControlTaskStatusWindow (MediaControl::CleanTask, windowWidth, UiText::instance->getText (UiTextId::MediaControlCleanWindowPrompt));
	addWidget (taskWindow);

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaControlCleanWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaControlBackButton"));
	executeButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cleanButton), Widget::EventCallbackContext (MediaControlCleanWindow::executeButtonClicked, this), UiText::instance->getText (UiTextId::MediaControlCleanWindowExecuteTooltip), "mediaControlExecuteButton"));
	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (MediaControlCleanWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::Cancel).capitalized (), "mediaControlCancelButton"));
	cancelButton->isVisible = false;
	clearButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (MediaControlCleanWindow::clearButtonClicked, this), UiText::instance->getText (UiTextId::Ok).capitalized (), "mediaControlClearButton"));
	clearButton->isVisible = false;

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
MediaControlCleanWindow::~MediaControlCleanWindow () {
}

void MediaControlCleanWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;

	MediaControl::instance->getStatus (&status, MediaControl::CleanTask, &taskresult);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		taskWindow->updateStatus (mediaControlStatus, taskresult);
		if (MediaControl::instance->isRunningTask (MediaControl::CleanTask)) {
			clearButton->isVisible = false;
			cancelButton->isVisible = true;
		}
		else {
			cancelButton->isVisible = false;
			clearButton->isVisible = (! taskresult.text1.empty ());
		}

		reflow ();
		eventCallback (layoutChangeCallback);
	}
	Panel::doUpdate (msElapsed);
}

void MediaControlCleanWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	titleLabel->flowDown (&layoutFlow);
	taskWindow->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	if (clearButton->isVisible) {
		clearButton->flowRight (&layoutFlow);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowRight (&layoutFlow);
	}
	backButton->flowRight (&layoutFlow);
	executeButton->flowRight (&layoutFlow);
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);

	bottomRightLayoutFlow ();
	executeButton->flowLeft (&layoutFlow);
	backButton->flowLeft (&layoutFlow);
}

void MediaControlCleanWindow::doResize () {
	taskWindow->windowWidth = windowWidth;
	Panel::doResize ();
}

void MediaControlCleanWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlCleanWindow *) itPtr)->eventCallback (((MediaControlCleanWindow *) itPtr)->backClickCallback);
}

void MediaControlCleanWindow::executeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->clean ();
	((MediaControlCleanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

void MediaControlCleanWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->cancelTask (MediaControl::CleanTask);
	((MediaControlCleanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

void MediaControlCleanWindow::clearButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->clearTaskResult (MediaControl::CleanTask);
	((MediaControlCleanWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}

MediaControlConfigureWindow::MediaControlConfigureWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
, stage (Uninitialized)
, isConfiguring (false)
, fsBrowserPanelHandle (&fsBrowserPanel)
{
	StdString prompt;
	Slider *slider;
	int i;

	optionWidth = App::instance->drawableWidth * configureOptionWidthScale;
	titleLabel = add (new Label (UiText::instance->getText (UiTextId::Configuration).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	mediaSourcePathLabel = add (new Label (UiText::instance->getText (UiTextId::MediaSourceDirectories).capitalized (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	mediaSourcePathAddButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_addButton), Widget::EventCallbackContext (MediaControlConfigureWindow::mediaSourcePathAddButtonClicked, this), UiText::instance->getText (UiTextId::AddMediaDirectory).capitalized (), "mediaControlAddMediaSourcePathButton"));
	mediaSourcePathListView = add (new ListView (optionWidth, 1, 3, UiConfiguration::CaptionFont));
	mediaSourcePathListView->setEmptyStateText (UiText::instance->getText (UiTextId::MediaControlConfigureWindowEmptySourcePathPrompt), UiConfiguration::CaptionFont, UiConfiguration::instance->errorTextColor);
	mediaSourcePathListView->setItems (MediaControl::instance->mediaSourcePath);

	dataPathLabel = add (new Label (UiText::instance->getText (UiTextId::DataDirectory).capitalized (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
#if PLATFORM_WINDOWS
	prompt = UiText::instance->getText (UiTextId::SourceMediaPathPromptWindows);
#else
	prompt = UiText::instance->getText (UiTextId::SourceMediaPathPrompt);
#endif
	dataPathTextField = add (new TextFieldWindow (optionWidth, prompt));
	dataPathTextField->setFixedPadding (true, 0.0f, 0.0f);
	dataPathTextField->setButtonsEnabled (TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesOption | TextFieldWindow::ClearButtonOption);
	dataPathTextField->setPromptErrorColor (true);
	dataPathTextField->setValue (MediaControl::instance->dataPath);

	mediaThumbnailCountLabel = add (new Label (UiText::instance->getText (UiTextId::MediaControlConfigureWindowMediaThumbnailCountText), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));

	slider = new Slider (0.0f, (double) (MediaControl::mediaThumbnailValuesCount - 1));
	for (i = 0; i < MediaControl::mediaThumbnailValuesCount; ++i) {
		slider->addSnapValue ((double) i);
	}
	mediaThumbnailCountSlider = add (new SliderWindow (slider));
	mediaThumbnailCountSlider->setPaddingScale (1.0f, 0.0f);
	mediaThumbnailCountSlider->setValueNameFunction (MediaControlConfigureWindow::mediaThumbnailCountSliderValueName);
	mediaThumbnailCountSlider->setValue ((double) MediaControl::instance->mediaThumbnailCount);

	promptIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon)));
	promptIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	promptIcon->isVisible = false;

	promptText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	promptText->setFixedPadding (true, 0.0f, 0.0f);
	promptText->isVisible = false;

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaControlConfigureWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaControlBackButton"));
	executeButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton), Widget::EventCallbackContext (MediaControlConfigureWindow::executeButtonClicked, this), UiText::instance->getText (UiTextId::MediaControlConfigureWindowExecuteTooltip), "mediaControlExecuteButton"));
}
MediaControlConfigureWindow::~MediaControlConfigureWindow () {
}

StdString MediaControlConfigureWindow::mediaThumbnailCountSliderValueName (double sliderValue) {
	int n;

	n = (int) sliderValue;
	switch (n) {
		case MediaControl::MediaThumbnailEveryHour: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailEveryHourDescription));
		}
		case MediaControl::MediaThumbnailEvery10Minutes: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailEvery10MinutesDescription));
		}
		case MediaControl::MediaThumbnailEveryMinute: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailEveryMinuteDescription));
		}
		case MediaControl::MediaThumbnailEvery10Seconds: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailEvery10SecondsDescription));
		}
		case MediaControl::MediaThumbnailDivide1: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailDivide1Description));
		}
		case MediaControl::MediaThumbnailDivide12: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailDivide12Description));
		}
		case MediaControl::MediaThumbnailDivide48: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailDivide48Description));
		}
		case MediaControl::MediaThumbnailDivide100: {
			return (UiText::instance->getText (UiTextId::MediaThumbnailDivide100Description));
		}
	}
	return (StdString ());
}

void MediaControlConfigureWindow::reflow () {
	double x;

	resetPadding ();
	topLeftLayoutFlow ();

	titleLabel->flowDown (&layoutFlow);
	mediaSourcePathLabel->flowDown (&layoutFlow);
	mediaSourcePathListView->flowRight (&layoutFlow);
	mediaSourcePathAddButton->flowRight (&layoutFlow);

	nextRowLayoutFlow ();
	dataPathLabel->flowDown (&layoutFlow);
	dataPathTextField->flowDown (&layoutFlow);
	mediaThumbnailCountLabel->flowDown (&layoutFlow);
	mediaThumbnailCountSlider->flowDown (&layoutFlow);
	x = layoutFlow.xExtent;

	nextRowLayoutFlow ();
	if (promptIcon->isVisible) {
		promptIcon->flowRight (&layoutFlow);
	}
	if (promptText->isVisible) {
		promptText->setViewWidth (x - promptIcon->width - (UiConfiguration::instance->marginSize * 2.0f) - (widthPadding * 2.0f));
		promptText->flowRight (&layoutFlow);
	}

	nextRowLayoutFlow ();
	executeButton->flowRight (&layoutFlow);
	backButton->flowRight (&layoutFlow);

	resetSize ();

	bottomRightLayoutFlow ();
	executeButton->flowLeft (&layoutFlow);
	backButton->flowLeft (&layoutFlow);
}

void MediaControlConfigureWindow::doResize () {
	Panel::doResize ();
	optionWidth = App::instance->drawableWidth * configureOptionWidthScale;
	mediaSourcePathListView->setViewWidth (optionWidth);
	dataPathTextField->setWindowWidth (optionWidth);
	reflow ();
}

void MediaControlConfigureWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;
	StringList items;
	StdString text;
	bool shouldreflow, showtaskresult;
	int nextstage;

	shouldreflow = false;
	showtaskresult = false;
	nextstage = stage;
	MediaControl::instance->getStatus (&status);
	if (! isConfiguring) {
		if (status.isTaskRunning && (status.taskType == MediaControl::ConfigureTask)) {
			isConfiguring = true;
		}
	}
	else {
		if (!(status.isTaskRunning && (status.taskType == MediaControl::ConfigureTask))) {
			isConfiguring = false;
			mediaSourcePathListView->setItems (MediaControl::instance->mediaSourcePath);
			dataPathTextField->setValue (MediaControl::instance->dataPath, true, true);
			mediaThumbnailCountSlider->setValue ((double) MediaControl::instance->mediaThumbnailCount, true);
			MediaControl::instance->getStatus (MediaControl::ConfigureTask, &taskresult);
			showtaskresult = true;
			MediaControl::instance->clearTaskResult (MediaControl::ConfigureTask);
		}
	}
	if (status.isTaskRunning && (status.taskType == MediaControl::ConfigureTask)) {
		nextstage = ConfigurationChangeWaiting;
	}
	else if (! isConfigurationValid ()) {
		nextstage = ConfigurationInvalid;
	}
	else {
		mediaSourcePathListView->getItems (&items);
		if (MediaControl::instance->matchConfiguration (items, dataPathTextField->getValue (), (int) mediaThumbnailCountSlider->value)) {
			nextstage = ConfigurationMatch;
		}
		else {
			if (status.isTaskRunning) {
				nextstage = ConfigurationChangeBlocked;
			}
			else {
				nextstage = ConfigurationChange;
			}
		}
	}

	if (stage != nextstage) {
		stage = nextstage;
		shouldreflow = true;
		switch (stage) {
			case ConfigurationChange: {
				promptText->setText (UiText::instance->getText (UiTextId::MediaControlConfigureWindowChangePrompt));
				promptIcon->isVisible = true;
				promptText->isVisible = true;
				mediaSourcePathListView->setDisabled (false);
				dataPathTextField->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (false);
				break;
			}
			case ConfigurationChangeBlocked: {
				promptText->setText (UiText::instance->getText (UiTextId::MediaControlConfigureWindowChangeBlockedPrompt));
				promptIcon->isVisible = true;
				promptText->isVisible = true;
				mediaSourcePathListView->setDisabled (false);
				mediaSourcePathAddButton->setDisabled (false);
				dataPathTextField->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (true);
				break;
			}
			case ConfigurationChangeWaiting: {
				promptText->setText (UiText::instance->getText (UiTextId::MediaControlConfigureWindowChangeWaitingPrompt));
				promptIcon->isVisible = true;
				promptText->isVisible = true;
				mediaSourcePathListView->setDisabled (true);
				mediaSourcePathAddButton->setDisabled (true);
				dataPathTextField->setDisabled (true);
				mediaThumbnailCountSlider->setDisabled (true);
				executeButton->setDisabled (true);
				break;
			}
			case ConfigurationMatch: {
				if (showtaskresult) {
					text.assign (taskresult.text1);
					if (! taskresult.text2.empty ()) {
						text.append (" (");
						text.append (taskresult.text2);
						text.append (")");
					}
					promptText->setText (text);
					promptIcon->isVisible = true;
					promptText->isVisible = true;
				}
				else {
					promptIcon->isVisible = false;
					promptText->isVisible = false;
				}
				mediaSourcePathListView->setDisabled (false);
				mediaSourcePathAddButton->setDisabled (false);
				dataPathTextField->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (false);
				break;
			}
			case ConfigurationInvalid: {
				promptIcon->isVisible = false;
				promptText->isVisible = false;
				mediaSourcePathListView->setDisabled (false);
				mediaSourcePathAddButton->setDisabled (false);
				dataPathTextField->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (true);
				break;
			}
		}
	}
	if (shouldreflow) {
		reflow ();
		eventCallback (layoutChangeCallback);
	}
	Panel::doUpdate (msElapsed);
}

bool MediaControlConfigureWindow::isConfigurationValid () {
	if (mediaSourcePathListView->getItemCount () <= 0) {
		return (false);
	}
	if (dataPathTextField->getValue ().empty ()) {
		return (false);
	}
	return (true);
}

void MediaControlConfigureWindow::mediaSourcePathAddButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControlConfigureWindow *it = (MediaControlConfigureWindow *) itPtr;
	FsBrowserWindow *fs;
	HashMap *prefs;
	StdString path;

	prefs = App::instance->lockPrefs ();
	path = prefs->find (App::fsBrowserPathKey, "");
	App::instance->unlockPrefs ();
	if (path.empty ()) {
		path = OsUtil::getUserHomePath ();
	}

	it->fsBrowserPanelHandle.destroyAndAssign (new Panel ());
	it->fsBrowserPanel->setFixedSize (true, App::instance->drawableWidth, App::instance->drawableHeight);
	it->fsBrowserPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	it->fsBrowserPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);

	fs = (FsBrowserWindow *) it->fsBrowserPanel->add (new FsBrowserWindow (App::instance->drawableWidth * fsBrowserWindowScale, App::instance->drawableHeight * fsBrowserWindowScale, path));
	fs->sortOrder = FsBrowserWindow::DirectoriesFirstSort;
	fs->selectType = FsBrowserWindow::SelectDirectories;
	fs->closeCallback = Widget::EventCallbackContext (MediaControlConfigureWindow::fsBrowserWindowClosed, it);
	fs->position.assign ((App::instance->drawableWidth - fs->width) / 2.0f, (App::instance->drawableHeight - fs->height) / 2.0f);

	App::instance->rootPanel->addWidget (it->fsBrowserPanel, App::instance->rootPanel->maxWidgetZLevel + 1);
}

void MediaControlConfigureWindow::fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr) {
	MediaControlConfigureWindow *it = (MediaControlConfigureWindow *) itPtr;
	FsBrowserWindow *fs = (FsBrowserWindow *) widgetPtr;
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	prefs->insert (App::fsBrowserPathKey, fs->browsePath, "");
	App::instance->unlockPrefs ();

	if (fs->isPathSelectionConfirmed) {
		it->mediaSourcePathListView->addItem (fs->selectedPath);
		it->reflow ();
		it->eventCallback (it->layoutChangeCallback);
	}
	it->fsBrowserPanelHandle.destroyAndClear ();
}

void MediaControlConfigureWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaControlConfigureWindow *) itPtr)->eventCallback (((MediaControlConfigureWindow *) itPtr)->backClickCallback);
}

void MediaControlConfigureWindow::executeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControlConfigureWindow *it = (MediaControlConfigureWindow *) itPtr;
	StringList items;

	if (it->stage == ConfigurationChange) {
		it->mediaSourcePathListView->getItems (&items);
		MediaControl::instance->configure (items, it->dataPathTextField->getValue (), (int) it->mediaThumbnailCountSlider->value);
	}
	else if (it->stage == ConfigurationMatch) {
		it->eventCallback (it->backClickCallback);
	}
}

MediaControlTaskStatusWindow::MediaControlTaskStatusWindow (int targetTaskType, double windowWidth, const StdString &promptTextValue)
: Panel ()
, targetTaskType (targetTaskType)
, windowWidth (windowWidth)
{
	promptText = add (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	promptText->setPaddingScale (1.0f, 0.0f);
	promptText->setText (promptTextValue);

	taskIconSpriteId.assign (SpriteId::SpriteGroup_taskInProgressIcon);
	taskIcon = add (new Image (SpriteGroup::instance->getSprite (taskIconSpriteId.c_str ())));
	taskIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	taskIcon->isVisible = false;

	taskText1Label = add (new Label (StdString (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	taskText1Label->isVisible = false;
	taskText2Label = add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	taskText2Label->isVisible = false;

	taskProgressBar = add (new ProgressBar ());
	taskProgressBar->setSize (windowWidth * progressBarScale, UiConfiguration::instance->progressBarHeight);
	taskProgressBar->isVisible = false;
}
MediaControlTaskStatusWindow::~MediaControlTaskStatusWindow () {
}

void MediaControlTaskStatusWindow::updateStatus (const MediaControl::Status &status, const MediaControl::TaskResult &taskResult) {
	StdString spriteid;

	if (status.isTaskRunning && (status.taskType == targetTaskType)) {
		promptText->isVisible = false;
		spriteid.assign (SpriteId::SpriteGroup_taskInProgressIcon);
		taskText1Label->setText (status.taskText1);
		taskText1Label->isVisible = true;
		if (! status.taskText2.empty ()) {
			taskText2Label->setText (UiConfiguration::instance->fonts[taskText2Label->textFontType]->truncatedText (status.taskText2, windowWidth - (widthPadding * 2.0f) - taskIcon->width - UiConfiguration::instance->marginSize, Font::dotTruncateSuffix));
			taskText2Label->isVisible = true;
		}
		else {
			taskText2Label->isVisible = false;
		}

		if (status.taskProgressPercent >= 0.0f) {
			taskProgressBar->setIndeterminate (false);
			taskProgressBar->setProgress (status.taskProgressPercent, 100.0f);
		}
		else {
			taskProgressBar->setIndeterminate (true);
		}
		taskProgressBar->isVisible = true;
	}
	else if (status.isTaskRunning && MediaControl::instance->isRunningTask (targetTaskType)) {
		promptText->isVisible = false;
		taskText2Label->isVisible = false;
		spriteid.assign (SpriteId::SpriteGroup_taskInProgressIcon);
		taskText1Label->setText (UiText::instance->getText (UiTextId::MediaControlTaskStatusWindowWaitingPrompt));
		taskText1Label->isVisible = true;
		taskProgressBar->setIndeterminate (true);
		taskProgressBar->isVisible = true;
	}
	else if (! taskResult.text1.empty ()) {
		spriteid.assign (SpriteId::SpriteGroup_largeMediaIcon);
		promptText->isVisible = false;
		taskProgressBar->isVisible = false;

		taskText1Label->setText (taskResult.text1);
		taskText1Label->isVisible = true;
		if (! taskResult.text2.empty ()) {
			taskText2Label->setText (UiConfiguration::instance->fonts[taskText2Label->textFontType]->truncatedText (taskResult.text2, windowWidth - (widthPadding * 2.0f) - taskIcon->width - UiConfiguration::instance->marginSize, Font::dotTruncateSuffix));
			taskText2Label->isVisible = true;
		}
		else {
			taskText2Label->isVisible = false;
		}
	}
	else {
		taskIcon->isVisible = false;
		taskText1Label->isVisible = false;
		taskText2Label->isVisible = false;
		taskProgressBar->isVisible = false;
		promptText->isVisible = true;
	}
	if ((! spriteid.empty ()) && (! taskIconSpriteId.equals (spriteid))) {
		taskIconSpriteId.assign (spriteid);
		taskIcon->setSprite (SpriteGroup::instance->getSprite (taskIconSpriteId.c_str ()));
	}
	reflow ();
}

void MediaControlTaskStatusWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	if (promptText->isVisible) {
		promptText->setViewWidth (windowWidth - (widthPadding * 2.0f));
		promptText->flowDown (&layoutFlow);
	}
	if (taskIcon->isVisible) {
		taskIcon->flowRight (&layoutFlow);
	}
	if (taskText1Label->isVisible) {
		taskText1Label->flowDown (&layoutFlow);
	}
	if (taskText2Label->isVisible) {
		taskText2Label->flowDown (&layoutFlow);
	}
	if (taskProgressBar->isVisible) {
		taskProgressBar->flowDown (&layoutFlow);
	}
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
}

void MediaControlTaskStatusWindow::doResize () {
	Panel::doResize ();
	resetPadding ();
	promptText->setViewWidth (windowWidth - (widthPadding * 2.0f));
	taskProgressBar->setSize (windowWidth * progressBarScale, UiConfiguration::instance->progressBarHeight);
	reflow ();
}
