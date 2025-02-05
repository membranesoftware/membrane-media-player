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
#include "UiText.h"
#include "UiTextId.h"
#include "UiConfiguration.h"
#include "SystemInterface.h"
#include "Input.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "Font.h"
#include "Button.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Toggle.h"
#include "Slider.h"
#include "TextFlow.h"
#include "ProgressRing.h"
#include "IconLabelWindow.h"
#include "Video.h"
#include "WaveformShader.h"
#include "PlayerTimelineWindow.h"
#include "PlayerWindow.h"

constexpr const double volumeSliderTrackWidthScale = 0.25f;
constexpr const int64_t fastSeekInterval = 30000;
constexpr const int controlHideDelay = 4800;
constexpr const double subtitleTextScale = 0.92f;
constexpr const double subtitleTextShadowColor = 0.08f;
constexpr const int subtitleTextShadowDx = -1;
constexpr const int subtitleTextShadowDy = 1;
constexpr const int videoZLevel = 0;
constexpr const int visualizerZLevel = 1;
constexpr const int subtitleZLevel = 2;
constexpr const int messageZLevel = 3;
constexpr const int timelineZLevel = 4;
constexpr const int controlZLevel = 5;

PlayerWindow::PlayerWindow (double windowWidth, double windowHeight, int soundMixVolume, bool isSoundMuted)
: Panel ()
, isDetached (false)
, windowWidth (windowWidth)
, windowHeight (windowHeight)
, isMaximized (false)
, isFullscreen (false)
, recordType (-1)
, playSeekPercent (0.0f)
, playSeekTimestamp (0)
, soundMixVolume (soundMixVolume)
, isSoundMuted (isSoundMuted)
, visualizerType (PlayerWindow::NoVisualizer)
, isSubtitleEnabled (true)
, isControlVisible (false)
, controlHideClock (0)
, controlHideMouseX (-1)
, controlHideMouseY (-1)
, nextPlaySeekPercent (0.0f)
, nextPlaySeekTimestamp (0)
, timelineHoverTimestamp (-1)
, timelinePopupTimestamp (-1)
, timelineHoverClock (0)
, timelinePopupHandle (&timelinePopup)
, waveformHandle (&waveform)
, subtitleTextHandle (&subtitleText)
, subtitleFont (UiConfiguration::MediumSubtitleFont)
, subtitleBottomMargin (0.0f)
, progressRingHandle (&progressRing)
, progressRingShowClock (0)
, isPlayStarting (false)
, maximizeWidth (1.0f)
, maximizeHeight (1.0f)
{
	classId = ClassId::PlayerWindow;
	progressRingShowClock = UiConfiguration::instance->activityIconLingerDuration;

	video = (Video *) addWidget (new Video (windowWidth, windowHeight, soundMixVolume, isSoundMuted), videoZLevel);
	video->retain ();
	video->setAudioIcon (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_audioIcon), UiConfiguration::instance->mediumSecondaryColor);
	video->fillBgColor.assign (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha);

	nameLabel = add (new LabelWindow (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)), timelineZLevel);
	nameLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	nameLabel->isVisible = false;

	pauseButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_pauseButton)), controlZLevel);
	pauseButton->isDropShadowDisabled = true;
	pauseButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::pauseButtonClicked, this);
	pauseButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	pauseButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	pauseButton->setInverseColor (true);
	pauseButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PauseButtonTooltip));
	pauseButton->isVisible = false;

	rewindButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_rewindButton)), controlZLevel);
	rewindButton->isDropShadowDisabled = true;
	rewindButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::rewindButtonClicked, this);
	rewindButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	rewindButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	rewindButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerWindowRewindButtonTooltip));
	rewindButton->setInverseColor (true);
	rewindButton->isVisible = false;

	forwardButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_forwardButton)), controlZLevel);
	forwardButton->isDropShadowDisabled = true;
	forwardButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::forwardButtonClicked, this);
	forwardButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	forwardButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	forwardButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerWindowForwardButtonTooltip));
	forwardButton->setInverseColor (true);
	forwardButton->isVisible = false;

	closeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton)), controlZLevel);
	closeButton->isDropShadowDisabled = true;
	closeButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::closeButtonClicked, this);
	closeButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	closeButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	closeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::Close).capitalized ());
	closeButton->setInverseColor (true);
	closeButton->isVisible = false;

	visualizerButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_visualizerButton)), controlZLevel);
	visualizerButton->isDropShadowDisabled = true;
	visualizerButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::visualizerButtonClicked, this);
	visualizerButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	visualizerButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	visualizerButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerWindowVisualizerButtonTooltip));
	visualizerButton->setInverseColor (true);
	visualizerButton->isVisible = false;

	subtitleToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_subtitleOffButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_subtitleOnButton)), controlZLevel);
	subtitleToggle->setChecked (true);
	subtitleToggle->stateChangeCallback = Widget::EventCallbackContext (PlayerWindow::subtitleToggleStateChanged, this);
	subtitleToggle->setImageColor (UiConfiguration::instance->inverseTextColor);
	subtitleToggle->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	subtitleToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::PlayerWindowSubtitleNotAvailableTooltip), UiText::instance->getText (UiTextId::PlayerWindowSubtitleNotAvailableTooltip));
	subtitleToggle->setInverseColor (true);
	subtitleToggle->setDisabled (true);
	subtitleToggle->isVisible = false;

	timeline = (PlayerTimelineWindow *) addWidget (new PlayerTimelineWindow (windowWidth - pauseButton->width), timelineZLevel);
	timeline->positionHoverCallback = Widget::EventCallbackContext (PlayerWindow::timelineWindowPositionHovered, this);
	timeline->positionClickCallback = Widget::EventCallbackContext (PlayerWindow::timelineWindowPositionClicked, this);
	timeline->isShortTimestampEnabled = true;
	timeline->setInverseColor (true);
	timeline->setLeftSnap (true);
	timeline->isVisible = false;

	detachButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_detachPlayerButton)), controlZLevel);
	detachButton->isDropShadowDisabled = true;
	detachButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::detachButtonClicked, this);
	detachButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	detachButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	detachButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerWindowDetachButtonTooltip));
	detachButton->setInverseColor (true);
	detachButton->isVisible = false;

	maximizeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_browserPlayButton)), controlZLevel);
	maximizeButton->isDropShadowDisabled = true;
	maximizeButton->mouseClickCallback = Widget::EventCallbackContext (PlayerWindow::maximizeButtonClicked, this);
	maximizeButton->setImageColor (UiConfiguration::instance->inverseTextColor);
	maximizeButton->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	maximizeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlayerWindowMaximizeButtonTooltip));
	maximizeButton->setInverseColor (true);
	maximizeButton->isVisible = false;

	soundToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_soundOffButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_soundOnButton)), controlZLevel);
	soundToggle->setChecked (! isSoundMuted);
	soundToggle->stateChangeCallback = Widget::EventCallbackContext (PlayerWindow::soundToggleStateChanged, this);
	soundToggle->setImageColor (UiConfiguration::instance->inverseTextColor);
	soundToggle->setTransparentBackground (UiConfiguration::instance->buttonUnfocusedShadeAlpha);
	soundToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Unmute).capitalized (), UiText::instance->getText (UiTextId::Mute).capitalized ());
	soundToggle->setInverseColor (true);
	soundToggle->isVisible = false;

	soundVolumePanel = add (new Panel (), controlZLevel);
	soundVolumeSlider = soundVolumePanel->add (new Slider ());
	soundVolumeSlider->setValue ((double) soundMixVolume / (double) SoundMixer::maxMixVolume);
	soundVolumeSlider->valueChangeCallback = Widget::EventCallbackContext (PlayerWindow::soundVolumeSliderValueChanged, this);
	soundVolumeSlider->setTrackWidthScale (volumeSliderTrackWidthScale);
	soundVolumeSlider->setInverseColor (true);
	soundVolumePanel->setFillBg (true, Color (0.5f, 0.5f, 0.5f, UiConfiguration::instance->buttonInverseFocusedShadeAlpha));
	soundVolumePanel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SoundMixLevel).capitalized ());
	soundVolumePanel->isVisible = false;

	messageIcon = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeErrorIcon), StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->errorTextColor), messageZLevel);
	messageIcon->setIconImageColor (UiConfiguration::instance->darkInverseTextColor);
	messageIcon->isVisible = false;

	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}
