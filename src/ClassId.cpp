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
#include "ClassId.h"

const char *ClassId::classIdName (int id) {
	switch (id) {
		case ClassId::ActionWindow: {
			return ("ActionWindow");
		}
		case ClassId::App: {
			return ("App");
		}
		case ClassId::AppCardWindow: {
			return ("AppCardWindow");
		}
		case ClassId::AppNews: {
			return ("AppNews");
		}
		case ClassId::AppUrl: {
			return ("AppUrl");
		}
		case ClassId::AppUtil: {
			return ("AppUtil");
		}
		case ClassId::Buffer: {
			return ("Buffer");
		}
		case ClassId::ButtonGradientEndSprite: {
			return ("ButtonGradientEndSprite");
		}
		case ClassId::ButtonGradientMiddleSprite: {
			return ("ButtonGradientMiddleSprite");
		}
		case ClassId::Button: {
			return ("Button");
		}
		case ClassId::CaptureWriter: {
			return ("CaptureWriter");
		}
		case ClassId::CardLabelWindow: {
			return ("CardLabelWindow");
		}
		case ClassId::CardView: {
			return ("CardView");
		}
		case ClassId::Chip: {
			return ("Chip");
		}
		case ClassId::Color: {
			return ("Color");
		}
		case ClassId::ComboBox: {
			return ("ComboBox");
		}
		case ClassId::ConsoleWindow: {
			return ("ConsoleWindow");
		}
		case ClassId::Database: {
			return ("Database");
		}
		case ClassId::DoubleList: {
			return ("DoubleList");
		}
		case ClassId::FloatList: {
			return ("FloatList");
		}
		case ClassId::Font: {
			return ("Font");
		}
		case ClassId::FsBrowserWindowItemLabel: {
			return ("FsBrowserWindowItemLabel");
		}
		case ClassId::FsBrowserWindow: {
			return ("FsBrowserWindow");
		}
		case ClassId::HashMap: {
			return ("HashMap");
		}
		case ClassId::HelpActionWindow: {
			return ("HelpActionWindow");
		}
		case ClassId::HelpWindow: {
			return ("HelpWindow");
		}
		case ClassId::HyperlinkWindow: {
			return ("HyperlinkWindow");
		}
		case ClassId::IconLabelWindow: {
			return ("IconLabelWindow");
		}
		case ClassId::Image: {
			return ("Image");
		}
		case ClassId::ImageWindow: {
			return ("ImageWindow");
		}
		case ClassId::Input: {
			return ("Input");
		}
		case ClassId::Int64List: {
			return ("Int64List");
		}
		case ClassId::IntList: {
			return ("IntList");
		}
		case ClassId::Ipv4Address: {
			return ("Ipv4Address");
		}
		case ClassId::Json: {
			return ("Json");
		}
		case ClassId::JsonList: {
			return ("JsonList");
		}
		case ClassId::Label: {
			return ("Label");
		}
		case ClassId::LabelWindow: {
			return ("LabelWindow");
		}
		case ClassId::ListViewItemLabel: {
			return ("ListViewItemLabel");
		}
		case ClassId::ListView: {
			return ("ListView");
		}
		case ClassId::Log: {
			return ("Log");
		}
		case ClassId::LuaAwaitloadimagesFunction: {
			return ("LuaAwaitloadimagesFunction");
		}
		case ClassId::LuaAwaitvalueFunction: {
			return ("LuaAwaitvalueFunction");
		}
		case ClassId::LuaCapturescreenFunction: {
			return ("LuaCapturescreenFunction");
		}
		case ClassId::LuaCapturescreenpathFunction: {
			return ("LuaCapturescreenpathFunction");
		}
		case ClassId::LuaClickFunction: {
			return ("LuaClickFunction");
		}
		case ClassId::LuaDofileFunction: {
			return ("LuaDofileFunction");
		}
		case ClassId::LuaFunction: {
			return ("LuaFunction");
		}
		case ClassId::LuaFunctionList: {
			return ("LuaFunctionList");
		}
		case ClassId::LuaGetenvFunction: {
			return ("LuaGetenvFunction");
		}
		case ClassId::LuaHelpFunction: {
			return ("LuaHelpFunction");
		}
		case ClassId::LuaKeypressFunction: {
			return ("LuaKeypressFunction");
		}
		case ClassId::LuaMenuselectFunction: {
			return ("LuaMenuselectFunction");
		}
		case ClassId::LuaMouseleftclickFunction: {
			return ("LuaMouseleftclickFunction");
		}
		case ClassId::LuaMouseoverFunction: {
			return ("LuaMouseoverFunction");
		}
		case ClassId::LuaMousepointerFunction: {
			return ("LuaMousepointerFunction");
		}
		case ClassId::LuaMousewarpFunction: {
			return ("LuaMousewarpFunction");
		}
		case ClassId::LuaOpenFunction: {
			return ("LuaOpenFunction");
		}
		case ClassId::LuaPrintcontrolsFunction: {
			return ("LuaPrintcontrolsFunction");
		}
		case ClassId::LuaPrintFunction: {
			return ("LuaPrintFunction");
		}
		case ClassId::LuaQuitFunction: {
			return ("LuaQuitFunction");
		}
		case ClassId::LuaScript: {
			return ("LuaScript");
		}
		case ClassId::LuaSetvalueFunction: {
			return ("LuaSetvalueFunction");
		}
		case ClassId::LuaSleepFunction: {
			return ("LuaSleepFunction");
		}
		case ClassId::LuaStartstreamFunction: {
			return ("LuaStartstreamFunction");
		}
		case ClassId::LuaStopstreamFunction: {
			return ("LuaStopstreamFunction");
		}
		case ClassId::LuaStreampathFunction: {
			return ("LuaStreampathFunction");
		}
		case ClassId::LuaTargetFunction: {
			return ("LuaTargetFunction");
		}
		case ClassId::LuaTimeoutFunction: {
			return ("LuaTimeoutFunction");
		}
		case ClassId::LuaUntargetFunction: {
			return ("LuaUntargetFunction");
		}
		case ClassId::MainToolbarWindow: {
			return ("MainToolbarWindow");
		}
		case ClassId::MathUtil: {
			return ("MathUtil");
		}
		case ClassId::MediaControl: {
			return ("MediaControl");
		}
		case ClassId::MediaControlCleanWindow: {
			return ("MediaControlCleanWindow");
		}
		case ClassId::MediaControlConfigureWindow: {
			return ("MediaControlConfigureWindow");
		}
		case ClassId::MediaControlMainWindow: {
			return ("MediaControlMainWindow");
		}
		case ClassId::MediaControlPrimeWindow: {
			return ("MediaControlPrimeWindow");
		}
		case ClassId::MediaControlScanWindow: {
			return ("MediaControlScanWindow");
		}
		case ClassId::MediaControlSearch: {
			return ("MediaControlSearch");
		}
		case ClassId::MediaControlTaskStatusWindow: {
			return ("MediaControlTaskStatusWindow");
		}
		case ClassId::MediaControlWindow: {
			return ("MediaControlWindow");
		}
		case ClassId::MediaItem: {
			return ("MediaItem");
		}
		case ClassId::MediaItemDetailWindow: {
			return ("MediaItemDetailWindow");
		}
		case ClassId::MediaItemImageWindow: {
			return ("MediaItemImageWindow");
		}
		case ClassId::MediaItemTagWindow: {
			return ("MediaItemTagWindow");
		}
		case ClassId::MediaItemUi: {
			return ("MediaItemUi");
		}
		case ClassId::MediaItemWindow: {
			return ("MediaItemWindow");
		}
		case ClassId::MediaPlaylist: {
			return ("MediaPlaylist");
		}
		case ClassId::MediaPlaylistItem: {
			return ("MediaPlaylistItem");
		}
		case ClassId::MediaPlaylistUi: {
			return ("MediaPlaylistUi");
		}
		case ClassId::MediaPlaylistViewWindowItem: {
			return ("MediaPlaylistViewWindowItem");
		}
		case ClassId::MediaPlaylistViewWindowItemLabel: {
			return ("MediaPlaylistViewWindowItemLabel");
		}
		case ClassId::MediaPlaylistViewWindow: {
			return ("MediaPlaylistViewWindow");
		}
		case ClassId::MediaPlaylistWindow: {
			return ("MediaPlaylistWindow");
		}
		case ClassId::MediaReader: {
			return ("MediaReader");
		}
		case ClassId::MediaSearch: {
			return ("MediaSearch");
		}
		case ClassId::MediaThumbnailWindow: {
			return ("MediaThumbnailWindow");
		}
		case ClassId::MediaUtil: {
			return ("MediaUtil");
		}
		case ClassId::MediaWriter: {
			return ("MediaWriter");
		}
		case ClassId::Menu: {
			return ("Menu");
		}
		case ClassId::Network: {
			return ("Network");
		}
		case ClassId::OsUtil: {
			return ("OsUtil");
		}
		case ClassId::PanelLayoutFlow: {
			return ("PanelLayoutFlow");
		}
		case ClassId::Panel: {
			return ("Panel");
		}
		case ClassId::PlayerControl: {
			return ("PlayerControl");
		}
		case ClassId::PlayerTimelineWindow: {
			return ("PlayerTimelineWindow");
		}
		case ClassId::PlayerUi: {
			return ("PlayerUi");
		}
		case ClassId::PlayerWindow: {
			return ("PlayerWindow");
		}
		case ClassId::PlayMarker: {
			return ("PlayMarker");
		}
		case ClassId::Position: {
			return ("Position");
		}
		case ClassId::Prng: {
			return ("Prng");
		}
		case ClassId::ProgressBar: {
			return ("ProgressBar");
		}
		case ClassId::ProgressRing: {
			return ("ProgressRing");
		}
		case ClassId::ProgressRingSprite: {
			return ("ProgressRingSprite");
		}
		case ClassId::RecordStore: {
			return ("RecordStore");
		}
		case ClassId::RenderResource: {
			return ("RenderResource");
		}
		case ClassId::Resource: {
			return ("Resource");
		}
		case ClassId::RoundedCornerSprite: {
			return ("RoundedCornerSprite");
		}
		case ClassId::ScrollBar: {
			return ("ScrollBar");
		}
		case ClassId::ScrollView: {
			return ("ScrollView");
		}
		case ClassId::ScrollViewWindow: {
			return ("ScrollViewWindow");
		}
		case ClassId::SdlUtil: {
			return ("SdlUtil");
		}
		case ClassId::SettingsWindow: {
			return ("SettingsWindow");
		}
		case ClassId::SharedBuffer: {
			return ("SharedBuffer");
		}
		case ClassId::Slider: {
			return ("Slider");
		}
		case ClassId::SliderThumbSprite: {
			return ("SliderThumbSprite");
		}
		case ClassId::SliderWindow: {
			return ("SliderWindow");
		}
		case ClassId::SnackbarWindow: {
			return ("SnackbarWindow");
		}
		case ClassId::SoundId: {
			return ("SoundId");
		}
		case ClassId::SoundMixer: {
			return ("SoundMixer");
		}
		case ClassId::SoundSample: {
			return ("SoundSample");
		}
		case ClassId::Sprite: {
			return ("Sprite");
		}
		case ClassId::SpriteGroup: {
			return ("SpriteGroup");
		}
		case ClassId::SpriteId: {
			return ("SpriteId");
		}
		case ClassId::StatsWindow: {
			return ("StatsWindow");
		}
		case ClassId::StdString: {
			return ("StdString");
		}
		case ClassId::StringList: {
			return ("StringList");
		}
		case ClassId::SystemInterface: {
			return ("SystemInterface");
		}
		case ClassId::TagActionWindow: {
			return ("TagActionWindow");
		}
		case ClassId::TaskGroup: {
			return ("TaskGroup");
		}
		case ClassId::TextArea: {
			return ("TextArea");
		}
		case ClassId::TextCardWindow: {
			return ("TextCardWindow");
		}
		case ClassId::TextField: {
			return ("TextField");
		}
		case ClassId::TextFieldWindow: {
			return ("TextFieldWindow");
		}
		case ClassId::TextFlow: {
			return ("TextFlow");
		}
		case ClassId::Toggle: {
			return ("Toggle");
		}
		case ClassId::ToggleWindow: {
			return ("ToggleWindow");
		}
		case ClassId::Toolbar: {
			return ("Toolbar");
		}
		case ClassId::TooltipWindow: {
			return ("TooltipWindow");
		}
		case ClassId::Ui: {
			return ("Ui");
		}
		case ClassId::UiConfiguration: {
			return ("UiConfiguration");
		}
		case ClassId::UiLog: {
			return ("UiLog");
		}
		case ClassId::UiLogWindow: {
			return ("UiLogWindow");
		}
		case ClassId::UiStack: {
			return ("UiStack");
		}
		case ClassId::UiText: {
			return ("UiText");
		}
		case ClassId::UiTextId: {
			return ("UiTextId");
		}
		case ClassId::Video: {
			return ("Video");
		}
		case ClassId::Widget: {
			return ("Widget");
		}
	}
	return ("");
}
