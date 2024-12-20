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
#include "libavformat/avio.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libswscale/swscale.h"
}
#include "SDL2/SDL_image.h"
#include "App.h"
#include "SdlUtil.h"
#include "TaskGroup.h"
#include "Resource.h"
#include "OsUtil.h"
#include "MediaReader.h"

MediaReader::MediaReader ()
: duration (0)
, mediaFileSize (0)
, totalBitrate (0)
, isVideo (false)
, isAudio (false)
, hasAudioAlbumArt (false)
, videoFrameWidth (0)
, videoFrameHeight (0)
, videoFrameRate (0.0f)
, videoBitrate (0)
, audioChannelCount (0)
, audioSampleRate (0)
, audioBitrate (0)
, videoFrameData (NULL)
, videoFramePitch (0)
, videoFrameScaledWidth (0)
, videoFrameScaledHeight (0)
, videoFrameTimestamp (0)
, videoFrameTexture (NULL)
, jpegQuality (MediaReader::defaultJpegQuality)
, avioContext (NULL)
, avFormatContext (NULL)
, videoStream (-1)
, audioStream (-1)
, videoCodec (NULL)
, videoCodecContext (NULL)
, audioCodec (NULL)
, audioCodecContext (NULL)
, avPacket (NULL)
, avFrame (NULL)
, swsContext (NULL)
, frameSeekPercent (0.0f)
, frameSeekTimestamp (-1)
, isResourceMediaPath (false)
, readVideoFrameCallback (NULL)
, readVideoFrameCallbackData (NULL)
, createTextureCallback (NULL)
, createTextureCallbackData (NULL)
, writeVideoFrameJpegCallback (NULL)
, writeVideoFrameJpegCallbackData (NULL)
, refcount (0)
{
	int i;

	SdlUtil::createMutex (&refcountMutex);
	for (i = 0; i < MediaReader::imageDataPlaneCount; ++i) {
		swsImageData[i] = NULL;
		swsImageLineSizes[i] = 0;
	}
}
MediaReader::~MediaReader () {
	clearRead ();
	clearVideoFrame ();
	SdlUtil::destroyMutex (&refcountMutex);
}

