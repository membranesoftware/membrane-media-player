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
#include "libavutil/log.h"
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#include "App.h"
#include "SdlUtil.h"
#include "OsUtil.h"
#include "ClassId.h"
#include "Buffer.h"
#include "TaskGroup.h"
#include "Resource.h"
#include "Sprite.h"
#include "UiText.h"
#include "UiTextId.h"
#include "MediaReader.h"
#include "SubtitleReader.h"
#include "SoundSample.h"
#include "Log.h"
#include "Video.h"

constexpr const int defaultReadaheadTime = 5000; // ms
constexpr const int minDtsDelay = 10;
constexpr const int maxDtsDelay = 2000;

Video::Video (double videoWidth, double videoHeight, int soundMixVolume, bool isSoundMuted)
: Widget ()
, fillBgColor (0.05f, 0.05f, 0.05f)
, drawAlpha (1.0f)
, isResourcePlayPath (false)
, soundSample (NULL)
, isPlaying (false)
, isPlayFailed (false)
, isPlayPresented (false)
, playSeekPercent (0.0f)
, playSeekTimestamp (0)
, readaheadTime (defaultReadaheadTime)
, isStopped (false)
, isPaused (false)
, isReadingPackets (false)
, isFirstVideoFrameRendered (false)
, isDroppingVideoFrames (false)
, isReadEnded (false)
, playTimestamp (0)
, playDuration (0)
, videoStreamDuration (0)
, audioStreamDuration (0)
, renderTargetWidth (0)
, renderTargetHeight (0)
, videoStreamFrameWidth (0)
, videoStreamFrameHeight (0)
, scaledFrameWidth (0)
, scaledFrameHeight (0)
, soundPlayerId (-1)
, soundMixVolume (soundMixVolume)
, isSoundMuted (isSoundMuted)
, isSubtitleLoaded (false)
, avioContext (NULL)
, avFormatContext (NULL)
, avPacket (NULL)
, formatStartTime (0)
, packetReadCount (0)
, videoStream (0)
, videoStreamTimeBaseNum (0)
, videoStreamTimeBaseDen (1)
, videoStreamStartTime (0)
, lastVideoFramePts (-1)
, videoPacketDecodeCount (0)
, videoFrameRenderCount (0)
, videoCodec (NULL)
, videoCodecContext (NULL)
, videoFrame (NULL)
, swsContext (NULL)
, videoFrameReader (NULL)
, isResizing (false)
, resizeWidth (0)
, resizeHeight (0)
, isRenderingVideoFrame (false)
, renderTexture (NULL)
, renderTextureWidth (0)
, renderTextureHeight (0)
, renderPixelFormat (NULL)
, renderPixelBytes (0)
, renderOffsetX (0)
, renderOffsetY (0)
, shouldClearRenderTexture (false)
, audioStream (0)
, audioStreamTimeBaseNum (0)
, audioStreamTimeBaseDen (1)
, audioStreamStartTime (0)
, audioStreamDecodedDuration (0)
, audioPacketDecodeCount (0)
, audioIconSprite (NULL)
, audioDisplayTexture (NULL)
, audioDisplayTextureWidth (0)
, audioDisplayTextureHeight (0)
, audioDisplayTextureDrawWidth (0)
, audioDisplayTextureDrawHeight (0)
, isAudioDisplayEnabled (false)
, playPositionStream (-1)
, playReferenceTime (0)
, pauseTime (0)
, isFirstSeekFrameFound (false)
, firstSeekFrameTimestamp (0)
, playEndCount (0)
, playEndTarget (0)
{
	int i;

	classId = ClassId::Video;
	soundSample = new SoundSample (StdString::createSprintf ("*_Video_%llx", (long long int) App::instance->getUniqueId ()).c_str (), SoundMixer::instance->outputSampleRate, SoundMixer::instance->outputFormat, SoundMixer::instance->outputChannelCount);
	soundSample->retain ();
	setVideoSize (videoWidth, videoHeight);
	renderPixelFormat = SDL_AllocFormat (SDL_PIXELFORMAT_RGBA32);

	for (i = 0; i < Video::imageDataPlaneCount; ++i) {
		imageData[i] = NULL;
		imageLineSizes[i] = 0;
	}

	SdlUtil::createMutex (&framesMutex);
	SdlUtil::createCond (&framesCond);
	SdlUtil::createMutex (&audioDisplayTextureMutex);
	SdlUtil::createMutex (&playEndMutex);
}
Video::~Video () {
	if (soundPlayerId >= 0) {
		SoundMixer::instance->stopPlayer (soundPlayerId);
		soundPlayerId = -1;
	}

	clearPlay ();
	clearFrames ();

	if (videoFrameReader) {
		videoFrameReader->release ();
		videoFrameReader = NULL;
	}

	if (soundSample) {
		soundSample->release ();
		soundSample = NULL;
	}

	if (! renderTexturePath.empty ()) {
		Resource::instance->unloadTexture (renderTexturePath);
		renderTexturePath.assign ("");
	}
	renderTexture = NULL;

	if (renderPixelFormat) {
		SDL_FreeFormat (renderPixelFormat);
		renderPixelFormat = NULL;
	}
	SdlUtil::destroyCond (&framesCond);
	SdlUtil::destroyMutex (&framesMutex);
	SdlUtil::destroyMutex (&audioDisplayTextureMutex);
	SdlUtil::destroyMutex (&playEndMutex);
}

Video *Video::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::Video) ? (Video *) widget : NULL);
}

