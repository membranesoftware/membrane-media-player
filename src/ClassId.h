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
// Index values that reference class types
#ifndef CLASS_ID_H
#define CLASS_ID_H

class ClassId {
public:
	static constexpr const int ActionWindow = 0;
	static constexpr const int App = 1;
	static constexpr const int AppCardWindow = 2;
	static constexpr const int AppNews = 3;
	static constexpr const int AppUrl = 4;
	static constexpr const int AppUtil = 5;
	static constexpr const int Buffer = 6;
	static constexpr const int ButtonGradientEndSprite = 7;
	static constexpr const int ButtonGradientMiddleSprite = 8;
	static constexpr const int Button = 9;
	static constexpr const int CaptureWriter = 10;
	static constexpr const int CardLabelWindow = 11;
	static constexpr const int CardView = 12;
	static constexpr const int Chip = 13;
	static constexpr const int Color = 14;
	static constexpr const int ComboBox = 15;
	static constexpr const int ConsoleWindow = 16;
	static constexpr const int Database = 17;
	static constexpr const int DoubleList = 18;
	static constexpr const int FloatList = 19;
	static constexpr const int Font = 20;
	static constexpr const int FsBrowserWindowItemLabel = 21;
	static constexpr const int FsBrowserWindow = 22;
	static constexpr const int HashMap = 23;
	static constexpr const int HelpActionWindow = 24;
	static constexpr const int HelpWindow = 25;
	static constexpr const int HyperlinkWindow = 26;
	static constexpr const int IconLabelWindow = 27;
	static constexpr const int Image = 28;
	static constexpr const int ImageWindow = 29;
	static constexpr const int Input = 30;
	static constexpr const int Int64List = 31;
	static constexpr const int IntList = 32;
	static constexpr const int Ipv4Address = 33;
	static constexpr const int Json = 34;
	static constexpr const int JsonList = 35;
	static constexpr const int Label = 36;
	static constexpr const int LabelWindow = 37;
	static constexpr const int ListViewItemLabel = 38;
	static constexpr const int ListView = 39;
	static constexpr const int Log = 40;
	static constexpr const int LuaAwaitloadimagesFunction = 41;
	static constexpr const int LuaAwaitvalueFunction = 42;
	static constexpr const int LuaCapturescreenFunction = 43;
	static constexpr const int LuaCapturescreenpathFunction = 44;
	static constexpr const int LuaClickFunction = 45;
	static constexpr const int LuaDofileFunction = 46;
	static constexpr const int LuaFunction = 47;
	static constexpr const int LuaFunctionList = 48;
	static constexpr const int LuaGetenvFunction = 49;
	static constexpr const int LuaHelpFunction = 50;
	static constexpr const int LuaKeypressFunction = 51;
	static constexpr const int LuaMenuselectFunction = 52;
	static constexpr const int LuaMouseleftclickFunction = 53;
	static constexpr const int LuaMouseoverFunction = 54;
	static constexpr const int LuaMousepointerFunction = 55;
	static constexpr const int LuaMousewarpFunction = 56;
	static constexpr const int LuaOpenFunction = 57;
	static constexpr const int LuaPrintcontrolsFunction = 58;
	static constexpr const int LuaPrintFunction = 59;
	static constexpr const int LuaQuitFunction = 60;
	static constexpr const int LuaScript = 61;
	static constexpr const int LuaSetvalueFunction = 62;
	static constexpr const int LuaSleepFunction = 63;
	static constexpr const int LuaStartstreamFunction = 64;
	static constexpr const int LuaStopstreamFunction = 65;
	static constexpr const int LuaStreampathFunction = 66;
	static constexpr const int LuaTargetFunction = 67;
	static constexpr const int LuaTimeoutFunction = 68;
	static constexpr const int LuaUntargetFunction = 69;
	static constexpr const int MainToolbarWindow = 70;
	static constexpr const int MathUtil = 71;
	static constexpr const int MediaControl = 72;
	static constexpr const int MediaControlCleanWindow = 73;
	static constexpr const int MediaControlConfigureWindow = 74;
	static constexpr const int MediaControlMainWindow = 75;
	static constexpr const int MediaControlPrimeWindow = 76;
	static constexpr const int MediaControlScanWindow = 77;
	static constexpr const int MediaControlSearch = 78;
	static constexpr const int MediaControlTaskStatusWindow = 79;
	static constexpr const int MediaControlWindow = 80;
	static constexpr const int MediaItem = 81;
	static constexpr const int MediaItemDetailWindow = 82;
	static constexpr const int MediaItemImageWindow = 83;
	static constexpr const int MediaItemTagWindow = 84;
	static constexpr const int MediaItemUi = 85;
	static constexpr const int MediaItemWindow = 86;
	static constexpr const int MediaPlaylist = 87;
	static constexpr const int MediaPlaylistItem = 88;
	static constexpr const int MediaPlaylistUi = 89;
	static constexpr const int MediaPlaylistViewWindowItem = 90;
	static constexpr const int MediaPlaylistViewWindowItemLabel = 91;
	static constexpr const int MediaPlaylistViewWindow = 92;
	static constexpr const int MediaPlaylistWindow = 93;
	static constexpr const int MediaReader = 94;
	static constexpr const int MediaSearch = 95;
	static constexpr const int MediaThumbnailWindow = 96;
	static constexpr const int MediaUtil = 97;
	static constexpr const int MediaWriter = 98;
	static constexpr const int Menu = 99;
	static constexpr const int Network = 100;
	static constexpr const int OsUtil = 101;
	static constexpr const int PanelLayoutFlow = 102;
	static constexpr const int Panel = 103;
	static constexpr const int Particle = 104;
	static constexpr const int PlayerControl = 105;
	static constexpr const int PlayerTimelineWindow = 106;
	static constexpr const int PlayerUi = 107;
	static constexpr const int PlayerWindow = 108;
	static constexpr const int PlayMarker = 109;
	static constexpr const int Position = 110;
	static constexpr const int Prng = 111;
	static constexpr const int ProgressBar = 112;
	static constexpr const int ProgressRing = 113;
	static constexpr const int ProgressRingSprite = 114;
	static constexpr const int RecordStore = 115;
	static constexpr const int RenderResource = 116;
	static constexpr const int Resource = 117;
	static constexpr const int RoundedCornerSprite = 118;
	static constexpr const int ScrollBar = 119;
	static constexpr const int ScrollView = 120;
	static constexpr const int ScrollViewWindow = 121;
	static constexpr const int SdlUtil = 122;
	static constexpr const int SettingsWindow = 123;
	static constexpr const int Shader = 124;
	static constexpr const int SharedBuffer = 125;
	static constexpr const int Slider = 126;
	static constexpr const int SliderThumbSprite = 127;
	static constexpr const int SliderWindow = 128;
	static constexpr const int SnackbarWindow = 129;
	static constexpr const int SoundId = 130;
	static constexpr const int SoundMixer = 131;
	static constexpr const int SoundSample = 132;
	static constexpr const int Sprite = 133;
	static constexpr const int SpriteGroup = 134;
	static constexpr const int SpriteId = 135;
	static constexpr const int StatsWindow = 136;
	static constexpr const int StdString = 137;
	static constexpr const int StringList = 138;
	static constexpr const int SubtitleReader = 139;
	static constexpr const int SystemInterface = 140;
	static constexpr const int TagActionWindow = 141;
	static constexpr const int TaskGroup = 142;
	static constexpr const int TextArea = 143;
	static constexpr const int TextCardWindow = 144;
	static constexpr const int TextField = 145;
	static constexpr const int TextFieldWindow = 146;
	static constexpr const int TextFlow = 147;
	static constexpr const int Toggle = 148;
	static constexpr const int ToggleWindow = 149;
	static constexpr const int Toolbar = 150;
	static constexpr const int TooltipWindow = 151;
	static constexpr const int Ui = 152;
	static constexpr const int UiConfiguration = 153;
	static constexpr const int UiLog = 154;
	static constexpr const int UiLogWindow = 155;
	static constexpr const int UiStack = 156;
	static constexpr const int UiText = 157;
	static constexpr const int UiTextId = 158;
	static constexpr const int Video = 159;
	static constexpr const int WaveformShader = 160;
	static constexpr const int Widget = 161;

	static const char *classIdName (int id);
};
#endif
