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

#include "plPostEffectMod.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "plDrawable.h"
#include "plPipeline.h"
#include "plProfile.h"
#include "hsResMgr.h"

#include "plPageTreeMgr.h"
#include "plSceneNode.h"
#include "plRenderRequest.h"

#include "pnMessage/plRefMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plRenderRequestMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plPipeline/plRenderTarget.h"

plProfile_CreateTimer("PostEffect", "RenderSetup", PostEffect);

plPostEffectMod::plPostEffectMod()
:   fHither(1.f),
    fYon(100.f),
    fFovX(hsConstants::pi<float> * 0.25f),
    fFovY(hsConstants::pi<float> * 0.25f * 0.75f),
    fPageMgr(),
    fRenderTarget(),
    fRenderRequest()
{
    fDefaultW2C = hsMatrix44::IdentityMatrix();
    fDefaultC2W = hsMatrix44::IdentityMatrix();

    ISetupRenderRequest();
}

plPostEffectMod::~plPostEffectMod()
{
    IDestroyRenderRequest();
}

void plPostEffectMod::ISetupRenderRequest()
{
    fRenderRequest = new plRenderRequest;
    uint32_t renderState = plPipeline::kRenderNormal
        | plPipeline::kRenderNoProjection
        | plPipeline::kRenderNoLights
        | plPipeline::kRenderClearDepth;
    fRenderRequest->SetRenderState(renderState);

    fRenderRequest->SetDrawableMask(plDrawable::kNormal);
    fRenderRequest->SetSubDrawableMask(plDrawable::kSubAllTypes);

    fRenderRequest->SetPerspective();

    fRenderRequest->SetRenderTarget(fRenderTarget);

    fPageMgr = new plPageTreeMgr;

    fRenderRequest->SetPageTreeMgr(fPageMgr);

    fRenderRequest->SetPriority(1.f);

    IUpdateRenderRequest();
}
void        plPostEffectMod::EnableLightsOnRenderRequest()
{
    fRenderRequest->SetRenderState( fRenderRequest->GetRenderState() & ~plPipeline::kRenderNoLights );
}

void plPostEffectMod::IDestroyRenderRequest()
{
    delete fRenderTarget;
    fRenderTarget = nullptr;
    delete fRenderRequest;
    fRenderRequest = nullptr;
    delete fPageMgr;
    fPageMgr = nullptr;
}

void plPostEffectMod::IRegisterForRenderMsg(bool on)
{
    if( on )
        plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
    else
        plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plPostEffectMod::ISetEnable(bool on)
{
    if( on )
    {
        IRegisterForRenderMsg(true);
        fState.SetBit(kEnabled);
    }
    else
    {
        IRegisterForRenderMsg(false);
        fState.ClearBit(kEnabled);
    }
}

bool plPostEffectMod::IIsEnabled() const
{
    return /*GetTarget() &&*/ !fPageMgr->Empty() && fState.IsBitSet(kEnabled);
}

bool plPostEffectMod::IEval(double secs, float del, uint32_t dirty)
{
    return false;
}

void plPostEffectMod::IUpdateRenderRequest()
{
    fRenderRequest->SetHither(fHither); 
    fRenderRequest->SetYon(fYon); 

    fRenderRequest->SetFovX(fFovX);
    fRenderRequest->SetFovY(fFovY);

    if( GetTarget() )
    {
        hsMatrix44 w2c = GetTarget()->GetWorldToLocal();
        hsMatrix44 c2w = GetTarget()->GetLocalToWorld();

        int i;
        for( i = 0; i < 4; i++ )
        {
            w2c.fMap[2][i] *= -1.f;
            c2w.fMap[i][2] *= -1.f;
        }
        w2c.NotIdentity();
        c2w.NotIdentity();

        fRenderRequest->SetCameraTransform(w2c, c2w);
    }
    else
        fRenderRequest->SetCameraTransform( fDefaultW2C, fDefaultC2W );
//      fRenderRequest->SetCameraTransform(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());
}

// If translating from a scene object, send WorldToLocal() and LocalToWorld(), in that order
void    plPostEffectMod::SetWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w )
{
    int i;


    fDefaultW2C = w2c;
    fDefaultC2W = c2w;

    for( i = 0; i < 4; i++ )
    {
        fDefaultW2C.fMap[2][i] *= -1.f;
        fDefaultC2W.fMap[i][2] *= -1.f;
    }
    fDefaultW2C.NotIdentity();
    fDefaultC2W.NotIdentity();
}

