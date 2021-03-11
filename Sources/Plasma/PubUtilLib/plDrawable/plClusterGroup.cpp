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
#include "plClusterGroup.h"

#include "plSpanTemplate.h"
#include "plCluster.h"

#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"

#include "plScene/plVisMgr.h"
#include "plScene/plVisRegion.h"

#include "plSurface/hsGMaterial.h"

#include "plGLight/plLightInfo.h"

#include "plDrawableSpans.h"

#include "hsBitVector.h"
#include "hsStream.h"
#include "hsResMgr.h"

//STUB
#include "plgDispatch.h"

plClusterGroup::plClusterGroup()
:   fTemplate(),
    fMaterial(),
    fUnPacked()
{
    fVisSet.SetBit(plVisMgr::kNormal);
}

plClusterGroup::~plClusterGroup()
{
    for (plCluster* cluster : fClusters)
        delete cluster;

    delete fTemplate;
}

plCluster* plClusterGroup::IAddCluster()
{
    plCluster* cluster = new plCluster;
    // Set the cluster's group.
    cluster->SetGroup(this);
    fClusters.emplace_back(cluster);
    return cluster;
}

plCluster* plClusterGroup::IGetCluster(int i) const
{ 
    return fClusters[i]; 
}

const plCluster* plClusterGroup::GetCluster(size_t i) const
{
    return fClusters[i];
}

void plClusterGroup::Read(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Read(stream, mgr);

    fTemplate = new plSpanTemplate;
    fTemplate->Read(stream);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefMaterial), plRefFlags::kActiveRef);

    const uint32_t numClust = stream->ReadLE32();
    fClusters.resize(numClust);
    for (uint32_t i = 0; i < numClust; i++)
    {
        fClusters[i] = new plCluster;
        fClusters[i]->Read(stream, this);
    }

    const uint32_t numRegions = stream->ReadLE32();
    for (uint32_t i = 0; i < numRegions; i++)
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefRegion), plRefFlags::kActiveRef);

    const uint32_t numLights = stream->ReadLE32();
    for (uint32_t i = 0; i < numLights; i++)
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefLight), plRefFlags::kActiveRef);

    fLOD.Read(stream);

    fRenderLevel.Set(stream->ReadLE32());

    fSceneNode = mgr->ReadKey(stream);

    //STUB
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plClusterGroup::Write(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Write(stream, mgr);

    fTemplate->Write(stream);

    mgr->WriteKey(stream, fMaterial);

    stream->WriteLE32((uint32_t)fClusters.size());
    for (plCluster* cluster : fClusters)
        cluster->Write(stream);

    stream->WriteLE32((uint32_t)fRegions.size());
    for (plVisRegion* region : fRegions)
        mgr->WriteKey(stream, region);

    stream->WriteLE32((uint32_t)fLights.size());
    for (plLightInfo* light : fLights)
        mgr->WriteKey(stream, light);

    fLOD.Write(stream);

    stream->WriteLE32(fRenderLevel.Level());

    mgr->WriteKey(stream, fSceneNode);
}

void plClusterGroup::ISendToSelf(RefType t, hsKeyedObject* ref)
{
    hsAssert(ref, "Sending self a nil object");
    plGenRefMsg* refMsg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, t);
    hsgResMgr::ResMgr()->SendRef(ref->GetKey(), refMsg, plRefFlags::kActiveRef);
}

bool plClusterGroup::IAddVisRegion(plVisRegion* reg)
{
    if( reg )
    {
        if (std::find(fRegions.cbegin(), fRegions.cend(), reg) == fRegions.cend())
        {
            fRegions.emplace_back(reg);
            if( reg->GetProperty(plVisRegion::kIsNot) )
                fVisNot.SetBit(reg->GetIndex());
            else
            {
                fVisSet.SetBit(reg->GetIndex());
                if( reg->ReplaceNormal() )
                    fVisSet.ClearBit(plVisMgr::kNormal);
            }
        }
    }
    return true;
}

bool plClusterGroup::IRemoveVisRegion(plVisRegion* reg)
{
    if( reg )
    {
        auto iter = std::find(fRegions.cbegin(), fRegions.cend(), reg);
        if (iter != fRegions.cend())
        {
            fRegions.erase(iter);
            if( reg->GetProperty(plVisRegion::kIsNot) )
                fVisNot.ClearBit(reg->GetIndex());
            else
                fVisSet.ClearBit(reg->GetIndex());
        }
    }
    return true;
}

bool plClusterGroup::IAddLight(plLightInfo* li)
{
    if (std::find(fLights.cbegin(), fLights.cend(), li) == fLights.cend())
    {
        fLights.emplace_back(li);
    }
    return true;
}

bool plClusterGroup::IRemoveLight(plLightInfo* li)
{
    auto iter = std::find(fLights.cbegin(), fLights.cend(), li);
    if (iter != fLights.cend())
    {
        fLights.erase(iter);
    }
    return true;
}

bool plClusterGroup::IOnReceive(plGenRefMsg* ref)
{
    switch( ref->fType )
    {
    case kRefMaterial:
        fMaterial = hsGMaterial::ConvertNoRef(ref->GetRef());
        return true;
    case kRefRegion:
        return IAddVisRegion(plVisRegion::ConvertNoRef(ref->GetRef()));
    case kRefLight:
        return IAddLight(plLightInfo::ConvertNoRef(ref->GetRef()));
    }
    return false;
}

bool plClusterGroup::IOnRemove(plGenRefMsg* ref)
{
    switch( ref->fType )
    {
    case kRefMaterial:
        fMaterial = nullptr;
        return true;
    case kRefRegion:
        return IRemoveVisRegion(plVisRegion::ConvertNoRef(ref->GetRef()));
    case kRefLight:
        return IRemoveLight(plLightInfo::ConvertNoRef(ref->GetRef()));
    }
    return false;
}

bool plClusterGroup::IOnRef(plGenRefMsg* ref)
{
    if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
    {
        return IOnReceive(ref);
    }

    return IOnRemove(ref);
}

bool plClusterGroup::MsgReceive(plMessage* msg)
{
    plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
    if( ref )
    {
        if( IOnRef(ref) )
            return true;
    }

    // STUB
    plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
    if (evalMsg)
    {
        UnPack();
        fUnPacked = true;
        plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

void plClusterGroup::UnPack()
{
    plDrawableSpans* drawable = new plDrawableSpans;
    fDrawable = hsgResMgr::ResMgr()->NewKey(GetKey()->GetName(), drawable, GetKey()->GetUoid().GetLocation());
    drawable->UnPackCluster(this);

    drawable->SetSceneNode(fSceneNode);
}

void plClusterGroup::SetVisible(bool visible)
{
    if (fDrawable)
    {
        plDrawableSpans *drawable = plDrawableSpans::ConvertNoRef(fDrawable->ObjectIsLoaded());
        if (drawable)
            drawable->SetProperty(0,!visible); // property 0 is the disable drawing property
    }
}

size_t plClusterGroup::NumInst() const
{
    size_t numInst = 0;
    for (plCluster* cluster : fClusters)
        numInst += cluster->NumInsts();

    return numInst;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void plLODDist::Read(hsStream* s)
{
    fMinDist = s->ReadLEFloat();
    fMaxDist = s->ReadLEFloat();
}

void plLODDist::Write(hsStream* s) const
{
    s->WriteLEFloat(fMinDist);
    s->WriteLEFloat(fMaxDist);
}
