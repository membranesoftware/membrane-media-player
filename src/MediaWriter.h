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
// Class that writes stream data to media files
#ifndef MEDIA_WRITER_H
#define MEDIA_WRITER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

class SharedBuffer;

class MediaWriter {
public:
	MediaWriter ();
	~MediaWriter ();

	static constexpr const char *defaultVideoCodecName = "libx264";
	static constexpr const double defaultVideoExpectedFrameRate = 30.0f;
	static constexpr const double defaultVideoBitrateMultiplier = 0.068f;
	static constexpr const double defaultVideoRatecontrolRangeMultiplier = 0.15f;
	static constexpr const double defaultVideoRatecontrolBufferMultiplier = 1.5f;
	static constexpr const double defaultVideoGopRate = 0.5f;
	static constexpr const int defaultVideoMaxBframes = 1;
	static constexpr const char *defaultX264Preset = "medium";
	static constexpr const int defaultVideoFrameDropLag = 10000;

	// Read-write data members
	StdString videoCodecName;
	int videoWidth;
	int videoHeight;
	AVPixelFormat videoPixelFormat;
	double videoExpectedFrameRate; // Frames per second
	int videoFrameDropLag;
	int64_t videoBitrate;
	double videoBitrateMultiplier;
	double videoRatecontrolRangeMultiplier;
	double videoRatecontrolBufferMultiplier;
	double videoGopRate; // Seconds per group of pictures
	int videoMaxBframes;

	// Read-only data members
	StdString writePath;
	bool isWriting;
	bool isWriteFailed;
	bool isRunning;
	bool isStopped;
	StdString lastErrorMessage;
	int64_t writeReferenceTime;
	int64_t frameReceiveCount;
	int64_t packetWriteCount;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Open the output stream and begin media write
	void start (const StdString &writePathValue);

	// Stop media write operations
	void stop ();

	// Add frame data to the encode queue
	void writeVideoFrame (SharedBuffer *imageData, int64_t captureTime, int imageLineSize, int imageWidth, int imageHeight);

private:
	struct VideoFrame {
		int64_t id;
		int64_t pts;
		SharedBuffer *imageData;
		int imageLineSize;
		int imageWidth;
		int imageHeight;

		VideoFrame ():
			id (0),
			pts (0),
			imageData (NULL),
			imageLineSize (0),
			imageWidth (0),
			imageHeight (0) { }
	};

	// Free objects allocated during the write operation
	void clearWrite ();

	// Free data allocated by swsContext and related values
	void clearSwsContext ();

	// Remove all items from the frames list
	void clearFrames ();

	// Set failure state for a play operation
	void failWrite (const StdString &lastErrorMessageValue, const char *logErrorMessage = NULL);

	// Write packets to the output context as frames are received
	static void writePackets (void *itPtr);
	void executeWritePackets ();
	bool addVideoStream ();
	bool resetSwsContext (int sourceWidth, int sourceHeight);
	bool processVideoFrame ();
	bool writeVideoPackets ();

	AVFormatContext *avFormatContext;
	AVCodec *videoCodec;
	AVCodecContext *videoCodecContext;
	AVStream *videoStream;
	AVPacket *avPacket;
	AVFrame *avFrame;
	SwsContext *swsContext;
	int swsSourceWidth;
	int swsSourceHeight;
	std::list<MediaWriter::VideoFrame> frames;
	SDL_mutex *framesMutex;
	SDL_cond *framesCond;
	MediaWriter::VideoFrame nextVideoFrame;
	uint8_t *videoFrameEncodeSlice[AV_NUM_DATA_POINTERS];
	int videoFrameEncodeStride[AV_NUM_DATA_POINTERS];
	int refcount;
	SDL_mutex *refcountMutex;
};
#endif
