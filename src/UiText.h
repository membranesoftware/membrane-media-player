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
// Class that holds strings for use in UI text
#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "UiTextId.h"

class UiText {
public:
	UiText ();
	~UiText ();
	static UiText *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static const StdString defaultLanguage;

	// Load text strings for the specified language and return a Result value
	OpResult load (const StdString &language = UiText::defaultLanguage);

	// Return the specified text string
	StdString getText (int stringIndex);

	// Return a string containing text summarizing an SDL_WINDOW flags value
	StdString getSdlWindowFlagsText (Uint32 flags);

	// Return a string containing text summarizing an SDL_RENDERER flags value
	StdString getSdlRendererFlagsText (Uint32 flags);

	// Return a string containing text suitable for displaying the specified count. If pluralStringIndex is not provided, default to use of singularStringIndex.
	StdString getCountText (int64_t amount, int singularStringIndex, int pluralStringIndex = -1);
	StdString getCountText (int amount, int singularStringIndex, int pluralStringIndex = -1);

	// Return a string containing text suitable for displaying a millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	StdString getTimestampText (int64_t timestamp = 0);

	// Return a string containing text suitable for displaying a millisecond timestamp in numeric format. A timestamp of zero or less indicates that the current timestamp should be used.
	StdString getTimestampNumber (int64_t timestamp = 0);

	// Return a string containing text suitable for displaying the date portion of a millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	StdString getDateText (int64_t timestamp = 0);

	// Return a string containing text suitable for displaying the time portion of a millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	StdString getTimeText (int64_t timestamp = 0);

	enum TimespanUnit {
		MillisecondsUnit = 0,
		SecondsUnit = 1,
		MinutesUnit = 2,
		HoursUnit = 3,
		DaysUnit = 4
	};
	// Return a string containing text representing the specified millisecond duration in timespan format
	StdString getTimespanText (int64_t duration, UiText::TimespanUnit minUnitType = UiText::MillisecondsUnit, bool hideMilliseconds = false);

	// Return a UiText::TimespanUnit value usable as a minUnitType value with the getTimespanText method, indicating the most suitable unit type for use with the specified millisecond duration (i.e. shorter durations should be shown with smaller unit types)
	UiText::TimespanUnit getMinTimespanUnit (int64_t duration);

	// Return a string containing text suitable for displaying the specified millisecond duration
	StdString getDurationText (int64_t duration);

	// Return a string containing text suitable for displaying the specified number of bytes in readable format
	StdString getByteCountText (int64_t bytes);

	// Return a string containing text suitable for displaying the specified storage numbers in readable format
	StdString getStorageAmountText (int64_t bytesFree, int64_t bytesTotal);

	// Return a string containing text suitable for displaying the name of the root filesystem path
	StdString getRootPathText ();

private:
	std::vector<StdString> textStrings;
};
#endif
