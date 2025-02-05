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
#include "libavutil/error.h"
#include "libavutil/mathematics.h"
#include "libavutil/channel_layout.h"
#include "libswresample/swresample.h"
}
#include "App.h"
#include "SdlUtil.h"
#include "Log.h"
#include "Resource.h"
#include "UiText.h"
#include "UiTextId.h"
#include "TaskGroup.h"
#include "SharedBuffer.h"
#include "SoundSample.h"

SoundSample::SoundSample (const char *soundId, int outputSampleRate, SDL_AudioFormat outputSdlAudioFormat, int outputChannelCount)
: loadCount (0)
, isLoadStopped (false)
, soundId (soundId)
, isLive (false)
, isLoaded (false)
, isLoading (false)
, isLoadFailed (false)
, inputSampleRate (0)
, inputFormat (AV_SAMPLE_FMT_NONE)
, outputSdlAudioFormat (outputSdlAudioFormat)
, outputSampleRate (outputSampleRate)
, outputFormat (AV_SAMPLE_FMT_NONE)
, outputChannelCount (outputChannelCount)
, outputBufferSize (0)
, livePlayerId (0)
, lastFramePts (0)
, avioContext (NULL)
, avFormatContext (NULL)
, audioStream (-1)
, audioStreamTimeBaseNum (0)
, audioStreamTimeBaseDen (1)
, avCodec (NULL)
, avCodecContext (NULL)
, avFrame (NULL)
, avPacket (NULL)
, swrContext (NULL)
, swrBuffer (NULL)
, swrBufferSize (0)
, swrBufferSizeSamples (0)
, nextFrameId (0)
, lastFrameDuration (0)
, isLoadEnded (false)
, reformatFn (NULL)
, frameCallback (NULL)
, frameCallbackData (NULL)
, refcount (0)
{
	SdlUtil::createMutex (&framesMutex);
	SdlUtil::createCond (&framesCond);
	SdlUtil::createMutex (&refcountMutex);
	outputFormat = getSampleFormat (outputSdlAudioFormat);
}
SoundSample::~SoundSample () {
	clearLoad ();
	clearFrames ();
	SdlUtil::destroyCond (&framesCond);
	SdlUtil::destroyMutex (&framesMutex);
	SdlUtil::destroyMutex (&refcountMutex);
}

void SoundSample::clearLoad () {
	lastErrorMessage.assign ("");
	avCodec = NULL;
	if (swrBuffer) {
		av_freep (&swrBuffer);
		swrBuffer = NULL;
	}
	swrBufferSize = 0;
	swrBufferSizeSamples = 0;
	if (swrContext) {
		swr_free (&swrContext);
		swrContext = NULL;
	}
	if (avPacket) {
		av_packet_free (&avPacket);
		avPacket = NULL;
	}
	if (avFrame) {
		av_frame_free (&avFrame);
		avFrame = NULL;
	}
	if (avCodecContext) {
		avcodec_free_context (&avCodecContext);
		avCodecContext = NULL;
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

void SoundSample::clearFrames () {
	std::list<SoundSample::AudioFrame>::iterator i1, i2;

	SDL_LockMutex (framesMutex);
	i1 = frames.begin ();
	i2 = frames.end ();
	while (i1 != i2) {
		if (i1->sampleData) {
			i1->sampleData->release ();
			i1->sampleData = NULL;
		}
		++i1;
	}
	frames.clear ();
	outputBufferSize = 0;
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}

void SoundSample::retain () {
	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}

void SoundSample::release () {
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

StdString SoundSample::toString () const {
	StdString s;

	s.sprintf ("<SoundSample id=%s", soundId.c_str ());
	if (isLive) {
		s.appendSprintf (" isLive=true livePlayerId=%lli", (long long int) livePlayerId);
	}
	s.append (">");
	return (s);
}

AVSampleFormat SoundSample::getSampleFormat (SDL_AudioFormat audioFormat) {
	switch (audioFormat) {
		case AUDIO_S8:
		case AUDIO_U8: {
			return (AV_SAMPLE_FMT_U8);
		}
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB: {
			return (AV_SAMPLE_FMT_S16);
		}
		case AUDIO_S32LSB:
		case AUDIO_S32MSB: {
			return (AV_SAMPLE_FMT_S32);
		}
		case AUDIO_F32LSB:
		case AUDIO_F32MSB: {
			return (AV_SAMPLE_FMT_FLT);
		}
	}
	return (AV_SAMPLE_FMT_NONE);
}

void SoundSample::setLive (int64_t playerId, SoundSample::FrameCallback callback, void *callbackData) {
	isLive = true;
	livePlayerId = playerId;
	frameCallback = callback;
	frameCallbackData = callbackData;
}

void SoundSample::loadResource () {
	if (isLoaded || isLoading) {
		return;
	}
	isLoading = true;
	isLoadFailed = false;
	isLoadStopped = false;
	retain ();
	if (! TaskGroup::instance->run (TaskGroup::RunContext (SoundSample::loadResourceSamples, this))) {
		isLoading = false;
		release ();
	}
}

void SoundSample::failLoad (const StdString &lastErrorMessageValue, const char *logErrorMessage) {
	isLoadFailed = true;
	lastErrorMessage.assign (lastErrorMessageValue);
	if (logErrorMessage) {
		Log::debug ("Failed to load sound sample %s: %s", soundId.c_str (), logErrorMessage);
	}
}

OpResult SoundSample::initLoad () {
	if (outputFormat == AV_SAMPLE_FMT_NONE) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("Target SDL_AudioFormat unsupported: %x", outputSdlAudioFormat).c_str ());
		return (OpResult::InvalidConfigurationError);
	}
	if ((outputSampleRate < 1) || (outputChannelCount < 1)) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("Invalid output format values, outputSampleRate=%i outputChannelCount=%i", outputSampleRate, outputChannelCount).c_str ());
		return (OpResult::InvalidConfigurationError);
	}
	isLoadEnded = false;
	reformatFn = NULL;
	if ((outputSdlAudioFormat == AUDIO_S8) && (outputFormat == AV_SAMPLE_FMT_U8)) {
		reformatFn = SoundSample::reformatSint8;
	}
	else if (((outputSdlAudioFormat == AUDIO_U16LSB) || (outputSdlAudioFormat == AUDIO_U16MSB)) && (outputFormat == AV_SAMPLE_FMT_S16)) {
		reformatFn = SoundSample::reformatUint16;
	}

	avPacket = av_packet_alloc ();
	if (! avPacket) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_packet_alloc error");
		return (OpResult::FfmpegOperationFailedError);
	}
	avFrame = av_frame_alloc ();
	if (! avFrame) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_frame_alloc error");
		return (OpResult::FfmpegOperationFailedError);
	}
	return (OpResult::Success);
}

