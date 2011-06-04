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
#include "plDynaTorpedoVSMgr.h"
#include "plDynaDecal.h"

#include "plWaveSetBase.h"
#include "plRipVSConsts.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "../pnMessage/plRefMsg.h"

plDynaTorpedoVSMgr::plDynaTorpedoVSMgr()
:	fWaveSetBase(nil)
{
}

plDynaTorpedoVSMgr::~plDynaTorpedoVSMgr()
{
}

int plDynaTorpedoVSMgr::INewDecal()
{
	int idx = fDecals.GetCount();

	plDynaRippleVS* rip = TRACKED_NEW plDynaRippleVS();
	rip->fC1U = fInitUVW.fX;
	rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

	rip->fC1V = fInitUVW.fY;
	rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

	fDecals.Append(rip);

	return idx;
}

hsBool plDynaTorpedoVSMgr::IHandleShot(plBulletMsg* bull)
{
	if( !ICheckRTMat() )
		return false;

	return plDynaTorpedoMgr::IHandleShot(bull);
}

hsBool plDynaTorpedoVSMgr::ICheckRTMat()
{
	if( !fMatRTShade )
		return false;

	if( !fWaveSetBase )
		return false;

	if( fMatRTShade->GetLayer(0)->GetVertexShader() )
		return true;

	plRipVSConsts ripConsts = IGetRippleConsts();

	return fWaveSetBase->SetupRippleMat(fMatRTShade, ripConsts);
}

plRipVSConsts plDynaTorpedoVSMgr::IGetRippleConsts() const
{
	plRipVSConsts ripConsts;

	ripConsts.fC1U = fInitUVW.fX;
	ripConsts.fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

	ripConsts.fC1V = fInitUVW.fY;
	ripConsts.fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

	ripConsts.fInitAtten = fInitAtten;
	ripConsts.fLife = fLifeSpan;
	ripConsts.fDecay = fDecayStart;
	ripConsts.fRamp = fRampEnd;

	return ripConsts;
}

hsBool plDynaTorpedoVSMgr::MsgReceive(plMessage* msg)
{
	hsBool retVal = plDynaTorpedoMgr::MsgReceive(msg);
	if( retVal )
		return true;

	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		switch( refMsg->fType )
		{
		case kRefWaveSetBase:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fWaveSetBase = plWaveSetBase::ConvertNoRef(refMsg->GetRef());
			else
				fWaveSetBase = nil;
			return true;
		}
	}
	return false;
}

void plDynaTorpedoVSMgr::Read(hsStream* stream, hsResMgr* mgr)
{
	plDynaTorpedoMgr::Read(stream, mgr);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefWaveSetBase), plRefFlags::kPassiveRef);
}

void plDynaTorpedoVSMgr::Write(hsStream* stream, hsResMgr* mgr)
{
	plDynaTorpedoMgr::Write(stream, mgr);

	mgr->WriteKey(stream, fWaveSetBase);
}
