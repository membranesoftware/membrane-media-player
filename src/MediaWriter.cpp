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
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libswscale/swscale.h"
}
#include "App.h"
#include "SdlUtil.h"
#include "MediaUtil.h"
#include "OsUtil.h"
#include "SharedBuffer.h"
#include "TaskGroup.h"
#include "UiText.h"
#include "MediaWriter.h"

MediaWriter::MediaWriter ()
: videoCodecName (MediaWriter::defaultVideoCodecName)
, videoWidth (0)
, videoHeight (0)
, videoExpectedFrameRate (0.0f)
, videoFrameDropLag (-1)
, videoBitrate (0)
, videoBitrateMultiplier (0.0f)
, videoRatecontrolRangeMultiplier (0.0f)
, videoRatecontrolBufferMultiplier (0.0f)
, videoGopRate (0.0f)
, videoMaxBframes (-1)
, isWriting (false)
, isWriteFailed (false)
, isRunning (false)
, isStopped (false)
, writeReferenceTime (0)
, frameReceiveCount (0)
, packetWriteCount (0)
, avFormatContext (NULL)
, videoCodec (NULL)
, videoCodecContext (NULL)
, videoStream (NULL)
, avPacket (NULL)
, avFrame (NULL)
, swsContext (NULL)
, swsSourceWidth (0)
, swsSourceHeight (0)
, refcount (0)
{
	videoPixelFormat = MediaUtil::swsEncodePixelFormat;
	SdlUtil::createMutex (&framesMutex);
	SdlUtil::createCond (&framesCond);
	SdlUtil::createMutex (&refcountMutex);
}
MediaWriter::~MediaWriter () {
	clearWrite ();
	clearFrames ();
	SdlUtil::destroyCond (&framesCond);
	SdlUtil::destroyMutex (&framesMutex);
	SdlUtil::destroyMutex (&refcountMutex);
}

void MediaWriter::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}
void MediaWriter::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);
	if (isdestroyed) {
		delete (this);
	}
}

void MediaWriter::clearWrite () {
	if (nextVideoFrame.imageData) {
		nextVideoFrame.imageData->release ();
		nextVideoFrame.imageData = NULL;
	}
	memset (videoFrameEncodeSlice, 0, sizeof (videoFrameEncodeSlice));
	memset (videoFrameEncodeStride, 0, sizeof (videoFrameEncodeStride));

	clearSwsContext ();
	if (avPacket) {
		av_packet_free (&avPacket);
		avPacket = NULL;
	}
	if (avFrame) {
		av_frame_free (&avFrame);
		avFrame = NULL;
	}
	if (videoCodecContext) {
		avcodec_free_context (&videoCodecContext);
		videoCodecContext = NULL;
	}
	if (avFormatContext) {
		avformat_close_input (&avFormatContext);
		avFormatContext = NULL;
	}
	videoStream = NULL;
}

void MediaWriter::clearSwsContext () {
	if (swsContext) {
		sws_freeContext (swsContext);
		swsContext = NULL;
	}
}

void MediaWriter::clearFrames () {
	std::list<MediaWriter::VideoFrame>::iterator i1, i2;

	SDL_LockMutex (framesMutex);
	i1 = frames.begin ();
	i2 = frames.end ();
	while (i1 != i2) {
		if (i1->imageData) {
			i1->imageData->release ();
			i1->imageData = NULL;
		}
		++i1;
	}
	frames.clear ();
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}

void MediaWriter::failWrite (const StdString &lastErrorMessageValue, const char *logErrorMessage) {
	isWriteFailed = true;
	lastErrorMessage.assign (lastErrorMessageValue);
	if (logErrorMessage) {
		Log::debug ("Stream publish failed: %s", logErrorMessage);
	}
}

