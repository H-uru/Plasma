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

#include "plHardRegionTypes.h"
#include "hsStream.h"
#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"


////////////////////////////////////////////////////////////////////////////////////////
// Base hard and complex
////////////////////////////////////////////////////////////////////////////////////////
plHardRegionComplex::plHardRegionComplex()
{
}

plHardRegionComplex::~plHardRegionComplex()
{
}

void plHardRegionComplex::Read(hsStream* s, hsResMgr* mgr)
{
    plHardRegion::Read(s, mgr);

    uint32_t n = s->ReadLE32();
    for (uint32_t i = 0; i < n; i++)
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kSubRegion), plRefFlags::kActiveRef);
}

void plHardRegionComplex::Write(hsStream* s, hsResMgr* mgr)
{
    plHardRegion::Write(s, mgr);

    s->WriteLE32((uint32_t)fSubRegions.size());
    for (plHardRegion* subRegion : fSubRegions)
        mgr->WriteKey(s, subRegion);
}

bool plHardRegionComplex::MsgReceive(plMessage* msg)
{
    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
        {
            plHardRegion* sub = plHardRegion::ConvertNoRef(refMsg->GetRef());
            hsAssert(std::find(fSubRegions.cbegin(), fSubRegions.cend(), sub) == fSubRegions.cend(),
                     "Adding subRegion I already have");
            fSubRegions.emplace_back(sub);
        }
        else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            plHardRegion* sub = (plHardRegion*)refMsg->GetRef();
            auto idx = std::find(fSubRegions.cbegin(), fSubRegions.cend(), sub);
            if (idx != fSubRegions.cend())
                fSubRegions.erase(idx);
        }
        return true;
    }
    return plHardRegion::MsgReceive(msg);
}

////////////////////////////////////////////////////////////////////////////////////////
// bools follow
////////////////////////////////////////////////////////////////////////////////////////

// Union
////////////////////////////////////////////////////////////////////////////////////////

plHardRegionUnion::plHardRegionUnion()
{
}

plHardRegionUnion::~plHardRegionUnion()
{
}

bool plHardRegionUnion::IIsInside(const hsPoint3& pos) const
{
    for (plHardRegion* subRegion : fSubRegions)
    {
        if (subRegion->IIsInside(pos))
            return true;
    }
    return false;
}

bool plHardRegionUnion::ICameraInside() const
{
    if( fState & kDirty )
    {
        fState &= ~(kCamInside | kDirty);
        for (plHardRegion* subRegion : fSubRegions)
        {
            if (subRegion->ICameraInside())
            {
                fState |= kCamInside;
                return true;
            }
        }
    }
    return false;
}

// Intersection
////////////////////////////////////////////////////////////////////////////////////////
plHardRegionIntersect::plHardRegionIntersect()
{
}

plHardRegionIntersect::~plHardRegionIntersect()
{
}

bool plHardRegionIntersect::IIsInside(const hsPoint3& pos) const
{
    for (plHardRegion* subRegion : fSubRegions)
    {
        if (!subRegion->IIsInside(pos))
            return false;
    }
    return true;
}

bool plHardRegionIntersect::ICameraInside() const
{
    if( fState & kDirty )
    {
        fState &= ~kDirty;
        fState |= kCamInside;
        for (plHardRegion* subRegion : fSubRegions)
        {
            if (!subRegion->ICameraInside())
            {
                fState &= ~kCamInside;
                return false;
            }
        }
    }
    return true;
}


// Invert
////////////////////////////////////////////////////////////////////////////////////////

plHardRegionInvert::plHardRegionInvert()
{
}

plHardRegionInvert::~plHardRegionInvert()
{
}

bool plHardRegionInvert::IIsInside(const hsPoint3& pos) const
{
    hsAssert(fSubRegions.size() <= 1, "Too many subRegions on inverter");
    return !fSubRegions[0]->IIsInside(pos);
}

bool plHardRegionInvert::ICameraInside() const
{
    hsAssert(fSubRegions.size() <= 1, "Too many subRegions on inverter");
    return !fSubRegions[0]->ICameraInside();
}

    ///////////////////////////////////////////////////////////////////////////////
