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

#include "plHardRegionTypes.h"
#include "hsStream.h"
#include "hsResMgr.h"


////////////////////////////////////////////////////////////////////////////////////////
// Base hard and complex
////////////////////////////////////////////////////////////////////////////////////////
plHardRegionComplex::plHardRegionComplex()
{
}

plHardRegionComplex::~plHardRegionComplex()
{
}

void plHardRegionComplex::Read(hsStream* s, hsResMgr* mgr)
{
	plHardRegion::Read(s, mgr);

	int n = s->ReadSwap32();
	int i;
	for( i = 0; i < n; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kSubRegion), plRefFlags::kActiveRef);
}

void plHardRegionComplex::Write(hsStream* s, hsResMgr* mgr)
{
	plHardRegion::Write(s, mgr);

	s->WriteSwap32(fSubRegions.GetCount());
	int i;
	for( i = 0; i < fSubRegions.GetCount(); i++ )
		mgr->WriteKey(s, fSubRegions[i]);
}

hsBool plHardRegionComplex::MsgReceive(plMessage* msg)
{
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
		{
			plHardRegion* sub = plHardRegion::ConvertNoRef(refMsg->GetRef());
			hsAssert(fSubRegions.kMissingIndex == fSubRegions.Find(sub), "Adding subRegion I already have");
			fSubRegions.Append(sub);
		}
		else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
		{
			plHardRegion* sub = (plHardRegion*)refMsg->GetRef();
			int idx = fSubRegions.Find(sub);
			if( idx != fSubRegions.kMissingIndex )
				fSubRegions.Remove(idx);
		}
		return true;
	}
	return plHardRegion::MsgReceive(msg);
}

////////////////////////////////////////////////////////////////////////////////////////
// Booleans follow
////////////////////////////////////////////////////////////////////////////////////////

// Union
////////////////////////////////////////////////////////////////////////////////////////

plHardRegionUnion::plHardRegionUnion()
{
}

plHardRegionUnion::~plHardRegionUnion()
{
}

hsBool plHardRegionUnion::IIsInside(const hsPoint3& pos) const
{
	int i;
	for( i = 0; i < fSubRegions.GetCount(); i++ )
	{
		if( fSubRegions[i]->IIsInside(pos) )
			return true;
	}
	return false;
}

hsBool plHardRegionUnion::ICameraInside() const
{
	if( fState & kDirty )
	{
		fState &= ~(kCamInside | kDirty);
		int i;
		for( i = 0; i < fSubRegions.GetCount(); i++ )
		{
			if( fSubRegions[i]->ICameraInside() )
			{
				fState |= kCamInside;
				return true;
			}
		}
	}
	return false;
}

// Intersection
////////////////////////////////////////////////////////////////////////////////////////
plHardRegionIntersect::plHardRegionIntersect()
{
}

plHardRegionIntersect::~plHardRegionIntersect()
{
}

hsBool plHardRegionIntersect::IIsInside(const hsPoint3& pos) const
{
	int i;
	for( i = 0; i < fSubRegions.GetCount(); i++ )
	{
		if( !fSubRegions[i]->IIsInside(pos) )
			return false;
	}
	return true;
}

hsBool plHardRegionIntersect::ICameraInside() const
{
	if( fState & kDirty )
	{
		fState &= ~kDirty;
		fState |= kCamInside;
		int i;
		for( i = 0; i < fSubRegions.GetCount(); i++ )
		{
			if( !fSubRegions[i]->ICameraInside() )
			{
				fState &= ~kCamInside;
				return false;
			}
		}
	}
	return true;
}


// Invert
////////////////////////////////////////////////////////////////////////////////////////

plHardRegionInvert::plHardRegionInvert()
{
}

plHardRegionInvert::~plHardRegionInvert()
{
}

hsBool plHardRegionInvert::IIsInside(const hsPoint3& pos) const
{
	hsAssert(fSubRegions.GetCount() <= 1, "Too many subRegions on inverter");
	return !fSubRegions[0]->IIsInside(pos);
}

hsBool plHardRegionInvert::ICameraInside() const
{
	hsAssert(fSubRegions.GetCount() <= 1, "Too many subRegions on inverter");
	return !fSubRegions[0]->ICameraInside();
}

	///////////////////////////////////////////////////////////////////////////////
