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
#include "Resource.h"
#include "Font.h"
#include "ButtonGradientMiddleSprite.h"
#include "ButtonGradientEndSprite.h"
#include "ProgressRingSprite.h"
#include "UiConfiguration.h"

UiConfiguration *UiConfiguration::instance = NULL;

constexpr const double buttonGradientHeightMultiplier = 2.25f;
constexpr const double buttonGradientScaleIncrement = 4.0f;

UiConfiguration::UiConfiguration ()
: paddingSize (12.0f)
, marginSize (12.0f)
, shortColorTranslateDuration (120)
, longColorTranslateDuration (768)
, shortColorAnimateDuration (1024)
, longColorAnimateDuration (2048)
, mouseHoverThreshold (1000)
, longPressThreshold (1000)
, idleUpdateThreshold (1200)
, blinkDuration (486)
, textCrawlRepeatDuration (7000)
, textCrawlAdvanceDuration (100)
, backgroundCrossFadeDuration (140)
, activityIconLingerDuration (1200)
, shortScrollPositionTranslateDuration (120)
, longScrollPositionTranslateDuration (500)
, cardViewRepositionTranslateDuration (150)
, lightPrimaryColor (0.317f, 0.290f, 0.674f)
, mediumPrimaryColor (0.094f, 0.133f, 0.486f)
, darkPrimaryColor (0.0f, 0.0f, 0.309f)
, lightSecondaryColor (1.0f, 1.0f, 0.658f)
, mediumSecondaryColor (1.0f, 0.945f, 0.462f)
, darkSecondaryColor (0.792f, 0.749f, 0.270f)
, lightBackgroundColor (1.0f, 1.0f, 1.0f)
, mediumBackgroundColor (0.941f, 0.941f, 0.941f)
, darkBackgroundColor (0.878f, 0.878f, 0.878f)
, lightInverseBackgroundColor (0.133f, 0.133f, 0.133f)
, mediumInverseBackgroundColor (0.066f, 0.066f, 0.066f)
, darkInverseBackgroundColor (0.0f, 0.0f, 0.0f)
, primaryTextColor (0.0f, 0.0f, 0.0f)
, lightPrimaryTextColor (0.376f, 0.376f, 0.376f)
, inverseTextColor (1.0f, 1.0f, 1.0f)
, darkInverseTextColor (0.596f, 0.596f, 0.596f)
, buttonTextColor (0.094f, 0.133f, 0.486f)
, linkTextColor (0.094f, 0.133f, 0.486f)
, errorTextColor (0.690f, 0.0f, 0.125f)
, warningTextColor (0.709f, 0.462f, 0.058f)
, tooltipTextColor (1.0f, 0.945f, 0.462f)
, statusOkTextColor (0.062f, 0.552f, 0.062f)
, dropShadowColor (0.0f, 0.0f, 0.0f, 0.78f)
, dropShadowWidth (2.0f)
, dividerColor (0.0f, 0.0f, 0.0f, 0.14f)
, cornerRadius (6)
, buttonUnfocusedShadeAlpha (0.24f)
, buttonFocusedShadeAlpha (0.06f)
, buttonPressedShadeAlpha (0.12f)
, buttonDisabledShadeAlpha (0.58f)
, buttonInverseFocusedShadeAlpha (0.42f)
, buttonInversePressedShadeAlpha (0.3f)
, buttonUnfocusedIconAlpha (0.74f)
, buttonFocusedIconAlpha (0.99f)
, buttonDisabledIconAlpha (0.58f)
, buttonTransparentBackgroundAlpha (0.25f)
, buttonUnfocusedDropShadowWidth (1.0f)
, buttonFocusedDropShadowWidth (2.0f)
, mouseoverBgColor (0.082f, 0.623f, 0.054f)
, mouseoverBgAlpha (0.53f)
, scrimBackgroundAlpha (0.77f)
, overlayWindowAlpha (0.72f)
, waitingShadeAlpha (0.81f)
, imageWindowOffscreenAreaMultiplier (1.5f)
, progressBarHeight (8.0f)
, progressRingSize (16.0f)
, mouseWheelScrollSpeed (0.12f)
, textLineHeightMargin (2.0f)
, textUnderlineMargin (2.0f)
, menuDividerLineWidth (2.0f)
, headlineDividerLineWidth (2.0f)
, selectionBorderWidth (5.0f)
, selectionBorderAlpha (0.72f)
, sliderThumbSize (16.0f)
, sliderTrackWidth (200.0f)
, sliderTrackHeight (6.0f)
, textAreaSmallLineCount (8)
, textAreaMediumLineCount (16)
, textAreaLargeLineCount (30)
, textFieldShortLineLength (16)
, textFieldMediumLineLength (32)
, textFieldLongLineLength (60)
, textFieldInsertCursorWidth (4.0f)
, textFieldOvertypeCursorScale (0.9f)
, comboBoxLineLength (30)
, comboBoxExpandViewItems (7)
, timelineMarkerWidth (16.0f)
, rightNavWidthScale (0.275f)
, snackbarTimeout (16000)
, snackbarScrollDuration (280)
, recordSyncDelayDuration (1200)
, rightDialogTranslateDuration (75)
, smallThumbnailImageScale (0.123f)
, mediumThumbnailImageScale (0.240f)
, largeThumbnailImageScale (0.450f)
, popupThumbnailImageScale (0.12f)
, isLoaded (false)
, buttonGradientMiddleSprite (NULL)
, buttonGradientEndSprite (NULL)
, progressRingSprite (NULL)
{
	memset (fonts, 0, sizeof (fonts));
	memset (fontSizes, 0, sizeof (fontSizes));

	fontNames[UiConfiguration::CaptionFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::CaptionFont] = 10;

	fontNames[UiConfiguration::BodyFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::BodyFont] = 12;

	fontNames[UiConfiguration::ButtonFont].assign ("font/Roboto-Medium.ttf");
	fontBaseSizes[UiConfiguration::ButtonFont] = 12;

	fontNames[UiConfiguration::TitleFont].assign ("font/Roboto-Medium.ttf");
	fontBaseSizes[UiConfiguration::TitleFont] = 14;

	fontNames[UiConfiguration::HeadlineFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::HeadlineFont] = 16;

	fontNames[UiConfiguration::ConsoleFont].assign ("font/IBMPlexMono-Regular.ttf");
	fontBaseSizes[UiConfiguration::ConsoleFont] = 10;

	fontNames[UiConfiguration::SmallSubtitleFont].assign ("font/Domine-SemiBold.ttf");
	fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 10;

	fontNames[UiConfiguration::MediumSubtitleFont].assign ("font/Domine-SemiBold.ttf");
	fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 14;

	fontNames[UiConfiguration::LargeSubtitleFont].assign ("font/Domine-Bold.ttf");
	fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 24;
}
UiConfiguration::~UiConfiguration () {
	unload ();
}

