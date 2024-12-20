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
// ScrollView that arranges widgets as rows of cards
#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include "HashMap.h"
#include "ScrollView.h"

class ScrollBar;
class CardLabelWindow;

class CardView : public ScrollView {
public:
	CardView (double viewWidth, double viewHeight);
	~CardView ();

	typedef bool (*MatchFunction) (void *data, Widget *itemWidget);
	typedef void (*ItemLabelFunction) (void *data, Widget *itemWidget, CardLabelWindow *cardLabel);

	static constexpr const double reducedSizeItemScale = 0.83f;

	// Read-write data members
	bool shouldSortItemList;

	// Read-only data members
	double cardAreaWidth;
	double cardAreaBottomPadding;
	double itemMarginSize;
	int rowCount;
	StdString highlightedItemId;

	// Set the size of the viewable area
	void setViewSize (double viewWidth, double viewHeight);

	// Set the size of padding space that should be added to the bottom of the view
	void setBottomPadding (double paddingSize);

	// Set the size of margin space that should be inserted between items in the view
	void setItemMarginSize (double marginSize);

	// Set the number of item rows in the view
	void setRowCount (int count);

	// Set a panel that should be shown as a header widget, immediately preceding items in the specified row number
	void setRowHeader (int rowNumber, Panel *headerPanel);

	// Set the margin size that should be used for items in the specified row, overriding any default item margin size that might have been set
	void setRowItemMarginSize (int rowNumber, double marginSize);

	// Set the reverse sort option for the specified row. If enabled, the view sorts items from that row in descending order.
	void setRowReverseSorted (int rowNumber, bool enable);

	// Set the animated selection option for the specified row. If enabled, the view uses scaling effects to animate selection of items in the row.
	void setRowSelectionAnimated (int rowNumber, bool enable);

	// Set the animated reposition option for the specified row. If enabled, the view uses translation effects to animate position changes for items in the row.
	void setRowRepositionAnimated (int rowNumber, bool enable);

	// Set the labeled option for the specified row. If enabled, the view shows a label widget with each item.
	void setRowLabeled (int rowNumber, bool enable, CardView::ItemLabelFunction labelFn = NULL, void *labelFnData = NULL);

	// Return a boolean value indicating if the card view contains no items
	bool empty ();

	// Return a boolean value indicating if the specified item exists in the view
	bool contains (const StdString &itemId);
	bool contains (const char *itemId);

	// Return a string value, suitable for use as a new item ID in the view
	StdString getAvailableItemId ();

	// Add an item to the view and return the Widget pointer that was added. If no itemId value is provided, the CardView generates one of its own. After adding the item, invoke reflow unless shouldSkipReflow is true.
	Widget *addItem (Panel *itemPanel, const StdString &itemId = StdString (), int rowNumber = 0, bool shouldSkipReflow = false);
	Widget *addItem (Panel *itemPanel, int rowNumber, bool shouldSkipReflow = false);

	// Return a pointer to the item widget with the specified ID, or NULL if the item wasn't found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	Widget *getItem (const StdString &itemId, bool shouldRetain = false);
	Widget *getItem (const char *itemId, bool shouldRetain = false);

	// Return the number of items in the view
	int getItemCount ();

	// Return the number of items in the specified row
	int getRowItemCount (int rowNumber);

	// Return a pointer to the first item widget reported matching by the provided function, or NULL if the item wasn't found. If shouldRetain is true, retain any matched widget before returning it and the caller is responsible for releasing it.
	Widget *findItem (CardView::MatchFunction fn, void *fnData, bool shouldRetain = false);

	// Return the itemId value of the first item widget reported matching by the provided function, or an empty string if the item wasn't found
	StdString findItemId (CardView::MatchFunction fn, void *fnData);

	// Execute a scale bump animation for the specified item
	void animateItemScaleBump (const StdString &itemId);

	// Remove the specified item from the view and destroy its underlying widget. After removing the item, invoke reflow unless shouldSkipReflow is true.
	void removeItem (const StdString &itemId, bool shouldSkipReflow = false);
	void removeItem (const char *itemId, bool shouldSkipReflow = false);

