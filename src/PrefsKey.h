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
// String constants for use as keys in the application prefs map
#ifndef PREFS_KEY_H
#define PREFS_KEY_H
class PrefsKey {
public:
	static constexpr const char *prefsVersion = "b1";
	static constexpr const char *networkThreads = "b2";
	static constexpr const char *displayMode = "b3";
	static constexpr const char *fontScale = "b4";
	static constexpr const char *allowUnverifiedHttps = "b5";
	static constexpr const char *soundVolume = "b6";
	static constexpr const char *fsBrowserPath = "b7";
	static constexpr const char *language = "b8";
	static constexpr const char *agentId = "b9";
	static constexpr const char *mediaFilescanPath = "b10";
	static constexpr const char *mediaDataPath = "b11";
	static constexpr const char *mediaThumbnailCount = "b12";
	static constexpr const char *mediaScan = "b13";
	static constexpr const char *savePlayHistory = "b14";
	static constexpr const char *savePlaylists = "b15";
	static constexpr const char *mediaItemUiTagWindowExpanded = "b16";
	static constexpr const char *mediaItemUiImageSize = "b17";
	static constexpr const char *mediaPlaylistUiImageSize = "b18";
	static constexpr const char *showClock = "b19";
	static constexpr const char *windowSizeSetting = "b20";
	static constexpr const char *appCardExpanded = "b21";
	static constexpr const char *playerUiImageSize = "b22";
	static constexpr const char *playerUiSortOrder = "b23";
	static constexpr const char *playerUiWindowMode = "b24";
	static constexpr const char *showPlaylists = "b25";
	static constexpr const char *soundMixVolume = "b26";
	static constexpr const char *soundMuted = "b27";
	static constexpr const char *mediaFilescanWindowExpanded = "b28";
	static constexpr const char *startUpdate = "b29";
	static constexpr const char *showAppNews = "b30";
	static constexpr const char *visualizerType = "b31";
	static constexpr const char *subtitleEnabled = "b32";
	static constexpr const char *mediaOptionWindowExpanded = "b33";
	static constexpr const char *showPlayHistory = "b34";
	static constexpr const char *skipPrimePanel = "b35";
};
#endif
