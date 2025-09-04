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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plLightModifier.h"

#include "HeadSpin.h"
#include "hsMath.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plGLight/plLightInfo.h"
#include "plInterp/plController.h"

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
// Basic light
plLightModifier::plLightModifier()
:   fLight(),
    fColorCtl(),
    fAmbientCtl(),
    fSpecularCtl()
{
}

plLightModifier::~plLightModifier()
{
    delete fColorCtl;
    fColorCtl = nullptr;
    delete fAmbientCtl;
    fAmbientCtl = nullptr;
    delete fSpecularCtl;
    fSpecularCtl = nullptr;
}

void plLightModifier::IClearCtls()
{
    delete fColorCtl;
    fColorCtl = nullptr;
    delete fAmbientCtl;
    fAmbientCtl = nullptr;
    delete fSpecularCtl;
    fSpecularCtl = nullptr;
}

void plLightModifier::AddTarget(plSceneObject* so)
{
    plSimpleModifier::AddTarget(so);
    if( so )
        fLight = plLightInfo::ConvertNoRef(so->GetGenericInterface(plLightInfo::Index()));
    else
        fLight = nullptr;
}

void plLightModifier::RemoveTarget(plSceneObject* so)
{
    if (so == fTarget)
        fLight = nullptr;
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
    if (fLight != nullptr)
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
    float len = MaxAnimLength(0);
    fTimeConvert.SetBegin(0);
    fTimeConvert.SetEnd(len);
    fTimeConvert.SetLoopPoints(0, len);
    fTimeConvert.Loop();
    fTimeConvert.Start();
}

float plLightModifier::MaxAnimLength(float len) const
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
:   fOmni(),
    fAttenCtl()
{
}

plOmniModifier::~plOmniModifier()
{
    delete fAttenCtl;
    fAttenCtl = nullptr;
}

void plOmniModifier::AddTarget(plSceneObject* so)
{
    plLightModifier::AddTarget(so);
    if( fLight )
        fOmni = plOmniLightInfo::ConvertNoRef(fLight);
    else
        fOmni = nullptr;
}

void plOmniModifier::RemoveTarget(plSceneObject* so)
{
    plLightModifier::RemoveTarget(so);
    if( !fLight )
        fOmni = nullptr;
}

void plOmniModifier::IClearCtls()
{
    plLightModifier::IClearCtls();

    delete fAttenCtl;
    fAttenCtl = nullptr;
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

float plOmniModifier::MaxAnimLength(float len) const
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
:   fSpot(),
    fInnerCtl(),
    fOuterCtl()
{
}

plSpotModifier::~plSpotModifier()
{
    delete fInnerCtl;
    fInnerCtl = nullptr;
    delete fOuterCtl;
    fOuterCtl = nullptr;
}

void plSpotModifier::AddTarget(plSceneObject* so)
{
    plOmniModifier::AddTarget(so);
    if( fLight )
        fSpot = plSpotLightInfo::ConvertNoRef(fLight);
    else
        fSpot = nullptr;
}

void plSpotModifier::RemoveTarget(plSceneObject* so)
{
    plOmniModifier::RemoveTarget(so);
    if( !fLight )
        fSpot = nullptr;
}

void plSpotModifier::IClearCtls()
{
    plOmniModifier::IClearCtls();

    delete fInnerCtl;
    fInnerCtl = nullptr;
    delete fOuterCtl;
    fOuterCtl = nullptr;
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

    float f;
    if( fInnerCtl )
    {
        fInnerCtl->Interp(fCurrentTime, &f);
        fSpot->SetSpotInner(hsDegreesToRadians(f)*0.5f);
    }
    if( fOuterCtl )
    {
        fOuterCtl->Interp(fCurrentTime, &f);
        fSpot->SetSpotOuter(hsDegreesToRadians(f)*0.5f);
    }
}

float plSpotModifier::MaxAnimLength(float len) const
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
:   fLtdDir(),
    fWidthCtl(),
    fHeightCtl(),
    fDepthCtl()
{
}

plLtdDirModifier::~plLtdDirModifier()
{
    delete fWidthCtl;
    fWidthCtl = nullptr;
    delete fHeightCtl;
    fHeightCtl = nullptr;
    delete fDepthCtl;
    fDepthCtl = nullptr;
}

void plLtdDirModifier::AddTarget(plSceneObject* so)
{
    plLightModifier::AddTarget(so);
    if( fLight )
        fLtdDir = plLimitedDirLightInfo::ConvertNoRef(fLight);
    else
        fLtdDir = nullptr;
}

void plLtdDirModifier::RemoveTarget(plSceneObject* so)
{
    plLightModifier::RemoveTarget(so);
    if( !fLight )
        fLtdDir = nullptr;
}

void plLtdDirModifier::IClearCtls()
{
    plLightModifier::IClearCtls();

    delete fWidthCtl;
    fWidthCtl = nullptr;
    delete fHeightCtl;
    fHeightCtl = nullptr;
    delete fDepthCtl;
    fDepthCtl = nullptr;
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

    float f;
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

float plLtdDirModifier::MaxAnimLength(float len) const
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