void UiConfiguration::createInstance () {
	if (! UiConfiguration::instance) {
		UiConfiguration::instance = new UiConfiguration ();
	}
}
void UiConfiguration::freeInstance () {
	if (UiConfiguration::instance) {
		delete (UiConfiguration::instance);
		UiConfiguration::instance = NULL;
	}
}

OpResult UiConfiguration::load (double fontScale) {
	Font *font;
	int i, sz;
	OpResult result;
	double buttonh;

	if (fontScale <= 0.0f) {
		return (OpResult::InvalidParamError);
	}
	if (isLoaded) {
		return (OpResult::Success);
	}

	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		sz = (int) (fontScale * (double) fontBaseSizes[i]);
		if (sz < 1) {
			sz = 1;
		}
		font = Resource::instance->loadFont (fontNames[i], sz);
		if (! font) {
			return (OpResult::FreetypeOperationFailedError);
		}
		fonts[i] = font;
		fontSizes[i] = sz;
	}

	buttonh = fontSizes[UiConfiguration::ButtonFont] + (paddingSize * 2.0f);
	if (buttonGradientMiddleSprite) {
		delete (buttonGradientMiddleSprite);
	}
	buttonGradientMiddleSprite = new ButtonGradientMiddleSprite ();
	result = buttonGradientMiddleSprite->load (buttonh, buttonh * buttonGradientHeightMultiplier, buttonGradientScaleIncrement);
	if (result != OpResult::Success) {
		return (result);
	}

	if (buttonGradientEndSprite) {
		delete (buttonGradientEndSprite);
	}
	buttonGradientEndSprite = new ButtonGradientEndSprite ();
	result = buttonGradientEndSprite->load (buttonh, buttonh * buttonGradientHeightMultiplier, buttonGradientScaleIncrement);
	if (result != OpResult::Success) {
		return (result);
	}

	if (progressRingSprite) {
		delete (progressRingSprite);
	}
	progressRingSprite = new ProgressRingSprite ();
	result = progressRingSprite->load (progressRingSize);
	if (result != OpResult::Success) {
		return (result);
	}

	isLoaded = true;
	return (OpResult::Success);
}

