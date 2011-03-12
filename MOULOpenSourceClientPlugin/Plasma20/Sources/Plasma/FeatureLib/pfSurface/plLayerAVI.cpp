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

#include "hsConfig.h"

#if HS_BUILD_FOR_WIN32
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#ifndef _WINDOWS_H_	// redundant include guard to minimize compile times
#define _WINDOWS_H_
#include <windows.h>
#endif // _WINDOWS_H_
#include "vfw.h"
#endif // HS_BUILD_FOR_WIN32

#include "hsTypes.h"
#include "plLayerAVI.h"
#include "../plGImage/plMipmap.h"

#if HS_BUILD_FOR_WIN32
class plAVIFileInfo
{
public:
	plAVIFileInfo() : fAVIStream(nil), fGetFrame(0) {}
	IAVIStream*					fAVIStream;
	AVISTREAMINFO				fAVIStreamInfo;
	PGETFRAME					fGetFrame;      // Where in the stream to get the next frame
};
#else // HS_BUILD_FOR_WIN32
struct plAVIFileInfo
{
};
#endif // HS_BUILD_FOR_WIN32

static hsBool						ICopySourceToTexture24(BITMAPINFO* bmi, plMipmap* t);
static hsBool						ICopySourceToTexture16(BITMAPINFO* bmi, plMipmap* t);


plLayerAVI::plLayerAVI()
{
	fAVIInfo = TRACKED_NEW plAVIFileInfo;
}

plLayerAVI::~plLayerAVI()
{
	ICloseMovie();

	delete fAVIInfo;
}


hsBool plLayerAVI::IInit()
{
#if HS_BUILD_FOR_WIN32
	int ret = AVIStreamOpenFromFile( &fAVIInfo->fAVIStream, 
								fMovieName,													
								streamtypeVIDEO, 
								0, 
								OF_READ,
								NULL);

	if (ret)
		return ISetFault("Error opening AVI");

	if( !(fAVIInfo->fGetFrame = AVIStreamGetFrameOpen(fAVIInfo->fAVIStream, NULL)) )
		return ISetFault("Error positioning AVI");

	if( AVIStreamInfo(fAVIInfo->fAVIStream, &fAVIInfo->fAVIStreamInfo, sizeof(AVISTREAMINFO)) )
		return ISetFault("Error getting AVI info");

	BITMAPINFO* bmi;
	if( !(bmi = (BITMAPINFO*)AVIStreamGetFrame(fAVIInfo->fGetFrame, fCurrentFrame >= 0 ? fCurrentFrame : 0)) )
		return ISetFault("Can't get first frame");
	ISetSize(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight);

	Int32 endFrame = fAVIInfo->fAVIStreamInfo.dwLength-1;
	hsScalar length = float(endFrame) * float(fAVIInfo->fAVIStreamInfo.dwScale) 
						/ float(fAVIInfo->fAVIStreamInfo.dwRate);
	ISetLength(length);
#endif

	return false;
}

Int32 plLayerAVI::ISecsToFrame(hsScalar secs)
{
	float timeScale = float(fAVIInfo->fAVIStreamInfo.dwRate) / float(fAVIInfo->fAVIStreamInfo.dwScale);
	return Int32(secs * timeScale + 0.5f);
}

hsBool plLayerAVI::IGetCurrentFrame()
{
	if( !fAVIInfo->fAVIStream )
		IInit();

	ICheckBitmap();
	
	BITMAPINFO* bmi;
	if( !(bmi = (BITMAPINFO*)AVIStreamGetFrame(fAVIInfo->fGetFrame, fCurrentFrame)) )
		return ISetFault("Can't get frame");

	switch(bmi->bmiHeader.biBitCount)
	{
	case 16:
		return ICopySourceToTexture16(bmi, plMipmap::ConvertNoRef(GetTexture()));
		break;
	case 24:
		return ICopySourceToTexture24(bmi, plMipmap::ConvertNoRef(GetTexture()));
		break;
	default:
		return ISetFault("Unknown AVI color depth");
	}
	return true;
}

static hsBool ICopySourceToTexture16(BITMAPINFO* bmi, plMipmap* b)
{
	hsAssert( b != nil, "nil mipmap passed to ICopySourceToTexture16()" );

	UInt16* pSrc = (UInt16*)( bmi->bmiHeader.biSize + (BYTE*)bmi );

	UInt32* pix = (UInt32*)b->GetImage();
	pix += b->GetWidth() * b->GetHeight();

	int width = bmi->bmiHeader.biWidth;
	int height = bmi->bmiHeader.biHeight;

	int useHeight = hsMinimum(height, b->GetHeight());
	int useWidth = hsMinimum(width, b->GetWidth());
	int i;
	for( i = 0; i < useHeight; i++ )
	{
		UInt16* src = pSrc;
		pSrc += width;

		pix -= b->GetWidth();
		UInt32* tPix = pix;
		int j;
		for( j = 0; j < useWidth; j++ )
		{
			*tPix = ((*src & 0x001f) << 3) // up 3 
				| ((*src & 0x03e0) << 6) // down 5 up 3 up 8
				| ((*src & 0x7c00) << 9) // down 10 up 3 up 16
				| (0xff << 24); // alpha
			src++;
			++tPix;
		}
	}
	
	return false;
}

static hsBool ICopySourceToTexture24(BITMAPINFO* bmi, plMipmap* b)
{
	hsAssert( b != nil, "nil mipmap passed to ICopySourceToTexture24()" );

	unsigned char* pSrc = (unsigned char*)( bmi->bmiHeader.biSize + (BYTE*)bmi );

	hsRGBAColor32* pix = (hsRGBAColor32*)b->GetImage();
	pix += b->GetWidth() * b->GetHeight();

	int width = bmi->bmiHeader.biWidth;
	int height = bmi->bmiHeader.biHeight;

	int useHeight = hsMinimum(height, b->GetHeight());
	int useWidth = hsMinimum(width, b->GetWidth());
	int i;

	for( i = 0; i < useHeight; i++ )
	{
		unsigned char* src = pSrc;
		pSrc += width * 3;

		pix -= b->GetWidth();
		hsRGBAColor32* tPix = pix;
		int j;
		for( j = 0; j < useWidth; j++ )
		{
			tPix->b = *src++;
			tPix->g = *src++;
			tPix->r = *src++;
			++tPix;
		}
	}
	
	return false;
}

hsBool plLayerAVI::ICloseMovie()
{
	if( fAVIInfo->fGetFrame )
		AVIStreamGetFrameClose(fAVIInfo->fGetFrame);
	fAVIInfo->fGetFrame = 0;

	if( fAVIInfo->fAVIStream )
		AVIStreamRelease(fAVIInfo->fAVIStream);

	fAVIInfo->fAVIStream = nil;

	return false;
}

hsBool plLayerAVI::IRelease()
{
	ICloseMovie();

	return false;
}