void MediaReader::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}
void MediaReader::release () {
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

void MediaReader::clearMetadata () {
	duration = 0;
	mediaFileSize = 0;
	totalBitrate = 0;
	isVideo = false;
	isAudio = false;
	hasAudioAlbumArt = false;
	videoFrameWidth = 0;
	videoFrameHeight = 0;
	videoFrameRate = 0.0f;
	videoBitrate = 0;
	audioChannelCount = 0;
	audioSampleRate = 0;
	audioBitrate = 0;
}

void MediaReader::clearRead () {
	int i;

	videoStream = -1;
	audioStream = -1;
	videoCodec = NULL;
	audioCodec = NULL;
	if (swsImageData[0]) {
		av_freep (&(swsImageData[0]));
	}
	for (i = 0; i < MediaReader::imageDataPlaneCount; ++i) {
		swsImageData[i] = NULL;
		swsImageLineSizes[i] = 0;
	}
	if (swsContext) {
		sws_freeContext (swsContext);
		swsContext = NULL;
	}
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
	if (audioCodecContext) {
		avcodec_free_context (&audioCodecContext);
		audioCodecContext = NULL;
	}
	if (avFormatContext) {
		avformat_close_input (&avFormatContext);
		avFormatContext = NULL;
	}
	if (avioContext) {
		if (avioContext->buffer) {
			av_free (avioContext->buffer);
		}
		avio_context_free (&avioContext);
		avioContext = NULL;
	}
	if (rwops.rwops) {
		SDL_RWclose (rwops.rwops);
		rwops.rwops = NULL;
		rwops.rwopsSize = 0;
	}
}

void MediaReader::clearVideoFrame () {
	if (videoFrameData) {
		av_freep (&videoFrameData);
		videoFrameData = NULL;
	}
	videoFramePitch = 0;
	videoFrameTimestamp = 0;
}

void MediaReader::setMediaPath (const StdString &mediaPathValue, bool isResourceMediaPathValue) {
	if (mediaPath.equals (mediaPathValue) && (isResourceMediaPath == isResourceMediaPathValue)) {
		return;
	}
	mediaPath.assign (mediaPathValue);
	isResourceMediaPath = isResourceMediaPathValue;
	clearMetadata ();
	clearRead ();
	clearVideoFrame ();
	frameSeekTimestamp = -1;
	frameSeekPercent = 0.0f;
}

void MediaReader::endRead (const StdString &errorMessage) {
	clearRead ();
	lastErrorMessage.assign (errorMessage);
}

OpResult MediaReader::readMetadata () {
	int result;
	AVStream *stream;
	int64_t streamduration;
	uint8_t *buf;

	if (mediaPath.empty ()) {
		endRead (StdString ("Empty media path"));
		return (OpResult::InvalidStateError);
	}
	clearMetadata ();
	clearRead ();
	avFormatContext = avformat_alloc_context ();
	if (! avFormatContext) {
		endRead (StdString ("avformat_alloc_context failed"));
		return (OpResult::FfmpegOperationFailedError);
	}
	if (isResourceMediaPath) {
		rwops.rwops = Resource::instance->openFile (mediaPath.c_str (), &(rwops.rwopsSize));
		if (! rwops.rwops) {
			endRead (StdString::createSprintf ("file open failed, %s", Resource::instance->lastErrorMessage.c_str ()));
			return (OpResult::FileOperationFailedError);
		}
		mediaFileSize = rwops.rwopsSize;
		buf = (uint8_t *) av_malloc (MediaUtil::avioBufferSize);
		if (! buf) {
			endRead (StdString ("av_malloc failed"));
			return (OpResult::FfmpegOperationFailedError);
		}
		avioContext = avio_alloc_context (buf, MediaUtil::avioBufferSize, 0, &rwops, MediaUtil::avioReadPacket, NULL, MediaUtil::avioSeek);
		if (! avioContext) {
			endRead (StdString ("avio_alloc_context failed"));
			return (OpResult::FfmpegOperationFailedError);
		}
		avFormatContext->pb = avioContext;
		avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
	}
	else {
		mediaFileSize = OsUtil::getFileSize (mediaPath);
		if (mediaFileSize < 0) {
			mediaFileSize = 0;
		}
	}
	result = avformat_open_input (&avFormatContext, mediaPath.c_str (), NULL, NULL);
	if (result != 0) {
		endRead (StdString ("avformat_open_input failed"));
		return (OpResult::FileOperationFailedError);
	}
	totalBitrate = avFormatContext->bit_rate;
	result = avformat_find_stream_info (avFormatContext, NULL);
	if (result < 0) {
		endRead (StdString ("avformat_find_stream_info failed"));
		return (OpResult::FfmpegOperationFailedError);
	}
	if (avFormatContext->duration > 0) {
		duration = avFormatContext->duration * 1000 / AV_TIME_BASE;
	}
	audioStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audioStream >= 0) {
		stream = avFormatContext->streams[audioStream];
		audioCodec = (AVCodec *) avcodec_find_decoder (stream->codecpar->codec_id);
		if (! audioCodec) {
			endRead (StdString ("Audio codec not supported"));
			return (OpResult::FfmpegOperationFailedError);
		}
		audioCodecContext = avcodec_alloc_context3 (audioCodec);
		if (! audioCodecContext) {
			endRead (StdString ("Audio avcodec_alloc_context3 failed"));
			return (OpResult::FfmpegOperationFailedError);
		}
		result = avcodec_parameters_to_context (audioCodecContext, stream->codecpar);
		if (result < 0) {
			endRead (StdString::createSprintf ("Audio avcodec_parameters_to_context error %i", result));
			return (OpResult::FfmpegOperationFailedError);
		}
		audioChannelCount = audioCodecContext->channels;
		audioSampleRate = audioCodecContext->sample_rate;
		audioBitrate = audioCodecContext->bit_rate;
		if ((stream->time_base.num <= 0) || (stream->time_base.den <= 0)) {
			endRead (StdString ("Invalid audio stream time_base"));
			return (OpResult::MalformedDataError);
		}
		if (stream->duration > 0) {
			streamduration = stream->duration * 1000 * stream->time_base.num / stream->time_base.den;
			if (streamduration > duration) {
				duration = streamduration;
			}
		}
		isAudio = true;
	}
	videoStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (videoStream >= 0) {
		stream = avFormatContext->streams[videoStream];
		videoCodec = (AVCodec *) avcodec_find_decoder (stream->codecpar->codec_id);
		if (! videoCodec) {
			endRead (StdString ("Video codec not supported"));
			return (OpResult::FfmpegOperationFailedError);
		}
		videoCodecContext = avcodec_alloc_context3 (videoCodec);
		if (! videoCodecContext) {
			endRead (StdString ("Video avcodec_alloc_context3 failed"));
			return (OpResult::FfmpegOperationFailedError);
		}
		result = avcodec_parameters_to_context (videoCodecContext, stream->codecpar);
		if (result < 0) {
			endRead (StdString::createSprintf ("Video avcodec_parameters_to_context error %i", result));
			return (OpResult::FfmpegOperationFailedError);
		}
		videoFrameWidth = videoCodecContext->width;
		videoFrameHeight = videoCodecContext->height;
		videoBitrate = videoCodecContext->bit_rate;
		if ((stream->time_base.num <= 0) || (stream->time_base.den <= 0)) {
			endRead (StdString ("Invalid video stream time_base"));
			return (OpResult::MalformedDataError);
		}
		if (videoCodecContext->pix_fmt == AV_PIX_FMT_NONE) {
			endRead (StdString ("Invalid video stream pixel format"));
			return (OpResult::MalformedDataError);
		}
		if ((stream->avg_frame_rate.num <= 0) || (stream->avg_frame_rate.den <= 0)) {
			if (isAudio) {
				hasAudioAlbumArt = true;
			}
			else {
				endRead (StdString ("Invalid video stream avg_frame_rate"));
				return (OpResult::MalformedDataError);
			}
		}
		else {
			if (stream->duration > 0) {
				streamduration = stream->duration * 1000 * stream->time_base.num / stream->time_base.den;
				if (streamduration > duration) {
					duration = streamduration;
				}
			}
			videoFrameRate = (double) stream->avg_frame_rate.num / (double) stream->avg_frame_rate.den;
			isVideo = true;
		}
	}
	if (!(isVideo || isAudio)) {
		endRead (StdString ("No video or audio streams found"));
		return (OpResult::MalformedDataError);
	}
	if (duration <= 0) {
		if (avFormatContext->duration > 0) {
			duration = avFormatContext->duration * 1000 / AV_TIME_BASE;
		}
	}
	if (duration <= 0) {
		endRead (StdString ("Invalid stream duration metadata"));
		return (OpResult::MalformedDataError);
	}
	if (totalBitrate <= 0) {
		if ((audioBitrate > 0) || (videoBitrate > 0)) {
			totalBitrate = audioBitrate + videoBitrate;
		}
		else {
			totalBitrate = mediaFileSize * 8;
			if (duration >= 1000) {
				totalBitrate /= (duration / 1000);
			}
		}
	}
	endRead ();
	return (OpResult::Success);
}

