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

#include "plArmatureEffects.h"

#include "plgDispatch.h"
#include "hsResMgr.h"

#include "plArmatureMod.h"

#include "pnMessage/plRefMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAudio/plAudioSystem.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "plStatusLog/plStatusLog.h"

#include "pfAudio/plRandomSoundMod.h"
#include "pfMessage/plArmatureEffectMsg.h"

const char *plArmatureEffectsMgr::SurfaceStrings[] = 
{
    "Dirt",
    "Puddle",
    "Water",
    "Tile",
    "Metal",
    "WoodBridge",
    "RopeLadder",
    "Grass",
    "Brush",
    "HardWood",
    "Rug",
    "Stone",
    "Mud",
    "MetalLadder",
    "WoodLadder",
    "DeepWater",
    "Maintainer(Glass)",
    "Maintainer(Stone)",
    "Swimming",
    "(none)" // Keep this one last
};


void plArmatureEffectsMgr::Read(hsStream *s, hsResMgr *mgr)
{
    hsKeyedObject::Read(s, mgr);

    int numEffects = s->ReadLE32();
    while (numEffects > 0)
    {
        plRefMsg *msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
        hsgResMgr::ResMgr()->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
        numEffects--;
    }
    
    plgDispatch::Dispatch()->RegisterForExactType(plAvatarStealthModeMsg::Index(), GetKey());
}

void plArmatureEffectsMgr::Write(hsStream *s, hsResMgr *mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteLE32((uint32_t)fEffects.size());
    for (plArmatureEffect* effect : fEffects)
        mgr->WriteKey(s, effect->GetKey());
}

bool plArmatureEffectsMgr::MsgReceive(plMessage* msg)
{
    plEventCallbackInterceptMsg *iMsg = plEventCallbackInterceptMsg::ConvertNoRef(msg);
    if (iMsg)
    {
        if (fEnabled)
            iMsg->SendMessageAndKeep();
    }

    plArmatureEffectMsg *eMsg = plArmatureEffectMsg::ConvertNoRef(msg);
    plArmatureEffectStateMsg *sMsg = plArmatureEffectStateMsg::ConvertNoRef(msg);
    if (eMsg || sMsg)
    {
        // Always handle state messages, but only trigger actual effects if we're enabled
        if (sMsg || fEnabled)
        {
            for (plArmatureEffect* effect : fEffects)
                effect->HandleTrigger(msg);
        }

        return true;
    }

    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plArmatureEffect *effect = plArmatureEffect::ConvertNoRef(refMsg->GetRef());
        if (effect)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fEffects.emplace_back(effect);
            else if (refMsg->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove))
            {
                auto idx = std::find(fEffects.cbegin(), fEffects.cend(), effect);
                if (idx != fEffects.cend())
                    fEffects.erase(idx);
            }

            return true;
        }
    }

    plAvatarStealthModeMsg *stealthMsg = plAvatarStealthModeMsg::ConvertNoRef(msg);
    if (stealthMsg && stealthMsg->GetSender() == fArmature->GetTarget(0)->GetKey())
    {
        if (stealthMsg->fMode == plAvatarStealthModeMsg::kStealthCloaked)
            fEnabled = false;
        else
            fEnabled = true;

        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

void plArmatureEffectsMgr::ResetEffects()
{
    for (plArmatureEffect* effect : fEffects)
        effect->Reset();
}

/////////////////////////////////////////////////////////////////////////////
//
//      Actual Effects
//
/////////////////////////////////////////////////////////////////////////////

plArmatureEffectFootSound::plArmatureEffectFootSound()
{
    plArmatureEffectFootSurface *surface = new plArmatureEffectFootSurface;
    surface->fID = plArmatureEffectsMgr::kFootNoSurface;
    surface->fTrigger = nullptr;
    fSurfaces.emplace_back(surface);
    for (int i = 0; i < plArmatureEffectsMgr::kMaxSurface; i++)
    {
        fMods[i] = nullptr;
    }
    SetFootType(kFootTypeShoe);
}

plArmatureEffectFootSound::~plArmatureEffectFootSound() 
{
    for (plArmatureEffectFootSurface* surface : fSurfaces)
        delete surface;
}

void plArmatureEffectFootSound::Read(hsStream* s, hsResMgr* mgr)
{
    plArmatureEffect::Read(s, mgr);

    uint32_t count = s->ReadByte();
    for (uint32_t i = 0; i < count; i++)
    {
        plGenRefMsg *msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, -1);
        mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
    }
}

