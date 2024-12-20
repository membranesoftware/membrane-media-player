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
#include "StringList.h"
#include "OsUtil.h"
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "UiStack.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Font.h"
#include "CardView.h"
#include "Chip.h"
#include "Button.h"
#include "Toolbar.h"
#include "HelpWindow.h"
#include "AppUrl.h"
#include "MediaItem.h"
#include "CardLabelWindow.h"
#include "IconLabelWindow.h"
#include "TextFieldWindow.h"
#include "Image.h"
#include "Slider.h"
#include "SliderWindow.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "MediaPlaylist.h"
#include "MediaPlaylistWindow.h"
#include "MediaThumbnailWindow.h"
#include "MediaPlaylistUi.h"

constexpr const SDL_Keycode selectAllKeycode = SDLK_a;
constexpr const double nameTextFieldScale = 0.25f;

constexpr const int OptionRow = 0;
constexpr const int ThumbnailRow = 1;

MediaPlaylistUi::MediaPlaylistUi (MediaPlaylistWindow *playlistPtr)
: Ui ()
, playlistHandle (&playlist, playlistPtr)
, nameTextFieldPanelHandle (&nameTextFieldPanel)
, nameTextFieldHandle (&nameTextField)
, shuffleToggleHandle (&shuffleToggle)
, startPositionSliderHandle (&startPositionSlider)
, playDurationSliderHandle (&playDurationSlider)
{
}
MediaPlaylistUi::~MediaPlaylistUi () {
}

StdString MediaPlaylistUi::getSpritePath () {
	return (StdString ("ui/MediaPlaylistUi/sprite"));
}

Widget *MediaPlaylistUi::createBreadcrumbWidget () {
	return (new Chip (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (playlist->playlist.name, App::instance->drawableWidth * 0.5f, Font::dotTruncateSuffix), sprites.getSprite (SpriteId::MediaPlaylistUi_breadcrumbIcon)));
}

void MediaPlaylistUi::setHelpWindowContent (HelpWindow *helpWindow) {
	helpWindow->setHelpText (UiText::instance->getText (UiTextId::MediaPlaylistUiHelpTitle), UiText::instance->getText (UiTextId::MediaPlaylistUiHelpText));
	if (playlist->playlist.items.empty ()) {
		helpWindow->addAction (UiText::instance->getText (UiTextId::MediaPlaylistUiHelpActionText), HelpWindow::ErrorAction);
	}
	helpWindow->addTopicLink (UiText::instance->getText (UiTextId::PlaylistEditorInterface).capitalized (), StdString (AppUrl::PlaylistEditorInterface));
}

OpResult MediaPlaylistUi::doLoad () {
	HashMap *prefs;
	Image *image;
	int imagesize;

	prefs = App::instance->lockPrefs ();
	imagesize = prefs->find (MediaPlaylistUi::imageSizeKey, (int) Ui::MediumSize);
	App::instance->unlockPrefs ();

	setDetailImageSize (imagesize);
	cardView->setItemMarginSize (UiConfiguration::instance->marginSize / 2.0f);
	cardView->setRowCount (2);
	cardView->setRowItemMarginSize (ThumbnailRow, 0.0f);
	cardView->setRowSelectionAnimated (ThumbnailRow, true);
	cardView->setRowRepositionAnimated (ThumbnailRow, true);
	cardView->setRowLabeled (ThumbnailRow, true, MediaPlaylistUi::thumbnailCardViewItemLabel, this);

	cardView->setRowHeader (OptionRow, createRowHeaderPanel (UiText::instance->getText (UiTextId::EditPlaylist).capitalized ()));

	nameTextFieldPanelHandle.assign (new Panel ());
	nameTextFieldPanel->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	image = nameTextFieldPanel->add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallPlaylistIcon)));
	image->setMouseHoverTooltip (UiText::instance->getText (UiTextId::PlaylistName).capitalized ());

	nameTextFieldHandle.assign (nameTextFieldPanel->add (new TextFieldWindow (App::instance->drawableWidth * nameTextFieldScale, UiText::instance->getText (UiTextId::EnterPlaylistNamePrompt))));
	nameTextField->setButtonsEnabled (TextFieldWindow::PasteButtonOption | TextFieldWindow::ClearButtonOption);
	nameTextField->setValue (playlist->playlist.name);
	nameTextFieldPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);
	cardView->addItem (nameTextFieldPanel, StdString (), OptionRow, true);

	shuffleToggleHandle.assign (playlist->createShuffleToggle ());
	shuffleToggle->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	shuffleToggle->setRightAligned (true);
	cardView->addItem (shuffleToggle, StdString (), OptionRow, true);

	startPositionSliderHandle.assign (playlist->createStartPositionSlider ());
	startPositionSlider->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	cardView->addItem (startPositionSlider, StdString (), OptionRow, true);

	playDurationSliderHandle.assign (playlist->createPlayDurationSlider ());
	playDurationSlider->setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	cardView->addItem (playDurationSlider, StdString (), OptionRow, true);

	cardView->reflow ();
	return (OpResult::Success);
}