void MediaReader::setVideoFrameSeekPercent (double frameSeekPercentValue) {
	frameSeekTimestamp = -1;
	if (frameSeekPercentValue < 0.0f) {
		frameSeekPercentValue = 0.0f;
	}
	if (frameSeekPercentValue > 100.0f) {
		frameSeekPercentValue = 100.0f;
	}
	frameSeekPercent = frameSeekPercentValue;
}

void MediaReader::setVideoFrameSeekTimestamp (int64_t frameSeekTimestampValue) {
	if (frameSeekTimestampValue < 0) {
		frameSeekTimestampValue = 0;
	}
	frameSeekTimestamp = frameSeekTimestampValue;
}

void MediaReader::readVideoFrame (int frameScaleWidth, int frameScaleHeight, MediaReader::ReadVideoFrameCallback callback, void *callbackData) {
	lastErrorMessage.assign ("");
	if (mediaPath.empty ()) {
		lastErrorMessage.assign ("Empty media path");
	}
	if (! lastErrorMessage.empty ()) {
		if (callback) {
			callback (callbackData, this);
		}
		return;
	}
	if (frameScaleWidth < 0) {
		frameScaleWidth = 0;
	}
	if (frameScaleHeight < 0) {
		frameScaleHeight = 0;
	}
	videoFrameScaledWidth = frameScaleWidth;
	videoFrameScaledHeight = frameScaleHeight;
	videoFrameTimestamp = 0;
	readVideoFrameCallback = callback;
	readVideoFrameCallbackData = callbackData;
	if (readVideoFrameCallback) {
		retain ();
		TaskGroup::instance->run (TaskGroup::RunContext (MediaReader::readFrame, this));
	}
	else {
		executeReadFrame ();
	}
}
void MediaReader::readFrame (void *itPtr) {
	MediaReader *it = (MediaReader *) itPtr;

	it->executeReadFrame ();
	if (it->readVideoFrameCallback) {
		it->readVideoFrameCallback (it->readVideoFrameCallbackData, it);
		it->readVideoFrameCallback = NULL;
		it->readVideoFrameCallbackData = NULL;
	}
	it->release ();
}
void MediaReader::executeReadFrame () {
	int result, i;
	AVStream *stream;
	uint8_t *buf;
	int64_t seekpos, timebasenum, timebaseden, starttime;
	bool keyframefound, framecomplete;

	clearRead ();
	clearVideoFrame ();
	if (mediaPath.empty ()) {
		endRead (StdString ("Empty media path"));
		return;
	}
	clearMetadata ();
	avPacket = av_packet_alloc ();
	if (! avPacket) {
		endRead (StdString ("av_packet_alloc failed"));
		return;
	}
	avFrame = av_frame_alloc ();
	if (! avFrame) {
		endRead (StdString ("av_frame_alloc failed"));
		return;
	}
	avFormatContext = avformat_alloc_context ();
	if (! avFormatContext) {
		endRead (StdString ("avformat_alloc_context failed"));
		return;
	}
	if (isResourceMediaPath) {
		rwops.rwops = Resource::instance->openFile (mediaPath.c_str (), &(rwops.rwopsSize));
		if (! rwops.rwops) {
			endRead (StdString::createSprintf ("file open failed, %s", Resource::instance->lastErrorMessage.c_str ()));
			return;
		}
		buf = (uint8_t *) av_malloc (MediaUtil::avioBufferSize);
		if (! buf) {
			endRead (StdString ("av_malloc failed"));
			return;
		}
		avioContext = avio_alloc_context (buf, MediaUtil::avioBufferSize, 0, &rwops, MediaUtil::avioReadPacket, NULL, MediaUtil::avioSeek);
		if (! avioContext) {
			endRead (StdString ("avio_alloc_context failed"));
			return;
		}
		avFormatContext->pb = avioContext;
		avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
	}
	result = avformat_open_input (&avFormatContext, mediaPath.c_str (), NULL, NULL);
	if (result != 0) {
		endRead (StdString ("avformat_open_input failed"));
		return;
	}
	result = avformat_find_stream_info (avFormatContext, NULL);
	if (result < 0) {
		endRead (StdString ("avformat_find_stream_info failed"));
		return;
	}
	videoStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (videoStream < 0) {
		endRead (StdString ("No video stream found"));
		return;
	}
	stream = avFormatContext->streams[videoStream];
	timebasenum = stream->time_base.num;
	timebaseden = stream->time_base.den;
	if ((timebasenum <= 0) || (timebaseden <= 0)) {
		endRead (StdString ("Invalid time base values in video stream"));
		return;
	}
	starttime = 0;
	if (avFormatContext->start_time > 0) {
		starttime = avFormatContext->start_time * timebaseden / timebasenum / AV_TIME_BASE;
	}
	else if (stream->start_time > 0) {
		starttime = stream->start_time;
	}
	videoCodec = (AVCodec *) avcodec_find_decoder (stream->codecpar->codec_id);
	if (! videoCodec) {
		endRead (StdString ("Video codec not supported"));
		return;
	}
	videoCodecContext = avcodec_alloc_context3 (videoCodec);
	if (! videoCodecContext) {
		endRead (StdString ("Video avcodec_alloc_context3 failed"));
		return;
	}
	result = avcodec_parameters_to_context (videoCodecContext, stream->codecpar);
	if (result < 0) {
		endRead (StdString::createSprintf ("Video avcodec_parameters_to_context error %i", result));
		return;
	}
	videoFrameWidth = videoCodecContext->width;
	videoFrameHeight = videoCodecContext->height;
	if ((videoFrameWidth <= 0) || (videoFrameHeight <= 0) || (stream->time_base.den <= 0) || (videoCodecContext->pix_fmt == AV_PIX_FMT_NONE)) {
		endRead (StdString ("Invalid video data in media file"));
		return;
	}
	if ((videoFrameScaledWidth <= 0) && (videoFrameScaledHeight <= 0)) {
		videoFrameScaledWidth = videoFrameWidth;
		videoFrameScaledHeight = videoFrameHeight;
	}
	else if (videoFrameScaledWidth <= 0) {
		videoFrameScaledWidth = videoFrameWidth * videoFrameScaledHeight / videoFrameHeight;
	}
	else if (videoFrameScaledHeight <= 0) {
		videoFrameScaledHeight = videoFrameHeight * videoFrameScaledWidth / videoFrameWidth;
	}
	result = avcodec_open2 (videoCodecContext, videoCodec, NULL);
	if (result < 0) {
		endRead (StdString::createSprintf ("avcodec_open2 error %i", result));
		return;
	}
	swsContext = sws_getContext (videoFrameWidth, videoFrameHeight, videoCodecContext->pix_fmt, videoFrameScaledWidth, videoFrameScaledHeight, MediaUtil::swsRenderPixelFormat, SWS_BILINEAR, NULL, NULL, NULL);
	if (! swsContext) {
		endRead (StdString ("sws_getContext error"));
		return;
	}
	result = av_image_alloc (swsImageData, swsImageLineSizes, videoFrameScaledWidth, videoFrameScaledHeight, MediaUtil::swsRenderPixelFormat, MediaUtil::avImageAllocAlign);
	if ((result < 0) || (! swsImageData[0]) || (swsImageLineSizes[0] <= 0)) {
		endRead (StdString ("av_image_alloc error"));
		return;
	}
	seekpos = 0;
	if ((stream->avg_frame_rate.num > 0) && (stream->avg_frame_rate.den > 0)) {
		seekpos = starttime;
		if (frameSeekTimestamp >= 0) {
			seekpos += (frameSeekTimestamp * timebaseden / timebasenum / 1000);
		}
		else {
			seekpos += (int64_t) ((double) stream->duration * frameSeekPercent / 100.0f);
		}
	}
	if (seekpos > 0) {
		result = av_seek_frame (avFormatContext, videoStream, seekpos, 0);
		if (result < 0) {
			endRead (StdString ("Failed to seek frame position"));
			return;
		}
	}

	keyframefound = false;
	framecomplete = false;
	while (! framecomplete) {
		result = av_read_frame (avFormatContext, avPacket);
		if (result < 0) {
			endRead (StdString::createSprintf ("av_read_frame error %i", result));
			return;
		}
		if (! keyframefound) {
			if ((avPacket->stream_index == videoStream) && (avPacket->flags & AV_PKT_FLAG_KEY)) {
				keyframefound = true;
				if (avPacket->pts != AV_NOPTS_VALUE) {
					videoFrameTimestamp = (avPacket->pts - starttime) * 1000 * timebasenum / timebaseden;
				}
				else if (avPacket->dts != AV_NOPTS_VALUE) {
					videoFrameTimestamp = (avPacket->dts - starttime) * 1000 * timebasenum / timebaseden;
				}
				else {
					videoFrameTimestamp = 0;
				}
			}
		}
		if (keyframefound && (avPacket->stream_index == videoStream)) {
			result = avcodec_send_packet (videoCodecContext, avPacket);
			if (result < 0) {
				av_packet_unref (avPacket);
				endRead (StdString::createSprintf ("avcodec_send_packet error %i", result));
				return;
			}
			while (true) {
				result = avcodec_receive_frame (videoCodecContext, avFrame);
				if ((result == AVERROR (EAGAIN)) || (result == AVERROR_EOF)) {
					break;
				}
				if (result < 0) {
					av_packet_unref (avPacket);
					endRead (StdString::createSprintf ("avcodec_send_packet error %i", result));
					return;
				}
				result = sws_scale (swsContext, avFrame->data, avFrame->linesize, 0, avFrame->height, swsImageData, swsImageLineSizes);
				if (result != videoFrameScaledHeight) {
					av_packet_unref (avPacket);
					endRead (StdString::createSprintf ("sws_scale unexpected result %i", result));
					return;
				}
				framecomplete = true;
				break;
			}
		}
		av_packet_unref (avPacket);
	}
	if (! framecomplete) {
		endRead (StdString ("No video frame found"));
		return;
	}

	videoFrameData = swsImageData[0];
	videoFramePitch = swsImageLineSizes[0];
	for (i = 0; i < MediaReader::imageDataPlaneCount; ++i) {
		swsImageData[i] = NULL;
	}
	endRead ();
}

