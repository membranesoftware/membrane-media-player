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
#include "Ui.h"
#include "PlayerUi.h"
#include "MediaControl.h"
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "TextFieldWindow.h"
#include "TextFlow.h"
#include "IconLabelWindow.h"
#include "ProgressRing.h"
#include "MediaOptionWindow.h"

MediaOptionWindow::MediaOptionWindow (SpriteGroup *playerUiSpriteGroup, const PlayerUi::Options &playerUiOptions)
: Panel ()
, isExpanded (false)
, playerUiOptions (playerUiOptions)
, isConfiguring (false)
, isStatusVisible (false)
, isDataPathVisible (false)
{
	StdString prompt;
	ProgressRing *ring;

	classId = ClassId::MediaOptionWindow;
	mediaControlStatus.updateCount = -1;
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	mediaControlMainOptions = MediaControl::instance->mainOptions;
	startPlayerUiOptions = playerUiOptions;

	headerIcon = add (new Image (playerUiSpriteGroup->getSprite (SpriteId::PlayerUi_largeMediaOptionIcon)));
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	nameLabel = add (new Label (UiText::instance->getText (UiTextId::MediaOptions).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->widgetName.assign ("mediaOptionExpandToggle");
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isInputSuspended = true;
	dividerPanel->isVisible = false;

	savePlayHistoryToggle = add (new ToggleWindow (new Toggle ()));
	savePlayHistoryToggle->widgetName.assign ("mediaOptionSavePlayHistoryToggle");
	savePlayHistoryToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::savePlayHistoryToggleStateChanged, this);
	savePlayHistoryToggle->setPaddingScale (1.0f, 0.0f);
	savePlayHistoryToggle->setText (UiText::instance->getText (UiTextId::MediaOptionWindowSavePlayHistoryPrompt));
	savePlayHistoryToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	savePlayHistoryToggle->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	savePlayHistoryToggle->setChecked (mediaControlMainOptions.savePlayHistory, true);
	savePlayHistoryToggle->isVisible = false;

	showPlayHistoryToggle = add (new ToggleWindow (new Toggle ()));
	showPlayHistoryToggle->widgetName.assign ("mediaOptionShowPlayHistoryToggle");
	showPlayHistoryToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::showPlayHistoryToggleStateChanged, this);
	showPlayHistoryToggle->setPaddingScale (1.0f, 0.0f);
	showPlayHistoryToggle->setText (UiText::instance->getText (UiTextId::MediaOptionWindowShowPlayHistoryPrompt));
	showPlayHistoryToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	showPlayHistoryToggle->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	showPlayHistoryToggle->setChecked (playerUiOptions.showPlayHistory, true);
	showPlayHistoryToggle->isVisible = false;

	savePlaylistsToggle = add (new ToggleWindow (new Toggle ()));
	savePlaylistsToggle->widgetName.assign ("mediaOptionSavePlaylistsToggle");
	savePlaylistsToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::savePlaylistsToggleStateChanged, this);
	savePlaylistsToggle->setPaddingScale (1.0f, 0.0f);
	savePlaylistsToggle->setText (UiText::instance->getText (UiTextId::MediaOptionWindowSavePlaylistsPrompt));
	savePlaylistsToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	savePlaylistsToggle->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	savePlaylistsToggle->setChecked (mediaControlMainOptions.savePlaylists, true);
	savePlaylistsToggle->isVisible = false;

	showPlaylistsToggle = add (new ToggleWindow (new Toggle ()));
	showPlaylistsToggle->widgetName.assign ("mediaOptionShowPlaylistsToggle");
	showPlaylistsToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::showPlaylistsToggleStateChanged, this);
	showPlaylistsToggle->setPaddingScale (1.0f, 0.0f);
	showPlaylistsToggle->setText (UiText::instance->getText (UiTextId::MediaOptionWindowShowPlaylistsPrompt));
	showPlaylistsToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	showPlaylistsToggle->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	showPlaylistsToggle->setChecked (playerUiOptions.showPlaylists, true);
	showPlaylistsToggle->isVisible = false;

	mediaScanToggle = add (new ToggleWindow (new Toggle ()));
	mediaScanToggle->widgetName.assign ("mediaOptionMediaScanToggle");
	mediaScanToggle->stateChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::mediaScanToggleStateChanged, this);
	mediaScanToggle->setPaddingScale (1.0f, 0.0f);
	mediaScanToggle->setText (UiText::instance->getText (UiTextId::MediaOptionWindowMediaScanPrompt));
	mediaScanToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	mediaScanToggle->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	mediaScanToggle->setChecked (mediaControlMainOptions.mediaScan, true);
	mediaScanToggle->isVisible = false;

	dataPathBgPanel = add (new Panel (), -1);
	dataPathBgPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	dataPathBgPanel->isVisible = false;
	dataPathBgPanel->isInputSuspended = true;

	dataPathIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_storageIcon)));
	dataPathIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

