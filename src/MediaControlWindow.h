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
// Panel that shows controls and status for the MediaControl instance
#ifndef MEDIA_CONTROL_WINDOW_H
#define MEDIA_CONTROL_WINDOW_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "MediaControl.h"
#include "Panel.h"

class Sprite;
class Image;
class Label;
class Button;
class Toggle;
class ProgressBar;
class TextFlow;
class ListView;
class IconLabelWindow;
class ActionWindow;
class StatsWindow;
class SliderWindow;
class MediaControlPrimeWindow;
class MediaControlMainWindow;
class MediaControlScanWindow;
class MediaControlCleanWindow;
class MediaControlConfigureWindow;
class MediaControlTaskStatusWindow;

class MediaControlWindow : public Panel {
public:
	MediaControlWindow ();
	~MediaControlWindow ();

	// Read-write data members
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext layoutChangeCallback;
	StdString itemId;

	// Read-only data members
	double windowWidth;
	bool isExpanded;

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static void expandToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void controlWindowLayoutChanged (void *itPtr, Widget *widgetPtr);
	static void controlWindowBackClicked (void *itPtr, Widget *widgetPtr);
	static void primeWindowConfigureComplete (void *itPtr, Widget *widgetPtr);
	static void mainWindowScanButtonClicked (void *itPtr, Widget *widgetPtr);
	static void mainWindowCleanButtonClicked (void *itPtr, Widget *widgetPtr);
	static void mainWindowConfigureButtonClicked (void *itPtr, Widget *widgetPtr);

	// Set the stage value and populate appropriate widgets
	void setPrime ();
	void setMainControl ();
	void setScanControl ();
	void setCleanControl ();
	void setConfigureControl ();

	// Set the stage value and clear stage widgets
	void setNextStage (int stageValue);

	// Reset the isVisible value for all conditional widgets
	void resetWidgetsVisible ();

	int stage;
	MediaControl::Status mediaControlStatus;
	Image *headerIcon;
	Label *nameLabel;
	Toggle *expandToggle;
	Panel *progressRingPanel;
	Panel *dividerPanel;
	IconLabelWindow *mediaCountIcon;
	WidgetHandle<MediaControlPrimeWindow> primeWindowHandle;
	MediaControlPrimeWindow *primeWindow;
	WidgetHandle<MediaControlMainWindow> mainWindowHandle;
	MediaControlMainWindow *mainWindow;
	WidgetHandle<MediaControlScanWindow> scanWindowHandle;
	MediaControlScanWindow *scanWindow;
	WidgetHandle<MediaControlCleanWindow> cleanWindowHandle;
	MediaControlCleanWindow *cleanWindow;
	WidgetHandle<MediaControlConfigureWindow> configureWindowHandle;
	MediaControlConfigureWindow *configureWindow;
};

class MediaControlPrimeWindow : public Panel {
public:
	MediaControlPrimeWindow (double windowWidth);
	~MediaControlPrimeWindow ();

	Widget::EventCallbackContext layoutChangeCallback;
	Widget::EventCallbackContext configureCompleteCallback;
	double windowWidth;
	bool isConfiguring;
	IconLabelWindow *statusIcon;
	ProgressBar *progressBar;
	TextFlow *promptText;
	ActionWindow *actionWindow;
	Button *nextButton;
	Button *backButton;
	Button *confirmButton;

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static void nextButtonClicked (void *itPtr, Widget *widgetPtr);
	static void backButtonClicked (void *itPtr, Widget *widgetPtr);
	static void confirmButtonClicked (void *itPtr, Widget *widgetPtr);
	static void actionWindowOptionChanged (void *itPtr, Widget *widgetPtr);
};

class MediaControlMainWindow : public Panel {
public:
	MediaControlMainWindow (double windowWidth);
	~MediaControlMainWindow ();

