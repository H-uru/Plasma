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
#include "plInstanceDrawInterface.h"
#include "plSharedMesh.h"
#include "plMorphSequence.h"
#include "plMessage/plReplaceGeometryMsg.h"
#include "plDrawableSpans.h"
#include "plGeometrySpan.h"
#include "plScene/plSceneNode.h"
#include "pnMessage/plDISpansMsg.h"
#include "hsResMgr.h"

plInstanceDrawInterface::plInstanceDrawInterface() : plDrawInterface(), fTargetID(-1) {}

plInstanceDrawInterface::~plInstanceDrawInterface() {}

void plInstanceDrawInterface::Read(hsStream* stream, hsResMgr* mgr)
{
    plDrawInterface::Read(stream, mgr);

    fTargetID = stream->ReadLE32();
    plSwapSpansRefMsg *sMsg = new plSwapSpansRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
    mgr->ReadKeyNotifyMe(stream, sMsg, plRefFlags::kActiveRef);
}

void plInstanceDrawInterface::Write(hsStream* stream, hsResMgr* mgr)
{
    plDrawInterface::Write(stream, mgr);
    
    stream->WriteLE32(fTargetID);
    mgr->WriteKey(stream, fDrawable->GetKey());
}

bool plInstanceDrawInterface::MsgReceive(plMessage* msg)
{
#if 0 // UNUSED
    // This currently isn't being used, so I'm commenting it out at
    // Bob's preference. If it does come back into play, the plReplaceGeometryMsg should also
    // contain an LOD field, saying which avatar LOD this is targetted for.
    // Just always specifying 0 won't break anything, it just circumvents some
    // optimizations the pipeline can make, so it'll look right, just slower.
    plReplaceGeometryMsg *rMsg = plReplaceGeometryMsg::ConvertNoRef(msg);
    if (rMsg)
    {
        if (rMsg->fFlags & rMsg->kAddingGeom)
            AddSharedMesh(rMsg->fMesh, rMsg->fMaterial, rMsg->fFlags & rMsg->kAddToFront);
        else
            RemoveSharedMesh(rMsg->fMesh);
        
        return true;
    }
#endif // UNUSED

    plSwapSpansRefMsg* refMsg = plSwapSpansRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            fDrawable = nullptr;
        else
            fDrawable = plDrawableSpans::ConvertNoRef(refMsg->GetRef());
        return true;
    }
    
    return plDrawInterface::MsgReceive(msg);
}

void plInstanceDrawInterface::AddSharedMesh(plSharedMesh *mesh, hsGMaterial *mat, bool addToFront, int lod, bool partialSort)
{
    if (fDrawable == nullptr)
    {
        hsAssert(false, "Missing drawable when instancing a shared mesh. Ignoring instance.");
        return;
    }

#ifdef MF_NOSHADOW_ACC
    // TESTHACKERY FOLLOWS - GlassesNoShadow
    uint32_t noShadHack = 0;
    if( mesh->GetKey() && (mesh->GetKey()->GetName().Find("lasses") >= 0 || mesh->GetKey()->GetName().Find("oggles") >= 0) )
        noShadHack = plGeometrySpan::kPropNoShadowCast;
#endif // MF_NOSHADOW_ACC

    for (plGeometrySpan* span : mesh->fSpans)
    {
        span->fMaterial = mat;

        if( partialSort )
            span->fProps |= plGeometrySpan::kPartialSort;
        else
            span->fProps &= ~plGeometrySpan::kPartialSort;

#ifdef MF_NOSHADOW_ACC
        span->fProps |= noShadHack;
#endif // MF_NOSHADOW_ACC
    }

    // Add the spans to the drawable
    uint32_t index = (uint32_t)-1;
    index = fDrawable->AppendDISpans(mesh->fSpans, index, false, true, addToFront, lod);

    // Tell the drawInterface what drawable and index it wants.
    size_t iDraw = GetNumDrawables();
    ISetDrawable(iDraw, fDrawable);
    SetDrawableMeshIndex(iDraw, index);
    SetSharedMesh(iDraw, mesh);
    if (GetProperty(kDisable))
        fDrawables[iDraw]->SetProperty(fDrawableIndices[iDraw], kDisable, true);

#ifdef HS_DEBUGGING
    plDISpansMsg* diMsg = new plDISpansMsg(fDrawable->GetKey(), plDISpansMsg::kAddingSpan, index, plDISpansMsg::kLeaveEmptyDrawable);
    diMsg->SetSender(GetKey());
    diMsg->Send();
#endif          

    plSharedMeshBCMsg *smMsg = new plSharedMeshBCMsg;
    smMsg->SetSender(GetKey());
    smMsg->fDraw = fDrawable;
    smMsg->fMesh = mesh;
    smMsg->fIsAdding = true;
    smMsg->Send();
            
    if (mesh->fMorphSet)
    {
        plMorphSequence *morph = const_cast<plMorphSequence*>(plMorphSequence::ConvertNoRef(fOwner->GetModifierByType(plMorphSequence::Index())));
        if (morph)
        {
            //hsgResMgr::ResMgr()->AddViaNotify(mesh->GetKey(), new plGenRefMsg(morph->GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kPassiveRef);
            morph->AddSharedMesh(mesh);
        }
    }
}