void Video::clearPlay () {
	if (videoFrameReader) {
		videoFrameReader->release ();
		videoFrameReader = NULL;
	}

	isAudioDisplayEnabled = false;
	SDL_LockMutex (audioDisplayTextureMutex);
	if (audioDisplayTexture && (! audioDisplayTexturePath.empty ())) {
		Resource::instance->unloadTexture (audioDisplayTexturePath);
	}
	audioDisplayTexture = NULL;
	audioDisplayTexturePath.assign ("");
	SDL_UnlockMutex (audioDisplayTextureMutex);

	videoCodec = NULL;
	if (avPacket) {
		av_packet_free (&avPacket);
		avPacket = NULL;
	}
	if (videoFrame) {
		av_frame_free (&videoFrame);
		videoFrame = NULL;
	}
	if (videoCodecContext) {
		avcodec_free_context (&videoCodecContext);
		videoCodecContext = NULL;
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
	clearSwsContext ();
}

void Video::clearSwsContext () {
	int i;

	if (imageData[0]) {
		av_freep (&(imageData[0]));
	}
	for (i = 0; i < Video::imageDataPlaneCount; ++i) {
		imageData[i] = NULL;
		imageLineSizes[i] = 0;
	}
	if (swsContext) {
		sws_freeContext (swsContext);
		swsContext = NULL;
	}
}

void Video::clearFrames () {
	std::list<Video::VideoFrame>::iterator i1, i2;

	SDL_LockMutex (framesMutex);
	i1 = frames.begin ();
	i2 = frames.end ();
	while (i1 != i2) {
		if (i1->imageData) {
			delete (i1->imageData);
			i1->imageData = NULL;
		}
		++i1;
	}
	frames.clear ();
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}

void Video::stop () {
	isStopped = true;
	if (soundPlayerId >= 0) {
		SoundMixer::instance->stopPlayer (soundPlayerId);
		soundPlayerId = -1;
	}
	clearFrames ();
}

void Video::pause () {
	int64_t now;

	if (! isPlaying) {
		return;
	}
	now = OsUtil::getTime ();
	if (soundPlayerId >= 0) {
		SoundMixer::instance->pausePlayer (soundPlayerId);
	}
	isPaused = (! isPaused);
	if (isPaused) {
		pauseTime = now;
	}
	else {
		playReferenceTime += (now - pauseTime);
		SDL_LockMutex (framesMutex);
		SDL_CondBroadcast (framesCond);
		SDL_UnlockMutex (framesMutex);
	}
}

void Video::setVideoSize (double videoWidth, double videoHeight) {
	width = floor (videoWidth);
	height = floor (videoHeight);
	if (width < 1.0f) {
		width = 1.0f;
	}
	if (height < 1.0f) {
		height = 1.0f;
	}
	if (isPlaying) {
		SDL_LockMutex (audioDisplayTextureMutex);
		if (audioDisplayTexture) {
			resetAudioDisplayTextureDrawSize ();
		}
		SDL_UnlockMutex (audioDisplayTextureMutex);
		resizeWidth = (int) width;
		resizeHeight = (int) height;
		isResizing = true;
	}
	else {
		renderTargetWidth = (int) width;
		renderTargetHeight = (int) height;
	}
}

void Video::setSoundMixVolume (int volume) {
	if (volume < 0) {
		volume = 0;
	}
	if (volume > SoundMixer::maxMixVolume) {
		volume = SoundMixer::maxMixVolume;
	}
	if (soundMixVolume != volume) {
		soundMixVolume = volume;
		if (soundPlayerId >= 0) {
			SoundMixer::instance->setPlayerMixVolume (soundPlayerId, soundMixVolume);
		}
	}
}

void Video::setSoundMuted (bool muted) {
	if (isSoundMuted != muted) {
		isSoundMuted = muted;
		if (soundPlayerId >= 0) {
			SoundMixer::instance->mutePlayer (soundPlayerId, isSoundMuted);
		}
	}
}

void Video::translateAlpha (double startAlpha, double targetAlpha, int durationMs) {
	if (startAlpha < 0.0f) {
		startAlpha = 0.0f;
	}
	else if (startAlpha > 1.0f) {
		startAlpha = 1.0f;
	}
	if (targetAlpha < 0.0f) {
		targetAlpha = 0.0f;
	}
	else if (targetAlpha > 1.0f) {
		targetAlpha = 1.0f;
	}

	drawAlpha = startAlpha;
	translateAlphaValue.translateX (startAlpha, targetAlpha, durationMs);
}

void Video::setPlayPath (const StdString &playPathValue, bool isResourcePlayPathValue) {
	playPath.assign (playPathValue);
	isResourcePlayPath = isResourcePlayPathValue;
	playSeekPercent = 0.0f;
	playSeekTimestamp = 0;
}

void Video::setPlaySeekPercent (double seekPercent) {
	if (seekPercent < 0.0f) {
		seekPercent = 0.0f;
	}
	if (seekPercent > 100.0f) {
		seekPercent = 100.0f;
	}
	playSeekPercent = seekPercent;
	playSeekTimestamp = -1;
}

void Video::setPlaySeekTimestamp (int64_t seekTimestamp) {
	if (seekTimestamp < 0) {
		seekTimestamp = 0;
	}
	playSeekTimestamp = seekTimestamp;
	playSeekPercent = 0.0f;
}

void Video::play (Widget::EventCallbackContext endCallback) {
	if (isPlaying) {
		eventCallback (endCallback);
		return;
	}
	if (playPath.empty ()) {
		failPlay (UiText::instance->getText (UiTextId::NoMediaSelected).capitalized (), "Empty play path");
		eventCallback (endCallback);
		return;
	}
	playEndCallback = endCallback;
	isPlaying = true;
	isPlayFailed = false;
	isPlayPresented = false;
	lastErrorMessage.assign ("");
	playEndCount = 0;
	playEndTarget = 0;
	playTimestamp = 0;
	playDuration = 0;
	formatStartTime = 0;
	isStopped = false;
	isPaused = false;
	isResizing = false;
	isFirstVideoFrameRendered = false;
	isFirstSeekFrameFound = false;
	firstSeekFrameTimestamp = 0;
	isDroppingVideoFrames = false;
	isRenderingVideoFrame = false;
	isReadEnded = false;
	isSubtitleLoaded = false;
	playPositionStream = -1;
	lastVideoFramePts = -1;
	renderTargetWidth = (int) width;
	renderTargetHeight = (int) height;
	isReadingPackets = true;
	retain ();
	if (! TaskGroup::instance->run (TaskGroup::RunContext (Video::readPackets, this))) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "TaskGroup run error");
		eventCallback (playEndCallback);
		playEndCallback = Widget::EventCallbackContext ();
		release ();
		return;
	}
	++playEndTarget;
	retain ();
	if (! TaskGroup::instance->run (TaskGroup::RunContext (Video::presentFrames, this))) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "TaskGroup run error");
		release ();
		return;
	}
	++playEndTarget;
}