void    plPostEffectMod::GetDefaultWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w )
{
    w2c = fDefaultW2C;
    c2w = fDefaultC2W;
}

void plPostEffectMod::ISubmitRequest()
{
    hsAssert(fState.IsBitSet(kEnabled), "Submitting request when not active");

    IUpdateRenderRequest();

    plRenderRequestMsg* req = new plRenderRequestMsg(GetKey(), fRenderRequest);
    plgDispatch::MsgSend(req);
}

void plPostEffectMod::IAddToPageMgr(plSceneNode* node)
{
    fPageMgr->AddNode(node);
}

void plPostEffectMod::IRemoveFromPageMgr(plSceneNode* node)
{
    fPageMgr->RemoveNode(node);
}

bool plPostEffectMod::MsgReceive(plMessage* msg)
{
    plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
    if( rend && IIsEnabled() )
    {
        plProfile_BeginLap(PostEffect, this->GetKey()->GetUoid().GetObjectName());
        ISubmitRequest();
        plProfile_EndLap(PostEffect, this->GetKey()->GetUoid().GetObjectName());

        return true;
    }
    plAnimCmdMsg* anim = plAnimCmdMsg::ConvertNoRef(msg);
    if( anim )
    {
        if( anim->Cmd(plAnimCmdMsg::kContinue) )
            ISetEnable(true);
        else if( anim->Cmd(plAnimCmdMsg::kStop) )
            ISetEnable(false);
        else if( anim->Cmd(plAnimCmdMsg::kToggleState) )
            ISetEnable(!fState.IsBitSet(kEnabled));

        return true;
    }
    plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
    if( ref )
    {
        switch( ref->fType )
        {
        case kNodeRef:
            if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
            {
                IAddToPageMgr(plSceneNode::ConvertNoRef(ref->GetRef()));
            }
            else if( ref->GetContext() & plRefMsg::kOnReplace )
            {
                IRemoveFromPageMgr(plSceneNode::ConvertNoRef(ref->GetOldRef()));
                IAddToPageMgr(plSceneNode::ConvertNoRef(ref->GetRef()));
            }
            else if( ref->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
            {
                IRemoveFromPageMgr(plSceneNode::ConvertNoRef(ref->GetRef()));
            }
            break;
        }
        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}

void plPostEffectMod::Read(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Read(s, mgr);

    fState.Read(s);

#if 0 // FORCE ENABLE ON LOAD - ONLY FOR DEBUGGING
    ISetEnable(true);
#endif // FORCE ENABLE ON LOAD - ONLY FOR DEBUGGING
    
    fHither = s->ReadLEFloat();
    fYon = s->ReadLEFloat();
    fFovX = s->ReadLEFloat();
    fFovY = s->ReadLEFloat();

    fNodeKey = mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kNodeRef), plRefFlags::kPassiveRef);

    fDefaultW2C.Read( s );
    fDefaultC2W.Read( s );

    IUpdateRenderRequest();
}

void plPostEffectMod::Write(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Write(s, mgr);

    fState.Write(s);
    
    s->WriteLEFloat(fHither);
    s->WriteLEFloat(fYon);
    s->WriteLEFloat(fFovX);
    s->WriteLEFloat(fFovY);

    mgr->WriteKey(s, fNodeKey);

    fDefaultW2C.Write( s );
    fDefaultC2W.Write( s );
}

const plViewTransform& plPostEffectMod::GetViewTransform()
{
    IUpdateRenderRequest();
    return fRenderRequest->GetViewTransform();
}