#if PLATFORM_WINDOWS
	prompt = UiText::instance->getText (UiTextId::MediaDataPathPromptWindows);
#else
	prompt = UiText::instance->getText (UiTextId::MediaDataPathPrompt);
#endif
	dataPathTextField = add (new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, prompt));
	dataPathTextField->widgetName.assign ("mediaOptionDataPathTextField");
	dataPathTextField->valueChangeCallback = Widget::EventCallbackContext (MediaOptionWindow::dataPathTextFieldValueChanged, this);
	dataPathTextField->setFixedPadding (true, 0.0f, 0.0f);
	dataPathTextField->setButtonsEnabled (TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption | TextFieldWindow::FsBrowseButtonSelectDirectoriesOption | TextFieldWindow::ClearButtonOption);
	dataPathTextField->setPromptErrorColor (true);
	dataPathTextField->setValue (mediaControlMainOptions.dataPath, true, true);
	dataPathTextField->isVisible = false;

	dataPathPromptText = add (new TextFlow (1.0f, UiConfiguration::CaptionFont));
	dataPathPromptText->setFixedPadding (true, 0.0f, 0.0f);
	dataPathPromptText->setText (UiText::instance->getText (UiTextId::MediaOptionWindowDataPathPrompt));
	dataPathPromptText->isVisible = false;

	statusIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon)));
	statusIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	statusIcon->isVisible = false;

	statusText = add (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	statusText->setFixedPadding (true, 0.0f, 0.0f);
	statusText->isVisible = false;

	progressRingPanel = add (new Panel (), 2);
	progressRingPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	ring = progressRingPanel->add (new ProgressRing (UiConfiguration::instance->progressRingSize));
	ring->setIndeterminate (true);
	progressRingPanel->setPaddingScale (0.1f, 0.1f);
	progressRingPanel->setLayout (Panel::RightFlowLayoutOption);
	progressRingPanel->isInputSuspended = true;
	progressRingPanel->isVisible = false;

	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (MediaOptionWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::MediaOptionWindowCancelTooltip), "mediaOptionCancelButton"));
	cancelButton->setDisabled (true);
	cancelButton->isVisible = false;

	confirmButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (MediaOptionWindow::confirmButtonClicked, this), UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowExecuteTooltip), "mediaOptionConfirmButton"));
	confirmButton->setDisabled (true);
	confirmButton->isVisible = false;

	resetControlState ();
	reflow ();
}
MediaOptionWindow::~MediaOptionWindow () {
}

MediaOptionWindow *MediaOptionWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::MediaOptionWindow) ? (MediaOptionWindow *) widget : NULL);
}

void MediaOptionWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
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

	resetControlState ();
	expandToggle->setChecked (isExpanded, true);
	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void MediaOptionWindow::reflow () {
	double x1, x2, y1, y2;

	resetPadding ();
	topLeftLayoutFlow ();

	headerIcon->flowRight (&layoutFlow);
	progressRingPanel->position.assign (headerIcon->position.x + (headerIcon->width / 2.0f) - (progressRingPanel->width / 2.0f), headerIcon->position.y + (headerIcon->height / 2.0f) - (progressRingPanel->height / 2.0f));
	nameLabel->flowDown (&layoutFlow);
	layoutFlow.y = heightPadding;
	nameLabel->centerVertical (&layoutFlow);

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);

	x1 = 0.0f;
	x2 = 0.0f;
	y1 = 0.0f;
	y2 = 0.0f;
	nextRowLayoutFlow ();
	if (isExpanded) {
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		savePlayHistoryToggle->flowRight (&layoutFlow);
		showPlayHistoryToggle->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		savePlaylistsToggle->flowRight (&layoutFlow);
		showPlaylistsToggle->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		mediaScanToggle->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		x1 = layoutFlow.x;
		y1 = layoutFlow.y;
		layoutFlow.x += UiConfiguration::instance->paddingSize;
		layoutFlow.y += (UiConfiguration::instance->paddingSize / 2.0f);
		if (dataPathIcon->isVisible) {
			dataPathIcon->flowRight (&layoutFlow);
		}
		if (dataPathPromptText->isVisible) {
			dataPathPromptText->setViewWidth (dataPathTextField->width);
			dataPathPromptText->flowDown (&layoutFlow);
		}
		if (dataPathTextField->isVisible) {
			dataPathTextField->setWindowWidth (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
			dataPathTextField->flowDown (&layoutFlow);
			x2 = dataPathTextField->position.x + dataPathTextField->width;
		}
		y2 = layoutFlow.yExtent;

		nextRowLayoutFlow ();
		layoutFlow.y += (UiConfiguration::instance->paddingSize / 2.0f);
		if (statusIcon->isVisible) {
			statusIcon->flowRight (&layoutFlow);
		}
		if (statusText->isVisible) {
			statusText->setViewWidth (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth);
			statusText->flowRight (&layoutFlow);
			statusText->position.assignY (statusIcon->position.y + (statusIcon->height / 2.0f) - (statusText->height / 2.0f));
		}

		nextRowLayoutFlow ();
		cancelButton->flowRight (&layoutFlow);
		confirmButton->flowRight (&layoutFlow);
		resetSize ();
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}
	else {
		resetSize ();
	}

	if (dataPathBgPanel->isVisible) {
		dataPathBgPanel->position.assign (x1, y1);
		dataPathBgPanel->setFixedSize (true, x2 - x1 + (UiConfiguration::instance->paddingSize * 2.0f), y2 - y1 + UiConfiguration::instance->paddingSize);
	}

	bottomRightLayoutFlow ();
	if (statusText->isVisible) {
		statusText->flowLeft (&layoutFlow);
	}
	if (statusIcon->isVisible) {
		statusIcon->flowLeft (&layoutFlow);
	}

	bottomRightLayoutFlow ();
	if (confirmButton->isVisible) {
		confirmButton->flowLeft (&layoutFlow);
	}
	if (cancelButton->isVisible) {
		cancelButton->flowLeft (&layoutFlow);
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);
}

void MediaOptionWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;
	bool shouldreflow;

	shouldreflow = false;
	MediaControl::instance->getStatus (&status);
	if (isConfiguring) {
		if (! MediaControl::instance->isTaskRunning (ClassId::MediaControlConfigureMainTask)) {
			MediaControl::instance->getStatus (ClassId::MediaControlConfigureMainTask, &taskresult);
			if (taskresult.isSuccess) {
				startPlayerUiOptions = playerUiOptions;
			}
			else {
				statusMessage.assign (taskresult.text2);
				statusColor.assign (UiConfiguration::instance->errorTextColor);
			}
			isConfiguring = false;
			shouldreflow = true;
			if (taskresult.isSuccess) {
				eventCallback (configureCallback);
			}
		}
	}
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		shouldreflow = true;
		if (isConfiguring) {
			isStatusVisible = false;
		}
		else {
			if (! statusMessage.empty ()) {
				statusText->setText (statusMessage);
				statusText->setTextColor (statusColor);
				isStatusVisible = true;
			}
			else if (mediaControlStatus.isTaskRunning && (!(MediaControl::instance->matchMainConfiguration (mediaControlMainOptions) && playerUiOptionsMatched ()))) {
				statusText->setText (UiText::instance->getText (UiTextId::MediaFilescanConfigureWindowChangeBlockedPrompt));
				statusText->setTextColor (UiConfiguration::instance->primaryTextColor);
				isStatusVisible = true;
			}
			else {
				isStatusVisible = false;
			}
		}
	}

	if (shouldreflow) {
		resetControlState ();
		reflow ();
		eventCallback (layoutChangeCallback);
	}
	Panel::doUpdate (msElapsed);
}

void MediaOptionWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

void MediaOptionWindow::savePlayHistoryToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->mediaControlMainOptions.savePlayHistory = toggle->isChecked;
	it->executeOptionChange ();
}

void MediaOptionWindow::showPlayHistoryToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->playerUiOptions.showPlayHistory = toggle->isChecked;
	it->executeOptionChange ();
}

void MediaOptionWindow::savePlaylistsToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->mediaControlMainOptions.savePlaylists = toggle->isChecked;
	it->executeOptionChange ();
}

void MediaOptionWindow::showPlaylistsToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->playerUiOptions.showPlaylists = toggle->isChecked;
	it->executeOptionChange ();
}

void MediaOptionWindow::mediaScanToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->mediaControlMainOptions.mediaScan = toggle->isChecked;
	it->executeOptionChange ();
}

void MediaOptionWindow::dataPathTextFieldValueChanged (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;
	TextFieldWindow *textfield = (TextFieldWindow *) widgetPtr;

	it->mediaControlMainOptions.dataPath.assign (textfield->getValue ());
	it->executeOptionChange ();
}

