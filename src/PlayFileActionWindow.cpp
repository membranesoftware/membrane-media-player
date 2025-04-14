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
#include "Ui.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Font.h"
#include "TaskGroup.h"
#include "RecordStore.h"
#include "SystemInterface.h"
#include "MediaReader.h"
#include "Image.h"
#include "Button.h"
#include "ProgressBar.h"
#include "TextFieldWindow.h"
#include "PlayFileActionWindow.h"

PlayFileActionWindow::PlayFileActionWindow (SpriteGroup *playerUiSpriteGroup)
: Panel ()
, isReadingFile (false)
, isReadFileComplete (false)
{
	setFillBg (true, UiConfiguration::instance->lightPrimaryColor);

	playIcon = add (new Image (playerUiSpriteGroup->getSprite (SpriteId::PlayerUi_playMediaButton), Button::LargeFrame));
	playIcon->setDrawColor (true, UiConfiguration::instance->inverseTextColor);

	pathTextField = add (new TextFieldWindow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiText::instance->getText (UiTextId::PlayFileActionWindowPathPrompt)));
	pathTextField->widgetName.assign ("playFilePathTextField");
	pathTextField->valueChangeCallback = Widget::EventCallbackContext (PlayFileActionWindow::pathTextFieldValueChanged, this);
	pathTextField->fsBrowserPathSelectCallback = Widget::EventCallbackContext (PlayFileActionWindow::pathTextFieldFsBrowserPathSelected, this);
	pathTextField->setFixedPadding (true, 0.0f, 0.0f);
	pathTextField->setButtonsEnabled (TextFieldWindow::ClearButtonOption | TextFieldWindow::PasteButtonOption | TextFieldWindow::FsBrowseButtonOption | TextFieldWindow::FsBrowseButtonSortDirectoriesFirstOption);

	playButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (PlayFileActionWindow::playButtonClicked, this), UiText::instance->getText (UiTextId::PlayFileActionWindowPlayTooltip), "playFilePlayButton"));
	playButton->setInverseColor (true);
	playButton->setDisabled (true);

	progressBar = add (new ProgressBar (playButton->width, UiConfiguration::instance->progressBarHeight), 1);;
	progressBar->setIndeterminate (true);
	progressBar->isPanelSizeClipEnabled = true;
	progressBar->isInputSuspended = true;
	progressBar->isVisible = false;

	reflow ();
}
PlayFileActionWindow::~PlayFileActionWindow () {
}

void PlayFileActionWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	playIcon->flowRight (&layoutFlow);
	pathTextField->flowRight (&layoutFlow);
	playButton->flowDown (&layoutFlow);
	resetSize ();

	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = height;
	playIcon->centerVertical (&layoutFlow);
	pathTextField->centerVertical (&layoutFlow);
	playButton->centerVertical (&layoutFlow);
	if (progressBar->isVisible) {
		progressBar->position.assign (playButton->position.x, playButton->position.y + playButton->height - progressBar->height);
	}
}

void PlayFileActionWindow::assignKeyFocus () {
	pathTextField->assignKeyFocus ();
}

void PlayFileActionWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (isReadingFile) {
		if (isReadFileComplete) {
			eventCallback (mediaReadCallback);
			isReadingFile = false;
			progressBar->isVisible = false;
		}
	}
}

void PlayFileActionWindow::mouseClickFsBrowseButton () {
	pathTextField->mouseClickFsBrowseButton ();
}

void PlayFileActionWindow::pathTextFieldValueChanged (void *itPtr, Widget *widgetPtr) {
	PlayFileActionWindow *it = (PlayFileActionWindow *) itPtr;
	StdString value;

	value = it->pathTextField->getValue ();
	it->playButton->setDisabled (value.empty ());
}

void PlayFileActionWindow::pathTextFieldFsBrowserPathSelected (void *itPtr, Widget *widgetPtr) {
	PlayFileActionWindow *it = (PlayFileActionWindow *) itPtr;

	it->playButton->mouseClick ();
}

void PlayFileActionWindow::playButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayFileActionWindow *it = (PlayFileActionWindow *) itPtr;

	it->pathTextField->setDisabled (true);
	it->playButton->setDisabled (true);
	it->progressBar->isVisible = true;
	it->reflow ();

	it->mediaItem.clear ();
	it->loadErrorMessage.assign ("");
	it->isReadingFile = true;
	it->isReadFileComplete = false;
	it->retain ();
	TaskGroup::instance->run (TaskGroup::RunContext (PlayFileActionWindow::readMediaFile, it));
}

void PlayFileActionWindow::readMediaFile (void *itPtr) {
	PlayFileActionWindow *it = (PlayFileActionWindow *) itPtr;
	StdString path;
	OpResult result;
	MediaReader reader;

	path = it->pathTextField->getValue ();
	if (! path.empty ()) {
		reader.setMediaPath (path);
		result = reader.readMetadata ();
		if (result == OpResult::Success) {
			it->mediaItem.mediaPath = path;
			it->mediaItem.name = OsUtil::getPathBasename (path);
			it->mediaItem.mediaDirname = OsUtil::getPathDirname (path);
			if (! it->mediaItem.readMediaReader (reader)) {
				result = OpResult::MalformedDataError;
			}
		}
		if (result != OpResult::Success) {
			it->loadErrorMessage.assign (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized ());
		}
		else {
			it->mediaItem.id = RecordStore::instance->getRecordId (SystemInterface::CommandId_MediaItem);
		}
	}
	if (it->mediaItem.id.empty () && it->loadErrorMessage.empty ()) {
		it->loadErrorMessage.assign (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized ());
	}
	it->isReadFileComplete = true;
	it->release ();
}
