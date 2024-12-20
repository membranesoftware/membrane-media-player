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
#include "Ui.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Color.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Menu.h"

Menu::Menu ()
: Panel ()
, isClickDestroyEnabled (false)
, selectionMarginSize (0.0f)
, itemHeight (0.0f)
, isItemFocused (false)
, lastFocusPanel (NULL)
{
	classId = ClassId::Menu;
	setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
	setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);

	focusBackgroundPanel = add (new Panel ());
	focusBackgroundPanel->zLevel = Widget::minZLevel;
	focusBackgroundPanel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	focusBackgroundPanel->isVisible = false;
}
Menu::~Menu () {
}

Menu *Menu::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::Menu) ? (Menu *) widget : NULL);
}

void Menu::addItem (const StdString &name, Sprite *sprite, const Widget::EventCallbackContext &callback, int selectionGroup, bool isSelected) {
	Menu::Item item;
	Panel *panel;
	Image *image;
	double w;

	item.isChoice = true;
	panel = new Panel ();
	panel->zLevel = 1;
	addWidget (panel);
	item.panel = panel;

	item.label = panel->add (new Label (name, UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	if (sprite) {
		item.image = panel->add (new Image (sprite));
		item.image->setDrawColor (true, Color (0.0f, 0.0f, 0.0f));
	}
	item.callback = callback;

	item.selectionGroup = selectionGroup;
	if (item.selectionGroup >= 0) {
		item.isSelected = isSelected;
		if (checkmarkImageMap.count (item.selectionGroup) <= 0) {
			image = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_checkmark)));
			image->isVisible = false;
			image->zLevel = panel->zLevel + 1;
			w = image->width + (UiConfiguration::instance->paddingSize / 2.0f);
			if (selectionMarginSize < w) {
				selectionMarginSize = w;
			}
			checkmarkImageMap.insert (std::pair<int, Image *> (item.selectionGroup, image));
		}
	}

	if (item.isSelected) {
		unselectItemGroup (item.selectionGroup);
	}

	itemList.push_back (item);
	reflow ();
}

void Menu::addDivider () {
	Menu::Item item;
	Panel *panel;

	item.isDivider = true;
	panel = new Panel ();
	panel->setFillBg (true, UiConfiguration::instance->primaryTextColor);
	panel->setFixedSize (true, width, UiConfiguration::instance->menuDividerLineWidth);
	addWidget (panel);

	item.panel = panel;

	itemList.push_back (item);
	reflow ();
}

bool Menu::contains (const StdString &itemName) const {
	std::list<Menu::Item>::const_iterator i1, i2;

	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->isChoice && i1->label && i1->label->text.equals (itemName)) {
			return (true);
		}
		++i1;
	}
	return (false);
}

void Menu::selectItem (const StdString &itemName) {
	std::list<Menu::Item>::const_iterator i1, i2;

	i1 = itemList.cbegin ();
	i2 = itemList.cend ();
	while (i1 != i2) {
		if (i1->isChoice && i1->label && i1->label->text.equals (itemName)) {
			selectedItemName.assign (i1->label->text);
			eventCallback (i1->callback);
			isDestroyed = true;
			break;
		}
		++i1;
	}
}

bool Menu::doProcessMouseState (const Widget::MouseState &mouseState) {
	std::list<Menu::Item>::iterator i1, i2;
	bool shouldreflow;

	shouldreflow = false;
	if (! mouseState.isEntered) {
		if (isItemFocused) {
			isItemFocused = false;
			lastFocusPanel = NULL;
			shouldreflow = true;
		}
	}
	else {
		i1 = itemList.begin ();
		i2 = itemList.end ();
		while (i1 != i2) {
			if (i1->isChoice) {
				if ((mouseState.enterDeltaY >= i1->panel->position.y) && (mouseState.enterDeltaY < (i1->panel->position.y + i1->panel->height))) {
					if ((! isItemFocused) || (i1->panel != lastFocusPanel)) {
						isItemFocused = true;
						lastFocusPanel = i1->panel;
						focusBackgroundPanel->position.assign (i1->panel->position);
						focusBackgroundPanel->setFixedSize (true, i1->panel->width, i1->panel->height);
						shouldreflow = true;
					}

					if (mouseState.isLeftClickReleased && mouseState.isLeftClickEntered) {
						if ((i1->selectionGroup >= 0) && (! i1->isSelected)) {
							unselectItemGroup (i1->selectionGroup);
							i1->isSelected = true;
							shouldreflow = true;
						}
						selectedItemName.assign (i1->label->text);
						eventCallback (i1->callback);
					}
					break;
				}
			}
			++i1;
		}
	}

	if (isClickDestroyEnabled) {
		if (mouseState.isLeftClickReleased) {
			isDestroyed = true;
		}
	}

	if (shouldreflow && (! isDestroyed)) {
		reflow ();
	}
	return (false);
}

