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
#include "SpriteId.h"
#include "SpriteGroup.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "OsUtil.h"
#include "Color.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Button.h"
#include "Label.h"
#include "ProgressRing.h"
#include "MainToolbarWindow.h"

MainToolbarWindow::MainToolbarWindow ()
: Panel ()
, isClockEnabled (false)
, networkActivityHideClock (0)
, uiActivityShowClock (0)
, lastDisplayTime (0)
{
	Image *image;
	ProgressRing *ring;

	logoImage = add (new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_appLogo)));
	logoImage->setDrawColor (true, UiConfiguration::instance->mediumSecondaryColor);

	image = new Image (SpriteGroup::instance->getSprite (SpriteId::SpriteGroup_networkActivityIcon));
	image->setDrawColor (true, UiConfiguration::instance->mediumSecondaryColor);
	image->drawColor.animate (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->darkBackgroundColor, UiConfiguration::instance->shortColorAnimateDuration);
	networkActivityImage = add (new ImageWindow (image));
	networkActivityImage->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	networkActivityImage->setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
	networkActivityImage->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	networkActivityImage->setWindowSize (true, logoImage->width, logoImage->height);
	networkActivityImage->borderColor.animate (UiConfiguration::instance->lightInverseBackgroundColor, UiConfiguration::instance->darkBackgroundColor, UiConfiguration::instance->shortColorAnimateDuration);
	networkActivityImage->isVisible = false;

	uiActivityProgressRingPanel = add (new Panel ());
	uiActivityProgressRingPanel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha));
	uiActivityProgressRingPanel->setBorder (true, UiConfiguration::instance->lightInverseBackgroundColor);
	uiActivityProgressRingPanel->setDropShadow (true, UiConfiguration::instance->dropShadowColor, UiConfiguration::instance->dropShadowWidth);
	uiActivityProgressRingPanel->setFixedSize (true, logoImage->width, logoImage->height);
	uiActivityProgressRingPanel->borderColor.animate (UiConfiguration::instance->lightInverseBackgroundColor, UiConfiguration::instance->darkBackgroundColor, UiConfiguration::instance->shortColorAnimateDuration);
	ring = uiActivityProgressRingPanel->add (new ProgressRing (UiConfiguration::instance->progressRingSize));
	ring->setIndeterminate (true);
	ring->position.assign ((uiActivityProgressRingPanel->width / 2.0f) - (ring->width / 2.0f), (uiActivityProgressRingPanel->height / 2.0f) - (ring->height / 2.0f));
	uiActivityProgressRingPanel->isVisible = false;
	uiActivityShowClock = UiConfiguration::instance->activityIconLingerDuration;

	clockPanel = add (new Panel ());
	clockPanel->setFixedPadding (true, 0.0f, 0.0f);
	dateLabel = clockPanel->add (new Label (UiText::instance->getDateText (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor));
	timeLabel = clockPanel->add (new Label (StdString (), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor));
	timeLabel->setText (UiText::instance->getTimeText ());
	clockPanel->isVisible = false;

	reflow ();
}
MainToolbarWindow::~MainToolbarWindow () {
}

void MainToolbarWindow::setClockEnabled (bool enable) {
	int64_t t;

	if (isClockEnabled == enable) {
		return;
	}
	isClockEnabled = enable;
	if (isClockEnabled) {
		t = OsUtil::getTime () / 1000;
		dateLabel->setText (UiText::instance->getDateText (t * 1000));
		timeLabel->setText (UiText::instance->getTimeText (t * 1000));
		lastDisplayTime = t;
		clockPanel->isVisible = true;
	}
	else {
		clockPanel->isVisible = false;
	}
	reflow ();
}

void MainToolbarWindow::reflow () {
	double x, y;

	x = 0.0f;
	y = 0.0f;
	logoImage->position.assign (x, y);
	x += logoImage->width + UiConfiguration::instance->marginSize;
	if (networkActivityImage->isVisible) {
		networkActivityImage->position.assign (logoImage->position);
	}
	if (uiActivityProgressRingPanel->isVisible) {
		uiActivityProgressRingPanel->position.assign (logoImage->position);
	}

	if (clockPanel->isVisible) {
		y = (UiConfiguration::instance->textLineHeightMargin * 2.0f);
		dateLabel->position.assign (0.0f, dateLabel->getLinePosition (y));
		y += dateLabel->maxLineHeight + UiConfiguration::instance->textLineHeightMargin;
		timeLabel->position.assign (0.0f, timeLabel->getLinePosition (y));
		clockPanel->reflow ();
		y = 0.0f;
		clockPanel->position.assign (x, y);
	}

	resetSize ();
}

void MainToolbarWindow::doUpdate (int msElapsed) {
	int64_t t;
	bool shouldreflow;

	Panel::doUpdate (msElapsed);
	shouldreflow = false;

	if (! uiActivityProgressRingPanel->isVisible) {
		if (App::instance->uiActivityCount > 0) {
			uiActivityShowClock -= msElapsed;
			if (uiActivityShowClock <= 0) {
				uiActivityProgressRingPanel->isVisible = true;
				shouldreflow = true;
			}
		}
		else {
			uiActivityShowClock = UiConfiguration::instance->activityIconLingerDuration;
		}
	}
	else {
		if (App::instance->uiActivityCount <= 0) {
			uiActivityProgressRingPanel->isVisible = false;
			shouldreflow = true;
		}
	}

	if (! networkActivityImage->isVisible) {
		if ((! uiActivityProgressRingPanel->isVisible) && (App::instance->networkActivityCount > 0)) {
			networkActivityImage->isVisible = true;
			networkActivityHideClock = UiConfiguration::instance->activityIconLingerDuration;
			shouldreflow = true;
		}
	}
	else {
		if (uiActivityProgressRingPanel->isVisible) {
			networkActivityImage->isVisible = false;
			shouldreflow = true;
		}
		else {
			if (App::instance->networkActivityCount <= 0) {
				networkActivityHideClock -= msElapsed;
				if (networkActivityHideClock <= 0) {
					networkActivityImage->isVisible = false;
					shouldreflow = true;
				}
			}
			else {
				networkActivityHideClock = UiConfiguration::instance->activityIconLingerDuration;
			}
		}
	}

	if (clockPanel->isVisible) {
		t = OsUtil::getTime () / 1000;
		if (lastDisplayTime != t) {
			lastDisplayTime = t;
			dateLabel->setText (UiText::instance->getDateText (t * 1000));
			timeLabel->setText (UiText::instance->getTimeText (t * 1000));
			shouldreflow = true;
		}
	}

	if (shouldreflow) {
		reflow ();
	}
}
