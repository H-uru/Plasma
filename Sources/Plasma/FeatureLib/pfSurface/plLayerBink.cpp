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

// I'm not really sure what of this is platform dependent. Shouldn't get compiled
// into servers stuff (Unix/Linux), but should work on the Mac. If you're trying to
// compile on one of those platforms and hit problems, let me know - mf
#ifdef BINK_SDK_AVAILABLE
#include "bink.h"
#endif

#include "HeadSpin.h"



#include "plLayerBink.h"
#include "plGImage/plMipmap.h"

#ifdef BINK_SDK_AVAILABLE
#if HS_BUILD_FOR_WIN32
typedef HBINK (CALLBACK *BINKOPEN)(char *,int);
typedef int32_t (CALLBACK *BINKCOPYTOBUFFER)(HBINK, void*, s32,u32,u32,u32, int);
typedef char * (CALLBACK *BINKGETERROR)(void);
typedef void (CALLBACK *BINKONLY) (HBINK);
typedef s32 (CALLBACK *BINKDOFRAME) (HBINK);
typedef void (CALLBACK *BINKGOTO)(HBINK, uint32_t, int);
#endif // HS_BUILD_FOR_WIN32

#if HS_BUILD_FOR_WIN32
#define BinkClose(bink) (*fBinkClose)(bink)
#define BinkGetError (*fBinkGetError)
#define BinkOpen(name,flags) (*fBinkOpen)(name,flags)
#define BinkCopyToBuffer(bink,dst, pt, ht, dx, dy,flags) (*fBinkCopyToBuffer)(bink,dst,pt,ht,dx,dy,flags)
#define BinkNextFrame(bink) (*fBinkNextFrame)(bink)
#define BinkDoFrame(bink) (*fBinkDoFrame)(bink)
#define BinkGoto(bink,frame,flags) (*fBinkGoto)(bink,frame,flags)

#define MAX_TIME_OUT    5.f // Five Seconds

static HINSTANCE            fBinkLib = nil;
static BINKONLY             fBinkClose = nil;
static BINKGETERROR         fBinkGetError = nil;
static BINKOPEN             fBinkOpen = nil;
static BINKCOPYTOBUFFER     fBinkCopyToBuffer = nil;
static BINKONLY             fBinkNextFrame = nil;
static BINKDOFRAME          fBinkDoFrame = nil;
static BINKGOTO             fBinkGoto = nil;    
#endif // HS_BUILD_FOR_WIN32
static int                  fBinkRef = 0;
#endif /* BINK_SDK_AVAILABLE */

#include "HeadSpin.h"
#include "plLayerBink.h"

#ifdef BINK_SDK_AVAILABLE
#define GetBink()   ((HBINK)(fHBink))
#else
#define GetBink()   fHBink
#endif
#define SetBink(f)  (fHBink = f)

plLayerBink::plLayerBink()
{
#ifdef BINK_SDK_AVAILABLE
#if HS_BUILD_FOR_WIN32
    if (fBinkRef == 0)
    {
        fBinkLib = LoadLibrary("binkw32.dll");
        if(!fBinkLib)
            hsMessageBox("Can't find bink Library. You need binkw32.dll","error",MB_OK);
        fBinkClose          = BINKONLY(GetProcAddress(fBinkLib,"_BinkClose@4"));
        fBinkGetError       = BINKGETERROR(GetProcAddress(fBinkLib,"_BinkGetError@0"));
        fBinkOpen           = BINKOPEN(GetProcAddress(fBinkLib,"_BinkOpen@8"));
        fBinkCopyToBuffer   = BINKCOPYTOBUFFER(GetProcAddress(fBinkLib,"_BinkCopyToBuffer@28"));
        fBinkNextFrame      = BINKONLY(GetProcAddress(fBinkLib,"_BinkNextFrame@4"));
        fBinkDoFrame        = BINKDOFRAME(GetProcAddress(fBinkLib,"_BinkDoFrame@4"));
        fBinkGoto           = BINKGOTO(GetProcAddress(fBinkLib,"_BinkGoto@12"));
        hsAssert( fBinkClose != nil && fBinkGetError != nil && fBinkOpen != nil &&
                    fBinkCopyToBuffer != nil && fBinkNextFrame != nil && fBinkDoFrame != nil &&
                    fBinkGoto != nil, "Bink procs couldn't load!!! AAK!" );
    }
    fBinkRef++;
#endif // HS_BUILD_FOR_WIN32
#endif /* BINK_SDK_AVAILABLE */
    SetBink(nil);
    fFPS = 0.f;
}

plLayerBink::~plLayerBink()
{
#ifdef BINK_SDK_AVAILABLE
#if HS_BUILD_FOR_WIN32
    if( !--fBinkRef )
    {
        FreeLibrary( fBinkLib );
    }
#endif // HS_BUILD_FOR_WIN32
#endif
}

hsBool plLayerBink::IInit()
{
#ifdef BINK_SDK_AVAILABLE
    SetBink(BinkOpen(fMovieName, BINKALPHA));
    if (!GetBink())
#endif
        return ISetFault("Failed to open Bink");

#ifdef BINK_SDK_AVAILABLE
    float nSecs = (GetBink()->Frames-1)*(float)(GetBink()->FrameRateDiv)/(float)(GetBink()->FrameRate);
    ISetLength(nSecs);
    ISetSize(GetBink()->Width, GetBink()->Height);
    fFPS = float(GetBink()->FrameRate) / float(GetBink()->FrameRateDiv) + 0.5f;

    return true;
#endif
}

int32_t plLayerBink::ISecsToFrame(float secs)
{
    // Calculate and set the current frame
    int32_t frame = (int32_t)(secs * fFPS);
    return frame;
}

hsBool plLayerBink::IGetCurrentFrame()
{
    if (!GetBink())
    {
        IInit();

        if (!GetBink())
            return ISetFault("previously failed but still calling Bink::IGetCurrentFrame()");
    }

#ifdef BINK_SDK_AVAILABLE
    ICheckBitmap();

    int32_t frame = fCurrentFrame + 1; // Bink Counts frames from frame 1
    
    if(frame > GetBink()->Frames)
        frame = GetBink()->Frames;

    if(frame < 0)
        frame = 0;

    BinkGoto(GetBink(), frame, NULL);
    BinkDoFrame(GetBink());
    BinkNextFrame(GetBink());

    plMipmap* b = plMipmap::ConvertNoRef( GetTexture() );
    if( !b->GetImage() )
    {
        b->SetImagePtr( HSMemory::New(b->GetHeight() * b->GetRowBytes()) );
    }

    void* dest = b->GetImage();

    BinkCopyToBuffer(GetBink(),
            dest, 
            GetBink()->Width * 4,
            GetBink()->Height,
            0, 0,
            BINKSURFACE32A | BINKCOPYALL
    );
#endif /* BINK_SDK_AVAILABLE */

    return false;
}

hsBool plLayerBink::IRelease()
{
#ifdef BINK_SDK_AVAILABLE
    if(GetBink())
    {
        BinkClose(GetBink());
        SetBink(nil);
        return false;
    }
#endif
    return true;
}
