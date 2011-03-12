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
#include "hsMemory.h"
#include "hsSfxObjDistFade.h"
#include "hsStream.h"
//#include "hsG3DDevice.h"
#include "../plPipeline/plPipeline.h"
#include "../plGeometry/hsTriangle3.h"

#include "../plIntersect/hsBounds.h"
#include "../plDrawable/plDrawable.h"

static hsScalar globalScale = 1.f;

hsSfxObjDistFade::hsSfxObjDistFade()
: fMinDist(0), fMaxDist(0), fTreeCnt(0)
{
}

hsSfxObjDistFade::~hsSfxObjDistFade()
{
}

hsScalar hsSfxObjDistFade::IOpacFromDist(hsScalar dist)
{
	if( dist <= fTable[0].fDistDel )
		return fTable[0].fOpacity;

	int i;
	for( i = 0; (i < fTable.GetCount()) && (dist >= fTable[i].fDistDel); i++ )
		dist -= fTable[i].fDistDel;

	if( i >= fTable.GetCount() )
		return fTable[fTable.GetCount()-1].fOpacity;

	dist *= fTable[i-1].fDistNorm;
	hsScalar opac0 = fTable[i-1].fOpacity;
	hsScalar opac1 = fTable[i].fOpacity;

	return opac0 + dist * (opac1 - opac0);
}

hsBool32 hsSfxObjDistFade::ISetOpac(plDrawable* refObj)
{
	hsPoint3 refPos;
	if( fFlags & kByBoundsCenter )
	{
		const hsBounds3Ext& bnd = refObj->GetWorldBounds();
		if( kBoundsNormal != bnd.GetType() )
			return true;
		refPos = bnd.GetCenter();
	}
	else
	{
		refObj->GetLocalToWorld().GetTranslate(&refPos);
	}
	
	fFlags &= ~(kCulled | kNOP);

	hsPoint3 vPos;
	if( GetObjectRef(1) )
	{
		GetObjectRef(1)->GetLocalToWorld().GetTranslate(&vPos);
	}
	else
	{
		vPos = fPipeline->GetViewPositionWorld();
	}

	hsScalar dist = hsVector3(&vPos, &refPos).Magnitude();

	if( (fFlags & kCullsBefore)
		&&(dist <= fMinDist) )
	{
		fFlags |= kCulled;
		return false;
	}

	if( (fFlags & kCullsBeyond)
		&&(dist > fMaxDist) )
	{
		fFlags |= kCulled;
		return false;
	}

	if( (fFlags & kIdleBefore)
		&&(dist < fMinIdle) )
		fFlags |= kNOP;
	else
	if( (fFlags & kIdleBeyond)
		&&(dist > fMaxIdle) )
		fFlags |= kNOP;
	else
	{
		hsScalar opac = IOpacFromDist(dist);
		hsColorRGBA col = fColorizer.GetCurrentColor();
		if( fColorizer.Alpharizing() )
			col.a *= opac;
		else
			col.a = opac;
		fColorizer.PushColorize(col, fColorizer.Colorizing());

		if( fFlags & kNoZTrans )
		{
			if( !(fPipeline->GetMaterialOverrideOff(hsGMatState::kZ) & hsGMatState::kZNoZWrite) )
			{
				fRestoreOver = fPipeline->PushMaterialOverride(hsGMatState::kZ, hsGMatState::kZNoZWrite, true);
			}
		}
	}

	return true;
}

hsBool32 hsSfxObjDistFade::BeginObject(plPipeline* pipe, plDrawable* obj)
{
	if( Inclusive() )
		return true;

	hsGRenderProcs::BeginObject(pipe, obj);

#if 0
	// This is bogus. We may want to fade something, but not fade it out entirely.
	if( !(fFlags & (kCullsBefore | kCullsBeyond | kIdleBefore | kIdleBeyond)) )
		return true;
#endif

	plDrawable* refObj = fFlags & kObjectRefs ? GetObjectRef(0) : nil;
	if( !refObj )
		refObj = obj;

	return ISetOpac(refObj);
}

hsBool32 hsSfxObjDistFade::BeginTree(plPipeline* pipe, plDrawable* obj)
{
	if( !Inclusive() )
		return true;

	if( fTreeCnt++ )
		return true;

	hsGRenderProcs::BeginTree(pipe, obj);

	plDrawable* refObj = fFlags & kObjectRefs ? GetObjectRef(0) : nil;
	if( !refObj )
		refObj = obj;

	return ISetOpac(refObj);
}

