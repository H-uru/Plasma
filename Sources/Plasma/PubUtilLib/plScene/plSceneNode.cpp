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
#include <algorithm>

#include "plSceneNode.h"
#include "pnDispatch/plDispatch.h"
#include "plMessage/plNodeCleanupMsg.h"
#include "pnMessage/plNodeRefMsg.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "pnSceneObject/plSceneObject.h"
#include "plDrawable.h"
#include "plPhysical.h"
#include "plAudible.h"
#include "plGLight/plLightInfo.h"
#include "pnMessage/plRefMsg.h"
#include "plPipeline.h"
#include "pnKeyedObject/plKey.h"
#include "plDrawable/plSpaceTreeMaker.h"
#include "plDrawable/plSpaceTree.h"
#include "plPageTreeMgr.h"
#include "plOccluder.h"

//MFHORSE
//BLACK
// temp hack for debugging
#include "plDrawable/plDrawableSpans.h"
#include "pnKeyedObject/plKeyImp.h"

plSceneNode::plSceneNode()
:   fDepth(0),
    fSpaceTree(nil),
    fFilterGenerics(false)
{
}

plSceneNode::~plSceneNode()
{
    plgDispatch::Dispatch()->UnRegisterForExactType(plNodeCleanupMsg::Index(), GetKey());

    delete fSpaceTree;
}

//// Init ////////////////////////////////////////////////////////////////////
//  Because we can't register for messages on construction. Doh.

void plSceneNode::Init()
{
    ///  :P
    plgDispatch::Dispatch()->RegisterForExactType(plNodeCleanupMsg::Index(), GetKey());
}

void plSceneNode::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);

    uint32_t n;
    int i;

    n = s->ReadLE32();
    fSceneObjects.clear();
    for( i = 0; i < n; i++ )
    {
        plNodeRefMsg* refMsg = new plNodeRefMsg(GetKey(), plRefMsg::kOnCreate, i, plNodeRefMsg::kObject);
        plKey key = mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
    }

    n = s->ReadLE32();
    fGenericPool.clear();
    for( i = 0; i < n; i++ )
    {
        plNodeRefMsg* refMsg = new plNodeRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric);
        mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
    }
}

void plSceneNode::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    int i;

    s->WriteLE32(fSceneObjects.size());
    for( i = 0; i < fSceneObjects.size(); i++ )
        mgr->WriteKey(s,fSceneObjects[i]);

    s->WriteLE32(fGenericPool.size());
    for( i = 0; i < fGenericPool.size(); i++ )
        mgr->WriteKey(s, fGenericPool[i]);
}

void plSceneNode::Harvest(plVolumeIsect* isect, hsTArray<plDrawVisList>& levList)
{
    static hsTArray<int16_t> visList;
    visList.SetCount(0);
    GetSpaceTree()->HarvestLeaves(isect, visList);
    static hsTArray<int16_t> visSpans;
    visSpans.SetCount(0);

    int i;
    for( i = 0; i < visList.GetCount(); i++ )
    {
        int idx = visList[i];
        fDrawPool[idx]->GetSpaceTree()->HarvestLeaves(isect, visSpans);
        if( visSpans.GetCount() )
        {
            plDrawVisList* drawVis = levList.Push();
            drawVis->fDrawable = fDrawPool[idx];
            drawVis->fVisList.Swap(visSpans);
        }
    }
}

void plSceneNode::CollectForRender(plPipeline* pipe, hsTArray<plDrawVisList>& levList, plVisMgr* visMgr)
{
    static hsTArray<int16_t> visList;
    visList.SetCount(0);
    pipe->HarvestVisible(GetSpaceTree(), visList);
    static hsTArray<int16_t> visSpans;
    visSpans.SetCount(0);

    int i;
    for( i = 0; i < visList.GetCount(); i++ )
    {
        int idx = visList[i];
        if( pipe->PreRender(fDrawPool[idx], visSpans, visMgr) )
        {
            plDrawVisList* drawVis = levList.Push();
            drawVis->fDrawable = fDrawPool[idx];
            drawVis->fVisList.Swap(visSpans);
        }
    }
}

void plSceneNode::SubmitOccluders(plPageTreeMgr* pageMgr) const
{
    pageMgr->AddOccluderList(fOccluders);
}

plSpaceTree* plSceneNode::IBuildSpaceTree()
{
    plSpaceTreeMaker maker;
    maker.Reset();
    
    hsBounds3Ext bnd;
    hsPoint3 zero(0, 0, 0);
    bnd.Reset(&zero);
    
    int i;
    for( i = 0; i < fDrawPool.size(); i++ )
    {
        if( fDrawPool[i] )
            maker.AddLeaf(fDrawPool[i]->GetSpaceTree()->GetWorldBounds());
        else
            maker.AddLeaf(bnd, true);
    }

    fSpaceTree = maker.MakeTree();
    fSpaceTree->MakeDirty();

    return fSpaceTree;
}