void MediaPlaylistUi::doUnload () {
	playlist->setPlaylistName (nameTextField->getValue ());
	playlist->setShuffle (shuffleToggle->isChecked);
	playlist->setStartPosition ((int) startPositionSlider->value);
	playlist->setPlayDuration ((int) playDurationSlider->value);
	playlist->resetItems (itemIndexList);

	eventCallback (endCallback);
	playlistHandle.clear ();
	nameTextFieldPanelHandle.clear ();
	nameTextFieldHandle.clear ();
	shuffleToggleHandle.clear ();
	startPositionSliderHandle.clear ();
	playDurationSliderHandle.clear ();
}

static void doPause_processItems (void *intListPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		((IntList *) intListPtr)->push_back (thumbnail->itemIndex);
	}
}
void MediaPlaylistUi::doPause () {
	HashMap *prefs;

	prefs = App::instance->lockPrefs ();
	prefs->insert (MediaPlaylistUi::imageSizeKey, detailImageSize);
	App::instance->unlockPrefs ();
	itemIndexList.clear ();
	cardView->processItems (doPause_processItems, &itemIndexList);
}

void MediaPlaylistUi::doResume () {
	MediaThumbnailWindow *thumbnail;
	std::vector<MediaPlaylistItem>::const_iterator i1, i2;
	MediaItem mediaitem;
	int pos;

	UiStack::instance->setNextBackgroundTexturePath ("ui/MediaPlaylistUi/bg");

	if (! isFirstResumeComplete) {
		pos = 1;
		i1 = playlist->playlist.items.cbegin ();
		i2 = playlist->playlist.items.cend ();
		while (i1 != i2) {
			if (mediaitem.readRecordStore (i1->mediaId)) {
				thumbnail = new MediaThumbnailWindow (mediaitem.width, mediaitem.height);
				thumbnail->itemId = cardView->getAvailableItemId ();
				thumbnail->mouseClickCallback = Widget::EventCallbackContext (MediaPlaylistUi::thumbnailClicked, this);
				thumbnail->imageLoadCallback = Widget::EventCallbackContext (MediaPlaylistUi::thumbnailImageLoaded, this);
				thumbnail->setDetailSize (detailImageSize, cardView->cardAreaWidth / CardView::reducedSizeItemScale);
				thumbnail->listPosition = pos;
				thumbnail->sortKey.sprintf ("%016llx", (unsigned long long) thumbnail->listPosition);
				thumbnail->itemIndex = pos - 1;
				if (mediaitem.isVideo || (mediaitem.isAudio && mediaitem.hasAudioAlbumArt)) {
					thumbnail->setSourceVideoFrame (mediaitem.mediaPath, i1->startTimestamp);
				}
				else if (mediaitem.isAudio) {
					thumbnail->setSourceSprite (mediaitem.mediaPath, SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_audioIcon), UiConfiguration::instance->darkPrimaryColor);
				}
				else {
					thumbnail->setSourceSprite (mediaitem.mediaPath, SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallErrorIcon), UiConfiguration::instance->primaryTextColor);
				}
				cardView->addItem (thumbnail, thumbnail->itemId, ThumbnailRow, true);
			}
			++pos;
			++i1;
		}
		cardView->reflow ();
	}
}

void MediaPlaylistUi::doAddMainToolbarItems (Toolbar *toolbar) {
	toolbar->addRightItem (createImageSizeButton ());
}