void hsSfxObjDistFade::EndObject()
{
	if( !Inclusive() )
	{
		fPipeline->PopMaterialOverride(fRestoreOver, true);
	}
	hsGRenderProcs::EndObject();
}

void hsSfxObjDistFade::EndTree()
{
	if( Inclusive() )
	{
		fPipeline->PopMaterialOverride(fRestoreOver, true);

		fTreeCnt--;
		hsAssert(fTreeCnt >= 0, "Push/Pop tree problem");
	}
	hsGRenderProcs::EndTree();
}

void hsSfxObjDistFade::MakeTable(float* distList, float* opacList, int num)
{
	fTable.Reset();
	if( !num )
		return;

	int i;
	for( i = 0; i < num; i++ )
	{
		hsSfxDfTableEntry* t = fTable.Append();
		t->fDistDel = distList[i];
		t->fOpacity = opacList[i];
	}
	for( i = num-1; i > 0; i-- )
		fTable[i].fDistDel -= fTable[i-1].fDistDel;
	for( i = 0; i < num-1; i++ )
		fTable[i].fDistNorm = hsScalarInvert(fTable[i+1].fDistDel);
	fTable[num-1].fDistNorm = 0;
	hsAssert(fTable.GetCount() == num, "Mismatch making table");

	if( fTable[0].fOpacity <= 0 )
		fFlags |= kCullsBefore;
	if( fTable[num-1].fOpacity <= 0 )
		fFlags |= kCullsBeyond;
	if( fTable[0].fOpacity >= 1.f )
		fFlags |= kIdleBefore;
	if( fTable[num-1].fOpacity >= 1.f )
		fFlags |= kIdleBeyond;

	int iMin;
	for( iMin = 0; (iMin < fTable.GetCount())&&(fTable[iMin].fOpacity <= 0); iMin++ );
	fMinDist = fTable[0].fDistDel;
	for( i = 1; i < iMin; i++ )
		fMinDist += fTable[i].fDistDel;

	for( iMin = 0; (iMin < fTable.GetCount())&&(fTable[iMin].fOpacity >= 1.f); iMin++ );
	fMinIdle = fTable[0].fDistDel;
	for( i = 1; i < iMin; i++ )
		fMinIdle += fTable[i].fDistDel;

	int iMax;
	for( iMax = fTable.GetCount()-1; (iMax >= 0)&&(fTable[iMax].fOpacity <= 0); iMax-- );
	if( ++iMax >= fTable.GetCount() )
		iMax = fTable.GetCount()-1;
	fMaxDist = fTable[0].fDistDel;
	for( i = 1; i <= iMax; i++ )
		fMaxDist += fTable[i].fDistDel;

	for( iMax = fTable.GetCount()-1; (iMax >= 0)&&(fTable[iMax].fOpacity >= 1.f); iMax-- );
	if( ++iMax >= fTable.GetCount() )
		iMax = fTable.GetCount()-1;
	fMaxIdle = fTable[0].fDistDel;
	for( i = 1; i <= iMax; i++ )
		fMaxIdle += fTable[i].fDistDel;
}

void hsSfxObjDistFade::Read(hsStream* s)
{
	fTable.Reset();

	fMinDist = s->ReadSwapScalar();
	fMaxDist = s->ReadSwapScalar();

	if( fFlags & (kIdleBefore | kIdleBeyond) )
	{
		fMinIdle = s->ReadSwapScalar();
		fMaxIdle = s->ReadSwapScalar();
	}

	Int32 cnt = s->ReadSwap32();

	if( cnt )
	{
		hsSfxDfTableEntry* arr = new hsSfxDfTableEntry[cnt];
		int i;
		for( i = 0; i < cnt; i++ )
		{
			arr[i].fDistDel = s->ReadSwapScalar();
			arr[i].fDistNorm = s->ReadSwapScalar();
			arr[i].fOpacity = s->ReadSwapScalar();
		}

		fTable.SetArray(arr, cnt);
	}
}

void hsSfxObjDistFade::Write(hsStream* s)
{
	s->WriteSwapScalar(fMinDist);
	s->WriteSwapScalar(fMaxDist);

	if( fFlags & (kIdleBefore | kIdleBeyond) )
	{
		s->WriteSwapScalar(fMinIdle);
		s->WriteSwapScalar(fMaxIdle);
	}

	s->WriteSwap32(fTable.GetCount());

	for( fTable.First(); fTable.More(); fTable.Plus() )
	{
		s->WriteSwapScalar(fTable.Current().fDistDel);
		s->WriteSwapScalar(fTable.Current().fDistNorm);
		s->WriteSwapScalar(fTable.Current().fOpacity);
	}
}

