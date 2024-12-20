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
#include "SpriteId.h"
#include "OsUtil.h"
#include "SpriteGroup.h"
#include "TaskGroup.h"
#include "Ui.h"
#include "UiText.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "LabelWindow.h"
#include "TextFieldWindow.h"
#include "ScrollViewWindow.h"
#include "IconLabelWindow.h"
#include "UiConfiguration.h"
#include "FsBrowserWindow.h"

// Stage values
constexpr const int Uninitialized = 0;
constexpr const int LoadWaiting = 1;
constexpr const int ShowingFiles = 2;

FsBrowserWindow::FsBrowserWindow (double windowWidth, double windowHeight, const StdString &initialPath)
: Panel ()
, sortOrder (FsBrowserWindow::NameSort)
, selectType (FsBrowserWindow::SelectFiles)
, windowWidth (windowWidth)
, windowHeight (windowHeight)
, isPathSelectionConfirmed (false)
, stage (Uninitialized)
, isLoading (false)
, isFirstLoad (true)
, shouldLoadPath (false)
, loadPathTarget (initialPath)
, loadPathResult (OpResult::InternalApplicationFailureError)
, pathFieldClock (0)
, componentButtonHeight (0.0f)
{
	classId = ClassId::FsBrowserWindow;
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setFixedSize (true, windowWidth, windowHeight);

	pathField = add (new TextFieldWindow (1.0f));
	pathField->widgetName.assign ("fsBrowserPathTextField");
	pathField->setButtonsEnabled (TextFieldWindow::PasteButtonOption | TextFieldWindow::ClearButtonOption);
	pathField->isVisible = false;

	pathView = (ScrollViewWindow *) addWidget (new ScrollViewWindow ());
	pathView->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	pathView->setViewLayout (Panel::DownFlowLayoutOption | Panel::LeftGravityLayoutOption, 0.0f);
	pathView->setScrollOptions (ScrollViewWindow::KeyboardScrollOption | ScrollViewWindow::MouseWheelScrollOption);

	enterTextButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_keyboardButton)));
	enterTextButton->widgetName.assign ("fsBrowserEnterTextButton");
	enterTextButton->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::enterTextButtonClicked, this);
	enterTextButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	enterTextButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FsBrowserWindowEnterTextButtonTooltip));

	componentBackButton = add (new Button (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_backButton)));
	componentBackButton->widgetName.assign ("fsBrowserBackButton");
	componentBackButton->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::componentBackButtonClicked, this);
	componentBackButton->setImageColor (UiConfiguration::instance->buttonTextColor);
	componentBackButton->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FsBrowserWindowComponentBackButtonTooltip));
	componentBackButton->setDisabled (true);

	componentPanel = add (new Panel ());
	componentPanel->setFixedPadding (true, 0.0f, 0.0f);
	componentPanel->layoutSpacing = 0.0f;
	componentPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	componentButtonPanel = componentPanel->add (new Panel ());
	componentButtonPanel->setFixedPadding (true, 0.0f, 0.0f);
	componentButtonPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	componentButtonPanel->layoutSpacing = 0.0f;
	componentButtonPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);

	selectedPathPanel = add (new Panel ());
	selectedPathPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	selectedPathPanel->setBorder (true, UiConfiguration::instance->mediumBackgroundColor);
	selectedPathPanel->setPaddingScale (1.0f, 0.5f);

	selectedPathLabel1 = selectedPathPanel->add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	selectedPathLabel1->isVisible = false;
	selectedPathLabel2 = selectedPathPanel->add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	selectedPathLabel2->isVisible = false;

	cancelButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_cancelButton), Widget::EventCallbackContext (FsBrowserWindow::cancelButtonClicked, this), UiText::instance->getText (UiTextId::Cancel).capitalized (), "fsBrowserCancelButton"));

	confirmButton = add (Ui::createIconButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_okButton), Widget::EventCallbackContext (FsBrowserWindow::confirmButtonClicked, this), UiText::instance->getText (UiTextId::FsBrowserWindowConfirmButtonTooltip), "fsBrowserConfirmButton"));
	confirmButton->setDisabled (true);

	statusLabel = add (new IconLabelWindow (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_largeErrorIcon), StdString (), UiConfiguration::BodyFont, UiConfiguration::instance->errorTextColor), 1);
	statusLabel->isVisible = false;

	reflow ();
}
FsBrowserWindow::~FsBrowserWindow () {
}

