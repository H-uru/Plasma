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

// singular
#include "plMultistageBehMod.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainGeneric.h"

// global
#include "hsResMgr.h"

//other
#include "pnMessage/plNotifyMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plInputCore/plAvatarInputInterface.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plMultistageMsg.h"

plMultistageBehMod::plMultistageBehMod()
    : fStages(), fFreezePhys(), fSmartSeek(), fReverseFBControlsOnRelease(),
      fNetProp(true), fNetForce()
{
}

plMultistageBehMod::plMultistageBehMod(plAnimStageVec* stages,
                                       bool freezePhys,
                                       bool smartSeek,
                                       bool reverseFBControlsOnRelease,
                                       std::vector<plKey>* receivers)
    : fStages(stages),
      fFreezePhys(freezePhys),
      fSmartSeek(smartSeek),
      fReverseFBControlsOnRelease(reverseFBControlsOnRelease),
      fNetProp(true),
      fNetForce(false)
{
    if (receivers)
        fReceivers = *receivers;
}

plMultistageBehMod::~plMultistageBehMod()
{
    IDeleteStageVec();
}

void plMultistageBehMod::Init(plAnimStageVec *stages,
                                      bool freezePhys,
                                      bool smartSeek,
                                      bool reverseFBControlsOnRelease,
                                      std::vector<plKey>* receivers)
{
    fStages = stages;
    fFreezePhys = freezePhys;
    fSmartSeek = smartSeek;
    fReverseFBControlsOnRelease = reverseFBControlsOnRelease;
    if (receivers)
        fReceivers = *receivers;
}

void plMultistageBehMod::IDeleteStageVec()
{
    if (fStages)
    {
        int numStages = fStages->size();
        for (int i = 0; i < numStages; i++)
        {
            plAnimStage* stage = (*fStages)[i];
            delete stage;
        }

        delete fStages;
        fStages = nullptr;
    }
}

bool plMultistageBehMod::MsgReceive(plMessage* msg)
{
    plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(msg);
    if (notifyMsg)
    {
        hsAssert(fStages, "Trying to trigger multistage, but no stages are present.");
        if(fStages)
        {
            const plKey& avKey = notifyMsg->GetAvatarKey();
            hsAssert(avKey, "Avatar key missing trying to trigger multistage.");
            if(avKey)
            {
                plSceneObject *avObj = plSceneObject::ConvertNoRef(avKey->ObjectIsLoaded());
                hsAssert(avObj, "Avatar not loaded when trying to trigger multistage.");
                if(avObj)
                {
                    // Create a copy of our reference anim stages to give to the brain
                    plAnimStageVec* stages = new plAnimStageVec;
                    int numStages = fStages->size();
                    stages->reserve(numStages);
                    // hack hack hack
                    bool ladder = false;
                    for (int i = 0; i < numStages; i++)
                    {
                        plAnimStage* stage = new plAnimStage;
                        *stage = *((*fStages)[i]);
                        stages->push_back(stage);
                        if (stage->GetAnimName().contains("adder"))
                            ladder = true;
                    }

                    const plArmatureMod *avMod = (plArmatureMod*)avObj->GetModifierByType(plArmatureMod::Index());
                    hsAssert(avMod, "Missing armature mod on avatar scene object.");

                    if(avMod)
                    {
                        const plKey& sender = notifyMsg->GetSender();
                        const plKey& avModKey = avMod->GetKey();
                        const plKey& seekKey = GetTarget()->GetKey();      // our seek point

#ifdef DEBUG_MULTISTAGE
                        char sbuf[256];
                        sprintf(sbuf,"plMultistageModMsg - starting multistage from %s",sender->GetName().c_str());
                        plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
                        plAvSeekMsg *seeker = new plAvSeekMsg(nullptr, avModKey, seekKey, 1.0f, fSmartSeek);
                        seeker->Send();

                        // these (currently unused) callbacks are for the brain itself, not any of the stages
                        plMessage *exitCallback = nullptr, *enterCallback = nullptr;
                        uint32_t exitFlags = plAvBrainGeneric::kExitNormal;

                        plAvBrainGeneric *brain = new plAvBrainGeneric(stages, exitCallback, enterCallback, sender, exitFlags, 
                                                                       plAvBrainGeneric::kDefaultFadeIn, plAvBrainGeneric::kDefaultFadeOut, 
                                                                       plAvBrainGeneric::kMoveRelative);
                        if (ladder)
                        {
                            brain->SetType(plAvBrainGeneric::kLadder);
                        }
                        brain->SetReverseFBControlsOnRelease(fReverseFBControlsOnRelease);
                        plAvPushBrainMsg* pushBrain = new plAvPushBrainMsg(GetKey(), avModKey, brain);
                        pushBrain->Send();
                    }
                }
            }
        }

        return true;
    }

    plMultistageModMsg *multiMsg = plMultistageModMsg::ConvertNoRef(msg);
    if (multiMsg)
    {
        if (multiMsg->GetCommand(plMultistageModMsg::kSetLoopCount))
        {
            ((*fStages)[multiMsg->fStageNum])->SetNumLoops(multiMsg->fNumLoops);
        }
        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}

void plMultistageBehMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Read(stream, mgr);

    fFreezePhys = stream->ReadBool();
    fSmartSeek = stream->ReadBool();
    fReverseFBControlsOnRelease = stream->ReadBool();

    IDeleteStageVec();
    fStages = new plAnimStageVec;
    int numStages = stream->ReadLE32();
    fStages->reserve(numStages);

    int i;
    for (i = 0; i < numStages; i++)
    {
        plAnimStage* stage = new plAnimStage;
        stage->Read(stream, mgr);
        stage->SetMod(this);
        fStages->push_back(stage);
    }

    int numReceivers = stream->ReadLE32();
    fReceivers.clear();
    fReceivers.reserve(numReceivers);
    for (i = 0; i < numReceivers; i++)
    {
        plKey key = mgr->ReadKey(stream);
        fReceivers.push_back(key);
    }
}

void plMultistageBehMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Write(stream, mgr);

    stream->WriteBool(fFreezePhys);
    stream->WriteBool(fSmartSeek);
    stream->WriteBool(fReverseFBControlsOnRelease);

    int numStages = fStages->size();
    stream->WriteLE32(numStages);

    int i;
    for (i = 0; i < numStages; i++)
    {
        plAnimStage* stage = (*fStages)[i];
        stage->Write(stream, mgr);
    }

    int numReceivers = fReceivers.size();
    stream->WriteLE32(numReceivers);
    for (i = 0; i < numReceivers; i++)
    {
        plKey key = fReceivers[i];
        mgr->WriteKey(stream, key);
    }
}
