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
// Widget that renders frames from an ffmpeg media stream
#ifndef VIDEO_H
#define VIDEO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "MediaUtil.h"
#include "SubtitleReader.h"
#include "SoundMixer.h"
#include "Color.h"
#include "Widget.h"

class Buffer;
class MediaReader;
class SoundSample;
class Sprite;

class Video : public Widget {
public:
	Video (double videoWidth, double videoHeight, int soundMixVolume = SoundMixer::maxMixVolume, bool isSoundMuted = false);
	~Video ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static Video *castWidget (Widget *widget);

	// Read-write data members
	Color fillBgColor;
	double drawAlpha;

	// Read-only data members
	StdString playPath;
	bool isResourcePlayPath;
	SoundSample *soundSample;
	bool isPlaying;
	bool isPlayFailed;
	bool isPlayPresented;
	StdString lastErrorMessage;
	double playSeekPercent;
	int64_t playSeekTimestamp;
	int readaheadTime;
	bool isStopped;
	bool isPaused;
	bool isReadingPackets;
	bool isFirstVideoFrameRendered;
	bool isDroppingVideoFrames;
	bool isReadEnded;
	int64_t playTimestamp;
	int64_t playDuration;
	int64_t videoStreamDuration;
	int64_t audioStreamDuration;
	int renderTargetWidth;
	int renderTargetHeight;
	int videoStreamFrameWidth;
	int videoStreamFrameHeight;
	int scaledFrameWidth;
	int scaledFrameHeight;
	int64_t soundPlayerId;
	int soundMixVolume;
	bool isSoundMuted;
	bool isSubtitleLoaded;

	// Set the play path targeted by the video
	void setPlayPath (const StdString &playPathValue, bool isResourcePlayPathValue = false);

	// Set the seek position for play operations
	void setPlaySeekPercent (double seekPercent);
	void setPlaySeekTimestamp (int64_t seekTimestamp);

	// Start a play operation
	void play (Widget::EventCallbackContext endCallback = Widget::EventCallbackContext ());

	// Stop a play operation in progress
	void stop ();

	// Pause or unpause video playback in progress
	void pause ();

	// Set the size of the video's render frame
	void setVideoSize (double videoWidth, double videoHeight);

	// Set the video's sound mix volume
	void setSoundMixVolume (int volume);

	// Set the video's sound mute state
	void setSoundMuted (bool muted);

	// Begin an operation to change the video's draw alpha value over time
	void translateAlpha (double startAlpha, double targetAlpha, int durationMs);

	// Set a sprite that should be shown by the video when playing an audio file without an album art image
	void setAudioIcon (Sprite *iconSprite, const Color &iconDrawColor = Color (0.0f, 0.0f, 0.0f, 0.0f));

	// Return a string containing subtitle text for the current play timestamp, or an empty string if no subtitle was found
	StdString getSubtitleText ();

protected:
	// Superclass override methods
	void doUpdate (int msElapsed);
	void doDraw (double originX, double originY);

private:
	struct VideoFrame {
		int64_t pts;
		int renderWidth;
		int renderHeight;
		Buffer *imageData;
		int imageLineSize;
		int imageWidth;
		int imageHeight;
		int imageOffsetX;
		int imageOffsetY;

		VideoFrame ():
			pts (0),
			renderWidth (0),
			renderHeight (0),
			imageData (NULL),
			imageLineSize (0),
			imageWidth (0),
			imageHeight (0),
			imageOffsetX (0),
			imageOffsetY (0) { }
	};

	// Set failure state for a play operation
	void failPlay (const StdString &lastErrorMessageValue, const char *logErrorMessage = NULL);

	// Execute an end play event for a completed task thread
	void endPlayTask ();

	// Free data allocated by the play operation
	void clearPlay ();

	// Free data allocated by swsContext and related values
	void clearSwsContext ();

	// Remove all items from the frames list
	void clearFrames ();

	// Read packets from the source until it closes
	static void readPackets (void *itPtr);
	void executeReadPackets ();
	static bool audioFrameTextureCreated (void *itPtr, MediaReader *reader, SDL_Texture *texture, const StdString &texturePath);

	// Process avPacket as a video packet
	void decodeVideoPacket ();

	// Process avPacket as an audio packet
	void decodeAudioPacket ();

	// Reset swsContext and related values as needed for the targeted render size and return true if the operation succeeded
	bool resetSwsContext ();

	// Delay until elapsed play time matches the duration of decoded audio samples
	void endAudioStream ();

	// Reset draw size values for audioDisplayTexture. This method must only be invoked while holding a lock on audioDisplayTextureMutex.
	void resetAudioDisplayTextureDrawSize ();

	// Render stored frames as their presentation times arrive
	static void presentFrames (void *itPtr);
	void executePresentFrames ();

	// Update the render texture with image data from the next frame
	static void renderFrame (void *itPtr);
	void executeRenderFrame ();

	// Find subtitle files associated with the current play path and read subtitle entries if found
	void readSubtitles ();

	static constexpr const int imageDataPlaneCount = 4;

	Position translateAlphaValue;
	AVIOContext *avioContext;
	AVFormatContext *avFormatContext;
	AVPacket *avPacket;
	int64_t formatStartTime;
	int packetReadCount;
	int videoStream;
	int64_t videoStreamTimeBaseNum;
	int64_t videoStreamTimeBaseDen;
	int64_t videoStreamStartTime;
	int64_t lastVideoFramePts;
	int videoPacketDecodeCount;
	int videoFrameRenderCount;
	AVCodec *videoCodec;
	AVCodecContext *videoCodecContext;
	AVFrame *videoFrame;
	SwsContext *swsContext;
	uint8_t *imageData[Video::imageDataPlaneCount];
	int imageLineSizes[Video::imageDataPlaneCount];
	MediaReader *videoFrameReader;
	MediaUtil::SdlRwOps rwops;
	bool isResizing;
	int resizeWidth;
	int resizeHeight;
	std::list<Video::VideoFrame> frames;
	SDL_mutex *framesMutex;
	SDL_cond *framesCond;
	bool isRenderingVideoFrame;
	SDL_Texture *renderTexture;
	StdString renderTexturePath;
	int renderTextureWidth;
	int renderTextureHeight;
	SDL_PixelFormat *renderPixelFormat;
	int renderPixelBytes;
	int renderOffsetX;
	int renderOffsetY;
	bool shouldClearRenderTexture;
	int audioStream;
	int64_t audioStreamTimeBaseNum;
	int64_t audioStreamTimeBaseDen;
	int64_t audioStreamStartTime;
	int64_t audioStreamDecodedDuration;
	int audioPacketDecodeCount;
	Sprite *audioIconSprite;
	Color audioIconDrawColor;
	SDL_Texture *audioDisplayTexture;
	StdString audioDisplayTexturePath;
	int audioDisplayTextureWidth;
	int audioDisplayTextureHeight;
	int audioDisplayTextureDrawWidth;
	int audioDisplayTextureDrawHeight;
	SDL_mutex *audioDisplayTextureMutex;
	bool isAudioDisplayEnabled;
	SubtitleReader subtitle;
	int playPositionStream;
	int64_t playReferenceTime;
	int64_t pauseTime;
	bool isFirstSeekFrameFound;
	int64_t firstSeekFrameTimestamp;
	int playEndCount;
	int playEndTarget;
	SDL_mutex *playEndMutex;
	Widget::EventCallbackContext playEndCallback;
};
#endif
