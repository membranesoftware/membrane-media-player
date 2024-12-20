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
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Widget.h"
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"
#include "UiConfiguration.h"
#include "PlayerTimelineWindow.h"

constexpr const int guideSegmentCount = 4;
constexpr const double markerLineAlpha = 0.5f;
constexpr const double timestampFillAlpha = 0.9f;
constexpr const double timestampFillHeightScale = 0.38f;

PlayerTimelineWindow::PlayerTimelineWindow (double barWidth)
: Panel ()
, isShortTimestampEnabled (false)
, recordType (-1)
, isInverseColor (false)
, hoverPosition (-1.0f)
, clickPosition (-1.0f)
, playPosition (-1)
, playDuration (0)
, highlightedPosition (-1)
, highlightPositionMarkerHandle (&highlightPositionMarker)
, playPositionMarkerHandle (&playPositionMarker)
, barWidth (barWidth)
, barHeight (0.0f)
, minDurationUnitType (UiText::MillisecondsUnit)
{
	classId = ClassId::PlayerTimelineWindow;
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	setBorder (true, UiConfiguration::instance->darkBackgroundColor);

	startTimeLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)), 5);
	startTimeLabel->setFixedPadding (true, UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->textLineHeightMargin);
	startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	startTimeLabel->isInputSuspended = true;
	startTimeLabel->reflow ();

	endTimeLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)), 5);
	endTimeLabel->setFixedPadding (true, UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->reflow ();

	highlightTimeLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)), 6);
	highlightTimeLabel->setFixedPadding (true, UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->isVisible = false;
	highlightTimeLabel->reflow ();

	guideSegmentPanel = add (new Panel (), 1);
	guideSegmentPanel->isInputSuspended = true;

	timestampFillPanel = add (new Panel (), 2);
	timestampFillPanel->isInputSuspended = true;

	barHeight = startTimeLabel->height * 2.0f;
	populateGuideSegments ();
	reflow ();
}
PlayerTimelineWindow::~PlayerTimelineWindow () {
}

void PlayerTimelineWindow::readRecord (const StdString &recordIdValue) {
	Json *record;
	double w;

	if (recordId.equals (recordIdValue)) {
		return;
	}
	recordId.assign ("");
	recordType = RecordStore::instance->getRecordIdCommand (recordIdValue);
	if (recordType != SystemInterface::CommandId_MediaItem) {
		return;
	}
	record = RecordStore::instance->find (recordIdValue, recordType);
	if (! record) {
		recordType = -1;
	}
	else {
		playDuration = SystemInterface::instance->getCommandNumberParam (record, SystemInterface::Field_duration, (int64_t) 0);
		delete (record);
	}
	if (recordType < 0) {
		return;
	}
	recordId.assign (recordIdValue);
	minDurationUnitType = UiText::instance->getMinTimespanUnit (playDuration);
	startTimeLabel->setText (UiText::instance->getTimespanText (0.0f, minDurationUnitType, isShortTimestampEnabled));
	w = startTimeLabel->getText ().length () * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth;
	startTimeLabel->setWindowWidth (w, true);
	endTimeLabel->setText (UiText::instance->getTimespanText (playDuration, minDurationUnitType, isShortTimestampEnabled));
	reflow ();
}

void PlayerTimelineWindow::setBarWidth (double widthValue) {
	if (FLOAT_EQUALS (barWidth, widthValue)) {
		return;
	}
	barWidth = widthValue;
	populateGuideSegments ();
	populateTimestampFill ();
	reflow ();
}

void PlayerTimelineWindow::setInverseColor (bool enable) {
	if (isInverseColor == enable) {
		return;
	}
	isInverseColor = enable;
	if (isInverseColor) {
		setFillBg (true, UiConfiguration::instance->darkInverseBackgroundColor);
		setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
		startTimeLabel->setFillBg (true, Color (0.5f, 0.5f, 0.5f, UiConfiguration::instance->scrimBackgroundAlpha));
		endTimeLabel->setFillBg (true, Color (0.5f, 0.5f, 0.5f, UiConfiguration::instance->scrimBackgroundAlpha));
		highlightTimeLabel->setFillBg (true, Color (0.5f, 0.5f, 0.5f, UiConfiguration::instance->scrimBackgroundAlpha));
	}
	else {
		setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		setBorder (true, UiConfiguration::instance->darkBackgroundColor);
		startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	}
	populateGuideSegments ();
	populateTimestampFill ();
}