void MediaPlaylistUi::doAddSecondaryToolbarItems (Toolbar *toolbar) {
	Panel *panel;
	Button *button;

	button = Ui::createToolbarIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_starHalfButton), Widget::EventCallbackContext (MediaPlaylistUi::selectAllButtonClicked, this), UiText::instance->getText (UiTextId::SelectAllTooltip));
	button->shortcutKey = selectAllKeycode;
	button->isShortcutKeyControlPress = true;
	toolbar->addRightItem (button);

	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaPlaylistUi_upButton), MediaPlaylistUi::upButtonClicked, MediaPlaylistUi::upButtonFocused, UiText::instance->getText (UiTextId::MediaPlaylistMoveUpTooltip), "upButton"));
	toolbar->addRightItem (createToolPopupButton (sprites.getSprite (SpriteId::MediaPlaylistUi_topButton), MediaPlaylistUi::topButtonClicked, MediaPlaylistUi::topButtonFocused, UiText::instance->getText (UiTextId::MediaPlaylistMoveToTopTooltip), "topButton"));

	panel = new Panel ();
	panel->add (createToolPopupButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_deleteButton), MediaPlaylistUi::removeButtonClicked, MediaPlaylistUi::removeButtonFocused, UiText::instance->getText (UiTextId::MediaPlaylistRemoveTooltip), "removeButton"));
	panel->setPaddingScale (2.0f, 0.0f);
	panel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);
	toolbar->addRightItem (panel);
}

void MediaPlaylistUi::doResize () {
	if (nameTextField) {
	  nameTextField->setWindowWidth (App::instance->drawableWidth * nameTextFieldScale);
		if (nameTextFieldPanel) {
			nameTextFieldPanel->reflow ();
		}
	}
	if (startPositionSlider) {
		MediaPlaylistWindow::resizeStartPositionSlider (startPositionSlider);
	}
	if (playDurationSlider) {
		MediaPlaylistWindow::resizePlayDurationSlider (playDurationSlider);
	}
	cardView->processItems (MediaPlaylistUi::doResize_processItems, this, true);
}
void MediaPlaylistUi::doResize_processItems (void *itPtr, Widget *itemWidget) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		thumbnail->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}

void MediaPlaylistUi::handleDetailImageSizeChange () {
	cardView->processItems (MediaPlaylistUi::handleDetailImageSizeChange_processItems, this, true);
}
void MediaPlaylistUi::handleDetailImageSizeChange_processItems (void *itPtr, Widget *itemWidget) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		thumbnail->setDetailSize (it->detailImageSize, it->cardView->cardAreaWidth / CardView::reducedSizeItemScale);
	}
}

void MediaPlaylistUi::thumbnailCardViewItemLabel (void *itPtr, Widget *itemWidget, CardLabelWindow *cardLabel) {
	MediaThumbnailWindow *thumbnail;
	StdString text;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		text.sprintf ("#%i ", thumbnail->listPosition);
		if (thumbnail->thumbnailTimestamp > 0) {
			text.appendSprintf ("<%s> ", UiText::instance->getTimespanText (thumbnail->thumbnailTimestamp, UiText::HoursUnit, true).c_str ());
		}
		text.append (OsUtil::getPathBasename (thumbnail->sourcePath));
		cardLabel->setText (text);
	}
}

void MediaPlaylistUi::thumbnailClicked (void *itPtr, Widget *widgetPtr) {
	MediaThumbnailWindow *thumbnail = (MediaThumbnailWindow *) widgetPtr;

	thumbnail->setSelected (! thumbnail->isSelected);
}

void MediaPlaylistUi::thumbnailImageLoaded (void *itPtr, Widget *widgetPtr) {
	((MediaPlaylistUi *) itPtr)->cardView->reflow ();
}

static void findSelectedThumbnails_processItems (void *stringListPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail && thumbnail->isSelected) {
		((StringList *) stringListPtr)->push_back (thumbnail->itemId);
	}
}

void MediaPlaylistUi::topButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StdString text;
	Color color;
	StringList ids;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::NoPlaylistItemSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getCountText ((int) ids.size (), UiTextId::PlaylistItem, UiTextId::PlaylistItems));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MoveToTop).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