PlayerWindow::~PlayerWindow () {
	nextPlayPath.assign ("");
	timelinePopupHandle.destroyAndClear ();
	if (video) {
		video->stop ();
		video->release ();
		video = NULL;
	}
}

PlayerWindow *PlayerWindow::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::PlayerWindow) ? (PlayerWindow *) widget : NULL);
}

void PlayerWindow::setWidgetNames () {
	if ((recordType != SystemInterface::CommandId_MediaItem) || targetMedia.name.empty ()) {
		return;
	}
	widgetName.sprintf ("%sPlayerWindow", targetMedia.name.c_str ());
	closeButton->widgetName.sprintf ("%sPlayerCloseButton", targetMedia.name.c_str ());
	pauseButton->widgetName.sprintf ("%sPlayerPauseButton", targetMedia.name.c_str ());
	rewindButton->widgetName.sprintf ("%sPlayerRewindButton", targetMedia.name.c_str ());
	forwardButton->widgetName.sprintf ("%sPlayerForwardButton", targetMedia.name.c_str ());
	detachButton->widgetName.sprintf ("%sPlayerDetachButton", targetMedia.name.c_str ());
	maximizeButton->widgetName.sprintf ("%sPlayerMaximizeButton", targetMedia.name.c_str ());
	soundToggle->widgetName.sprintf ("%sPlayerSoundToggle", targetMedia.name.c_str ());
	soundVolumeSlider->widgetName.sprintf ("%sPlayerSoundVolumeSlider", targetMedia.name.c_str ());
	visualizerButton->widgetName.sprintf ("%sPlayerVisualizerButton", targetMedia.name.c_str ());
	subtitleToggle->widgetName.sprintf ("%sPlayerSubtitleToggle", targetMedia.name.c_str ());
	timeline->widgetName.sprintf ("%sPlayerTimelineBar", targetMedia.name.c_str ());
}

