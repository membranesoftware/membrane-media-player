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
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "UiStack.h"
#include "PlayerUi.h"
#include "RenderResource.h"
#include "SoundMixer.h"
#include "Color.h"
#include "Button.h"
#include "Toggle.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "ToggleWindow.h"
#include "Toolbar.h"
#include "MainToolbarWindow.h"
#include "SettingsWindow.h"

constexpr const double windowWidthScale = 0.33f;
constexpr const double sliderWidthScale = 1.5f;
constexpr const double headerImageAspectRatio = 25.0f / 9.0f;

SettingsWindow::SettingsWindow ()
: Panel ()
{
	HashMap *prefs;
	Slider *slider;
	StdString modename;
	double scale;
	int mode;

	setFixedSize (true, App::instance->rootPanel->width * windowWidthScale, App::instance->rootPanel->height);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	headerImage = add (new ImageWindow ());
	headerImage->widgetName.assign ("settingsWindowHeaderImage");
	headerImage->onLoadScale (width, width / headerImageAspectRatio);
	headerImage->setWindowSize (true, width, width / headerImageAspectRatio);
	headerImage->loadImageFile (StdString::createSprintf ("bg/settings/%s/000.png", App::instance->imagePrefix.c_str ()));

	titleLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::Settings).capitalized (), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor)));
	titleLabel->setTextColor (UiConfiguration::instance->inverseTextColor);
	titleLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));

	closeButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_exitButton)), 1);
	closeButton->widgetName.assign ("settingsWindowCloseButton");
	closeButton->mouseClickCallback = Widget::EventCallbackContext (SettingsWindow::closeButtonClicked, this);
	closeButton->setColorBackground (Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	closeButton->setInverseColor (true);

	windowOptionsLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::AppWindow).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	windowOptionsLabel->setFixedPadding (true, 0.0f, 0.0f);
	windowOptionsLabel->reflow ();

	prefs = App::instance->lockPrefs ();
	mode = prefs->find (UiStack::windowSizeSettingKey, -1);
	modename = prefs->find (App::displayModeKey, "");
	App::instance->unlockPrefs ();
	if (mode < 0) {
		if (! modename.empty ()) {
			mode = RenderResource::instance->getNamedDisplayMode (modename);
		}
		if (mode < 0) {
			mode = RenderResource::instance->getFitWindowDisplayMode (App::instance->windowWidth, App::instance->windowHeight);
		}
		if (mode >= 0) {
			prefs = App::instance->lockPrefs ();
			prefs->insert (UiStack::windowSizeSettingKey, mode);
			App::instance->unlockPrefs ();
		}
	}
	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	if (mode == 0) {
		slider->setValue (0.0f, true);
	}
	else if (mode == 1) {
		slider->setValue (0.25f, true);
	}
	else if (mode == 2) {
		slider->setValue (0.5f, true);
	}
	else if (mode == 3) {
		slider->setValue (0.75f, true);
	}
	else if (mode == 4) {
		slider->setValue (1.0f, true);
	}
	windowSizeSlider = add (new SliderWindow (slider));
	windowSizeSlider->setTrackWidthScale (sliderWidthScale);
	windowSizeSlider->setPaddingScale (1.0f, 0.0f);
	windowSizeSlider->valueChangeCallback = Widget::EventCallbackContext (SettingsWindow::windowSizeSliderChanged, this);
	windowSizeSlider->setValueNameFunction (SettingsWindow::windowSizeSliderValueName);
	if (App::instance->isFullscreen) {
		windowSizeSlider->setDisabled (true);
	}

	fullscreenToggle = add (new ToggleWindow (new Toggle ()));
	fullscreenToggle->stateChangeCallback = Widget::EventCallbackContext (SettingsWindow::fullscreenToggleStateChanged, this);
	fullscreenToggle->setPaddingScale (1.0f, 0.0f);
	fullscreenToggle->setText (UiText::instance->getText (UiTextId::FullscreenWindow).capitalized ());
	fullscreenToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	fullscreenToggle->setChecked (App::instance->isFullscreen, true);

	textSizeLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::TextSize).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	textSizeLabel->setFixedPadding (true, 0.0f, 0.0f);
	textSizeLabel->reflow ();

	slider = new Slider ();
	slider->addSnapValue (0.0f);
	slider->addSnapValue (0.25f);
	slider->addSnapValue (0.5f);
	slider->addSnapValue (0.75f);
	slider->addSnapValue (1.0f);
	scale = App::instance->fontScale;
	if (FLOAT_EQUALS (scale, RenderResource::instance->fontScales[0])) {
		slider->setValue (0.0f);
	}
	else if (FLOAT_EQUALS (scale, RenderResource::instance->fontScales[1])) {
		slider->setValue (0.25f);
	}
	else if (FLOAT_EQUALS (scale, RenderResource::instance->fontScales[2])) {
		slider->setValue (0.5f);
	}
	else if (FLOAT_EQUALS (scale, RenderResource::instance->fontScales[3])) {
		slider->setValue (0.75f);
	}
	else if (FLOAT_EQUALS (scale, RenderResource::instance->fontScales[4])) {
		slider->setValue (1.0f);
	}
	textSizeSlider = add (new SliderWindow (slider));
	textSizeSlider->setTrackWidthScale (sliderWidthScale);
	textSizeSlider->setPaddingScale (1.0f, 0.0f);
	textSizeSlider->valueChangeCallback = Widget::EventCallbackContext (SettingsWindow::textSizeSliderChanged, this);
	textSizeSlider->setValueNameFunction (SettingsWindow::textSizeSliderValueName);

	showClockToggle = add (new ToggleWindow (new Toggle ()));
	showClockToggle->stateChangeCallback = Widget::EventCallbackContext (SettingsWindow::showClockToggleStateChanged, this);
	showClockToggle->setPaddingScale (1.0f, 0.0f);
	showClockToggle->setText (UiText::instance->getText (UiTextId::ShowClock).capitalized ());
	showClockToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	showClockToggle->setChecked (UiStack::instance->mainToolbarWindow->isClockEnabled, true);

	soundVolumeLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::SoundVolume).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	soundVolumeLabel->setFixedPadding (true, 0.0f, 0.0f);
	soundVolumeLabel->reflow ();

	soundVolumeSlider = add (new SliderWindow (new Slider ()));
	soundVolumeSlider->setTrackWidthScale (sliderWidthScale);
	soundVolumeSlider->setPaddingScale (1.0f, 0.0f);
	soundVolumeSlider->valueChangeCallback = Widget::EventCallbackContext (SettingsWindow::soundVolumeSliderChanged, this);
	soundVolumeSlider->setValue ((double) SoundMixer::instance->masterMixVolume / (double) SoundMixer::maxMixVolume, true);
	soundVolumeSlider->reflow ();

	networkLabel = add (new LabelWindow (new Label (UiText::instance->getText (UiTextId::Network).capitalized (), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor)));
	networkLabel->setFixedPadding (true, 0.0f, 0.0f);
	networkLabel->reflow ();

	checkUpdatesToggle = add (new ToggleWindow (new Toggle ()));
	checkUpdatesToggle->stateChangeCallback = Widget::EventCallbackContext (SettingsWindow::checkUpdatesToggleStateChanged, this);
	checkUpdatesToggle->setPaddingScale (1.0f, 0.0f);
	checkUpdatesToggle->setText (UiText::instance->getText (UiTextId::CheckUpdatesOnStartup).capitalized ());
	checkUpdatesToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	checkUpdatesToggle->setChecked (App::instance->isStartUpdateEnabled, true);

	reflow ();
}
SettingsWindow::~SettingsWindow () {
}

void SettingsWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();

	headerImage->position.assign (0.0f, 0.0f);
	titleLabel->position.assign (widthPadding, headerImage->height - titleLabel->height - heightPadding);
	closeButton->position.assign (width - closeButton->width - widthPadding, headerImage->height - closeButton->height - heightPadding);

	layoutFlow.y = headerImage->height + heightPadding + UiConfiguration::instance->marginSize;
	windowOptionsLabel->flowDown (&layoutFlow);
	nextRowLayoutFlow ();
	layoutFlow.x += UiConfiguration::instance->marginSize;
	windowSizeSlider->flowDown (&layoutFlow);
	fullscreenToggle->flowDown (&layoutFlow);
	layoutFlow.y -= UiConfiguration::instance->marginSize;
	showClockToggle->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	layoutFlow.y += UiConfiguration::instance->marginSize;
	textSizeLabel->flowDown (&layoutFlow);
	nextRowLayoutFlow ();
	layoutFlow.x += UiConfiguration::instance->marginSize;
	textSizeSlider->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	layoutFlow.y += UiConfiguration::instance->marginSize;
	soundVolumeLabel->flowDown (&layoutFlow);
	nextRowLayoutFlow ();
	layoutFlow.x += UiConfiguration::instance->marginSize;
	soundVolumeSlider->flowDown (&layoutFlow);

	nextRowLayoutFlow ();
	layoutFlow.y += UiConfiguration::instance->marginSize;
	networkLabel->flowDown (&layoutFlow);
	nextRowLayoutFlow ();
	layoutFlow.x += UiConfiguration::instance->marginSize;
	checkUpdatesToggle->flowDown (&layoutFlow);
}

void SettingsWindow::doResize () {
	setFixedSize (true, App::instance->rootPanel->width * windowWidthScale, App::instance->rootPanel->height);
	headerImage->onLoadScale (width, width / headerImageAspectRatio);
	headerImage->setWindowSize (true, width, width / headerImageAspectRatio);
	headerImage->loadImageFile (StdString::createSprintf ("bg/settings/%s/000.png", App::instance->imagePrefix.c_str ()));
	headerImage->reload ();
	Panel::doResize ();
}

