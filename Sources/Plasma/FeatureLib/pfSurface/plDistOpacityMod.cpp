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

#include "plDistOpacityMod.h"

#include "plFadeOpacityLay.h"
#include "../plSurface/hsGMaterial.h"

#include "../plDrawable/plAccessGeometry.h"
#include "../plDrawable/plAccessSpan.h"

#include "../plMessage/plMatRefMsg.h"

// If we're tracking the camera
#include "../plMessage/plRenderMsg.h"
#include "plPipeline.h"

// If we're tracking the avater
#include "../plMessage/plAvatarMsg.h"
#include "../plAvatar/plArmatureMod.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsQuat.h"

plDistOpacityMod::plDistOpacityMod()
:	fSetup(false)
{
	fDists[kNearTrans] = 0;
	fDists[kNearOpaq] = 0;
	fDists[kFarOpaq] = 0;
	fDists[kFarTrans] = 0;

	fRefPos.Set(0, 0, 0);
}

plDistOpacityMod::~plDistOpacityMod()
{
}

void plDistOpacityMod::SetKey(plKey k)
{
	plSingleModifier::SetKey(k);

	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plArmatureUpdateMsg::Index(), GetKey());
}


hsScalar plDistOpacityMod::ICalcOpacity(const hsPoint3& targPos, const hsPoint3& refPos) const
{
	hsScalar dist = hsVector3(&targPos, &refPos).Magnitude();

	if( dist > fDists[kFarTrans] )
		return 0;
	if( dist < fDists[kNearTrans] )
		return 0;

	if( dist > fDists[kFarOpaq] )
	{
		dist -= fDists[kFarOpaq];
		dist /= (fDists[kFarTrans] - fDists[kFarOpaq]);
		hsAssert(dist >= 0, "unexpected interpolation param - neg");
		hsAssert(dist <= 1.f, "unexpected interpolation param - > one");

		return 1.f - dist;
	}

	if( dist < fDists[kNearOpaq] )
	{
		dist -= fDists[kNearTrans];
		dist /= (fDists[kNearOpaq] - fDists[kNearTrans]);
		hsAssert(dist >= 0, "unexpected interpolation param - neg");
		hsAssert(dist <= 1.f, "unexpected interpolation param - > one");

		return dist;
	}

	return 1.f;
}

void plDistOpacityMod::ISetOpacity()
{
	if( !GetTarget() )
		return;

	if( !fSetup )
		ISetup();

	hsScalar opacity = ICalcOpacity(GetTarget()->GetLocalToWorld().GetTranslate(), fRefPos);

	const int num = fFadeLays.GetCount();
	int i;
	for( i = 0; i < num; i++ )
		fFadeLays[i]->SetOpacity(opacity);	

}

hsBool plDistOpacityMod::MsgReceive(plMessage* msg)
{
	plArmatureUpdateMsg* arm = plArmatureUpdateMsg::ConvertNoRef(msg);
	if( arm && arm->IsLocal() )
	{
		arm->fArmature->GetPositionAndRotationSim(&fRefPos, nil);

		return true;
	}

	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		if( HasFlag(kTrackCamera) )
			fRefPos = rend->Pipeline()->GetViewPositionWorld();

		ISetOpacity();

		return true;
	}

	plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
	if( ref )
	{
		switch(ref->fType)
		{
		case kRefFadeLay:
			if( ref->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				plFadeOpacityLay* lay = plFadeOpacityLay::ConvertNoRef(ref->GetRef());
				int idx = fFadeLays.Find(lay);
				if( idx != fFadeLays.kMissingIndex )
					fFadeLays.Remove(idx);
			}
			break;
		};
		return true;
	}

	return plSingleModifier::MsgReceive(msg);
}

void plDistOpacityMod::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	int i;
	for( i = 0; i < kNumDists; i++ )
		fDists[i] = s->ReadSwapScalar();

	ICheckDists();

	fSetup = false;
}

void plDistOpacityMod::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);

	int i;
	for( i = 0; i < kNumDists; i++ )
		s->WriteSwapScalar(fDists[i]);
}

void plDistOpacityMod::SetTarget(plSceneObject* so)
{
	plSingleModifier::SetTarget(so);

	fSetup = false;
}

void plDistOpacityMod::SetFarDist(hsScalar opaque, hsScalar transparent)
{
	fDists[kFarOpaq] = opaque;
	fDists[kFarTrans] = transparent;

	ICheckDists();
}

void plDistOpacityMod::SetNearDist(hsScalar transparent, hsScalar opaque)
{
	fDists[kNearOpaq] = opaque;
	fDists[kNearTrans] = transparent;

	ICheckDists();
}

class MatLayer
{
public:
	hsGMaterial*		fMat;
	plLayerInterface*	fLay;
};


void plDistOpacityMod::ISetup()
{
	fFadeLays.Reset();

	plSceneObject* so = GetTarget();
	if( !so )
		return;

	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return;

	hsTArray<MatLayer> todo;

	hsTArray<plAccessSpan> src;
	plAccessGeometry::Instance()->OpenRO(di, src, false);

	// We are guaranteed that each Max object will be given a unique
	// copy of materials and associated layers. But within an object,
	// a given layer may be shared across materials.
	// So we'll build a list of the layers that need a FadeOpacityLay applied,
	// making sure we don't include any layer more than once (strip repeats).
	// This would be grossly inefficient if the numbers involved weren't all
	// very small. So an n^2 search isn't bad if n <= 2.
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
				int k;
				for( k = 0; k < todo.GetCount(); k++ )
				{
					if( lay == todo[k].fLay )
						break;
				}
				if( k == todo.GetCount() )
				{
					MatLayer* push = todo.Push();
					push->fMat = mat;
					push->fLay = lay;
				}
			}
		}
	}

	plAccessGeometry::Instance()->Close(src);

	for( i = 0; i < todo.GetCount(); i++ )
	{
		hsGMaterial* mat = todo[i].fMat;
		plLayerInterface* lay = todo[i].fLay;

		plFadeOpacityLay* fade = TRACKED_NEW plFadeOpacityLay;

		hsgResMgr::ResMgr()->NewKey(lay->GetKey()->GetName(), fade, lay->GetKey()->GetUoid().GetLocation());

		fade->AttachViaNotify(lay);

		// We should add a ref or something here if we're going to hold on to this (even though we created and "own" it).
		fFadeLays.Append(fade);

		plMatRefMsg* msg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnReplace, i, plMatRefMsg::kLayer);
		msg->SetOldRef(lay);
		hsgResMgr::ResMgr()->SendRef(fade, msg, plRefFlags::kActiveRef);

		plGenRefMsg* toMe = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefFadeLay);
		hsgResMgr::ResMgr()->SendRef(fade, toMe, plRefFlags::kPassiveRef);
	}

	fSetup = true;
}