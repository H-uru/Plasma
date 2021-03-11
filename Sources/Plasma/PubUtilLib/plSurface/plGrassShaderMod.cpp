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
#include "plGrassShaderMod.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "hsGMaterial.h"
#include "plLayer.h"
#include "plShader.h"

#include "pnKeyedObject/plUoid.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plTimeMsg.h"

#include "plMessage/plMatRefMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plLayRefMsg.h"


void plGrassWave::Write(hsStream *s)
{
    s->WriteLEFloat(fDistX);
    s->WriteLEFloat(fDistY);
    s->WriteLEFloat(fDistZ);
    s->WriteLEFloat(fDirX);
    s->WriteLEFloat(fDirY);
    s->WriteLEFloat(fSpeed);
}

void plGrassWave::Read(hsStream *s)
{
    fDistX = s->ReadLEFloat();
    fDistY = s->ReadLEFloat();
    fDistZ = s->ReadLEFloat();
    fDirX = s->ReadLEFloat();
    fDirY = s->ReadLEFloat();
    fSpeed = s->ReadLEFloat();
}

/////////////////////////////////////////////////////////////////////////////////////////////

plGrassShaderMod::~plGrassShaderMod()
{
    plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
    plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
    plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
}

void plGrassShaderMod::ResetWaves()
{
    int i;
    for (i = 0; i < kNumWaves; i++)
    {
        fWaves[i].fDistX = 0.F;
        fWaves[i].fDistY = 0.F;
        fWaves[i].fDistZ = 0.F;
        fWaves[i].fDirX = 0.F;
        fWaves[i].fDirY = 0.F;
        fWaves[i].fSpeed = 0.F;
    }
    RefreshWaves();
}

void plGrassShaderMod::RefreshWaves()
{
    IRefreshWaves(fVShader);
}

void plGrassShaderMod::IRefreshWaves(plShader *vShader)
{
    // Dynamic params, set by artist
    vShader->SetVector(plGrassVS::kWaveDistX, fWaves[0].fDistX, fWaves[1].fDistX, fWaves[2].fDistX, fWaves[3].fDistX);
    vShader->SetVector(plGrassVS::kWaveDistY, fWaves[0].fDistY, fWaves[1].fDistY, fWaves[2].fDistY, fWaves[3].fDistY);
    vShader->SetVector(plGrassVS::kWaveDistZ, fWaves[0].fDistZ, fWaves[1].fDistZ, fWaves[2].fDistZ, fWaves[3].fDistZ);
    vShader->SetVector(plGrassVS::kWaveDirX, fWaves[0].fDirX, fWaves[1].fDirX, fWaves[2].fDirX, fWaves[3].fDirX);
    vShader->SetVector(plGrassVS::kWaveDirY, fWaves[0].fDirY, fWaves[1].fDirY, fWaves[2].fDirY, fWaves[3].fDirY);
    vShader->SetVector(plGrassVS::kWaveSpeed, fWaves[0].fSpeed, fWaves[1].fSpeed, fWaves[2].fSpeed, fWaves[3].fSpeed);
}

void plGrassShaderMod::AddTarget(plSceneObject *object)
{
    fTarget = object;
}

void plGrassShaderMod::RemoveTarget(plSceneObject *object)
{
    fTarget = nullptr;
}

bool plGrassShaderMod::MsgReceive(plMessage *msg)
{
    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        if (refMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
        {
            switch (refMsg->fType)
            {
            case kRefGrassVS:
                fVShader = plShader::ConvertNoRef(refMsg->GetRef());
                break;
            case kRefGrassPS:
                fPShader = plShader::ConvertNoRef(refMsg->GetRef());
                break;
            case kRefMaterial:
                fMaterial = hsGMaterial::ConvertNoRef(refMsg->GetRef());
                break;
            default:
                break;
            }
        }
        else
        {   
            switch (refMsg->fType)
            {
            case kRefGrassVS:
                fVShader = nullptr;
                break;
            case kRefGrassPS:
                fPShader = nullptr;
                break;
            case kRefMaterial:
                fMaterial = nullptr;
                break;
            default:
                break;
            }
        }
        return true;
    }

    plAgeLoadedMsg* ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg);
    if( (ageLoaded && ageLoaded->fLoaded) || plInitialAgeStateLoadedMsg::ConvertNoRef(msg) )
    {
        ISetupShaders();
        return true;
    }

    return plModifier::MsgReceive(msg);
}

void plGrassShaderMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plModifier::Write(stream, mgr);

    mgr->WriteKey(stream, fMaterial ? fMaterial->GetKey() : nullptr);

    int i;
    for (i = 0; i < kNumWaves; i++)
        fWaves[i].Write(stream);
}

void plGrassShaderMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plModifier::Read(stream, mgr);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefMaterial), plRefFlags::kActiveRef);

    int i;
    for (i = 0; i < kNumWaves; i++)
        fWaves[i].Read(stream);

    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
}

bool plGrassShaderMod::IEval(double secs, float del, uint32_t dirty)
{
    if (fVShader)
    {
        fVShader->SetVector(plGrassVS::kAppConsts, float(hsTimer::GetSysSeconds()), 0.f, 0.f, 0.f);
    }

    return true;
}

void plGrassShaderMod::ISetupShaders()
{
    if (!fVShader)
    {
        plShader* vShader = new plShader;
        ST::string buff = ST::format("{}_GrassVS", GetKey()->GetName());
        hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
        vShader->SetIsPixelShader(false);
        vShader->SetInputFormat(1);
        vShader->SetOutputFormat(0);

        vShader->SetNumConsts(plGrassVS::kNumConsts);
        vShader->SetVector(plGrassVS::kNumericConsts, 0.f, 0.5f, 1.f, 2.f);
        vShader->SetVector(plGrassVS::kPiConsts, 1.f / (8.f*hsConstants::pi<float>*4.f*4.f),
                                                 hsConstants::half_pi<float>,
                                                 hsConstants::pi<float>,
                                                 hsConstants::two_pi<float>);
        vShader->SetVector(plGrassVS::kSinConsts, -1.f/6.f, 1.f/120.f, -1.f/5040.f, 1.f/362880.f);

        IRefreshWaves(vShader);

        vShader->SetNumPipeConsts(1);
        vShader->SetPipeConst(0, plPipeConst::kLocalToNDC, plGrassVS::kLocalToNDC);

        vShader->SetDecl(plShaderTable::Decl(plShaderID::vs_GrassShader));
        hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefGrassVS), plRefFlags::kActiveRef);
    }

    if (!fPShader)
    {
        plShader* pShader = new plShader;
        ST::string buff = ST::format("{}_GrassPS", GetKey()->GetName());
        hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
        pShader->SetIsPixelShader(true);
        pShader->SetNumConsts(0);
        pShader->SetInputFormat(0);
        pShader->SetOutputFormat(0);
        pShader->SetDecl(plShaderTable::Decl(plShaderID::ps_GrassShader));
        hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefGrassPS), plRefFlags::kActiveRef);
    }

    plLayer* layer = plLayer::ConvertNoRef(fMaterial->GetLayer(0)->BottomOfStack());
    if (layer && (layer->GetVertexShader() != fVShader))
    {
        plLayRefMsg* refMsg = new plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kVertexShader); 
        hsgResMgr::ResMgr()->SendRef(fVShader->GetKey(), refMsg, plRefFlags::kActiveRef);
    }
    if (layer && (layer->GetPixelShader() != fPShader))
    {
        plLayRefMsg* refMsg = new plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kPixelShader);  
        hsgResMgr::ResMgr()->SendRef(fPShader->GetKey(), refMsg, plRefFlags::kActiveRef);
    }
}
