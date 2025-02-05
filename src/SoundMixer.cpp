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
#include "SdlUtil.h"
#include "OsUtil.h"
#include "Log.h"
#include "SharedBuffer.h"
#include "HashMap.h"
#include "SoundMixer.h"

SoundMixer *SoundMixer::instance = NULL;
constexpr const int defaultOutputSampleRate = 44100;
constexpr const SDL_AudioFormat defaultOutputFormat = AUDIO_F32;
constexpr const int outputCallbackPeriod = 25;

SoundMixer::SoundMixer ()
: deviceWritePeriod (10)
, readaheadTime (2000)
, masterMixVolume (SoundMixer::maxMixVolume)
, isActive (false)
, isStopped (false)
, audioDeviceId (-1)
, outputSampleRate (0)
, outputFormat (defaultOutputFormat)
, outputChannelCount (1)
, outputSampleSize (1)
, readaheadSize (0)
, playerThread (NULL)
, isPlayerThreadRunning (false)
, mixFn (SoundMixer::mixFloat32)
{
	SdlUtil::createMutex (&playerMutex);
	SdlUtil::createCond (&playerCond);
}
SoundMixer::~SoundMixer () {
	clearPlayerMap ();
	clearPlayerQueueMap ();
	clearSampleMap ();
	SdlUtil::destroyCond (&playerCond);
	SdlUtil::destroyMutex (&playerMutex);
}

void SoundMixer::createInstance () {
	if (! SoundMixer::instance) {
		SoundMixer::instance = new SoundMixer ();
	}
}
void SoundMixer::freeInstance () {
	if (SoundMixer::instance) {
		delete (SoundMixer::instance);
		SoundMixer::instance = NULL;
	}
}

void SoundMixer::clearSampleMap () {
	std::map<StdString, SoundSample *>::const_iterator i1, i2;

	i1 = sampleMap.cbegin ();
	i2 = sampleMap.cend ();
	while (i1 != i2) {
		if (i1->second) {
			i1->second->release ();
		}
		++i1;
	}
	sampleMap.clear ();
}

void SoundMixer::clearPlayerQueueMap () {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator i1, i2;

	i1 = queueMap.begin ();
	i2 = queueMap.end ();
	while (i1 != i2) {
		clearPlayerQueue (&(i1->second));
		++i1;
	}
	queueMap.clear ();
}
void SoundMixer::clearPlayerQueue (SoundMixer::PlayerQueue *playerQueue) {
	std::queue<SharedBuffer *> *q;
	SharedBuffer *buffer;

	q = &(playerQueue->sampleQueue);
	while (! q->empty ()) {
		buffer = q->front ();
		buffer->release ();
		q->pop ();
	}
	playerQueue->sampleQueueSize = 0;
	playerQueue->writePosition = 0;
	playerQueue->bufferPosition = 0;
}

void SoundMixer::clearPlayerMap () {
	std::map<int64_t, SoundMixer::Player>::iterator i1, i2;

	i1 = playerMap.begin ();
	i2 = playerMap.end ();
	while (i1 != i2) {
		clearPlayer (&(i1->second));
		++i1;
	}
	playerMap.clear ();
}
void SoundMixer::clearPlayer (SoundMixer::Player *player) {
	clearPlayerOutputSamples (player);
	if (player->sample) {
		player->sample->release ();
		player->sample = NULL;
	}
}
void SoundMixer::clearPlayerOutputSamples (SoundMixer::Player *player) {
	std::queue<SharedBuffer *> *q;
	SharedBuffer *buffer;

	q = &(player->outputSampleQueue);
	while (! q->empty ()) {
		buffer = q->front ();
		buffer->release ();
		q->pop ();
	}
}

void SoundMixer::waitThreads () {
	int result;

	SDL_LockMutex (playerMutex);
	SDL_CondBroadcast (playerCond);
	SDL_UnlockMutex (playerMutex);
	if (playerThread) {
		SDL_WaitThread (playerThread, &result);
		playerThread = NULL;
	}
}

