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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plPassMtlBase - Base class for all Plasma MAX materials					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsBitVector.h"

#include "Max.h"
#include "iparamb2.h"
#include "notify.h"
#include "notetrck.h"

#include "plPassMtlBase.h"
#include "plPassBaseParamIDs.h"
#include "plNoteTrackWatcher.h"
#include "plAnimStealthNode.h"

#include "../MaxComponent/plMaxAnimUtils.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

// For converting from a MAX Mtl
#include "plPassMtl.h"
#include "plBumpMtl.h"
#include "plDecalMtl.h"

using namespace plPassBaseParamIDs;

IMtlParams *plPassMtlBase::fIMtlParams = nil;

//// plPostLoadHandler ///////////////////////////////////////////////////////
//	Small class to keep track of all the materials to update after load

class plPostLoadHandler
{
	static bool fLoading;
	static hsTArray<plPassMtlBase *> fPostLoads;

	public:
		static bool IsLoading() { return fLoading; }

		static void PostLoadFixupFunction( void *param, NotifyInfo *info )
		{
			fLoading = false;

			for( int i = 0; i < fPostLoads.GetCount(); i++ )
				fPostLoads[ i ]->PostLoadAnimPBFixup();

			fPostLoads.Reset();
			UnRegisterNotification( PostLoadFixupFunction, param, NOTIFY_FILE_POST_OPEN );
			UnRegisterNotification( PostLoadFixupFunction, param, NOTIFY_FILE_POST_MERGE );
		}

		static void	AddPostLoad( plPassMtlBase *mtl )
		{
			fLoading = true;

			if( fPostLoads.GetCount() == 0 )
			{
				RegisterNotification( PostLoadFixupFunction, mtl, NOTIFY_FILE_POST_OPEN );
				RegisterNotification( PostLoadFixupFunction, mtl, NOTIFY_FILE_POST_MERGE );
			}

			mtl->SetLoadingFlag( true );
			fPostLoads.Append( mtl );
		}

		static void	RemovePostLoad( plPassMtlBase *mtl )
		{
			for( int i = 0; i < fPostLoads.GetCount(); i++ )
			{
				if( fPostLoads[ i ] == mtl )
				{
					fPostLoads.Remove( i );
					break;
				}
			}
		}
};
hsTArray<plPassMtlBase *>	plPostLoadHandler::fPostLoads;
bool plPostLoadHandler::fLoading = false;


plPassMtlBase::plPassMtlBase( BOOL loading ) : fNTWatcher( nil ), fBasicPB(NULL), fAdvPB(NULL), fLayersPB(NULL), fAnimPB(NULL),
								 fLoading( loading )
{
	fNTWatcher = TRACKED_NEW plNoteTrackWatcher( this );
	Reset();
}

plPassMtlBase::~plPassMtlBase()
{
	if( fLoading )
		plPostLoadHandler::RemovePostLoad( this );

	// Force the watcher's parent pointer to nil, otherwise the de-ref will attempt to re-delete us
	fNTWatcher->SetReference( plNoteTrackWatcher::kRefParentMtl, nil );
	delete fNTWatcher;
	fNTWatcher = nil;

	// Manually delete our notetrack refs, otherwise there'll be hell to pay
	for( int i = 0; i < fNotetracks.GetCount(); i++ )
	{
		if( fNotetracks[ i ] != nil )
			DeleteReference( kRefNotetracks + i );
	}
}

void	plPassMtlBase::Reset( void ) 
{
	fIValid.SetEmpty();
}

//// Stealth Accessors ///////////////////////////////////////////////////////

int		plPassMtlBase::GetNumStealths( void )
{
	return IGetNumStealths( true );
}

plAnimStealthNode	*plPassMtlBase::GetStealth( int index )
{
	return IGetStealth( index, false );
}

int	plPassMtlBase::IGetNumStealths( hsBool update )
{
	if( update )
		IUpdateAnimNodes();

	return fAnimPB->Count( (ParamID)kPBAnimStealthNodes );
}

