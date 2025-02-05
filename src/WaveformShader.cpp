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
#include "SdlUtil.h"
#include "ClassId.h"
#include "OsUtil.h"
#include "SharedBuffer.h"
#include "Particle.h"
#include "Video.h"
#include "SoundMixer.h"
#include "WaveformShader.h"

constexpr const double particleColorR = 0.3f;
constexpr const double particleColorG = 1.0f;
constexpr const double particleColorB = 0.3f;
constexpr const int decayPeriod = 200;
constexpr const double decayScale = 0.65f;

WaveformShader::WaveformShader (double shaderWidth, double shaderHeight, double particleDensity)
: Shader ()
, shaderWidth (shaderWidth)
, shaderHeight (shaderHeight)
, particleDensity (particleDensity)
, frameDuration (25)
, soundPlayerId (-1)
, audioSampleFormat (AUDIO_F32)
, audioSampleRate (1)
, audioSampleChannelCount (1)
, audioSampleSize (1)
, sourceSampleSize (0)
, readSampleFn (WaveformShader::readFloat32Sample)
, isDecayActive (false)
, decayClock (0)
, sourceVideoHandle (&sourceVideo)
, shouldStopSoundPlayer (false)
, bufferPosition (0)
, frameSampleCount (0)
, frameRenderClock (0)
, soundPlayerWriteDelta (0)
, sampleConsumeCount (0)
, sampleReceiveCount (0)
{
	classId = ClassId::WaveformShader;
	this->shaderWidth = floor (this->shaderWidth);
	if (this->shaderWidth < 0.0f) {
		this->shaderWidth = 0.0f;
	}
	this->shaderHeight = floor (this->shaderHeight);
	if (this->shaderHeight < 0.0f) {
		this->shaderHeight = 0.0f;
	}
	if (this->particleDensity > 1.0f) {
		this->particleDensity = 1.0f;
	}
	SdlUtil::createMutex (&particleMutex);
	SdlUtil::createMutex (&sourceSampleMutex);
	width = this->shaderWidth;
	height = this->shaderHeight;
}
WaveformShader::~WaveformShader () {
	setSourceVideo (NULL);
	if (soundPlayerId >= 0) {
		if (shouldStopSoundPlayer) {
			SoundMixer::instance->stopPlayer (soundPlayerId);
		}
		soundPlayerId = -1;
	}
	clearSourceSamples ();
	SdlUtil::destroyMutex (&particleMutex);
	SdlUtil::destroyMutex (&sourceSampleMutex);
}

WaveformShader *WaveformShader::castWidget (Widget *widget) {
	return (Widget::isWidgetClass (widget, ClassId::WaveformShader) ? (WaveformShader *) widget : NULL);
}

void WaveformShader::clearSourceSamples () {
	std::list<SharedBuffer *>::const_iterator i1, i2;
	SharedBuffer *buffer;

	SDL_LockMutex (sourceSampleMutex);
	i1 = sourceSamples.cbegin ();
	i2 = sourceSamples.cend ();
	while (i1 != i2) {
		buffer = *i1;
		buffer->release ();
		++i1;
	}
	sourceSamples.clear ();
	sourceSampleSize = 0;
	sampleConsumeCount = 0;
	sampleReceiveCount = 0;
	SDL_UnlockMutex (sourceSampleMutex);
}

