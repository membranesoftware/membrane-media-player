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
#include "Image.h"
#include "Label.h"
#include "TextFlow.h"
#include "ProgressBar.h"
#include "MediaControlTaskWindow.h"

constexpr const double progressBarScale = 0.38f;

MediaControlTaskWindow::MediaControlTaskWindow (double windowWidth, int targetTaskType, const StdString &promptTextValue)
: Panel ()
, windowWidth (windowWidth)
, targetTaskType (targetTaskType)
{
	promptText = add (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	promptText->setPaddingScale (1.0f, 0.0f);
	promptText->setText (promptTextValue);

	iconSpriteId.assign (SpriteId::SpriteGroup_taskInProgressIcon);
	iconImage = add (new Image (SpriteGroup::instance->getSprite (iconSpriteId.c_str ())));
	iconImage->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	iconImage->isVisible = false;

	text1Label = add (new Label (StdString (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	text1Label->isVisible = false;
	text2Label = add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	text2Label->isVisible = false;

	progressBar = add (new ProgressBar ());
	progressBar->setSize (windowWidth * progressBarScale, UiConfiguration::instance->progressBarHeight);
	progressBar->isVisible = false;

	reflow ();
}
MediaControlTaskWindow::~MediaControlTaskWindow () {
}

void MediaControlTaskWindow::setWindowWidth (double w) {
	if (FLOAT_EQUALS (windowWidth, w)) {
		return;
	}
	windowWidth = w;
	reflow ();
}

void MediaControlTaskWindow::updateStatus (const MediaControl::Status &status, const MediaControl::TaskResult &taskResult) {
	StdString spriteid;

	if (status.isTaskRunning && (status.taskType == targetTaskType)) {
		promptText->isVisible = false;
		spriteid.assign (SpriteId::SpriteGroup_taskInProgressIcon);
		text1Label->setText (status.taskText1);
		text1Label->isVisible = true;
		if (! status.taskText2.empty ()) {
			text2Label->setText (UiConfiguration::instance->fonts[text2Label->textFontType]->truncatedText (status.taskText2, windowWidth - (widthPadding * 2.0f) - iconImage->width - UiConfiguration::instance->marginSize, Font::dotTruncateSuffix));
			text2Label->isVisible = true;
		}
		else {
			text2Label->isVisible = false;
		}

		if (status.taskProgressPercent >= 0.0f) {
			progressBar->setIndeterminate (false);
			progressBar->setProgress (status.taskProgressPercent, 100.0f);
		}
		else {
			progressBar->setIndeterminate (true);
		}
		progressBar->isVisible = true;
	}
	else if (status.isTaskRunning && MediaControl::instance->isTaskRunning (targetTaskType)) {
		promptText->isVisible = false;
		text2Label->isVisible = false;
		spriteid.assign (SpriteId::SpriteGroup_taskInProgressIcon);
		text1Label->setText (UiText::instance->getText (UiTextId::MediaControlTaskStatusWindowWaitingPrompt));
		text1Label->isVisible = true;
		progressBar->setIndeterminate (true);
		progressBar->isVisible = true;
	}
	else if (! taskResult.text1.empty ()) {
		spriteid.assign (SpriteId::SpriteGroup_largeMediaIcon);
		promptText->isVisible = false;
		progressBar->isVisible = false;

		text1Label->setText (taskResult.text1);
		text1Label->isVisible = true;
		if (! taskResult.text2.empty ()) {
			text2Label->setText (UiConfiguration::instance->fonts[text2Label->textFontType]->truncatedText (taskResult.text2, windowWidth - (widthPadding * 2.0f) - iconImage->width - UiConfiguration::instance->marginSize, Font::dotTruncateSuffix));
			text2Label->isVisible = true;
		}
		else {
			text2Label->isVisible = false;
		}
	}
	else {
		iconImage->isVisible = false;
		text1Label->isVisible = false;
		text2Label->isVisible = false;
		progressBar->isVisible = false;
		promptText->isVisible = true;
	}
	if ((! spriteid.empty ()) && (! iconSpriteId.equals (spriteid))) {
		iconSpriteId.assign (spriteid);
		iconImage->setSprite (SpriteGroup::instance->getSprite (iconSpriteId.c_str ()));
	}
	reflow ();
}

void MediaControlTaskWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	if (promptText->isVisible) {
		promptText->setViewWidth (windowWidth - (widthPadding * 2.0f));
		promptText->flowDown (&layoutFlow);
	}
	if (iconImage->isVisible) {
		iconImage->flowRight (&layoutFlow);
	}
	if (text1Label->isVisible) {
		text1Label->flowDown (&layoutFlow);
	}
	if (text2Label->isVisible) {
		text2Label->flowDown (&layoutFlow);
	}
	if (progressBar->isVisible) {
		progressBar->flowDown (&layoutFlow);
	}
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
}
