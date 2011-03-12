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

#include "hsTypes.h"
#include "plDrawInterface.h"
#include "plDrawable.h"
#include "hsBounds.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plSceneObject.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plDISpansMsg.h"

plDrawInterface::plDrawInterface()
{
}

plDrawInterface::~plDrawInterface()
{

}

void plDrawInterface::SetDrawableMeshIndex( UInt8 which, UInt32 index ) 
{
	ICheckDrawableIndex(which);

	fDrawableIndices[which] = index; 
}

void plDrawInterface::SetProperty(int prop, hsBool on)
{
	plObjInterface::SetProperty(prop, on);

	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] )
			fDrawables[i]->SetProperty(fDrawableIndices[i], prop, on);
	}
}

void plDrawInterface::ISetSceneNode(plKey newNode)
{
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] )
			fDrawables[i]->SetSceneNode(newNode);
	}
}

void plDrawInterface::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( !GetProperty(kDisable) )
	{
		int i;
		for( i = 0; i < fDrawables.GetCount(); i++ )
		{
			if( fDrawables[i] )
				fDrawables[i]->SetTransform( fDrawableIndices[i], l2w, w2l );
		}
	}
}

const hsBounds3Ext plDrawInterface::GetLocalBounds() const
{
	hsBounds3Ext retVal;
	retVal.MakeEmpty();
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] )
			retVal.Union(&fDrawables[i]->GetLocalBounds(fDrawableIndices[i]));
	}
	return retVal;
}

const hsBounds3Ext plDrawInterface::GetWorldBounds() const
{
	hsBounds3Ext retVal;
	retVal.MakeEmpty();
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] )
			retVal.Union(&fDrawables[i]->GetWorldBounds(fDrawableIndices[i]));
	}
	return retVal;
}

const hsBounds3Ext plDrawInterface::GetMaxWorldBounds() const
{
	hsBounds3Ext retVal;
	retVal.MakeEmpty();
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] )
			retVal.Union(&fDrawables[i]->GetMaxWorldBounds(fDrawableIndices[i]));
	}
	return retVal;
}

void plDrawInterface::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);

	int nDrawables = s->ReadSwap32();
	if (nDrawables > 0) 
		ICheckDrawableIndex(nDrawables-1);
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		fDrawableIndices[i] = s->ReadSwap32();

		plIntRefMsg* refMsg = TRACKED_NEW plIntRefMsg(GetKey(), plRefMsg::kOnCreate, i, plIntRefMsg::kDrawable);
		mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);
	}

	int nReg = s->ReadSwap32();
	fRegions.SetCountAndZero(nReg);
	for( i = 0; i < nReg; i++ )
	{
		plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefVisRegion);
		mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
	}
}

void plDrawInterface::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);

	s->WriteSwap32(fDrawables.GetCount());
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		s->WriteSwap32(fDrawableIndices[i]);
		
		mgr->WriteKey(s, fDrawables[i]);
	}
	
	s->WriteSwap32(fRegions.GetCount());
	for( i = 0; i < fRegions.GetCount(); i++ )
	{
		mgr->WriteKey(s, fRegions[i]);
	}
}

//// ReleaseData //////////////////////////////////////////////////////////////
//	Called by SceneViewer to release the data for this given object (when
//	its parent sceneObject is deleted).

void	plDrawInterface::ReleaseData( void )
{
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] && (fDrawableIndices[i] != UInt32(-1)) )
		{
			plDISpansMsg* diMsg = TRACKED_NEW plDISpansMsg(fDrawables[i]->GetKey(), plDISpansMsg::kRemovingSpan, fDrawableIndices[i], 0);
			diMsg->SetSender(GetKey());
			diMsg->Send();
		}
		//fDrawableIndices[i] = UInt32(-1);
		fDrawables.Reset();
		fDrawableIndices.Reset();
	}
}

void plDrawInterface::ICheckDrawableIndex(UInt8 which)
{
	if( which >= fDrawableIndices.GetCount() )
	{
		fDrawables.ExpandAndZero(which+1);
		
		int n = fDrawableIndices.GetCount();
		fDrawableIndices.ExpandAndZero(which+1);
		int i;
		for( i = n; i <= which; i++ )
			fDrawableIndices[i] = UInt32(-1);
	}
}

void plDrawInterface::ISetDrawable(UInt8 which, plDrawable* dr)
{
	ICheckDrawableIndex(which);
	fDrawables[which] = dr;
	
	if( dr )
		dr->SetSceneNode(GetSceneNode());

	// We might read the vis regions before the drawables, so
	// we have to check for any already loaded.
	ISetVisRegions(which);
	
#ifdef HS_DEBUGGING
	if( fDrawableIndices[which] != (UInt32)-1 )
	{
		plDISpansMsg* diMsg = TRACKED_NEW plDISpansMsg(dr->GetKey(), plDISpansMsg::kAddingSpan, fDrawableIndices[which], 0);
		diMsg->SetSender(GetKey());
		diMsg->Send();
	}
#endif
}