void Menu::reflow () {
	std::list<Menu::Item>::iterator i1, i2;
	std::map<int, Image *>::iterator j1, j2;
	std::map<int, bool> checkfoundmap;
	Label *label;
	Image *image;
	Panel *panel;
	double x0, x, y, w, h, maxw, maxh, maximagew, padw, padh;

	x0 = UiConfiguration::instance->paddingSize + selectionMarginSize;
	maxw = 0.0f;
	maxh = 0.0f;
	maximagew = 0.0f;
	padw = UiConfiguration::instance->paddingSize;
	padh = UiConfiguration::instance->paddingSize;

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->isChoice) {
			image = i1->image;
			if (image && (image->width > maximagew)) {
				maximagew = image->width;
			}
		}
		++i1;
	}

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->isChoice) {
			label = i1->label;
			image = i1->image;

			w = label->width;
			if (image) {
				w += UiConfiguration::instance->marginSize + maximagew;
			}
			h = label->height;
			if (image && (image->height > h)) {
				h = image->height;
			}
			if (w > maxw) {
				maxw = w;
			}
			if (h > maxh) {
				maxh = h;
			}
		}
		++i1;
	}

	maxw += UiConfiguration::instance->paddingSize + selectionMarginSize;
	maxh += UiConfiguration::instance->paddingSize;
	itemHeight = maxh;

	y = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->isChoice) {
			label = i1->label;
			image = i1->image;
			panel = i1->panel;
			x = x0;
			if (image) {
				image->position.assign (x, (maxh / 2.0f) - (image->height / 2.0f));
				x += maximagew + UiConfiguration::instance->marginSize;
			}
			label->position.assign (x, (maxh / 2.0f) - (label->height / 2.0f));
			panel->setFixedSize (true, maxw, maxh);
			panel->position.assign (0.0f, y);

			if (i1->selectionGroup >= 0) {
				if (i1->isSelected) {
					j1 = checkmarkImageMap.find (i1->selectionGroup);
					if (j1 != checkmarkImageMap.end ()) {
						checkfoundmap.insert (std::pair<int, bool> (i1->selectionGroup, true));
						j1->second->position.assign (UiConfiguration::instance->paddingSize / 2.0f, y + (maxh / 2.0f) - (j1->second->height / 2.0f));
					}
				}
			}

			y += panel->height;
			padh = 0.0f;
		}
		else if (i1->isDivider) {
			panel = i1->panel;
			y += (UiConfiguration::instance->marginSize / 2.0f);
			panel->position.assign (0.0f, y);
			panel->setFixedSize (true, maxw + (UiConfiguration::instance->paddingSize * 2.0f), UiConfiguration::instance->menuDividerLineWidth);
			y += panel->height + (UiConfiguration::instance->marginSize / 2.0f);
			padw = 0.0f;
			padh = UiConfiguration::instance->paddingSize;
		}
		++i1;
	}

	j1 = checkmarkImageMap.begin ();
	j2 = checkmarkImageMap.end ();
	while (j1 != j2) {
		image = j1->second;
		if (checkfoundmap.count (j1->first) <= 0) {
			image->isVisible = false;
		}
		else {
			image->isVisible = true;
		}
		++j1;
	}

	focusBackgroundPanel->isVisible = false;
	resetSize ();

	w = extentX2 + padw;
	h = extentY2 + padh;
	if (! isItemFocused) {
		focusBackgroundPanel->isVisible = false;
	}
	else {
		focusBackgroundPanel->setFixedSize (true, w, focusBackgroundPanel->height);
		focusBackgroundPanel->isVisible = true;
	}
	setFixedSize (true, w, h);
}

void Menu::unselectItemGroup (int selectionGroup) {
	std::list<Menu::Item>::iterator i1, i2;

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->selectionGroup == selectionGroup) {
			i1->isSelected = false;
		}
		++i1;
	}
}