plAnimStealthNode	*plPassMtlBase::IGetStealth( int index, hsBool update )
{
	if( update )
		IUpdateAnimNodes();

	return (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, index );
}

plAnimStealthNode	*plPassMtlBase::IFindStealth( const char *segmentName )
{
	int		i;


	for( i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
	{
		plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
		const char *name = node->GetSegmentName();

		if( node != nil && strcmp( name, segmentName ) == 0 )
		{
			return node;
		}
	}

	return nil;
}

//// IVerifyStealthPresent ///////////////////////////////////////////////////
//	Ensures that we have a stealth for the given segment.

plAnimStealthNode	*plPassMtlBase::IVerifyStealthPresent( const char *animName )
{
	// If we're in the middle of loading, don't check
	if (plPostLoadHandler::IsLoading())
		return nil;

	plAnimStealthNode *stealth = IFindStealth( animName );
	if( stealth == nil )
	{
		// New segment, add a new stealth node
		stealth = (plAnimStealthNode *)GetCOREInterface()->CreateInstance( HELPER_CLASS_ID, ANIMSTEALTH_CLASSID );
		INode *node = GetCOREInterface()->CreateObjectNode( stealth );
		stealth->SetSegment( ( strcmp(animName, ENTIRE_ANIMATION_NAME) != 0 ) ? animName : nil );
		stealth->SetNodeName( GetName() );
		node->Freeze( true );

		// Skip the attach, since we might not find a valid INode. This will leave the node attached to the scene
		// root, which is fine. Since we just care about it being SOMEWHERE in the scene hierarchy
		/*
		if( fAnimPB->Count( (ParamID)kPBAnimStealthNodes ) > 0 )
		{
			plAnimStealthNode *first = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, 0 );
			first->GetINode()->AttachChild( node );
		}
		*/

		fAnimPB->Append( (ParamID)kPBAnimStealthNodes, 1, (ReferenceTarget **)&stealth );

		const char *realName = stealth->GetSegmentName();

		fStealthsChanged = true;
	}
	else
	{
		// Exists already, we're ok
		stealth->SetParentMtl( this );
	}
	return stealth;
}

//// Change Callbacks ////////////////////////////////////////////////////////

void	plPassMtlBase::RegisterChangeCallback( plMtlChangeCallback *callback )
{
	if( fChangeCallbacks.Find( callback ) == fChangeCallbacks.kMissingIndex )
		fChangeCallbacks.Append( callback );
}

void	plPassMtlBase::UnregisterChangeCallback( plMtlChangeCallback *callback )
{
	int idx = fChangeCallbacks.Find( callback );
	if( idx != fChangeCallbacks.kMissingIndex )
		fChangeCallbacks.Remove( idx );
}

//// IUpdateAnimNodes ////////////////////////////////////////////////////////
//	Updates the list of stealth nodes in the anim paramBlock to match our
//	list of anim segments.

void	plPassMtlBase::IUpdateAnimNodes( void )
{
	// Our beautiful hack, to make sure we don't update until we actually are loaded
	if( fLoading )
		return;

	SegmentMap *segMap = GetAnimSegmentMap( this, nil );

	hsTArray<plAnimStealthNode *>	goodNodes;
	

	// Keep track of whether we change anything
	fStealthsChanged = false;

	// Verify one for "entire animation"
	plAnimStealthNode *stealth = IVerifyStealthPresent( ENTIRE_ANIMATION_NAME );
	goodNodes.Append( stealth );

	// Verify segment nodes
	if( segMap != nil )
	{
		for( SegmentMap::iterator i = segMap->begin(); i != segMap->end(); i++ )
		{
			SegmentSpec *spec = (*i).second;

			if( spec->fType == SegmentSpec::kAnim )
			{
				plAnimStealthNode *stealth = IVerifyStealthPresent( spec->fName );
				goodNodes.Append( stealth );
			}
		}

		DeleteSegmentMap( segMap );
	}

	// Remove nodes that no longer have segments
	int	idx;
	for( idx = 0; idx < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); )
	{
		plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );

		if( node != nil && goodNodes.Find( node ) == goodNodes.kMissingIndex )
		{
			fAnimPB->Delete( (ParamID)kPBAnimStealthNodes, idx, 1 );
//			GetCOREInterface()->DeleteNode( node->GetINode() );
			fStealthsChanged = true;
		}
		else
			idx++;
	}

	if( fStealthsChanged )
	{
		// Yup, our list of stealths got updated. Notify everyone of such.
		for( idx = 0; idx < fChangeCallbacks.GetCount(); idx++ )
			fChangeCallbacks[ idx ]->SegmentListChanged();
	}
}

