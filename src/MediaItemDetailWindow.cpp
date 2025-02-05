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
#include "UiConfiguration.h"
#include "UiText.h"
#include "MediaUtil.h"
#include "Json.h"
#include "Widget.h"
#include "Sprite.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Button.h"
#include "MediaItemDetailWindow.h"

constexpr const double windowWidthScale = 0.8f;
constexpr const double tagTextWidthScale = 0.1f;

MediaItemDetailWindow::MediaItemDetailWindow (Json *mediaItemRecord)
: MediaItemWindow (mediaItemRecord)
, isControlVisible (false)
, nameFont (UiConfiguration::BodyFont)
{
	StdString text;

	classId = ClassId::MediaItemDetailWindow;
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	if (mediaItem.isVideo) {
		text.appendSprintf ("%ix%i  ", mediaItem.width, mediaItem.height);
	}
	if (mediaItem.videoBitrate > 0) {
		text.appendSprintf ("%s  ", MediaUtil::getBitrateDisplayString (mediaItem.videoBitrate).c_str ());
	}
	else if (mediaItem.totalBitrate > 0) {
		text.appendSprintf ("%s  ", MediaUtil::getBitrateDisplayString (mediaItem.totalBitrate).c_str ());
	}
	text.appendSprintf ("%s  %s", UiText::instance->getByteCountText (mediaItem.mediaFileSize).c_str (), UiText::instance->getDurationText (mediaItem.duration).c_str ());
	descriptionLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
	descriptionLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	descriptionLabel->setPaddingScale (1.0f, 0.5f);
	descriptionLabel->setText (text);
	descriptionLabel->isVisible = true;

	mediaNameLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	mediaNameLabel->isInputSuspended = true;

	tagLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->mediumSecondaryColor)));
	tagLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	tagLabel->setPaddingScale (1.0f, 0.5f);
	tagLabel->isInputSuspended = true;
	tagLabel->isVisible = false;

	imageClickPanel = add (new Panel (), -1);
	imageClickPanel->mouseClickCallback = Widget::EventCallbackContext (MediaItemDetailWindow::imageClickPanelClicked, this);

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
MediaItemDetailWindow::~MediaItemDetailWindow () {
}

MediaItemDetailWindow *MediaItemDetailWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::MediaItemDetailWindow) ? (MediaItemDetailWindow *) widget : NULL);
}

void MediaItemDetailWindow::refreshDetailSize () {
	if (detailSize == Ui::SmallSize) {
		nameFont = UiConfiguration::CaptionFont;
	}
	else {
		nameFont = UiConfiguration::BodyFont;
	}
	mediaNameLabel->setFont (nameFont);
	MediaItemWindow::refreshDetailSize ();
}

void MediaItemDetailWindow::reflow () {
	double imagew, panelw, namew;

	if (! isControlVisible) {
		viewButton->isVisible = false;
	}
	else {
		viewButton->isVisible = true;
	}

	topLeftLayoutFlow ();
	imagew = mediaImage->width;
	if (mediaIconImage && mediaIconImage->isVisible) {
		mediaImage->isVisible = false;
		mediaIconImage->position.assign (layoutFlow.x + (imagew / 2.0f) - (mediaIconImage->width / 2.0f), layoutFlow.y);
	}
	else {
		constexpr const double mediaImageHeightOffset = 0.1f;
		mediaImage->position.assign (layoutFlow.x, layoutFlow.y - (mediaImage->height * mediaImageHeightOffset));
		mediaImage->isVisible = true;
	}
	layoutFlow.x += (imagew + UiConfiguration::instance->marginSize);

	panelw = detailMaxWidth * windowWidthScale;
	namew = panelw - layoutFlow.x;
	if (detailSize == Ui::SmallSize) {
		namew -= (descriptionLabel->width + UiConfiguration::instance->marginSize);
	}
	else {
		namew -= (viewButton->width + UiConfiguration::instance->marginSize);
	}

	if (mediaItem.tags.empty ()) {
		tagLabel->isVisible = false;
	}
	else {
		tagLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (mediaItem.tags.join (", "), panelw * tagTextWidthScale, Font::dotTruncateSuffix));
		tagLabel->isVisible = true;
		namew -= tagLabel->width;
	}

	mediaNameLabel->setText (UiConfiguration::instance->fonts[nameFont]->truncatedText (mediaItem.name, namew, Font::dotTruncateSuffix));

	mediaNameLabel->flowRight (&layoutFlow);
	tagLabel->flowRight (&layoutFlow);
	viewButton->flowRight (&layoutFlow);
	setFixedSize (true, panelw, layoutFlow.yExtent);
	imageClickPanel->position.assign (0.0f, 0.0f);
	imageClickPanel->setFixedSize (true, width, height);
	mediaNameLabel->position.assignY ((height / 2.0f) - (mediaNameLabel->height / 2.0f));
	if (detailSize == Ui::SmallSize) {
		descriptionLabel->position.assign (width - descriptionLabel->width, (height / 2.0f) - (descriptionLabel->height / 2.0f));
		tagLabel->position.assign (panelw - descriptionLabel->width - widthPadding - tagLabel->width - UiConfiguration::instance->marginSize, (height / 2.0f) - (tagLabel->height / 2.0f));
	}
	else {
		descriptionLabel->position.assign (imagew - descriptionLabel->width, (height / 2.0f) - (descriptionLabel->height / 2.0f));
		tagLabel->position.assign (panelw - viewButton->width - widthPadding - tagLabel->width - UiConfiguration::instance->marginSize, (height / 2.0f) - (tagLabel->height / 2.0f));
	}
	if (mediaIconImage && mediaIconImage->isVisible) {
		mediaIconImage->position.assignY ((height / 2.0f) - (mediaIconImage->height / 2.0f));
	}
	if (timestampLabel->isVisible) {
		timestampLabel->position.assign (mediaImage->position.x + mediaImage->width - timestampLabel->width, 0.0f);
	}

	viewButton->position.assign (width - viewButton->width, (height / 2.0f) - (viewButton->height / 2.0f));
}

bool MediaItemDetailWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	MediaItemWindow::doProcessMouseState (mouseState);
	if (isControlVisible && (! mouseState.isEntered)) {
		isControlVisible = false;
		reflow ();
	}
	else if ((! isControlVisible) && mouseState.isEntered) {
		isControlVisible = true;
		reflow ();
	}
	return (false);
}

void MediaItemDetailWindow::imageClickPanelClicked (void *itPtr, Widget *widgetPtr) {
	((MediaItemDetailWindow *) itPtr)->eventCallback (((MediaItemDetailWindow *) itPtr)->mediaImageClickCallback);
}