void PlayerWindow::setWindowSize (double widthValue, double heightValue) {
	double w, h;

	if (FLOAT_EQUALS (windowWidth, widthValue) && FLOAT_EQUALS (windowHeight, heightValue)) {
		return;
	}
	windowWidth = widthValue;
	windowHeight = heightValue;
	video->setVideoSize (windowWidth, windowHeight);
	if (waveform) {
		getWaveformSize (&w, &h);
		waveform->setShaderSize (w, h);
	}
	if (subtitleText) {
		subtitleText->setViewWidth (windowWidth * subtitleTextScale);
	}
	reflow ();
}

void PlayerWindow::maximize (double targetWidth, double targetHeight, const Position &targetPosition, bool isFullscreenMaximize) {
	maximizeWidth = width;
	maximizeHeight = height;
	maximizePosition.assign (position);
	setWindowSize (targetWidth, targetHeight);
	setFillBg (true, Color (0.0f, 0.0f, 0.0f));
	position.assign (targetPosition);
	isMaximized = true;
	isFullscreen = isFullscreenMaximize;
	resetInputState ();
	setControlVisible (false);
}

void PlayerWindow::unmaximize () {
	if (! isMaximized) {
		return;
	}
	setWindowSize (maximizeWidth, maximizeHeight);
	setFillBg (false);
	position.assign (maximizePosition);
	isMaximized = false;
	isFullscreen = false;
	resetInputState ();
	setControlVisible (false);
}

