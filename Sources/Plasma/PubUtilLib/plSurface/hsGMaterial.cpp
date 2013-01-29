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

#include "hsGMaterial.h"
#include <cmath>

#include "HeadSpin.h"
#include "hsMemory.h"
#include "hsResMgr.h"
#include "plLayerInterface.h"
#include "plLayer.h"
#include "plMessage/plMatRefMsg.h"
#include "plProfile.h"

plProfile_CreateTimer("MaterialAnims", "Animation", MaterialAnims);

plLayer defaultLayer;
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

hsGMaterial::hsGMaterial() :
fLOD(0),
fCompFlags(0),
fLoadFlags(0),
fLastUpdateTime(0)
{
}

hsGMaterial::~hsGMaterial()
{
    IClearLayers();
}

plLayerInterface* hsGMaterial::GetPiggyBack(uint32_t which)
{
    return fPiggyBacks[which];
}

plLayerInterface* hsGMaterial::GetLayer(uint32_t which)
{
    return fLayers[which];
}

uint32_t hsGMaterial::IMakeExtraLayer()
{
    fLayers.ExpandAndZero(GetNumLayers()+1);
    return fLayers.GetCount();
}

void hsGMaterial::IClearLayers()
{
    fLayers.Reset();
}

void hsGMaterial::SetNumLayers(int cnt)
{
    if( cnt < fLayers.GetCount() )
        fLayers.SetCount(cnt);
    else
        fLayers.ExpandAndZero(cnt);
}

hsGMaterial* hsGMaterial::Clone()
{
    hsGMaterial* clo = CloneNoLayers();

    clo->SetNumLayers(GetNumLayers());
    
    int i;
    for( i = 0; i < GetNumLayers(); i++ )
        clo->SetLayer(fLayers[i], i);

    return clo;
}

hsGMaterial* hsGMaterial::CloneNoLayers()
{
    hsGMaterial* clo = new hsGMaterial;

    clo->fCompFlags = fCompFlags;
    clo->fLoadFlags = fLoadFlags;

    return clo;
}

plLayer* hsGMaterial::MakeBaseLayer()
{
    plLayer* newLay = new plLayer;
    newLay->InitToDefault();
    IClearLayers();
    
    hsAssert(GetKey(), "All materials need a key (or temp key)");

    plString buff;
    if( !GetKeyName().IsNull() )
        buff = plString::Format("%s_Layer", GetKeyName().c_str());
    else
        buff = "Layer";
    hsgResMgr::ResMgr()->NewKey( buff, newLay, GetKey() != nil ? GetKey()->GetUoid().GetLocation() : plLocation::kGlobalFixedLoc );

    // Add layer so we have it now.
    AddLayerViaNotify(newLay);

    return newLay;
}

uint32_t hsGMaterial::AddLayerViaNotify(plLayerInterface* layer)
{
    int idx = GetNumLayers();

    // Add via notify so we'll dispose of it properly later.
    plMatRefMsg* msg = new plMatRefMsg(GetKey(), plRefMsg::kOnRequest, idx, plMatRefMsg::kLayer);
    hsgResMgr::ResMgr()->SendRef(layer->GetKey(), msg, plRefFlags::kActiveRef);

    fLayers.SetCount(idx+1);
    fLayers[idx] = layer;

    return idx;
}

void hsGMaterial::ReplaceLayer(plLayerInterface* oldLay, plLayerInterface* newLay, bool piggyBack)
{
    hsTArray<plLayerInterface*>& layers = piggyBack ? fPiggyBacks : fLayers;
    int i;
    for( i = 0; i < layers.GetCount(); i++ )
    {
        if( layers[i] == oldLay )
            break;
    }
    hsAssert(i < layers.GetCount(), "Replacing a layer we don't have");
    if( i >= layers.GetCount() )
        return;
    SetLayer(newLay, i, piggyBack);
}

void hsGMaterial::RemoveLayer(plLayerInterface* lay, bool piggyBack)
{
    hsTArray<plLayerInterface*>& layers = piggyBack ? fPiggyBacks : fLayers;
    int i;
    for( i = 0; i < layers.GetCount(); i++ )
    {
        if( layers[i] == lay )
            break;
    }

    if (i >= layers.GetCount())
        return;

    layers.Remove(i);
}

void hsGMaterial::InsertLayer(plLayerInterface* layer, int32_t which, bool piggyBack)
{
    hsTArray<plLayerInterface*>& layers = piggyBack ? fPiggyBacks : fLayers;
    hsAssert(which <= layers.GetCount(), "Material layers Exceeding test depth");
    layers.InsertAtIndex(which, layer);
}