//// NameChanged /////////////////////////////////////////////////////////////
//	Notify from NTWatcher so we can update the names of our stealth nodes

void	plPassMtlBase::NameChanged( void )
{
	for( int idx = 0; idx < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); idx++ )
	{
		plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
		if( node != nil )
			node->SetNodeName( GetName() );
	}
}

//// NoteTrackAdded/Removed //////////////////////////////////////////////////
//	Notifies from NTWatcher so we can update our list of stealths

void	plPassMtlBase::NoteTrackAdded( void )
{
	int		i;


	// Make a ref to our new notetrack
	for( i = 0; i < NumNoteTracks(); i++ )
	{
		NoteTrack *track = GetNoteTrack( i );

		if( fNotetracks.Find( track ) == fNotetracks.kMissingIndex )
		{
			MakeRefByID( FOREVER, kRefNotetracks + fNotetracks.GetCount(), track );
			break;
		}
	}

	for( i = 0; i < fChangeCallbacks.GetCount(); i++ )
		fChangeCallbacks[ i ]->NoteTrackListChanged();
	IUpdateAnimNodes();
}

void	plPassMtlBase::NoteTrackRemoved( void )
{
	int			i;
	hsBitVector	stillThere;


	// Make a ref to our new notetrack
	for( i = 0; i < NumNoteTracks(); i++ )
	{
		NoteTrack *track = GetNoteTrack( i );

		int idx = fNotetracks.Find( track );
		if( idx != fNotetracks.kMissingIndex )
			stillThere.Set( idx );
	}

	for( i = 0; i < fNotetracks.GetCount(); i++ )
	{
		if( !stillThere.IsBitSet( i ) && fNotetracks[ i ] != nil )
		{
//			DeleteReference( kRefNotetracks + i );
			SetReference( kRefNotetracks + i, nil );
		}
	}

	for( i = 0; i < fChangeCallbacks.GetCount(); i++ )
		fChangeCallbacks[ i ]->NoteTrackListChanged();
	IUpdateAnimNodes();
}


//////////////////////////////////////////////////////////////////////////////
//// MAX Ref Stuff ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// NumRefs /////////////////////////////////////////////////////////////////

int	plPassMtlBase::NumRefs()
{
	return 4 + fNotetracks.GetCount();
}

//// GetReference ////////////////////////////////////////////////////////////

RefTargetHandle plPassMtlBase::GetReference( int i )
{
	if( i >= kRefNotetracks && i < kRefNotetracks + fNotetracks.GetCount() )
		return fNotetracks[ i - kRefNotetracks ];

	return NULL;
}

//// SetReference ////////////////////////////////////////////////////////////

void plPassMtlBase::SetReference(int i, RefTargetHandle rtarg)
{
	if( i >= kRefNotetracks )
	{
		fNotetracks.ExpandAndZero(i - kRefNotetracks + 1);
		fNotetracks[i - kRefNotetracks] = (NoteTrack*)rtarg;
	}
}

//// NotifyRefChanged ////////////////////////////////////////////////////////

