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
#include "Ui.h"
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "Database.h"
#include "TaskGroup.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "MediaControl.h"
#include "MediaItem.h"
#include "Image.h"
#include "Button.h"
#include "TextFlow.h"
#include "StatsWindow.h"
#include "MediaFilescanStatusWindow.h"

MediaFilescanStatusWindow::MediaFilescanStatusWindow (double windowWidth)
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
	statsWindow->setItem (UiText::instance->getText (UiTextId::Status).capitalized (), StdString (), StdString ("mediaScanStatus"));
	statsWindow->setItem (UiText::instance->getText (UiTextId::MediaFiles).capitalized (), StdString (), StdString ("mediaScanMediaFiles"));
	statsWindow->setItem (UiText::instance->getText (UiTextId::TotalFileSize).capitalized (), StdString ());
	statsWindow->setItem (UiText::instance->getText (UiTextId::TotalDuration).capitalized (), StdString ());

	promptIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon)));
	promptIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	promptIcon->isVisible = false;

	promptText = add (new TextFlow (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::CaptionFont));
	promptText->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	promptText->isVisible = false;

	scanButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_mediaScanButton), Widget::EventCallbackContext (MediaFilescanStatusWindow::scanButtonClicked, this), UiText::instance->getText (UiTextId::ScanMediaTooltip), "mediaScanScanButton"));
	cleanButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cleanButton), Widget::EventCallbackContext (MediaFilescanStatusWindow::cleanButtonClicked, this), UiText::instance->getText (UiTextId::CleanMediaTooltip), "mediaScanCleanButton"));
	configureButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_settingsGearButton), Widget::EventCallbackContext (MediaFilescanStatusWindow::configureButtonClicked, this), UiText::instance->getText (UiTextId::Configure).capitalized (), "mediaScanConfigureButton"));

	isFilescanConfigured = MediaControl::instance->isFilescanConfigured;
	if (! isFilescanConfigured) {
		scanButton->isVisible = false;
		cleanButton->isVisible = false;
		statsWindow->setItem (UiText::instance->getText (UiTextId::Status).capitalized (), UiText::instance->getText (UiTextId::ConfigurationRequired).capitalized ());
		statsWindow->setItemTextColor (UiText::instance->getText (UiTextId::Status).capitalized (), UiConfiguration::instance->errorTextColor);
		promptIcon->isVisible = true;
		promptText->setText (UiText::instance->getText (UiTextId::MediaFilescanConfigurePrompt));
		promptText->isVisible = true;
	}

	reflow ();
}
MediaFilescanStatusWindow::~MediaFilescanStatusWindow () {
}

void MediaFilescanStatusWindow::setWindowWidth (double w) {
	if (FLOAT_EQUALS (windowWidth, w)) {
		return;
	}
	windowWidth = w;
	reflow ();
}

void MediaFilescanStatusWindow::doUpdate (int msElapsed) {
	MediaControl::Status status;

	if (! isFilescanConfigured) {
		isFilescanConfigured = MediaControl::instance->isFilescanConfigured;
		if (isFilescanConfigured) {
			promptIcon->isVisible = false;
			promptText->isVisible = false;
			scanButton->isVisible = true;
			cleanButton->isVisible = true;
		}
	}
	if (isFilescanConfigured) {
		MediaControl::instance->getStatus (&status);
		if (mediaControlStatus.updateCount != status.updateCount) {
			mediaControlStatus = status;
			if ((mediaControlStatus.taskType >= 0) && mediaControlStatus.isTaskRunning) {
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
				TaskGroup::instance->run (TaskGroup::RunContext (MediaFilescanStatusWindow::loadMetadata, this));
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
	}
	Panel::doUpdate (msElapsed);
}

void MediaFilescanStatusWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	statsWindow->reflow ();
	statsWindow->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	if (promptIcon->isVisible && promptText->isVisible) {
		promptIcon->flowRight (&layoutFlow);
		promptText->setViewWidth (windowWidth - (UiConfiguration::instance->paddingSize * 2.0f) - layoutFlow.xExtent);
		promptText->flowRight (&layoutFlow);
		promptIcon->centerVertical (&layoutFlow);
	}

	nextRowLayoutFlow ();
	configureButton->flowRight (&layoutFlow);
	if (cleanButton->isVisible) {
		cleanButton->flowRight (&layoutFlow);
	}
	if (scanButton->isVisible) {
		scanButton->flowRight (&layoutFlow);
	}
	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);

	bottomRightLayoutFlow ();
	if (cleanButton->isVisible && scanButton->isVisible) {
		scanButton->flowLeft (&layoutFlow);
		cleanButton->flowLeft (&layoutFlow);
	}
	else {
		configureButton->flowLeft (&layoutFlow);
	}
}

void MediaFilescanStatusWindow::scanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaFilescanStatusWindow *) itPtr)->eventCallback (((MediaFilescanStatusWindow *) itPtr)->scanCallback);
}
void MediaFilescanStatusWindow::cleanButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaFilescanStatusWindow *) itPtr)->eventCallback (((MediaFilescanStatusWindow *) itPtr)->cleanCallback);
}
void MediaFilescanStatusWindow::configureButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaFilescanStatusWindow *) itPtr)->eventCallback (((MediaFilescanStatusWindow *) itPtr)->configureCallback);
}

void MediaFilescanStatusWindow::loadMetadata (void *itPtr) {
	MediaFilescanStatusWindow *it = (MediaFilescanStatusWindow *) itPtr;
	StdString dbpath, errmsg;
	int64_t sizetotal, durationtotal;

	if (MediaControl::instance->openDatabase (&dbpath) != OpResult::Success) {
		it->release ();
		return;
	}
	if (! MediaItem::readDatabaseMetadata (dbpath, MediaControl::filescanTableName, &errmsg, &sizetotal, &durationtotal)) {
		Log::debug ("Failed to read media metadata; err=%s", errmsg.c_str ());
		sizetotal = 0;
		durationtotal = 0;
	}
	Database::instance->close (dbpath);
	it->mediaSizeTotal = sizetotal;
	it->mediaDurationTotal = durationtotal;
	it->isLoadMetadataComplete = true;
	it->release ();
}
