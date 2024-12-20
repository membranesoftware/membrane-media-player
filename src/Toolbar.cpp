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
#include "SpriteGroup.h"
#include "Button.h"
#include "Menu.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "Toolbar.h"

Toolbar::Toolbar (double toolbarWidth)
: Panel ()
, barWidth (toolbarWidth)
, isModeActive (false)
, modeCount (0)
, mode (-1)
, leftItemPanelHandle (&leftItemPanel)
, rightItemPanelHandle (&rightItemPanel)
, leftCornerPanelHandle (&leftCornerPanel)
, rightCornerPanelHandle (&rightCornerPanel)
, modeButtonHandle (&modeButton)
, modeMenuHandle (&modeMenu)
{
	classId = ClassId::Toolbar;
	setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
	setPaddingScale (1.0f, 0.5f);
	reflow ();
}
Toolbar::~Toolbar () {
	clearAll ();
}

void Toolbar::setWidth (double toolbarWidth) {
	if (FLOAT_EQUALS (toolbarWidth, barWidth)) {
		return;
	}
	barWidth = toolbarWidth;
	reflow ();
}

bool Toolbar::empty () {
	if (isModeActive || leftCornerPanel || rightCornerPanel || leftItemPanel || rightItemPanel) {
		return (false);
	}
	return (true);
}

void Toolbar::clearAll () {
	modeButtonHandle.destroyAndClear ();
	modeMenuHandle.destroyAndClear ();
	leftItemPanelHandle.destroyAndClear ();
	rightItemPanelHandle.destroyAndClear ();
	leftCornerPanelHandle.destroyAndClear ();
	rightCornerPanelHandle.destroyAndClear ();
	clearModes ();
}

void Toolbar::clearLeftItems () {
	if (leftItemPanel) {
		leftItemPanelHandle.destroyAndClear ();
		reflow ();
	}
}

void Toolbar::clearRightItems () {
	if (rightItemPanel) {
		rightItemPanelHandle.destroyAndClear ();
		reflow ();
	}
}

void Toolbar::addLeftItem (Widget *itemWidget) {
	if (! leftItemPanel) {
		leftItemPanelHandle.assign (new Panel ());
		leftItemPanel->setFixedPadding (true, 0.0f, 0.0f);
		leftItemPanel->setLayout (Panel::RightFlowLayoutOption | Panel::VerticalCenterLayoutOption);
		add (leftItemPanel, 1);
	}
	leftItemPanel->addWidget (itemWidget);
	reflow ();
}

void Toolbar::addRightItem (Widget *itemWidget) {
	if (! rightItemPanel) {
		rightItemPanelHandle.assign (new Panel ());
		rightItemPanel->setFixedPadding (true, 0.0f, 0.0f);
		rightItemPanel->setLayout (Panel::LeftFlowLayoutOption | Panel::VerticalCenterLayoutOption);
		add (rightItemPanel);
	}
	rightItemPanel->addWidget (itemWidget);
	reflow ();
}

void Toolbar::addRightSpacer () {
	Panel *panel;

	panel = new Panel ();
	panel->setFixedSize (true, UiConfiguration::instance->marginSize, 2.0f);
	addRightItem (panel);
}

void Toolbar::setLeftCorner (Panel *cornerPanel) {
	leftCornerPanelHandle.destroyAndAssign (cornerPanel);
	if (leftCornerPanel) {
		addWidget (leftCornerPanel, 2);
	}
	reflow ();
}

void Toolbar::setRightCorner (Panel *cornerPanel) {
	rightCornerPanelHandle.destroyAndAssign (cornerPanel);
	if (rightCornerPanel) {
		addWidget (rightCornerPanel, 2);
	}
	reflow ();
}

bool Toolbar::hasLeftCorner () {
	return (leftCornerPanel != NULL);
}

bool Toolbar::hasRightCorner () {
	return (rightCornerPanel != NULL);
}