void Video::failPlay (const StdString &lastErrorMessageValue, const char *logErrorMessage) {
	isPlayFailed = true;
	lastErrorMessage.assign (lastErrorMessageValue);
	if (logErrorMessage) {
		Log::debug ("Video play failed: %s", logErrorMessage);
	}
}

void Video::endPlayTask () {
	bool ended;
	Widget::EventCallbackContext callback;

	ended = false;
	SDL_LockMutex (playEndMutex);
	++playEndCount;
	ended = (playEndCount == playEndTarget);
	SDL_UnlockMutex (playEndMutex);
	if (! ended) {
		SDL_LockMutex (framesMutex);
		SDL_CondBroadcast (framesCond);
		SDL_UnlockMutex (framesMutex);
	}
	else {
		if (soundPlayerId >= 0) {
			SoundMixer::instance->stopPlayer (soundPlayerId);
			soundPlayerId = -1;
		}
		callback = playEndCallback;
		playEndCallback = Widget::EventCallbackContext ();
		isPlaying = false;
		clearFrames ();
		eventCallback (callback);
	}
}

void Video::readPackets (void *itPtr) {
	Video *it = (Video *) itPtr;

	it->executeReadPackets ();
	it->clearPlay ();
	it->isReadingPackets = false;
	it->endPlayTask ();
	it->release ();
}
void Video::executeReadPackets () {
	int result;
	int64_t seekframets;
	uint8_t *buf;

	clearPlay ();
	avPacket = av_packet_alloc ();
	if (! avPacket) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_packet_alloc error");
		return;
	}
	videoFrame = av_frame_alloc ();
	if (! videoFrame) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_frame_alloc error");
		return;
	}
	avFormatContext = avformat_alloc_context ();
	if (! avFormatContext) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avformat_alloc_context failed");
		return;
	}
	if (isResourcePlayPath) {
		rwops.rwops = Resource::instance->openFile (playPath.c_str (), &(rwops.rwopsSize));
		if (! rwops.rwops) {
			failPlay (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), Resource::instance->lastErrorMessage.c_str ());
			return;
		}
		buf = (uint8_t *) av_malloc (MediaUtil::avioBufferSize);
		if (! buf) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_malloc failed");
			return;
		}
		avioContext = avio_alloc_context (buf, MediaUtil::avioBufferSize, 0, &rwops, MediaUtil::avioReadPacket, NULL, MediaUtil::avioSeek);
		if (! avioContext) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avio_alloc_context failed");
			return;
		}
		avFormatContext->pb = avioContext;
		avFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
	}

	result = avformat_open_input (&avFormatContext, playPath.c_str (), NULL, NULL);
	if (result != 0) {
		failPlay (UiText::instance->getText (UiTextId::FileOpenFailed).capitalized (), "avformat_open_input error");
		return;
	}
	result = avformat_find_stream_info (avFormatContext, NULL);
	if (result < 0) {
		failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "avformat_find_stream_info error");
		return;
	}
	if (avFormatContext->start_time > 0) {
		formatStartTime = avFormatContext->start_time * 1000 / AV_TIME_BASE;
	}
	if (avFormatContext->duration > 0) {
		playDuration = avFormatContext->duration * 1000 / AV_TIME_BASE;
	}
	videoStreamDuration = 0;
	audioStreamDuration = 0;
	audioStreamDecodedDuration = 0;

	videoStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (videoStream >= 0) {
		videoStreamTimeBaseNum = avFormatContext->streams[videoStream]->time_base.num;
		videoStreamTimeBaseDen = avFormatContext->streams[videoStream]->time_base.den;
		if ((videoStreamTimeBaseNum <= 0) || (videoStreamTimeBaseDen <= 0)) {
			failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), StdString::createSprintf ("Video stream %i provided invalid time base values", videoStream).c_str ());
			return;
		}
		videoCodec = (AVCodec *) avcodec_find_decoder (avFormatContext->streams[videoStream]->codecpar->codec_id);
		if (! videoCodec) {
			failPlay (UiText::instance->getText (UiTextId::UnknownVideoFormat).capitalized (), "Video codec not supported");
			return;
		}
		if (avFormatContext->streams[videoStream]->duration >= 0) {
			if ((avFormatContext->streams[videoStream]->avg_frame_rate.num > 0) && (avFormatContext->streams[videoStream]->avg_frame_rate.den > 0)) {
				videoStreamDuration = avFormatContext->streams[videoStream]->duration * 1000 * videoStreamTimeBaseNum / videoStreamTimeBaseDen;
			}
		}
		else if (avFormatContext->duration >= 0) {
			videoStreamDuration = (avFormatContext->duration * videoStreamTimeBaseDen / videoStreamTimeBaseNum) / AV_TIME_BASE;
		}
		if (avFormatContext->streams[videoStream]->start_time != AV_NOPTS_VALUE) {
			videoStreamStartTime = (avFormatContext->streams[videoStream]->start_time * 1000 * videoStreamTimeBaseNum) / videoStreamTimeBaseDen;
		}
		else if (formatStartTime > 0) {
			videoStreamStartTime = formatStartTime;
		}
		videoCodecContext = avcodec_alloc_context3 (videoCodec);
		if (! videoCodecContext) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "avcodec_alloc_context3 error");
			return;
		}
		result = avcodec_parameters_to_context (videoCodecContext, avFormatContext->streams[videoStream]->codecpar);
		if (result < 0) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_parameters_to_context error %i", result).c_str ());
			return;
		}
		if (videoCodec->capabilities & AV_CODEC_CAP_FRAME_THREADS) {
			videoCodecContext->thread_type = FF_THREAD_FRAME;
			videoCodecContext->thread_count = 0;
		}
		else if (videoCodec->capabilities & AV_CODEC_CAP_SLICE_THREADS) {
			videoCodecContext->thread_type = FF_THREAD_SLICE;
			videoCodecContext->thread_count = 0;
		}
		else {
			videoCodecContext->thread_count = 1;
		}
		result = avcodec_open2 (videoCodecContext, videoCodec, NULL);
		if (result < 0) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_open2 error %i", result).c_str ());
			return;
		}
		videoStreamFrameWidth = videoCodecContext->width;
		videoStreamFrameHeight = videoCodecContext->height;
		if ((videoStreamFrameWidth <= 0) || (videoStreamFrameHeight <= 0)) {
			failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "Video stream frame size not available");
			return;
		}
		if (videoCodecContext->pix_fmt == AV_PIX_FMT_NONE) {
			failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "Video pixel format not available");
			return;
		}
		if (! resetSwsContext ()) {
			return;
		}
	}

	audioStream = av_find_best_stream (avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (audioStream >= 0) {
		audioStreamTimeBaseNum = avFormatContext->streams[audioStream]->time_base.num;
		audioStreamTimeBaseDen = avFormatContext->streams[audioStream]->time_base.den;
		if ((audioStreamTimeBaseNum <= 0) || (audioStreamTimeBaseDen <= 0)) {
			failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), StdString::createSprintf ("Audio stream %i provided invalid time base values", audioStream).c_str ());
			return;
		}
		if (avFormatContext->streams[audioStream]->duration >= 0) {
			audioStreamDuration = avFormatContext->streams[audioStream]->duration * 1000 * audioStreamTimeBaseNum / audioStreamTimeBaseDen;
		}
		else if (avFormatContext->duration >= 0) {
			audioStreamDuration = (avFormatContext->duration * audioStreamTimeBaseDen / audioStreamTimeBaseNum) / AV_TIME_BASE;
		}
		if (avFormatContext->streams[audioStream]->start_time != AV_NOPTS_VALUE) {
			audioStreamStartTime = (avFormatContext->streams[audioStream]->start_time * 1000 * audioStreamTimeBaseNum) / audioStreamTimeBaseDen;
		}
		else if (formatStartTime > 0) {
			audioStreamStartTime = formatStartTime;
		}
		result = soundSample->loadStream (avFormatContext->streams[audioStream]);
		if (result != OpResult::Success) {
			Log::debug ("Video playback: Failed to open audio codec; path=\"%s\" codec_id=%lli err=\"%s\"", playPath.c_str (), (long long int) avFormatContext->streams[audioStream]->codecpar->codec_id, soundSample->lastErrorMessage.c_str ());
			audioStream = -1;
		}
	}

	readSubtitles ();

	playTimestamp = 0;
	firstSeekFrameTimestamp = 0;
	isFirstSeekFrameFound = false;
	seekframets = 0;
	if ((videoStream >= 0) && (videoStreamDuration > 0)) {
		playPositionStream = videoStream;
		if (playDuration <= 0) {
			playDuration = videoStreamDuration;
		}
		if (playSeekTimestamp >= 0) {
			firstSeekFrameTimestamp = playSeekTimestamp + videoStreamStartTime;
		}
		else if (playSeekPercent > 0.0f) {
			firstSeekFrameTimestamp = (int64_t) (playSeekPercent / 100.0f * (double) videoStreamDuration) + videoStreamStartTime;
		}
		if (firstSeekFrameTimestamp > 0) {
			playTimestamp = firstSeekFrameTimestamp - formatStartTime;
			playReferenceTime = OsUtil::getTime () - firstSeekFrameTimestamp + formatStartTime;
			seekframets = firstSeekFrameTimestamp * videoStreamTimeBaseDen / videoStreamTimeBaseNum / 1000;
		}
	}
	else if ((audioStream >= 0) && (audioStreamDuration > 0)) {
		playPositionStream = audioStream;
		if (playDuration <= 0) {
			playDuration = audioStreamDuration;
		}
		if (playSeekTimestamp >= 0) {
			firstSeekFrameTimestamp = playSeekTimestamp + audioStreamStartTime;
		}
		else if (playSeekPercent > 0.0f) {
			firstSeekFrameTimestamp = (int64_t) (playSeekPercent / 100.0f * (double) audioStreamDuration) + audioStreamStartTime;
		}
		if (firstSeekFrameTimestamp > 0) {
			playTimestamp = firstSeekFrameTimestamp - formatStartTime;
			playReferenceTime = OsUtil::getTime () - firstSeekFrameTimestamp + formatStartTime;
			seekframets = firstSeekFrameTimestamp * audioStreamTimeBaseDen / audioStreamTimeBaseNum / 1000;
		}
	}
	else {
		failPlay (UiText::instance->getText (UiTextId::InvalidMediaFile).capitalized (), "No audio or video streams found");
		return;
	}

	if (audioStream >= 0) {
		if ((videoStream < 0) && audioIconSprite) {
			isAudioDisplayEnabled = true;
		}
		else if ((videoStream >= 0) && (videoStreamDuration <= 0)) {
			if (videoFrameReader) {
				videoFrameReader->release ();
			}
			videoFrameReader = new MediaReader ();
			videoFrameReader->retain ();
			videoFrameReader->setMediaPath (playPath, isResourcePlayPath);
			videoFrameReader->readVideoFrame (videoStreamFrameWidth, videoStreamFrameHeight);
			if (videoFrameReader->lastErrorMessage.empty ()) {
				retain ();
				videoFrameReader->createVideoFrameTexture (Video::audioFrameTextureCreated, this);
			}
		}
	}

	if (firstSeekFrameTimestamp <= 0) {
		playReferenceTime = OsUtil::getTime ();
		isFirstSeekFrameFound = true;
	}
	playReferenceTime -= formatStartTime;
	if (seekframets > 0) {
		result = av_seek_frame (avFormatContext, playPositionStream, seekframets, 0);
		if (result < 0) {
			failPlay (UiText::instance->getText (UiTextId::SeekPositionNotFound).capitalized (), "Failed to seek play position");
			return;
		}
	}

	packetReadCount = 0;
	audioPacketDecodeCount = 0;
	videoPacketDecodeCount = 0;
	videoFrameRenderCount = 0;
	soundPlayerId = SoundMixer::instance->playLiveSample (soundSample, soundMixVolume, isSoundMuted);
	shouldClearRenderTexture = true;
	while (true) {
		if (isStopped || isPlayFailed) {
			break;
		}
		if (isPaused) {
			SDL_LockMutex (framesMutex);
			SDL_CondWait (framesCond, framesMutex);
			SDL_UnlockMutex (framesMutex);
			continue;
		}

		result = av_read_frame (avFormatContext, avPacket);
		if (result < 0) {
			break;
		}
		++packetReadCount;
		if (avPacket->stream_index == videoStream) {
			decodeVideoPacket ();
		}
		else if (avPacket->stream_index == audioStream) {
			decodeAudioPacket ();
		}
		av_packet_unref (avPacket);
	}
	isReadEnded = true;
	endAudioStream ();
}