OpResult SoundSample::openCodec (AVStream *avStream) {
	int result;
	uint64_t inputchannellayout;

	avCodec = (AVCodec *) avcodec_find_decoder (avStream->codecpar->codec_id);
	if (! avCodec) {
		failLoad (UiText::instance->getText (UiTextId::UnknownAudioFormat).capitalized (), "audio codec not supported");
		return (OpResult::FfmpegOperationFailedError);
	}
	avCodecContext = avcodec_alloc_context3 (avCodec);
	if (! avCodecContext) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_alloc_context3 error");
		return (OpResult::FfmpegOperationFailedError);
	}
	result = avcodec_parameters_to_context (avCodecContext, avStream->codecpar);
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_parameters_to_context error %i", result).c_str ());
		return (OpResult::FfmpegOperationFailedError);
	}
	result = avcodec_open2 (avCodecContext, avCodec, NULL);
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_open2 error %i", result).c_str ());
		return (OpResult::FfmpegOperationFailedError);
	}
	nextFrameId = 1;
	inputSampleRate = avCodecContext->sample_rate;
	inputFormat = avCodecContext->sample_fmt;
	audioStreamTimeBaseNum = avStream->time_base.num;
	audioStreamTimeBaseDen = avStream->time_base.den;
	if ((audioStreamTimeBaseNum <= 0) || (audioStreamTimeBaseDen <= 0)) {
		failLoad (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "Invalid time base values");
		return (OpResult::FfmpegOperationFailedError);
	}
	if (avCodecContext->channels <= 0) {
		failLoad (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "Invalid audio channel count");
		return (OpResult::FfmpegOperationFailedError);
	}
	inputchannellayout = avCodecContext->channel_layout;
	if (inputchannellayout == 0) {
		inputchannellayout = av_get_default_channel_layout (avCodecContext->channels);
	}
	swrContext = swr_alloc_set_opts (NULL, av_get_default_channel_layout (outputChannelCount), outputFormat, outputSampleRate, inputchannellayout, inputFormat, inputSampleRate, 0, NULL);
	if (! swrContext) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "swr_alloc_set_opts error");
		return (OpResult::FfmpegOperationFailedError);
	}
	result = swr_init (swrContext);
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("swr_init error %i", result).c_str ());
		return (OpResult::FfmpegOperationFailedError);
	}
	return (OpResult::Success);
}