OpResult SoundMixer::start () {
	HashMap *prefs;
	SDL_AudioSpec desired, obtained;

	prefs = App::instance->lockPrefs ();
	masterMixVolume = prefs->find (App::soundVolumeKey, SoundMixer::maxMixVolume);
	App::instance->unlockPrefs ();

	outputSampleRate = defaultOutputSampleRate;
	outputFormat = defaultOutputFormat;
	outputChannelCount = 1;
	SDL_zero (desired);
	desired.freq = defaultOutputSampleRate;
	desired.format = defaultOutputFormat;
	desired.channels = 2;
	desired.samples = deviceWritePeriod * desired.freq / 1000;
	desired.callback = SoundMixer::audioCallback;
	desired.userdata = this;
	audioDeviceId = SDL_OpenAudioDevice (NULL, 0, &desired, &obtained, 0);
	if (audioDeviceId <= 0) {
		lastErrorMessage.assign (SDL_GetError ());
		return (OpResult::SdlOperationFailedError);
	}

	outputSampleRate = obtained.freq;
	outputFormat = obtained.format;
	outputChannelCount = obtained.channels;
	outputSampleSize = SDL_AUDIO_BITSIZE (outputFormat);
	outputSampleSize = (int) ceil (((double) outputSampleSize) / 8.0f);
	if (outputSampleSize < 1) {
		outputSampleSize = 1;
	}
	readaheadSize = (int) ((int64_t) readaheadTime * (int64_t) outputSampleRate * (int64_t) outputSampleSize * (int64_t) outputChannelCount / 1000);
	switch (outputFormat) {
		case AUDIO_S8: {
			mixFn = SoundMixer::mixSint8;
			break;
		}
		case AUDIO_U8: {
			mixFn = SoundMixer::mixUint8;
			break;
		}
		case AUDIO_S16LSB:
		case AUDIO_S16MSB: {
			mixFn = SoundMixer::mixSint16;
			break;
		}
		case AUDIO_U16LSB:
		case AUDIO_U16MSB: {
			mixFn = SoundMixer::mixUint16;
			break;
		}
		case AUDIO_S32LSB:
		case AUDIO_S32MSB: {
			mixFn = SoundMixer::mixSint32;
			break;
		}
		case AUDIO_F32LSB:
		case AUDIO_F32MSB: {
			mixFn = SoundMixer::mixFloat32;
			break;
		}
	}

	isActive = true;
	playerThread = SDL_CreateThread (SoundMixer::runPlayers, "SoundMixer::runPlayers", this);
	if (! playerThread) {
		lastErrorMessage.assign (SDL_GetError ());
		isActive = false;
		SDL_CloseAudioDevice (audioDeviceId);
		audioDeviceId = -1;
		return (OpResult::SdlOperationFailedError);
	}
	isPlayerThreadRunning = true;
	Log::debug ("SoundMixer start; masterMixVolume=%i audioDeviceId=%i desired={format=0x%x freq=%i channels=%i silence=%i samples=%i padding=%i size=%i} obtained={format=0x%x freq=%i channels=%i silence=%i samples=%i padding=%i size=%i} outputSampleRate=%i outputFormat=0x%x outputChannelCount=%i outputSampleSize=%i readaheadSize=%i", masterMixVolume, audioDeviceId, desired.format, desired.freq, desired.channels, desired.silence, desired.samples, desired.padding, desired.size, obtained.format, obtained.freq, obtained.channels, obtained.silence, obtained.samples, obtained.padding, obtained.size, outputSampleRate, outputFormat, outputChannelCount, outputSampleSize, readaheadSize);
	return (OpResult::Success);
}

void SoundMixer::stop () {
	std::map<StdString, SoundSample *>::const_iterator i1, i2;

	isActive = false;
	isStopped = true;

	i1 = sampleMap.cbegin ();
	i2 = sampleMap.cend ();
	while (i1 != i2) {
		if (i1->second && i1->second->isLoading) {
			i1->second->isLoadStopped = true;
		}
		++i1;
	}

	if (audioDeviceId > 0) {
		SDL_CloseAudioDevice (audioDeviceId);
		audioDeviceId = -1;
	}
	SDL_LockMutex (playerMutex);
	SDL_CondBroadcast (playerCond);
	SDL_UnlockMutex (playerMutex);
	clearPlayerQueueMap ();
}