void FsBrowserWindow::setSelectPromptText (const StdString &text) {
	selectPromptText.assign (text);
	if (selectedPath.empty ()) {
		selectedPathLabel1->isVisible = true;
		selectedPathLabel1->setText (selectPromptText);
		selectedPathLabel2->isVisible = false;
		reflow ();
	}
}

void FsBrowserWindow::reflow () {
	double w, h, viewx;

	pathView->reflow ();
	resetPadding ();
	topLeftLayoutFlow ();
	w = windowWidth - (widthPadding * 2.0f) - enterTextButton->width - componentBackButton->width - (UiConfiguration::instance->marginSize * 2.0f);

	enterTextButton->flowRight (&layoutFlow);
	componentBackButton->flowRight (&layoutFlow);
	if (pathField->isVisible) {
		pathField->setWindowWidth (w);
		pathField->flowRight (&layoutFlow);
		pathField->centerVertical (&layoutFlow);
	}
	if (componentPanel->isVisible) {
		componentButtonPanel->setFixedSize (true, w, componentButtonHeight);
		componentButtonPanel->reflow ();
		viewx = componentButtonPanel->extentX2 - w;
		if (viewx < 0.0f) {
			viewx = 0.0f;
		}
		componentButtonPanel->setViewOrigin (viewx, 0.0f);

		componentPanel->reflow ();
		componentPanel->flowRight (&layoutFlow);
		componentPanel->centerVertical (&layoutFlow);
	}
	enterTextButton->centerVertical (&layoutFlow);
	confirmButton->centerVertical (&layoutFlow);

	nextRowLayoutFlow ();
	selectedPathPanel->setFixedSize (true, windowWidth - (widthPadding * 2.0f) - cancelButton->width - confirmButton->width - (UiConfiguration::instance->marginSize * 2.0f), selectedPathLabel1->maxLineHeight + UiConfiguration::instance->paddingSize);
	selectedPathPanel->layoutSpacing = ((double) UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->spaceWidth) / 2.0f;
	selectedPathPanel->setLayout (Panel::RightFlowLayoutOption);
	selectedPathPanel->reflow ();
	h = selectedPathPanel->height;
	if (cancelButton->height > h) {
		h = cancelButton->height;
	}
	if (confirmButton->height > h) {
		h = confirmButton->height;
	}
	pathView->setViewSize (windowWidth - (widthPadding * 2.0f), windowHeight - layoutFlow.y - heightPadding - h - UiConfiguration::instance->marginSize);
	pathView->flowDown (&layoutFlow);
	if (statusLabel->isVisible) {
		statusLabel->position.assign (pathView->position.x + (pathView->width / 2.0f) - (statusLabel->width / 2.0f), pathView->position.y + UiConfiguration::instance->paddingSize);
	}

	nextRowLayoutFlow ();
	selectedPathPanel->flowRight (&layoutFlow);
	cancelButton->flowRight (&layoutFlow);
	confirmButton->flowRight (&layoutFlow);
	selectedPathPanel->centerVertical (&layoutFlow);
	cancelButton->centerVertical (&layoutFlow);
	confirmButton->centerVertical (&layoutFlow);
}

void FsBrowserWindow::doUpdate (int msElapsed) {
	StdString val;

	switch (stage) {
		case Uninitialized: {
			startLoad ();
			stage = LoadWaiting;
			break;
		}
		case LoadWaiting: {
			if (! isLoading) {
				showLoadState ();
				stage = ShowingFiles;
			}
			break;
		}
		case ShowingFiles: {
			if (shouldLoadPath || (! loadPathTarget.empty ())) {
				shouldLoadPath = false;
				startLoad ();
				stage = LoadWaiting;
				break;
			}
			if (pathField->isVisible) {
				val = pathField->getValue ();
				if (val.empty ()) {
					if (! lastPathText.equals (val)) {
						lastPathText.assign (val);
						pathFieldClock = UiConfiguration::instance->idleUpdateThreshold;
						showEmptyState ();
					}
				}
				else {
					if ((! lastPathText.equals (val)) || lastLoadPathTarget.equals (val) || browsePath.equals (val)) {
						lastPathText.assign (val);
						pathFieldClock = UiConfiguration::instance->idleUpdateThreshold;
					}
					else {
						pathFieldClock -= msElapsed;
						if (pathFieldClock <= 0) {
							pathFieldClock = UiConfiguration::instance->idleUpdateThreshold;
							loadPathTarget.assign (val);
							startLoad ();
							stage = LoadWaiting;
							break;
						}
					}
				}
			}
			break;
		}
	}
	Panel::doUpdate (msElapsed);
}

