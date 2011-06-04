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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plInstanceDrawInterface.h"
#include "plSharedMesh.h"
#include "plMorphSequence.h"
#include "../plMessage/plReplaceGeometryMsg.h"
#include "plDrawableSpans.h"
#include "plGeometrySpan.h"
#include "../plScene/plSceneNode.h"
#include "../pnMessage/plDISpansMsg.h"
#include "hsResMgr.h"

plInstanceDrawInterface::plInstanceDrawInterface() : plDrawInterface(), fTargetID(-1) {}

plInstanceDrawInterface::~plInstanceDrawInterface() {}

void plInstanceDrawInterface::Read(hsStream* stream, hsResMgr* mgr)
{
	plDrawInterface::Read(stream, mgr);

	fTargetID = stream->ReadSwap32();
	plSwapSpansRefMsg *sMsg = TRACKED_NEW plSwapSpansRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
	mgr->ReadKeyNotifyMe(stream, sMsg, plRefFlags::kActiveRef);
}

void plInstanceDrawInterface::Write(hsStream* stream, hsResMgr* mgr)
{
	plDrawInterface::Write(stream, mgr);
	
	stream->WriteSwap32(fTargetID);
	mgr->WriteKey(stream, fDrawable->GetKey());
}

hsBool plInstanceDrawInterface::MsgReceive(plMessage* msg)
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
			fDrawable = nil;
		else
			fDrawable = plDrawableSpans::ConvertNoRef(refMsg->GetRef());
		return true;
	}
	
	return plDrawInterface::MsgReceive(msg);
}

void plInstanceDrawInterface::AddSharedMesh(plSharedMesh *mesh, hsGMaterial *mat, hsBool addToFront, int lod, hsBool partialSort)
{
	if (fDrawable == nil)
	{
		hsAssert(false, "Missing drawable when instancing a shared mesh. Ignoring instance.");
		return;
	}

#ifdef MF_NOSHADOW_ACC
	// TESTHACKERY FOLLOWS - GlassesNoShadow
	UInt32 noShadHack = 0;
	if( mesh->GetKey() && (strstr(mesh->GetKey()->GetName(), "lasses") || strstr(mesh->GetKey()->GetName(), "oggles")) )
		noShadHack = plGeometrySpan::kPropNoShadowCast;
#endif // MF_NOSHADOW_ACC

	int i;
	for (i = 0; i < mesh->fSpans.GetCount(); i++)
	{
		mesh->fSpans[i]->fMaterial = mat;

		if( partialSort )
		{
			mesh->fSpans[i]->fProps |= plGeometrySpan::kPartialSort;
		}
		else
			mesh->fSpans[i]->fProps &= ~plGeometrySpan::kPartialSort;

#ifdef MF_NOSHADOW_ACC
		mesh->fSpans[i]->fProps |= noShadHack;
#endif // MF_NOSHADOW_ACC
	}
			
	// Add the spans to the drawable
	UInt32 index = (UInt32)-1;
	index = fDrawable->AppendDISpans(mesh->fSpans, index, false, true, addToFront, lod);
			
	// Tell the drawInterface what drawable and index it wants.
	UInt8 iDraw = (UInt8)GetNumDrawables();
	ISetDrawable(iDraw, fDrawable);
	SetDrawableMeshIndex(iDraw, index);
	SetSharedMesh(iDraw, mesh);
	if (GetProperty(kDisable))
		fDrawables[iDraw]->SetProperty(fDrawableIndices[iDraw], kDisable, true);

#ifdef HS_DEBUGGING
	plDISpansMsg* diMsg = TRACKED_NEW plDISpansMsg(fDrawable->GetKey(), plDISpansMsg::kAddingSpan, index, plDISpansMsg::kLeaveEmptyDrawable);
	diMsg->SetSender(GetKey());
	diMsg->Send();
#endif			

	plSharedMeshBCMsg *smMsg = TRACKED_NEW plSharedMeshBCMsg;
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
			//hsgResMgr::ResMgr()->AddViaNotify(mesh->GetKey(), TRACKED_NEW plGenRefMsg(morph->GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kPassiveRef);
			morph->AddSharedMesh(mesh);
		}
	}
}

void plInstanceDrawInterface::RemoveSharedMesh(plSharedMesh *mesh)
{
	UInt32 geoIndex = fMeshes.Find(mesh);
	if (geoIndex != fMeshes.kMissingIndex)
	{
		IClearIndex((UInt8)geoIndex);
				
		plSharedMeshBCMsg *smMsg = TRACKED_NEW plSharedMeshBCMsg;
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

void plInstanceDrawInterface::ICheckDrawableIndex(UInt8 which)
{
	if( which >= fMeshes.GetCount() )
	{
		fMeshes.ExpandAndZero(which+1);
	}
	
	plDrawInterface::ICheckDrawableIndex(which);
}

void plInstanceDrawInterface::ReleaseData()
{
	fMeshes.Reset();
	
	plDrawInterface::ReleaseData();
}

void plInstanceDrawInterface::SetSharedMesh(UInt8 which, plSharedMesh *mesh)
{
	ICheckDrawableIndex(which);
	fMeshes[which] = mesh;
}

void plInstanceDrawInterface::IClearIndex(UInt8 which)
{
	plDrawableSpans *drawable = plDrawableSpans::ConvertNoRef(fDrawables[which]);
	if (drawable != nil)
	{
		plDISpansMsg* diMsg = TRACKED_NEW plDISpansMsg(fDrawable->GetKey(), plDISpansMsg::kRemovingSpan, fDrawableIndices[which], plDISpansMsg::kLeaveEmptyDrawable);
		diMsg->SetSender(GetKey());
		diMsg->Send();
	}
	
	fDrawables.Remove(which);
	fDrawableIndices.Remove(which);
	fMeshes.Remove(which);
}

// Temp testing - not really ideal. Check with Bob for real soln.
Int32 plInstanceDrawInterface::GetSharedMeshIndex(const plSharedMesh *mesh) const
{
	int i;
	for( i = 0; i < fMeshes.GetCount(); i++ )
	{
		if( fMeshes[i] == mesh )
			return i;
	}
	return -1;
}