bool SoundMixer::isStopComplete () {
	return (isStopped && (! isPlayerThreadRunning));
}

int SoundMixer::runPlayers (void *itPtr) {
	SoundMixer *it = (SoundMixer *) itPtr;

	it->executeRunPlayers ();
	it->isPlayerThreadRunning = false;
	return (0);
}
void SoundMixer::executeRunPlayers () {
	std::map<int64_t, SoundMixer::Player>::iterator i1, i2;
	std::list<int64_t> endedids;
	std::list<int64_t>::const_iterator j1, j2;
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	int64_t tnext, t;

	SDL_PauseAudioDevice (audioDeviceId, 0);
	SDL_LockMutex (playerMutex);
	while (true) {
		if (! isActive) {
			break;
		}
		if (playerMap.size () <= 0) {
			SDL_CondWait (playerCond, playerMutex);
			continue;
		}
		tnext = 0;
		endedids.clear ();
		i1 = playerMap.begin ();
		i2 = playerMap.end ();
		while (i1 != i2) {
			updatePlayer (&(i1->second));
			if (i1->second.isEnded) {
				endedids.push_back (i1->first);
			}
			else {
				t = i1->second.nextReadTime;
				if (t > 0) {
					if ((tnext <= 0) || (t < tnext)) {
						tnext = t;
					}
				}
			}
			++i1;
		}

		if (! endedids.empty ()) {
			j1 = endedids.cbegin ();
			j2 = endedids.cend ();
			while (j1 != j2) {
				i1 = playerMap.find (*j1);
				if (i1 != playerMap.end ()) {
					clearPlayer (&(i1->second));
					playerMap.erase (i1);
				}
				++j1;
			}

			j1 = endedids.cbegin ();
			j2 = endedids.cend ();
			SDL_UnlockMutex (playerMutex);
			SDL_LockAudioDevice (audioDeviceId);
			while (j1 != j2) {
				qpos = queueMap.find (*j1);
				if (qpos != queueMap.end ()) {
					qpos->second.isEnded = true;
				}
				++j1;
			}
			SDL_UnlockAudioDevice (audioDeviceId);
			SDL_LockMutex (playerMutex);
		}

		if (tnext <= 0) {
			SDL_CondWait (playerCond, playerMutex);
		}
		else {
			t = tnext - OsUtil::getTime ();
			if (t <= 0) {
				SDL_UnlockMutex (playerMutex);
				SDL_LockMutex (playerMutex);
			}
			else {
				SDL_CondWaitTimeout (playerCond, playerMutex, (int) t);
			}
		}
	}
	SDL_UnlockMutex (playerMutex);
}
void SoundMixer::updatePlayer (SoundMixer::Player *player) {
	OpResult result;
	SoundSample::AudioFrame frame;
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	int64_t t, now;

	now = OsUtil::getTime ();
	if ((! player->sample) || player->sample->isLoadFailed) {
		player->isEnded = true;
		return;
	}
	if (player->startTime <= 0) {
		player->startTime = now;
	}
	executeOutputCallback (player);
	if (player->isLive) {
		if (player->outputCallback.callback) {
			player->nextReadTime = now + outputCallbackPeriod;
		}
		else {
			player->nextReadTime = 0;
		}
		return;
	}
	if (player->nextReadTime > now) {
		return;
	}

	while (true) {
		if (! isActive) {
			break;
		}
		result = player->sample->getFrame (&frame, player->lastPts);
		if (result != OpResult::Success) {
			player->isEnded = true;
			break;
		}
		t = player->startTime + frame.pts - readaheadTime;
		if (t > now) {
			player->nextReadTime = t;
			break;
		}

		player->lastPts = frame.pts;
		player->nextReadTime = now + frame.duration;
		SDL_LockAudioDevice (audioDeviceId);
		qpos = queueMap.find (player->id);
		if (qpos != queueMap.end ()) {
			frame.sampleData->retain ();
			qpos->second.sampleQueue.push (frame.sampleData);
			qpos->second.sampleQueueSize += frame.sampleData->length;
		}
		SDL_UnlockAudioDevice (audioDeviceId);

		if (player->outputCallback.callback) {
			frame.sampleData->retain ();
			player->outputSampleQueue.push (frame.sampleData);
		}
	}
}
void SoundMixer::executeOutputCallback (SoundMixer::Player *player) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	SharedBuffer *buffer;
	int64_t writepos, delta;
	int writelen;
	bool readahead;

	if (! player->outputCallback.callback) {
		return;
	}
	writepos = -1;
	SDL_LockAudioDevice (audioDeviceId);
	qpos = queueMap.find (player->id);
	if (qpos != queueMap.end ()) {
		writepos = qpos->second.writePosition;
	}
	SDL_UnlockAudioDevice (audioDeviceId);
	delta = writepos - player->outputQueueWritePosition;
	if (delta <= 0) {
		return;
	}
	player->outputQueueWritePosition = writepos;
	player->outputReadPosition += delta;
	readahead = (! player->isOutputReadaheadComplete);
	if (readahead) {
		delta = 0;
	}
	if (player->outputReadPosition >= 0) {
		if (readahead) {
			writelen = (int) ((int64_t) readaheadTime * outputSampleRate * outputSampleSize * outputChannelCount / 1000);
		}
		else {
			writelen = (int) player->outputReadPosition;
		}
		while (writelen > 0) {
			if (player->outputSampleQueue.empty ()) {
				break;
			}
			buffer = player->outputSampleQueue.front ();
			player->outputCallback.callback (player->outputCallback.callbackData, buffer, (int) delta);
			writelen -= buffer->length;
			if (! readahead) {
				player->outputReadPosition -= buffer->length;
			}
			delta = 0;
			buffer->release ();
			player->outputSampleQueue.pop ();
		}
		if (readahead) {
			player->isOutputReadaheadComplete = true;
		}
	}
	if (delta > 0) {
		player->outputCallback.callback (player->outputCallback.callbackData, NULL, (int) delta);
	}
}