void FsBrowserWindow::startLoad () {
	isLoading = true;
	confirmButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonLoadingTooltip));
	componentBackButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonLoadingTooltip));
	retain ();
	TaskGroup::instance->run (TaskGroup::RunContext (FsBrowserWindow::loadPath, this));
}

void FsBrowserWindow::loadPath (void *itPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;

	if (! it->isDestroyed) {
		it->executeLoadPath ();
	}
	it->lastLoadPathTarget.assign (it->loadPathTarget);
	it->loadPathTarget.assign ("");
	it->isLoading = false;
	it->release ();
}

void FsBrowserWindow::executeLoadPath () {
	StdString dirpath, filepath;
	StringList filenames, nextfilepaths;
	StringList::const_iterator i1, i2;
	int type;

	dirpath.assign (loadPathTarget);
	if (dirpath.empty ()) {
		loadPathResult = OsUtil::readRootPath (&filenames);
		dirpath = OsUtil::getPathPrefix ();
	}
	else {
		loadPathResult = OsUtil::readDirectory (dirpath, &filenames);
	}
	if (isFirstLoad && (loadPathResult != OpResult::Success)) {
		dirpath.assign (OsUtil::getUserHomePath ());
		loadPathResult = OsUtil::readDirectory (dirpath, &filenames);
	}

	isFirstLoad = false;
	loadItems.clear ();
	browsePath.assign (dirpath);
	if (loadPathResult == OpResult::Success) {
		filenames.sort (StringList::compareCaseInsensitiveAscending);

		if (sortOrder == FsBrowserWindow::DirectoriesFirstSort) {
			i1 = filenames.cbegin ();
			i2 = filenames.cend ();
			while (i1 != i2) {
				filepath = *i1;
				++i1;
				if (! OsUtil::isFilename (filepath)) {
					continue;
				}
				type = OsUtil::getFileType (OsUtil::getJoinedPath (dirpath, filepath));
				if (type == OsUtil::DirectoryFile) {
					loadItems.push_back (FsBrowserWindow::FileData (filepath, type));
				}
				else if (type == OsUtil::RegularFile) {
					nextfilepaths.push_back (filepath);
				}
			}

			i1 = nextfilepaths.cbegin ();
			i2 = nextfilepaths.cend ();
			while (i1 != i2) {
				loadItems.push_back (FsBrowserWindow::FileData (*i1, OsUtil::RegularFile));
				++i1;
			}
		}
		else {
			i1 = filenames.cbegin ();
			i2 = filenames.cend ();
			while (i1 != i2) {
				filepath = *i1;
				++i1;
				if (! OsUtil::isFilename (filepath)) {
					continue;
				}
				type = OsUtil::getFileType (OsUtil::getJoinedPath (dirpath, filepath));
				if ((type == OsUtil::RegularFile) || (type == OsUtil::DirectoryFile)) {
					loadItems.push_back (FsBrowserWindow::FileData (filepath, type));
				}
			}
		}
	}
}

