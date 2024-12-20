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
#include "SDL2/SDL_image.h"
#include "App.h"
#include "SdlUtil.h"
#include "OsUtil.h"
#include "SharedBuffer.h"
#include "TaskGroup.h"
#include "RenderResource.h"
#include "UiText.h"
#include "MediaWriter.h"
#include "CaptureWriter.h"

CaptureWriter *CaptureWriter::instance = NULL;

CaptureWriter::CaptureWriter ()
: imageWriteCount (0)
, mediaWriter (NULL)
, imageWriteData (NULL)
, imageWriteWidth (0)
, imageWriteHeight (0)
, imageWritePitch (0)
, isCapturingFrame (false)
, isWritingImage (false)
{
	SdlUtil::createMutex (&baseWritePathMutex);
	SdlUtil::createMutex (&captureMutex);
}
CaptureWriter::~CaptureWriter () {
	SdlUtil::destroyMutex (&baseWritePathMutex);
	SDL_LockMutex (captureMutex);
	if (mediaWriter) {
		mediaWriter->release ();
		mediaWriter = NULL;
	}
	if (imageWriteData) {
		imageWriteData->release ();
		imageWriteData = NULL;
	}
	SDL_UnlockMutex (captureMutex);
	SdlUtil::destroyMutex (&captureMutex);
}

void CaptureWriter::createInstance () {
	if (! CaptureWriter::instance) {
		CaptureWriter::instance = new CaptureWriter ();
	}
}
void CaptureWriter::freeInstance () {
	if (CaptureWriter::instance) {
		delete (CaptureWriter::instance);
		CaptureWriter::instance = NULL;
	}
}

void CaptureWriter::setBaseWritePath (const StdString &path) {
	SDL_LockMutex (baseWritePathMutex);
	baseWritePath.assign (path);
	SDL_UnlockMutex (baseWritePathMutex);
}

StdString CaptureWriter::getMediaWritePath () {
	StdString s;

	s.sprintf ("%s_%s.%s", APPLICATION_PACKAGE_NAME, UiText::instance->getTimestampNumber (OsUtil::getTime ()).c_str (), CaptureWriter::mediaWriteFormat);
	SDL_LockMutex (baseWritePathMutex);
	if (! baseWritePath.empty ()) {
		s = OsUtil::getJoinedPath (baseWritePath, s);
	}
	SDL_UnlockMutex (baseWritePathMutex);
	return (s);
}

StdString CaptureWriter::getImageWritePath () {
	StdString s;

	s.sprintf ("%s_%s.%s", APPLICATION_PACKAGE_NAME, UiText::instance->getTimestampNumber (OsUtil::getTime ()).c_str (), CaptureWriter::imageWriteFormat);
	SDL_LockMutex (baseWritePathMutex);
	if (! baseWritePath.empty ()) {
		s = OsUtil::getJoinedPath (baseWritePath, s);
	}
	SDL_UnlockMutex (baseWritePathMutex);
	return (s);
}

void CaptureWriter::startMediaWriter (const StdString &writePath) {
	StdString path;

	if (writePath.empty ()) {
		path.assign (getMediaWritePath ());
	}
	else {
		path.assign (writePath);
		if (! OsUtil::getPathExtension (path).equals (CaptureWriter::mediaWriteFormat)) {
			path.appendSprintf (".%s", CaptureWriter::mediaWriteFormat);
		}
	}
	SDL_LockMutex (captureMutex);
	if (!(mediaWriter && mediaWriter->writePath.equals (path))) {
		nextMediaWriterPath.assign (path);
	}
	SDL_UnlockMutex (captureMutex);
}

void CaptureWriter::stopMediaWriter () {
	SDL_LockMutex (captureMutex);
	if (mediaWriter) {
		mediaWriter->stop ();
		mediaWriter->release ();
		mediaWriter = NULL;
	}
	SDL_UnlockMutex (captureMutex);
}

void CaptureWriter::toggleMediaWriter (const StdString &writePath) {
	StdString path;

	SDL_LockMutex (captureMutex);
	if (mediaWriter) {
		mediaWriter->stop ();
		mediaWriter->release ();
		mediaWriter = NULL;
	}
	else {
		if (writePath.empty ()) {
			path.assign (getMediaWritePath ());
		}
		else {
			path.assign (writePath);
			if (! OsUtil::getPathExtension (path).equals (CaptureWriter::mediaWriteFormat)) {
				path.appendSprintf (".%s", CaptureWriter::mediaWriteFormat);
			}
		}
		nextMediaWriterPath.assign (path);
	}
	SDL_UnlockMutex (captureMutex);
}

void CaptureWriter::writeRenderFrameImage (const StdString &imageFilename) {
	StdString path;

	if (imageFilename.empty ()) {
		path.assign (getImageWritePath ());
	}
	else {
		path.assign (imageFilename);
		if (! OsUtil::getPathExtension (path).equals (CaptureWriter::imageWriteFormat)) {
			path.appendSprintf (".%s", CaptureWriter::imageWriteFormat);
		}
	}
	SDL_LockMutex (captureMutex);
	imageWritePaths.push_back (path);
	SDL_UnlockMutex (captureMutex);
}