void PlayerTimelineWindow::populateGuideSegments () {
	Panel *panel;
	double x, dx;
	Color color;

	guideSegmentPanel->clear ();
	guideSegmentPanel->setFixedSize (true, barWidth, barHeight);

	x = 0.0f;
	dx = barWidth / (double) guideSegmentCount;
	if (dx < 1.0f) {
		dx = 1.0f;
	}
	x += dx;
	while (x < barWidth) {
		panel = guideSegmentPanel->add (new Panel ());
		color.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		panel->setFillBg (true, Color (color.r, color.g, color.b, markerLineAlpha));
		panel->setFixedSize (true, 1.0f, barHeight);
		panel->position.assign (x, 0.0f);
		panel->isInputSuspended = true;
		x += dx;
	}
}

void PlayerTimelineWindow::setHighlightColor (const Color &color) {
	highlightTimeLabel->setTextColor (color);
}

void PlayerTimelineWindow::setHighlightedPosition (int64_t timePosition) {
	Color color;
	double x;

	if (highlightPositionMarker) {
		highlightPositionMarker->bgColor.translate (highlightPositionMarker->bgColor.copy (0.0f), UiConfiguration::instance->shortColorTranslateDuration);
		highlightPositionMarker->setDestroyDelay (UiConfiguration::instance->shortColorTranslateDuration);
		highlightPositionMarkerHandle.clear ();
	}
	if ((timePosition < 0) || (playDuration <= 0)) {
		highlightedPosition = -1;
		highlightTimeLabel->isVisible = false;
	}
	else {
		highlightedPosition = timePosition;
		x = ((double) timePosition) * barWidth / (double) playDuration;
		if (x > barWidth) {
			x = barWidth;
		}
		highlightPositionMarkerHandle.destroyAndAssign (new Panel ());
		add (highlightPositionMarker, 4);
		color.assign (isInverseColor ? UiConfiguration::instance->lightSecondaryColor : UiConfiguration::instance->lightPrimaryColor);
		highlightPositionMarker->setFillBg (true, color.copy (0.0f));
		highlightPositionMarker->bgColor.translate (color, UiConfiguration::instance->shortColorTranslateDuration);
		highlightPositionMarker->setFixedSize (true, UiConfiguration::instance->timelineMarkerWidth, barHeight - 2.0f);
		highlightPositionMarker->position.assign (x - (UiConfiguration::instance->timelineMarkerWidth / 2.0f), 1.0f);
		highlightPositionMarker->isInputSuspended = true;

		highlightTimeLabel->setText (UiText::instance->getTimespanText ((int64_t) timePosition, minDurationUnitType, isShortTimestampEnabled));
		x = highlightPositionMarker->position.x + (highlightPositionMarker->width / 2.0f) - (highlightTimeLabel->width / 2.0f);
		if (x < 0.0f) {
			x = 0.0f;
		}
		if (x > (barWidth - highlightTimeLabel->width)) {
			x = (barWidth - highlightTimeLabel->width);
		}
		highlightTimeLabel->position.assignX (x);
		highlightTimeLabel->isVisible = true;
	}
}

