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
// Class that processes audio samples and writes to an output device
#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

extern "C" {
#include "libavutil/avutil.h"
}
#include "SoundSample.h"

class SharedBuffer;

class SoundMixer {
public:
	SoundMixer ();
	~SoundMixer ();
	static SoundMixer *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static constexpr const int maxMixVolume = 10000;

	typedef void (*OutputCallback) (void *itPtr, SharedBuffer *sampleData, int playerWriteDelta);
	struct OutputCallbackContext {
		SoundMixer::OutputCallback callback;
		void *callbackData;
		OutputCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		OutputCallbackContext (SoundMixer::OutputCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Read-write data members
	int deviceWritePeriod; // milliseconds
	int readaheadTime; // milliseconds
	int masterMixVolume;

	// Read-only data members
	bool isActive;
	bool isStopped;
	StdString lastErrorMessage;
	SDL_AudioDeviceID audioDeviceId;
	int outputSampleRate;
	SDL_AudioFormat outputFormat;
	int outputChannelCount;
	int outputSampleSize;
	int readaheadSize;

	// Initialize audio functionality and acquire resources as needed. Returns a Result value.
	OpResult start ();

	// Stop the sound mixer and release acquired resources
	void stop ();

	// Return a boolean value indicating if the sound mixer has completed its stop operation and its child threads have terminated
	bool isStopComplete ();

	// Join the sound mixer's child threads, blocking until the operation completes
	void waitThreads ();

	// Load sample data for the specified soundId
	void loadSample (const char *soundId);

	// Unload sample data for the specified soundId
	void unloadSample (const char *soundId);

	// Execute a play operation for the specified sample and return the created player ID, or a negative value if the player could not be created
	int64_t playResourceSample (const char *soundId, int mixVolume = SoundMixer::maxMixVolume, bool muted = false, SoundMixer::OutputCallbackContext outputCallback = SoundMixer::OutputCallbackContext ());

	// Execute a play operation for the specified sample and return the created player ID, or a negative value if the player could not be created
	int64_t playLiveSample (SoundSample *sample, int mixVolume = SoundMixer::maxMixVolume, bool muted = false, SoundMixer::OutputCallbackContext outputCallback = SoundMixer::OutputCallbackContext ());

	// Return true if playerId matches an active player
	bool isPlayerActive (int64_t playerId);

	// Stop playback for the specified player ID
	void stopPlayer (int64_t playerId);

	// Pause or unpause playback for the specified player ID
	void pausePlayer (int64_t playerId);

	// Toggle or set muted state for the specified player ID
	void mutePlayer (int64_t playerId);
	void mutePlayer (int64_t playerId, bool muted);

	// Set the mix volume level for the specified player ID
	void setPlayerMixVolume (int64_t playerId, int mixVolume);

	// Set the audio callback for the specified player ID
	void setOutputCallback (int64_t playerId, SoundMixer::OutputCallbackContext callback);

private:
	struct Player {
		int64_t id;
		SoundSample *sample;
		bool isLive;
		int64_t startTime;
		int64_t nextReadTime;
		int64_t lastPts;
		bool isEnded;
		SoundMixer::OutputCallbackContext outputCallback;
		std::queue<SharedBuffer *> outputSampleQueue;
		int64_t outputQueueWritePosition;
		int64_t outputReadPosition;
		bool isOutputReadaheadComplete;

		Player ():
			id (-1),
			sample (NULL),
			isLive (false),
			startTime (0),
			nextReadTime (0),
			lastPts (AV_NOPTS_VALUE),
			isEnded (false),
			outputQueueWritePosition (0),
			outputReadPosition (0),
			isOutputReadaheadComplete (false) { }
		Player (int64_t id, SoundSample *sample, bool isLive, SoundMixer::OutputCallbackContext outputCallback):
			id (id),
			sample (sample),
			isLive (isLive),
			startTime (0),
			nextReadTime (0),
			lastPts (AV_NOPTS_VALUE),
			isEnded (false),
			outputCallback (outputCallback),
			outputQueueWritePosition (0),
			outputReadPosition (0),
			isOutputReadaheadComplete (false) { }
	};

	struct PlayerQueue {
		std::queue<SharedBuffer *> sampleQueue;
		int64_t sampleQueueSize;
		int64_t writePosition;
		int bufferPosition;
		bool isLive;
		bool isPaused;
		int mixVolume;
		bool isMuted;
		bool isEnded;

		PlayerQueue (bool isLive, int mixVolume, bool isMuted):
			sampleQueueSize (0),
			writePosition (0),
			bufferPosition (0),
			isLive (isLive),
			isPaused (false),
			mixVolume (mixVolume),
			isMuted (isMuted),
			isEnded (false) { }
	};

	// Run a thread that reads sample data from active players
	static int runPlayers (void *itPtr);
	void executeRunPlayers ();

	// SDL_AudioCallback
	static void audioCallback (void *userdata, Uint8 *stream, int len);

	// SoundSample::FrameCallback
	static void liveSampleFrameCallback (void *itPtr, SoundSample *sample, const SoundSample::AudioFrame &frame);

	// Mix data from a player sample buffer into the device write buffer
	typedef void (*MixFunction) (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixSint8 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixUint8 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixSint16 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixUint16 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixSint32 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);
	static void mixFloat32 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize);

	// Clear the sample map
	void clearSampleMap ();

	// Clear the player queue map
	void clearPlayerQueueMap ();

	// Clear the contents of a PlayerQueue object
	void clearPlayerQueue (SoundMixer::PlayerQueue *playerQueue);

	// Clear the player map
	void clearPlayerMap ();

	// Clear the contents of a Player object
	void clearPlayer (SoundMixer::Player *player);

	// Clear the contents of a Player object's output sample queue
	void clearPlayerOutputSamples (SoundMixer::Player *player);

	// Advance player state during the player update loop
	void updatePlayer (SoundMixer::Player *player);
	void executeOutputCallback (SoundMixer::Player *player);

	SDL_Thread *playerThread;
	bool isPlayerThreadRunning;
	SDL_mutex *playerMutex;
	SDL_cond *playerCond;
	SoundMixer::MixFunction mixFn;
	std::map<StdString, SoundSample *> sampleMap;
	std::map<int64_t, SoundMixer::Player> playerMap;
	std::map<int64_t, SoundMixer::PlayerQueue> queueMap;
};
#endif
