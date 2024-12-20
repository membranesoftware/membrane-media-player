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
#include "MathUtil.h"
#include "SliderThumbSprite.h"

SliderThumbSprite::SliderThumbSprite ()
: Sprite ()
{
}
SliderThumbSprite::~SliderThumbSprite () {
}

OpResult SliderThumbSprite::load () {
	return (Sprite::load (4.0f, SliderThumbSprite::maxThumbSize, 2.0f));
}

void SliderThumbSprite::getRenderFrameSize (double scale, double *frameWidth, double *frameHeight) {
	if (frameWidth) {
		*frameWidth = ceil (scale / 2.0f) + 1.0f;
	}
	if (frameHeight) {
		*frameHeight = scale + 1.0f;
	}
}

void SliderThumbSprite::writeRenderPixels (double scale, Uint32 *pixels, double frameWidth, double frameHeight) {
	Uint32 *dest, color;
	double dist, targetalpha, framedist, cx, cy, x, y;
	uint8_t alpha;

	if (scale <= 0.0f) {
		return;
	}
	const double minalpha = 0.02f;
	const double opacity = 6.0f;
	dest = pixels;
	cx = frameWidth / 2.0f;
	cy = frameHeight / 2.0f;
	framedist = MathUtil::getDistance (cx, cy);
	y = 0.0f;
	while (y < frameHeight) {
		x = 0.0f;
		while (x < frameWidth) {
			dist = MathUtil::getDistance (2.0f * (cx - (x + 0.5f)), cy - (y + 0.5f), 0.0f, 0.0f);
			targetalpha = 1.0f - ((1.0f - minalpha) * (dist / framedist));
			if (targetalpha <= 0.0f) {
				targetalpha = 0.0f;
			}
			else {
				targetalpha *= opacity;
				if (targetalpha > 1.0f) {
					targetalpha = 1.0f;
				}
			}
			alpha = (uint8_t) (targetalpha * 255.0f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			color = 0xFFFFFF00 | (alpha & 0xFF);
#else
			color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
			*dest = color;
			++dest;
			x += 1.0f;
		}
		y += 1.0f;
	}
}
