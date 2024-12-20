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
// Panel that holds a ScrollView and an attached ScrollBar
#ifndef SCROLL_VIEW_WINDOW_H
#define SCROLL_VIEW_WINDOW_H

#include "Widget.h"
#include "Panel.h"

class ScrollBar;
class ScrollView;

class ScrollViewWindow : public Panel {
public:
	ScrollViewWindow ();
	virtual ~ScrollViewWindow ();

	// Read-only data members
	double scrollViewWidth;
	double scrollViewHeight;
	double scrollViewOriginX;
	double scrollViewOriginY;
	double scrollViewExtentX2;
	double scrollViewExtentY2;
	double scrollViewBottomPadding;

	// Set the size of the viewable area
	void setViewSize (double viewWidth, double viewHeight);

	// Set the layout type and optionally the layoutSpacing value of the viewable area
	void setViewLayout (int layoutType, double layoutSpacingValue = -1.0f);

	// Set the padding scale factor of the viewable area
	void setViewPaddingScale (double widthScale, double heightScale);

	// Set the size of padding space that should be added to the bottom of the viewable area
	void setViewBottomPadding (double paddingSize);

	static constexpr const int KeyboardScrollOption = 0x1;
	static constexpr const int MouseWheelScrollOption = 0x2;
	static constexpr const int ExitedMouseWheelScrollOption = 0x4;
	// Set option values for the window's ScrollView
	void setScrollOptions (int scrollOptions);

	static constexpr const int LeftEdgeScrollBarPosition = 0;
	static constexpr const int LeftInsetScrollBarPosition = 1;
	static constexpr const int RightInsetScrollBarPosition = 2;
	static constexpr const int RightEdgeScrollBarPosition = 3;
	// Set the position of the window's ScrollBar
	void setScrollBarPosition (int positionType, bool enableVerticalMargin = false);

	// Add a ScrollView item and return the same pointer
	Widget *addViewItem (Widget *itemWidget);

	// Set destroyed state for all ScrollView items
	void clearViewItems ();

	// Process all widgets in the window's ScrollView by executing the provided function
	void processViewItems (Widget::EventCallback fn, void *fnData);

	// Set the view's origin position using an optional translate duration
	void setViewScrollOrigin (double positionX, double positionY, int translateDuration = 0);

	static constexpr const int NearestScrollPosition = 0;
	static constexpr const int TopScrollPosition = 1;
	static constexpr const int BottomScrollPosition = 2;
	static constexpr const int CenterScrollPosition = 3;
	// Set the ScrollView origin to show itemWidget's position
	void scrollToItem (Widget *itemWidget, int scrollPositionType = ScrollViewWindow::NearestScrollPosition, int scrollTranslateDuration = 0);

	// Scroll the view to its top extent
	void scrollToTop ();

	// Return a boolean value indicating if the view has scrolled to the bottom extent, within an optional margin height
	bool isScrolledToBottom (double marginHeight = 0.0f);

	// Scroll the view to its bottom extent
	void scrollToBottom ();

	// Superclass override methods
	void reflow ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);

	ScrollView *windowScrollView;
	ScrollBar *windowScrollBar;

private:
	// Callback functions
	static void scrollBarPositionChanged (void *itPtr, Widget *widgetPtr);

	// Reset ScrollView bounds as appropriate for contained items
	void resetScrollBounds ();

	int scrollBarPosition;
	bool isScrollBarVerticalMarginEnabled;
};
#endif