void SoundMixer::audioCallback (void *userdata, Uint8 *stream, int len) {
	SoundMixer *it = (SoundMixer *) userdata;
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator i1, i2, qpos;
	SoundMixer::PlayerQueue *q;
	int writepos, writesize, framesize, volume;
	SharedBuffer *buffer;
	std::list<int64_t> endedids;
	std::list<int64_t>::const_iterator j1, j2;

	memset (stream, 0, len);
	i1 = it->queueMap.begin ();
	i2 = it->queueMap.end ();
	while (i1 != i2) {
		q = &(i1->second);
		writepos = 0;
		writesize = len;
		if (q->isPaused) {
			writesize = 0;
		}
		while ((writesize > 0) && (! q->sampleQueue.empty ())) {
			buffer = q->sampleQueue.front ();
			framesize = buffer->length - q->bufferPosition;
			if (framesize > writesize) {
				framesize = writesize;
			}
			volume = 0;
			if (! q->isMuted) {
				volume = q->mixVolume * it->masterMixVolume / SoundMixer::maxMixVolume;
				if (volume > SoundMixer::maxMixVolume) {
					volume = SoundMixer::maxMixVolume;
				}
			}
			if (volume > 0) {
				it->mixFn (stream + writepos, volume, buffer->data + q->bufferPosition, framesize);
			}
			writepos += framesize;
			writesize -= framesize;
			q->bufferPosition += framesize;
			q->writePosition += framesize;
			if (q->bufferPosition >= buffer->length) {
				q->bufferPosition = 0;
				q->sampleQueueSize -= buffer->length;
				q->sampleQueue.pop ();
				buffer->release ();
			}
		}
		if (q->sampleQueue.empty () && q->isEnded) {
			endedids.push_back (i1->first);
		}
		++i1;
	}

	j1 = endedids.cbegin ();
	j2 = endedids.cend ();
	while (j1 != j2) {
		qpos = it->queueMap.find (*j1);
		if (qpos != it->queueMap.end ()) {
			it->clearPlayerQueue (&(qpos->second));
			it->queueMap.erase (qpos);
		}
		++j1;
	}
}