void SoundSample::loadResourceSamples (void *itPtr) {
	SoundSample *it = (SoundSample *) itPtr;

	it->executeLoadResourceSamples ();
	it->isLoading = false;
	it->release ();
}
void SoundSample::executeLoadResourceSamples () {
	uint8_t *buf;
	int result;

	clearLoad ();
	if (initLoad () != OpResult::Success) {
		return;
	}
	buf = (uint8_t *) av_malloc (MediaUtil::avioBufferSize);
	if (! buf) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_malloc error");
		return;
	}
	avioContext = avio_alloc_context (buf, MediaUtil::avioBufferSize, 0, &rwops, MediaUtil::avioReadPacket, NULL, MediaUtil::avioSeek);
	if (! avioContext) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avio_alloc_context error");
		return;
	}
	avFormatContext = avformat_alloc_context ();
	if (! avFormatContext) {
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avformat_alloc_context error");
		return;
	}
	rwops.rwops = Resource::instance->openFile (soundId.c_str (), &(rwops.rwopsSize));
	if (! rwops.rwops) {
		failLoad (Resource::instance->lastErrorMessage);
		return;
	}
	avFormatContext->pb = avioContext;
	avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
	result = avformat_open_input (&avFormatContext, "", NULL, NULL);
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), StdString::createSprintf ("avformat_open_input error %i", result).c_str ());
		return;
	}
	result = avformat_find_stream_info (avFormatContext, NULL);
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "avformat_find_stream_info error");
		return;
	}
	audioStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audioStream < 0) {
		failLoad (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), StdString::createSprintf ("av_find_best_stream error %i", audioStream).c_str ());
		return;
	}
	if (openCodec (avFormatContext->streams[audioStream]) != OpResult::Success) {
		return;
	}

	while (true) {
		if (isLoadFailed || isLoadStopped) {
			break;
		}
		result = av_read_frame (avFormatContext, avPacket);
		if (result < 0) {
			if (result == AVERROR_EOF) {
				result = 0;
			}
			break;
		}
		if (avPacket->stream_index == audioStream) {
			decodeAudioPacket (avPacket);
		}
		av_packet_unref (avPacket);
	}
	if (result < 0) {
		failLoad (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), StdString::createSprintf ("av_read_frame error %i", result).c_str ());
		return;
	}
	result = endLoad ();
	if ((result == OpResult::Success) && (! isLoadFailed)) {
		isLoaded = true;
	}
}

void SoundSample::decodeAudioPacket (AVPacket *audioPacket) {
	int result, outputsamples, outputsize;
	int64_t pts, duration;

	result = avcodec_send_packet (avCodecContext, audioPacket);
	if (result < 0) {
		if (! isLive) {
			failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_send_packet error %i", result).c_str ());
		}
		else {
			avcodec_flush_buffers (avCodecContext);
		}
		return;
	}
	while (true) {
		if (isLoadFailed || isLoadStopped) {
			break;
		}
		result = avcodec_receive_frame (avCodecContext, avFrame);
		if ((result == AVERROR (EAGAIN)) || (result == AVERROR_EOF)) {
			break;
		}
		if (result < 0) {
			failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_receive_frame error %i", result).c_str ());
			break;
		}

		outputsamples = av_rescale_rnd (swr_get_delay (swrContext, inputSampleRate) + avFrame->nb_samples, outputSampleRate, inputSampleRate, AV_ROUND_UP);
		if (outputsamples > 0) {
			outputsize = av_samples_get_buffer_size (NULL, outputChannelCount, outputsamples, outputFormat, 1);
			if ((! swrBuffer) || (outputsize > swrBufferSize)) {
				swrBufferSize = outputsize;
				swrBufferSizeSamples = outputsamples;
				if (swrBuffer) {
					av_freep (&swrBuffer);
					swrBuffer = NULL;
				}
				av_samples_alloc (&swrBuffer, NULL, outputChannelCount, outputsamples, outputFormat, 0);
			}
			outputsamples = swr_convert (swrContext, &swrBuffer, outputsamples, (const uint8_t **) avFrame->data, avFrame->nb_samples);
			if (outputsamples < 0) {
				failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("swr_convert error %i", outputsamples).c_str ());
				break;
			}

			pts = avFrame->pts;
			if (pts != AV_NOPTS_VALUE) {
				pts *= 1000 * audioStreamTimeBaseNum;
				pts /= audioStreamTimeBaseDen;
			}
			duration = avFrame->pkt_duration;
			if (duration <= 0) {
				duration = (int64_t) outputsamples * 1000 / outputSampleRate;
			}
			else {
				duration *= 1000 * audioStreamTimeBaseNum;
				duration /= audioStreamTimeBaseDen;
			}
			processFrame (outputsamples, pts, duration);
		}
		av_frame_unref (avFrame);
	}
}

