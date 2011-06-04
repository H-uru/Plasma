/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plAVIWriter.h"

#include "hsTypes.h"

#include "hsWindows.h"
#include <vfw.h>

#include "hsTimer.h"
#include "plMipmap.h"
#include "../plMessage/plRenderMsg.h"
#include "plPipeline.h"
#include "../pnDispatch/plDispatch.h"
#include "../pnKeyedObject/plFixedKey.h"

bool plAVIWriter::fInitialized = false;

class plAVIWriterImp : public plAVIWriter
{
protected:
	PAVIFILE fFileHandle;
	PAVISTREAM fStreamHandle;
	PAVISTREAM fCompressedHandle;
	BITMAPINFOHEADER fBitmapInfo;

	hsBool fOldRealTime;
	hsScalar fOldFrameTimeInc;

	double fStartTime;

	void IFillStreamInfo(AVISTREAMINFO* inf, plPipeline* pipeline);
	void IFillBitmapInfo(BITMAPINFOHEADER* inf, plPipeline* pipeline);

	bool ICaptureFrame(plPipeline* pipeline);

public:
	plAVIWriterImp();
	virtual ~plAVIWriterImp();

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Shutdown();

	virtual bool Open(const char* fileName, plPipeline* pipeline);
	virtual void Close();
};

plAVIWriter::~plAVIWriter()
{
}

plAVIWriter& plAVIWriter::Instance()
{
	static plAVIWriterImp theInstance;

	if (!fInitialized)
	{
		theInstance.RegisterAs(kAVIWriter_KEY);
		fInitialized = true;
	}

	return theInstance;
}

////////////////////////////////////////////////////////////////////////////////

plAVIWriterImp::plAVIWriterImp() :
	fStartTime(0),
	fOldRealTime(false),
	fStreamHandle(nil),
	fCompressedHandle(nil),
	fFileHandle(nil)
{
	AVIFileInit();
}

plAVIWriterImp::~plAVIWriterImp()
{
}

void plAVIWriterImp::Shutdown()
{
	Close();
	UnRegisterAs(kAVIWriter_KEY);
	SetKey(nil);
}



#include "plProfile.h"
plProfile_CreateTimer("AviCapture", "RenderSetup", AviCapture);

hsBool plAVIWriterImp::MsgReceive(plMessage* msg)
{

	plRenderMsg* renderMsg = plRenderMsg::ConvertNoRef(msg);
	if (renderMsg)
	{
		plProfile_BeginTiming(AviCapture);

		ICaptureFrame(renderMsg->Pipeline());
		plProfile_EndTiming(AviCapture);

	}

	return hsKeyedObject::MsgReceive(msg);
}

static const int kFramesPerSec = 30;

bool plAVIWriterImp::Open(const char* fileName, plPipeline* pipeline)
{
	// Already writing, fail
	if (fStreamHandle)
		return false;

	fStartTime = hsTimer::GetSysSeconds();

	// If we're running in real time, set to frame time
	fOldRealTime = hsTimer::IsRealTime();
	if (fOldRealTime)
	{
		hsTimer::SetRealTime(false);
		hsTimer::SetFrameTimeInc(1.f / kFramesPerSec);
	}

	// Open AVI file
	HRESULT err;
	err = AVIFileOpen(	&fFileHandle,			// returned file pointer
						fileName,				// file name
						OF_WRITE | OF_CREATE,	// mode to open file with
						NULL);					// use handler determined
	hsAssert(err == AVIERR_OK, "Error creating AVI file in plAVIWriter::Open");
	if (err != AVIERR_OK)
	{
		Close();
		return false;
	}

	AVISTREAMINFO streamInfo;
	IFillStreamInfo(&streamInfo, pipeline);

	// Create a video stream in the file
	err = AVIFileCreateStream(	fFileHandle,		// file pointer
								&fStreamHandle,		// returned stream pointer
								&streamInfo );		// stream header
	hsAssert(err == AVIERR_OK, "Error creating video stream in plAVIWriter::Open");
	if (err != AVIERR_OK)
	{
		Close();
		return false;
	}

	do
	{
		AVICOMPRESSOPTIONS opts;
		AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};
		memset(&opts, 0, sizeof(opts));

		BOOL bErr = AVISaveOptions(NULL, ICMF_CHOOSE_DATARATE, 1, &fStreamHandle, (LPAVICOMPRESSOPTIONS FAR*)&aopts);
		hsAssert(bErr, "Error saving stream options in plAVIWriter::Open");
		if (!bErr)
		{
			Close();
			return false;
		}

		err = AVIMakeCompressedStream(&fCompressedHandle, fStreamHandle, &opts, NULL);
		hsAssert(err == AVIERR_OK, "Error creating compressed stream in plAVIWriter::Open");
		if (err != AVIERR_OK)
		{
			Close();
			return false;
		}

		IFillBitmapInfo(&fBitmapInfo, pipeline);
		err = AVIStreamSetFormat(	fCompressedHandle, 0, 
									&fBitmapInfo,	// stream format
									fBitmapInfo.biSize);
	} while (err != AVIERR_OK &&
			hsMessageBox("Codec unavailable, try again?", "AVI Writer", hsMessageBoxYesNo) == hsMBoxYes);

	if (err != AVIERR_OK)
	{
		Close();
		return false;
	}

	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());

	return true;
}

void plAVIWriterImp::Close()
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());

	hsTimer::SetRealTime(fOldRealTime);

	if (fStreamHandle)
	{
		AVIStreamClose(fStreamHandle);
		fStreamHandle = nil;
	}

	if (fCompressedHandle)
	{
		AVIStreamClose(fCompressedHandle);
		fCompressedHandle = nil;
	}

	if (fFileHandle)
	{
		AVIFileClose(fFileHandle);
		fFileHandle = nil;
	}

	AVIFileExit();
}

void plAVIWriterImp::IFillStreamInfo(AVISTREAMINFO* inf, plPipeline* pipeline)
{
	memset(inf, 0, sizeof(AVISTREAMINFO));
    inf->fccType = streamtypeVIDEO;
	inf->fccHandler = 0;
	inf->dwScale = 1;
    inf->dwRate = kFramesPerSec;

	SetRect(&inf->rcFrame,
			0,0,
			pipeline->Width(),
			pipeline->Height());
}

void plAVIWriterImp::IFillBitmapInfo(BITMAPINFOHEADER* inf, plPipeline* pipeline)
{
	memset(inf,0,sizeof(BITMAPINFOHEADER));
	inf->biSize = sizeof(BITMAPINFOHEADER);
	inf->biPlanes = 1;
	inf->biBitCount = 32;
	inf->biCompression = BI_RGB;
	inf->biSizeImage = 0;
	inf->biXPelsPerMeter = 0;
	inf->biYPelsPerMeter = 0;
	inf->biClrUsed = 0;
	inf->biClrImportant = 0;
	inf->biWidth = pipeline->Width();
	inf->biHeight = pipeline->Height();
}

bool plAVIWriterImp::ICaptureFrame(plPipeline* pipeline)
{
	plMipmap frame;
	pipeline->CaptureScreen(&frame, true);

	double time = hsTimer::GetSysSeconds() - fStartTime;
	time *= kFramesPerSec;

	HRESULT err;
	err = AVIStreamWrite(	fCompressedHandle,
							int(time),
							1,
							(LPBYTE)frame.GetAddr32(0,0),
							frame.GetTotalSize(),
							AVIIF_KEYFRAME,
							NULL,
							NULL);

	return (err == AVIERR_OK);
}