RefResult plPassMtlBase::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
										   PartID &partID, RefMessage message ) 
{
	switch( message )
	{
		case REFMSG_CHANGE:
			fIValid.SetEmpty();

			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item
			if (hTarget == fBasicPB || hTarget == fAdvPB || hTarget == fLayersPB || hTarget == fAnimPB)
			{
				IParamBlock2 *pb = (IParamBlock2*)hTarget;

				ParamID changingParam = pb->LastNotifyParamID();
				pb->GetDesc()->InvalidateUI( changingParam );
				
				// And let the SceneWatcher know that the material on some of it's
				// referenced objects changed.
				NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_MAT );
			}
			else 
			{
				// Was it a notetrack ref?
				if( fNotetracks.Find( (NoteTrack *)hTarget ) != fNotetracks.kMissingIndex )
				{
					// Yup, so update our notetrack list
					IUpdateAnimNodes();
				}
			}

			break;

		case REFMSG_TARGET_DELETED:
			NoteTrackRemoved();
			break;

	}

	return REF_SUCCEED;
}

//////////////////////////////////////////////////////////////////////////////
//// Standard IO (or non-standard, as the case may be) ///////////////////////
//////////////////////////////////////////////////////////////////////////////

//// PostLoadAnimPBFixup /////////////////////////////////////////////////////
//	Takes the old version of the anim paramblock and translates it into the
//	new version.
//	Note that there's an interesting (?) side effect of this: for new materials,
//	we'll incorrectly detect them as the "old" format and fix-up them as well.
//	This means that we'll end up with the same defaults for (Entire Animation)
//	that we had for the old materials. We can easily change the defaults by
//	changing the defaults for the old paramblock though.
//	Also, we go ahead and re-work the stealth parent pointers here, since it
//	appears to be the ONLY time we can do it and have it reliably work. ARRGH!
//
//	History of the options that we CAN'T do and why (and hence why this):
//		- ParamBlock accessor doesn't work. For some reason, the accessor isn't
//		  always called on load, even with P_CALLSETS_ON_LOAD specified
//		- Doing it on Load() doesn't work, because neither the ParamBlocks are
//		  guaranteed to be fully loaded (with their tabs filled) nor are the
//		  notetracks necessarily attached yet
//		- Notetracks can also possibly be attached BEFORE load, which doesn't
//		  do us a damned bit of good, so we need to make sure we're fully
//		  loaded before we run this function. Unfortunately, the only time
//		  we're guaranteed THAT is by a FILE_POST_OPEN notify. (post-load
//		  callbacks don't work because they're called right after our object
//		  is loaded but not necessarily before the notetracks are attached)