bool WaveformShader::updateRenderState (int msElapsed) {
	int i, particlecount, samplecount, advance;
	double x, y, dx, xmax;
	bool paused, shouldupdate, firstbuffer, result;
	std::list<SharedBuffer *>::iterator i1, i2;
	SharedBuffer *buffer;

	if (soundPlayerId >= 0) {
		if (! SoundMixer::instance->isPlayerActive (soundPlayerId)) {
			soundPlayerId = -1;
		}
	}
	sourceVideoHandle.compact ();
	paused = false;
	if (sourceVideo) {
		paused = sourceVideo->isPaused;
		if (soundPlayerId < 0) {
			soundPlayerId = sourceVideo->soundPlayerId;
			if (soundPlayerId >= 0) {
				clearSourceSamples ();
				SoundMixer::instance->setOutputCallback (soundPlayerId, SoundMixer::OutputCallbackContext (WaveformShader::soundMixerOutput, this));
				bufferPosition = 0;
				frameRenderClock = 0;
				soundPlayerWriteDelta = 0;
			}
		}
	}

	result = false;
	SDL_LockMutex (particleMutex);
	if (particles.empty () && (shaderWidth >= 1.0f) && (shaderHeight >= 1.0f)) {
		result = true;
		particlecount = (int) floor (shaderWidth * particleDensity);
		if (particlecount < 4) {
			particlecount = 4;
		}
		xmax = shaderWidth - 1.0f;
		dx = xmax / (double) (particlecount - 1);
		x = 0.0f;
		y = shaderHeight / 2.0f;
		for (i = 0; i < particlecount; ++i) {
			particles.push_back (Particle (particleColorR, particleColorG, particleColorB, x, y));
			x += dx;
			if (x > xmax) {
				x = xmax;
			}
		}
	}
	particlecount = (int) particles.size ();
	SDL_UnlockMutex (particleMutex);

	if ((particlecount > 0) && (! paused) && (audioSampleRate > 0) && (audioSampleSize > 0)) {
		shouldupdate = false;
		frameSamples.clear ();
		SDL_LockMutex (sourceSampleMutex);
		if (sourceSampleSize > 0) {
			if (frameRenderClock > 0) {
				frameRenderClock -= msElapsed;
			}
			if (frameRenderClock <= 0) {
				frameRenderClock += frameDuration;
				if (frameRenderClock < 0) {
					frameRenderClock = 0;
				}
				shouldupdate = true;
			}
			if (soundPlayerWriteDelta > 0) {
				bufferPosition += soundPlayerWriteDelta;
				soundPlayerWriteDelta = 0;
			}
			else if (soundPlayerId < 0) {
				advance = (int) ceil ((double) msElapsed * (double) audioSampleRate / 1000.0f);
				advance *= audioSampleSize;
				bufferPosition += advance;
			}
			while (! sourceSamples.empty ()) {
				i1 = sourceSamples.begin ();
				buffer = *i1;
				if (bufferPosition < buffer->length) {
					break;
				}
				sourceSamples.erase (i1);
				bufferPosition -= buffer->length;
				sourceSampleSize -= buffer->length;
				sampleConsumeCount += (buffer->length / audioSampleSize);
				buffer->release ();
			}
			if (shouldupdate) {
				firstbuffer = true;
				samplecount = 0;
				i1 = sourceSamples.begin ();
				i2 = sourceSamples.end ();
				while (i1 != i2) {
					buffer = *i1;
					samplecount += (buffer->length / audioSampleSize);
					if (firstbuffer) {
						firstbuffer = false;
						if (bufferPosition > 0) {
							samplecount -= (bufferPosition / audioSampleSize);
						}
					}
					buffer->retain ();
					frameSamples.push_back (buffer);
					if (samplecount >= frameSampleCount) {
						break;
					}
					++i1;
				}
			}
		}
		SDL_UnlockMutex (sourceSampleMutex);
		if (shouldupdate && (! frameSamples.empty ())) {
			assignParticlePositions ();
			result = true;
		}
		if (result) {
			decayClock = 0;
			isDecayActive = true;
		}
		else {
			if (isDecayActive) {
				decayClock += msElapsed;
				while (decayClock >= decayPeriod) {
					decayClock -= decayPeriod;
					if (! decayParticlePositions ()) {
						isDecayActive = false;
						break;
					}
					result = true;
				}
			}
		}
	}
	return (result);
}
void WaveformShader::assignParticlePositions () {
	std::vector<Particle>::iterator i1, i2;
	std::list<SharedBuffer *>::const_iterator j1, j2;
	SharedBuffer *buffer;
	WaveformShader::ReadSampleFunction fn;
	uint8_t *buf;
	double y0, y, sum;
	int pos, buflen, particlecount, samplecount, sumcount, delta;

	fn = readSampleFn;
	y0 = shaderHeight / 2.0f;
	y = y0;
	pos = bufferPosition;
	samplecount = 0;
	delta = 0;
	sumcount = 0;
	sum = 0.0f;

	SDL_LockMutex (particleMutex);
	particlecount = (int) particles.size ();
	i1 = particles.begin ();
	i2 = particles.end ();
	j1 = frameSamples.cbegin ();
	j2 = frameSamples.cend ();
	while (j1 != j2) {
		buffer = *j1;
		buf = buffer->data;
		buflen = buffer->length;
		if (pos > 0) {
			buf += pos;
			buflen -= pos;
			pos = 0;
		}
		while (buflen >= audioSampleSize) {
			if ((i1 == i2) || (samplecount >= frameSampleCount)) {
				break;
			}
			sum += fn (buf, audioSampleChannelCount);
			++sumcount;
			++samplecount;
			delta += particlecount;
			y = y0 - ((sum / (double) sumcount) * y0);
			if (delta >= frameSampleCount) {
				i1->position.assignY (y);
				++i1;
				delta -= frameSampleCount;
				while (delta >= frameSampleCount) {
					if (i1 == i2) {
						break;
					}
					i1->position.assignY (y);
					++i1;
					delta -= frameSampleCount;
				}
				sum = 0.0f;
				sumcount = 0;
			}
			buf += audioSampleSize;
			buflen -= audioSampleSize;
		}

		++j1;
	}
	while (i1 != i2) {
		i1->position.assignY (y);
		++i1;
	}
	SDL_UnlockMutex (particleMutex);

	j1 = frameSamples.cbegin ();
	j2 = frameSamples.cend ();
	while (j1 != j2) {
		buffer = *j1;
		buffer->release ();
		++j1;
	}
	frameSamples.clear ();
}
bool WaveformShader::decayParticlePositions () {
	std::vector<Particle>::iterator i1, i2;
	double y, dy;
	bool found;

	y = shaderHeight / 2.0f;
	SDL_LockMutex (particleMutex);
	found = false;
	i1 = particles.begin ();
	i2 = particles.end ();
	while (i1 != i2) {
		dy = i1->position.y - y;
		if (FLOAT_EQUALS (dy, 0.0f)) {
			i1->position.assignY (y);
		}
		else {
			found = true;
			dy *= decayScale;
			if (dy < 1.0f) {
				i1->position.assignY (y);
			}
			else {
				i1->position.assignY (y + dy);
			}
		}
		++i1;
	}
	SDL_UnlockMutex (particleMutex);
	return (found);
}