void MediaReader::createVideoFrameTexture (MediaReader::CreateTextureCallback callback, void *callbackData) {
	if (! callback) {
		lastErrorMessage.assign ("Missing callback function");
		return;
	}
	if (! videoFrameData) {
		lastErrorMessage.assign ("Frame data not loaded");
		callback (callbackData, this, NULL, StdString ());
		return;
	}
	lastErrorMessage.assign ("");
	createTextureCallback = callback;
	createTextureCallbackData = callbackData;
	retain ();
	App::instance->addPredrawTask (MediaReader::createTexture, this);
}
void MediaReader::createTexture (void *itPtr) {
	MediaReader *it = (MediaReader *) itPtr;

	it->executeCreateTexture ();
	if ((! it->videoFrameTexture) && it->lastErrorMessage.empty ()) {
		it->lastErrorMessage.assign ("Failed to create texture");
	}
	App::instance->addUpdateTask (MediaReader::endCreateTexture, it);
}
void MediaReader::executeCreateTexture () {
	uint8_t *src, *dst, *dstpixels;
	int dstpitch, cpsize, y, y2;

	if ((! videoFrameData) || (videoFramePitch <= 0) || (videoFrameScaledWidth <= 0) || (videoFrameScaledHeight <= 0)) {
		lastErrorMessage.assign ("Missing video frame data");
		return;
	}
	videoFrameTexturePath.sprintf ("*_MediaReader_%llx", (long long int) App::instance->getUniqueId ());
	videoFrameTexture = Resource::instance->createTexture (videoFrameTexturePath, videoFrameScaledWidth, videoFrameScaledHeight, true);
	if (! videoFrameTexture) {
		videoFrameTexturePath.assign ("");
		lastErrorMessage.assign ("Failed to create texture");
		return;
	}
	if (SDL_LockTexture (videoFrameTexture, NULL, (void **) &dstpixels, &dstpitch) != 0) {
		Resource::instance->unloadTexture (videoFrameTexturePath);
		lastErrorMessage.sprintf ("Failed to update video texture, SDL_LockTexture: %s", SDL_GetError ());
		videoFrameTexturePath.assign ("");
		videoFrameTexture = NULL;
		return;
	}
	cpsize = videoFramePitch;
	if (cpsize > dstpitch) {
		cpsize = dstpitch;
	}
	src = videoFrameData;
	dst = dstpixels;
	y = 0;
	y2 = videoFrameScaledHeight;
	while (y < y2) {
		memcpy (dst, src, cpsize);
		src += videoFramePitch;
		dst += dstpitch;
		++y;
	}
	SDL_UnlockTexture (videoFrameTexture);
}
void MediaReader::endCreateTexture (void *itPtr) {
	MediaReader *it = (MediaReader *) itPtr;
	bool textureprocessed;

	textureprocessed = false;
	if (it->createTextureCallback) {
		textureprocessed = it->createTextureCallback (it->createTextureCallbackData, it, it->videoFrameTexture, it->videoFrameTexturePath);
		it->createTextureCallback = NULL;
		it->createTextureCallbackData = NULL;
	}
	if (! textureprocessed) {
		Resource::instance->unloadTexture (it->videoFrameTexturePath);
	}
	else {
		it->clearVideoFrame ();
	}
	it->videoFrameTexture = NULL;
	it->videoFrameTexturePath.assign ("");
	it->release ();
}