void	plPassMtlBase::PostLoadAnimPBFixup( void )
{
	SetLoadingFlag( false );

#ifdef MCN_UPGRADE_OLD_ANIM_BLOCKS
	if( fAnimPB->Count( (ParamID)kPBAnimStealthNodes ) == 0 )
	{
		// Yup, old style. So our update process looks like this:
		// 1) Create stealths for all our segments as we are now
		// 2) Set the parameters on all of them to our old defaults (no autostart,
		//    loop on entire, no ease).
		// 3) Copy the old paramblock values to the single stealth indicated by
		//    the old PB

		// Step 1...
		IUpdateAnimNodes();

		// Step 2...
		for( int i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
		{
			plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
		const char *name = node->GetSegmentName();
			node->SetAutoStart( false );
			node->SetLoop( true, ENTIRE_ANIMATION_NAME );
			node->SetEaseIn( plAnimEaseTypes::kNoEase, 1.f, 1.f, 1.f );
			node->SetEaseOut( plAnimEaseTypes::kNoEase, 1.f, 1.f, 1.f );
		}

		// Step 3...
		const char *oldSel = (const char *)fAnimPB->GetStr( (ParamID)kPBAnimName );
		if( oldSel == nil )
			oldSel = ENTIRE_ANIMATION_NAME;
		plAnimStealthNode *myNew = IFindStealth( oldSel );
		if( myNew != nil )
		{
#pragma warning( push )	// Forcing value to bool true or false (go figure, i'm even explicitly casting)
#pragma warning( disable:4800 )	// Forcing value to bool true or false (go figure, i'm even explicitly casting)
			myNew->SetAutoStart( (bool)fAnimPB->GetInt( (ParamID)kPBAnimAutoStart ) );
			myNew->SetLoop( (bool)fAnimPB->GetInt( (ParamID)kPBAnimLoop ),
							(char *)fAnimPB->GetStr( (ParamID)kPBAnimLoopName ) );
			myNew->SetEaseIn( (UInt8)fAnimPB->GetInt( (ParamID)kPBAnimEaseInType ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInLength ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInMin ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInMax ) );
			myNew->SetEaseOut( (UInt8)fAnimPB->GetInt( (ParamID)kPBAnimEaseOutType ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutLength ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutMin ),
								(hsScalar)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutMax ) );
#pragma warning( pop )
		}				
	}
#endif // MCN_UPGRADE_OLD_ANIM_BLOCKS


	// Make sure the parent is set tho. Note: we have to do this because, for some *(#$&(* reason,
	// when we're loading a file, MAX can somehow add the stealths to our tab list WITHOUT calling
	// the accessor for it (and the tab is empty on the CallSetsOnLoad() pass, for some reason).
	for( int i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
	{
		plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
		node->SetParentMtl( this );
	}
}


#define MTL_HDR_CHUNK 0x4000

//// Load ////////////////////////////////////////////////////////////////////
//	Our actual MAX load function

IOResult plPassMtlBase::Load(ILoad *iload)
{
	plPostLoadHandler::AddPostLoad( this );

	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(id = iload->CurChunkID())
		{
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

//// Save ////////////////////////////////////////////////////////////////////
//	The MAX flip-side

IOResult plPassMtlBase::Save(ISave *isave)
{ 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}	

//// ICloneBase //////////////////////////////////////////////////////////////

void	plPassMtlBase::ICloneBase( plPassMtlBase *target, RemapDir &remap )
{
	*((MtlBase*)target) = *((MtlBase*)this); 
	ICloneRefs( target, remap );

	for( int idx = 0; idx < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); idx++ )
	{
		IParamBlock2 *pb = target->fAnimPB;
		plAnimStealthNode *stealth = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
		pb->SetValue( (ParamID)kPBAnimStealthNodes, 0, remap.CloneRef( stealth ), idx );

		stealth = (plAnimStealthNode *)pb->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
		INode *node = GetCOREInterface()->CreateObjectNode( stealth );
		stealth->SetNodeName( GetName() );
		node->Freeze( true );
	}

	BaseClone(this, target, remap);
	target->fIValid.SetEmpty();	
}

//// ConvertToPassMtl ////////////////////////////////////////////////////////
//	Static convert to our plPassMtlBase type, if possible

plPassMtlBase	*plPassMtlBase::ConvertToPassMtl( Mtl *mtl )
{
	if( mtl == nil )
		return nil;

	if( mtl->ClassID() == PASS_MTL_CLASS_ID 
		|| mtl->ClassID() == BUMP_MTL_CLASS_ID
		|| mtl->ClassID() == DECAL_MTL_CLASS_ID )
	{
		return (plPassMtlBase *)mtl;
	}

	return nil;
}

//// SetupProperties /////////////////////////////////////////////////////////

hsBool	plPassMtlBase::SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg )
{
	hsBool ret = true;

	// Call SetupProperties on all our animStealths if we have any
	int i, count = IGetNumStealths();
	for( i = 0; i < count; i++ )
	{
		if( !IGetStealth( i, false )->SetupProperties( node, pErrMsg ) )
			ret = false;
	}

	return ret;
}

//// ConvertDeInit ///////////////////////////////////////////////////////////

hsBool	plPassMtlBase::ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
	hsBool ret = true;

	// Call ConvertDeInit on all our animStealths if we have any
	int i, count = IGetNumStealths();
	for( i = 0; i < count; i++ )
	{
		if( !IGetStealth( i, false )->ConvertDeInit( node, pErrMsg ) )
			ret = false;
	}

	return ret;
}
