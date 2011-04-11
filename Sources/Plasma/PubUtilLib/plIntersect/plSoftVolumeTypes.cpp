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
#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plVolumeIsect.h"
#include "plSoftVolumeTypes.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeSimple::plSoftVolumeSimple()
:	fVolume(nil),
	fSoftDist(0)
{
}

plSoftVolumeSimple::~plSoftVolumeSimple()
{
	delete fVolume;
}

hsScalar plSoftVolumeSimple::IGetStrength(const hsPoint3& pos) const
{
	if( !fVolume || GetProperty(kDisable) )
		return 0;

	hsScalar dist = fVolume->Test(pos);

	if( dist <= 0 )
		return 1.f;

	if( dist >= fSoftDist )
		return 0;

	dist /= fSoftDist;

	return 1.f - dist;
}

void plSoftVolumeSimple::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fVolume )
		fVolume->SetTransform(l2w, w2l);
}

void plSoftVolumeSimple::Read(hsStream* s, hsResMgr* mgr)
{
	plSoftVolume::Read(s, mgr);

	fSoftDist = s->ReadSwapScalar();

	fVolume = plVolumeIsect::ConvertNoRef(mgr->ReadCreatable(s));
}

void plSoftVolumeSimple::Write(hsStream* s, hsResMgr* mgr)
{
	plSoftVolume::Write(s, mgr);

	s->WriteSwapScalar(fSoftDist);

	mgr->WriteCreatable(s, fVolume);
}

void plSoftVolumeSimple::SetVolume(plVolumeIsect* v)
{
	delete fVolume;
	fVolume = v;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeComplex::plSoftVolumeComplex()
{
}

plSoftVolumeComplex::~plSoftVolumeComplex()
{
}

void plSoftVolumeComplex::Read(hsStream* s, hsResMgr* mgr)
{
	plSoftVolume::Read(s, mgr);

	int n = s->ReadSwap32();
	int i;
	for( i = 0; i < n; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kSubVolume), plRefFlags::kActiveRef);
}

void plSoftVolumeComplex::Write(hsStream* s, hsResMgr* mgr)
{
	plSoftVolume::Write(s, mgr);

	s->WriteSwap32(fSubVolumes.GetCount());
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
		mgr->WriteKey(s, fSubVolumes[i]);
}

hsBool plSoftVolumeComplex::MsgReceive(plMessage* msg)
{
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
		{
			plSoftVolume* sub = plSoftVolume::ConvertNoRef(refMsg->GetRef());
			hsAssert(fSubVolumes.kMissingIndex == fSubVolumes.Find(sub), "Adding subvolume I already have");
			fSubVolumes.Append(sub);
		}
		else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
		{
			plSoftVolume* sub = (plSoftVolume*)refMsg->GetRef();
			int idx = fSubVolumes.Find(sub);
			if( idx != fSubVolumes.kMissingIndex )
				fSubVolumes.Remove(idx);
		}
		return true;
	}
	return plSoftVolume::MsgReceive(msg);
}

void plSoftVolumeComplex::UpdateListenerPosition(const hsPoint3& pos)
{
	plSoftVolume::UpdateListenerPosition(pos);
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
		fSubVolumes[i]->UpdateListenerPosition(pos);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeUnion::plSoftVolumeUnion()
{
}

plSoftVolumeUnion::~plSoftVolumeUnion()
{
}

hsScalar plSoftVolumeUnion::IGetStrength(const hsPoint3& pos) const
{
	hsScalar retVal = 0;
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
	{
		hsScalar subRet = fSubVolumes[i]->GetStrength(pos);
		if( subRet >= 1.f )
			return 1.f;
		if( subRet > retVal )
			retVal = subRet;
	}
	return retVal;
}

hsScalar plSoftVolumeUnion::IUpdateListenerStrength() const
{
	hsScalar retVal = 0;
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
	{
		hsScalar subRet = fSubVolumes[i]->GetListenerStrength();
		if( subRet >= 1.f )
		{
			retVal = 1.f;
			break;
		}
		if( subRet > retVal )
			retVal = subRet;
	}
	return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeIntersect::plSoftVolumeIntersect()
{
}

plSoftVolumeIntersect::~plSoftVolumeIntersect()
{
}

hsScalar plSoftVolumeIntersect::IGetStrength(const hsPoint3& pos) const
{
	hsScalar retVal = 1.f;
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
	{
		hsScalar subRet = fSubVolumes[i]->GetStrength(pos);
		if( subRet <= 0 )
			return 0;
		if( subRet < retVal )
			retVal = subRet;
	}
	return retVal;
}

hsScalar plSoftVolumeIntersect::IUpdateListenerStrength() const
{
	hsScalar retVal = 1.f;
	int i;
	for( i = 0; i < fSubVolumes.GetCount(); i++ )
	{
		hsScalar subRet = fSubVolumes[i]->GetListenerStrength();
		if( subRet <= 0 )
		{
			retVal = 0.f;
			break;
		}
		if( subRet < retVal )
			retVal = subRet;
	}
	return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

plSoftVolumeInvert::plSoftVolumeInvert()
{
}

plSoftVolumeInvert::~plSoftVolumeInvert()
{
}

hsScalar plSoftVolumeInvert::IGetStrength(const hsPoint3& pos) const
{
	hsAssert(fSubVolumes.GetCount() <= 1, "Too many subvolumes on inverter");
	if( fSubVolumes.GetCount() )
		return 1.f - fSubVolumes[0]->GetStrength(pos);

	return 1.f;
}

hsScalar plSoftVolumeInvert::IUpdateListenerStrength() const
{
	hsAssert(fSubVolumes.GetCount() <= 1, "Too many subvolumes on inverter");
	hsScalar retVal = 1.f;
	if( fSubVolumes.GetCount() )
		retVal = (1.f - fSubVolumes[0]->GetListenerStrength());

	return fListenStrength = IRemapStrength(retVal);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
