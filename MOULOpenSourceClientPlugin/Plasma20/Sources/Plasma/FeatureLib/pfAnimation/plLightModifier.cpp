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
#include "plLightModifier.h"
#include "../plGLight/plLightInfo.h"
#include "../plInterp/plController.h"

#include "hsStream.h"
#include "hsResMgr.h"

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Basic light
plLightModifier::plLightModifier()
:	fLight(nil),
	fColorCtl(nil),
	fAmbientCtl(nil),
	fSpecularCtl(nil)
{
}

plLightModifier::~plLightModifier()
{
	delete fColorCtl;
	fColorCtl = nil;
	delete fAmbientCtl;
	fAmbientCtl = nil;
	delete fSpecularCtl;
	fSpecularCtl = nil;
}

void plLightModifier::IClearCtls()
{
	delete fColorCtl;
	fColorCtl = nil;
	delete fAmbientCtl;
	fAmbientCtl = nil;
	delete fSpecularCtl;
	fSpecularCtl = nil;
}

void plLightModifier::AddTarget(plSceneObject* so)
{
	plSimpleModifier::AddTarget(so);
	if( so )
		fLight = plLightInfo::ConvertNoRef(so->GetGenericInterface(plLightInfo::Index()));
	else
		fLight = nil;
}

void plLightModifier::RemoveTarget(plSceneObject* so)
{
	if( so = fTarget )
		fLight = nil;
	plSimpleModifier::RemoveTarget(so);
}

void plLightModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plSimpleModifier::Read(s, mgr);

	fColorCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fAmbientCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fSpecularCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
}

void plLightModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plSimpleModifier::Write(s, mgr);

	mgr->WriteCreatable(s, fColorCtl);
	mgr->WriteCreatable(s, fAmbientCtl);
	mgr->WriteCreatable(s, fSpecularCtl);
}

void plLightModifier::IApplyDynamic()
{
	hsColorRGBA col;
	if( fLight != nil )
	{
		if( fColorCtl )
		{
			col.Set(0,0,0,1.f);
			fColorCtl->Interp(fCurrentTime, &col);
			fLight->SetDiffuse(col);
		}
		if( fAmbientCtl )
		{
			col.Set(0,0,0,1.f);
			fAmbientCtl->Interp(fCurrentTime, &col);
			fLight->SetAmbient(col);
		}
		if( fSpecularCtl )
		{
			col.Set(0,0,0,1.f);
			fSpecularCtl->Interp(fCurrentTime, &col);
			fLight->SetSpecular(col);
		}
	}
}

void plLightModifier::DefaultAnimation()
{
	hsScalar len = MaxAnimLength(0);
	fTimeConvert.SetBegin(0);
	fTimeConvert.SetEnd(len);
	fTimeConvert.SetLoopPoints(0, len);
	fTimeConvert.Loop();
	fTimeConvert.Start();
}

hsScalar plLightModifier::MaxAnimLength(hsScalar len) const
{
	if( fColorCtl && (fColorCtl->GetLength() > len) )
		len = fColorCtl->GetLength();
	if( fAmbientCtl && (fAmbientCtl->GetLength() > len) )
		len = fAmbientCtl->GetLength();
	if( fSpecularCtl && (fSpecularCtl->GetLength() > len) )
		len = fSpecularCtl->GetLength();
	return len;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Omni Lights
plOmniModifier::plOmniModifier()
:	fOmni(nil),
	fAttenCtl(nil)
{
}

plOmniModifier::~plOmniModifier()
{
	delete fAttenCtl;
	fAttenCtl = nil;
}

void plOmniModifier::AddTarget(plSceneObject* so)
{
	plLightModifier::AddTarget(so);
	if( fLight )
		fOmni = plOmniLightInfo::ConvertNoRef(fLight);
	else
		fOmni = nil;
}

void plOmniModifier::RemoveTarget(plSceneObject* so)
{
	plLightModifier::RemoveTarget(so);
	if( !fLight )
		fOmni = nil;
}

void plOmniModifier::IClearCtls()
{
	plLightModifier::IClearCtls();

	delete fAttenCtl;
	fAttenCtl = nil;
}

void plOmniModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plLightModifier::Read(s, mgr);

	fAttenCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fInitAtten.Read(s);
}

void plOmniModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plLightModifier::Write(s, mgr);

	mgr->WriteCreatable(s, fAttenCtl);
	fInitAtten.Write(s);
}

void plOmniModifier::IApplyDynamic()
{
	plLightModifier::IApplyDynamic();

	if( fAttenCtl )
	{
		hsPoint3 p = fInitAtten;
		fAttenCtl->Interp(fCurrentTime, &p);
		fOmni->SetConstantAttenuation(p.fX);
		fOmni->SetLinearAttenuation(p.fY);
		fOmni->SetQuadraticAttenuation(p.fZ);
	}
}