void Video::decodeVideoPacket () {
	int64_t dts, pts, playts, now, delta;
	int result;
	Video::VideoFrame frame;
	Buffer *buffer;

	++videoPacketDecodeCount;
	pts = -1;
	if (avPacket->pts != AV_NOPTS_VALUE) {
		pts = (avPacket->pts * 1000 * videoStreamTimeBaseNum) / videoStreamTimeBaseDen;
	}
	dts = (avPacket->dts * 1000 * videoStreamTimeBaseNum) / videoStreamTimeBaseDen;
	now = OsUtil::getTime ();
	playts = now - playReferenceTime;
	if (isDroppingVideoFrames) {
		if (!((avPacket->flags & AV_PKT_FLAG_KEY) && (pts >= playts))) {
			return;
		}
		isDroppingVideoFrames = false;
	}
	else {
		if (isFirstVideoFrameRendered && (pts >= 0) && (pts < playts)) {
			isDroppingVideoFrames = true;
			return;
		}
	}

	delta = 0;
	if (isFirstVideoFrameRendered) {
		delta = dts - playts - readaheadTime;
	}
	while (delta >= minDtsDelay) {
		if (isStopped || isPlayFailed) {
			return;
		}
		SDL_Delay (delta > maxDtsDelay ? maxDtsDelay : (int) delta);
		now = OsUtil::getTime ();
		playts = now - playReferenceTime;
		delta = dts - playts - readaheadTime;
	}
	if (isStopped || isPlayFailed) {
		return;
	}

	result = avcodec_send_packet (videoCodecContext, avPacket);
	if (result < 0) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_send_packet error %i", result).c_str ());
		return;
	}
	while (true) {
		result = avcodec_receive_frame (videoCodecContext, videoFrame);
		if ((result == AVERROR (EAGAIN)) || (result == AVERROR_EOF)) {
			break;
		}
		if (result < 0) {
			failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("avcodec_receive_frame error %i", result).c_str ());
			break;
		}
		pts = 0;
		if (videoFrame->pts != AV_NOPTS_VALUE) {
			pts = (videoFrame->pts * 1000 * videoStreamTimeBaseNum) / videoStreamTimeBaseDen;
		}
		else if (videoFrame->pkt_dts != AV_NOPTS_VALUE) {
			pts = (videoFrame->pkt_dts * 1000 * videoStreamTimeBaseNum) / videoStreamTimeBaseDen;
		}
		else if (lastVideoFramePts >= 0) {
			pts = lastVideoFramePts;
		}

		if (! isFirstSeekFrameFound) {
			playReferenceTime -= (pts - firstSeekFrameTimestamp);
			firstSeekFrameTimestamp = pts;
			isFirstSeekFrameFound = true;
		}
		lastVideoFramePts = pts;

		if (videoStreamDuration > 0) {
			if (! resetSwsContext ()) {
				break;
			}
			result = sws_scale (swsContext, videoFrame->data, videoFrame->linesize, 0, videoFrame->height, imageData, imageLineSizes);
			if (result != scaledFrameHeight) {
				failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), StdString::createSprintf ("sws_scale unexpected result %i", result).c_str ());
			}
			else {
				buffer = new Buffer ();
				result = buffer->add (imageData[0], imageLineSizes[0] * scaledFrameHeight);
				if (result != OpResult::Success) {
					delete (buffer);
					failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "Failed to allocate memory for image data");
				}
				else {
					frame.pts = pts;
					frame.renderWidth = renderTargetWidth;
					frame.renderHeight = renderTargetHeight;
					frame.imageData = buffer;
					frame.imageLineSize = imageLineSizes[0];
					frame.imageWidth = scaledFrameWidth;
					frame.imageHeight = scaledFrameHeight;
					frame.imageOffsetX = renderOffsetX;
					frame.imageOffsetY = renderOffsetY;
					SDL_LockMutex (framesMutex);
					frames.push_back (frame);
					SDL_CondBroadcast (framesCond);
					SDL_UnlockMutex (framesMutex);
				}
			}
		}

		av_frame_unref (videoFrame);
	}
}