void PlayerWindow::reflow () {
	StdString *namestr;
	double x, y, w, scale;

	namestr = NULL;
	if (recordType == SystemInterface::CommandId_MediaItem) {
		namestr = &(targetMedia.name);
	}
	if ((! namestr) || namestr->empty ()) {
		nameLabel->setText (StdString ());
	}
	else {
		w = windowWidth - closeButton->width - (nameLabel->widthPadding * 2.0f);
		nameLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (*namestr, w, Font::dotTruncateSuffix));
	}

	nameLabel->position.assign (0.0f, 0.0f);
	y = nameLabel->height;
	maximizeButton->position.assign (0.0f, y);
	y += maximizeButton->height;
	detachButton->position.assign (0.0f, y);

	x = maximizeButton->width + UiConfiguration::instance->marginSize;
	y = nameLabel->height;
	soundToggle->position.assign (x, y);
	x += soundToggle->width;
	soundVolumePanel->setFixedSize (true, soundVolumeSlider->width + (UiConfiguration::instance->paddingSize * 2.0f), soundToggle->height);
	soundVolumeSlider->position.assign ((soundVolumePanel->width / 2.0f) - (soundVolumeSlider->width / 2.0f), (soundVolumePanel->height / 2.0f) - (soundVolumeSlider->height / 2.0f));
	soundVolumePanel->position.assign (x, y);

	if (isMaximized) {
		x += soundVolumePanel->width + UiConfiguration::instance->marginSize;
	}
	else {
		x = soundToggle->position.x;
		y += soundVolumePanel->height;
	}
	if (visualizerButton->isVisible) {
		visualizerButton->position.assign (x, y);
		x += visualizerButton->width;
	}
	if (subtitleToggle->isVisible) {
		subtitleToggle->position.assign (x, y);
		x += subtitleToggle->width;
	}

	x = 0.0f;
	if (rewindButton->isVisible && forwardButton->isVisible) {
		rewindButton->position.assign (x, windowHeight - rewindButton->height);
		x += rewindButton->width + UiConfiguration::instance->marginSize;
		w = windowWidth - rewindButton->width - forwardButton->width - (UiConfiguration::instance->marginSize * 2.0f);
		timeline->setBarWidth (w);
		timeline->position.assign (x, windowHeight - timeline->height);
		x += timeline->width + UiConfiguration::instance->marginSize;
		forwardButton->position.assign (x, windowHeight - forwardButton->height);
		pauseButton->position.assign (x, forwardButton->position.y - pauseButton->height - UiConfiguration::instance->marginSize);
	}
	else {
		w = windowWidth - pauseButton->width - UiConfiguration::instance->marginSize;
		timeline->setBarWidth (w);
		timeline->position.assign (x, windowHeight - timeline->height);
		x += timeline->width + UiConfiguration::instance->marginSize;
		pauseButton->position.assign (x, windowHeight - pauseButton->height);
	}
	closeButton->position.assign (windowWidth - closeButton->width, 0.0f);

	setFixedSize (true, windowWidth, windowHeight);
	if (messageIcon->isVisible) {
		messageIcon->position.assign ((width / 2.0f) - (messageIcon->width / 2.0f), (height / 2.0f) - (messageIcon->height / 2.0f));
	}
	if (progressRing) {
		progressRing->position.assign ((width / 2.0f) - (progressRing->width / 2.0f), (height / 2.0f) - (progressRing->height / 2.0f));
	}
	if (waveform) {
		waveform->position.assign ((width / 2.0f) - (waveform->width / 2.0f), height - waveform->height);
	}

	if (width < (App::instance->drawableWidth * 0.25f)) {
		subtitleFont = UiConfiguration::SmallSubtitleFont;
		scale = 0.5f;
	}
	else if (width > (App::instance->drawableWidth * 0.5f)) {
		subtitleFont = UiConfiguration::LargeSubtitleFont;
		scale = 1.0f;
	}
	else {
		subtitleFont = UiConfiguration::MediumSubtitleFont;
		scale = 0.75f;
	}
	subtitleBottomMargin = floor (((double) UiConfiguration::instance->fonts[subtitleFont]->maxLineHeight) * scale);
	if (subtitleText) {
		subtitleText->setFont (subtitleFont);
		subtitleText->position.assign ((width / 2.0f) - (subtitleText->width / 2.0f), height - subtitleText->height - subtitleBottomMargin);
	}
}

void PlayerWindow::setControlVisible (bool visible) {
	isControlVisible = visible;
	if (isControlVisible) {
		nameLabel->isVisible = true;
		timeline->isVisible = true;
		closeButton->isVisible = true;
		pauseButton->isVisible = true;
		forwardButton->isVisible = (! isDetached) || isMaximized;
		rewindButton->isVisible = (! isDetached) || isMaximized;
		visualizerButton->isVisible = (! isDetached) || isMaximized;
		subtitleToggle->isVisible = (! isDetached) || isMaximized;
		detachButton->isVisible = (! isDetached) && (! isMaximized);
		maximizeButton->isVisible = true;
		soundToggle->isVisible = ((! isDetached) || isMaximized) && (! isFullscreen);
		soundVolumePanel->isVisible = soundToggle->isVisible && (! isSoundMuted);
		controlHideClock = controlHideDelay;
		controlHideMouseX = Input::instance->mouseX;
		controlHideMouseY = Input::instance->mouseY;
	}
	else {
		nameLabel->isVisible = false;
		timeline->isVisible = false;
		closeButton->isVisible = false;
		pauseButton->isVisible = false;
		forwardButton->isVisible = false;
		rewindButton->isVisible = false;
		visualizerButton->isVisible = false;
		subtitleToggle->isVisible = false;
		detachButton->isVisible = false;
		maximizeButton->isVisible = false;
		soundToggle->isVisible = false;
		soundVolumePanel->isVisible = false;
	}
	reflow ();
}

