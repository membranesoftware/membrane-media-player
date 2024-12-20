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
// Utility methods related to media files and streams
#ifndef MEDIA_UTIL_H
#define MEDIA_UTIL_H

extern "C" {
#include "libavutil/pixfmt.h"
}

class MediaUtil {
public:
	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static const constexpr double defaultAspectRatio = 16.0f / 9.0f;
	static const constexpr int avioBufferSize = 8192;
	static const constexpr int avImageAllocAlign = 32;
	static const constexpr AVPixelFormat swsRenderPixelFormat = AV_PIX_FMT_BGR32;
	static const constexpr SDL_PixelFormatEnum swsRenderPixelFormatSdl = SDL_PIXELFORMAT_XBGR8888;
	static const constexpr AVPixelFormat swsEncodePixelFormat = AV_PIX_FMT_YUV420P;

	// Return the av_strerror text for an ffmpeg error number
	static StdString avStrerror (int errnum);

	// Return the four-character string representation of fourccValue
	static StdString fourcc (uint32_t fourccValue);

	struct SdlRwOps {
		SDL_RWops *rwops;
		uint64_t rwopsSize;
		SdlRwOps ():
			rwops (NULL),
			rwopsSize (0) { }
	};
	// avio_alloc_context functions, with opaque as a pointer to MediaUtil::SdlRwOps
	static int avioReadPacket (void *opaque, uint8_t *buf, int buf_size);
	static int64_t avioSeek (void *opaque, int64_t offset, int whence);

	// Return a string containing the name of the specified aspect ratio, or an empty string if no such name was found.
	static StdString getAspectRatioDisplayString (int width, int height);
	static StdString getAspectRatioDisplayString (double ratio);

	// Return a string containing text representing the specified bitrate in readable format
	static StdString getBitrateDisplayString (int64_t bitsPerSecond);

	// Return a string containing the name of the specified frame size, or an empty string if no such name was found.
	static StdString getFrameSizeName (int width, int height);

	// Return a boolean value indicating if the provided extension indicates a media file
	static bool isMediaFileExtension (const StdString &extension);
};
#endif
