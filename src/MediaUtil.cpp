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
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avio.h"
#include "libavutil/log.h"
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#include "Log.h"
#include "MediaUtil.h"

static const char *mediaFileExtensions[] = {
	"avi",
	"mp4",
	"wmv",
	"mkv",
	"vob",
	"mpg",
	"mpeg",
	"mpeg4",
	"m4v",
	"mov",
	"flv",
	"ogv",
	"webm",
	"divx",
	"mp3",
	"ogg",
	"wma"
};
static std::map<StdString, bool> mediaFileExtensionMap;

constexpr const double aspectRatioMatchEpsilon = 0.1f;

static void avLog (void *avcl, int level, const char *fmt, va_list vl) {
	bool show;
	char line[4096];
	int print_prefix, len;
	Log::LogLevel loglevel;

	show = false;
	loglevel = Log::Debug3Level;
	switch (level) {
		case AV_LOG_PANIC:
		case AV_LOG_FATAL:
		case AV_LOG_ERROR:
		case AV_LOG_WARNING: {
			loglevel = Log::DebugLevel;
			show = true;
			break;
		}
		case AV_LOG_INFO: {
			show = true;
			break;
		}
	}
	if (! show) {
		return;
	}

	print_prefix = 1;
	len = av_log_format_line2 (avcl, level, fmt, vl, line, sizeof (line), &print_prefix);
	if (len < 0) {
		Log::debug ("ffmpeg log message failed, av_log_format_line2 error %i", len);
		return;
	}
	--len;
	while (len >= 0) {
		if ((line[len] != '\r') && (line[len] != '\n')) {
			break;
		}
		line[len] = '\0';
		--len;
	}
	Log::write (loglevel, "ffmpeg.%i: %s", level, line);
}

void MediaUtil::createInstance () {
	int i, count;

	av_log_set_callback (avLog);

	count = (int) (sizeof (mediaFileExtensions) / sizeof (char *));
	for (i = 0; i < count; ++i) {
		mediaFileExtensionMap.insert (std::pair<StdString, bool> (StdString (mediaFileExtensions[i]), true));
	}
}

void MediaUtil::freeInstance () {
	av_log_set_callback (av_log_default_callback);
	mediaFileExtensionMap.clear ();
}

StdString MediaUtil::avStrerror (int errnum) {
	char errbuf[4096];

	av_strerror (errnum, errbuf, sizeof (errbuf));
	return (StdString (errbuf));
}

StdString MediaUtil::fourcc (uint32_t fourccValue) {
	char s[AV_FOURCC_MAX_STRING_SIZE];

	av_fourcc_make_string (s, fourccValue);
	return (StdString (s));
}

int MediaUtil::avioReadPacket (void *opaque, uint8_t *buf, int buf_size) {
	MediaUtil::SdlRwOps *it = (MediaUtil::SdlRwOps *) opaque;
	int result;

	result = SDL_RWread (it->rwops, buf, 1, buf_size);
	if (result <= 0) {
		return (AVERROR_EOF);
	}
	return (result);
}
int64_t MediaUtil::avioSeek (void *opaque, int64_t offset, int whence) {
	MediaUtil::SdlRwOps *it = (MediaUtil::SdlRwOps *) opaque;
	int rwopswhence;

	if (whence & AVSEEK_SIZE) {
		return ((int64_t) it->rwopsSize);
	}
	rwopswhence = RW_SEEK_CUR;
	switch (whence) {
		case SEEK_END: {
			rwopswhence = RW_SEEK_END;
			break;
		}
		case SEEK_SET: {
			rwopswhence = RW_SEEK_SET;
			break;
		}
	}
	return ((int64_t) SDL_RWseek (it->rwops, offset, rwopswhence));
}

StdString MediaUtil::getAspectRatioDisplayString (int width, int height) {
	return (MediaUtil::getAspectRatioDisplayString ((double) width / (double) height));
}

StdString MediaUtil::getAspectRatioDisplayString (double ratio) {
	if (fabs (ratio - (16.0f / 9.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("16:9"));
	}
	if (fabs (ratio - (4.0f / 3.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("4:3"));
	}
	if (fabs (ratio - (3.0f / 2.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("3:2"));
	}
	if (fabs (ratio - (5.0f / 3.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("5:3"));
	}
	if (fabs (ratio - (5.0f / 4.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("5:4"));
	}
	if (fabs (ratio - (8.0f / 5.0f)) <= aspectRatioMatchEpsilon) {
		return (StdString ("8:5"));
	}
	if (fabs (ratio - 1.0f) <= aspectRatioMatchEpsilon) {
		return (StdString ("1:1"));
	}
	if (fabs (ratio - 1.85f) <= aspectRatioMatchEpsilon) {
		return (StdString ("1.85:1"));
	}
	if (fabs (ratio - 3.0f) <= aspectRatioMatchEpsilon) {
		return (StdString ("3:1"));
	}
	return (StdString ());
}

StdString MediaUtil::getBitrateDisplayString (int64_t bitsPerSecond) {
	if (bitsPerSecond <= 0) {
		return (StdString ("0kbps"));
	}
	if (bitsPerSecond < 1024) {
		return (StdString::createSprintf ("%ibps", (int) bitsPerSecond));
	}
	return (StdString::createSprintf ("%llikbps", (long long int) (bitsPerSecond / 1024)));
}

StdString MediaUtil::getFrameSizeName (int width, int height) {
	if ((width == 3840) && (height == 2160)) {
		return (StdString ("4K Ultra HD 1"));
	}
	if ((width == 1920) && (height == 1280)) {
		return (StdString ("Full HD Plus"));
	}
	if ((width == 1920) && (height == 1080)) {
		return (StdString ("Full HD"));
	}
	if ((width == 1600) && (height == 1200)) {
		return (StdString ("Ultra XGA"));
	}
	if ((width == 1600) && (height == 900)) {
		return (StdString ("HD+"));
	}
	if ((width == 1280) && (height == 1024)) {
		return (StdString ("Super XGA"));
	}
	if ((width == 1280) && (height == 720)) {
		return (StdString ("720p HD"));
	}
	if ((width == 1280) && (height == 800)) {
		return (StdString ("Wide XGA"));
	}
	if ((width == 1024) && (height == 768)) {
		return (StdString ("XGA"));
	}
	if ((width == 960) && (height == 540)) {
		return (StdString ("qHD"));
	}
	if ((width == 800) && (height == 600)) {
		return (StdString ("Super VGA"));
	}
	if ((width == 640) && (height == 480)) {
		return (StdString ("VGA"));
	}
	if ((width == 432) && (height == 240)) {
		return (StdString ("Wide QVGA"));
	}
	if ((width == 320) && (height == 240)) {
		return (StdString ("QVGA"));
	}
	if ((width == 320) && (height == 200)) {
		return (StdString ("CGA"));
	}
	if ((width == 240) && (height == 160)) {
		return (StdString ("HQVGA"));
	}
	if ((width == 160) && (height == 120)) {
		return (StdString ("QQVGA"));
	}
	return (StdString ());
}

bool MediaUtil::isMediaFileExtension (const StdString &extension) {
	return (mediaFileExtensionMap.count (extension.lowercased ()) > 0);
}
