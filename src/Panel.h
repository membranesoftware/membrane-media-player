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
// Widget that can hold other widgets
#ifndef PANEL_H
#define PANEL_H

#include <stdint.h>
#include "StringList.h"
#include "Color.h"
#include "Position.h"
#include "WidgetHandle.h"
#include "Widget.h"

class Button;
class CardView;
class ComboBox;
class IconLabelWindow;
class Image;
class ImageWindow;
class Label;
class LabelWindow;
class ListView;
class ProgressBar;
class ProgressRing;
class ScrollBar;
class ScrollView;
class Slider;
class SliderWindow;
class TextArea;
class TextField;
class TextFieldWindow;
class TextFlow;
class Toggle;
class ToggleWindow;

class PanelLayoutFlow {
public:
	PanelLayoutFlow ();
	~PanelLayoutFlow ();

	double x;
	double y;
	double xExtent;
	double yExtent;
};

class Panel : public Widget {
public:
	Panel ();
	virtual ~Panel ();

	// Read-write data members
	Color bgColor;
	Color borderColor;
	Color dropShadowColor;
	double layoutSpacing;
	bool isDrawClipEnabled;

	// Read-only data members
	double extentX1;
	double extentY1;
	double extentX2;
	double extentY2;
	int maxWidgetZLevel;
	double viewOriginX;
	double viewOriginY;
	bool isViewOriginBoundEnabled;
	double minViewOriginX;
	double minViewOriginY;
	double maxViewOriginX;
	double maxViewOriginY;
	double widthPadding;
	double heightPadding;
	double widthPaddingScale;
	double heightPaddingScale;
	bool isFixedPadding;
	bool isFilledBg;
	int topLeftCornerRadius;
	int topRightCornerRadius;
	int bottomLeftCornerRadius;
	int bottomRightCornerRadius;
	bool isBordered;
	double borderWidth;
	bool isDropShadowed;
	double dropShadowWidth;
	bool isFixedSize;
	bool isWaiting;
	int layout;
	int detailSize;
	double detailMaxWidth;

	// Set the panel's fill bg option. If enabled, the panel is drawn with a background fill using the specified color.
	void setFillBg (bool enable, const Color &color = Color ());

	// Set the panel's corner radius values. If the panel's fill bg option is enabled, each fill rectangle corner with a nonzero radius value is drawn with a rounded shape.
	void setCornerRadius (int radius);
	void setCornerRadius (int topLeftRadius, int topRightRadius, int bottomLeftRadius, int bottomRightRadius);

	// Set the panel's border option. If enabled, the panel is drawn with a border using the specified color and width.
	void setBorder (bool enable, const Color &color = Color (), double borderWidthValue = 1.0f);

	// Set the panel's drop shadow option. If enabled, the panel is drawn with a drop shadow effect using the specified color and width.
	void setDropShadow (bool enable, const Color &color = Color (), double dropShadowWidthValue = 1.0f);

	static constexpr const int LeftFlowLayoutOption = 0x1;
	static constexpr const int RightFlowLayoutOption = 0x2;
	static constexpr const int UpFlowLayoutOption = 0x4;
	static constexpr const int DownFlowLayoutOption = 0x8;
	static constexpr const int LeftGravityLayoutOption = 0x10;
	static constexpr const int RightGravityLayoutOption = 0x20;
	static constexpr const int UpGravityLayoutOption = 0x40;
	static constexpr const int DownGravityLayoutOption = 0x80;
	static constexpr const int HorizontalCenterLayoutOption = 0x100;
	static constexpr const int VerticalCenterLayoutOption = 0x200;
	static constexpr const int NoLayout = 0;
	// Set the layout type that should be used to arrange the panel's widgets
	virtual void setLayout (int layoutType);

	// Reset the panel's widget layout as appropriate for its content and configuration
	virtual void reflow ();

	// Remove all widgets from the panel and mark them as destroyed
	void clear ();

	// Add a widget to the panel and return the same pointer
	Widget *addWidget (Widget *widget);
	Widget *addWidget (Widget *widget, const Position &position);
	Widget *addWidget (Widget *widget, int zLevel);
	Widget *addWidget (Widget *widget, const Position &position, int zLevel);

	// Add a widget subclass object to the panel and return the same pointer
	Button *add (Button *widget, int zLevel = 0);
	CardView *add (CardView *widget, int zLevel = 0);
	ComboBox *add (ComboBox *widget, int zLevel = 0);
	IconLabelWindow *add (IconLabelWindow *widget, int zLevel = 0);
	Image *add (Image *widget, int zLevel = 0);
	ImageWindow *add (ImageWindow *widget, int zLevel = 0);
	Label *add (Label *widget, int zLevel = 0);
	LabelWindow *add (LabelWindow *widget, int zLevel = 0);
	ListView *add (ListView *widget, int zLevel = 0);
	Panel *add (Panel *widget, int zLevel = 0);
	ProgressBar *add (ProgressBar *widget, int zLevel = 0);
	ProgressRing *add (ProgressRing *widget, int zLevel = 0);
	ScrollBar *add (ScrollBar *widget, int zLevel = 0);
	ScrollView *add (ScrollView *widget, int zLevel = 0);
	Slider *add (Slider *widget, int zLevel = 0);
	SliderWindow *add (SliderWindow *widget, int zLevel = 0);
	TextArea *add (TextArea *widget, int zLevel = 0);
	TextField *add (TextField *widget, int zLevel = 0);
	TextFieldWindow *add (TextFieldWindow *widget, int zLevel = 0);
	TextFlow *add (TextFlow *widget, int zLevel = 0);
	Toggle *add (Toggle *widget, int zLevel = 0);
	ToggleWindow *add (ToggleWindow *widget, int zLevel = 0);