void Video::decodeAudioPacket () {
	int64_t dts, pts, playts, now, delta;

	++audioPacketDecodeCount;
	dts = -1;
	delta = 0;
	now = OsUtil::getTime ();
	playts = now - playReferenceTime;
	if (avPacket->dts != AV_NOPTS_VALUE) {
		dts = (avPacket->dts * 1000 * audioStreamTimeBaseNum) / audioStreamTimeBaseDen;
	}
	if (dts >= 0) {
		if ((videoStream < 0) || (videoStreamDuration <= 0) || (lastVideoFramePts >= 0)) {
			delta = dts - playts - readaheadTime;
		}
	}
	while (delta >= minDtsDelay) {
		if (isStopped || isPlayFailed) {
			return;
		}
		SDL_Delay (delta > maxDtsDelay ? maxDtsDelay : (int) delta);
		now = OsUtil::getTime ();
		playts = now - playReferenceTime;
		delta = dts - playts - readaheadTime;
	}
	if (isStopped || isPlayFailed) {
		return;
	}

	soundSample->decodeAudioPacket (avPacket);
	if (! isFirstSeekFrameFound) {
		if (dts >= 0) {
			playReferenceTime -= (dts - firstSeekFrameTimestamp);
			firstSeekFrameTimestamp = dts;
			isFirstSeekFrameFound = true;
		}
		else if (avPacket->pts != AV_NOPTS_VALUE) {
			pts = avPacket->pts * 1000 * audioStreamTimeBaseNum / audioStreamTimeBaseDen;
			playReferenceTime -= (pts - firstSeekFrameTimestamp);
			firstSeekFrameTimestamp = pts;
			isFirstSeekFrameFound = true;
		}
	}
	if (avPacket->pts != AV_NOPTS_VALUE) {
		audioStreamDecodedDuration = ((avPacket->pts + avPacket->duration) * 1000 * audioStreamTimeBaseNum) / audioStreamTimeBaseDen;
	}
	if (playPositionStream == audioStream) {
		isPlayPresented = true;
	}
	if (soundSample->lastFramePts < readaheadTime) {
		playts = firstSeekFrameTimestamp - formatStartTime;
	}
	else {
		playts = soundSample->lastFramePts - readaheadTime - audioStreamStartTime;
	}
	if (playTimestamp < playts) {
		playTimestamp = playts;
	}
}