void PlayerWindow::doUpdate (int msElapsed) {
	double x, y, w, h;
	StdString text;

	Panel::doUpdate (msElapsed);

	if (isDetached && playlistId.empty () && (! isPlaying ())) {
		isDestroyed = true;
		return;
	}
	if (isPlayStarting) {
		if (nextPlayPath.empty () && video->isPlayPresented) {
			isPlayStarting = false;
			progressRingShowClock = UiConfiguration::instance->activityIconLingerDuration;
			progressRingHandle.destroyAndClear ();
			reflow ();
		}
		else {
			if (! progressRing) {
				if (! video->isPlayFailed) {
					progressRingShowClock -= msElapsed;
					if (progressRingShowClock <= 0) {
						progressRingHandle.destroyAndAssign (new ProgressRing (UiConfiguration::instance->progressRingSize));
						progressRing->setIndeterminate (true);
						add (progressRing, messageZLevel);
						reflow ();
					}
				}
			}
		}
	}

	if (! isControlVisible) {
		if (isMouseEntered && (! isFullscreen)) {
			if ((controlHideMouseX != Input::instance->mouseX) || (controlHideMouseY != Input::instance->mouseY)) {
				setControlVisible (true);
			}
		}
	}
	else {
		if (! isMouseEntered) {
			setControlVisible (false);
		}
		else {
			if ((! video->isMouseEntered) || (controlHideMouseX != Input::instance->mouseX) || (controlHideMouseY != Input::instance->mouseY)) {
				controlHideClock = controlHideDelay;
				controlHideMouseX = Input::instance->mouseX;
				controlHideMouseY = Input::instance->mouseY;
			}
			else {
				controlHideClock -= msElapsed;
				if (controlHideClock <= 0) {
					setControlVisible (false);
				}
			}
		}
	}

	timeline->setPlayPosition (video->isPlaying ? video->playTimestamp : -1);
	if ((recordType < 0) || (! timeline->isVisible) || (timelineHoverTimestamp < 0) || (video->videoStreamFrameWidth <= 0) || (video->videoStreamFrameHeight <= 0)) {
		timelinePopupHandle.destroyAndClear ();
		timelinePopupTimestamp = -1;
	}
	else {
		if (timelineHoverTimestamp != timelinePopupTimestamp) {
			timelinePopupHandle.destroyAndClear ();
			timelinePopupTimestamp = -1;
			timelineHoverClock -= msElapsed;
			if (timelineHoverClock <= 0) {
				w = App::instance->drawableWidth * UiConfiguration::instance->popupThumbnailImageScale;
				h = w * ((double) video->videoStreamFrameHeight) / ((double) video->videoStreamFrameWidth);
				x = timeline->screenX + timeline->hoverPosition - (w / 2.0f);
				y = timeline->screenY - h - (UiConfiguration::instance->marginSize / 2.0f);
				timelinePopupHandle.destroyAndAssign (new ImageWindow ());
				timelinePopup->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
				timelinePopup->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
				timelinePopup->setLoadingSize (w, h);
				timelinePopup->onLoadScale (w);
				if (recordType == SystemInterface::CommandId_MediaItem) {
					timelinePopup->loadSeekTimestampVideoFrame (targetMedia.mediaPath, timelineHoverTimestamp, true);
				}
				timelinePopup->isInputSuspended = true;
				timelinePopup->position.assignBounded (x, y, 0.0f, y, App::instance->drawableWidth - w, y);
				timelinePopupTimestamp = timelineHoverTimestamp;
				App::instance->rootPanel->add (timelinePopup, App::instance->rootPanel->maxWidgetZLevel + 1);
			}
		}
	}

	if (video->isPlaying && video->isSubtitleLoaded) {
		if (subtitleToggle->isDisabled) {
			subtitleToggle->setDisabled (false);
			subtitleToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::PlayerWindowSubtitleEnableTooltip), UiText::instance->getText (UiTextId::PlayerWindowSubtitleDisableTooltip));
		}

		if (! isSubtitleEnabled) {
			if (subtitleText) {
				subtitleTextContent.assign ("");
				subtitleTextHandle.destroyAndClear ();
			}
		}
		else {
			if (! subtitleText) {
				subtitleTextHandle.assign (new TextFlow (windowWidth * subtitleTextScale, subtitleFont));
				subtitleText->setPaddingScale (1.0f, 0.0f);
				subtitleText->setTextColor (UiConfiguration::instance->inverseTextColor);
				subtitleText->setTextShadowed (true, Color (subtitleTextShadowColor, subtitleTextShadowColor, subtitleTextShadowColor), subtitleTextShadowDx, subtitleTextShadowDy);
				subtitleText->setCenterAligned (true);
				subtitleText->isVisible = false;
				add (subtitleText, subtitleZLevel);
				subtitleTextContent.assign ("");
			}
			text = video->getSubtitleText ();
			if (! subtitleTextContent.equals (text)) {
				subtitleTextContent.assign (text);
				if (subtitleTextContent.empty ()) {
					subtitleText->isVisible = false;
				}
				else {
					subtitleText->setText (subtitleTextContent);
					subtitleText->position.assign ((width / 2.0f) - (subtitleText->width / 2.0f), height - subtitleText->height - subtitleBottomMargin);
					subtitleText->isVisible = true;
				}
			}
		}
	}
	else {
		if (subtitleText) {
			subtitleTextContent.assign ("");
			subtitleTextHandle.destroyAndClear ();
		}
		if (! subtitleToggle->isDisabled) {
			subtitleToggle->setDisabled (true);
			subtitleToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::PlayerWindowSubtitleNotAvailableTooltip), UiText::instance->getText (UiTextId::PlayerWindowSubtitleNotAvailableTooltip));
		}
	}
}

