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
#include "hsSfxAngleFade.h"
#include "hsStream.h"
#include "../plPipeline/plPipeline.h"
//#include "../plPipeline/hsG3DDevice.h"
#include "../plGeometry/hsTriangle3.h"
#include "../plMath/hsFastMath.h"

hsSfxAngleFade::hsSfxAngleFade()
{
}

hsSfxAngleFade::~hsSfxAngleFade()
{
}

hsScalar hsSfxAngleFade::IOpacFromDot(hsScalar dot)
{
	if( (fFlags & kTwoSided)
		&&(dot < 0) )
		dot = -dot;

	if( dot <= fTable[0].fCosineDel )
		return fTable[0].fOpacity;

	int i;
	for( i = 0; (i < fTable.GetCount()) && (dot >= fTable[i].fCosineDel); i++ )
		dot -= fTable[i].fCosineDel;

	if( i >= fTable.GetCount() )
		return fTable[fTable.GetCount()-1].fOpacity;

	dot *= fTable[i-1].fCosineNorm;
	hsScalar opac0 = fTable[i-1].fOpacity;
	hsScalar opac1 = fTable[i].fOpacity;

	return opac0 + dot * (opac1 - opac0);
}

void hsSfxAngleFade::ProcessPreInterpTris(hsExpander<hsTriangle3*>& tList, hsExpander<hsGTriVertex*>& vList)
{
	if( !(fFlags & kFaceNormals) )
		return;

#if 0		// Taken out 2.26.2001 mcn 'cause it accesses the (now defunct) 3DDevice directly

	hsPoint3 vPos = fPipeline->GetViewPositionLocal();
	hsG3DDevice* dev = fPipeline->Get3DDevice();

	fSetVector.Clear();

	for( tList.First(); tList.More(); tList.Plus() )
	{
		hsTriangle3* tri = tList.Current();
		hsVector3& norm = tri->fNormal;


		hsScalar dot, opac;
		hsGVertex3* vtx;
		hsGShadeVertex* shade;
		hsVector3 vDir;

		vtx = tri->GetVertex(0);
		if( !fSetVector.IsBitSet(vtx->fShadeIdx) )
		{
			vDir.Set(&vPos, &vtx->fLocalPos);
			dot = hsFastMath::InvSqrtAppr(vDir.MagnitudeSquared());
			dot *= norm.InnerProduct(vDir);

			shade = dev->GetShadeEntry(vtx);
			opac = IOpacFromDot(dot);
			shade->fColor.a *= opac;

			fSetVector.SetBit(vtx->fShadeIdx);
		}

		vtx = tri->GetVertex(1);
		if( !fSetVector.IsBitSet(vtx->fShadeIdx) )
		{
			vDir.Set(&vPos, &vtx->fLocalPos);
			dot = hsFastMath::InvSqrtAppr(vDir.MagnitudeSquared());
			dot *= norm.InnerProduct(vDir);

			shade = dev->GetShadeEntry(vtx);
			opac = IOpacFromDot(dot);
			shade->fColor.a *= opac;

			fSetVector.SetBit(vtx->fShadeIdx);
		}

		vtx = tri->GetVertex(2);
		if( !fSetVector.IsBitSet(vtx->fShadeIdx) )
		{
			vDir.Set(&vPos, &vtx->fLocalPos);
			dot = hsFastMath::InvSqrtAppr(vDir.MagnitudeSquared());
			dot *= norm.InnerProduct(vDir);

			shade = dev->GetShadeEntry(vtx);
			opac = IOpacFromDot(dot);
			shade->fColor.a *= opac;

			fSetVector.SetBit(vtx->fShadeIdx);
		}

	}
#endif
}

void hsSfxAngleFade::ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	if( fFlags & kFaceNormals )
		return;

	hsVector3 vDir =fPipeline->GetViewDirLocal();
	hsPoint3 vPos = fPipeline->GetViewPositionLocal();
	
	for( vList.First(); vList.More(); vList.Plus() )
	{
		hsGShadeVertex* shade = vList.Current();

		hsScalar dot;
		if( !(fFlags & kDirectional) )
		{
			vDir.Set(&vPos, &shade->fLocalPos);
			dot = hsFastMath::InvSqrtAppr(vDir.MagnitudeSquared());
			dot *= shade->fNormal.InnerProduct(vDir);
		}
		else
		{
			dot = shade->fNormal.InnerProduct(vDir);
		}

		hsScalar opac = IOpacFromDot(dot);

		shade->fShade.a *= opac;
	}
}

void hsSfxAngleFade::MakeTable(float* cosList, float* opacList, int num)
{
	fTable.Reset();
	if( !num )
		return;

	int i;
	for( i = 0; i < num; i++ )
	{
		hsSfxAfTableEntry* t = fTable.Append();
		t->fCosineDel = cosList[i];
		t->fOpacity = opacList[i];
	}
	for( i = num-1; i > 0; i-- )
		fTable[i].fCosineDel -= fTable[i-1].fCosineDel;
	for( i = 0; i < num-1; i++ )
		fTable[i].fCosineNorm = hsScalarInvert(fTable[i+1].fCosineDel);
	fTable[num-1].fCosineNorm = 0;
	hsAssert(fTable.GetCount() == num, "Mismatch making table");
}

void hsSfxAngleFade::Read(hsStream* s)
{
	fTable.Reset();

	Int32 cnt = s->ReadSwap32();

	if( cnt )
	{
		hsSfxAfTableEntry* arr = new hsSfxAfTableEntry[cnt];
		int i;
		for( i = 0; i < cnt; i++ )
		{
			arr[i].fCosineDel = s->ReadSwapScalar();
			arr[i].fCosineNorm = s->ReadSwapScalar();
			arr[i].fOpacity = s->ReadSwapScalar();
		}

		fTable.SetArray(arr, cnt);
	}
}

void hsSfxAngleFade::Write(hsStream* s)
{
	s->WriteSwap32(fTable.GetCount());

	for( fTable.First(); fTable.More(); fTable.Plus() )
	{
		s->WriteSwapScalar(fTable.Current().fCosineDel);
		s->WriteSwapScalar(fTable.Current().fCosineNorm);
		s->WriteSwapScalar(fTable.Current().fOpacity);
	}
}