void CaptureWriter::update (int msElapsed) {
	bool shouldwriteimage, shouldcapture;

	shouldwriteimage = false;
	shouldcapture = false;
	SDL_LockMutex (captureMutex);
	if (imageWriteData) {
		if (imageWritePaths.empty ()) {
			imageWriteData->release ();
			imageWriteData = NULL;
		}
		else {
			if (! isWritingImage) {
				shouldwriteimage = true;
			}
		}
	}
	if (! isCapturingFrame) {
		if (! nextMediaWriterPath.empty ()) {
			if (mediaWriter) {
				mediaWriter->stop ();
				mediaWriter->release ();
			}
			mediaWriter = new MediaWriter ();
			mediaWriter->retain ();
			mediaWriter->videoWidth = App::instance->windowWidth;
			mediaWriter->videoHeight = App::instance->windowHeight;
			mediaWriter->videoExpectedFrameRate = 1000.0f / (double) (App::instance->minUpdateFrameDelay * 2);
			mediaWriter->start (nextMediaWriterPath);
			if (mediaWriter->isWriteFailed) {
				Log::debug ("Failed to start stream publish; err=%s", mediaWriter->lastErrorMessage.c_str ());
			}
			nextMediaWriterPath.assign ("");
		}
		if (mediaWriter && mediaWriter->isWriting) {
			shouldcapture = true;
		}
		else if ((! imageWritePaths.empty ()) && (! imageWriteData)) {
			shouldcapture = true;
		}
	}
	SDL_UnlockMutex (captureMutex);

	if (shouldwriteimage) {
		isWritingImage = true;
		TaskGroup::instance->run (TaskGroup::RunContext (CaptureWriter::writeImage, this));
	}
	if (shouldcapture) {
		isCapturingFrame = true;
		App::instance->addPostdrawTask (CaptureWriter::captureRenderFrame, this);
	}
}

void CaptureWriter::captureRenderFrame (void *itPtr) {
	CaptureWriter *it = (CaptureWriter *) itPtr;

	it->executeCaptureRenderFrame ();
	it->isCapturingFrame = false;
}
void CaptureWriter::executeCaptureRenderFrame () {
	bool shouldcapture;
	int result, w, h, pitch, datasize;
	int64_t t;
	SharedBuffer *b;

	shouldcapture = false;
	SDL_LockMutex (captureMutex);
	if ((mediaWriter && mediaWriter->isWriting) || (! imageWritePaths.empty ())) {
		shouldcapture = true;
	}
	SDL_UnlockMutex (captureMutex);
	if (! shouldcapture) {
		return;
	}
	w = App::instance->windowWidth;
	h = App::instance->windowHeight;
	pitch = w * sizeof (Uint32);
	datasize = w * h * sizeof (Uint32);
	b = new SharedBuffer ();
	b->retain ();
	if (b->expand (datasize) != OpResult::Success) {
		Log::debug ("Failed to capture render frame, out of memory");
		b->release ();
		return;
	}

	t = OsUtil::getTime ();
	result = SDL_RenderReadPixels (App::instance->render, NULL, SDL_PIXELFORMAT_RGBA32, b->data, pitch);
	if (result == 0) {
		SDL_LockMutex (captureMutex);
		if (mediaWriter && mediaWriter->isWriting) {
			mediaWriter->writeVideoFrame (b, t, pitch, w, h);
		}
		if (! imageWritePaths.empty ()) {
			if (imageWriteData) {
				imageWriteData->release ();
			}
			imageWriteData = b;
			imageWriteData->retain ();
			imageWriteWidth = w;
			imageWriteHeight = h;
			imageWritePitch = pitch;
		}
		SDL_UnlockMutex (captureMutex);
	}
	b->release ();
}

void CaptureWriter::writeImage (void *itPtr) {
	CaptureWriter *it = (CaptureWriter *) itPtr;

	it->executeWriteImage ();
	++(it->imageWriteCount);
	it->isWritingImage = false;
}
void CaptureWriter::executeWriteImage () {
	StringList filenames;
	StringList::const_iterator i1, i2;
	SharedBuffer *b;
	SDL_Surface *surface1, *surface2;
	int w, h, pitch, result;

	SDL_LockMutex (captureMutex);
	filenames.assign (imageWritePaths);
	b = imageWriteData;
	w = imageWriteWidth;
	h = imageWriteHeight;
	pitch = imageWritePitch;
	imageWriteData = NULL;
	imageWritePaths.clear ();
	SDL_UnlockMutex (captureMutex);
	if (! b) {
		return;
	}
	surface1 = SDL_CreateRGBSurfaceFrom (b->data, w, h, 32, pitch, RenderResource::instance->pixelRMask, RenderResource::instance->pixelGMask, RenderResource::instance->pixelBMask, RenderResource::instance->pixelAMask);
	if (! surface1) {
		Log::debug ("Failed to create surface for capture image, SDL_CreateRGBSurfaceFrom; err=%s", SDL_GetError ());
	}
	else {
		surface2 = SDL_CreateRGBSurfaceWithFormat (0, w, h, 24, SDL_PIXELFORMAT_RGB888);
		if (! surface2) {
			Log::debug ("Failed to convert surface for capture image, SDL_CreateRGBSurfaceWithFormat; err=%s", SDL_GetError ());
		}
		else {
			SDL_BlitSurface (surface1, NULL, surface2, NULL);
			i1 = filenames.cbegin ();
			i2 = filenames.cend ();
			while (i1 != i2) {
				result = IMG_SavePNG (surface2, i1->c_str ());
				if (result != 0) {
					Log::debug ("Failed to save file for capture image, IMG_SavePNG; filename=\"%s\" result=%i err=%s", i1->c_str (), result, SDL_GetError ());
				}
				++i1;
			}
			SDL_FreeSurface (surface2);
		}
		SDL_FreeSurface (surface1);
	}
	b->release ();
}