void Toolbar::reflow () {
	std::map<int, Toolbar::Mode>::iterator pos;
	Toolbar *modeitem;
	double x, y, h, maxleftw, maxrightw;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	h = 0.0f;
	maxleftw = barWidth - (widthPadding * 2.0f);
	maxrightw = barWidth - (widthPadding * 2.0f);
	modeitem = NULL;
	if (isModeActive) {
		pos = modeMap.find (mode);
		if (pos != modeMap.end ()) {
			modeitem = pos->second.toolbar;
		}
	}

	if (leftCornerPanel) {
		leftCornerPanel->reflow ();
		leftCornerPanel->position.assign (x, y);
		x += (leftCornerPanel->width + UiConfiguration::instance->marginSize);
		if (h < leftCornerPanel->height) {
			h = leftCornerPanel->height;
		}
		maxleftw -= (leftCornerPanel->width + UiConfiguration::instance->marginSize);
		maxrightw -= (leftCornerPanel->width + UiConfiguration::instance->marginSize);
	}
	if (rightCornerPanel) {
		rightCornerPanel->reflow ();
		maxleftw -= (rightCornerPanel->width + UiConfiguration::instance->marginSize);
		maxrightw -= (rightCornerPanel->width + UiConfiguration::instance->marginSize);
	}
	if (modeitem) {
		if (leftItemPanel) {
			leftItemPanel->isVisible = false;
		}
		if (rightItemPanel) {
			rightItemPanel->isVisible = false;
		}
		modeitem->setWidth (maxleftw);
		modeitem->position.assign (x, y);
		modeitem->isVisible = true;
		if (h < modeitem->height) {
			h = modeitem->height;
		}
		x += (modeitem->width + UiConfiguration::instance->marginSize);
	}
	else {
		if (leftItemPanel) {
			leftItemPanel->reflow ();
			leftItemPanel->setFixedSize (true, leftItemPanel->extentX2 < maxleftw ? leftItemPanel->extentX2 : maxleftw, leftItemPanel->extentY2);
			leftItemPanel->position.assign (x, y);
			leftItemPanel->isVisible = true;
			if (h < leftItemPanel->height) {
				h = leftItemPanel->height;
			}
			x += (leftItemPanel->width + UiConfiguration::instance->marginSize);
			maxrightw -= (leftItemPanel->width + UiConfiguration::instance->marginSize);
		}
	}

	x = barWidth - widthPadding;
	if (rightCornerPanel) {
		x -= rightCornerPanel->width;
		rightCornerPanel->position.assign (x, y);
		x -= UiConfiguration::instance->marginSize;
		if (h < rightCornerPanel->height) {
			h = rightCornerPanel->height;
		}
	}

	if ((! modeitem) && rightItemPanel) {
		if (maxrightw <= 0.0f) {
			rightItemPanel->isVisible = false;
		}
		else {
			rightItemPanel->reflow ();
			rightItemPanel->setFixedSize (true, rightItemPanel->extentX2 < maxrightw ? rightItemPanel->extentX2 : maxrightw, rightItemPanel->extentY2);
			x -= rightItemPanel->width;
			rightItemPanel->position.assign (x, y);
			if (h < rightItemPanel->height) {
				h = rightItemPanel->height;
			}
			rightItemPanel->isVisible = true;
		}
	}

	h += (heightPadding * 2.0f);
	setFixedSize (true, barWidth, h);

	if (modeitem) {
		modeitem->position.assignY ((h / 2.0f) - (modeitem->height / 2.0f));
	}
	else {
		if (leftItemPanel) {
			leftItemPanel->position.assignY ((h / 2.0f) - (leftItemPanel->height / 2.0f));
		}
		if (rightItemPanel) {
			rightItemPanel->position.assignY ((h / 2.0f) - (rightItemPanel->height / 2.0f));
		}
	}
	if (leftCornerPanel) {
		leftCornerPanel->position.assignY ((h / 2.0f) - (leftCornerPanel->height / 2.0f));
	}
	if (rightCornerPanel) {
		rightCornerPanel->position.assignY ((h / 2.0f) - (rightCornerPanel->height / 2.0f));
	}
}

void Toolbar::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	modeMenuHandle.compact ();
}

double Toolbar::getLeftWidth () {
	double w;

	w = barWidth - (widthPadding * 2.0f);
	if (rightItemPanel) {
		w -= (rightItemPanel->width + UiConfiguration::instance->marginSize);
	}
	if (rightCornerPanel) {
		w -= (rightCornerPanel->width + UiConfiguration::instance->marginSize);
	}
	return (w);
}

void Toolbar::clearModes () {
	std::map<int, Toolbar::Mode>::iterator i1, i2;

	i1 = modeMap.begin ();
	i2 = modeMap.end ();
	while (i1 != i2) {
		if (i1->second.toolbar) {
			i1->second.toolbar->isDestroyed = true;
			i1->second.toolbar->release ();
			i1->second.toolbar = NULL;
		}
		i1->second.iconSprite = NULL;
		++i1;
	}
	modeMap.clear ();
	modeCount = 0;
	isModeActive = false;
}

