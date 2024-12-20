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
// Class that holds audio samples from decoded sound files
#ifndef SOUND_SAMPLE_H
#define SOUND_SAMPLE_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
}
#include "MediaUtil.h"

class Buffer;

class SoundSample {
public:
	SoundSample (const char *soundId, int outputSampleRate, SDL_AudioFormat outputSdlAudioFormat, int outputChannelCount);
	~SoundSample ();

	struct AudioFrame {
		int id;
		int64_t pts;
		int64_t duration;
		Buffer *sampleData;

		AudioFrame ():
			id (0),
			pts (0),
			duration (0),
			sampleData (NULL) { }
	};
	typedef void (*FrameCallback) (void *data, SoundSample *sample, const SoundSample::AudioFrame &frame);

	// Read-write data members
	int loadCount;
	bool isLoadStopped;

	// Read-only data members
	StdString soundId;
	bool isLive;
	bool isLoaded;
	bool isLoading;
	bool isLoadFailed;
	StdString lastErrorMessage;
	int inputSampleRate;
	AVSampleFormat inputFormat;
	SDL_AudioFormat outputSdlAudioFormat;
	int outputSampleRate;
	AVSampleFormat outputFormat;
	int outputChannelCount;
	int64_t outputBufferSize;
	int64_t livePlayerId;
	int64_t lastFramePts;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Return a string description of the object
	StdString toString () const;

	// Set the sample to live mode, providing a callback invocation for each decode frame instead of storing frames list items. The callback function is expected to delete the frame sampleData object when it's no longer needed.
	void setLive (int64_t playerId, SoundSample::FrameCallback callback, void *callbackData);

	// Schedule a task to load sample data from the resource entry matching soundId
	void loadResource ();
	static void loadResourceSamples (void *itPtr);

	// Prepare to decode audio from the provided AVStream and return a result value
	OpResult loadStream (AVStream *avStream);

	// Decode sample data from an AVPacket and store the resulting AudioFrame item
	void decodeAudioPacket (AVPacket *audioPacket);

	// End a load operation after all samples have been received and return a result value
	OpResult endLoad ();

	// Store AudioFrame values from the specified position into destFrame and return a result value
	OpResult getFrame (SoundSample::AudioFrame *destFrame, int64_t lastPts);

private:
	// Return the AVSampleFormat value matching the provided SDL_AudioFormat
	AVSampleFormat getSampleFormat (SDL_AudioFormat audioFormat);

	// Remove all items from the frames list
	void clearFrames ();

	// Free structures and resources allocated during load operations
	void clearLoad ();

	// Set failure state for a load operation
	void failLoad (const StdString &lastErrorMessageValue, const char *logErrorMessage = NULL);

	// Initialize state for a load operation and return a result value
	OpResult initLoad ();

	// Prepare to decode audio samples from an AVStream and return a result value
	OpResult openCodec (AVStream *avStream);

	// Generate an AudioFrame from the contents of swrBuffer
	void processFrame (int sampleCount, int64_t pts, int64_t duration);

	// Execute the loadResourceSamples operation
	void executeLoadResourceSamples ();

	typedef void (*ReformatFunction) (uint8_t *sampleData, int sampleDataSize);
	static void reformatSint8 (uint8_t *sampleData, int sampleDataSize);
	static void reformatUint16 (uint8_t *sampleData, int sampleDataSize);

	MediaUtil::SdlRwOps rwops;
	AVIOContext *avioContext;
	AVFormatContext *avFormatContext;
	int audioStream;
	int64_t audioStreamTimeBaseNum;
	int64_t audioStreamTimeBaseDen;
	AVCodec *avCodec;
	AVCodecContext *avCodecContext;
	AVFrame *avFrame;
	AVPacket *avPacket;
	SwrContext *swrContext;
	uint8_t *swrBuffer;
	int swrBufferSize;
	int swrBufferSizeSamples;
	int nextFrameId;
	int64_t lastFrameDuration;
	bool isLoadEnded;
	SoundSample::ReformatFunction reformatFn;
	std::list<SoundSample::AudioFrame> frames;
	SDL_mutex *framesMutex;
	SDL_cond *framesCond;
	SoundSample::FrameCallback frameCallback;
	void *frameCallbackData;
	SDL_mutex *refcountMutex;
	int refcount;
};
#endif