bool PlayerWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	bool consumed;

	consumed = Panel::doProcessMouseState (mouseState);
	if (! consumed) {
		if (isFullscreen && mouseState.isEntered) {
			consumed = true;
		}
	}
	return (consumed);
}

void PlayerWindow::closeButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;

	it->stop ();
	it->isDestroyed = true;
}

void PlayerWindow::videoPlayEnded (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;

	if (it->nextPlayPath.empty ()) {
		if (it->video->isPlayFailed) {
			it->messageIcon->setText (it->video->lastErrorMessage);
			it->messageIcon->isVisible = true;
			it->reflow ();
		}
		else {
			it->messageIcon->isVisible = false;
		}
	}
	else {
		it->playPath.assign (it->nextPlayPath);
		it->playSeekPercent = it->nextPlaySeekPercent;
		it->playSeekTimestamp = it->nextPlaySeekTimestamp;
		it->nextPlayPath.assign ("");
		it->nextPlaySeekPercent = 0.0f;
		it->nextPlaySeekTimestamp = 0;
		it->messageIcon->isVisible = false;

		it->video->setPlayPath (it->playPath);
		if (it->playSeekTimestamp >= 0) {
			it->video->setPlaySeekTimestamp (it->playSeekTimestamp);
		}
		else {
			it->video->setPlaySeekPercent (it->playSeekPercent);
		}
		it->isPlayStarting = true;
		it->retain ();
		it->video->play (Widget::EventCallbackContext (PlayerWindow::videoPlayEnded, it));
	}
	it->timelinePopupHandle.destroyAndClear ();
	it->timeline->setHighlightedPosition (-1);
	it->timelineHoverTimestamp = -1;
	it->timelinePopupTimestamp = -1;
	it->release ();
}

void PlayerWindow::stop () {
	video->stop ();
	timelinePopupHandle.destroyAndClear ();
	timelinePopupTimestamp = -1;
}

