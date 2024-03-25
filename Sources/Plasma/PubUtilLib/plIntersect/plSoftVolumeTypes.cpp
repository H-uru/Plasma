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
#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plVolumeIsect.h"
#include "plSoftVolumeTypes.h"

#include "pnMessage/plRefMsg.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeSimple::plSoftVolumeSimple()
:   fVolume(),
    fSoftDist()
{
}

plSoftVolumeSimple::~plSoftVolumeSimple()
{
    delete fVolume;
}

float plSoftVolumeSimple::IGetStrength(const hsPoint3& pos) const
{
    if( !fVolume || GetProperty(kDisable) )
        return 0;

    float dist = fVolume->Test(pos);

    if( dist <= 0 )
        return 1.f;

    if( dist >= fSoftDist )
        return 0;

    dist /= fSoftDist;

    return 1.f - dist;
}

void plSoftVolumeSimple::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    if( fVolume )
        fVolume->SetTransform(l2w, w2l);
}

void plSoftVolumeSimple::Read(hsStream* s, hsResMgr* mgr)
{
    plSoftVolume::Read(s, mgr);

    fSoftDist = s->ReadLEFloat();

    fVolume = plVolumeIsect::ConvertNoRef(mgr->ReadCreatable(s));
}

void plSoftVolumeSimple::Write(hsStream* s, hsResMgr* mgr)
{
    plSoftVolume::Write(s, mgr);

    s->WriteLEFloat(fSoftDist);

    mgr->WriteCreatable(s, fVolume);
}

void plSoftVolumeSimple::SetVolume(plVolumeIsect* v)
{
    delete fVolume;
    fVolume = v;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeComplex::plSoftVolumeComplex()
{
}

plSoftVolumeComplex::~plSoftVolumeComplex()
{
}

void plSoftVolumeComplex::Read(hsStream* s, hsResMgr* mgr)
{
    plSoftVolume::Read(s, mgr);

    uint32_t n = s->ReadLE32();
    for (uint32_t i = 0; i < n; i++)
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kSubVolume), plRefFlags::kActiveRef);
}

void plSoftVolumeComplex::Write(hsStream* s, hsResMgr* mgr)
{
    plSoftVolume::Write(s, mgr);

    s->WriteLE32((uint32_t)fSubVolumes.size());
    for (plSoftVolume* subVolume : fSubVolumes)
        mgr->WriteKey(s, subVolume);
}

bool plSoftVolumeComplex::MsgReceive(plMessage* msg)
{
    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
        {
            plSoftVolume* sub = plSoftVolume::ConvertNoRef(refMsg->GetRef());
            hsAssert(std::find(fSubVolumes.cbegin(), fSubVolumes.cend(), sub) == fSubVolumes.cend(),
                     "Adding subvolume I already have");
            fSubVolumes.emplace_back(sub);
        }
        else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            plSoftVolume* sub = (plSoftVolume*)refMsg->GetRef();
            auto idx = std::find(fSubVolumes.cbegin(), fSubVolumes.cend(), sub);
            if (idx != fSubVolumes.cend())
                fSubVolumes.erase(idx);
        }
        return true;
    }
    return plSoftVolume::MsgReceive(msg);
}

void plSoftVolumeComplex::UpdateListenerPosition(const hsPoint3& pos)
{
    plSoftVolume::UpdateListenerPosition(pos);
    for (plSoftVolume* subVolume : fSubVolumes)
        subVolume->UpdateListenerPosition(pos);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeUnion::plSoftVolumeUnion()
{
}

plSoftVolumeUnion::~plSoftVolumeUnion()
{
}

float plSoftVolumeUnion::IGetStrength(const hsPoint3& pos) const
{
    float retVal = 0;
    for (plSoftVolume* subVolume : fSubVolumes)
    {
        float subRet = subVolume->GetStrength(pos);
        if( subRet >= 1.f )
            return 1.f;
        if( subRet > retVal )
            retVal = subRet;
    }
    return retVal;
}

float plSoftVolumeUnion::IUpdateListenerStrength() const
{
    float retVal = 0;
    for (plSoftVolume* subVolume : fSubVolumes)
    {
        float subRet = subVolume->GetListenerStrength();
        if( subRet >= 1.f )
        {
            retVal = 1.f;
            break;
        }
        if( subRet > retVal )
            retVal = subRet;
    }
    return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeIntersect::plSoftVolumeIntersect()
{
}

plSoftVolumeIntersect::~plSoftVolumeIntersect()
{
}

float plSoftVolumeIntersect::IGetStrength(const hsPoint3& pos) const
{
    float retVal = 1.f;
    for (plSoftVolume* subVolume : fSubVolumes)
    {
        float subRet = subVolume->GetStrength(pos);
        if( subRet <= 0 )
            return 0;
        if( subRet < retVal )
            retVal = subRet;
    }
    return retVal;
}

float plSoftVolumeIntersect::IUpdateListenerStrength() const
{
    float retVal = 1.f;
    for (plSoftVolume* subVolume : fSubVolumes)
    {
        float subRet = subVolume->GetListenerStrength();
        if( subRet <= 0 )
        {
            retVal = 0.f;
            break;
        }
        if( subRet < retVal )
            retVal = subRet;
    }
    return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeInvert::plSoftVolumeInvert()
{
}

plSoftVolumeInvert::~plSoftVolumeInvert()
{
}

float plSoftVolumeInvert::IGetStrength(const hsPoint3& pos) const
{
    hsAssert(fSubVolumes.size() <= 1, "Too many subvolumes on inverter");
    if (!fSubVolumes.empty())
        return 1.f - fSubVolumes[0]->GetStrength(pos);

    return 1.f;
}

float plSoftVolumeInvert::IUpdateListenerStrength() const
{
    hsAssert(fSubVolumes.size() <= 1, "Too many subvolumes on inverter");
    float retVal = 1.f;
    if (!fSubVolumes.empty())
        retVal = (1.f - fSubVolumes[0]->GetListenerStrength());

    return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
