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
#include "OsUtil.h"
#include "PrefsKey.h"
#include "Ui.h"
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "MediaControl.h"
#include "Label.h"
#include "Image.h"
#include "TextFlow.h"
#include "Button.h"
#include "ListView.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "FsBrowserWindow.h"
#include "MediaFilescanConfigureWindow.h"

constexpr const double optionWidthScale = 0.68f;

// Stage values
constexpr const int ConfigurationMatch = 0;
constexpr const int ConfigurationChange = 1;
constexpr const int ConfigurationChangeBlocked = 2;
constexpr const int ConfigurationChangeWaiting = 3;
constexpr const int ConfigurationInvalid = 4;

MediaFilescanConfigureWindow::MediaFilescanConfigureWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
, fsBrowserPanelHandle (&fsBrowserPanel)
, stage (ConfigurationMatch)
, isConfiguring (false)
{
	Slider *slider;
	int i;

	nameLabel = add (new Label (UiText::instance->getText (UiTextId::Configuration).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	promptIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon)));
	promptIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	promptIcon->isVisible = false;

	promptText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	promptText->setFixedPadding (true, 0.0f, 0.0f);
	promptText->isVisible = false;

	sourcePathLabel = add (new Label (UiText::instance->getText (UiTextId::MediaSourceDirectories).capitalized (), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	sourcePathAddButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_addButton), Widget::EventCallbackContext (MediaFilescanConfigureWindow::sourcePathAddButtonClicked, this), UiText::instance->getText (UiTextId::AddMediaDirectory).capitalized (), "mediaScanAddSourcePathButton"));
	sourcePathListView = add (new ListView (windowWidth * optionWidthScale, 1, 3, UiConfiguration::CaptionFont));
	sourcePathListView->listChangeCallback = Widget::EventCallbackContext (MediaFilescanConfigureWindow::sourcePathListViewChanged, this);
	sourcePathListView->setEmptyStateText (UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowEmptySourcePathPrompt), UiConfiguration::CaptionFont, UiConfiguration::instance->errorTextColor);
	sourcePathListView->setItems (MediaControl::instance->filescanOptions.scanPath, true);

	mediaThumbnailCountLabel = add (new Label (UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowMediaThumbnailCountText), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));

	slider = new Slider (0.0f, (double) (MediaControl::mediaThumbnailValuesCount - 1));
	for (i = 0; i < MediaControl::mediaThumbnailValuesCount; ++i) {
		slider->addSnapValue ((double) i);
	}
	mediaThumbnailCountSlider = add (new SliderWindow (slider));
	mediaThumbnailCountSlider->setPaddingScale (1.0f, 0.0f);
	mediaThumbnailCountSlider->setValueNameFunction (MediaFilescanConfigureWindow::mediaThumbnailCountSliderValueName);
	mediaThumbnailCountSlider->setValue ((double) MediaControl::instance->filescanOptions.mediaThumbnailCount);

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaFilescanConfigureWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaScanBackButton"));
	executeButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton), Widget::EventCallbackContext (MediaFilescanConfigureWindow::executeButtonClicked, this), UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowExecuteTooltip), "mediaScanExecuteButton"));
	if (! isConfigurationValid ()) {
		executeButton->setDisabled (true);
	}

	reflow ();
}
MediaFilescanConfigureWindow::~MediaFilescanConfigureWindow () {
}

void MediaFilescanConfigureWindow::setWindowWidth (double w) {
	if (FLOAT_EQUALS (windowWidth, w)) {
		return;
	}
	windowWidth = w;
	reflow ();
}

void MediaFilescanConfigureWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;
	MediaControl::FilescanOptions options;
	StdString text;
	bool shouldreflow, shouldclickback, showtaskresult;
	int nextstage;

	shouldreflow = false;
	shouldclickback = false;
	showtaskresult = false;
	nextstage = stage;
	MediaControl::instance->getStatus (&status);
	if (! isConfiguring) {
		if (status.isTaskRunning && (status.taskType == ClassId::MediaControlConfigureFilescanTask)) {
			isConfiguring = true;
		}
	}
	else {
		if (!(status.isTaskRunning && (status.taskType == ClassId::MediaControlConfigureFilescanTask))) {
			isConfiguring = false;
			sourcePathListView->setItems (MediaControl::instance->filescanOptions.scanPath);
			mediaThumbnailCountSlider->setValue ((double) MediaControl::instance->filescanOptions.mediaThumbnailCount, true);
			MediaControl::instance->getStatus (ClassId::MediaControlConfigureFilescanTask, &taskresult);
			if (taskresult.isSuccess) {
				shouldclickback = true;
			}
			else {
				showtaskresult = true;
			}
			MediaControl::instance->clearTaskResult (ClassId::MediaControlConfigureFilescanTask);
		}
	}
	if (status.isTaskRunning && (status.taskType == ClassId::MediaControlConfigureFilescanTask)) {
		nextstage = ConfigurationChangeWaiting;
	}
	else if (! isConfigurationValid ()) {
		nextstage = ConfigurationInvalid;
	}
	else {
		sourcePathListView->getItems (&(options.scanPath));
		options.mediaThumbnailCount = (int) mediaThumbnailCountSlider->value;
		if (MediaControl::instance->matchFilescanConfiguration (options)) {
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
				if (MediaControl::instance->filescanOptions.scanPath.empty ()) {
					promptIcon->isVisible = false;
					promptText->isVisible = false;
				}
				else {
					promptText->setText (UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowChangeRescanPrompt));
					promptIcon->isVisible = true;
					promptText->isVisible = true;
				}
				sourcePathListView->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (false);
				break;
			}
			case ConfigurationChangeBlocked: {
				promptText->setText (UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowChangeBlockedPrompt));
				promptIcon->isVisible = true;
				promptText->isVisible = true;
				sourcePathListView->setDisabled (false);
				sourcePathAddButton->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (true);
				break;
			}
			case ConfigurationChangeWaiting: {
				promptIcon->isVisible = false;
				promptText->isVisible = false;
				sourcePathListView->setDisabled (true);
				sourcePathAddButton->setDisabled (true);
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
				sourcePathListView->setDisabled (false);
				sourcePathAddButton->setDisabled (false);
				mediaThumbnailCountSlider->setDisabled (false);
				executeButton->setDisabled (false);
				break;
			}
			case ConfigurationInvalid: {
				promptIcon->isVisible = false;
				promptText->isVisible = false;
				sourcePathListView->setDisabled (false);
				sourcePathAddButton->setDisabled (false);
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
	if (shouldclickback) {
		eventCallback (backCallback);
	}
	Panel::doUpdate (msElapsed);
}

void MediaFilescanConfigureWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	nameLabel->flowDown (&layoutFlow);

	sourcePathListView->setViewWidth (windowWidth * optionWidthScale);
	sourcePathLabel->flowDown (&layoutFlow);
	sourcePathListView->flowRight (&layoutFlow);
	sourcePathAddButton->flowRight (&layoutFlow);

	nextRowLayoutFlow ();
	mediaThumbnailCountLabel->flowDown (&layoutFlow);
	mediaThumbnailCountSlider->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	if (promptIcon->isVisible) {
		promptIcon->flowRight (&layoutFlow);
	}
	if (promptText->isVisible) {
		promptText->setViewWidth (windowWidth - promptIcon->width - (UiConfiguration::instance->marginSize * 2.0f) - (widthPadding * 2.0f));
		promptText->flowRight (&layoutFlow);
	}

	nextRowLayoutFlow ();
	backButton->flowRight (&layoutFlow);
	executeButton->flowRight (&layoutFlow);

	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);

	bottomRightLayoutFlow ();
	executeButton->flowLeft (&layoutFlow);
	backButton->flowLeft (&layoutFlow);
}

StdString MediaFilescanConfigureWindow::mediaThumbnailCountSliderValueName (double sliderValue) {
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

bool MediaFilescanConfigureWindow::isConfigurationValid () {
	if (sourcePathListView->getItemCount () <= 0) {
		return (false);
	}
	return (true);
}

void MediaFilescanConfigureWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaFilescanConfigureWindow *) itPtr)->eventCallback (((MediaFilescanConfigureWindow *) itPtr)->backCallback);
}

void MediaFilescanConfigureWindow::executeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanConfigureWindow *it = (MediaFilescanConfigureWindow *) itPtr;
	MediaControl::FilescanOptions options;

	if (it->stage == ConfigurationChange) {
		it->sourcePathListView->getItems (&(options.scanPath));
		options.mediaThumbnailCount = (int) it->mediaThumbnailCountSlider->value;
		MediaControl::instance->configureFilescan (options);
	}
	else if (it->stage == ConfigurationMatch) {
		it->eventCallback (it->backCallback);
	}
}

void MediaFilescanConfigureWindow::sourcePathListViewChanged (void *itPtr, Widget *widgetPtr) {
	MediaFilescanConfigureWindow *it = (MediaFilescanConfigureWindow *) itPtr;

	if (it->isConfigurationValid ()) {
		it->executeButton->setDisabled (false);
	}
	else {
		it->executeButton->setDisabled (true);
	}
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}
void MediaFilescanConfigureWindow::sourcePathAddButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanConfigureWindow *it = (MediaFilescanConfigureWindow *) itPtr;
	FsBrowserWindow *fs;
	HashMap *prefs;
	StdString path;

	prefs = App::instance->lockPrefs ();
	path = prefs->find (PrefsKey::fsBrowserPath, "");
	App::instance->unlockPrefs ();
	if (path.empty ()) {
		path = OsUtil::getUserHomePath ();
	}

	it->fsBrowserPanelHandle.destroyAndAssign (new Panel ());
	it->fsBrowserPanel->setFixedSize (true, App::instance->drawableWidth, App::instance->drawableHeight);
	it->fsBrowserPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	it->fsBrowserPanel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);

	fs = (FsBrowserWindow *) it->fsBrowserPanel->add (new FsBrowserWindow (App::instance->drawableWidth * UiConfiguration::instance->fsBrowserWindowScale, App::instance->drawableHeight * UiConfiguration::instance->fsBrowserWindowScale, path));
	fs->sortOrder = FsBrowserWindow::DirectoriesFirstSort;
	fs->selectType = FsBrowserWindow::SelectDirectories;
	fs->closeCallback = Widget::EventCallbackContext (MediaFilescanConfigureWindow::fsBrowserWindowClosed, it);
	fs->position.assign ((App::instance->drawableWidth - fs->width) / 2.0f, (App::instance->drawableHeight - fs->height) / 2.0f);

	App::instance->rootPanel->addWidget (it->fsBrowserPanel, App::instance->rootPanel->maxWidgetZLevel + 1);
}
void MediaFilescanConfigureWindow::fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr) {
	MediaFilescanConfigureWindow *it = (MediaFilescanConfigureWindow *) itPtr;
	FsBrowserWindow *fs = (FsBrowserWindow *) widgetPtr;
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	prefs->insert (PrefsKey::fsBrowserPath, fs->browsePath, "");
	App::instance->unlockPrefs ();

	if (fs->isPathSelectionConfirmed) {
		it->sourcePathListView->addItem (fs->selectedPath);
		it->reflow ();
		it->eventCallback (it->layoutChangeCallback);
	}
	it->fsBrowserPanelHandle.destroyAndClear ();
}
