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
#include "plFadeOpacityMod.h"

#include "plFadeOpacityLay.h"

#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plMatRefMsg.h"

#include "../plSurface/hsGMaterial.h"

#include "../plDrawable/plVisLOSMgr.h"
#include "../plDrawable/plAccessGeometry.h"
#include "../plDrawable/plAccessSpan.h"
#include "../plDrawable/plDrawableSpans.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"

#include "../plScene/plVisMgr.h"

#include "plgDispatch.h"
#include "plPipeline.h"
#include "hsBounds.h"
#include "hsTimer.h"
#include "hsResMgr.h"

/*
	curr = (t - t0) / fadeUp;
	curr = 1. - (t - t0) / fadeDown;

	switch from fadeUp to fadeDown
	curr = 1 - (t - t0) / fadeDown;
	t - t0 = (1 - curr) * fadeDown
	t0 = t - (1 - curr) * fadeDown;

	switch from fadeDown to fadeUp
	curr = (t - t0) / fadeUp;
	curr * fadeUp = t - t0;
	t0 = t - curr * fadeUp;
*/

hsBool plFadeOpacityMod::fLOSCheckDisabled = false;

const hsScalar kDefFadeUp(5.f);
const hsScalar kDefFadeDown(1.f);

plFadeOpacityMod::plFadeOpacityMod()
:	fFadeUp(kDefFadeUp),
	fFadeDown(kDefFadeDown),
	fOpCurrent(1.f),
	fStart(0),
	fFade(kImmediate),
	fLastEye(0.f, 0.f, 0.f),
	fSetup(false)
{
}

plFadeOpacityMod::~plFadeOpacityMod()
{
}

hsBool plFadeOpacityMod::MsgReceive(plMessage* msg)
{
	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		IOnRenderMsg(rend);
		return true;
	}
	return plSingleModifier::MsgReceive(msg);
}

void plFadeOpacityMod::SetKey(plKey k)
{
	hsKeyedObject::SetKey(k);
	if( k )
	{
		plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	}
}


void plFadeOpacityMod::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	fFadeUp = s->ReadSwapScalar();
	fFadeDown = s->ReadSwapScalar();
}

void plFadeOpacityMod::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);

	s->WriteSwapScalar(fFadeUp);
	s->WriteSwapScalar(fFadeDown);
}

void plFadeOpacityMod::SetTarget(plSceneObject* so)
{
	plSingleModifier::SetTarget(so);

	fSetup = false;
}

hsBool plFadeOpacityMod::IShouldCheck(plPipeline* pipe)
{
	if (pipe->TestVisibleWorld(GetTarget()))
		return true;

	fFade = kImmediate;

	return false;
}

void plFadeOpacityMod::IOnRenderMsg(plRenderMsg* rend)
{
	// Okay, are we set up enough to proceed?
	if( !IReady() )
		return;

	// Okay, we're going to check.

	hsBool hit = false;

	if( !fLOSCheckDisabled )
	{
		// Should we check to see if we're visible before anything else?
		if( !IShouldCheck(rend->Pipeline()) )
			return;

		hsPoint3 eyePos = rend->Pipeline()->GetViewPositionWorld();
		hsPoint3 ourPos = IGetOurPos();

		if( fFade != kImmediate )
		{
			// If we've moved more than 3 feet in a frame, we'll consider this a 
			// camera cut. In that case, don't fade up or down, just go there.
			const hsScalar kCutMagSquared = 3.f * 3.f;
			if( hsVector3(&eyePos, &fLastEye).MagnitudeSquared() > kCutMagSquared )
				fFade = kImmediate;
		}
		fLastEye = eyePos;

		// Cast the ray from eye to us.
		plVisHit hitInfo;
		hit = plVisLOSMgr::Instance()->Check(eyePos, ourPos, hitInfo);
	}

	// If the ray made it, fade us up
	// Else fade us down.
	if( !hit )
		IFadeUp();
	else
		IFadeDown();

	ISetOpacity();
}

hsBool plFadeOpacityMod::IReady()
{
	plSceneObject* so = GetTarget();
	if( !so )
		return false;

	if( !so->GetDrawInterface() )
		return false;

	if( !fSetup )
		ISetup(so);

	if( !fFadeLays.GetCount() )
		return false;

	return true;
}

