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
// Widget that shows a horizontal bar and allows other widgets to be added as clickable items
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "Widget.h"
#include "WidgetHandle.h"
#include "Panel.h"

class Sprite;
class Button;
class Menu;

class Toolbar : public Panel {
public:
	Toolbar (double toolbarWidth);
	~Toolbar ();

	// Read-write data members
	Widget::EventCallbackContext modeChangeCallback;

	// Read-only data members
	double barWidth;
	bool isModeActive;
	int modeCount;
	int mode;

	// Reset the toolbar's width
	void setWidth (double toolbarWidth);

	// Return a boolean value indicating if the toolbar contains no items
	bool empty ();

	// Remove all items, modes, and corner widgets from the bar
	void clearAll ();

	// Remove all left side items from the bar
	void clearLeftItems ();

	// Remove all right side items from the bar
	void clearRightItems ();

	// Add the provided widget as an item on the left side of the bar
	void addLeftItem (Widget *itemWidget);

	// Add the provided widget as an item on the right side of the bar
	void addRightItem (Widget *itemWidget);

	// Add a spacer item to the right side of the bar
	void addRightSpacer ();

	// Add the provided widget as the left corner item. If cornerPanel is NULL, clear any left corner widget that might already be present.
	void setLeftCorner (Panel *cornerPanel);

	// Add the provided widget as the right corner item. If cornerPanel is NULL, clear any right corner widget that might already be present.
	void setRightCorner (Panel *cornerPanel);

	// Return true if the toolbar contains a left corner item
	bool hasLeftCorner ();

	// Return true if the toolbar contains a right corner item
	bool hasRightCorner ();

	// Return the width of the toolbar's left item area
	double getLeftWidth ();

	// Set content for the specified mode number and add a mode selection button if not already present
	void setModeContent (int modeNumber, Toolbar *modeToolbar, const StdString &modeName, Sprite *modeIconSprite);

	// Set the sprite and an optional widgetName value for the mode select button
	void setModeButton (Sprite *sprite, const StdString &buttonWidgetName = StdString ());

	// Set the toolbar's active mode
	void setMode (int modeNumber, bool shouldSkipModeChangeCallback = false);

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

private:
	// Callback functions
	static void modeButtonClicked (void *itPtr, Widget *widgetPtr);
	static void modeMenuItemClicked (void *itPtr, Widget *widgetPtr);

	// Remove all items from modeMap
	void clearModes ();

	// Execute the mode button click
	void handleModeButtonClick ();

	// Execute the mode menu item click
	void handleModeMenuItemClick (const StdString &itemName);

	WidgetHandle<Panel> leftItemPanelHandle;
	Panel *leftItemPanel;
	WidgetHandle<Panel> rightItemPanelHandle;
	Panel *rightItemPanel;
	WidgetHandle<Panel> leftCornerPanelHandle;
	Panel *leftCornerPanel;
	WidgetHandle<Panel> rightCornerPanelHandle;
	Panel *rightCornerPanel;
	WidgetHandle<Button> modeButtonHandle;
	Button *modeButton;
	WidgetHandle<Menu> modeMenuHandle;
	Menu *modeMenu;

	struct Mode {
		Toolbar *toolbar;
		Sprite *iconSprite;
		StdString name;
		Mode ():
			toolbar (NULL),
			iconSprite (NULL) { }
	};
	std::map<int, Toolbar::Mode> modeMap;
};
#endif
