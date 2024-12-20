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
// UI that shows controls for editing MediaPlaylistWindow content
#ifndef MEDIA_PLAYLIST_UI_H
#define MEDIA_PLAYLIST_UI_H

#include "IntList.h"
#include "WidgetHandle.h"
#include "Ui.h"

class Toolbar;
class HelpWindow;
class CardLabelWindow;
class TextFieldWindow;
class SliderWindow;
class ToggleWindow;
class MediaPlaylistWindow;

class MediaPlaylistUi : public Ui {
public:
	MediaPlaylistUi (MediaPlaylistWindow *playlistPtr);
	~MediaPlaylistUi ();

	// Prefs keys
	static constexpr const char *imageSizeKey = "MediaPlaylistUiA";

	// Read-write data members
	Ui::EventCallbackContext endCallback;

	// Read-only data members
	MediaPlaylistWindow *playlist;
	WidgetHandle<MediaPlaylistWindow> playlistHandle;

	// Superclass override methods
	void setHelpWindowContent (HelpWindow *helpWindow);

protected:
	// Superclass override methods
	StdString getSpritePath ();
	Widget *createBreadcrumbWidget ();
	OpResult doLoad ();
	void doUnload ();
	void doAddMainToolbarItems (Toolbar *toolbar);
	void doAddSecondaryToolbarItems (Toolbar *toolbar);
	void doPause ();
	void doResume ();
	void doResize ();
	void handleDetailImageSizeChange ();

private:
	// Callback functions
	static void doResize_processItems (void *itPtr, Widget *itemWidget);
	static void handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget);
	static void thumbnailCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel);
	static void thumbnailClicked (void *itPtr, Widget *widgetPtr);
	static void thumbnailImageLoaded (void *itPtr, Widget *widgetPtr);
	static void topButtonFocused (void *itPtr, Widget *widgetPtr);
	static void topButtonClicked (void *itPtr, Widget *widgetPtr);
	static void upButtonFocused (void *itPtr, Widget *widgetPtr);
	static void upButtonClicked (void *itPtr, Widget *widgetPtr);
	static void removeButtonFocused (void *itPtr, Widget *widgetPtr);
	static void removeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void selectAllButtonClicked (void *itPtr, Widget *widgetPtr);

	IntList itemIndexList;
	WidgetHandle<Panel> nameTextFieldPanelHandle;
	Panel *nameTextFieldPanel;
	WidgetHandle<TextFieldWindow> nameTextFieldHandle;
	TextFieldWindow *nameTextField;
	WidgetHandle<ToggleWindow> shuffleToggleHandle;
	ToggleWindow *shuffleToggle;
	WidgetHandle<SliderWindow> startPositionSliderHandle;
	SliderWindow *startPositionSlider;
	WidgetHandle<SliderWindow> playDurationSliderHandle;
	SliderWindow *playDurationSlider;
};
#endif
