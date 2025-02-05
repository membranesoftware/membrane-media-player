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
// Card view item that shows app info
#ifndef APP_CARD_WINDOW_H
#define APP_CARD_WINDOW_H

#include "Widget.h"
#include "IntList.h"
#include "StringList.h"
#include "AppNews.h"
#include "Panel.h"

class Image;
class Label;
class LabelWindow;
class Button;
class Toggle;
class TextFlow;
class HyperlinkWindow;
class SharedBuffer;

class AppCardWindow : public Panel {
public:
	AppCardWindow ();
	~AppCardWindow ();

	// Read-write data members
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext layoutChangeCallback;
	StdString itemId;

	// Read-only data members
	double windowWidth;
	double unexpandedTextWidth;
	bool isExpanded;
	bool isInitializing;

	// Begin operations to read initial state and execute a check for updates if requested
	void initialize (bool showNewsPostsFirst, bool executeCheckForUpdates);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Execute a check for updates operation
	void checkForUpdates ();

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doResize ();

private:
	// Callback functions
	static void expandToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void updateButtonClicked (void *itPtr, Widget *widgetPtr);
	static void aboutButtonClicked (void *itPtr, Widget *widgetPtr);
	static void nextButtonClicked (void *itPtr, Widget *widgetPtr);
	static void updateRequestComplete (void *itPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void updateLinkOpened (void *itPtr, Widget *widgetPtr);

	// Populate info text content
	void resetInfoText ();

	// Advance info text to the next item and update widget content
	void setNextInfoText ();

	// Reset visible state for update row widgets
	void resetUpdateVisible ();

	// Execute initialize operations to read news state from a stored AppNews record and execute a check for updates if requested
	static void awaitAppNewsReady (void *itPtr);
	void executeAwaitAppNewsReady ();
	static void readAppNewsRecord (void *itPtr);
	void executeReadAppNewsRecord ();
	static void showLoadResult (void *itPtr);
	void executeShowLoadResult ();
	void endInitialize ();

	// Process response data from a check for updates operation
	void receiveUpdateResponse (int statusCode, SharedBuffer *responseData);
	static void showUpdateResult (void *itPtr);
	void executeShowUpdateResult ();

	// Set window state using fields from AppNews data. If showNewsPosts is true, set info text state to news post content if available.
	void readNewsState (AppNews::NewsState *state);

	int currentTextId;
	int nextTextId;
	IntList infoTextIds;
	bool isTextCrawlEnabled;
	bool isShowingUpdateRow;
	bool isShowingUpdateLink;
	bool isCheckingForUpdates;
	bool isUpdateNewsReceived;
	bool isLoadNewsReceived;
	bool isShowingNewsPosts;
	int currentNewsPostId;
	bool isInitializeShowingNewsPostsFirst;
	bool isInitializeCheckingForUpdates;
	AppNews::NewsState *loadNewsState;
	AppNews::NewsState *updateNewsState;
	StringList newsPosts;
	SDL_mutex *newsPostMutex;
	Image *headerIcon;
	Label *nameLabel;
	Panel *progressRingPanel;
	LabelWindow *unexpandedTextLabel;
	Toggle *expandToggle;
	Panel *dividerPanel;
	Image *versionIcon;
	TextFlow *versionText;
	Image *updateIcon;
	ProgressBar *updateProgressBar;
	TextFlow *updateText;
	HyperlinkWindow *updateLink;
	Image *headerUpdateIcon;
	Image *infoIcon;
	TextFlow *infoText;
	Button *updateButton;
	Button *aboutButton;
	Button *nextButton;
};
#endif
