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
// Index values that reference UiText strings
#ifndef UI_TEXT_ID_H
#define UI_TEXT_ID_H

#if PLATFORM_WINDOWS
#ifdef small
#undef small
#endif
#endif

class UiTextId {
public:
	static constexpr const int ConsoleStartText1 = 1;
	static constexpr const int ConsoleStartText2 = 2;
	static constexpr const int ConsoleStartText3 = 3;
	static constexpr const int ConsoleWindowRunButtonTooltip = 4;
	static constexpr const int ConsoleWindowLuaPrompt = 5;
	static constexpr const int ConsoleWindowSelectRunFilePrompt = 6;
	static constexpr const int LuaScriptHelpHelpText = 7;
	static constexpr const int LuaScriptQuitHelpText = 8;
	static constexpr const int LuaScriptPrintHelpText = 9;
	static constexpr const int LuaScriptDofileHelpText = 10;
	static constexpr const int LuaScriptSleepHelpText = 11;
	static constexpr const int LuaScriptPrintcontrolsHelpText = 12;
	static constexpr const int LuaScriptCapturescreenHelpText = 13;
	static constexpr const int LuaScriptCapturescreenpathHelpText = 14;
	static constexpr const int LuaScriptStartstreamHelpText = 15;
	static constexpr const int LuaScriptStopstreamHelpText = 16;
	static constexpr const int LuaScriptStreampathHelpText = 17;
	static constexpr const int LuaScriptClickHelpText = 18;
	static constexpr const int LuaScriptOpenHelpText = 19;
	static constexpr const int LuaScriptTargetHelpText = 20;
	static constexpr const int LuaScriptUntargetHelpText = 21;
	static constexpr const int LuaScriptTimeoutHelpText = 22;
	static constexpr const int LuaScriptKeypressHelpText = 23;
	static constexpr const int LuaScriptMousewarpHelpText = 24;
	static constexpr const int LuaScriptMouseoverHelpText = 25;
	static constexpr const int LuaScriptMouseleftclickHelpText = 26;
	static constexpr const int LuaScriptMousepointerHelpText = 27;
	static constexpr const int LuaScriptAwaitvalueHelpText = 28;
	static constexpr const int LuaScriptSetvalueHelpText = 29;
	static constexpr const int LuaScriptMenuselectHelpText = 30;
	static constexpr const int LuaScriptGetenvHelpText = 31;
	static constexpr const int LuaScriptAwaitloadimagesHelpText = 32;
	static constexpr const int LuaSyntaxErrorText = 33;
	static constexpr const int LuaParseErrorText = 34;
	static constexpr const int LuaScriptExecutionErrorText = 35;
	static constexpr const int EnvironmentVariables = 36;
	static constexpr const int FunctionTimeout = 37;
	static constexpr const int LuaScriptRunScriptHelpText = 38;
	static constexpr const int LuaScriptRunFileHelpText = 39;
	static constexpr const int LuaScriptMissingFunctionArgumentErrorText = 40;
	static constexpr const int LuaScriptEmptyStringArgumentErrorText = 41;
	static constexpr const int LuaScriptLoadScriptErrorText = 42;
	static constexpr const int LuaScriptLoadScriptFileErrorText = 43;
	static constexpr const int LuaScriptLoadScriptSyntaxErrorText = 44;
	static constexpr const int LuaScriptLoadScriptExecutionErrorText = 45;
	static constexpr const int Month1Abbreviation = 46;
	static constexpr const int Month2Abbreviation = 47;
	static constexpr const int Month3Abbreviation = 48;
	static constexpr const int Month4Abbreviation = 49;
	static constexpr const int Month5Abbreviation = 50;
	static constexpr const int Month6Abbreviation = 51;
	static constexpr const int Month7Abbreviation = 52;
	static constexpr const int Month8Abbreviation = 53;
	static constexpr const int Month9Abbreviation = 54;
	static constexpr const int Month10Abbreviation = 55;
	static constexpr const int Month11Abbreviation = 56;
	static constexpr const int Month12Abbreviation = 57;
	static constexpr const int Ok = 58;
	static constexpr const int Yes = 59;
	static constexpr const int No = 60;
	static constexpr const int ShuttingDown = 61;
	static constexpr const int Close = 62;
	static constexpr const int Confirm = 63;
	static constexpr const int Cancel = 64;
	static constexpr const int Remove = 65;
	static constexpr const int Console = 66;
	static constexpr const int ApplicationLog = 67;
	static constexpr const int ClearLog = 68;
	static constexpr const int InternalApplicationError = 69;
	static constexpr const int NoMediaSelected = 70;
	static constexpr const int InvalidMediaFile = 71;
	static constexpr const int InvalidFileFormat = 72;
	static constexpr const int FileOpenFailed = 73;
	static constexpr const int FileOperationError = 74;
	static constexpr const int UnknownVideoFormat = 75;
	static constexpr const int UnknownAudioFormat = 76;
	static constexpr const int DataLoadError = 77;
	static constexpr const int SeekPositionNotFound = 78;
	static constexpr const int Edit = 79;
	static constexpr const int Help = 80;
	static constexpr const int Status = 81;
	static constexpr const int Exit = 82;
	static constexpr const int Select = 83;
	static constexpr const int Next = 84;
	static constexpr const int Clear = 85;
	static constexpr const int Log = 86;
	static constexpr const int Settings = 87;
	static constexpr const int About = 88;
	static constexpr const int SendFeedback = 89;
	static constexpr const int CheckForUpdates = 90;
	static constexpr const int CheckUpdatesOnStartup = 91;
	static constexpr const int CheckingForUpdates = 92;
	static constexpr const int NoUpdatesPrompt = 93;
	static constexpr const int UpdateErrorPrompt = 94;
	static constexpr const int UpdateFoundPrompt = 95;
	static constexpr const int AppWindow = 96;
	static constexpr const int FullscreenWindow = 97;
	static constexpr const int ShowClock = 98;
	static constexpr const int TextSize = 99;
	static constexpr const int SoundVolume = 100;
	static constexpr const int SoundMixLevel = 101;
	static constexpr const int Network = 102;
	static constexpr const int AppInfo = 103;
	static constexpr const int Version = 104;
	static constexpr const int BuildDateText = 105;
	static constexpr const int Media = 106;
	static constexpr const int MediaFile = 107;
	static constexpr const int MediaFiles = 108;
	static constexpr const int EditPlaylist = 109;
	static constexpr const int DeletePlaylist = 110;
	static constexpr const int AddPlaylistItems = 111;
	static constexpr const int Shuffle = 112;
	static constexpr const int StartPosition = 113;
	static constexpr const int PlayDuration = 114;
	static constexpr const int Playlist = 115;
	static constexpr const int Playlists = 116;
	static constexpr const int Smallest = 117;
	static constexpr const int Small = 118;
	static constexpr const int Medium = 119;
	static constexpr const int Large = 120;
	static constexpr const int Largest = 121;
	static constexpr const int Player = 122;
	static constexpr const int Play = 123;
	static constexpr const int Stop = 124;
	static constexpr const int Pause = 125;
	static constexpr const int Resume = 126;
	static constexpr const int Disabled = 127;
	static constexpr const int LearnMore = 128;
	static constexpr const int SearchForHelp = 129;
	static constexpr const int Create = 130;
	static constexpr const int Open = 131;
	static constexpr const int OpenHelp = 132;
	static constexpr const int Configuring = 133;
	static constexpr const int Loading = 134;
	static constexpr const int MainMenu = 135;
	static constexpr const int FileSize = 136;
	static constexpr const int Duration = 137;
	static constexpr const int Configure = 138;
	static constexpr const int Configuration = 139;
	static constexpr const int ConfigurationRequired = 140;
	static constexpr const int EnterPlaylistNamePrompt = 141;
	static constexpr const int EnterSearchKeyPrompt = 142;
	static constexpr const int Expand = 143;
	static constexpr const int Minimize = 144;
	static constexpr const int ExpandAll = 145;
	static constexpr const int MinimizeAll = 146;
	static constexpr const int MainMenuTooltip = 147;
	static constexpr const int TextFieldEnterTooltip = 148;
	static constexpr const int TextFieldPasteTooltip = 149;
	static constexpr const int TextFieldClearTooltip = 150;
	static constexpr const int TextFieldRandomizeTooltip = 151;
	static constexpr const int TextFieldVisibilityToggleTooltip = 152;
	static constexpr const int TextFieldFsBrowseTooltip = 153;
	static constexpr const int AppInfoPrimeConfigureText = 154;
	static constexpr const int AppInfoText1 = 155;
	static constexpr const int AppInfoText2 = 156;
	static constexpr const int AppInfoText3 = 157;
	static constexpr const int AppInfoText4 = 158;
	static constexpr const int AppInfoText5 = 159;
	static constexpr const int AppInfoText6 = 160;
	static constexpr const int AppInfoText7 = 161;
	static constexpr const int AppInfoText8 = 162;
	static constexpr const int AppInfoText9 = 163;
	static constexpr const int AppInfoText10 = 164;
	static constexpr const int AppInfoText11 = 165;
	static constexpr const int AppInfoText12 = 166;
	static constexpr const int AppInfoText13 = 167;
	static constexpr const int AppInfoText14 = 168;
	static constexpr const int AppInfoText15 = 169;
	static constexpr const int AppInfoText16 = 170;
	static constexpr const int MediaPlaylistItem = 171;
	static constexpr const int MediaPlaylistItems = 172;
	static constexpr const int Playing = 173;
	static constexpr const int SourceMediaPath = 174;
	static constexpr const int SourceMediaPathDescription = 175;
	static constexpr const int SourceMediaPathPrompt = 176;
	static constexpr const int SourceMediaPathPromptWindows = 177;
	static constexpr const int MediaDataPath = 178;
	static constexpr const int MediaDataPathDescription = 179;
	static constexpr const int MediaDataPathPrompt = 180;
	static constexpr const int MediaDataPathPromptWindows = 181;
	static constexpr const int MediaScan = 182;
	static constexpr const int ScanMediaTooltip = 183;
	static constexpr const int CleanMediaTooltip = 184;
	static constexpr const int NoPlaylistItemSelectedPrompt = 185;
	static constexpr const int PlayerUiSearchTooltip = 186;
	static constexpr const int PlayerUiMenuTooltip = 187;
	static constexpr const int PlayerUiEmptyMediaStatusTitle = 188;
	static constexpr const int PlayerUiEmptyMediaStatusText = 189;
	static constexpr const int PlayerUiEmptySearchResultStatusTitle = 190;
	static constexpr const int PlayerUiEmptySearchResultStatusText = 191;
	static constexpr const int PlayerUiHelpTitle = 192;
	static constexpr const int PlayerUiHelpText = 193;
	static constexpr const int PlayerUiHelpAction1Text = 194;
	static constexpr const int PlayerUiHelpAction2Text = 195;
	static constexpr const int PlayerUiHelpAction3Text = 196;
	static constexpr const int PlayerUiNoMediaSelectedPrompt = 197;
	static constexpr const int PlayerUiNoPlayersPrompt = 198;
	static constexpr const int PlayerUiCreatePlaylistTooltip = 199;
	static constexpr const int PlayerUiPlayTooltip = 200;
	static constexpr const int PlayerUiPauseTooltip = 201;
	static constexpr const int PlayerUiFullscreenTooltip = 202;
	static constexpr const int PlayerUiStopTooltip = 203;
	static constexpr const int PlayerUiTagMenuTooltip = 204;
	static constexpr const int PlayerUiTagActionPrompt = 205;
	static constexpr const int PlayerUiAddTagCompleteText = 206;
	static constexpr const int PlayerUiRemoveTagCompleteText = 207;
	static constexpr const int PlayerUiTagRecordUpdateError = 208;
	static constexpr const int PlayerUiAudioDisabledAlertWindowTitle = 209;
	static constexpr const int PlayerUiAudioDisabledAlertWindowText = 210;
	static constexpr const int SearchEmptyResultTooltip = 211;
	static constexpr const int SearchComplete = 212;
	static constexpr const int SearchInProgress = 213;
	static constexpr const int SearchInProgressTooltip = 214;
	static constexpr const int SelectAllPrompt = 215;
	static constexpr const int SelectAllTooltip = 216;
	static constexpr const int ShowPlaylists = 217;
	static constexpr const int Result = 218;
	static constexpr const int Results = 219;
	static constexpr const int ViewMediaDetails = 220;
	static constexpr const int MediaItemUiHelpTitle = 221;
	static constexpr const int MediaItemUiHelpText = 222;
	static constexpr const int MediaItemUiHelpAction1Text = 223;
	static constexpr const int MediaItemUiHelpAction2Text = 224;
	static constexpr const int MediaItemUiViewBeforeTooltip = 225;
	static constexpr const int MediaItemUiViewBeforePrompt = 226;
	static constexpr const int MediaItemUiViewAfterTooltip = 227;
	static constexpr const int MediaItemUiViewAfterPrompt = 228;
	static constexpr const int MediaItemUiPlayTooltip = 229;
	static constexpr const int MediaItemUiSelectPlayPositionTooltip = 230;
	static constexpr const int MediaItemUiSelectPlayPositionPrompt = 231;
	static constexpr const int MediaItemUiAddMarkerTooltip = 232;
	static constexpr const int MediaItemUiAddMarkerPrompt = 233;
	static constexpr const int MediaItemUiNoFrameSelectedPrompt = 234;
	static constexpr const int MediaItemUiNoFrameOrMarkerSelectedPrompt = 235;
	static constexpr const int MediaItemUiLoadThumbnailError = 236;
	static constexpr const int MediaItemUiAddTagAlreadyExistsText = 237;
	static constexpr const int MediaItemUiTagRecordUpdateError = 238;
	static constexpr const int MediaItemUiAddTagCompleteText = 239;
	static constexpr const int MediaItemUiRemoveTagCompleteText = 240;
	static constexpr const int MediaName = 241;
	static constexpr const int ThumbnailImageSizeTooltip = 242;
	static constexpr const int UiBackTooltip = 243;
	static constexpr const int RelatedHelpTopics = 244;
	static constexpr const int OpenHelpUrlError = 245;
	static constexpr const int OpenAboutUrlError = 246;
	static constexpr const int OpenFeedbackUrlError = 247;
	static constexpr const int LaunchedWebBrowser = 248;
	static constexpr const int InternalError = 249;
	static constexpr const int ShuffleTooltip = 250;
	static constexpr const int ClickRenameTooltip = 251;
	static constexpr const int HyperlinkTooltip = 252;
	static constexpr const int ActionWindowInvalidDataTooltip = 253;
	static constexpr const int AddedPlaylistItem = 254;
	static constexpr const int AddedPlaylistItems = 255;
	static constexpr const int PlaylistItem = 256;
	static constexpr const int PlaylistItems = 257;
	static constexpr const int MediaAttributesTooltip = 258;
	static constexpr const int ToolbarModeButtonTooltip = 259;
	static constexpr const int WindowsRootPathName = 260;
	static constexpr const int UnixRootPathName = 261;
	static constexpr const int FsBrowserWindowLoadErrorText = 262;
	static constexpr const int FsBrowserWindowEnterTextButtonTooltip = 263;
	static constexpr const int FsBrowserWindowComponentButtonTooltip = 264;
	static constexpr const int FsBrowserWindowComponentBackButtonTooltip = 265;
	static constexpr const int FsBrowserWindowConfirmButtonTooltip = 266;
	static constexpr const int FsBrowserWindowDisabledButtonLoadingTooltip = 267;
	static constexpr const int FsBrowserWindowDisabledButtonLoadErrorTooltip = 268;
	static constexpr const int FsBrowserWindowDisabledButtonEmptyPathTooltip = 269;
	static constexpr const int FsBrowserWindowDisabledButtonSelectFileTooltip = 270;
	static constexpr const int FsBrowserWindowDisabledButtonSelectDirectoryTooltip = 271;
	static constexpr const int FsBrowserWindowDisabledButtonSelectFileOrDirectoryTooltip = 272;
	static constexpr const int SortByName = 273;
	static constexpr const int SortByNewest = 274;
	static constexpr const int SortByFilePath = 275;
	static constexpr const int RemovePlaylistDescription = 276;
	static constexpr const int MediaPlaylistZeroStartPositionDescription = 277;
	static constexpr const int MediaPlaylistNearBeginningStartPositionDescription = 278;
	static constexpr const int MediaPlaylistMiddleStartPositionDescription = 279;
	static constexpr const int MediaPlaylistNearEndPositionDescription = 280;
	static constexpr const int MediaPlaylistFullRangePositionDescription = 281;
	static constexpr const int MediaPlaylistVeryShortPlayDurationDescription = 282;
	static constexpr const int MediaPlaylistShortPlayDurationDescription = 283;
	static constexpr const int MediaPlaylistMediumPlayDurationDescription = 284;
	static constexpr const int MediaPlaylistLongPlayDurationDescription = 285;
	static constexpr const int MediaPlaylistVeryLongPlayDurationDescription = 286;
	static constexpr const int MediaPlaylistFullPlayDurationDescription = 287;
	static constexpr const int MediaPlaylistWindowSettingsTooltip = 288;
	static constexpr const int MediaPlaylistWindowSkipPreviousTooltip = 289;
	static constexpr const int MediaPlaylistUiHelpTitle = 290;
	static constexpr const int MediaPlaylistUiHelpText = 291;
	static constexpr const int MediaPlaylistUiHelpActionText = 292;
	static constexpr const int MediaPlaylistMoveUpTooltip = 293;
	static constexpr const int MediaPlaylistMoveToTopTooltip = 294;
	static constexpr const int MediaPlaylistRemoveTooltip = 295;
	static constexpr const int MoveUp = 296;
	static constexpr const int MoveToTop = 297;
	static constexpr const int PlaylistName = 298;
	static constexpr const int CreatedPlaylist = 299;
	static constexpr const int ImageDialogLoadError = 300;
	static constexpr const int AddSearchKey = 301;
	static constexpr const int RemoveSearchKey = 302;
	static constexpr const int TagActionWindowDescriptionText = 303;
	static constexpr const int TagActionWindowTextFieldPrompt = 304;
	static constexpr const int SearchKey = 305;
	static constexpr const int SearchKeys = 306;
	static constexpr const int AddMediaTagPrompt = 307;
	static constexpr const int RemoveMediaTagPrompt = 308;
	static constexpr const int MediaItemTagWindowTextFieldPrompt = 309;
	static constexpr const int VideoPlayer = 310;
	static constexpr const int VideoPlayers = 311;
	static constexpr const int FillWindow = 312;
	static constexpr const int ShowGrid = 313;
	static constexpr const int ShowLines = 314;
	static constexpr const int MediaPlayerInterface = 315;
	static constexpr const int MediaInspectorInterface = 316;
	static constexpr const int PlaylistEditorInterface = 317;
	static constexpr const int Ready = 318;
	static constexpr const int Initializing = 319;
	static constexpr const int Scanning = 320;
	static constexpr const int Cleaning = 321;
	static constexpr const int ConfigurationError = 322;
	static constexpr const int ConfigurationUpdated = 323;
	static constexpr const int MediaSourceDirectoryNotFoundErrorText = 324;
	static constexpr const int DataDirectoryNotFoundErrorText = 325;
	static constexpr const int MediaControlReadyErrorText = 326;
	static constexpr const int MediaControlReadDataErrorText = 327;
	static constexpr const int MediaControlWriteDataErrorText = 328;
	static constexpr const int MediaSourceDirectories = 329;
	static constexpr const int AddMediaDirectory = 330;
	static constexpr const int DataDirectory = 331;
	static constexpr const int MediaControlPrimeWindowPrompt = 332;
	static constexpr const int TotalFileSize = 333;
	static constexpr const int TotalDuration = 334;
	static constexpr const int ScanCancelled = 335;
	static constexpr const int ScanComplete = 336;
	static constexpr const int ReadingMediaDirectory = 337;
	static constexpr const int MediaControlScanWindowTitle = 338;
	static constexpr const int MediaControlScanWindowPrompt = 339;
	static constexpr const int MediaControlScanWindowExecuteTooltip = 340;
	static constexpr const int MediaControlCleanWindowTitle = 341;
	static constexpr const int MediaControlCleanWindowPrompt = 342;
	static constexpr const int MediaControlCleanWindowExecuteTooltip = 343;
	static constexpr const int MediaControlConfigureWindowEmptySourcePathPrompt = 344;
	static constexpr const int MediaControlConfigureWindowMediaThumbnailCountText = 345;
	static constexpr const int MediaControlConfigureWindowExecuteTooltip = 346;
	static constexpr const int MediaControlConfigureWindowChangePrompt = 347;
	static constexpr const int MediaControlConfigureWindowChangeBlockedPrompt = 348;
	static constexpr const int MediaControlConfigureWindowChangeWaitingPrompt = 349;
	static constexpr const int MediaThumbnailEveryHourDescription = 350;
	static constexpr const int MediaThumbnailEvery10MinutesDescription = 351;
	static constexpr const int MediaThumbnailEveryMinuteDescription = 352;
	static constexpr const int MediaThumbnailEvery10SecondsDescription = 353;
	static constexpr const int MediaThumbnailDivide1Description = 354;
	static constexpr const int MediaThumbnailDivide12Description = 355;
	static constexpr const int MediaThumbnailDivide48Description = 356;
	static constexpr const int MediaThumbnailDivide100Description = 357;
	static constexpr const int MediaControlTaskStatusWindowWaitingPrompt = 358;
	static constexpr const int BeginMediaScan = 359;
	static constexpr const int MediaScanFailed = 360;
	static constexpr const int MediaScanCancelled = 361;
	static constexpr const int EndMediaScan = 362;
	static constexpr const int NewFileFound = 363;
	static constexpr const int NewFilesFound = 364;
	static constexpr const int ScanError = 365;
	static constexpr const int ScanErrors = 366;
	static constexpr const int BeginMediaDataClean = 367;
	static constexpr const int EndMediaDataClean = 368;
	static constexpr const int CleanFailed = 369;
	static constexpr const int CleanComplete = 370;
	static constexpr const int MediaRecordRemoved = 371;
	static constexpr const int MediaRecordsRemoved = 372;
	static constexpr const int Freed = 373;
	static constexpr const int TimeMarkers = 374;
	static constexpr const int RemoveTimeMarker = 375;
	static constexpr const int Frames = 376;
	static constexpr const int PauseButtonTooltip = 377;
	static constexpr const int PlayerWindowRewindButtonTooltip = 378;
	static constexpr const int PlayerWindowForwardButtonTooltip = 379;
	static constexpr const int PlayerWindowDetachButtonTooltip = 380;
	static constexpr const int PlayerWindowMaximizeButtonTooltip = 381;
	static constexpr const int Mute = 382;
	static constexpr const int Unmute = 383;
};
#endif
