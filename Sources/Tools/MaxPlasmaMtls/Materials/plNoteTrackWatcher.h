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

#ifndef _plNoteTrackWatcher_h
#define _plNoteTrackWatcher_h

#include "Max.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "hsTypes.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define NTWATCHER_CLASSID		Class_ID(0x313f531e, 0xa5f132f)

#define REFMSG_NOTETRACK_ADDED	REFMSG_USER + 1

class plPassMtlBase;
class NoteTrack;

//// Class Def ///////////////////////////////////////////////////////////////

class plNoteTrackWatcher : public ReferenceMaker
{
protected:
	plPassMtlBase	*fParentMtl;

	// For tracking notetrack additions to the parent
	int				fNoteTrackCount;

public:

	enum Refs
	{
		kRefParentMtl = 0
	};

	plNoteTrackWatcher( plPassMtlBase *parentMtl );
	virtual ~plNoteTrackWatcher();
	void DeleteThis() { delete this; }

	Class_ID	ClassID()				{ return NTWATCHER_CLASSID; }      
	SClass_ID	SuperClassID()			{ return REF_MAKER_CLASS_ID; }
	
	int				NumRefs();
	RefTargetHandle	GetReference(int i);
	void			SetReference(int i, RefTargetHandle rtarg);
	RefResult		NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message);

	virtual BOOL	IsRealDependency( ReferenceTarget *rtarg );
};

#endif //_plNoteTrackWatcher_h 