void MediaWriter::start (const StdString &writePathValue) {
	double n;

	if (isRunning) {
		return;
	}
	if ((videoWidth % 2) != 0) {
		--videoWidth;
	}
	if ((videoHeight % 2) != 0) {
		--videoHeight;
	}
	if ((videoWidth < 2) || (videoHeight < 2)) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "Invalid stream video size");
		return;
	}
	if (videoFrameDropLag < 0) {
		videoFrameDropLag = MediaWriter::defaultVideoFrameDropLag;
	}
	if (videoExpectedFrameRate <= 0.0f) {
		videoExpectedFrameRate = MediaWriter::defaultVideoExpectedFrameRate;
	}
	if (videoBitrateMultiplier <= 0.0f) {
		videoBitrateMultiplier = MediaWriter::defaultVideoBitrateMultiplier;
	}
	if (videoRatecontrolRangeMultiplier <= 0.0f) {
		videoRatecontrolRangeMultiplier = MediaWriter::defaultVideoRatecontrolRangeMultiplier;
	}
	if (videoRatecontrolBufferMultiplier <= 0.0f) {
		videoRatecontrolBufferMultiplier = MediaWriter::defaultVideoRatecontrolBufferMultiplier;
	}
	if (videoBitrate <= 0) {
		n = (double) videoWidth * (double) videoHeight;
		n *= videoExpectedFrameRate;
		n *= videoBitrateMultiplier;
		videoBitrate = (int64_t) n;
		if (videoBitrate < 1) {
			videoBitrate = 1;
		}
	}
	if (videoGopRate <= 0.0f) {
		videoGopRate = MediaWriter::defaultVideoGopRate;
	}
	if (videoMaxBframes < 0) {
		videoMaxBframes = MediaWriter::defaultVideoMaxBframes;
	}
	writePath.assign (writePathValue);
	isStopped = false;
	writeReferenceTime = 0;
	frameReceiveCount = 0;
	packetWriteCount = 0;
	isRunning = true;
	retain ();
	if (! TaskGroup::instance->run (TaskGroup::RunContext (MediaWriter::writePackets, this))) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "TaskGroup run error");
		release ();
		return;
	}
}

void MediaWriter::stop () {
	isStopped = true;
	SDL_LockMutex (framesMutex);
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}

void MediaWriter::writePackets (void *itPtr) {
	MediaWriter *it = (MediaWriter *) itPtr;

	it->executeWritePackets ();
	it->clearWrite ();
	it->isWriting = false;
	it->isRunning = false;
	it->release ();
}
void MediaWriter::executeWritePackets () {
	std::list<MediaWriter::VideoFrame>::iterator i;
	int64_t now;
	int result;

	clearWrite ();
	avPacket = av_packet_alloc ();
	if (! avPacket) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_packet_alloc error");
		return;
	}
	avFrame = av_frame_alloc ();
	if (! avFrame) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_frame_alloc error");
		return;
	}
	avformat_alloc_output_context2 (&avFormatContext, NULL, NULL, writePath.c_str ());
	if (! avFormatContext) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avformat_alloc_output_context2 failed");
		return;
	}
	if (! resetSwsContext (App::instance->windowWidth, App::instance->windowHeight)) {
		return;
	}
	if (! addVideoStream ()) {
		return;
	}
	if (!(avFormatContext->oformat->flags & AVFMT_NOFILE)) {
		result = avio_open (&(avFormatContext->pb), writePath.c_str (), AVIO_FLAG_WRITE);
		if (result < 0) {
			failWrite (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), "Failed to open stream output file");
			return;
		}
	}
	result = avformat_write_header (avFormatContext, NULL);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), "Failed to write output stream");
		return;
	}

	isWriting = true;
	while (true) {
		if (isStopped || isWriteFailed) {
			break;
		}
		SDL_LockMutex (framesMutex);
		while (true) {
			if (isStopped || isWriteFailed) {
				break;
			}
			if (frames.empty ()) {
				SDL_CondWait (framesCond, framesMutex);
				continue;
			}
			i = frames.begin ();
			nextVideoFrame = *i;
			frames.erase (i);
			break;
		}
		SDL_UnlockMutex (framesMutex);
		if (isStopped || isWriteFailed) {
			break;
		}
		if ((videoFrameDropLag > 0) && (writeReferenceTime > 0)) {
			now = OsUtil::getTime ();
			if ((now - writeReferenceTime - nextVideoFrame.pts) > (int64_t) videoFrameDropLag) {
				if (nextVideoFrame.imageData) {
					nextVideoFrame.imageData->release ();
					nextVideoFrame.imageData = NULL;
				}
				continue;
			}
		}
		if (! processVideoFrame ()) {
			break;
		}
	}

	while (true) {
		if (isWriteFailed) {
			break;
		}
		nextVideoFrame.imageData = NULL;
		SDL_LockMutex (framesMutex);
		if (! frames.empty ()) {
			i = frames.begin ();
			nextVideoFrame = *i;
			frames.erase (i);
		}
		SDL_UnlockMutex (framesMutex);
		if (! nextVideoFrame.imageData) {
			break;
		}
		if (! processVideoFrame ()) {
			break;
		}
	}
	if (! isWriteFailed) {
		result = avcodec_send_frame (videoCodecContext, NULL);
		writeVideoPackets ();
	}
	if (! isWriteFailed) {
		result = av_write_trailer (avFormatContext);
		if (result < 0) {
			failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_write_trailer failed");
		}
	}
}