void WaveformShader::updateRenderTexture () {
	std::vector<Particle>::const_iterator i1, i2;
	int x1, y1, x2, y2;
	bool first;

	memset (renderPixels, 0, textureHeight * renderPitch);
	SDL_LockMutex (particleMutex);
	first = true;
	i1 = particles.cbegin ();
	i2 = particles.cend ();
	while (i1 != i2) {
		if (first) {
			first = false;
			x1 = (int) i1->position.x;
			y1 = (int) i1->position.y;
		}
		else {
			x2 = (int) i1->position.x;
			y2 = (int) i1->position.y;
			drawLine (i1->color, x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
		++i1;
	}
	SDL_UnlockMutex (particleMutex);
}

void WaveformShader::soundMixerOutput (void *itPtr, SharedBuffer *sampleData, int playerWriteDelta) {
	WaveformShader *it = (WaveformShader *) itPtr;

	if (it->isDestroyed || (it->audioSampleSize <= 0)) {
		return;
	}
	if (sampleData) {
		sampleData->retain ();
	}
	SDL_LockMutex (it->sourceSampleMutex);
	if (sampleData) {
		it->sourceSamples.push_back (sampleData);
		it->sourceSampleSize += sampleData->length;
		it->sampleReceiveCount += (sampleData->length / it->audioSampleSize);
	}
	it->soundPlayerWriteDelta += playerWriteDelta;
	SDL_UnlockMutex (it->sourceSampleMutex);
}

void WaveformShader::assignReadSampleFn () {
	switch (audioSampleFormat) {
		case AUDIO_S8: {
			readSampleFn = WaveformShader::readSint8Sample;
			break;
		}
		case AUDIO_U8: {
			readSampleFn = WaveformShader::readUint8Sample;
			break;
		}
		case AUDIO_S16LSB:
		case AUDIO_S16MSB: {
			readSampleFn = WaveformShader::readSint16Sample;
			break;
		}
		case AUDIO_U16LSB:
		case AUDIO_U16MSB: {
			readSampleFn = WaveformShader::readUint16Sample;
			break;
		}
		case AUDIO_S32LSB:
		case AUDIO_S32MSB: {
			readSampleFn = WaveformShader::readSint32Sample;
			break;
		}
		default: {
			readSampleFn = WaveformShader::readFloat32Sample;
			break;
		}
	}
}
double WaveformShader::readSint8Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	int32_t sum;
	int count;

	b = sampleBuffer;
	sum = 0;
	count = channelCount;
	while (count > 0) {
		sum += (*((int8_t *) b)) + 128;
		b += 1;
		--count;
	}
	return (-1.0f + (2.0f * (double) sum / (double) channelCount / 255.0f));
}
double WaveformShader::readUint8Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	int32_t sum;
	int count;

	b = sampleBuffer;
	sum = 0;
	count = channelCount;
	while (count > 0) {
		sum += *((uint8_t *) b);
		b += 1;
		--count;
	}
	return (-1.0f + (2.0f * (double) sum / (double) channelCount / 255.0f));
}
double WaveformShader::readSint16Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	int32_t sum;
	int count;

	b = sampleBuffer;
	sum = 0;
	count = channelCount;
	while (count > 0) {
		sum += (*((int16_t *) b)) + 32768;
		b += 2;
		--count;
	}
	return (-1.0f + (2.0f * (double) sum / (double) channelCount / 65535.0f));
}
double WaveformShader::readUint16Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	int32_t sum;
	int count;

	b = sampleBuffer;
	sum = 0;
	count = channelCount;
	while (count > 0) {
		sum += *((uint16_t *) b);
		b += 2;
		--count;
	}
	return (-1.0f + (2.0f * (double) sum / (double) channelCount / 65535.0f));
}
double WaveformShader::readSint32Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	int64_t sum;
	int count;

	b = sampleBuffer;
	sum = 0;
	count = channelCount;
	while (count > 0) {
		sum += *((int32_t *) b);
		sum += 2147483648;
		b += 4;
		--count;
	}
	return (-1.0f + (2.0f * (double) sum / (double) channelCount / 4294967296.0f));
}
double WaveformShader::readFloat32Sample (uint8_t *sampleBuffer, int channelCount) {
	uint8_t *b;
	float sum;
	int count;

	b = sampleBuffer;
	sum = 0.0f;
	count = channelCount;
	while (count > 0) {
		sum += *((float *) b);
		b += 4;
		--count;
	}
	return ((double) sum / (double) channelCount);
}