void hsGMaterial::SetLayer(plLayerInterface* layer, int32_t which, bool insert, bool piggyBack)
{
    if( insert )
    {
        InsertLayer(layer, which, piggyBack);
    }
    else
    {
        hsTArray<plLayerInterface*>& layers = piggyBack ? fPiggyBacks : fLayers;
        if( which < 0 )
            which = layers.GetCount();
        hsAssert(which <= layers.GetCount(), "Material layers Exceeding test depth");
        if( which < layers.GetCount() )
            layers[which] = layer;
        else
            layers.Append(layer);
    }
}


void hsGMaterial::Write(hsStream* s)
{
    s->WriteLE32(fLoadFlags);
    s->WriteLE32(fCompFlags);

    s->WriteLE32(GetNumLayers());
    s->WriteLE32(GetNumPiggyBacks());
}

void hsGMaterial::Read(hsStream* s)
{
    fLoadFlags = s->ReadLE32();
    fCompFlags = s->ReadLE32();

    IClearLayers();
    int n = s->ReadLE32();
    fLayers.SetCountAndZero(n);
    n = s->ReadLE32();
    fPiggyBacks.SetCountAndZero(n);
}

void hsGMaterial::Write(hsStream *stream, hsResMgr *group)
{
    plSynchedObject::Write(stream, group);

    Write(stream);

    // Write one (or many) texture indices
    int iLay;
    for( iLay = 0; iLay < GetNumLayers(); iLay++ )
    {
        group->WriteKey(stream,GetLayer(iLay));
    }
    for( iLay = 0; iLay < GetNumPiggyBacks(); iLay++ )
    {
        group->WriteKey(stream, GetPiggyBack(iLay));
    }
}

void hsGMaterial::Read(hsStream *stream, hsResMgr *group)
{
    plSynchedObject::Read(stream, group);

    Read(stream);

    int iLay;    
    // Assign texture(s)
    for (iLay = 0; iLay < GetNumLayers(); iLay++)
    {
        plMatRefMsg* msg = new plMatRefMsg(GetKey(), plRefMsg::kOnCreate, iLay, plMatRefMsg::kLayer);
        plKey key = group->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
    }
    for (iLay = 0; iLay < GetNumPiggyBacks(); iLay++)
    {
        plMatRefMsg* msg = new plMatRefMsg(GetKey(), plRefMsg::kOnCreate, iLay, plMatRefMsg::kPiggyBack);
        plKey key = group->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
    }
}

void hsGMaterial::Eval(double secs, uint32_t frame)
{
    plProfile_BeginLap(MaterialAnims, GetKeyName().c_str());

    int i;
    for( i = 0; i < GetNumLayers(); i++ )
    {
        if( fLayers[i] )
            fLayers[i]->Eval(secs, frame, 0);
    }
    for( i = 0; i < GetNumPiggyBacks(); i++ )
    {
        if( fPiggyBacks[i] )
            fPiggyBacks[i]->Eval(secs, frame, 0);
    }

    plProfile_EndLap(MaterialAnims, GetKeyName().c_str());
}

void hsGMaterial::Reset()
{
    int i;
    for( i = 0; i < GetNumLayers(); i++ )
    {
        if( fLayers[i] )
            fLayers[i]->Eval(0, 0, 0);
    }
}

void hsGMaterial::Init()
{
    Reset();
}

bool hsGMaterial::MsgReceive(plMessage* msg)
{
    plMatRefMsg* refMsg = plMatRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        int which = refMsg->fWhich;
        bool piggyBack = 0 != (refMsg->fType & plMatRefMsg::kPiggyBack);
        plLayerInterface* lay= plLayerInterface::ConvertNoRef(refMsg->GetRef());
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
        {
            bool insert = 0 != (refMsg->fType & plMatRefMsg::kInsert);
            SetLayer(lay, which, 
                insert,
                piggyBack );
        }
        else if( refMsg->GetContext() & plRefMsg::kOnReplace )
            ReplaceLayer(plLayerInterface::ConvertNoRef(refMsg->GetOldRef()), lay, piggyBack);
        else if( refMsg->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
            RemoveLayer(lay, piggyBack);
        else
            ReplaceLayer(lay, &defaultLayer, piggyBack);
        return true;
    }
    return plSynchedObject::MsgReceive(msg);
}