plSpaceTree* plSceneNode::ITrashSpaceTree()
{
    delete fSpaceTree;
    return fSpaceTree = nil;
}

void plSceneNode::IDirtySpaceTree()
{
    for (size_t i = 0; i < fDrawPool.size(); ++i)
    {
        plDrawable* drawable = fDrawPool[i];
        if (drawable && drawable->GetSpaceTree()->IsDirty() )
        {
            drawable->GetSpaceTree()->Refresh();
            fSpaceTree->MoveLeaf(i, drawable->GetSpaceTree()->GetWorldBounds());
        }
    }
}

plSpaceTree* plSceneNode::GetSpaceTree()
{
    if (!fSpaceTree)
        IBuildSpaceTree();
    IDirtySpaceTree();
    return fSpaceTree;
}

void plSceneNode::ISetDrawable(plDrawable* d)
{
    if( !d )
        return;

    if (std::find(fDrawPool.begin(), fDrawPool.end(), d) == fDrawPool.end()) {
        fDrawPool.push_back(d);
        ITrashSpaceTree();
    }
}

void plSceneNode::ISetAudible(plAudible* a)
{
    if( !a )
        return;

    if (std::find(fAudioPool.begin(), fAudioPool.end(), a) == fAudioPool.end())
        fAudioPool.push_back(a);
}

void plSceneNode::ISetPhysical(plPhysical* p)
{
    if( !p )
        return;

    if (std::find(fSimulationPool.begin(), fSimulationPool.end(), p) == fSimulationPool.end())
        fSimulationPool.push_back(p);
}

void plSceneNode::ISetObject(plSceneObject* o)
{
    if (!o)
        return;

    if (std::find(fSceneObjects.begin(), fSceneObjects.end(), o) == fSceneObjects.end())
        fSceneObjects.push_back(o);

    // MF_NET_GROUPS_TEST
    // This will have no effect on members of NetGroupConstants
    o->SetNetGroup(o->SelectNetGroup(GetKey()));

    o->SetSceneNode(GetKey());
}

void plSceneNode::ISetLight(plLightInfo* l)
{
    if (std::find(fLightPool.begin(), fLightPool.end(), l) == fLightPool.end())
        fLightPool.push_back(l);
}

void plSceneNode::ISetOccluder(plOccluder* o)
{
    if( fOccluders.kMissingIndex == fOccluders.Find(o) )
    {
        fOccluders.Append(o);
    }
}

void plSceneNode::ISetGeneric(hsKeyedObject* k)
{
    if (std::find(fGenericPool.begin(), fGenericPool.end(), k) == fGenericPool.end())
        fGenericPool.push_back(k);
}

void plSceneNode::IRemoveDrawable(plDrawable* d)
{
    auto it = std::find(fDrawPool.begin(), fDrawPool.end(), d);
    if (it != fDrawPool.end()) {
        fDrawPool.erase(it);
        ITrashSpaceTree();
    }
}

void plSceneNode::IRemoveAudible(plAudible* a)
{
    auto it = std::find(fAudioPool.begin(), fAudioPool.end(), a);
    if (it != fAudioPool.end())
        fAudioPool.erase(it);
}

void plSceneNode::IRemovePhysical(plPhysical* p)
{
    hsAssert(p, "Removing nil physical");

#ifdef HS_DEBUGGING
    if (p)
    {
        plKey oldNodeKey = p->GetSceneNode();
        if (oldNodeKey && oldNodeKey != GetKey())
        {
            char buf[256];
            sprintf(buf, "Trying to remove physical %s from scenenode %s,\nbut it's actually in %s",
                p->GetKeyName().c_str(), GetKeyName().c_str(), oldNodeKey->GetName().c_str());
            hsAssert(0, buf);
        }
    }
#endif

    auto it = std::find(fSimulationPool.begin(), fSimulationPool.end(), p);
    if (it != fSimulationPool.end())
        fSimulationPool.erase(it);
}

void plSceneNode::IRemoveObject(plSceneObject* o)
{
    auto it = std::find(fSceneObjects.begin(), fSceneObjects.end(), o);
    if (it != fSceneObjects.end())
        fSceneObjects.erase(it);
}

