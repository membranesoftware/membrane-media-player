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
// UI that shows details and controls for a media item
#ifndef MEDIA_ITEM_UI_H
#define MEDIA_ITEM_UI_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "MediaItem.h"
#include "StringList.h"
#include "Ui.h"

class Json;
class Button;
class Panel;
class Toolbar;
class HelpWindow;
class ImageWindow;
class TextCardWindow;
class IconLabelWindow;
class CardLabelWindow;
class MediaItemWindow;
class MediaThumbnailWindow;
class MediaItemTagWindow;
class PlayerTimelineWindow;

class MediaItemUi : public Ui {
public:
	MediaItemUi (MediaItemWindow *mediaItemWindow);
	~MediaItemUi ();

	// Prefs keys
	static constexpr const char *tagWindowExpandedKey = "MediaItemUiA";
	static constexpr const char *imageSizeKey = "MediaItemUiB";

	// Read-write data members
	Ui::EventCallbackContext endCallback;

	// Read-only data members
	int64_t playTimestamp;
	int64_t selectPlayPositionTimestamp;
	StdString mediaId;
	MediaItem mediaItem;

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
	void doResume ();
	void doPause ();
	static void doPause_processItems (void *markerPtr, Widget *itemWidget);
	void doUpdate (int msElapsed);
	void doResize ();
	void handleDetailImageSizeChange ();
	static void handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget);
	Widget *findLuaTargetWidget (const char *targetName);
	void executeLuaTarget (Widget *targetWidget);
	void executeLuaUntarget (Widget *targetWidget);

private:
	// Callback functions
	static void doResize_processItems (void *itPtr, Widget *itemWidget);
	static void thumbnailMouseEntered (void *itPtr, Widget *widgetPtr);
	static void thumbnailMouseExited (void *itPtr, Widget *widgetPtr);
	static void thumbnailClicked (void *itPtr, Widget *widgetPtr);
	static void thumbnailSelectStateChanged (void *itPtr, Widget *widgetPtr);
	static void thumbnailVideoFrameLoaded (void *itPtr, Widget *widgetPtr);
	static void thumbnailCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel);
	static void markerDeleteClicked (void *itPtr, Widget *widgetPtr);
	static void markerPlayClicked (void *itPtr, Widget *widgetPtr);
	static void timelinePositionHovered (void *itPtr, Widget *widgetPtr);
	static void timelinePositionClicked (void *itPtr, Widget *widgetPtr);
	static void viewBeforeButtonFocused (void *itPtr, Widget *widgetPtr);
	static void viewBeforeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void viewAfterButtonFocused (void *itPtr, Widget *widgetPtr);
	static void viewAfterButtonClicked (void *itPtr, Widget *widgetPtr);
	static void addMarkerButtonFocused (void *itPtr, Widget *widgetPtr);
	static void addMarkerButtonClicked (void *itPtr, Widget *widgetPtr);
	static void selectPlayPositionButtonFocused (void *itPtr, Widget *widgetPtr);
	static void selectPlayPositionButtonClicked (void *itPtr, Widget *widgetPtr);
	static void playButtonFocused (void *itPtr, Widget *widgetPtr);
	static void playButtonClicked (void *itPtr, Widget *widgetPtr);
	static void tagWindowExpandStateChanged (void *itPtr, Widget *widgetPtr);
	static void tagWindowAddClicked (void *itPtr, Widget *widgetPtr);
	static void tagWindowItemDeleted (void *itPtr, Widget *widgetPtr);
	static void addTagActionClosed (void *itPtr, Widget *widgetPtr);
	static void removeTagActionClosed (void *itPtr, Widget *widgetPtr);

	struct TagTask {
		MediaItemUi *ui;
		StdString tag;
		TagTask ():
			ui (NULL) { }
	};
	// Task functions
	static void addMediaItemTag (void *taskPtr);
	void executeAddMediaItemTag (const StdString &tag);
	static void removeMediaItemTag (void *taskPtr);
	void executeRemoveMediaItemTag (const StdString &tag);

	// Write the source MediaItem record to the RecordStore
	void storeRecord ();

	// Return a newly created MediaThumbnailWindow for use as a card view item
	MediaThumbnailWindow *createFrameThumbnailWindow ();
	MediaThumbnailWindow *createMarkerThumbnailWindow ();

	// Return an itemId value for use with card view items of a specified timestamp value
	StdString getFrameThumbnailItemId (int64_t timestamp);
	StdString getMarkerThumbnailItemId (int64_t timestamp);

	// Return the string that should be added to thumbnailCardTimestamps for the specified timestamp value
	StdString getTimestampString (int64_t timestamp);

	WidgetHandle<MediaItemTagWindow> tagWindowHandle;
	MediaItemTagWindow *tagWindow;
	WidgetHandle<PlayerTimelineWindow> timelineWindowHandle;
	PlayerTimelineWindow *timelineWindow;
	WidgetHandle<MediaThumbnailWindow> selectedThumbnailHandle;
	MediaThumbnailWindow *selectedThumbnail;
	WidgetHandle<ImageWindow> timelinePopupImageWindowHandle;
	ImageWindow *timelinePopupImageWindow;
	std::list<MediaThumbnailWindow *> loadThumbnails;
	SDL_mutex *loadThumbnailsMutex;
	StringList thumbnailCardTimestamps;
	double timelineHoverPosition;
	int64_t timelineHoverTimestamp;
	int timelineHoverClock;
	bool shouldLoadMarkers;
	bool isTagWindowExpanded;
	bool shouldEnableTagWindow;
	StdString lastRemoveTagValue;
};
#endif
