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
// Class that reads data from media files
#ifndef MEDIA_READER_H
#define MEDIA_READER_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libswscale/swscale.h"
}
#include "MediaUtil.h"

class MediaReader {
public:
	MediaReader ();
	~MediaReader ();

	// Read-only data members
	StdString mediaPath;
	StdString lastErrorMessage;
	int64_t duration;
	int64_t mediaFileSize;
	int64_t totalBitrate;
	bool isVideo;
	bool isAudio;
	bool hasAudioAlbumArt;
	int videoFrameWidth;
	int videoFrameHeight;
	double videoFrameRate;
	int64_t videoBitrate;
	int audioChannelCount;
	int audioSampleRate;
	int64_t audioBitrate;
	uint8_t *videoFrameData;
	int videoFramePitch;
	int videoFrameScaledWidth;
	int videoFrameScaledHeight;
	int64_t videoFrameTimestamp;
	SDL_Texture *videoFrameTexture;
	StdString videoFrameTexturePath;
	StdString writeOutputPath;
	int jpegQuality;

	// Increase the object's refcount
	void retain ();

	// Decrease the object's refcount. If this reduces the refcount to zero or less, delete the object.
	void release ();

	// Set the media path targeted by the reader
	void setMediaPath (const StdString &mediaPathValue, bool isResourceMediaPathValue = false);

	// Read metadata from the targeted media file and return a Result value
	OpResult readMetadata ();

	// Set the seek position for the readVideoFrame operation
	void setVideoFrameSeekPercent (double frameSeekPercentValue);
	void setVideoFrameSeekTimestamp (int64_t frameSeekTimestampValue);

	typedef void (*ReadVideoFrameCallback) (void *data, MediaReader *reader);
	// Populate videoFrameData and videoFramePitch with a video frame from a media file and invoke callback when complete. If frameScaleWidth or frameScaleHeight are zero or less, choose a value that preserves the source aspect ratio. If callback is not provided, execute the read operation inline before returning.
	void readVideoFrame (int frameScaleWidth = 0, int frameScaleHeight = 0, MediaReader::ReadVideoFrameCallback callback = NULL, void *callbackData = NULL);

	typedef bool (*CreateTextureCallback) (void *data, MediaReader *reader, SDL_Texture *texture, const StdString &texturePath);
	// Create an SDL_Texture from previously loaded videoFrameData and invoke callback when complete. The callback should return true if it successfully processes the texture, or false if the reader should destroy any created texture.
	void createVideoFrameTexture (MediaReader::CreateTextureCallback callback, void *callbackData);

	static constexpr const int defaultJpegQuality = 66;
	typedef void (*WriteVideoFrameJpegCallback) (void *data, MediaReader *reader);
	// Write a jpg file from previously loaded videoFrameData and invoke callback when complete. If callback is not provided, execute the write operation inline before returning.
	void writeVideoFrameJpeg (const StdString &outputPath, int jpegQualityValue = MediaReader::defaultJpegQuality, MediaReader::WriteVideoFrameJpegCallback callback = NULL, void *callbackData = NULL);

private:
	// Reset metadata fields to default values
	void clearMetadata ();

	// Free objects allocated during the read operation
	void clearRead ();

	// Free frame data created by the readVideoFrame operation
	void clearVideoFrame ();

	// End the read operation
	void endRead (const StdString &errorMessage = StdString ());

	// Task functions
	static void readFrame (void *itPtr);
	void executeReadFrame ();
	static void createTexture (void *itPtr);
	void executeCreateTexture ();
	static void endCreateTexture (void *itPtr);
	static void writeJpeg (void *itPtr);
	void executeWriteJpeg ();

	static constexpr const int imageDataPlaneCount = 4;

	AVIOContext *avioContext;
	AVFormatContext *avFormatContext;
	int videoStream;
	int audioStream;
	AVCodec *videoCodec;
	AVCodecContext *videoCodecContext;
	AVCodec *audioCodec;
	AVCodecContext *audioCodecContext;
	AVPacket *avPacket;
	AVFrame *avFrame;
	SwsContext *swsContext;
	uint8_t *swsImageData[MediaReader::imageDataPlaneCount];
	int swsImageLineSizes[MediaReader::imageDataPlaneCount];
	MediaUtil::SdlRwOps rwops;
	double frameSeekPercent;
	int64_t frameSeekTimestamp;
	bool isResourceMediaPath;
	MediaReader::ReadVideoFrameCallback readVideoFrameCallback;
	void *readVideoFrameCallbackData;
	MediaReader::CreateTextureCallback createTextureCallback;
	void *createTextureCallbackData;
	MediaReader::WriteVideoFrameJpegCallback writeVideoFrameJpegCallback;
	void *writeVideoFrameJpegCallbackData;
	int refcount;
	SDL_mutex *refcountMutex;
};
#endif