void PlayerWindow::setPlayMedia (const StdString &mediaId) {
	if (! targetMedia.mediaId.equals (mediaId)) {
		targetMedia.clear ();
		if (! targetMedia.readRecordStore (mediaId)) {
			return;
		}
		timeline->readRecord (mediaId);
	}
	recordType = SystemInterface::CommandId_MediaItem;
	setWidgetNames ();
	playSeekPercent = 0.0f;
	playSeekTimestamp = 0;
}

void PlayerWindow::setPlaySeekPercent (double seekPercent) {
	if (seekPercent < 0.0f) {
		seekPercent = 0.0f;
	}
	if (seekPercent > 100.0f) {
		seekPercent = 100.0f;
	}
	playSeekPercent = seekPercent;
	playSeekTimestamp = -1;
}

void PlayerWindow::setPlaySeekTimestamp (int64_t seekTimestamp) {
	if (seekTimestamp < 0) {
		seekTimestamp = 0;
	}
	playSeekTimestamp = seekTimestamp;
	playSeekPercent = 0.0f;
}

void PlayerWindow::play () {
	if (recordType == SystemInterface::CommandId_MediaItem) {
		playPath.assign (targetMedia.mediaPath);
	}
	else {
		return;
	}

	messageIcon->isVisible = false;
	isPlayStarting = true;
	if (! video->isPlaying) {
		video->setPlayPath (playPath);
		if (playSeekTimestamp >= 0) {
			video->setPlaySeekTimestamp (playSeekTimestamp);
		}
		else {
			video->setPlaySeekPercent (playSeekPercent);
		}
		retain ();
		video->play (Widget::EventCallbackContext (PlayerWindow::videoPlayEnded, this));
	}
	else {
		nextPlayPath.assign (playPath);
		nextPlaySeekPercent = playSeekPercent;
		nextPlaySeekTimestamp = playSeekTimestamp;
		if (! video->isStopped) {
			video->stop ();
		}
	}
	reflow ();
}

void PlayerWindow::pauseButtonClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerWindow *) itPtr)->pause ();
}

void PlayerWindow::pause () {
	video->pause ();
}

bool PlayerWindow::isPlaying () {
	return (video->isPlaying || (! nextPlayPath.empty ()));
}

bool PlayerWindow::isPaused () {
	return (video->isPlaying && video->isPaused);
}

void PlayerWindow::forwardButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	int64_t delta, pos;

	delta = it->timeline->playDuration * 33 / 100;
	if (delta > fastSeekInterval) {
		delta = fastSeekInterval;
	}
	pos = it->video->playTimestamp + delta;
	if (pos > it->timeline->playDuration) {
		pos = it->timeline->playDuration;
	}
	it->executeSeekTimestamp (pos);
}
void PlayerWindow::rewindButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	int64_t delta, pos;

	delta = it->timeline->playDuration * 33 / 100;
	if (delta > fastSeekInterval) {
		delta = fastSeekInterval;
	}
	pos = it->video->playTimestamp - delta;
	if (pos < 0) {
		pos = 0;
	}
	it->executeSeekTimestamp (pos);
}
void PlayerWindow::executeSeekTimestamp (int64_t targetTimestamp) {
	isPlayStarting = true;
	if (! video->isPlaying) {
		setPlaySeekTimestamp (targetTimestamp);
		video->setPlayPath (playPath);
		video->setPlaySeekTimestamp (playSeekTimestamp);
		messageIcon->isVisible = false;
		retain ();
		video->play (Widget::EventCallbackContext (PlayerWindow::videoPlayEnded, this));
	}
	else {
		if (targetTimestamp < 0) {
			targetTimestamp = 0;
		}
		nextPlayPath.assign (playPath);
		nextPlaySeekTimestamp = targetTimestamp;
		nextPlaySeekPercent = 0.0f;
		if (! video->isStopped) {
			video->stop ();
		}
	}
}

void PlayerWindow::visualizerButtonClicked (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	int t;

	t = it->visualizerType;
	++t;
	if (t >= PlayerWindow::VisualizerTypeCount) {
		t = PlayerWindow::NoVisualizer;
	}
	it->setVisualizerType (t);
	it->eventCallback (it->settingsChangeCallback);
}