hsScalar plOmniModifier::MaxAnimLength(hsScalar len) const
{
	len = plLightModifier::MaxAnimLength(len);
	if( fAttenCtl && (fAttenCtl->GetLength() > len) )
		len = fAttenCtl->GetLength();

	return len;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Spot Lights
plSpotModifier::plSpotModifier()
:	fSpot(nil),
	fInnerCtl(nil),
	fOuterCtl(nil)
{
}

plSpotModifier::~plSpotModifier()
{
	delete fInnerCtl;
	fInnerCtl = nil;
	delete fOuterCtl;
	fOuterCtl = nil;
}

void plSpotModifier::AddTarget(plSceneObject* so)
{
	plOmniModifier::AddTarget(so);
	if( fLight )
		fSpot = plSpotLightInfo::ConvertNoRef(fLight);
	else
		fSpot = nil;
}

void plSpotModifier::RemoveTarget(plSceneObject* so)
{
	plOmniModifier::RemoveTarget(so);
	if( !fLight )
		fSpot = nil;
}

void plSpotModifier::IClearCtls()
{
	plOmniModifier::IClearCtls();

	delete fInnerCtl;
	fInnerCtl = nil;
	delete fOuterCtl;
	fOuterCtl = nil;
}

void plSpotModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plOmniModifier::Read(s, mgr);

	fInnerCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fOuterCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
}

void plSpotModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plOmniModifier::Write(s, mgr);

	mgr->WriteCreatable(s, fInnerCtl);
	mgr->WriteCreatable(s, fOuterCtl);
}

void plSpotModifier::IApplyDynamic()
{
	plOmniModifier::IApplyDynamic();

	hsScalar f;
	if( fInnerCtl )
	{
		fInnerCtl->Interp(fCurrentTime, &f);
		fSpot->SetSpotInner(hsScalarDegToRad(f)*0.5f);
	}
	if( fOuterCtl )
	{
		fOuterCtl->Interp(fCurrentTime, &f);
		fSpot->SetSpotOuter(hsScalarDegToRad(f)*0.5f);
	}
}

hsScalar plSpotModifier::MaxAnimLength(hsScalar len) const
{
	len = plOmniModifier::MaxAnimLength(len);
	if( fInnerCtl && (fInnerCtl->GetLength() > len) )
		len = fInnerCtl->GetLength();
	if( fOuterCtl && (fOuterCtl->GetLength() > len) )
		len = fOuterCtl->GetLength();

	return len;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// LtdDir Lights
plLtdDirModifier::plLtdDirModifier()
:	fLtdDir(nil),
	fWidthCtl(nil),
	fHeightCtl(nil),
	fDepthCtl(nil)
{
}

plLtdDirModifier::~plLtdDirModifier()
{
	delete fWidthCtl;
	fWidthCtl = nil;
	delete fHeightCtl;
	fHeightCtl = nil;
	delete fDepthCtl;
	fDepthCtl = nil;
}

void plLtdDirModifier::AddTarget(plSceneObject* so)
{
	plLightModifier::AddTarget(so);
	if( fLight )
		fLtdDir = plLimitedDirLightInfo::ConvertNoRef(fLight);
	else
		fLtdDir = nil;
}

void plLtdDirModifier::RemoveTarget(plSceneObject* so)
{
	plLightModifier::RemoveTarget(so);
	if( !fLight )
		fLtdDir = nil;
}

void plLtdDirModifier::IClearCtls()
{
	plLightModifier::IClearCtls();

	delete fWidthCtl;
	fWidthCtl = nil;
	delete fHeightCtl;
	fHeightCtl = nil;
	delete fDepthCtl;
	fDepthCtl = nil;
}

void plLtdDirModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plLightModifier::Read(s, mgr);

	fWidthCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fHeightCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fDepthCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
}

void plLtdDirModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plLightModifier::Write(s, mgr);

	mgr->WriteCreatable(s, fWidthCtl);
	mgr->WriteCreatable(s, fHeightCtl);
	mgr->WriteCreatable(s, fDepthCtl);
}

void plLtdDirModifier::IApplyDynamic()
{
	plLightModifier::IApplyDynamic();

	hsScalar f;
	if( fWidthCtl )
	{
		fWidthCtl->Interp(fCurrentTime, &f);
		fLtdDir->SetWidth(f);
	}
	if( fHeightCtl )
	{
		fHeightCtl->Interp(fCurrentTime, &f);
		fLtdDir->SetHeight(f);
	}
	if( fDepthCtl )
	{
		fDepthCtl->Interp(fCurrentTime, &f);
		fLtdDir->SetDepth(f);
	}
}

hsScalar plLtdDirModifier::MaxAnimLength(hsScalar len) const
{
	len = plLightModifier::MaxAnimLength(len);
	if( fWidthCtl && (fWidthCtl->GetLength() > len) )
		len = fWidthCtl->GetLength();
	if( fHeightCtl && (fHeightCtl->GetLength() > len) )
		len = fHeightCtl->GetLength();
	if( fDepthCtl && (fDepthCtl->GetLength() > len) )
		len = fDepthCtl->GetLength();

	return len;
}
