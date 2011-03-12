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
#include "hsSfxDistShade.h"
#include "hsStream.h"
#include "../plGeometry/hsTriangle3.h"

#include "../plIntersect/hsBounds.h"
#include "../plDrawable/plDrawable.h"
#include "../plPipeline/plPipeline.h"

static hsScalar globalScale = 1.f;

hsSfxDistShade::hsSfxDistShade()
: fMinDist(0), fMaxDist(0)
{
}

hsSfxDistShade::~hsSfxDistShade()
{
}

hsScalar hsSfxDistShade::IShadeFromDist(hsScalar dist)
{
	if( dist <= fTable[0].fDistDel )
		return fTable[0].fShade;

	int i;
	for( i = 0; (i < fTable.GetCount()) && (dist >= fTable[i].fDistDel); i++ )
		dist -= fTable[i].fDistDel;

	if( i >= fTable.GetCount() )
		return fTable[fTable.GetCount()-1].fShade;

	dist *= fTable[i-1].fDistNorm;
	hsScalar shade0 = fTable[i-1].fShade;
	hsScalar shade1 = fTable[i].fShade;

	return shade0 + dist * (shade1 - shade0);
}



hsBool32 hsSfxDistShade::BeginObject(plPipeline* pipe, plDrawable* obj)
{
	hsGRenderProcs::BeginObject(pipe, obj);

	fFlags &= ~(kCulled | kNOP);

	hsPoint3 vPos;
	if( GetObjectRef(1) )
	{
		hsPoint3 wPos;
		GetObjectRef(1)->GetLocalToWorld().GetTranslate(&wPos);
		hsMatrix44 w2l = pipe->GetWorldToLocal();
		vPos = w2l * wPos;
	}
	else
	{
		vPos = pipe->GetViewPositionLocal();
	}
	hsVector3 vDir = -pipe->GetViewDirLocal();

	hsScalar scale = 1.f / fPipeline->GetLocalScale();
	scale *= globalScale;

	hsScalar vD = -(vDir.InnerProduct(vPos));
	vD *= scale;

	const hsBounds3Ext& bnd = obj->GetLocalBounds();

	hsPoint3 corner;
	bnd.GetCorner(&corner);
	hsVector3 axis[3];
	bnd.GetAxes(axis+0, axis+1, axis+2);

	hsScalar dist = vDir.InnerProduct(corner) + vD;
	hsScalar minDist = dist;
	hsScalar maxDist = dist;

	int i;
	for( i = 0; i < 3; i++ )
	{
		dist = vDir.InnerProduct(axis[i]);
		if( dist < 0 )
			minDist += dist;
		else
			maxDist += dist;
	}

	minDist *= scale;
	maxDist *= scale;

	fFlags &= ~kShadeConstant;
	if( maxDist < fMinDist )
	{
		fFlags |= kShadeConstant;
		fConstShade = fTable[0].fShade;
	}
	else if( minDist > fMaxDist )
	{
		fFlags |= kShadeConstant;
		fConstShade = fTable[fTable.GetCount()-1].fShade;
	}
	return true;
}

void hsSfxDistShade::ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	if( fFlags & kShadeConstant )
	{
		IConstShadeVerts(vList);
	}
	else
	{
		ICalcShadeVerts(vList);
	}
}

void hsSfxDistShade::IConstShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	for( vList.First(); vList.More(); vList.Plus() )
	{
		hsGShadeVertex* svtx = vList.Current();
		svtx->fShade.r *= fConstShade;
		svtx->fShade.g *= fConstShade;
		svtx->fShade.b *= fConstShade;
	}
}

void hsSfxDistShade::ICalcShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
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
	hsVector3 vDir = fPipeline->GetViewDirLocal();

	hsScalar vDist = vDir.InnerProduct(vPos);

	hsScalar scale = 1.f / fPipeline->GetLocalScale();
	scale *= globalScale;
	
	for( vList.First(); vList.More(); vList.Plus() )
	{
		hsGShadeVertex* svtx = vList.Current();

		hsScalar dist = -vDir.InnerProduct(svtx->fLocalPos);
		dist += vDist;
		dist *= scale;

		hsScalar shade = IShadeFromDist(dist);

		if( shade > 0 )
		{
			svtx->fShade.r *= shade;
			svtx->fShade.g *= shade;
			svtx->fShade.b *= shade;
		}
		else
		{
			svtx->fShade.r = 0;
			svtx->fShade.g = 0;
			svtx->fShade.b = 0;
		}
	}
}

void hsSfxDistShade::MakeTable(float* distList, float* shadeList, int num)
{
	fTable.Reset();
	if( !num )
		return;

	int i;
	for( i = 0; i < num; i++ )
	{
		hsSfxDfTableEntry* t = fTable.Append();
		t->fDistDel = distList[i];
		t->fShade = shadeList[i];
	}
	for( i = num-1; i > 0; i-- )
		fTable[i].fDistDel -= fTable[i-1].fDistDel;
	for( i = 0; i < num-1; i++ )
		fTable[i].fDistNorm = hsScalarInvert(fTable[i+1].fDistDel);
	fTable[num-1].fDistNorm = 0;
	hsAssert(fTable.GetCount() == num, "Mismatch making table");

	int iMin;
	for( iMin = 0; (iMin < fTable.GetCount())&&(fTable[iMin].fShade <= 0); iMin++ );
	fMinDist = fTable[0].fDistDel;
	for( i = 1; i < iMin; i++ )
		fMinDist += fTable[i].fDistDel;

	int iMax;
	for( iMax = fTable.GetCount()-1; (iMax >= 0)&&(fTable[iMax].fShade <= 0); iMax-- );
	if( ++iMax >= fTable.GetCount() )
		iMax = fTable.GetCount()-1;
	fMaxDist = fTable[0].fDistDel;
	for( i = 1; i <= iMax; i++ )
		fMaxDist += fTable[i].fDistDel;

}

void hsSfxDistShade::Read(hsStream* s)
{
	fTable.Reset();

	fMinDist = s->ReadSwapScalar();
	fMaxDist = s->ReadSwapScalar();

	Int32 cnt = s->ReadSwap32();

	if( cnt )
	{
		hsSfxDfTableEntry* arr = TRACKED_NEW hsSfxDfTableEntry[cnt];
		int i;
		for( i = 0; i < cnt; i++ )
		{
			arr[i].fDistDel = s->ReadSwapScalar();
			arr[i].fDistNorm = s->ReadSwapScalar();
			arr[i].fShade = s->ReadSwapScalar();
		}

		fTable.SetArray(arr, cnt);
	}
}

void hsSfxDistShade::Write(hsStream* s)
{
	s->WriteSwapScalar(fMinDist);
	s->WriteSwapScalar(fMaxDist);

	s->WriteSwap32(fTable.GetCount());

	for( fTable.First(); fTable.More(); fTable.Plus() )
	{
		s->WriteSwapScalar(fTable.Current().fDistDel);
		s->WriteSwapScalar(fTable.Current().fDistNorm);
		s->WriteSwapScalar(fTable.Current().fShade);
	}
}

