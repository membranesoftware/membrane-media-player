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
// Widget that draws image data from a Sprite resource
#ifndef IMAGE_H
#define IMAGE_H

#include "Position.h"
#include "Color.h"
#include "Widget.h"

class Sprite;

class Image : public Widget {
public:
	Image (Sprite *sprite, int spriteFrame = 0, bool shouldDestroySprite = false);
	~Image ();

	// Read-write data members
	double drawAlpha;
	bool isResizeDisabled;

	// Read-only data members
	Sprite *sprite;
	SDL_Texture *spriteTexture;
	StdString spriteTexturePath;
	int spriteTextureWidth;
	int spriteTextureHeight;
	int spriteFrame;
	double maxSpriteWidth;
	double maxSpriteHeight;
	double drawScale;
	bool isDrawColorEnabled;
	Color drawColor;

	// Set the sprite object used to draw the image
	void setSprite (Sprite *targetSprite, int frame = 0);

	// Set the sprite frame for use in drawing the image
	void setFrame (int frame);

	// Set the scale factor for use in drawing the image
	void setScale (double scale);

	// Set the image's sprite destroy option. If enabled, the image unloads and destroys its source sprite when itself destroyed.
	void setSpriteDestroy (bool enable);

	// Set the image's draw color option. If enabled, the image renders its texture with a filter of the specified color.
	void setDrawColor (bool enable, const Color &color = Color ());

	// Begin an operation to change the image's draw alpha value over time
	void translateAlpha (double startAlpha, double targetAlpha, int durationMs);

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doDraw (double originX, double originY);
	void doResize ();

	// Reset the widget's width and height values
	void resetSize ();

	SDL_mutex *spriteTextureMutex;
	Position translateAlphaValue;
	bool shouldDestroySprite;
};
#endif
