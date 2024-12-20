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
#include "Config.h"
#include "App.h"
#include "ClassId.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SpriteGroup.h"
#include "SpriteId.h"
#include "Image.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "ListView.h"
#include "MediaItemTagWindow.h"

constexpr const double windowWidthScale = 0.44f;
constexpr const double tagTextWidthScale = 0.2f;

MediaItemTagWindow::MediaItemTagWindow (const StringList &tagValues)
: Panel ()
, windowWidth (0.0f)
, isExpanded (false)
{
	classId = ClassId::MediaItemTagWindow;
	tags.assign (tagValues);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setPaddingScale (0.5f, 0.5f);
	windowWidth = App::instance->drawableWidth * windowWidthScale;

	headerIcon = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_tagIcon)));
	headerIcon->setDrawColor (true, UiConfiguration::instance->primaryTextColor);
	headerIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchKeys).capitalized ());

	nameLabel = add (new Label (UiText::instance->getText (UiTextId::SearchKeys).capitalized (), UiConfiguration::HeadlineFont, UiConfiguration::instance->primaryTextColor));
	nameLabel->isVisible = false;

	tagTextLabel = add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	tagTextLabel->setMouseHoverTooltip (UiText::instance->getText (UiTextId::SearchKeys).capitalized ());

	expandToggle = add (new Toggle (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandMoreButton), SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_expandLessButton)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MediaItemTagWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->buttonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextId::Expand).capitalized (), UiText::instance->getText (UiTextId::Minimize).capitalized ());

	dividerPanel = add (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	tagListView = add (new ListView (windowWidth, 2, 6, UiConfiguration::CaptionFont));
	tagListView->setItemDeleteCallback (Widget::EventCallbackContext (MediaItemTagWindow::listViewItemDeleted, this));
	tagListView->setEmptyStateText (UiText::instance->getText (UiTextId::MediaItemTagWindowTextFieldPrompt), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor);
	tagListView->setItems (tags, true);
	tagListView->isVisible = false;

	addButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_addButton), Widget::EventCallbackContext (MediaItemTagWindow::addButtonClicked, this), UiText::instance->getText (UiTextId::AddSearchKey).capitalized (), "mediaControlTagWindowAddButton"));
	addButton->isVisible = false;

	resetTagText ();
}
MediaItemTagWindow::~MediaItemTagWindow () {
}

void MediaItemTagWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPaddingScale (1.0f, 1.0f);
	}
	else {
		setPaddingScale (0.5f, 0.5f);
	}
	nameLabel->isVisible = isExpanded;
	tagTextLabel->isVisible = (! isExpanded);
	dividerPanel->isVisible = isExpanded;
	tagListView->isVisible = isExpanded;
	addButton->isVisible = isExpanded;
	expandToggle->setChecked (isExpanded, true);

	reflow ();
	if (! shouldSkipStateChangeCallback) {
		expandToggle->eventCallback (expandToggle->stateChangeCallback);
	}
}

void MediaItemTagWindow::setDisabled (bool disabled) {
	tagListView->setDisabled (disabled);
	addButton->setDisabled (disabled);
}

void MediaItemTagWindow::setTags (const StringList &tagValues) {
	tags.assign (tagValues);
	tagListView->setItems (tags);
	resetTagText ();
}

void MediaItemTagWindow::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();
	headerIcon->flowRight (&layoutFlow);

	if (nameLabel->isVisible) {
		nameLabel->flowRight (&layoutFlow);
	}
	if (tagTextLabel->isVisible) {
		tagTextLabel->flowRight (&layoutFlow);
	}

	nextColumnLayoutFlow ();
	expandToggle->flowRight (&layoutFlow);
	headerIcon->centerVertical (&layoutFlow);
	if (nameLabel->isVisible) {
		nameLabel->centerVertical (&layoutFlow);
	}
	if (tagTextLabel->isVisible) {
		tagTextLabel->centerVertical (&layoutFlow);
	}

	if (isExpanded) {
		nextRowLayoutFlow ();
		layoutFlow.x = 0.0f;
		dividerPanel->flowDown (&layoutFlow);
		layoutFlow.x = widthPadding;

		tagListView->setViewWidth (windowWidth - (widthPadding * 2.0f));
		tagListView->flowDown (&layoutFlow);

		addButton->flowDown (&layoutFlow);

		setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
	}
	else {
		setFixedSize (false);
		resetSize ();
	}

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	bottomRightLayoutFlow ();
	expandToggle->flowLeft (&layoutFlow);

	if (addButton->isVisible) {
		bottomRightLayoutFlow ();
		addButton->flowLeft (&layoutFlow);
	}
}

void MediaItemTagWindow::doResize () {
	Panel::doResize ();
	windowWidth = App::instance->drawableWidth * windowWidthScale;
	resetTagText ();
}

void MediaItemTagWindow::resetTagText () {
	StdString text;

	text.sprintf ("(%i)", (int) tags.size ());
	if (! tags.empty ()) {
		text.append (" ");
		text.append (tags.join (", "));
	}
	tagTextLabel->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (text, App::instance->windowWidth * tagTextWidthScale, Font::dotTruncateSuffix));
	reflow ();
}

void MediaItemTagWindow::expandToggleStateChanged (void *itPtr, Widget *widgetPtr) {
	MediaItemTagWindow *it = (MediaItemTagWindow *) itPtr;
	Toggle *toggle = (Toggle *) widgetPtr;

	it->setExpanded (toggle->isChecked, true);
	it->eventCallback (it->expandStateChangeCallback);
}

Widget::Rectangle MediaItemTagWindow::getAddButtonScreenRect () {
	return (addButton->getScreenRect ());
}

Widget::Rectangle MediaItemTagWindow::getDeleteButtonScreenRect () {
	return (tagListView->getDeleteButtonScreenRect ());
}

void MediaItemTagWindow::addButtonClicked (void *itPtr, Widget *widgetPtr) {
	((MediaItemTagWindow *) itPtr)->eventCallback (((MediaItemTagWindow *) itPtr)->addClickCallback);
}

void MediaItemTagWindow::listViewItemDeleted (void *itPtr, Widget *widgetPtr) {
	MediaItemTagWindow *it = (MediaItemTagWindow *) itPtr;
	ListView *listview = (ListView *) widgetPtr;

	it->itemDeleteTag.assign (listview->getItemText (listview->focusItemIndex));
	it->eventCallback (it->itemDeleteCallback);
}
