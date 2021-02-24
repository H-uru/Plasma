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

#include "HeadSpin.h"
#include "plLODMipmap.h"

#include "hsResMgr.h"
#include "hsGDeviceRef.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"
#include "pnMessage/plRefMsg.h"


plLODMipmap::plLODMipmap() 
:   fBase(),
    fLOD()
{
    int i;
    for( i = 0; i < kNumLODs; i++ )
        fDeviceRefs[i] = nullptr;
}

plLODMipmap::plLODMipmap(plMipmap* mip)
:   fBase(),
    fLOD()
{
    int i;
    for( i = 0; i < kNumLODs; i++ )
        fDeviceRefs[i] = nullptr;

    hsgResMgr::ResMgr()->NewKey(mip->GetKey()->GetName(), this, mip->GetKey()->GetUoid().GetLocation());

    // Need some kind of reffing assignment for the mipmap here
    fBase = mip;
    fLevelSizes = new uint32_t[fBase->GetNumLevels()];

    ISetup();

    hsgResMgr::ResMgr()->AddViaNotify(mip->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefBase), plRefFlags::kActiveRef);

}

plLODMipmap::~plLODMipmap()
{
    // And matching unreffing of the mipmap here.
    fImage = nullptr;

    int i;
    for( i = 0; i < kNumLODs; i++ )
    {
        hsRefCnt_SafeUnRef(fDeviceRefs[i]);
        fDeviceRefs[i] = nullptr;
    }
}

hsGDeviceRef* plLODMipmap::GetDeviceRef() const 
{ 
    return fDeviceRefs[GetLOD()]; 
}

void plLODMipmap::SetDeviceRef( hsGDeviceRef *const devRef )
{
    hsRefCnt_SafeAssign(fDeviceRefs[GetLOD()], devRef);
}


void plLODMipmap::SetLOD(int lod)
{
    hsAssert(fBase, "UnInitialized");

    const int kMaxLOD = 5;
    if( lod > kMaxLOD )
        lod = kMaxLOD;
    if( lod >= fBase->GetNumLevels() )
        lod = fBase->GetNumLevels() - 1;

    if( fLOD != lod )
    {
        fLOD = lod;
        ISetup();
    }
}


void plLODMipmap::ISetup()
{
    hsAssert(fBase, "UnInitialized");
    hsAssert(fBase->GetNumLevels() > GetLOD(), "Skipping all mip levels");

    fBase->SetCurrLevel(GetLOD());

    // plBitmap
    fPixelSize = fBase->GetPixelSize(); 
//  fSpace = fBase->fSpace;     
    fFlags = fBase->GetFlags();     

    fCompressionType = fBase->fCompressionType;
    if( !fBase->IsCompressed() )
    {
        fUncompressedInfo = fBase->fUncompressedInfo;
    }
    else
    {
        fDirectXInfo = fBase->fDirectXInfo;
    }

    // plMipmap
    fImage = fBase->GetCurrLevelPtr();
    fWidth = fBase->GetCurrWidth();
    fHeight = fBase->GetCurrHeight();

    fRowBytes = fBase->GetRowBytes();

    if( !fLevelSizes )
        fLevelSizes = new uint32_t[fBase->GetNumLevels()];
        
    fNumLevels = fBase->GetNumLevels() - GetLOD();
    fNumLevels = 1;

    fTotalSize = 0;
    int i;
    for( i = 0; i < fNumLevels; i++ )
    {
        fLevelSizes[i] = fBase->GetLevelSize(i + GetLOD());
        fTotalSize += fBase->GetLevelSize(i + GetLOD());
    }

    ISetupCurrLevel();

    IMarkDirty();
}

void plLODMipmap::ISetupCurrLevel()
{
    fCurrLevelPtr = fBase->GetCurrLevelPtr();
    fCurrLevel = (uint8_t)(fBase->GetCurrLevel());
    fCurrLevelWidth = fBase->GetCurrWidth();
    fCurrLevelHeight = fBase->GetCurrHeight();
    fCurrLevelRowBytes = fBase->GetRowBytes();
}

void plLODMipmap::INilify()
{
    fBase = nullptr;

    fPixelSize = 0;
    fSpace = kNoSpace;
    fFlags = 0;

    fCompressionType = kUncompressed;
    fUncompressedInfo.fType = UncompressedInfo::kRGB8888;

    // plMipmap
    fImage = nullptr;
    fWidth = 0;
    fHeight = 0;

    fRowBytes = 0;

    fNumLevels = 0;
    fTotalSize = 0;

    fCurrLevelPtr = nullptr;
    fCurrLevel = 0;
    fCurrLevelWidth = 0;
    fCurrLevelHeight = 0;
    fCurrLevelRowBytes = 0;

    delete [] fLevelSizes;
    fLevelSizes = nullptr;

    int i;
    for( i = 0; i < kNumLODs; i++ )
    {
        hsRefCnt_SafeUnRef(fDeviceRefs[i]);
        fDeviceRefs[i] = nullptr;
    }
}

void plLODMipmap::IMarkDirty()
{
    int i;
    for( i = 0; i < kNumLODs; i++ )
    {
        if( fDeviceRefs[i] )
            fDeviceRefs[i]->SetDirty(true);
    }
}

void plLODMipmap::SetCurrLevel(uint8_t level)
{
    fBase->SetCurrLevel(level + GetLOD());

    ISetupCurrLevel();
}

void plLODMipmap::Reset()
{
    fBase->Reset();
    ISetup();
}

void plLODMipmap::ScaleNicely(uint32_t *destPtr, uint16_t destWidth, uint16_t destHeight,
                            uint16_t destStride, plMipmap::ScaleFilter filter) const
{
    fBase->ScaleNicely(destPtr, destWidth, destHeight, destStride, filter);
}

bool plLODMipmap::ResizeNicely(uint16_t newWidth, uint16_t newHeight, plMipmap::ScaleFilter filter)
{
    bool retVal = fBase->ResizeNicely(newWidth, newHeight, filter);
    ISetup();
    return retVal;
}

void plLODMipmap::CopyFrom(const plMipmap *source)
{
    fBase->CopyFrom(source);
    ISetup();
}

void plLODMipmap::Composite(plMipmap *source, uint16_t x, uint16_t y, CompositeOptions *options) 
{ 
    fBase->Composite(source, x, y, options); 
    IMarkDirty();
}

bool plLODMipmap::MsgReceive(plMessage *msg)
{
    plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
    if( ref )
    {
        if( ref->fType == kRefBase )
        {
            INilify();
            if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fBase = plMipmap::ConvertNoRef(ref->GetRef());
                ISetup();
            }

            return true;
        }
    }

    return plMipmap::MsgReceive(msg);
}

void plLODMipmap::Read(hsStream *s, hsResMgr *mgr)
{
    INilify();
    hsKeyedObject::Read(s, mgr);
    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefBase), plRefFlags::kActiveRef); // fBase
}

void plLODMipmap::Write(hsStream *s, hsResMgr *mgr)
{
    hsKeyedObject::Write(s, mgr);
    mgr->WriteKey(s, fBase);
}