void FsBrowserWindow::showLoadState () {
	std::list<FsBrowserWindow::FileData>::const_iterator i1, i2;
	FsBrowserWindowItemLabel *item;
	Button *button;
	StringList c;
	StringList::const_iterator j1, j2;
	StdString targetpath;
	int textid;

	pathView->clearViewItems ();
	components.clear ();
	componentButtonPanel->clear ();
	if (loadPathResult != OpResult::Success) {
		statusLabel->setText (UiText::instance->getText (UiTextId::FsBrowserWindowLoadErrorText));
		statusLabel->isVisible = true;

		confirmButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonLoadErrorTooltip));
		componentBackButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonLoadErrorTooltip));
	}
	else {
		i1 = loadItems.cbegin ();
		i2 = loadItems.cend ();
		while (i1 != i2) {
			item = new FsBrowserWindowItemLabel (pathView->scrollViewWidth, i1->name, i1->type);
			item->mouseEnterCallback = Widget::EventCallbackContext (FsBrowserWindow::itemMouseEntered, this);
			item->mouseExitCallback = Widget::EventCallbackContext (FsBrowserWindow::itemMouseExited, this);
			item->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::itemMouseClicked, this);
			pathView->addViewItem (item);
			++i1;
		}
		statusLabel->isVisible = false;
		if (selectedPath.empty ()) {
			if (selectType == FsBrowserWindow::SelectDirectories) {
				textid = UiTextId::FsBrowserWindowDisabledButtonSelectDirectoryTooltip;
			}
			else if (selectType == FsBrowserWindow::SelectFilesAndDirectories) {
				textid = UiTextId::FsBrowserWindowDisabledButtonSelectFileOrDirectoryTooltip;
			}
			else {
				textid = UiTextId::FsBrowserWindowDisabledButtonSelectFileTooltip;
			}
			confirmButton->setDisabled (true, UiText::instance->getText (textid));
		}
		else {
			confirmButton->setDisabled (false, UiText::instance->getText (UiTextId::FsBrowserWindowConfirmButtonTooltip));
		}

		if (OsUtil::isRootPath (browsePath)) {
			componentBackButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowComponentBackButtonTooltip));
		}
		else {
			componentBackButton->setDisabled (false, UiText::instance->getText (UiTextId::FsBrowserWindowComponentBackButtonTooltip));
		}
	}

	button = componentButtonPanel->add (new Button (UiText::instance->getRootPathText ()));
	button->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::componentButtonClicked, this);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FsBrowserWindowComponentButtonTooltip));
	components.push_back (ComponentItem (StdString (), button));
	componentButtonHeight = button->height;

	if (! browsePath.empty ()) {
		OsUtil::splitPath (browsePath, &c);
		targetpath = OsUtil::getPathPrefix ();
		j1 = c.cbegin ();
		j2 = c.cend ();
		while (j1 != j2) {
			if (! j1->empty ()) {
				targetpath = OsUtil::getJoinedPath (targetpath, *j1);
				button = componentButtonPanel->add (new Button (*j1));
				button->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::componentButtonClicked, this);
				button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FsBrowserWindowComponentButtonTooltip));
				components.push_back (ComponentItem (targetpath, button));
				if (button->height > componentButtonHeight) {
					componentButtonHeight = button->height;
				}
			}
			++j1;
		}
	}
	pathField->setValue (browsePath);

	componentButtonPanel->reflow ();
	componentPanel->reflow ();
	reflow ();
}

void FsBrowserWindow::showEmptyState () {
	Button *button;

	pathView->clearViewItems ();
	components.clear ();
	componentButtonPanel->clear ();

	button = componentButtonPanel->add (new Button (UiText::instance->getRootPathText ()));
	button->mouseClickCallback = Widget::EventCallbackContext (FsBrowserWindow::componentButtonClicked, this);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::FsBrowserWindowComponentButtonTooltip));
	components.push_back (ComponentItem (StdString (), button));
	componentButtonHeight = button->height;

	statusLabel->isVisible = false;

	confirmButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonEmptyPathTooltip));
	componentBackButton->setDisabled (true, UiText::instance->getText (UiTextId::FsBrowserWindowDisabledButtonEmptyPathTooltip));

	componentButtonPanel->reflow ();
	componentPanel->reflow ();
	reflow ();
}