void plSceneNode::IRemoveLight(plLightInfo* l)
{
    hsAssert(l, "Removing nil light");

    auto it = std::find(fLightPool.begin(), fLightPool.end(), l);
    if (it != fLightPool.end())
        fLightPool.erase(it);
}

void plSceneNode::IRemoveOccluder(plOccluder* o)
{
    int idx = fOccluders.Find(o);
    if( idx != fOccluders.kMissingIndex )
        fOccluders.Remove(idx);
}

void plSceneNode::IRemoveGeneric(hsKeyedObject* k)
{
    auto it = std::find(fGenericPool.begin(), fGenericPool.end(), k);
    if (it != fGenericPool.end())
        fGenericPool.erase(it);
}

bool plSceneNode::IOnRemove(plNodeRefMsg* refMsg)
{

    switch( refMsg->fType )
    {
    case plNodeRefMsg::kDrawable:
        IRemoveDrawable(plDrawable::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kPhysical:
        IRemovePhysical(plPhysical::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kAudible:
        IRemoveAudible(plAudible::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kObject:
        IRemoveObject(plSceneObject::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kLight:
        IRemoveLight(plLightInfo::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kOccluder:
        IRemoveOccluder(plOccluder::ConvertNoRef(refMsg->GetRef()));
        break;
    case plNodeRefMsg::kGeneric:
        IRemoveGeneric(refMsg->GetRef());
        break;
    }
    if( refMsg->GetRef() && (refMsg->GetContext() & plRefMsg::kOnRemove) )
        GetKey()->Release(refMsg->GetRef()->GetKey());
        
    return true;
}

bool plSceneNode::IOnAdd(plNodeRefMsg* refMsg)
{
    int which = refMsg->fWhich;

    switch( refMsg->fType )
    {
    case plNodeRefMsg::kDrawable:
        ISetDrawable(plDrawable::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kPhysical:
        ISetPhysical(plPhysical::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kAudible:
        ISetAudible(plAudible::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kObject:
        ISetObject(plSceneObject::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kLight:
        ISetLight(plLightInfo::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kOccluder:
        ISetOccluder(plOccluder::ConvertNoRef(refMsg->GetRef()));
        return true;
    case plNodeRefMsg::kGeneric:
        ISetGeneric(refMsg->GetRef());
    }
    return true;
}

bool plSceneNode::MsgReceive(plMessage* msg)
{
    plNodeCleanupMsg    *cleanMsg = plNodeCleanupMsg::ConvertNoRef( msg );

    if( cleanMsg )
    {
        ICleanUp();
        return true;
    }

    plNodeRefMsg* refMsg = plNodeRefMsg::ConvertNoRef(msg);
    
    if( refMsg )
    {
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            return IOnAdd(refMsg);
        else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            return IOnRemove(refMsg);

    }

    return hsKeyedObject::MsgReceive(msg);
}

//// ICleanUp ////////////////////////////////////////////////////////////////
//  Export only: Clean up the scene node (i.e. make sure drawables optimize)

void    plSceneNode::ICleanUp()
{
    /// Go find drawables to delete
    for (auto draw : fDrawPool)
        draw->Optimize();

    if (fFilterGenerics)
    {
        int i;
        for ( i = fSceneObjects.size() - 1; i >= 0; i--)
            GetKey()->Release(fSceneObjects[i]->GetKey());
        for ( i = fDrawPool.size() - 1; i >= 0; i--)
            GetKey()->Release(fDrawPool[i]->GetKey());
        for ( i = fSimulationPool.size() - 1; i >= 0; i--)
            GetKey()->Release(fSimulationPool[i]->GetKey());
        for ( i = fAudioPool.size() - 1; i >= 0; i--)
            GetKey()->Release(fAudioPool[i]->GetKey());
        for ( i = fOccluders.GetCount() - 1; i >= 0; i--)
            GetKey()->Release(fOccluders[i]->GetKey());
        for ( i = fLightPool.size() - 1; i >= 0; i--)
            GetKey()->Release(fLightPool[i]->GetKey());
    }

    ITrashSpaceTree();
}

//// GetMatchingDrawable /////////////////////////////////////////////////////
//  Export only: Query for a given drawable.

plDrawable  *plSceneNode::GetMatchingDrawable( const plDrawableCriteria& crit )
{
    for (auto draw : fDrawPool) {
        if (draw->DoIMatch(crit))
            return draw;
    }
    return nullptr;
}

//// OptimizeDrawables ///////////////////////////////////////////////////////
//  Loops through all the drawables and calls Optimize on each one. For the
//  export side, to be called right before writing the drawables to disk.

void    plSceneNode::OptimizeDrawables()
{
    for (auto draw : fDrawPool)
        draw->Optimize();
}

