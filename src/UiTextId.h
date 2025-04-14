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
	static constexpr const int SeekPositionNotFound = 77;
	static constexpr const int Help = 78;
	static constexpr const int Status = 79;
	static constexpr const int Exit = 80;
	static constexpr const int Next = 81;
	static constexpr const int Log = 82;
	static constexpr const int Settings = 83;
	static constexpr const int About = 84;
	static constexpr const int SendFeedback = 85;
	static constexpr const int CheckForUpdates = 86;
	static constexpr const int CheckUpdatesOnStartup = 87;
	static constexpr const int CheckingForUpdates = 88;
	static constexpr const int NoUpdatesPrompt = 89;
	static constexpr const int UpdateErrorPrompt = 90;
	static constexpr const int UpdateFoundPrompt = 91;
	static constexpr const int AppWindow = 92;
	static constexpr const int FullscreenWindow = 93;
	static constexpr const int ShowClock = 94;
	static constexpr const int TextSize = 95;
	static constexpr const int SoundVolume = 96;
	static constexpr const int SoundMixLevel = 97;
	static constexpr const int Network = 98;
	static constexpr const int AppInfo = 99;
	static constexpr const int Version = 100;
	static constexpr const int BuildDateText = 101;
	static constexpr const int MediaFile = 102;
	static constexpr const int MediaFiles = 103;
	static constexpr const int EditPlaylist = 104;
	static constexpr const int DeletePlaylist = 105;
	static constexpr const int AddPlaylistItems = 106;
	static constexpr const int StartPosition = 107;
	static constexpr const int PlayDuration = 108;
	static constexpr const int Playlist = 109;
	static constexpr const int Playlists = 110;
	static constexpr const int PlayedItems = 111;
	static constexpr const int Smallest = 112;
	static constexpr const int Small = 113;
	static constexpr const int Medium = 114;
	static constexpr const int Large = 115;
	static constexpr const int Largest = 116;
	static constexpr const int Player = 117;
	static constexpr const int Play = 118;
	static constexpr const int Stop = 119;
	static constexpr const int Pause = 120;
	static constexpr const int Resume = 121;
	static constexpr const int Disabled = 122;
	static constexpr const int SearchForHelp = 123;
	static constexpr const int Configuring = 124;
	static constexpr const int Loading = 125;
	static constexpr const int FileSize = 126;
	static constexpr const int Duration = 127;
	static constexpr const int Configure = 128;
	static constexpr const int Configuration = 129;
	static constexpr const int ConfigurationRequired = 130;
	static constexpr const int EnterPlaylistNamePrompt = 131;
	static constexpr const int EnterSearchKeyPrompt = 132;
	static constexpr const int Expand = 133;
	static constexpr const int Minimize = 134;
	static constexpr const int ExpandAll = 135;
	static constexpr const int MinimizeAll = 136;
	static constexpr const int MainMenuTooltip = 137;
	static constexpr const int TextFieldEnterTooltip = 138;
	static constexpr const int TextFieldPasteTooltip = 139;
	static constexpr const int TextFieldClearTooltip = 140;
	static constexpr const int TextFieldRandomizeTooltip = 141;
	static constexpr const int TextFieldVisibilityToggleTooltip = 142;
	static constexpr const int TextFieldFsBrowseTooltip = 143;
	static constexpr const int AppInfoText1 = 144;
	static constexpr const int AppInfoText2 = 145;
	static constexpr const int AppInfoText3 = 146;
	static constexpr const int AppInfoText4 = 147;
	static constexpr const int AppInfoText5 = 148;
	static constexpr const int AppInfoText6 = 149;
	static constexpr const int AppInfoText7 = 150;
	static constexpr const int AppInfoText8 = 151;
	static constexpr const int AppInfoText9 = 152;
	static constexpr const int AppInfoText10 = 153;
	static constexpr const int AppInfoText11 = 154;
	static constexpr const int AppInfoText12 = 155;
	static constexpr const int AppInfoText13 = 156;
	static constexpr const int AppInfoText14 = 157;
	static constexpr const int AppInfoText15 = 158;
	static constexpr const int AppInfoText16 = 159;
	static constexpr const int AppInfoText17 = 160;
	static constexpr const int AppInfoPrimeTooltip = 161;
	static constexpr const int AppInfoBackTooltip = 162;
	static constexpr const int AppInfoPrimeText1 = 163;
	static constexpr const int AppInfoPrimeText2 = 164;
	static constexpr const int AppInfoPrimeText3 = 165;
	static constexpr const int MediaOptions = 166;
	static constexpr const int MediaOptionWindowSavePlayHistoryPrompt = 167;
	static constexpr const int MediaOptionWindowShowPlayHistoryPrompt = 168;
	static constexpr const int MediaOptionWindowSavePlaylistsPrompt = 169;
	static constexpr const int MediaOptionWindowShowPlaylistsPrompt = 170;
	static constexpr const int MediaOptionWindowMediaScanPrompt = 171;
	static constexpr const int MediaOptionWindowDataPathPrompt = 172;
	static constexpr const int MediaOptionWindowCancelTooltip = 173;
	static constexpr const int PlayFileActionWindowPathPrompt = 174;
	static constexpr const int PlayFileActionWindowPlayTooltip = 175;
	static constexpr const int PlayFileError = 176;
	static constexpr const int MediaPlaylistItem = 177;
	static constexpr const int MediaPlaylistItems = 178;
	static constexpr const int Playing = 179;
	static constexpr const int MediaDataPathPrompt = 180;
	static constexpr const int MediaDataPathPromptWindows = 181;
	static constexpr const int MediaScan = 182;
	static constexpr const int ScanMediaTooltip = 183;
	static constexpr const int CleanMediaTooltip = 184;
	static constexpr const int NoPlaylistItemSelectedPrompt = 185;
	static constexpr const int PlayerUiSearchTooltip = 186;
	static constexpr const int PlayerUiMenuTooltip = 187;
	static constexpr const int PlayerUiNavigateTooltip = 188;
	static constexpr const int PlayerUiPlayFileTooltip = 189;
	static constexpr const int PlayerUiClearPlayHistoryTooltip = 190;
	static constexpr const int PlayerUiEmptyMediaStatusTitle = 191;
	static constexpr const int PlayerUiEmptyMediaStatusText = 192;
	static constexpr const int PlayerUiEmptySearchResultStatusTitle = 193;
	static constexpr const int PlayerUiEmptySearchResultStatusText = 194;
	static constexpr const int PlayerUiHelpTitle = 195;
	static constexpr const int PlayerUiHelpText = 196;
	static constexpr const int PlayerUiHelpAction1Text = 197;
	static constexpr const int PlayerUiHelpAction2Text = 198;
	static constexpr const int PlayerUiNoMediaSelectedPrompt = 199;
	static constexpr const int PlayerUiNoPlayersPrompt = 200;
	static constexpr const int PlayerUiEmptyPlayHistoryPrompt = 201;
	static constexpr const int PlayerUiCreatePlaylistTooltip = 202;
	static constexpr const int PlayerUiPlayTooltip = 203;
	static constexpr const int PlayerUiPauseTooltip = 204;
	static constexpr const int PlayerUiFullscreenTooltip = 205;
	static constexpr const int PlayerUiStopTooltip = 206;
	static constexpr const int PlayerUiNoTagMediaSelectedPrompt = 207;
	static constexpr const int PlayerUiTagActionTooltip = 208;
	static constexpr const int PlayerUiTagActionPrompt = 209;
	static constexpr const int PlayerUiAddTagCompleteText = 210;
	static constexpr const int PlayerUiRemoveTagCompleteText = 211;
	static constexpr const int PlayerUiTagRecordUpdateError = 212;
	static constexpr const int PlayerUiAudioDisabledAlertWindowTitle = 213;
	static constexpr const int PlayerUiAudioDisabledAlertWindowText = 214;
	static constexpr const int PlayerUiMaxPlayerCountError = 215;
	static constexpr const int MediaOptionConfiguredText = 216;
	static constexpr const int SearchStatusIconTooltip = 217;
	static constexpr const int SearchCompleteTooltip = 218;
	static constexpr const int SearchInProgressTooltip = 219;
	static constexpr const int SelectAllPrompt = 220;
	static constexpr const int SelectAllTooltip = 221;
	static constexpr const int ViewMediaDetails = 222;
	static constexpr const int MediaItemUiHelpTitle = 223;
	static constexpr const int MediaItemUiHelpText = 224;
	static constexpr const int MediaItemUiHelpAction1Text = 225;
	static constexpr const int MediaItemUiHelpAction2Text = 226;
	static constexpr const int MediaItemUiHelpAction3Text = 227;
	static constexpr const int MediaItemUiHelpAction4Text = 228;
	static constexpr const int MediaItemUiViewBeforeTooltip = 229;
	static constexpr const int MediaItemUiViewBeforePrompt = 230;
	static constexpr const int MediaItemUiViewAfterTooltip = 231;
	static constexpr const int MediaItemUiViewAfterPrompt = 232;
	static constexpr const int MediaItemUiPlayTooltip = 233;
	static constexpr const int MediaItemUiSelectPlayPositionTooltip = 234;
	static constexpr const int MediaItemUiSelectPlayPositionPrompt = 235;
	static constexpr const int MediaItemUiAddMarkerTooltip = 236;
	static constexpr const int MediaItemUiAddMarkerPrompt = 237;
	static constexpr const int MediaItemUiNoFrameSelectedPrompt = 238;
	static constexpr const int MediaItemUiNoFrameOrMarkerSelectedPrompt = 239;
	static constexpr const int MediaItemUiLoadThumbnailError = 240;
	static constexpr const int MediaItemUiAddTagAlreadyExistsText = 241;
	static constexpr const int MediaItemUiTagRecordUpdateError = 242;
	static constexpr const int MediaItemUiAddTagCompleteText = 243;
	static constexpr const int MediaItemUiRemoveTagCompleteText = 244;
	static constexpr const int MediaName = 245;
	static constexpr const int ThumbnailImageSizeTooltip = 246;
	static constexpr const int UiBackTooltip = 247;
	static constexpr const int RelatedHelpTopics = 248;
	static constexpr const int OpenHelpUrlError = 249;
	static constexpr const int OpenAboutUrlError = 250;
	static constexpr const int OpenFeedbackUrlError = 251;
	static constexpr const int OpenUpdateUrlError = 252;
	static constexpr const int OpenUpdateUrlCompleteText = 253;
	static constexpr const int LaunchedWebBrowser = 254;
	static constexpr const int InternalError = 255;
	static constexpr const int ShuffleTooltip = 256;
	static constexpr const int ClickRenameTooltip = 257;
	static constexpr const int HyperlinkTooltip = 258;
	static constexpr const int ActionWindowInvalidDataTooltip = 259;
	static constexpr const int AddedPlaylistItem = 260;
	static constexpr const int AddedPlaylistItems = 261;
	static constexpr const int PlaylistItem = 262;
	static constexpr const int PlaylistItems = 263;
	static constexpr const int VideoAttributes = 264;
	static constexpr const int AudioAttributes = 265;
	static constexpr const int AlbumArt = 266;
	static constexpr const int ToolbarModeButtonTooltip = 267;
	static constexpr const int WindowsRootPathName = 268;
	static constexpr const int UnixRootPathName = 269;
	static constexpr const int FsBrowserWindowLoadErrorText = 270;
	static constexpr const int FsBrowserWindowEnterTextButtonTooltip = 271;
	static constexpr const int FsBrowserWindowComponentButtonTooltip = 272;
	static constexpr const int FsBrowserWindowComponentBackButtonTooltip = 273;
	static constexpr const int FsBrowserWindowConfirmButtonTooltip = 274;
	static constexpr const int FsBrowserWindowDisabledButtonLoadingTooltip = 275;
	static constexpr const int FsBrowserWindowDisabledButtonLoadErrorTooltip = 276;
	static constexpr const int FsBrowserWindowDisabledButtonEmptyPathTooltip = 277;
	static constexpr const int FsBrowserWindowDisabledButtonSelectFileTooltip = 278;
	static constexpr const int FsBrowserWindowDisabledButtonSelectDirectoryTooltip = 279;
	static constexpr const int FsBrowserWindowDisabledButtonSelectFileOrDirectoryTooltip = 280;
	static constexpr const int SortByName = 281;
	static constexpr const int SortByNewest = 282;
	static constexpr const int SortByFilePath = 283;
	static constexpr const int RemovePlaylistDescription = 284;
	static constexpr const int MediaPlaylistZeroStartPositionDescription = 285;
	static constexpr const int MediaPlaylistNearBeginningStartPositionDescription = 286;
	static constexpr const int MediaPlaylistMiddleStartPositionDescription = 287;
	static constexpr const int MediaPlaylistNearEndPositionDescription = 288;
	static constexpr const int MediaPlaylistFullRangePositionDescription = 289;
	static constexpr const int MediaPlaylistVeryShortPlayDurationDescription = 290;
	static constexpr const int MediaPlaylistShortPlayDurationDescription = 291;
	static constexpr const int MediaPlaylistMediumPlayDurationDescription = 292;
	static constexpr const int MediaPlaylistLongPlayDurationDescription = 293;
	static constexpr const int MediaPlaylistVeryLongPlayDurationDescription = 294;
	static constexpr const int MediaPlaylistFullPlayDurationDescription = 295;
	static constexpr const int MediaPlaylistWindowSettingsTooltip = 296;
	static constexpr const int MediaPlaylistWindowSkipPreviousTooltip = 297;
	static constexpr const int MediaPlaylistUiHelpTitle = 298;
	static constexpr const int MediaPlaylistUiHelpText = 299;
	static constexpr const int MediaPlaylistUiHelpActionText = 300;
	static constexpr const int MediaPlaylistMoveUpTooltip = 301;
	static constexpr const int MediaPlaylistMoveToTopTooltip = 302;
	static constexpr const int MediaPlaylistRemoveTooltip = 303;
	static constexpr const int MoveUp = 304;
	static constexpr const int MoveToTop = 305;
	static constexpr const int PlaylistName = 306;
	static constexpr const int CreatedPlaylist = 307;
	static constexpr const int ImageDialogLoadError = 308;
	static constexpr const int AddSearchKey = 309;
	static constexpr const int RemoveSearchKey = 310;
	static constexpr const int TagActionWindowDescriptionText = 311;
	static constexpr const int TagActionWindowTextFieldPrompt = 312;
	static constexpr const int SearchKey = 313;
	static constexpr const int SearchKeys = 314;
	static constexpr const int AddMediaTagPrompt = 315;
	static constexpr const int RemoveMediaTagPrompt = 316;
	static constexpr const int MediaItemTagWindowTextFieldPrompt = 317;
	static constexpr const int VideoPlayer = 318;
	static constexpr const int VideoPlayers = 319;
	static constexpr const int FillWindow = 320;
	static constexpr const int ShowGrid = 321;
	static constexpr const int ShowLines = 322;
	static constexpr const int MediaPlayerInterface = 323;
	static constexpr const int MediaPlayerWindow = 324;
	static constexpr const int MediaInspectorInterface = 325;
	static constexpr const int PlaylistEditorInterface = 326;
	static constexpr const int Ready = 327;
	static constexpr const int Initializing = 328;
	static constexpr const int Scanning = 329;
	static constexpr const int Cleaning = 330;
	static constexpr const int ConfigurationError = 331;
	static constexpr const int ConfigurationUpdated = 332;
	static constexpr const int DataDirectoryNotFoundErrorText = 333;
	static constexpr const int MediaSourceDirectoryNotFoundErrorText = 334;
	static constexpr const int MediaControlReadyErrorText = 335;
	static constexpr const int MediaControlReadDataErrorText = 336;
	static constexpr const int MediaControlWriteDataErrorText = 337;
	static constexpr const int MediaSourceDirectories = 338;
	static constexpr const int AddMediaDirectory = 339;
	static constexpr const int MediaFilescanConfigurePrompt = 340;
	static constexpr const int MediaFilescanConfiguredText = 341;
	static constexpr const int TotalFileSize = 342;
	static constexpr const int TotalDuration = 343;
	static constexpr const int ScanCancelled = 344;
	static constexpr const int ScanComplete = 345;
	static constexpr const int ReadingMediaDirectory = 346;
	static constexpr const int MediaFilescanRunWindowTitle = 347;
	static constexpr const int MediaFilescanRunWindowPrompt = 348;
	static constexpr const int MediaFilescanRunWindowExecuteTooltip = 349;
	static constexpr const int MediaFilescanCleanWindowTitle = 350;
	static constexpr const int MediaFilescanCleanWindowPrompt = 351;
	static constexpr const int MediaFilescanCleanWindowExecuteTooltip = 352;
	static constexpr const int MediaFilescanConfigureWindowEmptySourcePathPrompt = 353;
	static constexpr const int MediaFilescanConfigureWindowMediaThumbnailCountText = 354;
	static constexpr const int MediaFilescanConfigureWindowExecuteTooltip = 355;
	static constexpr const int MediaFilescanConfigureWindowChangeRescanPrompt = 356;
	static constexpr const int MediaFilescanConfigureWindowChangeBlockedPrompt = 357;
	static constexpr const int MediaThumbnailEveryHourDescription = 358;
	static constexpr const int MediaThumbnailEvery10MinutesDescription = 359;
	static constexpr const int MediaThumbnailEveryMinuteDescription = 360;
	static constexpr const int MediaThumbnailEvery10SecondsDescription = 361;
	static constexpr const int MediaThumbnailDivide1Description = 362;
	static constexpr const int MediaThumbnailDivide12Description = 363;
	static constexpr const int MediaThumbnailDivide48Description = 364;
	static constexpr const int MediaThumbnailDivide100Description = 365;
	static constexpr const int MediaControlTaskStatusWindowWaitingPrompt = 366;
	static constexpr const int BeginMediaScan = 367;
	static constexpr const int MediaScanFailed = 368;
	static constexpr const int MediaScanCancelled = 369;
	static constexpr const int EndMediaScan = 370;
	static constexpr const int NewFileFound = 371;
	static constexpr const int NewFilesFound = 372;
	static constexpr const int ScanError = 373;
	static constexpr const int ScanErrors = 374;
	static constexpr const int BeginMediaDataClean = 375;
	static constexpr const int EndMediaDataClean = 376;
	static constexpr const int CleanFailed = 377;
	static constexpr const int CleanComplete = 378;
	static constexpr const int MediaRecordRemoved = 379;
	static constexpr const int MediaRecordsRemoved = 380;
	static constexpr const int Freed = 381;
	static constexpr const int TimeMarkers = 382;
	static constexpr const int RemoveTimeMarker = 383;
	static constexpr const int Frames = 384;
	static constexpr const int PauseButtonTooltip = 385;
	static constexpr const int PlayerWindowRewindButtonTooltip = 386;
	static constexpr const int PlayerWindowForwardButtonTooltip = 387;
	static constexpr const int PlayerWindowDetachButtonTooltip = 388;
	static constexpr const int PlayerWindowMaximizeButtonTooltip = 389;
	static constexpr const int PlayerWindowVisualizerButtonTooltip = 390;
	static constexpr const int PlayerWindowSubtitleNotAvailableTooltip = 391;
	static constexpr const int PlayerWindowSubtitleEnableTooltip = 392;
	static constexpr const int PlayerWindowSubtitleDisableTooltip = 393;
	static constexpr const int Mute = 394;
	static constexpr const int Unmute = 395;
};
#endif
