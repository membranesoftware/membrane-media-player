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
#include "Label.h"
#include "UiConfiguration.h"
#include "StatsWindow.h"

StatsWindow::StatsWindow ()
: Panel ()
, isEmptyItemValueIgnored (false)
, maxLineHeight (0.0f)
{
}
StatsWindow::~StatsWindow () {
}

void StatsWindow::clearItems () {
	std::list<StatsWindow::Item>::iterator i1, i2;

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->nameLabel) {
			i1->nameLabel->isDestroyed = true;
		}
		if (i1->valueLabel) {
			i1->valueLabel->isDestroyed = true;
		}
		++i1;
	}
	itemList.clear ();
	maxLineHeight = 0.0f;
	resetSize ();
}

void StatsWindow::setItem (const StdString &itemName, const StdString &itemValue, const StdString &itemValueWidgetName) {
	std::list<StatsWindow::Item>::iterator i1, i2;
	StatsWindow::Item item;
	Label *label;
	bool found;

	if (isEmptyItemValueIgnored && itemValue.empty ()) {
		return;
	}
	found = false;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->nameLabel->text.equals (itemName)) {
			found = true;
			label = i1->valueLabel;
			if (! label->text.equals (itemValue)) {
				if (! label->text.empty ()) {
					label->textColor.assign (UiConfiguration::instance->primaryTextColor);
					label->textColor.translate (UiConfiguration::instance->lightPrimaryTextColor, UiConfiguration::instance->longColorTranslateDuration);
				}
				label->setText (itemValue);
				reflow ();
			}
			if (! itemValueWidgetName.empty ()) {
				label->widgetName.assign (itemValueWidgetName);
			}
			break;
		}
		++i1;
	}

	if (! found) {
		label = add (new Label (itemName, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
		if (label->maxLineHeight > maxLineHeight) {
			maxLineHeight = label->maxLineHeight;
		}
		item.nameLabel = label;

		label = add (new Label (itemValue, UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
		if (label->maxLineHeight > maxLineHeight) {
			maxLineHeight = label->maxLineHeight;
		}
		item.valueLabel = label;
		if (! itemValueWidgetName.empty ()) {
			label->widgetName.assign (itemValueWidgetName);
		}

		itemList.push_back (item);
		reflow ();
	}
}

void StatsWindow::setItemTextColor (const StdString &itemName, const Color &itemTextColor) {
	std::list<StatsWindow::Item>::iterator i1, i2;

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->nameLabel->text.equals (itemName)) {
			i1->valueLabel->textColor.assign (itemTextColor);
			break;
		}
		++i1;
	}
}

void StatsWindow::doResize () {
	std::list<StatsWindow::Item>::iterator i1, i2;

	Panel::doResize ();
	maxLineHeight = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		if (i1->nameLabel->maxLineHeight > maxLineHeight) {
			maxLineHeight = i1->nameLabel->maxLineHeight;
		}
		++i1;
	}
}

void StatsWindow::reflow () {
	std::list<StatsWindow::Item>::iterator i1, i2;
	Label *label;
	double x, y, w;

	resetPadding ();
	x = widthPadding;
	y = heightPadding;
	w = 0.0f;
	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		label = i1->nameLabel;
		if (label->width > w) {
			w = label->width;
		}
		++i1;
	}

	i1 = itemList.begin ();
	i2 = itemList.end ();
	while (i1 != i2) {
		label = i1->nameLabel;
		label->position.assign (x + w - label->width, label->getLinePosition (y));
		label = i1->valueLabel;
		label->position.assign (x + w + UiConfiguration::instance->marginSize, label->getLinePosition (y));
		y += maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		++i1;
	}

	resetSize ();
}