void PlayerWindow::timelineWindowPositionHovered (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	PlayerTimelineWindow *timeline = (PlayerTimelineWindow *) widgetPtr;
	double t;

	if (it->recordType == SystemInterface::CommandId_MediaItem) {
		t = -1.0f;
		if (timeline->hoverSeekPercent >= 0.0f) {
			t = timeline->hoverSeekPercent / 100.0f * (double) timeline->playDuration;
		}
		if (it->targetMedia.isVideo) {
			it->timelineHoverTimestamp = (int64_t) t;
			it->timelineHoverClock = UiConfiguration::instance->mouseHoverThreshold;
		}
		timeline->setHighlightedPosition ((int64_t) t);
	}
}

void PlayerWindow::timelineWindowPositionClicked (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	PlayerTimelineWindow *timeline = (PlayerTimelineWindow *) widgetPtr;

	if (it->recordType == SystemInterface::CommandId_MediaItem) {
		it->executeSeekPercent (timeline->clickSeekPercent);
	}
}
void PlayerWindow::executeSeekPercent (double targetPercent) {
	isPlayStarting = true;
	if (! video->isPlaying) {
		setPlaySeekPercent (targetPercent);
		video->setPlayPath (playPath);
		video->setPlaySeekPercent (playSeekPercent);
		messageIcon->isVisible = false;
		retain ();
		video->play (Widget::EventCallbackContext (PlayerWindow::videoPlayEnded, this));
	}
	else {
		if (targetPercent < 0.0f) {
			targetPercent = 0.0f;
		}
		if (targetPercent > 100.0f) {
			targetPercent = 100.0f;
		}
		nextPlayPath.assign (playPath);
		nextPlaySeekPercent = targetPercent;
		nextPlaySeekTimestamp = -1;
		if (! video->isStopped) {
			video->stop ();
		}
	}
}

void PlayerWindow::detachButtonClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerWindow *) itPtr)->eventCallback (((PlayerWindow *) itPtr)->detachCallback);
}

void PlayerWindow::maximizeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((PlayerWindow *) itPtr)->eventCallback (((PlayerWindow *) itPtr)->maximizeCallback);
}

void PlayerWindow::soundToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->isSoundMuted = (! toggle->isChecked);
	it->soundVolumePanel->isVisible = (! it->isSoundMuted);
	it->video->setSoundMuted (it->isSoundMuted);
	it->eventCallback (it->settingsChangeCallback);
}

void PlayerWindow::soundVolumeSliderValueChanged (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	Slider *slider = (Slider *) widgetPtr;

	it->soundMixVolume = (int) floor (slider->value * (double) SoundMixer::maxMixVolume);
	it->video->setSoundMixVolume (it->soundMixVolume);
	it->eventCallback (it->settingsChangeCallback);
}

void PlayerWindow::subtitleToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	PlayerWindow *it = (PlayerWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->isSubtitleEnabled = toggle->isChecked;
	it->eventCallback (it->settingsChangeCallback);
}

void PlayerWindow::setVisualizerType (int typeValue) {
	double w, h;

	if (visualizerType == typeValue) {
		return;
	}
	visualizerType = typeValue;
	if (visualizerType == PlayerWindow::NoVisualizer) {
		if (waveform) {
			waveformHandle.destroyAndClear ();
			reflow ();
		}
	}
	else {
		getWaveformSize (&w, &h);
		if (waveform) {
			waveform->setShaderSize (w, h);
		}
		else {
			waveformHandle.assign (new WaveformShader (w, h));
			waveform->isInputSuspended = true;
			waveform->setSourceVideo (video);
			addWidget (waveform, visualizerZLevel);
		}
		reflow ();
	}
}

void PlayerWindow::setSubtitleEnabled (bool enable) {
	if (isSubtitleEnabled == enable) {
		return;
	}
	isSubtitleEnabled = enable;
	subtitleToggle->setChecked (isSubtitleEnabled, true);
}

void PlayerWindow::getWaveformSize (double *w, double *h) {
	switch (visualizerType) {
		case PlayerWindow::LargeWaveformVisualizer: {
			*w = windowWidth;
			*h = windowHeight;
			break;
		}
		case PlayerWindow::MediumWaveformVisualizer: {
			*w = windowWidth;
			*h = windowHeight * 0.25f;
			break;
		}
		default: {
			*w = windowWidth * 0.63f;
			*h = windowHeight * 0.12f;
			break;
		}
	}
}