void SoundSample::processFrame (int sampleCount, int64_t pts, int64_t duration) {
	SoundSample::AudioFrame frame;
	OpResult result;
	int outputsize;

	outputsize = av_samples_get_buffer_size (NULL, outputChannelCount, sampleCount, outputFormat, 1);
	if (reformatFn) {
		reformatFn (swrBuffer, outputsize);
	}

	lastFramePts = pts;
	lastFrameDuration = duration;
	frame.pts = pts;
	frame.duration = duration;
	frame.sampleData = new SharedBuffer ();
	frame.sampleData->retain ();
	result = frame.sampleData->add (swrBuffer, outputsize);
	if (result != OpResult::Success) {
		frame.sampleData->release ();
		failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "Failed to allocate memory for sample data");
	}
	else {
		frame.id = nextFrameId;
		++nextFrameId;
		if (isLive) {
			if (frameCallback) {
				frameCallback (frameCallbackData, this, frame);
			}
			frame.sampleData->release ();
		}
		else {
			SDL_LockMutex (framesMutex);
			frames.push_back (frame);
			outputBufferSize += frame.sampleData->length;
			SDL_CondBroadcast (framesCond);
			SDL_UnlockMutex (framesMutex);
		}
	}
}

OpResult SoundSample::endLoad () {
	int outputsamples;
	int64_t pts, duration;

	if (isLoadEnded || (! swrContext)) {
		return (OpResult::Success);
	}
	outputsamples = swr_convert (swrContext, &swrBuffer, swrBufferSizeSamples, NULL, 0);
	if (outputsamples <= 0) {
		if (outputsamples < 0) {
			failLoad (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("swr_convert error %i", outputsamples).c_str ());
			return (OpResult::FfmpegOperationFailedError);
		}
		return (OpResult::Success);
	}
	pts = lastFramePts;
	if (pts != AV_NOPTS_VALUE) {
		pts += lastFrameDuration;
	}
	duration = (int64_t) outputsamples * 1000 / outputSampleRate;
	if (duration < 1) {
		duration = 1;
	}
	processFrame (outputsamples, pts, duration);
	isLoadEnded = true;
	return (OpResult::Success);
}

void SoundSample::reformatSint8 (uint8_t *sampleData, int sampleDataSize) {
	int8_t *d, *end;
	uint8_t *src;
	int16_t n;

	src = sampleData;
	d = (int8_t *) sampleData;
	end = (int8_t *) (sampleData + sampleDataSize);
	while (d < end) {
		n = (*src - 0x80);
		if (n < -0x80) {
			*d = -0x80;
		}
		else if (n > 0x7F) {
			*d = 0x7F;
		}
		else {
			*d = (int8_t) n;
		}
		++src;
		++d;
	}
}

void SoundSample::reformatUint16 (uint8_t *sampleData, int sampleDataSize) {
	uint16_t *d, *end;
	int16_t *src;
	int32_t n;

	src = (int16_t *) sampleData;
	d = (uint16_t *) sampleData;
	end = (uint16_t *) (sampleData + sampleDataSize);
	while (d < end) {
		n = (*src + 0x8000);
		if (n < 0) {
			*d = 0;
		}
		else if (n > 0xFFFF) {
			*d = 0xFFFF;
		}
		else {
			*d = (uint16_t) n;
		}
		++src;
		++d;
	}
}

OpResult SoundSample::getFrame (SoundSample::AudioFrame *destFrame, int64_t lastPts) {
	std::list<SoundSample::AudioFrame>::const_iterator i1, i2;
	bool found;
	OpResult result;

	result = OpResult::KeyNotFoundError;
	found = false;
	SDL_LockMutex (framesMutex);
	i1 = frames.cbegin ();
	i2 = frames.cend ();
	while (i1 != i2) {
		if ((lastPts == AV_NOPTS_VALUE) || found) {
			if (destFrame) {
				*destFrame = *i1;
			}
			result = OpResult::Success;
			break;
		}
		if (i1->pts >= lastPts) {
			found = true;
		}
		++i1;
	}
	SDL_UnlockMutex (framesMutex);
	return (result);
}

OpResult SoundSample::loadStream (AVStream *avStream) {
	OpResult result;

	clearLoad ();
	result = initLoad ();
	if (result != OpResult::Success) {
		return (result);
	}
	result = openCodec (avStream);
	if (result != OpResult::Success) {
		return (result);
	}
	lastFramePts = 0;
	lastFrameDuration = 0;
	return (OpResult::Success);
}
