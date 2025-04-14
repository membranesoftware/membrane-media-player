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
	static constexpr const int MediaControlCleanFilescanTask = 73;
	static constexpr const int MediaControlConfigureFilescanTask = 74;
	static constexpr const int MediaControlConfigureMainTask = 75;
	static constexpr const int MediaControlDatabaseExecTask = 76;
	static constexpr const int MediaControlFilescanTask = 77;
	static constexpr const int MediaControlReadyTask = 78;
	static constexpr const int MediaControlSearch = 79;
	static constexpr const int MediaControlTask = 80;
	static constexpr const int MediaControlTaskWindow = 81;
	static constexpr const int MediaFilescanCleanWindow = 82;
	static constexpr const int MediaFilescanConfigureWindow = 83;
	static constexpr const int MediaFilescanRunWindow = 84;
	static constexpr const int MediaFilescanStatusWindow = 85;
	static constexpr const int MediaFilescanWindow = 86;
	static constexpr const int MediaItem = 87;
	static constexpr const int MediaItemDetailWindow = 88;
	static constexpr const int MediaItemImageWindow = 89;
	static constexpr const int MediaItemTagWindow = 90;
	static constexpr const int MediaItemUi = 91;
	static constexpr const int MediaItemWindow = 92;
	static constexpr const int MediaOptionWindow = 93;
	static constexpr const int MediaPlaylist = 94;
	static constexpr const int MediaPlaylistItem = 95;
	static constexpr const int MediaPlaylistUi = 96;
	static constexpr const int MediaPlaylistViewWindowItem = 97;
	static constexpr const int MediaPlaylistViewWindowItemLabel = 98;
	static constexpr const int MediaPlaylistViewWindow = 99;
	static constexpr const int MediaPlaylistWindow = 100;
	static constexpr const int MediaReader = 101;
	static constexpr const int MediaSearch = 102;
	static constexpr const int MediaSearchGroup = 103;
	static constexpr const int MediaThumbnailWindow = 104;
	static constexpr const int MediaUtil = 105;
	static constexpr const int MediaWriter = 106;
	static constexpr const int Menu = 107;
	static constexpr const int Network = 108;
	static constexpr const int OsUtil = 109;
	static constexpr const int PanelLayoutFlow = 110;
	static constexpr const int Panel = 111;
	static constexpr const int Particle = 112;
	static constexpr const int PlayerControl = 113;
	static constexpr const int PlayerTimelineWindow = 114;
	static constexpr const int PlayerUi = 115;
	static constexpr const int PlayerWindow = 116;
	static constexpr const int PlayFileActionWindow = 117;
	static constexpr const int PlayMarker = 118;
	static constexpr const int Position = 119;
	static constexpr const int PrefsKey = 120;
	static constexpr const int Prng = 121;
	static constexpr const int ProgressBar = 122;
	static constexpr const int ProgressRing = 123;
	static constexpr const int ProgressRingSprite = 124;
	static constexpr const int RecordStore = 125;
	static constexpr const int RenderResource = 126;
	static constexpr const int Resource = 127;
	static constexpr const int RoundedCornerSprite = 128;
	static constexpr const int ScrollBar = 129;
	static constexpr const int ScrollView = 130;
	static constexpr const int ScrollViewWindow = 131;
	static constexpr const int SdlUtil = 132;
	static constexpr const int SettingsWindow = 133;
	static constexpr const int Shader = 134;
	static constexpr const int SharedBuffer = 135;
	static constexpr const int Slider = 136;
	static constexpr const int SliderThumbSprite = 137;
	static constexpr const int SliderWindow = 138;
	static constexpr const int SnackbarWindow = 139;
	static constexpr const int SoundId = 140;
	static constexpr const int SoundMixer = 141;
	static constexpr const int SoundSample = 142;
	static constexpr const int Sprite = 143;
	static constexpr const int SpriteGroup = 144;
	static constexpr const int SpriteId = 145;
	static constexpr const int StatsWindow = 146;
	static constexpr const int StdString = 147;
	static constexpr const int StringList = 148;
	static constexpr const int SubtitleReader = 149;
	static constexpr const int SystemInterface = 150;
	static constexpr const int TagActionWindow = 151;
	static constexpr const int TaskGroup = 152;
	static constexpr const int TextArea = 153;
	static constexpr const int TextCardWindow = 154;
	static constexpr const int TextField = 155;
	static constexpr const int TextFieldWindow = 156;
	static constexpr const int TextFlow = 157;
	static constexpr const int Toggle = 158;
	static constexpr const int ToggleWindow = 159;
	static constexpr const int Toolbar = 160;
	static constexpr const int TooltipWindow = 161;
	static constexpr const int Ui = 162;
	static constexpr const int UiConfiguration = 163;
	static constexpr const int UiLog = 164;
	static constexpr const int UiLogWindow = 165;
	static constexpr const int UiStack = 166;
	static constexpr const int UiText = 167;
	static constexpr const int UiTextId = 168;
	static constexpr const int VideoCycleWindow = 169;
	static constexpr const int Video = 170;
	static constexpr const int WaveformShader = 171;
	static constexpr const int Widget = 172;

	static const char *classIdName (int id);
};
#endif