bool MediaWriter::resetSwsContext (int sourceWidth, int sourceHeight) {
	if (swsContext) {
		if ((sourceWidth == swsSourceWidth) && (sourceHeight == swsSourceHeight)) {
			return (true);
		}
	}
	clearSwsContext ();
	swsSourceWidth = sourceWidth;
	swsSourceHeight = sourceHeight;
	swsContext = sws_getContext (swsSourceWidth, swsSourceHeight, MediaUtil::swsRenderPixelFormat, videoWidth, videoHeight, videoPixelFormat, SWS_BILINEAR, NULL, NULL, NULL);
	if (! swsContext) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "sws_getContext error");
		return (false);
	}
	return (true);
}

bool MediaWriter::addVideoStream () {
	AVDictionary *dict;
	int result;
	double n;

	videoCodec = (AVCodec *) avcodec_find_encoder_by_name (videoCodecName.c_str ());
	if (! videoCodec) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_find_encoder_by_name failed");
		return (false);
	}
	videoStream = avformat_new_stream (avFormatContext, NULL);
	if (! videoStream) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avformat_new_stream failed");
		return (false);
	}
	videoCodecContext = avcodec_alloc_context3 (videoCodec);
	if (! videoCodecContext) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_alloc_context3 failed");
		return (false);
	}
	videoCodecContext->width = videoWidth;
	videoCodecContext->height = videoHeight;
	videoCodecContext->pix_fmt = videoPixelFormat;
	videoCodecContext->bit_rate = videoBitrate;
	videoCodecContext->max_b_frames = videoMaxBframes;

	n = (double) videoBitrate;
	videoCodecContext->rc_min_rate = (int64_t) (n * (1.0f - videoRatecontrolRangeMultiplier));
	if (videoCodecContext->rc_min_rate < 1) {
		videoCodecContext->rc_min_rate = 1;
	}
	videoCodecContext->rc_max_rate = (int64_t) (n * (1.0f + videoRatecontrolRangeMultiplier));

	n = videoExpectedFrameRate * videoGopRate;
	videoCodecContext->gop_size = (int) n;
	videoCodecContext->keyint_min = (int) n;

	n = ((double) videoBitrate) * videoRatecontrolBufferMultiplier / 8.0f;
	videoCodecContext->rc_buffer_size = (int) n;

	videoCodecContext->time_base.num = 1;
	videoCodecContext->time_base.den = 1000;
	videoCodecContext->framerate.num = (int) videoExpectedFrameRate;
	videoCodecContext->framerate.den = 1;
	videoCodecContext->sample_aspect_ratio.num = 1;
	videoCodecContext->sample_aspect_ratio.den = 1;
	if (avFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
		videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	dict = NULL;
	av_dict_set (&dict, "preset", MediaWriter::defaultX264Preset, 0);
	result = avcodec_open2 (videoCodecContext, videoCodec, &dict);
	av_dict_free (&dict);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_open2 failed");
		return (false);
	}
	result = avcodec_parameters_from_context (videoStream->codecpar, videoCodecContext);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_parameters_from_context failed");
		return (false);
	}

	avFrame->format = videoCodecContext->pix_fmt;
	avFrame->width = videoCodecContext->width;
	avFrame->height = videoCodecContext->height;
	result = av_frame_get_buffer (avFrame, 0);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_frame_get_buffer failed");
		return (false);
	}
	return (true);
}