void MediaReader::writeVideoFrameJpeg (const StdString &outputPath, int jpegQualityValue, MediaReader::WriteVideoFrameJpegCallback callback, void *callbackData) {
	lastErrorMessage.assign ("");
	if (outputPath.empty ()) {
		lastErrorMessage.assign ("Empty output path");
	}
	if (! videoFrameData) {
		lastErrorMessage.assign ("Frame data not loaded");
	}
	if (! lastErrorMessage.empty ()) {
		if (callback) {
			callback (callbackData, this);
		}
		return;
	}
	if (jpegQualityValue < 0) {
		jpegQualityValue = 0;
	}
	if (jpegQualityValue > 100) {
		jpegQualityValue = 100;
	}
	writeOutputPath.assign (outputPath);
	jpegQuality = jpegQualityValue;
	writeVideoFrameJpegCallback = callback;
	writeVideoFrameJpegCallbackData = callbackData;
	if (writeVideoFrameJpegCallback) {
		retain ();
		TaskGroup::instance->run (TaskGroup::RunContext (MediaReader::writeJpeg, this));
	}
	else {
		executeWriteJpeg ();
	}
}
void MediaReader::writeJpeg (void *itPtr) {
	MediaReader *it = (MediaReader *) itPtr;

	it->executeWriteJpeg ();
	if (it->writeVideoFrameJpegCallback) {
		it->writeVideoFrameJpegCallback (it->writeVideoFrameJpegCallbackData, it);
		it->writeVideoFrameJpegCallback = NULL;
		it->writeVideoFrameJpegCallbackData = NULL;
	}
	it->release ();
}
void MediaReader::executeWriteJpeg () {
	SDL_Surface *surface;
	int result;

	if (writeOutputPath.empty () || (! videoFrameData)) {
		return;
	}
	surface = SDL_CreateRGBSurfaceWithFormatFrom (videoFrameData, videoFrameScaledWidth, videoFrameScaledHeight, 32, videoFramePitch, MediaUtil::swsRenderPixelFormatSdl);
	if (! surface) {
		lastErrorMessage.sprintf ("Failed to create image surface, SDL_CreateRGBSurfaceWithFormatFrom: %s", SDL_GetError ());
		return;
	}
	result = IMG_SaveJPG (surface, writeOutputPath.c_str (), jpegQuality);
	SDL_FreeSurface (surface);
	if (result != 0) {
		lastErrorMessage.sprintf ("Failed to save image file, IMG_SaveJPG: %s", SDL_GetError ());
	}
}
