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
// Class that holds configuration values for UI elements
#ifndef UI_CONFIGURATION_H
#define UI_CONFIGURATION_H

#include "Color.h"

class Font;
class ButtonGradientMiddleSprite;
class ButtonGradientEndSprite;
class ProgressRingSprite;

class UiConfiguration {
public:
	UiConfiguration ();
	~UiConfiguration ();
	static UiConfiguration *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	enum FontType {
		CaptionFont = 0,
		BodyFont = 1,
		ButtonFont = 2,
		TitleFont = 3,
		HeadlineFont = 4,
		ConsoleFont = 5,
		NoFont = -1
	};
	static constexpr const int FontCount = 6;

	// Load resources referenced by the UiConfiguration and return a result value
	OpResult load (double fontScale = 1.0f);

	// Free resources allocated by any previous load operation
	void unload ();

	// Reset configuration values as appropriate for the current application image scale
	void resetScale ();

	// Free any loaded font resources and replace them with new ones at the specified scale
	int reloadFonts (double fontScale);

	double paddingSize;
	double marginSize;
	int shortColorTranslateDuration;
	int longColorTranslateDuration;
	int shortColorAnimateDuration;
	int longColorAnimateDuration;
	int mouseHoverThreshold;
	int longPressThreshold;
	int idleUpdateThreshold;
	int blinkDuration;
	int textCrawlRepeatDuration;
	int textCrawlAdvanceDuration;
	int backgroundCrossFadeDuration;
	int activityIconLingerDuration;
	int shortScrollPositionTranslateDuration;
	int longScrollPositionTranslateDuration;
	int cardViewRepositionTranslateDuration;
	StdString fontNames[UiConfiguration::FontCount];
	int fontBaseSizes[UiConfiguration::FontCount];
	int fontSizes[UiConfiguration::FontCount];
	Font *fonts[UiConfiguration::FontCount];
	Color lightPrimaryColor;
	Color mediumPrimaryColor;
	Color darkPrimaryColor;
	Color lightSecondaryColor;
	Color mediumSecondaryColor;
	Color darkSecondaryColor;
	Color lightBackgroundColor;
	Color mediumBackgroundColor;
	Color darkBackgroundColor;
	Color lightInverseBackgroundColor;
	Color mediumInverseBackgroundColor;
	Color darkInverseBackgroundColor;
	Color primaryTextColor;
	Color lightPrimaryTextColor;
	Color inverseTextColor;
	Color darkInverseTextColor;
	Color buttonTextColor;
	Color linkTextColor;
	Color errorTextColor;
	Color warningTextColor;
	Color tooltipTextColor;
	Color statusOkTextColor;
	Color dropShadowColor;
	double dropShadowWidth;
	Color dividerColor;
	int cornerRadius;
	double buttonUnfocusedShadeAlpha;
	double buttonFocusedShadeAlpha;
	double buttonPressedShadeAlpha;
	double buttonDisabledShadeAlpha;
	double buttonInverseFocusedShadeAlpha;
	double buttonInversePressedShadeAlpha;
	double buttonUnfocusedIconAlpha;
	double buttonFocusedIconAlpha;
	double buttonDisabledIconAlpha;
	double buttonTransparentBackgroundAlpha;
	double buttonUnfocusedDropShadowWidth;
	double buttonFocusedDropShadowWidth;
	Color mouseoverBgColor;
	double mouseoverBgAlpha;
	double scrimBackgroundAlpha;
	double overlayWindowAlpha;
	double waitingShadeAlpha;
	double imageWindowOffscreenAreaMultiplier;
	double progressBarHeight;
	double progressRingSize;
	double mouseWheelScrollSpeed; // portion of total window height per wheel scroll event, from 0.0f to 1.0f
	double textLineHeightMargin;
	double textUnderlineMargin;
	double menuDividerLineWidth;
	double headlineDividerLineWidth;
	double selectionBorderWidth;
	double selectionBorderAlpha;
	double sliderThumbSize;
	double sliderTrackWidth;
	double sliderTrackHeight;
	int textAreaSmallLineCount;
	int textAreaMediumLineCount;
	int textAreaLargeLineCount;
	int textFieldShortLineLength;
	int textFieldMediumLineLength;
	int textFieldLongLineLength;
	double textFieldInsertCursorWidth;
	double textFieldOvertypeCursorScale; // portion of text font max glyph width, from 0.0f to 1.0f
	int comboBoxLineLength;
	int comboBoxExpandViewItems;
	double timelineMarkerWidth;
	double rightNavWidthScale; // portion of total window width, from 0.0f to 1.0f
	int snackbarTimeout;
	int snackbarScrollDuration;
	int recordSyncDelayDuration;
	int rightDialogTranslateDuration;
	double smallThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	double mediumThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	double largeThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	double popupThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	bool isLoaded;
	ButtonGradientMiddleSprite *buttonGradientMiddleSprite;
	ButtonGradientEndSprite *buttonGradientEndSprite;
	ProgressRingSprite *progressRingSprite;
};
#endif