void SoundMixer::mixSint8 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	int8_t *dest, *src, *srcend;
	int32_t n;

	dest = (int8_t *) destBuffer;
	src = (int8_t *) sourceBuffer;
	srcend = (int8_t *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		n = *src;
		n *= mixVolume;
		n /= SoundMixer::maxMixVolume;
		n += *dest;
		if (n > 0x7F) {
			*dest = 0x7F;
		}
		else if (n < -0x80) {
			*dest = 0x80;
		}
		else {
			*dest = (int8_t) n;
		}
		++dest;
		++src;
	}
}

void SoundMixer::mixUint8 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	uint8_t *dest, *src, *srcend;
	uint32_t n;

	dest = (uint8_t *) destBuffer;
	src = (uint8_t *) sourceBuffer;
	srcend = (uint8_t *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		n = *src;
		n *= mixVolume;
		n /= SoundMixer::maxMixVolume;
		n += *dest;
		if (n > 0xFF) {
			*dest = 0xFF;
		}
		else {
			*dest = (uint8_t) n;
		}
		++dest;
		++src;
	}
}

void SoundMixer::mixSint16 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	int16_t *dest, *src, *srcend;
	int32_t n;

	dest = (int16_t *) destBuffer;
	src = (int16_t *) sourceBuffer;
	srcend = (int16_t *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		n = *src;
		n *= mixVolume;
		n /= SoundMixer::maxMixVolume;
		n += *dest;
		if (n < -0x8000) {
			*dest = -0x8000;
		}
		else if (n > 0x7FFF) {
			*dest = 0x7FFF;
		}
		else {
			*dest = (int16_t) n;
		}
		++dest;
		++src;
	}
}

void SoundMixer::mixUint16 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	uint16_t *dest, *src, *srcend;
	uint32_t n;

	dest = (uint16_t *) destBuffer;
	src = (uint16_t *) sourceBuffer;
	srcend = (uint16_t *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		n = *src;
		n *= mixVolume;
		n /= SoundMixer::maxMixVolume;
		n += *dest;
		if (n > 0xFFFF) {
			*dest = 0xFFFF;
		}
		else {
			*dest = (uint16_t) n;
		}
		++dest;
		++src;
	}
}

void SoundMixer::mixSint32 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	int32_t *dest, *src, *srcend;
	int64_t n;
	const int32_t minLevel = -(0x80000000LL);
	const int32_t maxLevel = 0x7FFFFFFF;

	dest = (int32_t *) destBuffer;
	src = (int32_t *) sourceBuffer;
	srcend = (int32_t *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		n = *src;
		n *= mixVolume;
		n /= SoundMixer::maxMixVolume;
		n += *dest;
		if (n < minLevel) {
			*dest = minLevel;
		}
		else if (n > maxLevel) {
			*dest = maxLevel;
		}
		else {
			*dest = (int32_t) n;
		}
		++dest;
		++src;
	}
}

void SoundMixer::mixFloat32 (uint8_t *destBuffer, int mixVolume, uint8_t *sourceBuffer, int64_t sampleDataSize) {
	float *dest, *src, *srcend;

	dest = (float *) destBuffer;
	src = (float *) sourceBuffer;
	srcend = (float *) (sourceBuffer + sampleDataSize);
	while (src < srcend) {
		*dest += ((*src * (float) mixVolume) / (float) SoundMixer::maxMixVolume);;
		++dest;
		++src;
	}
}

void SoundMixer::loadSample (const char *soundId) {
	std::map<StdString, SoundSample *>::iterator pos;
	SoundSample *sample;

	if (! isActive) {
		return;
	}
	pos = sampleMap.find (StdString (soundId));
	if (pos != sampleMap.end ()) {
		sample = pos->second;
		++(sample->loadCount);
	}
	else {
		sample = new SoundSample (soundId, outputSampleRate, outputFormat, outputChannelCount);
		sample->retain ();
		sample->loadCount = 1;
		sampleMap.insert (std::pair<StdString, SoundSample *> (sample->soundId, sample));
	}
	sample->loadResource ();
}