void MediaOptionWindow::executeOptionChange () {
	statusMessage.assign ("");
	resetControlState ();
	reflow ();
	mediaControlStatus.updateCount = -1;
	eventCallback (layoutChangeCallback);
}

void MediaOptionWindow::resetControlState () {
	bool disablecancel, disableconfirm, prevdatapathvisible;

	prevdatapathvisible = isDataPathVisible;
	dividerPanel->isVisible = isExpanded;
	savePlayHistoryToggle->isVisible = isExpanded;
	showPlayHistoryToggle->isVisible = isExpanded;
	savePlaylistsToggle->isVisible = isExpanded;
	showPlaylistsToggle->isVisible = isExpanded;
	mediaScanToggle->isVisible = isExpanded;
	cancelButton->isVisible = isExpanded;
	confirmButton->isVisible = isExpanded;
	isDataPathVisible = savePlayHistoryToggle->isChecked || savePlaylistsToggle->isChecked || mediaScanToggle->isChecked;
	dataPathBgPanel->isVisible = isExpanded && isDataPathVisible;
	dataPathIcon->isVisible = isExpanded && isDataPathVisible;
	dataPathTextField->isVisible = isExpanded && isDataPathVisible;
	dataPathPromptText->isVisible = isExpanded && isDataPathVisible;
	statusIcon->isVisible = isExpanded && isStatusVisible;
	statusText->isVisible = isExpanded && isStatusVisible;

	if (prevdatapathvisible != isDataPathVisible) {
		mediaControlMainOptions.dataPath.assign (MediaControl::instance->mainOptions.dataPath);
		dataPathTextField->setValue (mediaControlMainOptions.dataPath, true, true);
	}

	if (isConfiguring) {
		savePlayHistoryToggle->setDisabled (true);
		showPlayHistoryToggle->setDisabled (true);
		savePlaylistsToggle->setDisabled (true);
		showPlaylistsToggle->setDisabled (true);
		mediaScanToggle->setDisabled (true);
		dataPathTextField->setDisabled (true);
		cancelButton->setDisabled (true);
		confirmButton->setDisabled (true);
		progressRingPanel->isVisible = true;
	}
	else {
		progressRingPanel->isVisible = false;
		savePlayHistoryToggle->setDisabled (false);
		showPlayHistoryToggle->setDisabled (false);
		savePlaylistsToggle->setDisabled (false);
		showPlaylistsToggle->setDisabled (! savePlaylistsToggle->isChecked);
		mediaScanToggle->setDisabled (false);
		dataPathTextField->setDisabled (false);

		disablecancel = false;
		disableconfirm = false;
		if (MediaControl::instance->matchMainConfiguration (mediaControlMainOptions) && playerUiOptionsMatched ()) {
			disableconfirm = true;
			disablecancel = true;
		}
		else {
			if (isDataPathVisible && dataPathTextField->getValue ().empty ()) {
				disableconfirm = true;
			}
			else if (mediaControlStatus.isTaskRunning) {
				disableconfirm = true;
			}
		}
		confirmButton->setDisabled (disableconfirm);
		cancelButton->setDisabled (disablecancel);
	}
}

bool MediaOptionWindow::playerUiOptionsMatched () {
	if (playerUiOptions.showPlayHistory != startPlayerUiOptions.showPlayHistory) {
		return (false);
	}
	if (playerUiOptions.showPlaylists != startPlayerUiOptions.showPlaylists) {
		return (false);
	}
	return (true);
}

void MediaOptionWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;

	it->playerUiOptions = it->startPlayerUiOptions;
	it->mediaControlMainOptions = MediaControl::instance->mainOptions;
	it->savePlayHistoryToggle->setChecked (it->mediaControlMainOptions.savePlayHistory, true);
	it->showPlayHistoryToggle->setChecked (it->playerUiOptions.showPlayHistory, true);
	it->savePlaylistsToggle->setChecked (it->mediaControlMainOptions.savePlaylists, true);
	it->showPlaylistsToggle->setChecked (it->playerUiOptions.showPlaylists, true);
	it->mediaScanToggle->setChecked (it->mediaControlMainOptions.mediaScan, true);
	it->dataPathTextField->setValue (it->mediaControlMainOptions.dataPath, true, true);
	it->dataPathTextField->setKeyFocus (false);
	it->statusMessage.assign ("");
	it->isStatusVisible = false;
	it->resetControlState ();
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaOptionWindow::confirmButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaOptionWindow *it = (MediaOptionWindow *) itPtr;

	if (it->isConfiguring) {
		return;
	}
	it->isConfiguring = true;
	MediaControl::instance->configureMain (it->mediaControlMainOptions);
	it->resetControlState ();
}