void plInstanceDrawInterface::RemoveSharedMesh(plSharedMesh *mesh)
{
    auto iter = std::find(fMeshes.cbegin(), fMeshes.cend(), mesh);
    if (iter != fMeshes.cend())
    {
        IClearIndex(iter - fMeshes.cbegin());

        plSharedMeshBCMsg *smMsg = new plSharedMeshBCMsg;
        smMsg->SetSender(GetKey());
        smMsg->fDraw = fDrawable;
        smMsg->fMesh = mesh;
        smMsg->fIsAdding = false;
        smMsg->Send();
                
        if (mesh->fMorphSet)
        {
            plMorphSequence *morph = const_cast<plMorphSequence*>(plMorphSequence::ConvertNoRef(fOwner->GetModifierByType(plMorphSequence::Index())));
            if (morph)
            {
                //morph->GetKey()->Release(mesh->GetKey());
                morph->RemoveSharedMesh(mesh);
            }
        }
    }
}

void plInstanceDrawInterface::ICheckDrawableIndex(size_t which)
{
    if (which >= fMeshes.size())
        fMeshes.resize(which + 1);

    plDrawInterface::ICheckDrawableIndex(which);
}

void plInstanceDrawInterface::ReleaseData()
{
    fMeshes.clear();
    
    plDrawInterface::ReleaseData();
}

void plInstanceDrawInterface::SetSharedMesh(size_t which, plSharedMesh *mesh)
{
    ICheckDrawableIndex(which);
    fMeshes[which] = mesh;
}

void plInstanceDrawInterface::IClearIndex(size_t which)
{
    plDrawableSpans *drawable = plDrawableSpans::ConvertNoRef(fDrawables[which]);
    if (drawable != nullptr)
    {
        plDISpansMsg* diMsg = new plDISpansMsg(fDrawable->GetKey(), plDISpansMsg::kRemovingSpan, fDrawableIndices[which], plDISpansMsg::kLeaveEmptyDrawable);
        diMsg->SetSender(GetKey());
        diMsg->Send();
    }
    
    fDrawables.erase(fDrawables.cbegin() + which);
    fDrawableIndices.erase(fDrawableIndices.cbegin() + which);
    fMeshes.erase(fMeshes.cbegin() + which);
}

// Temp testing - not really ideal. Check with Bob for real soln.
hsSsize_t plInstanceDrawInterface::GetSharedMeshIndex(const plSharedMesh *mesh) const
{
    auto iter = std::find(fMeshes.cbegin(), fMeshes.cend(), mesh);
    if (iter != fMeshes.cend())
        return hsSsize_t(iter - fMeshes.cbegin());

    return -1;
}