StdString SettingsWindow::windowSizeSliderValueName (double sliderValue) {
	int mode, textid;

	mode = -1;
	textid = -1;
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		mode = 0;
		textid = UiTextId::Smallest;
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		mode = 1;
		textid = UiTextId::Small;
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		mode = 2;
		textid = UiTextId::Medium;
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		mode = 3;
		textid = UiTextId::Large;
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		mode = 4;
		textid = UiTextId::Largest;
	}
	if (mode < 0) {
		return (StdString ());
	}
	return (StdString::createSprintf ("%s - %ix%i", UiText::instance->getText (textid).capitalized ().c_str (), RenderResource::instance->displayModes[mode].width, RenderResource::instance->displayModes[mode].height));
}

StdString SettingsWindow::textSizeSliderValueName (double sliderValue) {
	if (FLOAT_EQUALS (sliderValue, 0.0f)) {
		return (UiText::instance->getText (UiTextId::Smallest).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.25f)) {
		return (UiText::instance->getText (UiTextId::Small).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.5f)) {
		return (UiText::instance->getText (UiTextId::Medium).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 0.75f)) {
		return (UiText::instance->getText (UiTextId::Large).capitalized ());
	}
	if (FLOAT_EQUALS (sliderValue, 1.0f)) {
		return (UiText::instance->getText (UiTextId::Largest).capitalized ());
	}
	return (StdString ());
}

void SettingsWindow::closeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((SettingsWindow *) itPtr)->isDestroyed = true;
}

void SettingsWindow::windowSizeSliderChanged (void *itPtr, Widget *widgetPtr) {
	SliderWindow *slider = (SliderWindow *) widgetPtr;
	HashMap *prefs;
	int mode;

	mode = -1;
	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		mode = 0;
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		mode = 1;
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		mode = 2;
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		mode = 3;
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		mode = 4;
	}
	if (mode >= 0) {
		App::instance->setDisplayMode (mode);
		prefs = App::instance->lockPrefs ();
		prefs->insert (UiStack::windowSizeSettingKey, mode);
		App::instance->unlockPrefs ();
	}
}

void SettingsWindow::fullscreenToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	SettingsWindow *it = (SettingsWindow *) itPtr;
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;

	it->windowSizeSlider->setDisabled (toggle->isChecked);
	if (toggle->isChecked) {
		App::instance->setDisplayMode (5);
	}
	else {
		it->windowSizeSlider->eventCallback (it->windowSizeSlider->valueChangeCallback);
	}
}

void SettingsWindow::showClockToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;
	HashMap *prefs;
	bool enable;

	enable = toggle->isChecked;
	UiStack::instance->mainToolbarWindow->setClockEnabled (enable);
	UiStack::instance->mainToolbar->reflow ();
	prefs = App::instance->lockPrefs ();
	prefs->insert (UiStack::showClockKey, enable, false);
	App::instance->unlockPrefs ();
}

void SettingsWindow::textSizeSliderChanged (void *itPtr, Widget *widgetPtr) {
	SliderWindow *slider = (SliderWindow *) widgetPtr;

	if (FLOAT_EQUALS (slider->value, 0.0f)) {
		App::instance->setFontScale (0);
	}
	if (FLOAT_EQUALS (slider->value, 0.25f)) {
		App::instance->setFontScale (1);
	}
	if (FLOAT_EQUALS (slider->value, 0.5f)) {
		App::instance->setFontScale (2);
	}
	if (FLOAT_EQUALS (slider->value, 0.75f)) {
		App::instance->setFontScale (3);
	}
	if (FLOAT_EQUALS (slider->value, 1.0f)) {
		App::instance->setFontScale (4);
	}
}

void SettingsWindow::soundVolumeSliderChanged (void *itPtr, Widget *widgetPtr) {
	SliderWindow *slider = (SliderWindow *) widgetPtr;
	HashMap *prefs;

	SoundMixer::instance->masterMixVolume = (int) (slider->value * (double) SoundMixer::maxMixVolume);
	prefs = App::instance->lockPrefs ();
	prefs->insert (App::soundVolumeKey, SoundMixer::instance->masterMixVolume, SoundMixer::maxMixVolume);
	App::instance->unlockPrefs ();
}

void SettingsWindow::checkUpdatesToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	ToggleWindow *toggle = (ToggleWindow *) widgetPtr;
	HashMap *prefs;

	App::instance->isStartUpdateEnabled = toggle->isChecked;
	prefs = App::instance->lockPrefs ();
	prefs->insert (PlayerUi::startUpdateKey, App::instance->isStartUpdateEnabled, true);
	App::instance->unlockPrefs ();
}