	// Remove the specified widget from the panel
	void removeWidget (Widget *targetWidget);

	// Return the topmost child widget at the specified screen position, or NULL if no such widget was found. If requireMouseHoverEnabled is true, return a widget only if it has enabled the isMouseHoverEnabled option.
	Widget *findWidget (double screenPositionX, double screenPositionY, bool requireMouseHoverEnabled = false);

	// Return the child widget that holds a matching widgetName value, or NULL if no such widget was found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	Widget *findWidget (const StdString &widgetName, bool shouldRetain = false);

	// Return the child widget matching the provided function, or NULL if no such widget was found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	Widget *findWidget (Widget::FindMatchFunction fn, void *fnData, bool shouldRetain = false);

	// Process all widgets in the panel by executing the provided function
	void processWidgets (Widget::EventCallback fn, void *fnData);

	// Find all child widgets with a non-empty widgetName value and append the names to destList
	void getWidgetNames (StringList *destList);

	// Set the panel's view origin coordinates
	void setViewOrigin (double originX, double originY);

	// Set the minimum and maximum bounds for view origin coordinates
	void setViewOriginBounds (double originX1, double originY1, double originX2, double originY2);

	// Set the fixed padding option. If enabled, the panel uses the specified padding size values instead of choosing its own padding size.
	void setFixedPadding (bool enable, double widthPaddingSize = 0.0f, double heightPaddingSize = 0.0f);

	// Set a scale factor to apply when computing padding sizes
	void setPaddingScale (double widthScale, double heightScale);

	// Set the fixed size option. If enabled, the panel uses the specified width and height values instead of dynamic resizing to fit contained elements.
	void setFixedSize (bool enable, double fixedWidth = 0.0f, double fixedHeight = 0.0f);

	// Set the waiting option. If enabled, the panel disables input, shades it content, and shows a progress bar widget.
	void setWaiting (bool enable);

	// Update widget state to reflect the latest input events
	void processInput ();

	// Update widget state as appropriate for records present in the application's RecordStore object
	virtual void syncRecordStore ();

	// Set the size type for detail scaling of the panel's contents
	void setDetailSize (int detailSizeValue, double detailMaxWidthValue);

	// Return the scale for use with the current detailSize value
	double getDetailThumbnailScale ();

	// Execute subclass-specific operations appropriate when the detailSize value has changed
	virtual void refreshDetailSize ();

protected:
	// Superclass override methods
	virtual void doUpdate (int msElapsed);
	virtual void doDraw (double originX, double originY);
	virtual void doResize ();
	virtual void doResetInputState ();
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);
	virtual bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Reset the panel's extent coordinate values as appropriate for its content
	void resetExtents ();

	// Reset the panel's width and height values as appropriate for its extents and configuration
	void resetSize ();

	// Reset the panel's padding size as appropriate for its configuration
	void resetPadding ();

	// Reset layout flow state to the top left position
	void topLeftLayoutFlow ();

	// Reset layout flow state to the bottom right position
	void bottomRightLayoutFlow ();

	// Move layout flow position down to the next line
	void nextRowLayoutFlow ();

	// Move layout flow position right to the next column
	void nextColumnLayoutFlow ();

	// Execute actions appropriate after a change to view extent coordinates
	virtual void doSetExtents ();

	// Execute actions appropriate after a change to view origin coordinates
	virtual void doSetViewOrigin ();

	// Check if the widget list is correctly sorted for drawing by z-level, and sort the list if not. This method must only be invoked while holding a lock on widgetListMutex.
	void sortWidgetList ();

	bool isMouseInputStarted;
	int lastMouseLeftUpCount;
	int lastMouseLeftDownCount;
	int lastMouseRightUpCount;
	int lastMouseRightDownCount;
	int lastMouseWheelUpCount;
	int lastMouseWheelDownCount;
	int lastMouseDownX;
	int lastMouseDownY;
	int64_t lastMouseDownTime;
	int cornerCenterDx;
	int cornerCenterDy;
	int cornerCenterDw;
	int cornerCenterDh;
	int cornerTopDx;
	int cornerTopDy;
	int cornerTopDw;
	int cornerTopDh;
	int cornerLeftDx;
	int cornerLeftDy;
	int cornerLeftDw;
	int cornerLeftDh;
	int cornerRightDx;
	int cornerRightDy;
	int cornerRightDw;
	int cornerRightDh;
	int cornerBottomDx;
	int cornerBottomDy;
	int cornerBottomDw;
	int cornerBottomDh;
	int cornerSize;
	PanelLayoutFlow layoutFlow;
	SDL_mutex *widgetListMutex;
	std::list<Widget *> widgetList;
	SDL_mutex *widgetAddListMutex;
	std::list<Widget *> widgetAddList;
	Panel *waitPanel;
	WidgetHandle<Panel> waitPanelHandle;
	ProgressBar *waitProgressBar;
	WidgetHandle<ProgressBar> waitProgressBarHandle;
};
#endif
