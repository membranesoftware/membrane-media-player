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
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "MediaControl.h"
#include "Label.h"
#include "Image.h"
#include "Toggle.h"
#include "ProgressRing.h"
#include "IconLabelWindow.h"
#include "MediaFilescanStatusWindow.h"
#include "MediaFilescanConfigureWindow.h"
#include "MediaFilescanRunWindow.h"
#include "MediaFilescanCleanWindow.h"
#include "MediaFilescanWindow.h"

constexpr const double windowWidthScale = 0.32f;
constexpr const int contentZLevel = 1;

MediaFilescanWindow::MediaFilescanWindow ()
: Panel ()
, isExpanded (false)
, configureWindowHandle (&configureWindow)
, runWindowHandle (&runWindow)
, cleanWindowHandle (&cleanWindow)
{
	ProgressRing *ring;

	classId = ClassId::MediaFilescanWindow;
	mediaControlStatus.updateCount = -1;
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;

	headerIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeComputerIcon)));
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	nameLabel = add (new Label (UiText::instance->getText (UiTextId::MediaScan).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaFilescanWindow::expandToggleStateChanged, this);
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
	dividerPanel->isVisible = false;

	statusWindow = createStatusWindow ();
	add (statusWindow);
	statusWindow->isVisible = false;

	mediaCountIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallMediaIcon), StdString ("0"), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	mediaCountIcon->setFixedPadding (true, 0.0f, 0.0f);
	mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (0, UiTextId::MediaFile, UiTextId::MediaFiles));

	reflow ();
}
MediaFilescanWindow::~MediaFilescanWindow () {
}

MediaFilescanWindow *MediaFilescanWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::MediaFilescanWindow) ? (MediaFilescanWindow *) widget : NULL);
}

void MediaFilescanWindow::clearContentWindow () {
	configureWindowHandle.destroyAndClear ();
	runWindowHandle.destroyAndClear ();
	cleanWindowHandle.destroyAndClear ();
}

MediaFilescanStatusWindow *MediaFilescanWindow::createStatusWindow () {
	MediaFilescanStatusWindow *window;

	window = new MediaFilescanStatusWindow (windowWidth);
	window->layoutChangeCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowLayoutChanged, this);
	window->scanCallback = Widget::EventCallbackContext (MediaFilescanWindow::statusWindowScanClicked, this);
	window->cleanCallback = Widget::EventCallbackContext (MediaFilescanWindow::statusWindowCleanClicked, this);
	window->configureCallback = Widget::EventCallbackContext (MediaFilescanWindow::statusWindowConfigureClicked, this);
	return (window);
}

MediaFilescanConfigureWindow *MediaFilescanWindow::createConfigureWindow () {
	MediaFilescanConfigureWindow *window;

	window = new MediaFilescanConfigureWindow (windowWidth);
	window->layoutChangeCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowLayoutChanged, this);
	window->backCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowBackClicked, this);
	return (window);
}

MediaFilescanRunWindow *MediaFilescanWindow::createRunWindow () {
	MediaFilescanRunWindow *window;

	window = new MediaFilescanRunWindow (windowWidth);
	window->layoutChangeCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowLayoutChanged, this);
	window->backCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowBackClicked, this);
	return (window);
}

MediaFilescanCleanWindow *MediaFilescanWindow::createCleanWindow () {
	MediaFilescanCleanWindow *window;

	window = new MediaFilescanCleanWindow (windowWidth);
	window->layoutChangeCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowLayoutChanged, this);
	window->backCallback = Widget::EventCallbackContext (MediaFilescanWindow::contentWindowBackClicked, this);
	return (window);
}

void MediaFilescanWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	bool showstatus;

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
	dividerPanel->isVisible = isExpanded;
	mediaCountIcon->isVisible = (! isExpanded);

	showstatus = isExpanded;
	if (configureWindow) {
		configureWindow->isVisible = isExpanded;
		showstatus = false;
	}
	if (runWindow) {
		runWindow->isVisible = isExpanded;
		showstatus = false;
	}
	if (cleanWindow) {
		cleanWindow->isVisible = isExpanded;
		showstatus = false;
	}
	statusWindow->isVisible = showstatus;

	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void MediaFilescanWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerIcon->flowRight (&layoutFlow);
	progressRingPanel->position.assign (headerIcon->position.x + (headerIcon->width / 2.0f) - (progressRingPanel->width / 2.0f), headerIcon->position.y + (headerIcon->height / 2.0f) - (progressRingPanel->height / 2.0f));

	nameLabel->flowDown (&layoutFlow);
	if (isExpanded) {
		layoutFlow.y = heightPadding;
		nameLabel->centerVertical (&layoutFlow);
	}
	else {
		mediaCountIcon->flowRight (&layoutFlow);
	}

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);

	nextRowLayoutFlow ();
	if (isExpanded) {
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);

		nextRowLayoutFlow ();
		layoutFlow.x = 0.0f;
		layoutFlow.y -= UiConfiguration::instance->marginSize;
		if (statusWindow->isVisible) {
			statusWindow->setWindowWidth (windowWidth);
			statusWindow->reflow ();
			statusWindow->flowDown (&layoutFlow);
		}
		if (configureWindow) {
			configureWindow->setWindowWidth (windowWidth);
			configureWindow->reflow ();
			configureWindow->flowDown (&layoutFlow);
		}
		if (runWindow) {
			runWindow->setWindowWidth (windowWidth);
			runWindow->reflow ();
			runWindow->flowDown (&layoutFlow);
		}
		if (cleanWindow) {
			cleanWindow->setWindowWidth (windowWidth);
			cleanWindow->reflow ();
			cleanWindow->flowDown (&layoutFlow);
		}
		setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
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

void MediaFilescanWindow::doResize () {
	Panel::doResize ();
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	reflow ();
}

void MediaFilescanWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	bool taskrunning;

	Panel::doUpdate (msElapsed);
	MediaControl::instance->getStatus (&status);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		taskrunning = mediaControlStatus.isTaskRunning && ((mediaControlStatus.taskType == ClassId::MediaControlConfigureFilescanTask) || (mediaControlStatus.taskType == ClassId::MediaControlFilescanTask) || (mediaControlStatus.taskType == ClassId::MediaControlCleanFilescanTask));
		if (! progressRingPanel->isVisible) {
			if (taskrunning) {
				progressRingPanel->isVisible = true;
				reflow ();
			}
		}
		else {
			if (! taskrunning) {
				progressRingPanel->isVisible = false;
				reflow ();
			}
		}
		if (status.mediaCount >= 0) {
			mediaCountIcon->setText (StdString::createSprintf ("%i", status.mediaCount));
			mediaCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (status.mediaCount, UiTextId::MediaFile, UiTextId::MediaFiles));
		}
	}
}

void MediaFilescanWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

void MediaFilescanWindow::contentWindowLayoutChanged (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;

	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaFilescanWindow::contentWindowBackClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;

	it->statusWindow->isVisible = true;
	it->clearContentWindow ();
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaFilescanWindow::statusWindowScanClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;

	it->clearContentWindow ();
	it->runWindowHandle.assign (it->createRunWindow ());
	it->add (it->runWindow, contentZLevel);
	it->statusWindow->isVisible = false;
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaFilescanWindow::statusWindowCleanClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;

	it->clearContentWindow ();
	it->cleanWindowHandle.assign (it->createCleanWindow ());
	it->add (it->cleanWindow, contentZLevel);
	it->statusWindow->isVisible = false;
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}

void MediaFilescanWindow::statusWindowConfigureClicked (void *itPtr, Widget *widgetPtr) {
	MediaFilescanWindow *it = (MediaFilescanWindow *) itPtr;

	it->clearContentWindow ();
	it->configureWindowHandle.assign (it->createConfigureWindow ());
	it->add (it->configureWindow, contentZLevel);
	it->statusWindow->isVisible = false;
	it->reflow ();
	it->eventCallback (it->layoutChangeCallback);
}