void Video::endAudioStream () {
	int64_t now, playts, delta;

	if (!(isStopped || isPlayFailed)) {
		now = OsUtil::getTime ();
		playts = now - playReferenceTime;
		delta = audioStreamDecodedDuration - playts;
		while (delta > 0) {
			if (isStopped || isPlayFailed) {
				break;
			}
			if (playPositionStream == audioStream) {
				playTimestamp = soundSample->lastFramePts - audioStreamStartTime;
			}
			SDL_Delay (delta > maxDtsDelay ? maxDtsDelay : (int) delta);
			now = OsUtil::getTime ();
			playts = now - playReferenceTime;
			delta = audioStreamDecodedDuration - playts;
		}
	}

	if (playPositionStream == audioStream) {
		playTimestamp = soundSample->lastFramePts - audioStreamStartTime;
	}
	soundSample->endLoad ();
}

bool Video::resetSwsContext () {
	AVPixFmtDescriptor *pixfmtdesc;
	int result, i;

	if (swsContext && imageData[0]) {
		if (! isResizing) {
			return (true);
		}
		isResizing = false;
		if ((resizeWidth == renderTargetWidth) && (resizeHeight == renderTargetHeight)) {
			return (true);
		}
		renderTargetWidth = resizeWidth;
		renderTargetHeight = resizeHeight;
	}
	clearSwsContext ();
	renderOffsetX = 0;
	renderOffsetY = 0;
	scaledFrameWidth = renderTargetWidth;
	scaledFrameHeight = (videoStreamFrameHeight * renderTargetWidth) / videoStreamFrameWidth;
	if (scaledFrameHeight <= renderTargetHeight) {
		renderOffsetY = (renderTargetHeight - scaledFrameHeight) / 2;
		if (renderOffsetY < 0) {
			renderOffsetY = 0;
		}
	}
	else {
		scaledFrameHeight = renderTargetHeight;
		scaledFrameWidth = (renderTargetHeight * videoStreamFrameWidth) / videoStreamFrameHeight;
		renderOffsetX = (renderTargetWidth - scaledFrameWidth) / 2;
		if (renderOffsetX < 0) {
			renderOffsetX = 0;
		}
	}
	renderPixelBytes = 0;
	pixfmtdesc = (AVPixFmtDescriptor *) av_pix_fmt_desc_get (MediaUtil::swsRenderPixelFormat);
	if (pixfmtdesc) {
		for (i = 0; i < pixfmtdesc->nb_components; ++i) {
			renderPixelBytes += pixfmtdesc->comp[i].depth;
		}
	}
	renderPixelBytes /= 8;
	if (renderPixelBytes <= 0) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "Invalid pixel format for video render output");
		return (false);
	}

	swsContext = sws_getContext (videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, scaledFrameWidth, scaledFrameHeight, MediaUtil::swsRenderPixelFormat, SWS_BILINEAR, NULL, NULL, NULL);
	if (! swsContext) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "sws_getContext error");
		return (false);
	}
	result = av_image_alloc (imageData, imageLineSizes, scaledFrameWidth, scaledFrameHeight, MediaUtil::swsRenderPixelFormat, MediaUtil::avImageAllocAlign);
	if ((result < 0) || (! imageData[0]) || (imageLineSizes[0] <= 0)) {
		failPlay (UiText::instance->getText (UiTextId::InternalApplicationError).capitalized (), "av_image_alloc error");
		return (false);
	}
	memset (imageData[0], 0, imageLineSizes[0] * scaledFrameHeight);
	return (true);
}