bool WaveformShader::playResourceSample (const char *soundId, int mixVolume, bool muted) {
	audioSampleFormat = SoundMixer::instance->outputFormat;
	audioSampleRate = SoundMixer::instance->outputSampleRate;
	audioSampleChannelCount = SoundMixer::instance->outputChannelCount;
	audioSampleSize = SoundMixer::instance->outputSampleSize;
	if ((audioSampleRate < 1) || (audioSampleChannelCount < 1) || (audioSampleSize < 1)) {
		return (false);
	}
	audioSampleSize *= audioSampleChannelCount;
	frameSampleCount = frameDuration * audioSampleRate / 1000;
	assignReadSampleFn ();
	if (soundPlayerId >= 0) {
		if (shouldStopSoundPlayer) {
			SoundMixer::instance->stopPlayer (soundPlayerId);
			shouldStopSoundPlayer = false;
		}
	}
	soundPlayerId = SoundMixer::instance->playResourceSample (soundId, mixVolume, muted, SoundMixer::OutputCallbackContext (WaveformShader::soundMixerOutput, this));
	if (soundPlayerId < 0) {
		return (false);
	}
	bufferPosition = 0;
	frameRenderClock = 0;
	shouldStopSoundPlayer = true;
	return (true);
}

void WaveformShader::setSourceVideo (Video *video) {
	if (sourceVideo == video) {
		return;
	}
	if (sourceVideo) {
		if (soundPlayerId >= 0) {
			SoundMixer::instance->setOutputCallback (soundPlayerId, SoundMixer::OutputCallbackContext ());
		}
	}
	sourceVideoHandle.assign (video);
	soundPlayerId = -1;
	if (sourceVideo) {
		audioSampleFormat = SoundMixer::instance->outputFormat;
		audioSampleRate = SoundMixer::instance->outputSampleRate;
		audioSampleChannelCount = SoundMixer::instance->outputChannelCount;
		audioSampleSize = SoundMixer::instance->outputSampleSize;
		if ((audioSampleRate < 1) || (audioSampleChannelCount < 1) || (audioSampleSize < 1)) {
			return;
		}
		audioSampleSize *= audioSampleChannelCount;
		frameSampleCount = frameDuration * audioSampleRate / 1000;
		assignReadSampleFn ();
		shouldStopSoundPlayer = false;
	}
}

void WaveformShader::setShaderSize (double targetWidth, double targetHeight) {
	double w, h;

	w = floor (targetWidth);
	h = floor (targetHeight);
	if (w < 0.0f) {
		w = 0.0f;
	}
	if (h < 0.0f) {
		h = 0.0f;
	}
	if (FLOAT_EQUALS (shaderWidth, w) && FLOAT_EQUALS (shaderHeight, h)) {
		return;
	}
	width = w;
	height = h;
	SDL_LockMutex (particleMutex);
	particles.clear ();
	shaderWidth = w;
	shaderHeight = h;
	SDL_UnlockMutex (particleMutex);
}
