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
#include "hsSfxObjDistShade.h"
#include "hsStream.h"
#include "../plPipeline/plPipeline.h"
#include "../plGeometry/hsTriangle3.h"

#include "../plIntersect/hsBounds.h"
#include "../plDrawable/plDrawable.h"

static hsScalar globalScale = 1.f;

hsSfxObjDistShade::hsSfxObjDistShade()
: fMinDist(0), fMaxDist(0), fTreeCnt(0)
{
}

hsSfxObjDistShade::~hsSfxObjDistShade()
{
}

hsScalar hsSfxObjDistShade::IShadeFromDist(hsScalar dist)
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

hsBool32 hsSfxObjDistShade::ISetShade(plDrawable* refObj)
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

	hsScalar shade = IShadeFromDist(dist);
	
	hsColorRGBA col = fColorizer.GetCurrentColor();
	if( fColorizer.Colorizing() )
	{
		col.r *= shade;
		col.g *= shade;
		col.b *= shade;
	}
	else
	{
		col.r = shade;
		col.g = shade;
		col.b = shade;
	}
	fColorizer.PushColorize(col, false);

	return true;
}



hsBool32 hsSfxObjDistShade::BeginObject(plPipeline* pipe, plDrawable* obj)
{
	hsGRenderProcs::BeginObject(pipe, obj);

	plDrawable* refObj = fFlags & kObjectRefs ? GetObjectRef(0) : nil;
	if( !refObj )
		refObj = obj;

	return ISetShade(refObj);
}

void hsSfxObjDistShade::MakeTable(float* distList, float* shadeList, int num)
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

void hsSfxObjDistShade::Read(hsStream* s)
{
	fTable.Reset();

	fMinDist = s->ReadSwapScalar();
	fMaxDist = s->ReadSwapScalar();

	Int32 cnt = s->ReadSwap32();

	if( cnt )
	{
		hsSfxDfTableEntry* arr = new hsSfxDfTableEntry[cnt];
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

void hsSfxObjDistShade::Write(hsStream* s)
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

