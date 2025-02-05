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
// Panel that shows a thumbnail image from a media or stream item, using content loaded from a provided source URL
#ifndef MEDIA_THUMBNAIL_WINDOW_H
#define MEDIA_THUMBNAIL_WINDOW_H

#include "Color.h"
#include "Widget.h"
#include "Panel.h"

class Sprite;
class ImageWindow;
class Button;

class MediaThumbnailWindow : public Panel {
public:
	MediaThumbnailWindow (int sourceWidth, int sourceHeight);
	~MediaThumbnailWindow ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaThumbnailWindow *castWidget (Widget *widget);

	// Read-write data members
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext imageLoadCallback;
	Widget::EventCallbackContext deleteClickCallback;
	Widget::EventCallbackContext playClickCallback;
	StdString itemId;
	int listPosition;
	int itemIndex;

	// Read-only data members
	int sourceWidth;
	int sourceHeight;
	StdString sourcePath;
	int imageSourceType;
	Sprite *imageSprite;
	Color imageSpriteDrawColor;
	bool isImageLoaded;
	int64_t thumbnailTimestamp;
	bool isSelected;
	bool isMarkerControlEnabled;

	// Set the enable option specifying if the window should show marker control widgets
	void setMarkerControl (bool enable);

	// Set the window's thumbnail content from a URL
	void setSourceImageUrl (const StdString &url, int64_t timestamp);

	// Set the window's thumbnail content from an image file
	void setSourceImageFile (const StdString &path, int64_t timestamp);

	// Set the window's thumbnail content from a frame in a video file
	void setSourceVideoFrame (const StdString &path, int64_t timestamp);

	// Set the window's thumbnail content from a sprite
	void setSourceSprite (const StdString &path, Sprite *sprite, const Color &drawColor = Color (1.0f, 1.0f, 1.0f, 1.0f));

	// Set the window to load thumbnail content from the same source loaded by another thumbnail window
	void setSourceCopy (MediaThumbnailWindow *sourceThumbnail);

	// Set the window's selected state
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Superclass override methods
	void reflow ();
	void refreshDetailSize ();

protected:
	// Superclass override methods
	void doProcessComposeDrawChange ();

private:
	// Callback functions
	static void thumbnailImageLoaded (void *itPtr, Widget *widgetPtr);
	static void thumbnailImageLongPressed (void *itPtr, Widget *widgetPtr);
	static void deleteButtonClicked (void *itPtr, Widget *widgetPtr);

	ImageWindow *thumbnailImage;
	Button *deleteButton;
};
#endif
