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
// Panel that contains elements representing a playlist of media items
#ifndef MEDIA_PLAYLIST_WINDOW_H
#define MEDIA_PLAYLIST_WINDOW_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "StringList.h"
#include "MediaPlaylist.h"
#include "SequenceList.h"
#include "Panel.h"

class IntList;
class Image;
class LabelWindow;
class IconLabelWindow;
class Button;
class Toggle;
class ToggleWindow;
class SliderWindow;
class PlayerWindow;
class MediaPlaylistViewWindow;
class MediaPlaylistViewWindowItem;

class MediaPlaylistWindow : public Panel {
public:
	MediaPlaylistWindow ();
	~MediaPlaylistWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaPlaylistWindow *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext renameClickCallback;
	Widget::EventCallbackContext removeClickCallback;
	Widget::EventCallbackContext addItemClickCallback;
	Widget::EventCallbackContext addItemFocusCallback;
	Widget::EventCallbackContext addItemUnfocusCallback;
	Widget::EventCallbackContext optionChangeCallback;
	Widget::EventCallbackContext editClickCallback;
	Widget::EventCallbackContext playClickCallback;
	StdString itemId;

	// Read-only data members
	double windowWidth;
	bool isExpanded;
	bool isExecuting;
	bool isShowingSettings;
	MediaPlaylist playlist;
	PlayerWindow *player;

	// Reset window content to show state from mediaPlaylist
	void read (const MediaPlaylist &mediaPlaylist);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Return the screen extent rectangle for the window's remove button
	Widget::Rectangle getRemoveButtonScreenRect ();

	// Return a newly created settings widget with its value set to match playlist state
	ToggleWindow *createShuffleToggle ();
	SliderWindow *createStartPositionSlider ();
	SliderWindow *createPlayDurationSlider ();

	// Add an item to the window's playlist
	void addItem (const StdString &mediaId, int64_t startTimestamp);
	void addItem (const MediaPlaylistItem &playlistItem);

	// Remove and reorder playlist items as needed to match the provided index list
	void resetItems (const IntList &indexList);

	// Generate a UUID value and write it to playlist.id
	void resetPlaylistId ();

	// Set the playlist name shown by the window
	void setPlaylistName (const StdString &name);

	// Set playlist options
	void setShuffle (bool shuffle);
	void setStartPosition (int startPosition);
	void setPlayDuration (int playDuration);

	// Start a playback sequence of list items targeting the provided PlayerWindow
	void play (PlayerWindow *playerWindow);

	// Skip to the next item in the current playback sequence
	void skipNext ();

	// Skip to the previous item in the current playback sequence
	void skipPrevious ();

	// Stop playlist execution
	void stop ();

	// Execute operations as needed for any active playlist state
	void updatePlay (int msElapsed);

	// Superclass override methods
	void reflow ();

	// Slider value name functions
	static StdString startPositionSliderValueName (double sliderValue);
	static StdString playDurationSliderValueName (double sliderValue);
	static double getStartPositionSliderValueMaxTextWidth ();
	static double getPlayDurationSliderValueNameMaxTextWidth ();
	static void resizeStartPositionSlider (SliderWindow *slider);
	static void resizePlayDurationSlider (SliderWindow *slider);

protected:
	// Superclass override methods
	StdString toStringDetail ();
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static void headerIconClicked (void *itPtr, Widget *widgetPtr);
	static void nameLabelClicked (void *itPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void settingsButtonClicked (void *itPtr, Widget *widgetPtr);
	static void addItemButtonClicked (void *itPtr, Widget *widgetPtr);
	static void addItemButtonFocused (void *itPtr, Widget *widgetPtr);
	static void addItemButtonUnfocused (void *itPtr, Widget *widgetPtr);
	static void editButtonClicked (void *itPtr, Widget *widgetPtr);
	static void skipNextButtonClicked (void *itPtr, Widget *widgetPtr);
	static void skipPreviousButtonClicked (void *itPtr, Widget *widgetPtr);
	static void removeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void shuffleToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void startPositionSliderValueChanged (void *itPtr, Widget *widgetPtr);
	static void playDurationSliderValueChanged (void *itPtr, Widget *widgetPtr);
	static void viewItemClicked (void *itPtr, Widget *widgetPtr);

	// Set the state of the isShowingSettings option
	void setShowingSettings (bool showing);

	// Reset the text shown by the name label, truncating it as needed to fit in its available space
	void resetNameLabel ();

	// Update state as appropriate after a change to item count
	void resetItemCount ();

	// Reset the contents of playItemIds
	void resetPlayItemIds ();

	// End any active playlist execution
	void endPlay ();

	// Task functions
	static void loadRecords (void *itPtr);
	void executeLoadRecords ();

	WidgetHandle<PlayerWindow> playerHandle;
	MediaPlaylistViewWindow *view;
	Image *headerIcon;
	LabelWindow *nameLabel;
	IconLabelWindow *topItemCountLabel;
	IconLabelWindow *bottomItemCountLabel;
	IconLabelWindow *playStatusLabel;
	Toggle *expandToggle;
	Panel *dividerPanel;
	Panel *settingsPanel;
	Panel *bottomLeftPanel;
	Panel *bottomRightPanel;
	Button *removeButton;
	Button *settingsButton;
	Button *addItemButton;
	Button *editButton;
	Button *playButton;
	Button *skipNextButton;
	Button *skipPreviousButton;
	ToggleWindow *shuffleToggle;
	SliderWindow *startPositionSlider;
	SliderWindow *playDurationSlider;
	SDL_mutex *loadMutex;
	StringList mediaItemIds;
	StringList loadIds;
	bool isLoadingRecords;
	SequenceList<int> playItemIndexes;
	int currentPlayItemIndex;
	int64_t currentPlayDuration;
	int nextPlayIndex;
	bool shouldResetPlayItemIds;
	bool shouldPlayNext;
	bool shouldPlayNextReverse;
	bool shouldScrollOnPlay;
};
#endif