static void topButtonClicked_processItemsSelected (void *intPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;
	int *position;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail && thumbnail->isSelected) {
		position = (int *) intPtr;
		thumbnail->listPosition = *position;
		thumbnail->sortKey.sprintf ("%016llx", (unsigned long long) thumbnail->listPosition);
		++(*position);
	}
}
static void topButtonClicked_processItemsUnselected (void *intPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;
	int *position;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail && (! thumbnail->isSelected)) {
		position = (int *) intPtr;
		thumbnail->listPosition = *position;
		thumbnail->sortKey.sprintf ("%016llx", (unsigned long long) thumbnail->listPosition);
		++(*position);
	}
}
void MediaPlaylistUi::topButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StringList ids;
	int position;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		return;
	}
	position = 1;
	it->cardView->processItems (topButtonClicked_processItemsSelected, &position);
	it->cardView->processItems (topButtonClicked_processItemsUnselected, &position);
	it->cardView->shouldSortItemList = true;
	it->cardView->resetItemLabels ();
	it->cardView->reflow ();
}

void MediaPlaylistUi::upButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StdString text;
	Color color;
	StringList ids;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::NoPlaylistItemSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getCountText ((int) ids.size (), UiTextId::PlaylistItem, UiTextId::PlaylistItems));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::MoveUp).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

static bool upButtonClicked_findItem (void *intPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	return (thumbnail && (thumbnail->listPosition == *((int *) intPtr)));
}
void MediaPlaylistUi::upButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StringList ids;
	StringList::const_iterator i1, i2;
	MediaThumbnailWindow *thumbnail, *prev;
	int pos;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		return;
	}
	i1 = ids.cbegin ();
	i2 = ids.cend ();
	while (i1 != i2) {
		thumbnail = MediaThumbnailWindow::castWidget (it->cardView->getItem (*i1));
		if (thumbnail && (thumbnail->listPosition > 1)) {
			pos = thumbnail->listPosition - 1;
			prev = MediaThumbnailWindow::castWidget (it->cardView->findItem (upButtonClicked_findItem, &pos));
			if (prev && (! ids.contains (prev->itemId))) {
				thumbnail->listPosition = pos;
				thumbnail->sortKey.sprintf ("%016llx", (unsigned long long) thumbnail->listPosition);
				prev->listPosition = pos + 1;
				prev->sortKey.sprintf ("%016llx", (unsigned long long) prev->listPosition);
			}
		}
		++i1;
	}
	it->cardView->shouldSortItemList = true;
	it->cardView->resetItemLabels ();
	it->cardView->reflow ();
}

void MediaPlaylistUi::removeButtonFocused (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StdString text;
	Color color;
	StringList ids;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		text.assign (UiText::instance->getText (UiTextId::NoPlaylistItemSelectedPrompt));
		color.assign (UiConfiguration::instance->errorTextColor);
	}
	else {
		text.assign (UiText::instance->getCountText ((int) ids.size (), UiTextId::PlaylistItem, UiTextId::PlaylistItems));
		color.assign (UiConfiguration::instance->primaryTextColor);
	}
	it->showToolPopup (widgetPtr, UiText::instance->getText (UiTextId::Remove).capitalized (), it->createToolPopupLabel (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_smallStreamIcon), text, color));
}

static void removeButtonClicked_processItems (void *intPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;
	int *position;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		position = (int *) intPtr;
		thumbnail->listPosition = (*position);
		thumbnail->sortKey.sprintf ("%016llx", (unsigned long long) thumbnail->listPosition);
		++(*position);
	}
}
void MediaPlaylistUi::removeButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StringList ids;
	StringList::const_iterator i1, i2;
	int position;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	if (ids.empty ()) {
		return;
	}
	i1 = ids.cbegin ();
	i2 = ids.cend ();
	while (i1 != i2) {
		it->cardView->removeItem (*i1, true);
		++i1;
	}

	position = 1;
	it->cardView->processItems (removeButtonClicked_processItems, &position);
	it->cardView->shouldSortItemList = true;
	it->cardView->resetItemLabels ();
	it->cardView->reflow ();
}

static void selectAllButtonClicked_processItems (void *boolPtr, Widget *itemWidget) {
	MediaThumbnailWindow *thumbnail;

	thumbnail = MediaThumbnailWindow::castWidget (itemWidget);
	if (thumbnail) {
		thumbnail->setSelected (*((bool *) boolPtr), true);
	}
}
void MediaPlaylistUi::selectAllButtonClicked (void *itPtr, Widget *widgetPtr) {
	MediaPlaylistUi *it = (MediaPlaylistUi *) itPtr;
	StringList ids;
	bool selected;

	it->cardView->processItems (findSelectedThumbnails_processItems, &ids);
	selected = ids.empty ();
	it->cardView->processItems (selectAllButtonClicked_processItems, &selected);
	it->cardView->reflow ();
}