void Video::presentFrames (void *itPtr) {
	Video *it = (Video *) itPtr;

	it->executePresentFrames ();
	it->endPlayTask ();
	it->release ();
}
void Video::executePresentFrames () {
	std::list<Video::VideoFrame>::iterator i1, i2;
	int64_t playts, delta, lastpts;
	int skipcount;

	SDL_LockMutex (framesMutex);
	while (true) {
		if (isStopped || isPlayFailed) {
			break;
		}
		if (isPaused) {
			SDL_CondWait (framesCond, framesMutex);
			continue;
		}
		if (frames.empty ()) {
			if (! isReadingPackets) {
				break;
			}
			SDL_CondWait (framesCond, framesMutex);
			continue;
		}
		if (isRenderingVideoFrame) {
			SDL_CondWait (framesCond, framesMutex);
			continue;
		}

		skipcount = 0;
		lastpts = -1;
		i1 = frames.begin ();
		i2 = frames.end ();
		playts = OsUtil::getTime () - playReferenceTime;
		while (i1 != i2) {
			if (i1->pts >= playts) {
				break;
			}
			if (lastpts >= 0) {
				++skipcount;
			}
			lastpts = i1->pts;
			++i1;
		}
		while ((skipcount > 0) && (frames.size () > 1)) {
			i1 = frames.begin ();
			if (i1->imageData) {
				delete (i1->imageData);
				i1->imageData = NULL;
			}
			frames.erase (i1);
			--skipcount;
		}

		i1 = frames.begin ();
		delta = i1->pts - playts;
		if (delta > 0) {
			SDL_CondWaitTimeout (framesCond, framesMutex, (delta > 1000) ? 1000 : (Uint32) delta);
			continue;
		}
		isRenderingVideoFrame = true;
		retain ();
		App::instance->addPredrawTask (Video::renderFrame, this);

		++i1;
		if (i1 == frames.end ()) {
			SDL_CondWait (framesCond, framesMutex);
		}
		else {
			delta = i1->pts - playts;
			if (delta < 1) {
				delta = 1;
			}
			SDL_CondWaitTimeout (framesCond, framesMutex, (delta > 1000) ? 1000 : (Uint32) delta);
		}
	}
	SDL_UnlockMutex (framesMutex);
}