void SoundMixer::unloadSample (const char *soundId) {
	std::map<StdString, SoundSample *>::iterator pos;

	if (! isActive) {
		return;
	}
	pos = sampleMap.find (StdString (soundId));
	if (pos == sampleMap.end ()) {
		return;
	}
	--(pos->second->loadCount);
	if (pos->second->loadCount <= 0) {
		pos->second->release ();
		sampleMap.erase (pos);
	}
}

int64_t SoundMixer::playResourceSample (const char *soundId, int mixVolume, bool muted, SoundMixer::OutputCallbackContext outputCallback) {
	std::map<StdString, SoundSample *>::iterator pos;
	SoundSample *sample;
	int64_t id;

	if (! isActive) {
		return (-1);
	}
	pos = sampleMap.find (StdString (soundId));
	if (pos == sampleMap.end ()) {
		return (-1);
	}
	id = -1;
	sample = pos->second;
	if (sample->isLoaded || (sample->outputBufferSize >= readaheadSize)) {
		id = App::instance->getUniqueId ();
		sample->retain ();
		SDL_LockMutex (playerMutex);
		playerMap.insert (std::pair<int64_t, SoundMixer::Player> (id, SoundMixer::Player (id, sample, false, outputCallback)));
		SDL_CondBroadcast (playerCond);
		SDL_UnlockMutex (playerMutex);

		SDL_LockAudioDevice (audioDeviceId);
		queueMap.insert (std::pair<int64_t, SoundMixer::PlayerQueue> (id, SoundMixer::PlayerQueue (false, mixVolume, muted)));
		SDL_UnlockAudioDevice (audioDeviceId);
	}
	return (id);
}

int64_t SoundMixer::playLiveSample (SoundSample *sample, int mixVolume, bool muted, SoundMixer::OutputCallbackContext outputCallback) {
	int64_t id;

	if (! isActive) {
		return (-1);
	}
	id = App::instance->getUniqueId ();
	sample->retain ();
	sample->setLive (id, SoundMixer::liveSampleFrameCallback, this);
	SDL_LockMutex (playerMutex);
	playerMap.insert (std::pair<int64_t, SoundMixer::Player> (id, SoundMixer::Player (id, sample, true, outputCallback)));
	SDL_CondBroadcast (playerCond);
	SDL_UnlockMutex (playerMutex);

	SDL_LockAudioDevice (audioDeviceId);
	queueMap.insert (std::pair<int64_t, SoundMixer::PlayerQueue> (id, SoundMixer::PlayerQueue (true, mixVolume, muted)));
	SDL_UnlockAudioDevice (audioDeviceId);

	return (id);
}
void SoundMixer::liveSampleFrameCallback (void *itPtr, SoundSample *sample, const SoundSample::AudioFrame &frame) {
	SoundMixer *it = (SoundMixer *) itPtr;
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	std::map<int64_t, SoundMixer::Player>::iterator playerpos;
	int playerid;

	if (! it->isActive) {
		return;
	}
	playerid = sample->livePlayerId;
	SDL_LockAudioDevice (it->audioDeviceId);
	qpos = it->queueMap.find (playerid);
	if (qpos != it->queueMap.end ()) {
		frame.sampleData->retain ();
		qpos->second.sampleQueue.push (frame.sampleData);
		qpos->second.sampleQueueSize += frame.sampleData->length;
	}
	SDL_UnlockAudioDevice (it->audioDeviceId);

	SDL_LockMutex (it->playerMutex);
	playerpos = it->playerMap.find (playerid);
	if (playerpos != it->playerMap.end ()) {
		if ((! playerpos->second.isEnded) && playerpos->second.outputCallback.callback) {
			frame.sampleData->retain ();
			playerpos->second.outputSampleQueue.push (frame.sampleData);
			SDL_CondBroadcast (it->playerCond);
		}
	}
	SDL_UnlockMutex (it->playerMutex);
}

