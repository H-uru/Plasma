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

#include "HeadSpin.h"
#include "plDynaTorpedoVSMgr.h"
#include "plDynaDecal.h"

#include "plWaveSetBase.h"
#include "plRipVSConsts.h"

#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"

plDynaTorpedoVSMgr::plDynaTorpedoVSMgr()
:   fWaveSetBase()
{
}

plDynaTorpedoVSMgr::~plDynaTorpedoVSMgr()
{
}

size_t plDynaTorpedoVSMgr::INewDecal()
{
    size_t idx = fDecals.size();

    plDynaRippleVS* rip = new plDynaRippleVS();
    rip->fC1U = fInitUVW.fX;
    rip->fC2U = (fInitUVW.fX - fFinalUVW.fX) / (fLifeSpan * fFinalUVW.fX);

    rip->fC1V = fInitUVW.fY;
    rip->fC2V = (fInitUVW.fY - fFinalUVW.fY) / (fLifeSpan * fFinalUVW.fY);

    fDecals.emplace_back(rip);

    return idx;
}

bool plDynaTorpedoVSMgr::IHandleShot(plBulletMsg* bull)
{
    if( !ICheckRTMat() )
        return false;

    return plDynaTorpedoMgr::IHandleShot(bull);
}

bool plDynaTorpedoVSMgr::ICheckRTMat()
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

bool plDynaTorpedoVSMgr::MsgReceive(plMessage* msg)
{
    bool retVal = plDynaTorpedoMgr::MsgReceive(msg);
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
                fWaveSetBase = nullptr;
            return true;
        }
    }
    return false;
}

void plDynaTorpedoVSMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaTorpedoMgr::Read(stream, mgr);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefWaveSetBase), plRefFlags::kPassiveRef);
}

void plDynaTorpedoVSMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaTorpedoMgr::Write(stream, mgr);

    mgr->WriteKey(stream, fWaveSetBase);
}
