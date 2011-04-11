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
#include "hsUtils.h"
#include "hsScalar.h"
#include "hsMemory.h"
#include "hsGRenderProcs.h"
#include "hsStream.h"
#include "../plResMgr/plKey.h"
#include "../plResMgr/hsResMgr.h"

hsGRenderProcs::hsGRenderProcs()
: fNext(nil), 
	fBack(nil), 
	fPipeline(nil), 
	fFlags(kNone),
	fLinkCount(0)
{
}

hsGRenderProcs::~hsGRenderProcs()
{
}

hsGRenderProcs** hsGRenderProcs::IOneBeforeMe(hsGRenderProcs** base)
{
	hsAssert(base, "Searching for place in baseless list");

	if( !*base ||((*base)->GetPriority() > GetPriority()) )
		return base;

	hsGRenderProcs* trav = *base;

	while( trav->fNext && (trav->fNext->GetPriority() > GetPriority()) )
		trav = trav->fNext;

	hsAssert((trav != this)&&(trav->fNext != this), "Found self in bad place");
	return &trav->fNext;
}

void hsGRenderProcs::IInsert(hsGRenderProcs** ptr)
{
	hsAssert(*ptr != this, "Re-Inserting self");
	hsAssert(ptr, "Inserting into nil list");
	if( *ptr )
		(*ptr)->fBack = &fNext;
	fNext = *ptr;
	fBack = ptr;
	*ptr = this;
}

void hsGRenderProcs::IDetach()
{
	if( fNext )
		fNext->fBack = fBack;
	*fBack = fNext;

	fNext = nil;
	fBack = nil;
}

void hsGRenderProcs::Enqueue(hsGRenderProcs** base)
{
	// Already linked? Just note another link.
	if( fLinkCount++ )
		return;

	IInsert(IOneBeforeMe(base));
	Ref();
}

void hsGRenderProcs::Dequeue()
{
	if( fBack && !--fLinkCount )
	{
		IDetach();
		UnRef();
	}
}


void hsGRenderProcs::Read(hsStream* s, hsResMgr* mgr)
{
	SetFlags(s->ReadSwap32());
	ReadObjectRefs(s, mgr);
	Read(s);
}

void hsGRenderProcs::ReadObjectRefs(hsStream* s, hsResMgr* mgr)
{
	if( fFlags & kObjectRefs )
	{
		int n = s->ReadSwap32();
		fObjectRefs.SetCount(n);
		int i;
		for( i = 0; i < n; i++ )
		{
			fObjectRefs[i] = mgr->ReadKey(s);
		}
	}
}

void hsGRenderProcs::WriteObjectRefs(hsStream* s, hsResMgr* mgr)
{
	if( fFlags & kObjectRefs )
	{
		s->WriteSwap32(fObjectRefs.GetCount());
		int i;
		for( i = 0; i < fObjectRefs.GetCount(); i++ )
		{
//			if( fObjectRefs[i] )
			{
				mgr->WriteKey(s,fObjectRefs[i]);	// writes nil any...right?
			}
//			else
//			{
//				mgr->WriteKey(s, nil);
//			}
		}
	}
}

void hsGRenderProcs::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap32(fFlags);

	WriteObjectRefs(s, mgr);
	Write(s);
}

plDrawable* hsGRenderProcs::GetObjectRef(int i) 
{ 
	return (plDrawable*)((i < fObjectRefs.GetCount()) && fObjectRefs[i] ? fObjectRefs[i]->GetObjectPtr() : nil); 
}

void hsGRenderProcs::SetNumObjectRefs(int n)
{
	if( n > fObjectRefs.GetCount() )
	{
		int oldCnt = fObjectRefs.GetCount();
		fObjectRefs.SetCount(n);
		int i;
		for( i = oldCnt; i < n; i++ )
			fObjectRefs[i] = nil;
	}
}

void hsGRenderProcs::SetObjectRef(plKey* key, int i) 
{ 
	if( i >= fObjectRefs.GetCount() )
		SetNumObjectRefs(i+1);
	fObjectRefs[i] = key; 
	fFlags |= kObjectRefs; 
}

hsBool32 hsGRenderProcs::BeginTree(plPipeline* pipe, plDrawable* root) 
{ 
	hsAssert(fFlags & kObjectRefsInit, "Should have had refs initialized on read");

	fPipeline = pipe; 

	if( Inclusive() )
	{
		fColorizer.Init(pipe);
		hsColorRGBA col = fColorizer.GetCurrentColor();
		if( !fColorizer.Colorizing() )
		{
			col.r = col.g = col.b = 1.f;
		}
		if( !fColorizer.Alpharizing() )
			col.a = 0.999f;
		fColorizer.PushColorize(col, !fColorizer.Colorizing() /* alpha only */);
	}
	return true; 
}

hsBool32 hsGRenderProcs::BeginObject(plPipeline* pipe, plDrawable* obj) 
{ 
	hsAssert(fFlags & kObjectRefsInit, "Should have had refs initialized on read");

	fPipeline = pipe; 

	if( !Inclusive() )
	{
		fColorizer.Init(pipe);
		hsColorRGBA col = fColorizer.GetCurrentColor();
		if( !fColorizer.Colorizing() )
		{
			col.r = col.g = col.b = 1.f;
		}
		if( !fColorizer.Alpharizing() )
			col.a = 0.999f;
		fColorizer.PushColorize(col, !fColorizer.Colorizing() /* alpha only */);
	}
	return true; 
} 

void hsGRenderProcs::EndObject() 
{ 
	if( !Inclusive() )
		fColorizer.PopColorize();
}

void hsGRenderProcs::EndTree() 
{ 
	if( Inclusive() )
		fColorizer.PopColorize();
	fPipeline = nil; 
}

