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
#include "Button.h"
#include "MediaControlTaskWindow.h"
#include "MediaFilescanRunWindow.h"

MediaFilescanRunWindow::MediaFilescanRunWindow (double windowWidth)
: Panel ()
, windowWidth (windowWidth)
{
	mediaControlStatus.updateCount = -1;

	nameLabel = add (new Label (UiText::instance->getText (UiTextId::MediaFilescanRunWindowTitle), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));

	taskWindow = new MediaControlTaskWindow (windowWidth, ClassId::MediaControlFilescanTask, UiText::instance->getText (UiTextId::MediaFilescanRunWindowPrompt));
	addWidget (taskWindow);

	backButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton), Widget::EventCallbackContext (MediaFilescanRunWindow::backButtonClicked, this), UiText::instance->getText (UiTextId::UiBackTooltip), "mediaScanBackButton"));
	executeButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_mediaScanButton), Widget::EventCallbackContext (MediaFilescanRunWindow::executeButtonClicked, this), UiText::instance->getText (UiTextId::MediaFilescanRunWindowExecuteTooltip).capitalized (), "mediaScanExecuteButton"));
	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (MediaFilescanRunWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::Cancel).capitalized (), "mediaScanCancelButton"));
	cancelButton->isVisible = false;
	clearButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (MediaFilescanRunWindow::clearButtonClicked, this), UiText::instance->getText (UiTextId::Ok).capitalized (), "mediaScanClearButton"));
	clearButton->isVisible = false;

	reflow ();
}
MediaFilescanRunWindow::~MediaFilescanRunWindow () {
}

void MediaFilescanRunWindow::setWindowWidth (double w) {
	if (FLOAT_EQUALS (windowWidth, w)) {
		return;
	}
	windowWidth = w;
	reflow ();
}

void MediaFilescanRunWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	nameLabel->flowDown (&layoutFlow);
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

void MediaFilescanRunWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;
	MediaControl::TaskResult taskresult;

	MediaControl::instance->getStatus (&status, ClassId::MediaControlFilescanTask, &taskresult);
	if (mediaControlStatus.updateCount != status.updateCount) {
		mediaControlStatus = status;
		taskWindow->updateStatus (mediaControlStatus, taskresult);
		if (MediaControl::instance->isTaskRunning (ClassId::MediaControlFilescanTask)) {
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

void MediaFilescanRunWindow::backButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaFilescanRunWindow *) itPtr)->eventCallback (((MediaFilescanRunWindow *) itPtr)->backCallback);
}
void MediaFilescanRunWindow::executeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->filescan ();
	((MediaFilescanRunWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}
void MediaFilescanRunWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->cancelTask (ClassId::MediaControlFilescanTask);
	((MediaFilescanRunWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}
void MediaFilescanRunWindow::clearButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaControl::instance->clearTaskResult (ClassId::MediaControlFilescanTask);
	((MediaFilescanRunWindow *) itPtr)->mediaControlStatus.updateCount = -1;
}