	Widget::EventCallbackContext layoutChangeCallback;
	Widget::EventCallbackContext scanClickCallback;
	Widget::EventCallbackContext cleanClickCallback;
	Widget::EventCallbackContext configureClickCallback;
	double windowWidth;
	MediaControl::Status mediaControlStatus;
	StatsWindow *statsWindow;
	Button *scanButton;
	Button *cleanButton;
	Button *configureButton;
	int64_t mediaSizeTotal;
	int64_t mediaDurationTotal;
	bool isLoadingMetadata;
	bool isLoadMetadataComplete;

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void scanButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cleanButtonClicked (void *itPtr, Widget *widgetPtr);
	static void configureButtonClicked (void *itPtr, Widget *widgetPtr);

	// Task functions
	static void loadMetadata (void *itPtr);
};

class MediaControlScanWindow : public Panel {
public:
	MediaControlScanWindow (double windowWidth);
	~MediaControlScanWindow ();

	Widget::EventCallbackContext layoutChangeCallback;
	Widget::EventCallbackContext backClickCallback;
	double windowWidth;
	MediaControl::Status mediaControlStatus;
	Label *titleLabel;
	MediaControlTaskStatusWindow *taskWindow;
	Button *backButton;
	Button *executeButton;
	Button *cancelButton;
	Button *clearButton;

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static void backButtonClicked (void *itPtr, Widget *widgetPtr);
	static void executeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *itPtr, Widget *widgetPtr);
	static void clearButtonClicked (void *itPtr, Widget *widgetPtr);
};

class MediaControlCleanWindow : public Panel {
public:
	MediaControlCleanWindow (double windowWidth);
	~MediaControlCleanWindow ();

	Widget::EventCallbackContext layoutChangeCallback;
	Widget::EventCallbackContext backClickCallback;
	double windowWidth;
	MediaControl::Status mediaControlStatus;
	Label *titleLabel;
	MediaControlTaskStatusWindow *taskWindow;
	Button *backButton;
	Button *executeButton;
	Button *cancelButton;
	Button *clearButton;

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static void backButtonClicked (void *itPtr, Widget *widgetPtr);
	static void executeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *itPtr, Widget *widgetPtr);
	static void clearButtonClicked (void *itPtr, Widget *widgetPtr);
};

class MediaControlConfigureWindow : public Panel {
public:
	MediaControlConfigureWindow (double windowWidth);
	~MediaControlConfigureWindow ();

	Widget::EventCallbackContext layoutChangeCallback;
	Widget::EventCallbackContext backClickCallback;
	double windowWidth;
	double optionWidth;
	int stage;
	bool isConfiguring;
	Label *titleLabel;
	Image *promptIcon;
	TextFlow *promptText;
	Label *mediaSourcePathLabel;
	ListView *mediaSourcePathListView;
	Button *mediaSourcePathAddButton;
	Label *dataPathLabel;
	TextFieldWindow *dataPathTextField;
	WidgetHandle<Panel> fsBrowserPanelHandle;
	Panel *fsBrowserPanel;
	Label *mediaThumbnailCountLabel;
	SliderWindow *mediaThumbnailCountSlider;
	Button *backButton;
	Button *executeButton;

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doResize ();

private:
	// Callback functions
	static StdString mediaThumbnailCountSliderValueName (double sliderValue);
	static void mediaSourcePathAddButtonClicked (void *itPtr, Widget *widgetPtr);
	static void fsBrowserWindowClosed (void *itPtr, Widget *widgetPtr);
	static void backButtonClicked (void *itPtr, Widget *widgetPtr);
	static void executeButtonClicked (void *itPtr, Widget *widgetPtr);

	// Return true if configured settings are valid for use with MediaControl
	bool isConfigurationValid ();
};

class MediaControlTaskStatusWindow : public Panel {
public:
	MediaControlTaskStatusWindow (int targetTaskType, double windowWidth, const StdString &promptTextValue);
	~MediaControlTaskStatusWindow ();

	int targetTaskType;
	double windowWidth;
	TextFlow *promptText;
	StdString taskIconSpriteId;
	Image *taskIcon;
	Label *taskText1Label;
	Label *taskText2Label;
	ProgressBar *taskProgressBar;

	// Update window state with MediaControl status fields
	void updateStatus (const MediaControl::Status &status, const MediaControl::TaskResult &taskResult);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doResize ();
};
#endif