hsPoint3 plFadeOpacityMod::IGetOurPos()
{
	hsAssert(GetTarget(), "Weed out target-less earlier");

	if( HasFlag(kBoundsCenter) )
	{
		return GetTarget()->GetDrawInterface()->GetWorldBounds().GetCenter();
	}
	else
	{
		return GetTarget()->GetLocalToWorld().GetTranslate();
	}
}

void plFadeOpacityMod::ICalcOpacity()
{
	double t = hsTimer::GetSysSeconds();
	switch( fFade )
	{
	case kFadeUp:
		fOpCurrent = (hsScalar)(t - fStart);
		if( fOpCurrent > fFadeUp )
		{
			fOpCurrent = 1.f;
			fFade = kUp;
		}
		else
		{
			fOpCurrent /= fFadeUp;
		}
		break;
	case kFadeDown:
		fOpCurrent = (hsScalar)(t - fStart);
		if( fOpCurrent > fFadeDown )
		{
			fOpCurrent = 0.f;
			fFade = kDown;
		}
		else
		{
			fOpCurrent = 1.f - fOpCurrent / fFadeDown;
		}
		break;
	case kUp:
	case kDown:
		break;
	case kImmediate:
	default:
		hsAssert(false, "Invalid state");
		break;
	};
}

void plFadeOpacityMod::ISetOpacity()
{
	ICalcOpacity();

	const int num = fFadeLays.GetCount();
	int i;
	for( i = 0; i < num; i++ )
		fFadeLays[i]->SetOpacity(fOpCurrent);	
}

void plFadeOpacityMod::IFadeUp()
{
	const double t = hsTimer::GetSysSeconds();
	switch( fFade )
	{
	case kImmediate:
		fOpCurrent = 1.f;
		fFade = kUp;
		break;
	case kFadeDown:
		{
			fStart = t - fOpCurrent * fFadeUp;

			fFade = kFadeUp;
		}
		break;
	case kDown:
		{
			fStart = t;
			fFade = kFadeUp;
		}
		break;
	case kUp:
	case kFadeUp:
		break;
	default:
		hsAssert(false, "Bad State");
		break;
	};
}

void plFadeOpacityMod::IFadeDown()
{
	const double t = hsTimer::GetSysSeconds();
	switch( fFade )
	{
	case kImmediate:
		fOpCurrent = 0.f;
		fFade = kDown;
		break;
	case kFadeUp:
		{
			fStart = t - (1.f - fOpCurrent) * fFadeDown;

			fFade = kFadeDown;
		}
		break;
	case kUp:
		{
			fStart = t;
			fFade = kFadeDown;
		}
	case kFadeDown:
	case kDown:
		break;
	default:
		hsAssert(false, "Bad State");
		break;
	};
}

void plFadeOpacityMod::ISetup(plSceneObject* so)
{
	fFadeLays.Reset();
	
	if( !so )
		return;

	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return;

	hsTArray<plAccessSpan> src;
	plAccessGeometry::Instance()->OpenRO(di, src, false);

	int i;
	for( i = 0; i < src.GetCount(); i++ )
	{
		hsGMaterial* mat = src[i].GetMaterial();

		int j;
		for( j = 0; j < mat->GetNumLayers(); j++ )
		{
			plLayerInterface* lay = mat->GetLayer(j);
			if( !j || !(lay->GetZFlags() & hsGMatState::kZNoZWrite) || (lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere) )
			{
				plFadeOpacityLay* fade = NEW(plFadeOpacityLay);

				hsgResMgr::ResMgr()->NewKey(lay->GetKey()->GetName(), fade, lay->GetKey()->GetUoid().GetLocation());

				fade->AttachViaNotify(lay);

				// We should add a ref or something here if we're going to hold on to this (even though we created and "own" it).
				fFadeLays.Append(fade);

				plMatRefMsg* msg = NEW(plMatRefMsg)(mat->GetKey(), plRefMsg::kOnReplace, i, plMatRefMsg::kLayer);
				msg->SetOldRef(lay);
				hsgResMgr::ResMgr()->SendRef(fade, msg, plRefFlags::kActiveRef);

				plGenRefMsg* toMe = NEW(plGenRefMsg)(GetKey(), plRefMsg::kOnRequest, 0, kRefFadeLay);
				hsgResMgr::ResMgr()->SendRef(fade, toMe, plRefFlags::kPassiveRef);
			}
		}
	}

	plAccessGeometry::Instance()->Close(src);

	fSetup = true;
	fFade = kImmediate;
}