bool SoundMixer::isPlayerActive (int64_t playerId) {
	std::map<int64_t, SoundMixer::Player>::iterator pos;
	bool result;

	result = false;
	SDL_LockMutex (playerMutex);
	pos = playerMap.find (playerId);
	if (pos != playerMap.end ()) {
		if (! pos->second.isEnded) {
			result = true;
		}
	}
	SDL_UnlockMutex (playerMutex);
	return (result);
}

void SoundMixer::stopPlayer (int64_t playerId) {
	std::map<int64_t, SoundMixer::Player>::iterator pos;
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	bool found;

	found = false;
	SDL_LockMutex (playerMutex);
	pos = playerMap.find (playerId);
	if (pos != playerMap.end ()) {
		found = true;
		clearPlayer (&(pos->second));
		playerMap.erase (pos);
	}
	SDL_UnlockMutex (playerMutex);

	if (found) {
		SDL_LockAudioDevice (audioDeviceId);
		qpos = queueMap.find (playerId);
		if (qpos != queueMap.end ()) {
			clearPlayerQueue (&(qpos->second));
			queueMap.erase (playerId);
		}
		SDL_UnlockAudioDevice (audioDeviceId);
	}
}

void SoundMixer::pausePlayer (int64_t playerId) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;

	SDL_LockAudioDevice (audioDeviceId);
	qpos = queueMap.find (playerId);
	if (qpos != queueMap.end ()) {
		qpos->second.isPaused = (! qpos->second.isPaused);
	}
	SDL_UnlockAudioDevice (audioDeviceId);
}

void SoundMixer::mutePlayer (int64_t playerId) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;

	SDL_LockAudioDevice (audioDeviceId);
	qpos = queueMap.find (playerId);
	if (qpos != queueMap.end ()) {
		qpos->second.isMuted = (! qpos->second.isMuted);
	}
	SDL_UnlockAudioDevice (audioDeviceId);
}

void SoundMixer::mutePlayer (int64_t playerId, bool muted) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;

	SDL_LockAudioDevice (audioDeviceId);
	qpos = queueMap.find (playerId);
	if (qpos != queueMap.end ()) {
		qpos->second.isMuted = muted;
	}
	SDL_UnlockAudioDevice (audioDeviceId);
}

void SoundMixer::setPlayerMixVolume (int64_t playerId, int mixVolume) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;

	if (mixVolume < 0) {
		mixVolume = 0;
	}
	if (mixVolume > SoundMixer::maxMixVolume) {
		mixVolume = SoundMixer::maxMixVolume;
	}
	SDL_LockAudioDevice (audioDeviceId);
	qpos = queueMap.find (playerId);
	if (qpos != queueMap.end ()) {
		qpos->second.mixVolume = mixVolume;
	}
	SDL_UnlockAudioDevice (audioDeviceId);
}

void SoundMixer::setOutputCallback (int64_t playerId, SoundMixer::OutputCallbackContext callback) {
	std::map<int64_t, SoundMixer::PlayerQueue>::iterator qpos;
	std::map<int64_t, SoundMixer::Player>::iterator playerpos;
	int64_t writepos, queuesize;

	writepos = 0;
	queuesize = 0;
	if (callback.callback) {
		SDL_LockAudioDevice (audioDeviceId);
		qpos = queueMap.find (playerId);
		if (qpos != queueMap.end ()) {
			writepos = qpos->second.writePosition;
			queuesize = qpos->second.sampleQueueSize;
		}
		SDL_UnlockAudioDevice (audioDeviceId);
	}

	SDL_LockMutex (playerMutex);
	playerpos = playerMap.find (playerId);
	if (playerpos != playerMap.end ()) {
		playerpos->second.outputCallback = callback;
		playerpos->second.outputQueueWritePosition = writepos;
		playerpos->second.outputReadPosition = -queuesize;
		playerpos->second.isOutputReadaheadComplete = false;
		if (! playerpos->second.outputCallback.callback) {
			clearPlayerOutputSamples (&(playerpos->second));
		}
		else {
			SDL_CondBroadcast (playerCond);
		}
	}
	SDL_UnlockMutex (playerMutex);
}