uint32_t plArmatureEffectFootSound::IFindSurfaceByTrigger(plKey trigger)
{
    // Skip index 0. It's the special "NoSurface" that should always be at the stack bottom
    for (size_t i = 1; i < fSurfaces.size(); i++)
    {
        if (fSurfaces[i]->fTrigger == trigger)
            return i;
    }

    return -1;
}

void plArmatureEffectFootSound::Write(hsStream* s, hsResMgr* mgr)
{
    plArmatureEffect::Write(s, mgr);

    s->WriteByte((uint8_t)plArmatureEffectsMgr::kMaxSurface);
    for (size_t i = 0; i < plArmatureEffectsMgr::kMaxSurface; i++)
        mgr->WriteKey(s, (fMods[i] ? fMods[i]->GetKey() : nullptr));
}

bool plArmatureEffectFootSound::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plRandomSoundMod *rsMod = plRandomSoundMod::ConvertNoRef(refMsg->GetRef());
        if (rsMod)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fMods[refMsg->fWhich] = rsMod;
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )    
                fMods[refMsg->fWhich] = nullptr;

            return true;
        }
    }

    return plArmatureEffect::MsgReceive(msg);
}

bool plArmatureEffectFootSound::HandleTrigger(plMessage* msg)
{
    plArmatureEffectMsg *eMsg = plArmatureEffectMsg::ConvertNoRef(msg);
    if (eMsg)
    {
        uint32_t curSurfaceIndex = fSurfaces.back()->fID;

        if (curSurfaceIndex < plArmatureEffectsMgr::kMaxSurface && fMods[curSurfaceIndex] != nullptr)
        {
            if (plgAudioSys::Active() && fActiveSurfaces.IsBitSet(curSurfaceIndex))
            {
                fMods[curSurfaceIndex]->SetCurrentGroup(eMsg->fTriggerIdx);
                plAnimCmdMsg *animMsg = new plAnimCmdMsg;
                animMsg->AddReceiver(fMods[curSurfaceIndex]->GetKey());
                animMsg->SetCmd(plAnimCmdMsg::kContinue);
                plgDispatch::MsgSend(animMsg);
            }
        }

        return true;
    }

    plArmatureEffectStateMsg *sMsg = plArmatureEffectStateMsg::ConvertNoRef(msg);
    if (sMsg)
    {
        // It doesn't matter if we're adding or removing a surface, our load/unload is the same:
        // unload the old surface and load the new one
        
        if (sMsg->fAddSurface)
        {
            if (IFindSurfaceByTrigger(sMsg->GetSender()) == -1) // Check that it's not a repeat msg
            {
                plStatusLog::AddLineSF("audio.log", "FTSP: Switching to surface - {}", 
                                       plArmatureEffectsMgr::SurfaceStrings[sMsg->fSurface]);
                plArmatureEffectFootSurface *surface = new plArmatureEffectFootSurface;
                surface->fID = sMsg->fSurface;
                surface->fTrigger = sMsg->GetSender();
                fSurfaces.emplace_back(surface);
            }   
        }
        else
        {
            uint32_t index = IFindSurfaceByTrigger(sMsg->GetSender());
            if (index != (uint32_t)-1)
            {
                if (index == fSurfaces.size() - 1) // It's the top on the stack
                    plStatusLog::AddLineSF("audio.log", "FTSP: Switching to surface - {}", 
                                           plArmatureEffectsMgr::SurfaceStrings[fSurfaces[index - 1]->fID]);
                delete fSurfaces[index];
                fSurfaces.erase(fSurfaces.begin() + index);
            }
        }
        return true;
    }

    return false;
}

void plArmatureEffectFootSound::Reset()
{
    while (fSurfaces.size() > 1)
    {
        delete fSurfaces.back();
        fSurfaces.pop_back();
    }
}

void plArmatureEffectFootSound::SetFootType(uint8_t type)
{
    if (type == kFootTypeBare)
    {
        fActiveSurfaces.Clear();
        fActiveSurfaces.SetBit(plArmatureEffectsMgr::kFootDeepWater);
        fActiveSurfaces.SetBit(plArmatureEffectsMgr::kFootPuddle);
        fActiveSurfaces.SetBit(plArmatureEffectsMgr::kFootWater);
        fActiveSurfaces.SetBit(plArmatureEffectsMgr::kFootSwimming);
    }
    else
    {
        fActiveSurfaces.Set(plArmatureEffectsMgr::kMaxSurface);
    }
}