void Video::renderFrame (void *itPtr) {
	Video *it = (Video *) itPtr;

	it->executeRenderFrame ();
	it->release ();
}
void Video::executeRenderFrame () {
	std::list<Video::VideoFrame>::iterator i;
	Video::VideoFrame frame;
	Buffer *buffer;
	uint8_t *src, *dst, *dstpixels;
	int srcpitch, dstpitch, cpsize, x, y, y2;
	Uint32 pixel;
	int64_t playts;

	buffer = NULL;
	SDL_LockMutex (framesMutex);
	if (! frames.empty ()) {
		i = frames.begin ();
		frame = *i;
		frames.erase (i);
		buffer = frame.imageData;
	}
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
	if (! buffer) {
		return;
	}

	if (renderTexture) {
		if ((renderTextureWidth != frame.renderWidth) || (renderTextureHeight != frame.renderHeight)) {
			Resource::instance->unloadTexture (renderTexturePath);
			renderTexturePath.assign ("");
			renderTexture = NULL;
		}
	}
	if (! renderTexture) {
		renderTextureWidth = frame.renderWidth;
		renderTextureHeight = frame.renderHeight;
		renderTexturePath.sprintf ("*_Video_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
		renderTexture = Resource::instance->createTexture (renderTexturePath, renderTextureWidth, renderTextureHeight, true);
		if (! renderTexture) {
			renderTexturePath.assign ("");
		}
	}

	srcpitch = frame.imageLineSize;
	if (renderTexture && (srcpitch > 0)) {
		if (SDL_LockTexture (renderTexture, NULL, (void **) &dstpixels, &dstpitch) != 0) {
			Log::err ("Failed to update video texture, SDL_LockTexture: %s", SDL_GetError ());
		}
		else {
			if (shouldClearRenderTexture) {
				shouldClearRenderTexture = false;
				pixel = SDL_MapRGBA (renderPixelFormat, 0, 0, 0, 255);
				y = 0;
				while (y < frame.imageOffsetY) {
					x = 0;
					dst = dstpixels + (y * dstpitch);
					while (x < renderTextureWidth) {
						*((Uint32 *) dst) = pixel;
						dst += renderPixelBytes;
						++x;
					}
					++y;
				}
				y = frame.imageOffsetY + frame.imageHeight;
				while (y < renderTextureHeight) {
					x = 0;
					dst = dstpixels + (y * dstpitch);
					while (x < renderTextureWidth) {
						*((Uint32 *) dst) = pixel;
						dst += renderPixelBytes;
						++x;
					}
					++y;
				}

				y2 = frame.imageOffsetY + frame.imageHeight;
				x = 0;
				while (x < frame.imageOffsetX) {
					y = frame.imageOffsetY;
					dst = dstpixels + (y * dstpitch) + (x * renderPixelBytes);
					while (y < y2) {
						*((Uint32 *) dst) = pixel;
						dst += dstpitch;
						++y;
					}
					++x;
				}
				x = frame.imageOffsetX + frame.imageWidth;
				while (x < renderTextureWidth) {
					y = frame.imageOffsetY;
					dst = dstpixels + (y * dstpitch) + (x * renderPixelBytes);
					while (y < y2) {
						*((Uint32 *) dst) = pixel;
						dst += dstpitch;
						++y;
					}
					++x;
				}
			}

			cpsize = frame.imageWidth * renderPixelBytes;
			if (cpsize > srcpitch) {
				cpsize = srcpitch;
			}
			if (cpsize > dstpitch) {
				cpsize = dstpitch;
			}
			dst = dstpixels + (frame.imageOffsetY * dstpitch) + (frame.imageOffsetX * renderPixelBytes);
			src = buffer->data;
			y = 0;
			while (y < frame.imageHeight) {
				memcpy (dst, src, cpsize);
				src += srcpitch;
				dst += dstpitch;
				++y;
			}
			SDL_UnlockTexture (renderTexture);
		}
	}
	delete (buffer);

	isFirstVideoFrameRendered = true;
	++videoFrameRenderCount;
	isRenderingVideoFrame = false;
	if (playPositionStream == videoStream) {
		isPlayPresented = true;
	}
	playts = frame.pts - formatStartTime;
	if (playTimestamp < playts) {
		playTimestamp = playts;
	}
	SDL_LockMutex (framesMutex);
	SDL_CondBroadcast (framesCond);
	SDL_UnlockMutex (framesMutex);
}

void Video::setAudioIcon (Sprite *iconSprite, const Color &iconDrawColor) {
	audioIconSprite = iconSprite;
	audioIconDrawColor.assign (iconDrawColor);
}

bool Video::audioFrameTextureCreated (void *itPtr, MediaReader *reader, SDL_Texture *texture, const StdString &texturePath) {
	Video *it = (Video *) itPtr;

	if ((reader->videoFrameScaledWidth <= 0) || (reader->videoFrameScaledHeight <= 0)) {
		return (false);
	}
	SDL_LockMutex (it->audioDisplayTextureMutex);
	if (it->audioDisplayTexture && (! it->audioDisplayTexturePath.empty ())) {
		Resource::instance->unloadTexture (it->audioDisplayTexturePath);
	}
	it->audioDisplayTexture = texture;
	it->audioDisplayTexturePath.assign (texturePath);
	it->audioDisplayTextureWidth = reader->videoFrameScaledWidth;
	it->audioDisplayTextureHeight = reader->videoFrameScaledHeight;
	it->resetAudioDisplayTextureDrawSize ();
	SDL_UnlockMutex (it->audioDisplayTextureMutex);
	it->isAudioDisplayEnabled = true;
	it->release ();
	return (true);
}
void Video::resetAudioDisplayTextureDrawSize () {
	double w, h, texturew, textureh;

	if ((width <= 0.0f) || (height <= 0.0f) || (! audioDisplayTexture) || (audioDisplayTextureWidth <= 0) || (audioDisplayTextureHeight <= 0)) {
		return;
	}
	texturew = (double) audioDisplayTextureWidth;
	textureh = (double) audioDisplayTextureHeight;
	if (width >= height) {
		w = width;
		h = (textureh * width) / texturew;
		if (h > height) {
			h = height;
			w = (texturew * height) / textureh;
		}
	}
	else {
		h = height;
		w = (texturew * height) / textureh;
		if (w > width) {
			w = width;
			h = (textureh * width) / texturew;
		}
	}
	if (w < 1.0f) {
		w = 1.0f;
	}
	if (h < 1.0f) {
		h = 1.0f;
	}
	audioDisplayTextureDrawWidth = (int) w;
	audioDisplayTextureDrawHeight = (int) h;
}

void Video::readSubtitles () {
	StdString path;
	OpResult result;

	if (playPath.empty ()) {
		subtitle.setFilePath (StdString ());
		return;
	}
	path.assign (OsUtil::getReplaceExtensionPath (playPath, StdString (SubtitleReader::srtExtension)));
	subtitle.setFilePath (path);
	result = subtitle.readSubtitles ();
	if (result == OpResult::FileOpenFailedError) {
		path.assign (OsUtil::getAppendExtensionPath (playPath, StdString (SubtitleReader::srtExtension)));
		subtitle.setFilePath (path);
		result = subtitle.readSubtitles ();
	}
	if (result != OpResult::Success) {
		isSubtitleLoaded = false;
	}
	else {
		isSubtitleLoaded = true;
	}
}

StdString Video::getSubtitleText () {
	int n;

	if ((! isSubtitleLoaded) || (! isPlaying)) {
		return (StdString ());
	}
	n = subtitle.findEntry (playTimestamp);
	if (n < 0) {
		return (StdString ());
	}
	return (subtitle.entryList.at (n).text);
}

void Video::doUpdate (int msElapsed) {
	if (translateAlphaValue.isTranslating) {
		translateAlphaValue.update (msElapsed);
		drawAlpha = translateAlphaValue.x;
	}
}

void Video::doDraw (double originX, double originY) {
	SDL_Rect rect;
	SDL_Texture *texture;
	int texturew, textureh;
	bool found;

	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;
	if (isPlaying && isFirstVideoFrameRendered && renderTexture) {
		if (drawAlpha < 1.0f) {
			SDL_SetTextureAlphaMod (renderTexture, (Uint8) (drawAlpha * 255.0f));
			SDL_SetTextureBlendMode (renderTexture, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetTextureBlendMode (renderTexture, SDL_BLENDMODE_NONE);
		}
		SDL_RenderCopy (App::instance->render, renderTexture, NULL, &rect);
	}
	else {
		if (fillBgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (App::instance->render, SDL_BLENDMODE_BLEND);
		}
		else {
			SDL_SetRenderDrawBlendMode (App::instance->render, SDL_BLENDMODE_NONE);
		}
		SDL_SetRenderDrawColor (App::instance->render, fillBgColor.rByte, fillBgColor.gByte, fillBgColor.bByte, fillBgColor.aByte);
		SDL_RenderFillRect (App::instance->render, &rect);
		if (fillBgColor.aByte < 255) {
			SDL_SetRenderDrawBlendMode (App::instance->render, SDL_BLENDMODE_NONE);
		}

		if (isAudioDisplayEnabled) {
			found = false;
			SDL_LockMutex (audioDisplayTextureMutex);
			if (audioDisplayTexture) {
				found = true;
				rect.x = (int) (originX + position.x + ((int) (width / 2.0f)) - (audioDisplayTextureDrawWidth / 2));
				rect.y = (int) (originY + position.y + ((int) (height / 2.0f)) - (audioDisplayTextureDrawHeight / 2));
				rect.w = audioDisplayTextureDrawWidth;
				rect.h = audioDisplayTextureDrawHeight;
				SDL_RenderCopy (App::instance->render, audioDisplayTexture, NULL, &rect);
			}
			SDL_UnlockMutex (audioDisplayTextureMutex);

			if ((! found) && audioIconSprite) {
				texture = audioIconSprite->getTexture (0, &texturew, &textureh);
				if (texture) {
					rect.x = (int) (originX + position.x + ((int) (width / 2.0f)) - (texturew / 2));
					rect.y = (int) (originY + position.y + ((int) (height / 2.0f)) - (textureh / 2));
					rect.w = texturew;
					rect.h = textureh;

					if (audioIconDrawColor.aByte > 0) {
						SDL_SetTextureColorMod (texture, audioIconDrawColor.rByte, audioIconDrawColor.gByte, audioIconDrawColor.bByte);
						SDL_SetTextureAlphaMod (texture, audioIconDrawColor.aByte);
						SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_BLEND);
					}
					SDL_RenderCopy (App::instance->render, texture, NULL, &rect);
					if (audioIconDrawColor.aByte > 0) {
						SDL_SetTextureColorMod (texture, 255, 255, 255);
						SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_NONE);
					}
				}
			}
		}
	}
}