bool MediaWriter::processVideoFrame () {
	int result;

	if (! nextVideoFrame.imageData) {
		return (true);
	}
	if (! resetSwsContext (nextVideoFrame.imageWidth, nextVideoFrame.imageHeight)) {
		return (false);
	}
	result = av_frame_make_writable (avFrame);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_frame_make_writable failed");
		return (false);
	}
	videoFrameEncodeSlice[0] = nextVideoFrame.imageData->data;
	videoFrameEncodeStride[0] = nextVideoFrame.imageLineSize;
	result = sws_scale (swsContext, videoFrameEncodeSlice, videoFrameEncodeStride, 0, nextVideoFrame.imageHeight, avFrame->data, avFrame->linesize);
	nextVideoFrame.imageData->release ();
	nextVideoFrame.imageData = NULL;
	if (result != videoHeight) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "sws_scale failed");
		return (false);
	}
	avFrame->pts = nextVideoFrame.pts;
	result = avcodec_send_frame (videoCodecContext, avFrame);
	if (result < 0) {
		failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_send_frame failed");
		return (false);
	}
	if (! writeVideoPackets ()) {
		return (false);
	}
	return (true);
}

bool MediaWriter::writeVideoPackets () {
	int result;

	while (true) {
		result = avcodec_receive_packet (videoCodecContext, avPacket);
		if ((result == AVERROR (EAGAIN)) || (result == AVERROR_EOF)) {
			return (true);
		}
		if (result < 0) {
			failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_receive_packet failed");
			return (false);
		}
		avPacket->stream_index = videoStream->index;
		avPacket->time_base = videoStream->time_base;
		av_packet_rescale_ts (avPacket, videoCodecContext->time_base, videoStream->time_base);
		result = av_interleaved_write_frame (avFormatContext, avPacket);
		av_packet_unref (avPacket);
		++packetWriteCount;
		if (result < 0) {
			failWrite (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_interleaved_write_frame failed");
			return (false);
		}
	}
	return (true);
}

void MediaWriter::writeVideoFrame (SharedBuffer *imageData, int64_t captureTime, int imageLineSize, int imageWidth, int imageHeight) {
	MediaWriter::VideoFrame frame;

	if (isStopped || isWriteFailed) {
		return;
	}
	if (writeReferenceTime <= 0) {
		writeReferenceTime = captureTime;
		frame.pts = 0;
	}
	else {
		frame.pts = captureTime - writeReferenceTime;
		if (frame.pts < 0) {
			frame.pts = 0;
		}
	}
	frame.imageData = imageData;
	frame.imageData->retain ();
	frame.imageLineSize = imageLineSize;
	frame.imageWidth = imageWidth;
	frame.imageHeight = imageHeight;

	SDL_LockMutex (framesMutex);
	frame.id = frameReceiveCount;
	++frameReceiveCount;
	frames.push_back (frame);
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}
