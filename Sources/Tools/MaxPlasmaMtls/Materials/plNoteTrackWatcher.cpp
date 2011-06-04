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
//	plNoteTrackWatcher - Dummy object to watch for notetrack additions or	//
//						 removals from the main material that owns it.		//
//						 All of this is required because MAX will notify	//
//						 an object's dependents about notetrack actions but	//
//						 NOT the object itself, and the Add/DeleteNoteTrack	//
//						 functions are non-virtual. ARRRGH!					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plNoteTrackWatcher.h"
#include "plPassMtlBase.h"

#include "iparamm2.h"

#include "resource.h"


//// Watcher Class Desc //////////////////////////////////////////////////////

plNoteTrackWatcher::plNoteTrackWatcher( plPassMtlBase *parentMtl ) : fParentMtl(nil)
{
	fNoteTrackCount = parentMtl->NumNoteTracks();
	MakeRefByID( FOREVER, kRefParentMtl, parentMtl );
}

plNoteTrackWatcher::~plNoteTrackWatcher()
{
	if( fParentMtl != nil )
	{
		fParentMtl->fNTWatcher = nil;
		DeleteReference( kRefParentMtl );
	}
	DeleteAllRefsFromMe();
}

BOOL	plNoteTrackWatcher::IsRealDependency( ReferenceTarget *rtarg )
{
	if( rtarg == fParentMtl )
		return false;

	return true;
}

int plNoteTrackWatcher::NumRefs()
{
	return 1;
}

RefTargetHandle plNoteTrackWatcher::GetReference( int i )
{
	if( i == kRefParentMtl )
		return fParentMtl;

	return nil;
}

void plNoteTrackWatcher::SetReference( int i, RefTargetHandle rtarg )
{
	if( i == kRefParentMtl )
		fParentMtl = (plPassMtlBase *)rtarg;
}

RefResult plNoteTrackWatcher::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch( message )
	{
		case REFMSG_SUBANIM_STRUCTURE_CHANGED:
			if( hTarget == fParentMtl && fParentMtl != nil )
			{
				// Structure of parent material changed--did it gain or lose a notetrack?
				int oldCount = fNoteTrackCount;
				fNoteTrackCount = fParentMtl->NumNoteTracks();
				if( oldCount != fNoteTrackCount )
				{
					// Is it an addition?
					if( fNoteTrackCount > oldCount )
						// Yes, notify parent.
						fParentMtl->NoteTrackAdded();
					else 
						// Deletion, also notify parent
						fParentMtl->NoteTrackRemoved();
				}
			}
			break;

		case REFMSG_NODE_NAMECHANGE:
			if( hTarget == fParentMtl )
			{
				fParentMtl->NameChanged();
			}
			break;

		case REFMSG_TARGET_DELETED:
			fParentMtl = nil;
			break;
	}

	return REF_SUCCEED;
}