	// Remove all items in the specified row from the view and destroy their underlying widgets
	void removeRowItems (int row);

	// Move an item in the view to the specified row, then invoke reflow unless shouldSkipReflow is true.
	void setItemRow (const StdString &itemId, int targetRow, bool shouldSkipReflow = false);

	// Remove all items from the view and destroy their underlying widgets
	void removeAllItems ();

	// Process all items in the view by executing the provided function, optionally resetting widget positions afterward
	void processItems (Widget::EventCallback fn, void *fnData, bool shouldReflow = false);

	// Process all items in the specified row by executing the provided function, optionally resetting widget positions afterward
	void processRowItems (int rowNumber, Widget::EventCallback fn, void *fnData, bool shouldReflow = false);

	// Change the view's vertical scroll position to display the specified row, adding an optional position delta
	void scrollToRow (int rowNumber, double positionDeltaY = 0.0f);

	// Change the view's vertical scroll position to display the specified item
	void scrollToItem (const StdString &itemId);

	// Reset content for all item labels
	void resetItemLabels ();

	// Superclass override methods
	void reflow ();

	// Item compare functions
	static bool matchPointerValue (void *ptr, Widget *widgetPtr);

	// Callback functions
	static void scrollBarPositionChanged (void *itPtr, Widget *widgetPtr);
	static void composeAnimationComplete (void *itPtr, Widget *widgetPtr);

protected:
	// Superclass override methods
	bool doProcessMouseState (const Widget::MouseState &mouseState);
	void doUpdate (int msElapsed);

private:
	struct Item {
		StdString id;
		Panel *itemPanel;
		CardLabelWindow *itemLabel;
		int row;
		bool isPositionAssigned;
		bool isHighlighted;
		bool isAnimatingSelection;
		Item ():
			itemPanel (NULL),
			itemLabel (NULL),
			row (0),
			isPositionAssigned (false),
			isHighlighted (false),
			isAnimatingSelection (false) { }
	};

	struct Row {
		Panel *headerPanel;
		double itemMarginSize;
		bool isReverseSorted;
		bool isSelectionAnimated;
		bool isRepositionAnimated;
		bool isLabeled;
		CardView::ItemLabelFunction itemLabelFn;
		void *itemLabelFnData;
		int itemCount;
		double positionY;
		Row ():
			headerPanel (NULL),
			itemMarginSize (-1.0f),
			isReverseSorted (false),
			isSelectionAnimated (false),
			isRepositionAnimated (false),
			isLabeled (false),
			itemLabelFn (NULL),
			itemLabelFnData (NULL),
			itemCount (0),
			positionY (0.0f) { }
	};

	// Sort the item list and populate secondary data structures. This method must be invoked only while holding a lock on itemMutex.
	void doSort ();

	// Reset the contents of itemIdMap to match item state. This method must be invoked only while holding a lock on itemMutex.
	void resetItemIdMap ();

	// Return an iterator positioned at the specified item in the item list, or the end of the item list if the item wasn't found. This method must be invoked only while holding a lock on itemMutex.
	std::list<CardView::Item>::iterator findItemPosition (const StdString &itemId);

	// Clear fields in an Item struct
	void clearItem (CardView::Item *item);

	// Return a pointer to the specified row entry, or NULL if the row was not found
	CardView::Row *getRow (int rowNumber);

	// Return a newly created CardLabelWindow for use as an item label
	CardLabelWindow *createItemLabel (Panel *itemPanel);

	static bool compareItemsAscending (const CardView::Item &a, const CardView::Item &b);
	static bool compareItemsDescending (const CardView::Item &a, const CardView::Item &b);

	SDL_mutex *itemMutex;
	std::list<CardView::Item> itemList;
	HashMap itemIdMap; // A map of item ID strings to numbers indicating the item's position in itemList
	std::vector<CardView::Row> rowList;
	ScrollBar *scrollBar;
};
#endif
