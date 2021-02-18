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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plLayerAVI.h"

#if HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#   include <vfw.h>
#endif // HS_BUILD_FOR_WIN32

#include "plGImage/plMipmap.h"

#if HS_BUILD_FOR_WIN32
class plAVIFileInfo
{
public:
    plAVIFileInfo() : fAVIStream(), fGetFrame(), fAVIStreamInfo() { }
    IAVIStream*                 fAVIStream;
    AVISTREAMINFO               fAVIStreamInfo;
    PGETFRAME                   fGetFrame;      // Where in the stream to get the next frame
};
#else // HS_BUILD_FOR_WIN32
struct plAVIFileInfo
{
};
#endif // HS_BUILD_FOR_WIN32

#if HS_BUILD_FOR_WIN32
static bool                         ICopySourceToTexture24(BITMAPINFO* bmi, plMipmap* t);
static bool                         ICopySourceToTexture16(BITMAPINFO* bmi, plMipmap* t);
#endif


plLayerAVI::plLayerAVI()
{
    fAVIInfo = new plAVIFileInfo;
}

plLayerAVI::~plLayerAVI()
{
    ICloseMovie();

    delete fAVIInfo;
}


bool plLayerAVI::IInit()
{
#if HS_BUILD_FOR_WIN32
    int ret = AVIStreamOpenFromFileW( &fAVIInfo->fAVIStream,
                                fMovieName.WideString().data(),
                                streamtypeVIDEO,
                                0,
                                OF_READ,
                                nullptr);

    if (ret)
        return ISetFault("Error opening AVI");

    if (fAVIInfo->fGetFrame = AVIStreamGetFrameOpen(fAVIInfo->fAVIStream, nullptr); !fAVIInfo->fGetFrame)
        return ISetFault("Error positioning AVI");

    if (FAILED(AVIStreamInfo(fAVIInfo->fAVIStream, &fAVIInfo->fAVIStreamInfo, sizeof(AVISTREAMINFO))))
        return ISetFault("Error getting AVI info");

    BITMAPINFO* bmi;
    if( !(bmi = (BITMAPINFO*)AVIStreamGetFrame(fAVIInfo->fGetFrame, fCurrentFrame >= 0 ? fCurrentFrame : 0)) )
        return ISetFault("Can't get first frame");
    ISetSize(bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight);

    int32_t endFrame = fAVIInfo->fAVIStreamInfo.dwLength-1;
    float length = float(endFrame) * float(fAVIInfo->fAVIStreamInfo.dwScale) 
                        / float(fAVIInfo->fAVIStreamInfo.dwRate);
    ISetLength(length);
#endif

    return false;
}

int32_t plLayerAVI::ISecsToFrame(float secs)
{
#if HS_BUILD_FOR_WIN32
    float timeScale = float(fAVIInfo->fAVIStreamInfo.dwRate) / float(fAVIInfo->fAVIStreamInfo.dwScale);
#else
    float timeScale = 1.0f;
#endif
    return int32_t(secs * timeScale + 0.5f);
}

bool plLayerAVI::IGetCurrentFrame()
{
#if HS_BUILD_FOR_WIN32
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
#endif
    return true;
}

#if HS_BUILD_FOR_WIN32
static bool ICopySourceToTexture16(BITMAPINFO* bmi, plMipmap* b)
{
    hsAssert(b != nullptr, "nil mipmap passed to ICopySourceToTexture16()");

    uint16_t* pSrc = (uint16_t*)( bmi->bmiHeader.biSize + (BYTE*)bmi );

    uint32_t* pix = (uint32_t*)b->GetImage();
    pix += b->GetWidth() * b->GetHeight();

    uint32_t width = bmi->bmiHeader.biWidth;
    uint32_t height = bmi->bmiHeader.biHeight;

    uint32_t useHeight = std::min(height, b->GetHeight());
    uint32_t useWidth = std::min(width, b->GetWidth());

    for (uint32_t i = 0; i < useHeight; i++)
    {
        uint16_t* src = pSrc;
        pSrc += width;

        pix -= b->GetWidth();
        uint32_t* tPix = pix;
        for (uint32_t j = 0; j < useWidth; j++)
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

static bool ICopySourceToTexture24(BITMAPINFO* bmi, plMipmap* b)
{
    hsAssert(b != nullptr, "nil mipmap passed to ICopySourceToTexture24()");

    unsigned char* pSrc = (unsigned char*)( bmi->bmiHeader.biSize + (BYTE*)bmi );

    hsRGBAColor32* pix = (hsRGBAColor32*)b->GetImage();
    pix += b->GetWidth() * b->GetHeight();

    uint32_t width = bmi->bmiHeader.biWidth;
    uint32_t height = bmi->bmiHeader.biHeight;

    uint32_t useHeight = std::min(height, b->GetHeight());
    uint32_t useWidth = std::min(width, b->GetWidth());

    for (uint32_t i = 0; i < useHeight; i++)
    {
        unsigned char* src = pSrc;
        pSrc += width * 3;

        pix -= b->GetWidth();
        hsRGBAColor32* tPix = pix;
        for (uint32_t j = 0; j < useWidth; j++)
        {
            tPix->b = *src++;
            tPix->g = *src++;
            tPix->r = *src++;
            ++tPix;
        }
    }
    
    return false;
}
#endif

bool plLayerAVI::ICloseMovie()
{
#if HS_BUILD_FOR_WIN32
    if( fAVIInfo->fGetFrame )
        AVIStreamGetFrameClose(fAVIInfo->fGetFrame);
    fAVIInfo->fGetFrame = nullptr;

    if( fAVIInfo->fAVIStream )
        AVIStreamRelease(fAVIInfo->fAVIStream);

    fAVIInfo->fAVIStream = nullptr;
#endif

    return false;
}

bool plLayerAVI::IRelease()
{
    ICloseMovie();

    return false;
}

