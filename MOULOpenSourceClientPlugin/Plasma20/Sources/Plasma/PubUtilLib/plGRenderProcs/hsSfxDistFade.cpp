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
#include "hsSfxDistFade.h"
#include "hsStream.h"
#include "../plPipeline/plPipeline.h"
#include "../plGeometry/hsTriangle3.h"
#include "../plDrawable/plDrawable.h"

#include "../plIntersect/hsBounds.h"


static hsScalar globalScale = 1.f;

hsSfxDistFade::hsSfxDistFade()
: fMinDist(0), fMaxDist(0)
{
}

hsSfxDistFade::~hsSfxDistFade()
{
}

hsScalar hsSfxDistFade::IOpacFromDist(hsScalar dist)
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

hsBool32 hsSfxDistFade::BeginObject(plPipeline* pipe, plDrawable* obj)
{
	hsGRenderProcs::BeginObject(pipe, obj);

	fFlags &= ~(kCulled | kNOP);

	hsPoint3 vPos;
	if( GetObjectRef(1) )
	{
		hsPoint3 wPos;
		GetObjectRef(1)->GetLocalToWorld().GetTranslate(&wPos);
		hsMatrix44 w2l = fPipeline->GetWorldToLocal();
		vPos = w2l * wPos;
	}
	else
	{
		vPos = fPipeline->GetViewPositionLocal();
	}

	hsScalar scale = 1.f / fPipeline->GetLocalScale();
	scale *= globalScale;

	const hsBounds3Ext& bnd = obj->GetLocalBounds();

	hsPoint3 inner, outer;
	bnd.ClosestPoint(vPos, inner, outer);

	hsScalar minDist, maxDist;

	minDist = hsVector3(&vPos, &inner).Magnitude();
	maxDist = hsVector3(&vPos, &outer).Magnitude();

	minDist *= scale;
	maxDist *= scale;

	if( (fFlags & kCullsBefore)
		&&(maxDist <= fMinDist) )
	{
		fFlags |= kCulled;
		return false;
	}

	if( (fFlags & kCullsBeyond)
		&&(minDist > fMaxDist) )
	{
		fFlags |= kCulled;
		return false;
	}

	if( (fFlags & kIdleBefore)
		&&(maxDist < fMinIdle) )
		fFlags |= kNOP;

	if( (fFlags & kIdleBeyond)
		&&(minDist > fMaxIdle) )
		fFlags |= kNOP;

	return true;
}

void hsSfxDistFade::ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	if( fFlags & (kPostInterp | kNOP) )
		return;

	hsPoint3 vPos;
	if( GetObjectRef(1) )
	{
		hsPoint3 wPos;
		GetObjectRef(1)->GetLocalToWorld().GetTranslate(&wPos);
		hsMatrix44 w2l = fPipeline->GetWorldToLocal();
		vPos = w2l * wPos;
	}
	else
	{
		vPos = fPipeline->GetViewPositionLocal();
	}
	hsScalar scale = 1.f / fPipeline->GetLocalScale();

	scale *= globalScale;
	
	for( vList.First(); vList.More(); vList.Plus() )
	{
		hsGShadeVertex* shade = vList.Current();

		hsScalar dist = hsVector3(&shade->fLocalPos, &vPos).Magnitude();
		dist *= scale;

		hsScalar opac = IOpacFromDist(dist);

		if( opac > 0 )
			shade->fShade.a *= opac;
		else
		{
			shade->fShade.a = 0;
			shade->fBaseVertex->fFlags |= hsGVertex3::kCulled;
		}
	}
}

void hsSfxDistFade::ProcessPostInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	if( !(fFlags & kPostInterp) )
		return;

	if( fFlags & kNOP )
		return;

	hsPoint3 vPos;
	if( GetObjectRef(1) )
	{
		hsPoint3 wPos;
		GetObjectRef(1)->GetLocalToWorld().GetTranslate(&wPos);
		hsMatrix44 w2l = fPipeline->GetWorldToLocal();
		vPos = w2l * wPos;
	}
	else
	{
		vPos = fPipeline->GetViewPositionLocal();
	}
	hsScalar scale = 1.f / fPipeline->GetLocalScale();
	
	for( vList.First(); vList.More(); vList.Plus() )
	{
		hsGShadeVertex* shade = vList.Current();

		hsScalar dist = hsVector3(&shade->fLocalPos, &vPos).Magnitude();
		dist *= scale;

		hsScalar opac = IOpacFromDist(dist);

		if( opac > 0 )
			shade->fColor.a *= opac;
		else
		{
			shade->fColor.a = 0;
			shade->fBaseVertex->fFlags |= hsGVertex3::kCulled;
		}
	}
}

void hsSfxDistFade::MakeTable(float* distList, float* opacList, int num)
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

void hsSfxDistFade::Read(hsStream* s)
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

void hsSfxDistFade::Write(hsStream* s)
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