void FsBrowserWindow::componentButtonClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;
	Button *button = (Button *) widgetPtr;
	std::list<ComponentItem>::const_iterator i1, i2;
	StdString targetpath;

	if (it->isLoading) {
		return;
	}
	i1 = it->components.cbegin ();
	i2 = it->components.cend ();
	while (i1 != i2) {
		if (i1->button == button) {
			targetpath.assign (i1->path);
			break;
		}
		++i1;
	}
	if (targetpath.empty ()) {
		it->loadPathTarget.assign ("");
		it->shouldLoadPath = true;
	}
	else {
		it->loadPathTarget.assign (targetpath);
		if ((it->selectType == FsBrowserWindow::SelectDirectories) || (it->selectType == FsBrowserWindow::SelectFilesAndDirectories)) {
			it->setSelectedPath (targetpath);
		}
	}
}

void FsBrowserWindow::componentBackButtonClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;
	StdString path;

	if (it->isLoading) {
		return;
	}
	path = OsUtil::getPathDirname (it->browsePath);
	if (! path.empty ()) {
		it->loadPathTarget.assign (path);
		if ((it->selectType == FsBrowserWindow::SelectDirectories) || (it->selectType == FsBrowserWindow::SelectFilesAndDirectories)) {
			it->setSelectedPath (path);
		}
	}
	else if (OsUtil::isRootPath (path)) {
		it->loadPathTarget.assign (path);
		it->shouldLoadPath = true;
	}
}

void FsBrowserWindow::itemMouseEntered (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;
	FsBrowserWindowItemLabel *item = (FsBrowserWindowItemLabel *) widgetPtr;
	bool highlight;

	highlight = false;
	if (item->fileType == OsUtil::DirectoryFile) {
		highlight = true;
	}
	else if (item->fileType == OsUtil::RegularFile) {
		if ((it->selectType == FsBrowserWindow::SelectFiles) || (it->selectType == FsBrowserWindow::SelectFilesAndDirectories)) {
			highlight = true;
		}
	}
	if (highlight) {
		item->setHighlighted (true);
	}
}

void FsBrowserWindow::itemMouseExited (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindowItemLabel *item = (FsBrowserWindowItemLabel *) widgetPtr;

	item->setHighlighted (false);
}

void FsBrowserWindow::itemMouseClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;
	FsBrowserWindowItemLabel *item = (FsBrowserWindowItemLabel *) widgetPtr;
	bool selected;

	selected = false;
	if (item->fileType == OsUtil::DirectoryFile) {
		it->loadPathTarget = OsUtil::getJoinedPath (it->browsePath, item->fileName);
		if ((it->selectType == FsBrowserWindow::SelectDirectories) || (it->selectType == FsBrowserWindow::SelectFilesAndDirectories)) {
			selected = true;
		}
	}
	else if (item->fileType == OsUtil::RegularFile) {
		if ((it->selectType == FsBrowserWindow::SelectFiles) || (it->selectType == FsBrowserWindow::SelectFilesAndDirectories)) {
			selected = true;
		}
	}
	if (selected) {
		it->setSelectedPath (OsUtil::getJoinedPath (it->browsePath, item->fileName));
	}
}

void FsBrowserWindow::enterTextButtonClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;

	if (it->componentPanel->isVisible) {
		it->componentPanel->isVisible = false;
		it->pathField->setValue (it->browsePath);
		it->pathField->isVisible = true;
	}
	else {
		it->pathField->isVisible = false;
		it->componentPanel->isVisible = true;
	}
	it->reflow ();
}

