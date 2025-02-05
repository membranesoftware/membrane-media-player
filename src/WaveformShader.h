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
// Shader that renders a waveform animation from audio sample data
#ifndef WAVEFORM_SHADER_H
#define WAVEFORM_SHADER_H

#include "Particle.h"
#include "SoundMixer.h"
#include "WidgetHandle.h"
#include "Shader.h"

class Video;
class SharedBuffer;

class WaveformShader : public Shader {
public:
	WaveformShader (double shaderWidth, double shaderHeight, double particleDensity = 0.25f);
	~WaveformShader ();

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static WaveformShader *castWidget (Widget *widget);

	// Read-only data members
	double shaderWidth;
	double shaderHeight;
	double particleDensity;
	int frameDuration;
	int64_t soundPlayerId;
	SDL_AudioFormat audioSampleFormat;
	int audioSampleRate;
	int audioSampleChannelCount;
	int audioSampleSize;
	Video *sourceVideo;

	// Execute a play operation for the specified sample, store the created player ID, and return true if successful
	bool playResourceSample (const char *soundId, int mixVolume = SoundMixer::maxMixVolume, bool muted = false);

	// Set a Video widget that should provide audio data for waveform render, or clear any existing source video if NULL
	void setSourceVideo (Video *video);

	// Set the size of the shader texture
	void setShaderSize (double targetWidth, double targetHeight);

protected:
	// Update render state as appropriate for an elapsed millisecond time period and return true if render surface updates are needed
	bool updateRenderState (int msElapsed);

	// Update the content of renderTexture as appropriate for render state
	void updateRenderTexture ();

private:
	// Remove all items from sourceSamples
	void clearSourceSamples ();

	// Reset readSampleFn for use with audioSampleFormat
	void assignReadSampleFn ();

	// Reposition particles as appropriate for data in frameSamples
	void assignParticlePositions ();

	// Reposition particles as appropriate for a period of time without sample data and return true if any positions were changed
	bool decayParticlePositions ();

	// Return a sample value from sampleBuffer, scaled to a range from -1.0 to 1.0.
	typedef double (*ReadSampleFunction) (uint8_t *sampleBuffer, int channelCount);
	static double readSint8Sample (uint8_t *sampleBuffer, int channelCount);
	static double readUint8Sample (uint8_t *sampleBuffer, int channelCount);
	static double readSint16Sample (uint8_t *sampleBuffer, int channelCount);
	static double readUint16Sample (uint8_t *sampleBuffer, int channelCount);
	static double readSint32Sample (uint8_t *sampleBuffer, int channelCount);
	static double readFloat32Sample (uint8_t *sampleBuffer, int channelCount);

	// Callback functions
	static void soundMixerOutput (void *itPtr, SharedBuffer *sampleData, int playerWriteDelta);

	std::vector<Particle> particles;
	SDL_mutex *particleMutex;
	std::list<SharedBuffer *> sourceSamples;
	SDL_mutex *sourceSampleMutex;
	int sourceSampleSize;
	WaveformShader::ReadSampleFunction readSampleFn;
	bool isDecayActive;
	int decayClock;
	WidgetHandle<Video> sourceVideoHandle;
	bool shouldStopSoundPlayer;
	int bufferPosition;
	int frameSampleCount;
	int frameRenderClock;
	int soundPlayerWriteDelta;
	std::list<SharedBuffer *> frameSamples;
	int64_t sampleConsumeCount;
	int64_t sampleReceiveCount;
};
#endif
