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
// Panel with controls for browsing file system contents
#ifndef FS_BROWSER_WINDOW_H
#define FS_BROWSER_WINDOW_H

#include "Widget.h"
#include "StringList.h"
#include "Panel.h"

class Image;
class Label;
class Button;
class TextFieldWindow;
class ScrollViewWindow;
class IconLabelWindow;
class FsBrowserWindowItemLabel;

class FsBrowserWindow : public Panel {
public:
	FsBrowserWindow (double windowWidth, double windowHeight, const StdString &initialPath = StdString ());
	~FsBrowserWindow ();

	// sortOrder values
	static constexpr const int NameSort = 0;
	static constexpr const int DirectoriesFirstSort = 1;

	// selectType values
	static constexpr const int SelectFiles = 0;
	static constexpr const int SelectDirectories = 1;
	static constexpr const int SelectFilesAndDirectories = 2;

	// Read-write data members
	Widget::EventCallbackContext closeCallback;
	int sortOrder;
	int selectType;

	// Read-only data members
	double windowWidth;
	double windowHeight;
	bool isPathSelectionConfirmed;
	StdString browsePath;
	StdString selectedPath;

	// Set text to show when the selected path value is empty
	void setSelectPromptText (const StdString &text);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void itemMouseEntered (void *itPtr, Widget *widgetPtr);
	static void itemMouseExited (void *itPtr, Widget *widgetPtr);
	static void itemMouseClicked (void *itPtr, Widget *widgetPtr);
	static void enterTextButtonClicked (void *itPtr, Widget *widgetPtr);
	static void componentButtonClicked (void *itPtr, Widget *widgetPtr);
	static void componentBackButtonClicked (void *itPtr, Widget *widgetPtr);
	static void cancelButtonClicked (void *itPtr, Widget *widgetPtr);
	static void confirmButtonClicked (void *itPtr, Widget *widgetPtr);

	// Start the loadPath task targeting the loadPathTarget value
	void startLoad ();

	// Execute filesystem operations to update load path data
	static void loadPath (void *itPtr);
	void executeLoadPath ();

	// Populate widgets to reflect the latest load path data
	void showLoadState ();

	// Populate widgets to reflect state empty of path data
	void showEmptyState ();

	// Set the selectedPath value
	void setSelectedPath (const StdString &targetPath);

	struct ComponentItem {
		StdString path;
		Button *button;
		ComponentItem ():
			button (NULL) { }
		ComponentItem (const StdString &path, Button *button):
			path (path),
			button (button) { }
	};

	struct FileData {
		StdString name;
		int type;
		FileData (const StdString &name, int type):
			name (name),
			type (type) { }
	};

	int stage;
	StdString selectPromptText;
	bool isLoading;
	bool isFirstLoad;
	bool shouldLoadPath;
	StdString loadPathTarget;
	StdString lastLoadPathTarget;
	OpResult loadPathResult;
	std::list<FsBrowserWindow::FileData> loadItems;
	int pathFieldClock;
	StdString lastPathText;
	Button *enterTextButton;
	TextFieldWindow *pathField;
	ScrollViewWindow *pathView;
	Panel *componentPanel;
	Button *componentBackButton;
	Panel *componentButtonPanel;
	std::list<ComponentItem> components;
	double componentButtonHeight;
	Panel *selectedPathPanel;
	Label *selectedPathLabel1;
	Label *selectedPathLabel2;
	Button *cancelButton;
	Button *confirmButton;
	IconLabelWindow *statusLabel;
};

class FsBrowserWindowItemLabel : public Panel {
public:
	FsBrowserWindowItemLabel (double windowWidth, const StdString &fileName, int fileType);
	~FsBrowserWindowItemLabel ();

	double windowWidth;
	StdString fileName;
	int fileType;
	Image *iconImage;
	Label *nameLabel;
	bool isActive;
	bool isHighlighted;

	// Superclass override methods
	void reflow ();

	// Set label state
	void setActive (bool isActiveValue);
	void setHighlighted (bool isHighlightedValue);
	void resetColors ();
};
#endif