void FsBrowserWindow::setSelectedPath (const StdString &targetPath) {
	Font::Metrics metrics;
	StdString dirname, basename;
	double textw;
	int textid;

	if (selectedPath.equals (targetPath)) {
		return;
	}
	selectedPath.assign (targetPath);
	if (selectedPath.empty ()) {
		selectedPathLabel1->isVisible = true;
		selectedPathLabel1->setText (selectPromptText);
		selectedPathLabel2->isVisible = false;

		if (selectType == FsBrowserWindow::SelectDirectories) {
			textid = UiTextId::FsBrowserWindowDisabledButtonSelectDirectoryTooltip;
		}
		else if (selectType == FsBrowserWindow::SelectFilesAndDirectories) {
			textid = UiTextId::FsBrowserWindowDisabledButtonSelectFileOrDirectoryTooltip;
		}
		else {
			textid = UiTextId::FsBrowserWindowDisabledButtonSelectFileTooltip;
		}
		confirmButton->setDisabled (true, UiText::instance->getText (textid));
	}
	else {
		textw = selectedPathPanel->width - (selectedPathPanel->widthPadding * 2.0f) - selectedPathPanel->layoutSpacing;
		UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, selectedPath);
		if (metrics.textWidth < textw) {
			selectedPathLabel1->isVisible = true;
			selectedPathLabel1->setText (selectedPath);
			selectedPathLabel2->isVisible = false;
		}
		else {
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, Font::dotTruncateSuffix);
			textw -= metrics.textWidth;
			basename = OsUtil::getPathBasename (selectedPath);
			UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->resetMetrics (&metrics, basename);
			if (metrics.textWidth > textw) {
				selectedPathLabel1->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (basename, textw, Font::dotTruncateSuffix));
				selectedPathLabel1->isVisible = true;
				selectedPathLabel2->isVisible = false;
			}
			else {
				dirname = OsUtil::getPathDirname (selectedPath);
				selectedPathLabel1->setText (UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->truncatedText (dirname, textw - metrics.textWidth, Font::dotTruncateSuffix));
				selectedPathLabel2->setText (basename);
				selectedPathLabel1->isVisible = true;
				selectedPathLabel2->isVisible = true;
			}
		}

		confirmButton->setDisabled (false, UiText::instance->getText (UiTextId::FsBrowserWindowConfirmButtonTooltip));
	}
	reflow ();
}

void FsBrowserWindow::cancelButtonClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;

	it->isPathSelectionConfirmed = false;
	it->eventCallback (it->closeCallback);
}

void FsBrowserWindow::confirmButtonClicked (void *itPtr, Widget *widgetPtr) {
	FsBrowserWindow *it = (FsBrowserWindow *) itPtr;

	it->isPathSelectionConfirmed = true;
	it->eventCallback (it->closeCallback);
}

FsBrowserWindowItemLabel::FsBrowserWindowItemLabel (double windowWidth, const StdString &fileName, int fileType)
: Panel ()
, windowWidth (windowWidth)
, fileName (fileName)
, fileType (fileType)
, iconImage (NULL)
, nameLabel (NULL)
, isActive (false)
, isHighlighted (false)
{
	Sprite *sprite;

	if (fileType == OsUtil::DirectoryFile) {
		sprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_directoryIcon);
	}
	else {
		sprite = SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_fileIcon);
	}
	iconImage = add (new Image (sprite));
	iconImage->setDrawColor (true, UiConfiguration::instance->primaryTextColor);

	nameLabel = add (new Label (fileName, UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	setFixedPadding (true, 0.0f, 0.0f);
	reflow ();
}

FsBrowserWindowItemLabel::~FsBrowserWindowItemLabel () {
}

void FsBrowserWindowItemLabel::reflow () {
	resetPadding ();
	topLeftLayoutFlow ();
	iconImage->flowRight (&layoutFlow);
	nameLabel->flowRight (&layoutFlow);

	setFixedSize (true, windowWidth, layoutFlow.yExtent + heightPadding);
	layoutFlow.y = 0.0f;
	layoutFlow.yExtent = height;
	iconImage->centerVertical (&layoutFlow);
	nameLabel->centerVertical (&layoutFlow);
}

void FsBrowserWindowItemLabel::setActive (bool isActiveValue) {
	if (isActive == isActiveValue) {
		return;
	}
	isActive = isActiveValue;
	resetColors ();
}

void FsBrowserWindowItemLabel::setHighlighted (bool isHighlightedValue) {
	if (isHighlighted == isHighlightedValue) {
		return;
	}
	isHighlighted = isHighlightedValue;
	resetColors ();
}

void FsBrowserWindowItemLabel::resetColors () {
	if (isActive) {
		bgColor.translate (UiConfiguration::instance->darkPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->textColor.translate (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else if (isHighlighted) {
		bgColor.translate (UiConfiguration::instance->lightPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->textColor.translate (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		bgColor.translate (Color (0.0f, 0.0f, 0.0f, 0.0f), UiConfiguration::instance->shortColorTranslateDuration);
		nameLabel->textColor.translate (UiConfiguration::instance->primaryTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
}