void UiConfiguration::unload () {
	int i;

	isLoaded = false;
	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		if (fonts[i]) {
			fonts[i] = NULL;
			Resource::instance->unloadFont (fontNames[i], fontSizes[i]);
		}
	}
	if (buttonGradientMiddleSprite) {
		delete (buttonGradientMiddleSprite);
		buttonGradientMiddleSprite = NULL;
	}
	if (buttonGradientEndSprite) {
		delete (buttonGradientEndSprite);
		buttonGradientEndSprite = NULL;
	}
	if (progressRingSprite) {
		delete (progressRingSprite);
		progressRingSprite = NULL;
	}
}

int UiConfiguration::reloadFonts (double fontScale) {
	Font *font;
	OpResult result;
	int i, sz, unloadsizes[UiConfiguration::FontCount];
	double buttonh;

	if (fontScale <= 0.0f) {
		return (OpResult::InvalidParamError);
	}
	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		sz = (int) (fontScale * (double) fontBaseSizes[i]);
		if (sz < 1) {
			sz = 1;
		}
		font = Resource::instance->loadFont (fontNames[i], sz);
		if (! font) {
			return (OpResult::FreetypeOperationFailedError);
		}
		if (fonts[i]) {
			unloadsizes[i] = fontSizes[i];
		}
		else {
			unloadsizes[i] = 0;
		}
		fonts[i] = font;
		fontSizes[i] = sz;
	}
	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		if (unloadsizes[i] > 0) {
			Resource::instance->unloadFont (fontNames[i], unloadsizes[i]);
		}
	}

	buttonh = fontSizes[UiConfiguration::ButtonFont] + (paddingSize * 2.0f);
	if (buttonGradientMiddleSprite) {
		delete (buttonGradientMiddleSprite);
	}
	buttonGradientMiddleSprite = new ButtonGradientMiddleSprite ();
	result = buttonGradientMiddleSprite->load (buttonh, buttonh * buttonGradientHeightMultiplier, buttonGradientScaleIncrement);
	if (result != OpResult::Success) {
		return (result);
	}

	if (buttonGradientEndSprite) {
		delete (buttonGradientEndSprite);
	}
	buttonGradientEndSprite = new ButtonGradientEndSprite ();
	result = buttonGradientEndSprite->load (buttonh, buttonh * buttonGradientHeightMultiplier, buttonGradientScaleIncrement);
	if (result != OpResult::Success) {
		return (result);
	}
	return (OpResult::Success);
}

