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
#include "hsSfxGlobalShade.h"
#include "hsStream.h"
//#include "../plPipeline/hsG3DDevice.h"
#include "../plPipeline/plPipeline.h"
#include "../plGLight/hsGProjector3.h"
#include "../plSurface/hsGLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../plDrawable/plDrawable.h"
#include "../plIntersect/hsBounds.h"


void hsSfxGlobalShade::ISetIntensity(hsPoint3& pos)
{
	if( fGSFlags & kFromFog )
		ISetFromFog(pos);
	else
	if( fGSFlags & kFromClear )
		ISetFromClear(pos);
	else
	if( fGSFlags & kFromLights )
		ISetFromLights(pos);

	fIntensity.a = hsMaximum(fIntensity.r, hsMaximum(fIntensity.g, fIntensity.b));
}

void hsSfxGlobalShade::ISetFromClear(hsPoint3& pos)
{
	fIntensity.Set(0,0,0,0);
#if 0		// Taken out 2.26.2001 mcn 'cause it accesses the (now defunct) 3DDevice directly
	hsG3DDevice* dev = fPipeline->Get3DDevice();
	hsGEnvironment* env = dev->GetEnvironment();
	if( env && (env->GetFlags() & hsGEnvironment::kClearColorSet) )
	{
		fIntensity = env->GetClearColor();
	}
#endif
}

void hsSfxGlobalShade::ISetFromFog(hsPoint3& pos)
{
	fIntensity.Set(0,0,0,0);
#if 0		// Taken out 2.26.2001 mcn 'cause it accesses the (now defunct) 3DDevice directly
	hsG3DDevice* dev = fPipeline->Get3DDevice();
	hsGEnvironment* env = dev->GetEnvironment();
	if( env && (env->GetFlags() & hsGEnvironment::kFogColorSet) )
	{
		fIntensity = env->GetFogColor();
	}
#endif
}

void hsSfxGlobalShade::ISetFromLights(hsPoint3& pos)
{
	fIntensity = ISumLights(pos);
}

hsColorRGBA hsSfxGlobalShade::ISumLights(hsPoint3& pos)
{
	hsColorRGBA accum;
	accum.Set(0,0,0,0);

#if 0		// Taken out 2.26.2001 mcn 'cause it accesses the (now defunct) 3DDevice directly
	hsG3DDevice* dev = fPipeline->Get3DDevice();
	for( dev->FirstProjector(); dev->MoreProjectors(); dev->IncProjector() )
	{
		hsGProjector3* proj = dev->CurrProjector();

		if( proj->IsOmni() )
		{
			hsScalar intensity = proj->AttenuatePoint(&pos) * proj->GetIntensity();
			
			if( intensity > 0.f )
			{
				hsColorRGBA col = intensity * proj->GetLightColor();				
				accum += col;
			}
		}
		else
		if( proj->IsPerspective() ) // spot
		{
			hsPoint4 ang;
			UInt32 clips;
			proj->GetNdcPoints(1, &pos, sizeof(pos), &ang, kClipAll, &clips);

			if( !clips
				|| !( proj->IsAttenuated() || proj->AttenuatesAlpha() || (clips & ~kClipYon) )
				)
			{
				hsScalar intensity = proj->AttenuatePoint(&pos) * proj->GetIntensity();
				
				if( intensity > 0.f )
				{
					hsColorRGBA col = intensity * proj->GetLightColor();				
					accum += col;
				}
			}
		}
		else // directional
		{
			hsColorRGBA col = proj->GetIntensity() * proj->GetLightColor();				
			accum += col;
		}
	}
#endif
	return accum;
}

void hsSfxGlobalShade::ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList)
{
	if( fCurrentLayer )
	{
		if( fGSFlags & kAffectDiffuse )
			fCurrentLayer->SetColor(fRestoreColor.r, fRestoreColor.g, fRestoreColor.b, fRestoreColor.a);
		else
			fCurrentLayer->SetAmbientColor(fRestoreColor.r, fRestoreColor.g, fRestoreColor.b, fRestoreColor.a);
	}

#if 0		// Taken out 2.26.2001 mcn 'cause it accesses the (now defunct) 3DDevice directly
	hsG3DDevice* dev = fPipeline->Get3DDevice();
	hsRefCnt_SafeAssign(fCurrentLayer, dev->GetCurrentLayer());
	if( fCurrentLayer )
	{
		fRestoreColor = fGSFlags & kAffectDiffuse ? fCurrentLayer->GetColor() : fCurrentLayer->GetAmbientColor();
		hsColorRGBA col = fAmbient;
		if( fGSFlags & kScalarIntensity )
		{
			col.r += fDiffuse.r * fIntensity.a;
			col.g += fDiffuse.g * fIntensity.a;
			col.b += fDiffuse.b * fIntensity.a;
		}
		else
		{
			col.r += fDiffuse.r * fIntensity.r;
			col.g += fDiffuse.g * fIntensity.g;
			col.b += fDiffuse.b * fIntensity.b;
		}
		if( fGSFlags & kAffectDiffuse )
			fCurrentLayer->SetColor(col.r, col.g, col.b, fRestoreColor.a);
		else
			fCurrentLayer->SetAmbientColor(col.r, col.g, col.b, fRestoreColor.a);
	}
#endif
}

hsBool32 hsSfxGlobalShade::BeginObject(plPipeline* pipe, plDrawable* obj)
{
	hsBool32 retVal = hsGRenderProcs::BeginObject(pipe, obj);

	const hsBounds3Ext& bnd = obj->GetLocalBounds();
	hsPoint3 pos = bnd.GetCenter();
	ISetIntensity(pos);

	return retVal;
}

void hsSfxGlobalShade::EndObject()
{
	hsGRenderProcs::EndObject();
	if( fCurrentLayer )
	{
		if( fGSFlags & kAffectDiffuse )
			fCurrentLayer->SetColor(fRestoreColor.r, fRestoreColor.g, fRestoreColor.b, fRestoreColor.a);
		else
			fCurrentLayer->SetAmbientColor(fRestoreColor.r, fRestoreColor.g, fRestoreColor.b, fRestoreColor.a);
		hsRefCnt_SafeUnRef(fCurrentLayer);
		fCurrentLayer = nil;
	}
}

void hsSfxGlobalShade::Read(hsStream* s)
{
	fGSFlags = s->ReadSwap32();
	fAmbient.Read(s);
	fDiffuse.Read(s);
	if( fGSFlags & kFromLights )
		fGSFlags |= kAffectDiffuse;
}

void hsSfxGlobalShade::Write(hsStream* s)
{
	s->WriteSwap32(fGSFlags);
	fAmbient.Write(s);
	fDiffuse.Write(s);
}

hsSfxGlobalShade::hsSfxGlobalShade()
{
	fCurrentLayer = nil;
	fGSFlags = 0;
	fAmbient.Set(0,0,0,0);
	fDiffuse.Set(1.f,1.f,1.f,1.f);
}

hsSfxGlobalShade::~hsSfxGlobalShade()
{
	hsRefCnt_SafeUnRef(fCurrentLayer); // should be nil anyway unless we're destroyed during processing
}

