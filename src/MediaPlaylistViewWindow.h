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
// ScrollView that shows MediaPlaylistWindow items
#ifndef MEDIA_PLAYLIST_VIEW_WINDOW_H
#define MEDIA_PLAYLIST_VIEW_WINDOW_H

#include "MediaItem.h"
#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class MediaPlaylistItem;
class LabelWindow;
class ImageWindow;
class ScrollViewWindow;
class MediaPlaylistViewWindowItem;
class MediaPlaylistViewWindowItemLabel;

class MediaPlaylistViewWindow : public Panel {
public:
	MediaPlaylistViewWindow (double windowWidth, double windowHeight);
	~MediaPlaylistViewWindow ();

	// Read-write data members
	Widget::EventCallbackContext itemClickCallback;

	// Read-only data members
	int clickItemIndex;

	// Set the window's size
	void setWindowSize (double windowWidth, double windowHeight);

	// Remove all view items
	void clearItems ();

	// Add an item to the window's playlist
	void addItem (const MediaPlaylistItem &playlistItem);

	// Set the list position value for the item that should be shown in an active state, or clear any active item if itemIndex is negative
	void setActiveItem (int itemIndex);

	// Move the scroll origin to view the specified item
	void scrollToItem (int itemIndex);

	// Superclass override methods
	void reflow ();
	void syncRecordStore ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void thumbnailImageLongPressed (void *itPtr, Widget *widgetPtr);
	static void thumbnailImageLoaded (void *itPtr, Widget *widgetPtr);
	static void itemNameLabelClicked (void *itPtr, Widget *widgetPtr);
	static void itemNameLabelMouseEntered (void *itPtr, Widget *widgetPtr);
	static void itemNameLabelMouseExited (void *itPtr, Widget *widgetPtr);

	// Return a newly created widget for use in an item entry
	ImageWindow *createThumbnailImage ();
	MediaPlaylistViewWindowItemLabel *createNameLabel ();

	double itemWidth;
	Panel *thumbnailPanel;
	ScrollViewWindow *scrollWindow;
	std::vector<MediaPlaylistViewWindowItem *> itemList;
	SDL_mutex *itemListMutex;
	Position thumbnailPanelViewOrigin;
	WidgetHandle<ImageWindow> highlightedThumbnailImageHandle;
	ImageWindow *highlightedThumbnailImage;
};

class MediaPlaylistViewWindowItem {
public:
	MediaPlaylistViewWindowItem ();
	~MediaPlaylistViewWindowItem ();

	// Set the item's list position value
	void setListPosition (int position);

	int listPosition;
	bool isLoaded;
	StdString mediaId;
	MediaItem mediaItem;
	StdString mediaName;
	int64_t startTimestamp;
	int thumbnailWidth;
	int thumbnailHeight;
	WidgetHandle<MediaPlaylistViewWindowItemLabel> itemLabelHandle;
	MediaPlaylistViewWindowItemLabel *itemLabel;
	WidgetHandle<ImageWindow> thumbnailImageHandle;
	ImageWindow *thumbnailImage;
};

class MediaPlaylistViewWindowItemLabel : public Panel {
public:
	MediaPlaylistViewWindowItemLabel (double windowWidth);
	~MediaPlaylistViewWindowItemLabel ();

	// Set the label's window width value
	void setWindowWidth (double widthValue);

	// Set the label's start timestamp text
	void setStartTimestamp (int64_t startTimestamp, int64_t streamDuration);

	// Set the label's active state
	void setActive (bool active);

	// Set the label's highlighted state
	void setHighlighted (bool highlighted);

	// Reset the panel's colors as appropriate for its content and configuration
	void resetColors ();

	// Superclass override methods
	void reflow ();

	// Read-write data members
	int itemIndex;
	double statusLabelWidth;
	double nameLabelWidth;
	double startPositionLabelWidth;
	bool isErrorStatus;

	// Read-only data members
	double windowWidth;
	bool isActive;
	bool isHighlighted;
	LabelWindow *statusLabel;
	LabelWindow *nameLabel;
	LabelWindow *startPositionLabel;
};
#endif