void UiConfiguration::resetScale () {
	if (App::instance->windowHeight <= 432) {
		paddingSize = 6.0f;
		marginSize = 6.0f;
		cornerRadius = 3;
		progressBarHeight = 8.0f;
		progressRingSize = 16.0f;
		sliderTrackWidth = 100.0f;
		sliderTrackHeight = 4.0f;
		sliderThumbSize = 10.0f;
		timelineMarkerWidth = 12.0f;
		dropShadowWidth = 2.0f;
		comboBoxExpandViewItems = 6;
		fontBaseSizes[UiConfiguration::CaptionFont] = 8;
		fontBaseSizes[UiConfiguration::BodyFont] = 10;
		fontBaseSizes[UiConfiguration::ButtonFont] = 10;
		fontBaseSizes[UiConfiguration::TitleFont] = 12;
		fontBaseSizes[UiConfiguration::HeadlineFont] = 14;
		fontBaseSizes[UiConfiguration::ConsoleFont] = 8;
		fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 7;
		fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 10;
		fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 22;
	}
	else if (App::instance->windowHeight <= 576) {
		paddingSize = 12.0f;
		marginSize = 12.0f;
		cornerRadius = 6;
		progressBarHeight = 8.0f;
		progressRingSize = 20.0f;
		sliderTrackWidth = 130.0f;
		sliderTrackHeight = 4.0f;
		sliderThumbSize = 12.0f;
		timelineMarkerWidth = 16.0f;
		dropShadowWidth = 2.0f;
		comboBoxExpandViewItems = 6;
		fontBaseSizes[UiConfiguration::CaptionFont] = 8;
		fontBaseSizes[UiConfiguration::BodyFont] = 10;
		fontBaseSizes[UiConfiguration::ButtonFont] = 10;
		fontBaseSizes[UiConfiguration::TitleFont] = 12;
		fontBaseSizes[UiConfiguration::HeadlineFont] = 14;
		fontBaseSizes[UiConfiguration::ConsoleFont] = 8;
		fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 8;
		fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 10;
		fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 22;
	}
	else if (App::instance->windowHeight <= 720) {
		paddingSize = 12.0f;
		marginSize = 12.0f;
		cornerRadius = 8;
		progressBarHeight = 10.0f;
		progressRingSize = 24.0f;
		sliderTrackWidth = 180.0f;
		sliderTrackHeight = 5.0f;
		sliderThumbSize = 16.0f;
		timelineMarkerWidth = 16.0f;
		dropShadowWidth = 3.0f;
		comboBoxExpandViewItems = 7;
		fontBaseSizes[UiConfiguration::CaptionFont] = 10;
		fontBaseSizes[UiConfiguration::BodyFont] = 12;
		fontBaseSizes[UiConfiguration::ButtonFont] = 12;
		fontBaseSizes[UiConfiguration::TitleFont] = 14;
		fontBaseSizes[UiConfiguration::HeadlineFont] = 16;
		fontBaseSizes[UiConfiguration::ConsoleFont] = 10;
		fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 10;
		fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 14;
		fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 24;
	}
	else if (App::instance->windowHeight <= 900) {
		paddingSize = 16.0f;
		marginSize = 16.0f;
		cornerRadius = 8;
		progressBarHeight = 10.0f;
		progressRingSize = 28.0f;
		sliderTrackWidth = 240.0f;
		sliderTrackHeight = 8.0f;
		sliderThumbSize = 20.0f;
		timelineMarkerWidth = 20.0f;
		dropShadowWidth = 3.0f;
		comboBoxExpandViewItems = 8;
		fontBaseSizes[UiConfiguration::CaptionFont] = 10;
		fontBaseSizes[UiConfiguration::BodyFont] = 12;
		fontBaseSizes[UiConfiguration::ButtonFont] = 12;
		fontBaseSizes[UiConfiguration::TitleFont] = 16;
		fontBaseSizes[UiConfiguration::HeadlineFont] = 20;
		fontBaseSizes[UiConfiguration::ConsoleFont] = 10;
		fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 10;
		fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 16;
		fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 28;
	}
	else {
		paddingSize = 20.0f;
		marginSize = 20.0f;
		cornerRadius = 10;
		progressBarHeight = 12.0f;
		progressRingSize = 32.0f;
		sliderTrackWidth = 300.0f;
		sliderTrackHeight = 10.0f;
		sliderThumbSize = 24.0f;
		timelineMarkerWidth = 20.0f;
		dropShadowWidth = 4.0f;
		comboBoxExpandViewItems = 8;
		fontBaseSizes[UiConfiguration::CaptionFont] = 12;
		fontBaseSizes[UiConfiguration::BodyFont] = 14;
		fontBaseSizes[UiConfiguration::ButtonFont] = 14;
		fontBaseSizes[UiConfiguration::TitleFont] = 18;
		fontBaseSizes[UiConfiguration::HeadlineFont] = 22;
		fontBaseSizes[UiConfiguration::ConsoleFont] = 12;
		fontBaseSizes[UiConfiguration::SmallSubtitleFont] = 12;
		fontBaseSizes[UiConfiguration::MediumSubtitleFont] = 18;
		fontBaseSizes[UiConfiguration::LargeSubtitleFont] = 32;
	}
}