void plDrawInterface::IRemoveDrawable(plDrawable *dr)
{
	int idx = fDrawables.Find(dr);
	if( fDrawables.kMissingIndex != idx )
	{
		fDrawables[idx] = nil;
		fDrawableIndices[idx] = UInt32(-1);
	}
	else
	{
		hsAssert(false, "Trying to remove a drawable that doesn't belong to us");
	}
}

void plDrawInterface::ISetVisRegion(hsKeyedObject* reg, hsBool on)
{
	int i;
	for( i = 0; i < fDrawables.GetCount(); i++ )
	{
		if( fDrawables[i] && (fDrawableIndices[i] != UInt32(-1)) )
		{
			fDrawables[i]->SetDISpanVisSet(fDrawableIndices[i], reg, on);
		}
	}
	int idx = fRegions.Find(reg);
	if( on )
	{
		if( idx == fRegions.kMissingIndex )
			fRegions.Append(reg);
	}
	else
	{
		if( idx != fRegions.kMissingIndex )
			fRegions.Remove(idx);
	}

}

void plDrawInterface::ISetVisRegions(int iDraw)
{
	if( fDrawables[iDraw] && (fDrawableIndices[iDraw] != UInt32(-1)) )
	{
		int i;
		for( i = 0; i < fRegions.GetCount(); i++ )
		{
			fDrawables[iDraw]->SetDISpanVisSet(fDrawableIndices[iDraw], fRegions[i], true);
		}
	}
}

// Export only. Use messages for runtime
void plDrawInterface::SetDrawable(UInt8 which, plDrawable *dr)
{
	if( dr )
	{
		// This is a little convoluted, but it makes GCC happy and doesn't hurt anybody.
		plIntRefMsg* intRefMsg = TRACKED_NEW plIntRefMsg(GetKey(), plRefMsg::kOnCreate, which, plIntRefMsg::kDrawable);
		plRefMsg* refMsg = intRefMsg;
//		hsgResMgr::ResMgr()->SendRef(dr->GetKey(), intRefMsg, plRefFlags::kActiveRef); // THIS WON'T COMPILE UNDER GCC
		hsgResMgr::ResMgr()->SendRef(dr, refMsg, plRefFlags::kActiveRef);
	}
	else
	{
		ISetDrawable(which, nil);
	}
}

hsBool plDrawInterface::MsgReceive(plMessage* msg)
{
	plIntRefMsg* intRefMsg = plIntRefMsg::ConvertNoRef(msg);
	if( intRefMsg )
	{
		switch( intRefMsg->fType )
		{
		case plIntRefMsg::kDrawable:
			if( intRefMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				IRemoveDrawable(plDrawable::ConvertNoRef(intRefMsg->GetRef()));
			}
			else
			{
				ISetDrawable((UInt8)intRefMsg->fWhich, plDrawable::ConvertNoRef(intRefMsg->GetRef()));
			}
			return true;
		default:
			break;
		}
	}
	plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(msg);
	if( genRefMsg )
	{
		switch( genRefMsg->fType )
		{
		case kRefVisRegion:
			if( genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				ISetVisRegion(genRefMsg->GetRef(), true);
			else
				ISetVisRegion(genRefMsg->GetRef(), false);
			break;
		default:
			break;
		}
		return true;
	}
	plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef( msg );
	if (pEnableMsg)
	{
		SetProperty(kDisable, pEnableMsg->Cmd(plEnableMsg::kDisable));
		if( GetOwner() )
			SetTransform(GetOwner()->GetLocalToWorld(), GetOwner()->GetWorldToLocal());
		return true;
	}
	return plObjInterface::MsgReceive(msg);
}

void	plDrawInterface::SetUpForParticleSystem( UInt32 maxNumEmitters, UInt32 maxNumParticles, hsGMaterial *material, hsTArray<plKey>& lights )
{
	hsAssert( fDrawables[0] != nil, "No drawable to use for particle system!" );
	SetDrawableMeshIndex( 0, fDrawables[0]->CreateParticleSystem( maxNumEmitters, maxNumParticles, material ) );
	int i;
	for( i = 0; i < lights.GetCount(); i++ )
	{
		hsgResMgr::ResMgr()->AddViaNotify(lights[i], TRACKED_NEW plGenRefMsg(fDrawables[0]->GetKey(), plRefMsg::kOnCreate, fDrawableIndices[0], plDrawable::kMsgPermaLightDI), plRefFlags::kPassiveRef);
	}

	ISetVisRegions(0);
}

void	plDrawInterface::ResetParticleSystem( void )
{
	hsAssert( fDrawables[0] != nil, "No drawable to use for particle system!" );
	fDrawables[0]->ResetParticleSystem( fDrawableIndices[0] );
}

void	plDrawInterface::AssignEmitterToParticleSystem( plParticleEmitter *emitter )
{
	hsAssert( fDrawables[0] != nil, "No drawable to use for particle system!" );
	fDrawables[0]->AssignEmitterToParticleSystem( fDrawableIndices[0], emitter );
}