void PlayerTimelineWindow::setPlayPosition (int64_t timePosition) {
	std::vector<double>::const_iterator i1, i2;
	double x;

	if ((playPosition < 0) && (timePosition < 0)) {
		return;
	}
	if (playPosition == timePosition) {
		return;
	}
	playPosition = timePosition;
	if ((playDuration <= 0) || (playPosition < 0)) {
		playPositionMarkerHandle.destroyAndClear ();
		startTimeLabel->setText (UiText::instance->getTimespanText (0.0f, minDurationUnitType, isShortTimestampEnabled));
	}
	else {
		x = ((double) timePosition) * barWidth / (double) playDuration;
		if (x > barWidth) {
			x = barWidth;
		}
		if (! playPositionMarker) {
			playPositionMarkerHandle.destroyAndAssign (new Panel ());
			add (playPositionMarker, 3);
			playPositionMarker->setFillBg (true, isInverseColor ? Color (1.0f, 1.0f, 1.0f, UiConfiguration::instance->scrimBackgroundAlpha) : Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
			playPositionMarker->setFixedSize (true, (UiConfiguration::instance->timelineMarkerWidth / 2.0f), barHeight - 2.0f);
			playPositionMarker->isInputSuspended = true;
		}
		playPositionMarker->position.assign (x - (playPositionMarker->width / 2.0f), 1.0f);
		startTimeLabel->setText (UiText::instance->getTimespanText (playPosition, minDurationUnitType, isShortTimestampEnabled));
	}
}

void PlayerTimelineWindow::addTimestampFill (int timestamp) {
	timestampFillPositions.push_back (timestamp);
	timestampFillPositions.sort ();
	populateTimestampFill ();
	reflow ();
}

void PlayerTimelineWindow::populateTimestampFill () {
	std::list<int64_t>::const_iterator i1, i2;
	Panel *panel;
	Color color;
	double x, lastx, panelx, dx, w, minw;

	timestampFillPanel->setFixedSize (true, barWidth, barHeight * timestampFillHeightScale);
	timestampFillPanel->clear ();
	if (playDuration <= 0) {
		return;
	}
	lastx = 0.0f;
	panelx = -1.0f;
	minw = UiConfiguration::instance->paddingSize / 2.0f;
	i1 = timestampFillPositions.cbegin ();
	i2 = timestampFillPositions.cend ();
	while (i1 != i2) {
		x = barWidth * (double) *i1 / (double) playDuration;
		if (panelx < 0.0f) {
			panelx = x;
			w = minw;
		}
		else {
			dx = x - lastx;
			if (dx <= minw) {
				w += dx;
			}
			else {
				panel = timestampFillPanel->add (new Panel ());
				color.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
				panel->setFillBg (true, Color (color.r, color.g, color.b, timestampFillAlpha));
				panel->setFixedSize (true, w, timestampFillPanel->height);
				panel->position.assign (panelx - (minw / 2.0f), 0.0f);
				panel->isInputSuspended = true;
				panelx = x;
				w = minw;
			}
		}
		lastx = x;
		++i1;
	}
	if (panelx >= 0.0f) {
		panel = timestampFillPanel->add (new Panel ());
		color.assign (isInverseColor ? UiConfiguration::instance->inverseTextColor : UiConfiguration::instance->primaryTextColor);
		panel->setFillBg (true, Color (color.r, color.g, color.b, timestampFillAlpha));
		panel->setFixedSize (true, w, timestampFillPanel->height);
		panel->position.assign (panelx - (minw / 2.0f), 0.0f);
		panel->isInputSuspended = true;
	}
}

void PlayerTimelineWindow::reflow () {
	setFixedSize (true, barWidth, barHeight);
	guideSegmentPanel->position.assign (0.0f, 0.0f);
	timestampFillPanel->position.assign (0.0f, (barHeight / 2.0f) - (timestampFillPanel->height / 2.0f));
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

bool PlayerTimelineWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (mouseState.isEntered) {
		if (! FLOAT_EQUALS (hoverPosition, mouseState.enterDeltaX)) {
			hoverPosition = mouseState.enterDeltaX;
			eventCallback (positionHoverCallback);
		}
		if (mouseState.isLeftClicked) {
			clickPosition = mouseState.enterDeltaX;
			eventCallback (positionClickCallback);
		}
	}
	else {
		if (hoverPosition >= 0.0f) {
			hoverPosition = -1.0f;
			eventCallback (positionHoverCallback);
		}
	}
	return (false);
}