void Toolbar::setModeContent (int modeNumber, Toolbar *modeToolbar, const StdString &modeName, Sprite *modeIconSprite) {
	std::map<int, Toolbar::Mode>::iterator pos;
	Toolbar::Mode *m;

	if (! modeToolbar) {
		modeToolbar = new Toolbar (barWidth);
	}
	pos = modeMap.find (modeNumber);
	if (pos == modeMap.end ()) {
		modeMap.insert (std::pair<int, Toolbar::Mode> (modeNumber, Toolbar::Mode ()));
		pos = modeMap.find (modeNumber);
	}
	m = &(pos->second);
	if (m->toolbar != modeToolbar) {
		if (m->toolbar) {
			m->toolbar->isDestroyed = true;
			m->toolbar->release ();
		}
		m->toolbar = modeToolbar;
		m->toolbar->retain ();
		m->toolbar->setFixedPadding (true, 0.0f, 0.0f);
		m->toolbar->setFillBg (false);
		m->toolbar->isVisible = false;
		addWidget (m->toolbar);
	}
	m->name.assign (modeName.empty () ? "---" : modeName);
	m->iconSprite = modeIconSprite;

	modeCount = (int) modeMap.size ();
	if (! isModeActive) {
		if (! modeButton) {
			setModeButton (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_appMenuButton));
		}
		setMode (modeNumber, true);
	}
}

void Toolbar::setModeButton (Sprite *sprite, const StdString &buttonWidgetName) {
	Button *button;

	button = new Button (sprite);
	button->mouseClickCallback = Widget::EventCallbackContext (Toolbar::modeButtonClicked, this);
	button->setInverseColor (true);
	button->setMouseHoverTooltip (UiText::instance->getText (UiTextId::ToolbarModeButtonTooltip), Widget::LeftAlignment);
	if (! buttonWidgetName.empty ()) {
		button->widgetName.assign (buttonWidgetName);
	}
	setRightCorner (button);
	modeButtonHandle.destroyAndAssign (button);
}

void Toolbar::setMode (int modeNumber, bool shouldSkipModeChangeCallback) {
	std::map<int, Toolbar::Mode>::iterator pos;

	pos = modeMap.find (modeNumber);
	if (pos == modeMap.end ()) {
		return;
	}
	if (isModeActive && (mode == modeNumber)) {
		return;
	}
	pos = modeMap.find (mode);
	if (pos != modeMap.end ()) {
		pos->second.toolbar->isVisible = false;
	}
	isModeActive = true;
	mode = modeNumber;
	reflow ();
	if (! shouldSkipModeChangeCallback) {
		eventCallback (modeChangeCallback);
	}
}

void Toolbar::modeButtonClicked (void *itPtr, Widget *widgetPtr) {
	((Toolbar *) itPtr)->handleModeButtonClick ();
}

void Toolbar::handleModeButtonClick () {
	Menu *menu;
	std::map<int, Toolbar::Mode>::iterator i1, i2;
	double x, y;

	if (modeMenu) {
		modeMenuHandle.destroyAndClear ();
		return;
	}
	menu = new Menu ();
	menu->isClickDestroyEnabled = true;
	i1 = modeMap.begin ();
	i2 = modeMap.end ();
	while (i1 != i2) {
		menu->addItem (i1->second.name, i1->second.iconSprite, Widget::EventCallbackContext (Toolbar::modeMenuItemClicked, this), 0, mode == i1->first);
		++i1;
	}
	modeMenuHandle.assign (menu);
	x = screenX + width;
	y = screenY - menu->height;
	if (modeButton) {
		x = modeButton->screenX + modeButton->width - menu->width;
		y = modeButton->screenY - menu->height;
	}
	App::instance->rootPanel->addWidget (menu, Position (x, y), App::instance->rootPanel->maxWidgetZLevel + 1);
}

void Toolbar::modeMenuItemClicked (void *itPtr, Widget *widgetPtr) {
	((Toolbar *) itPtr)->handleModeMenuItemClick (((Menu *) widgetPtr)->selectedItemName);
}

void Toolbar::handleModeMenuItemClick (const StdString &itemName) {
	std::map<int, Toolbar::Mode>::iterator i1, i2;

	i1 = modeMap.begin ();
	i2 = modeMap.end ();
	while (i1 != i2) {
		if (i1->second.name.equals (itemName)) {
			setMode (i1->first);
			break;
		}
		++i1;
	}
}
