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
// Panel that shows controls for adding and removing media item tags
#ifndef MEDIA_ITEM_TAG_WINDOW_H
#define MEDIA_ITEM_TAG_WINDOW_H

#include "Widget.h"
#include "StringList.h"
#include "Panel.h"

class Image;
class Label;
class Toggle;
class ListView;
class Button;

class MediaItemTagWindow : public Panel {
public:
	MediaItemTagWindow (const StringList &tagValues);
	~MediaItemTagWindow ();

	// Read-write data members
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext addClickCallback;
	Widget::EventCallbackContext itemDeleteCallback;
	StdString itemId;

	// Read-only data members
	StringList tags;
	double windowWidth;
	bool isExpanded;
	StdString itemDeleteTag;

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Set the window's disabled state
	void setDisabled (bool disabled);

	// Set the window's tag items
	void setTags (const StringList &tagValues);

	// Return the screen extent rectangle for the window's add button
	Widget::Rectangle getAddButtonScreenRect ();

	// Return the screen extent rectangle for the window's delete button
	Widget::Rectangle getDeleteButtonScreenRect ();

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doResize ();

private:
	// Callback functions
	static void expandToggleStateChanged (void *itPtr, Widget *widgetPtr);
	static void addButtonClicked (void *itPtr, Widget *widgetPtr);
	static void listViewItemDeleted (void *itPtr, Widget *widgetPtr);

	// Reset tagTextLabel content
	void resetTagText ();

	Image *headerIcon;
	Label *nameLabel;
	Label *tagTextLabel;
	Toggle *expandToggle;
	Panel *dividerPanel;
	ListView *tagListView;
	Button *addButton;
};
#endif
