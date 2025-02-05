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
// Panel that shows a horizontal bar with position indicators, representing the playback timeline for a media item
#ifndef PLAYER_TIMELINE_WINDOW_H
#define PLAYER_TIMELINE_WINDOW_H

#include "UiText.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class LabelWindow;

class PlayerTimelineWindow : public Panel {
public:
	PlayerTimelineWindow (double timelineBarWidth);
	~PlayerTimelineWindow ();

	// Read-write data members
	Widget::EventCallbackContext positionHoverCallback;
	Widget::EventCallbackContext positionClickCallback;
	bool isShortTimestampEnabled;

	// Read-only data members
	StdString recordId;
	int recordType;
	bool isInverseColor;
	double hoverPosition; // A negative value indicates that the mouse is positioned outside the timeline bar
	double clickPosition;
	double hoverSeekPercent; // A negative value indicates that the mouse is positioned outside the timeline bar
	double clickSeekPercent;
	bool isLeftSnapEnabled;
	int64_t playPosition;
	int64_t playDuration;
	int64_t highlightedPosition;
	std::list<int64_t> timestampFillPositions;

	// Reset the bar to show content from the MediaItem record matching recordIdValue
	void readRecord (const StdString &recordIdValue);

	// Set the bar's width
	void setBarWidth (double widthValue);

	// Set the enable state for the bar's left snap option. If enabled, the bar provides a larger area on its left side for the zero seek position.
	void setLeftSnap (bool enable);

	// Set the bar's inverse color option
	void setInverseColor (bool enable);

	// Set the highlight marker's text color
	void setHighlightColor (const Color &color);

	// Set the bar to show a highlight marker at the specified stream timestamp. A negative timePosition value indicates that any existing highlight should be cleared.
	void setHighlightedPosition (int64_t timePosition);

	// Set the bar to show a play position indicator at the specified stream timestamp. A negative index value indicates that any existing position indicator should be cleared.
	void setPlayPosition (int64_t timePosition);

	// Set the bar's timestamp fill to include the specified value
	void addTimestampFill (int timestamp);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	bool doProcessMouseState (const Widget::MouseState &mouseState);

private:
	// Clear guideSegmentPanel and populate it with a set of segment guidelines
	void populateGuideSegments ();

	// Clear timestampFillPanel and populate it with a set of fill areas
	void populateTimestampFill ();

	// Assign the playPositionMarker draw position
	void repositionPlayMarker ();

	Panel *guideSegmentPanel;
	Panel *timestampFillPanel;
	WidgetHandle<Panel> highlightPositionMarkerHandle;
	Panel *highlightPositionMarker;
	WidgetHandle<Panel> playPositionMarkerHandle;
	Panel *playPositionMarker;
	LabelWindow *startTimeLabel;
	LabelWindow *endTimeLabel;
	LabelWindow *highlightTimeLabel;
	double barWidth;
	double barHeight;
	double barLeftSnapWidth;
	UiText::TimespanUnit minDurationUnitType;
};
#endif
