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
// Panel that shows a media player and playback controls
#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include "SoundMixer.h"
#include "MediaItem.h"
#include "Position.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class Button;
class Video;
class LabelWindow;
class ImageWindow;
class Toggle;
class Slider;
class ProgressRing;
class IconLabelWindow;
class PlayerTimelineWindow;

class PlayerWindow : public Panel {
public:
	PlayerWindow (double windowWidth, double windowHeight, int soundMixVolume = SoundMixer::maxMixVolume, bool isSoundMuted = false);
	~PlayerWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static PlayerWindow *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext detachCallback;
	Widget::EventCallbackContext maximizeCallback;
	bool isDetached;
	StdString playlistId;

	// Read-only data members
	double windowWidth;
	double windowHeight;
	bool isMaximized;
	bool isFullscreen;
	int recordType;
	MediaItem targetMedia;
	StdString playPath;
	double playSeekPercent;
	int64_t playSeekTimestamp;
	int soundMixVolume;
	bool isSoundMuted;

	// Set the window size
	void setWindowSize (double widthValue, double heightValue);

	// Resize the window and set maximized state
	void maximize (double targetWidth, double targetHeight, const Position &targetPosition, bool isFullscreenMaximize);

	// Clear maximized state and restore the previous window size
	void unmaximize ();

	// Stop video playback in progress
	void stop ();

	// Set the MediaItem record to target for playback
	void setPlayMedia (const StdString &mediaId);

	// Set the seek position for play operations
	void setPlaySeekPercent (double seekPercent);
	void setPlaySeekTimestamp (int64_t seekTimestamp);

	// Start playback targeting the configured record
	void play ();

	// Pause or unpause playback in progress
	void pause ();

	// Return true if playback is in progress
	bool isPlaying ();

	// Return true if playback is in progress and paused
	bool isPaused ();

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	StdString toStringDetail ();
	void doUpdate (int msElapsed);
	bool doProcessMouseState (const Widget::MouseState &mouseState);

private:
	// Callback functions
	static void videoPlayEnded (void *itPtr, Widget *widgetPtr);
	static void timelineWindowPositionHovered (void *itPtr, Widget *widgetPtr);
	static void timelineWindowPositionClicked (void *itPtr, Widget *widgetPtr);
	static void closeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void pauseButtonClicked (void *itPtr, Widget *widgetPtr);
	static void forwardButtonClicked (void *itPtr, Widget *widgetPtr);
	static void rewindButtonClicked (void *itPtr, Widget *widgetPtr);
	static void detachButtonClicked (void *itPtr, Widget *widgetPtr);
	static void maximizeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void soundToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void soundVolumeSliderValueChanged (void *itPtr, Widget *widgetPtr);

	// Set widgetName values for control widgets
	void setWidgetNames ();

	// Restart playback at the specified position
	void executeSeekPercent (double targetPercent);
	void executeSeekTimestamp (int64_t targetTimestamp);

	// Set control visible state
	void setControlVisible (bool visible);

	bool isControlVisible;
	int controlHideClock;
	int controlHideMouseX;
	int controlHideMouseY;
	Video *video;
	LabelWindow *nameLabel;
	PlayerTimelineWindow *timeline;
	Button *closeButton;
	Button *pauseButton;
	Button *rewindButton;
	Button *forwardButton;
	Button *detachButton;
	Button *maximizeButton;
	Toggle *soundToggle;
	Slider *soundVolumeSlider;
	Panel *soundVolumePanel;
	IconLabelWindow *messageIcon;
	StdString nextPlayPath;
	double nextPlaySeekPercent;
	int64_t nextPlaySeekTimestamp;
	int64_t timelineHoverTimestamp;
	int64_t timelinePopupTimestamp;
	int timelineHoverClock;
	WidgetHandle<ImageWindow> timelinePopupHandle;
	ImageWindow *timelinePopup;
	WidgetHandle<ProgressRing> progressRingHandle;
	ProgressRing *progressRing;
	int progressRingShowClock;
	bool isPlayStarting;
	Position maximizePosition;
	double maximizeWidth;
	double maximizeHeight;
};
#endif
