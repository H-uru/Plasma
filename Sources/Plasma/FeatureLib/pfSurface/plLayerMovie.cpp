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

#include "plLayerMovie.h"

#include "HeadSpin.h"
#include "hsGDeviceRef.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include <string_theory/format>

#include "plMessage/plAnimCmdMsg.h"
#include "plGImage/plMipmap.h"

plLayerMovie::plLayerMovie()
:   fCurrentFrame(-1),
    fLength(0),
    fWidth(32),
    fHeight(32)
{
    fOwnedChannels |= kTexture;
    fTexture = new plBitmap*;
    *fTexture = nullptr;
}

plLayerMovie::~plLayerMovie()
{
    delete *fTexture;
}

bool plLayerMovie::ISetFault(const char* errStr)
{
#ifdef HS_DEBUGGING
    hsStatusMessage(ST::format("ERROR {}: {}", fMovieName, errStr).c_str());
#endif // HS_DEBUGGING
    fMovieName = "";
    return true;
}

bool plLayerMovie::ISetLength(float secs)
{
    fLength = secs;
    return false;
}

int plLayerMovie::GetWidth() const
{
    plMipmap    *mip = plMipmap::ConvertNoRef( GetTexture() );
    return mip ? mip->GetWidth() : 0;
}

int plLayerMovie::GetHeight() const
{
    plMipmap    *mip = plMipmap::ConvertNoRef( GetTexture() );
    return mip ? mip->GetHeight() : 0;
}

bool plLayerMovie::ISetSize(int width, int height)
{
    fWidth = width;
    fHeight = height;
    return false;
}

bool plLayerMovie::ISetupBitmap()
{
    if( !GetTexture() )
    {
        plMipmap* b = new plMipmap( fWidth, fHeight, plMipmap::kARGB32Config, 1 );
        memset(b->GetImage(), 0x10, b->GetHeight() * b->GetRowBytes() );
        b->SetFlags( b->GetFlags() | plMipmap::kDontThrowAwayImage );

        ST::string name = ST::format("{}_BMap", fMovieName);
        hsgResMgr::ResMgr()->NewKey( name, b, plLocation::kGlobalFixedLoc );

        *fTexture = (plBitmap *)b;
    }

    return false;
}

bool plLayerMovie::ICheckBitmap()
{
    if( !GetTexture() )
        ISetupBitmap();

    return false;
}

bool plLayerMovie::IMovieIsIdle()
{
    IRelease();

    return false;
}

bool plLayerMovie::ICurrentFrameDirty(double wSecs)
{
    float secs = fTimeConvert.WorldToAnimTime(wSecs);
    uint32_t frame = ISecsToFrame(secs);
    if( frame == fCurrentFrame )
        return false;
    fCurrentFrame = frame;

    return true;
}

uint32_t plLayerMovie::Eval(double wSecs, uint32_t frame, uint32_t ignore)
{
    uint32_t dirty = plLayerAnimation::Eval(wSecs, frame, ignore);

    if( !IGetFault() && !(ignore & kTexture) )
    {
        if( ICurrentFrameDirty(wSecs) )
        {           
            if( IGetCurrentFrame() )
                ISetFault("Getting current frame");

            if( GetTexture() )
            {
                hsGDeviceRef* ref = GetTexture()->GetDeviceRef();
                if( ref )
                    ref->SetDirty(true);
            }
        }
        else
        if( IsStopped() )
        {
            IMovieIsIdle();
        }
        
        dirty |= kTexture;
    }
    return dirty;
}

void plLayerMovie::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimation::Read(s, mgr);

    int len = s->ReadLE32();
    if( len )
    {
        ST::char_buffer movieName;
        movieName.allocate(len);
        s->Read(len, movieName.data());
        fMovieName = ST::string(movieName);
    }
    else
    {
        hsAssert(false, "Reading empty string for movie name");
        fMovieName = "";
    }
}

void plLayerMovie::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerAnimation::Write(s, mgr);

    s->WriteLE32((uint32_t)fMovieName.GetSize());
    s->Write(fMovieName.GetSize(), fMovieName.AsString().c_str());
}

bool plLayerMovie::MsgReceive(plMessage* msg)
{   
    return plLayerAnimation::MsgReceive(msg);
}

void plLayerMovie::DefaultMovie()
{
}
