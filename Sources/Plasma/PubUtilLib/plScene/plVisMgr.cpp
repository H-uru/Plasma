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
#include "plVisMgr.h"

#include "plVisRegion.h"

hsBitVector plVisMgr::fIdxSet;
hsBitVector plVisMgr::fIdxNot;

plVisMgr::plVisMgr()
:   fMaxSet(kCharacter),
    fMaxNot(-1)
{
    ResetNormal();
}

plVisMgr::~plVisMgr()
{
}

hsBool plVisMgr::MsgReceive(plMessage* msg)
{

    return hsKeyedObject::MsgReceive(msg);
}

void plVisMgr::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);

}

void plVisMgr::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);
}

void plVisMgr::Register(plVisRegion* reg, hsBool bnot)
{
    // This should happen pretty infrequently, or
    // I wouldn't be doing it so cloth-headed-ly.
    hsTArray<plVisRegion*>& regions = bnot ? fNotRegions : fRegions;
    hsBitVector& indices = bnot ? fIdxNot : fIdxSet;
    int& maxIdx = bnot ? fMaxNot : fMaxSet;
    int i;
    for( i = kNumReserved; ; i++ )
    {
        if( !indices.IsBitSet(i) )
        {
            if( i > maxIdx )
                maxIdx = i;

            indices.SetBit(i);
            reg->SetIndex(i);
            regions.Append(reg);
            return;
        }
    }
    hsAssert(false, "Infinite bitvector has all bits set?");
}

void plVisMgr::UnRegister(plVisRegion* reg, hsBool bnot)
{
    // Mark our index for recycling
    hsBitVector& indices= bnot ? fIdxNot : fIdxSet;
    indices.ClearBit(reg->GetIndex());

    // Nuke the region from our list.
    hsTArray<plVisRegion*>& regions = bnot ? fNotRegions : fRegions;
    int idx = regions.Find(reg);
    if( regions.kMissingIndex != idx )
        regions.Remove(idx);
}

void plVisMgr::Eval(const hsPoint3& pos)
{
    fVisSet = fOnBitSet;

    int i;
    for( i = 0; i < fRegions.GetCount(); i++ )
    {
        hsAssert(fRegions[i], "Nil region in list");
        if( !fOffBitSet.IsBitSet(fRegions[i]->GetIndex()) )
        {
            if( fRegions[i]->Eval(pos) )
            {
                fVisSet.SetBit(fRegions[i]->GetIndex());
                if( fRegions[i]->DisableNormal() )
                {
                    fVisSet.ClearBit(kNormal);
                    fVisSet.SetBit(kCharacter);
                }
            }
        }
    }

    fVisNot = fOnBitNot;

    for( i = 0; i < fNotRegions.GetCount(); i++ )
    {
        hsAssert(fNotRegions[i], "Nil region in list");
        if( !fOffBitNot.IsBitSet(fNotRegions[i]->GetIndex()) )
        {
            if( fNotRegions[i]->Eval(pos) )
            {
                fVisNot.SetBit(fNotRegions[i]->GetIndex());
            }
        }
    }

    ResetNormal();
}

void plVisMgr::ResetNormal()
{
    fOnBitSet.Clear();
    fOnBitSet.SetBit(kNormal);

    fOffBitSet.Clear();
    fOnBitNot.Clear();
    fOffBitNot.Clear();
}

void plVisMgr::DisableNormal()
{
    fOnBitSet.Clear();
    if( fMaxSet > 0 )
        fOffBitSet.Set(fMaxSet);

    fOnBitNot.Clear();
    if( fMaxNot > 0 )
        fOffBitNot.Set(fMaxNot);
}

void plVisMgr::EnableVisSet(int idx, hsBool isNot)
{
    hsBitVector& offs = isNot ? fOffBitNot : fOffBitSet;

    offs.ClearBit(idx);
}

void plVisMgr::EnableVisSets(const hsBitVector& enabled, hsBool isNot)
{
    hsBitVector& offs = isNot ? fOffBitNot : fOffBitSet;
    offs -= enabled;
}

void plVisMgr::ForceVisSet(int idx, hsBool isNot)
{
    EnableVisSet(idx, isNot);
    hsBitVector& ons = isNot ? fOnBitNot : fOnBitSet;
    ons.SetBit(idx);
}

void plVisMgr::ForceVisSets(const hsBitVector& enabled, hsBool isNot)
{
    EnableVisSets(enabled, isNot);
    hsBitVector& ons = isNot ? fOnBitNot : fOnBitSet;
    ons |= enabled;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

plVisMgr* plGlobalVisMgr::fInstance = nil;

void plGlobalVisMgr::Init()
{
    fInstance = new plVisMgr;
    fInstance->RegisterAs(kGlobalVisMgr_KEY);
}

void plGlobalVisMgr::DeInit()
{
    if (fInstance)
    {
        fInstance->UnRegisterAs(kGlobalVisMgr_KEY);
        fInstance = nil;
    }
}
