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
#include "plAGMasterMod.h"

// local
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plAGModifier.h"
#include "plMatrixChannel.h"

// global
#include "hsResMgr.h"
#include "plgDispatch.h"

#include <string_theory/format>

// other
#include "plInterp/plAnimEaseTypes.h"
#include "plInterp/plAnimTimeConvert.h"
#include "pnKeyedObject/plMsgForwarder.h"
#include "plMessage/plAnimCmdMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plSDLNotificationMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "plModifier/plAGMasterSDLModifier.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

////////////////
// PLAGMASTERMOD
////////////////
// Coordinates the activities of a bunch of plAGModifiers
// std::map<char *, plAGMasterMod *, stringISorter> plAGMasterMod::fInstances;

// CTOR
plAGMasterMod::plAGMasterMod()
: fTarget(),
  fNeedEval(false),
  fFirstEval(true),
  fAGMasterSDLMod(),
  fNeedCompile(false),
  fIsGrouped(false),
  fIsGroupMaster(false),
  fMsgForwarder()
{
}

// DTOR
plAGMasterMod::~plAGMasterMod()
{
}

void plAGMasterMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plModifier::Write(stream, mgr);

    int length = 0;
    stream->WriteLE32(length); // backwards compatability. Nuke on next format change.
    stream->WriteLE32((uint32_t)fPrivateAnims.size());
    for (plAGAnim* anim : fPrivateAnims)
    {
        mgr->WriteKey(stream, anim->GetKey());
    }
    stream->WriteBool(fIsGrouped);
    stream->WriteBool(fIsGroupMaster);
    if (fIsGroupMaster)
        mgr->WriteKey(stream, fMsgForwarder->GetKey());

    // maybe later... WriteCachedMessages(stream, mgr);
}

void plAGMasterMod::Read(hsStream * stream, hsResMgr *mgr)
{
    plModifier::Read(stream, mgr);

    //////////////////////////////////////////
    int nameLength = stream->ReadLE32();  // Unused. Nuke next format change.
    char *junk = new char[nameLength+1];    //
    stream->Read(nameLength, junk);         //
    junk[nameLength] = 0;                   //
    delete [] junk;                         //
    //////////////////////////////////////////

    int numPrivateAnims = stream->ReadLE32();
    fPrivateAnims.reserve(numPrivateAnims);             // pre-allocate for performance
    int i;
    for (i = 0; i < numPrivateAnims; i++)
    {
        plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPrivateAnim);
        mgr->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
    }
    fIsGrouped = stream->ReadBool();
    fIsGroupMaster = stream->ReadBool();
    if (fIsGroupMaster)
    {
        plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0);
        mgr->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
    }

    // maybe later... ReadCachedMessages(stream, mgr);
}

// ADDTARGET
// Collect all the plAGModifiers from our children and attach private anims.
void plAGMasterMod::AddTarget(plSceneObject * object)
{
    plSynchEnabler p(false);    // turn off dirty tracking while in this function   
    
    fTarget = object;
    int autoIdx = -1;
    int initialIdx = -1;
    int timeIdx = 0;
    int i;

    for (i = 0; i < fPrivateAnims.size(); i++)
    {
        plATCAnim *atcAnim = plATCAnim::ConvertNoRef(fPrivateAnims[i]);
        if (!atcAnim)
            continue;

        if (atcAnim->GetAutoStart())
            autoIdx = i;
        if (atcAnim->GetInitial() != -1)
            initialIdx = i;
        if (atcAnim->GetStart() < fPrivateAnims[timeIdx]->GetStart())
            timeIdx = i;
    }

    int masterIdx;
    if (autoIdx != -1)
        masterIdx = autoIdx;        // If something autostarts, it wins.
    else if (initialIdx != -1)
        masterIdx = initialIdx;     // Otherwise, the fellow with the @initial point wins
    else
        masterIdx = timeIdx;        // Default case: the earliest anim wins

    for (i = 0; i < fPrivateAnims.size(); i++)
    {
        AttachAnimationBlended(fPrivateAnims[i], i == masterIdx ? 1.f : 0.f);
    }

    // Force one eval after we init
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());

    if (!fIsGrouped || fIsGroupMaster)
    {
        // add sdl modifier
        delete fAGMasterSDLMod;
        fAGMasterSDLMod = new plAGMasterSDLModifier;
        object->AddModifier(fAGMasterSDLMod);
    }
}

void plAGMasterMod::RemoveTarget(plSceneObject* o)
{
    hsAssert(o == fTarget, "Removing target I don't have");

    DetachAllAnimations();

    // remove sdl modifier
    if (o)
    {
        if (fAGMasterSDLMod)
            o->RemoveModifier(fAGMasterSDLMod);
    }
    delete fAGMasterSDLMod;
    fAGMasterSDLMod = nullptr;

    fTarget = nullptr;
}

#include "plProfile.h"

plProfile_CreateTimer("ApplyAnimation", "Animation", ApplyAnimation);
plProfile_CreateTimer("  AffineValue", "Animation", AffineValue);
plProfile_CreateTimer("    AffineInterp", "Animation", AffineInterp);
plProfile_CreateTimer("    AffineBlend", "Animation", AffineBlend);
plProfile_CreateTimer("  AffineCompose", "Animation", AffineCompose);
plProfile_CreateTimer("  AffineApplicator", "Animation", MatrixApplicator);
plProfile_CreateTimer("AnimatingPhysicals", "Animation", AnimatingPhysicals);
plProfile_CreateTimer("StoppedAnimPhysicals", "Animation", StoppedAnimPhysicals);

// IEVAL
bool plAGMasterMod::IEval(double secs, float del, uint32_t dirty)
{
    if (fFirstEval)
    {
        int i;
        for (i = 0; i < fAnimInstances.size(); i++)
            fAnimInstances[i]->SearchForGlobals();

        fFirstEval = false;
    }
    ApplyAnimations(secs, del);
    
    // We might get registered for just a single eval. If we don't need to eval anymore, unregister
    if (!fNeedEval) 
        plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());


    return true;
}

// APPLYANIMATIONS
void plAGMasterMod::ApplyAnimations(double time, float elapsed)
{
    plProfile_BeginLap(ApplyAnimation, this->GetKey()->GetUoid().GetObjectName());

    // update any fades
    for (int i = 0; i < fAnimInstances.size(); i++)
    {
        fAnimInstances[i]->ProcessFade(elapsed);
    }
    
    AdvanceAnimsToTime(time);

    plProfile_EndLap(ApplyAnimation,this->GetKey()->GetUoid().GetObjectName());
}

void plAGMasterMod::AdvanceAnimsToTime(double time)
{
    if(fNeedCompile)
        Compile(time);
    
    for(plChannelModMap::iterator j = fChannelMods.begin(); j != fChannelMods.end(); j++)
    {
        plAGModifier *mod = (*j).second;
        mod->Apply(time);
    }
}

void plAGMasterMod::SetNeedCompile(bool needCompile)
{
    fNeedCompile = true;
}

void plAGMasterMod::Compile(double time)
{
    plChannelModMap::iterator end = fChannelMods.end();
    fNeedCompile = false;

    for(plChannelModMap::iterator j = fChannelMods.begin(); j != end; j++)
    {
        plAGModifier *mod = (*j).second;
        plAGApplicator *app = mod->GetApplicator(kAGPinTransform);

        if(app) {
            plAGChannel *channel = app->GetChannel();
            if(channel)
            {
                plMatrixChannel *topChannel = plMatrixChannel::ConvertNoRef(channel);
                if(topChannel)
                    topChannel->Optimize(time);
            }
        }
    }
}

void plAGMasterMod::DumpAniGraph(const char *justThisChannel, bool optimized, double time)
{
    plChannelModMap::iterator end = fChannelMods.end();
    fNeedCompile = false;

    for(plChannelModMap::iterator j = fChannelMods.begin(); j != end; j++)
    {
        plAGModifier *mod = (*j).second;
        if(!justThisChannel || mod->GetChannelName().compare(justThisChannel, ST::case_insensitive) == 0)
        {
            plAGApplicator *app = mod->GetApplicator(kAGPinTransform);

            if(app) {
                plAGChannel *channel = app->GetChannel();
                if(channel)
                {
                    plMatrixChannel *topChannel = plMatrixChannel::ConvertNoRef(channel);
                    if(topChannel)
                    {
                        hsStatusMessage(ST::format("AGModifier: <{}>", mod->GetChannelName()).c_str());
                        topChannel->Dump(1, optimized, time);
                    }
                }
            }
            if(justThisChannel)
                break;
        }
    }
}

// GETCHANNELMOD(name)
// Get the modifier that controls the channel with the given name
plAGModifier * plAGMasterMod::GetChannelMod(const ST::string & name, bool dontCache ) const
{
    plAGModifier * result = nullptr;
    std::map<ST::string, plAGModifier *>::const_iterator i = fChannelMods.find(name);

    if (i != fChannelMods.end()) {
        result = (*i).second;
    } else {
        plSceneObject *SO = GetTarget(0);
        if(SO) {
            result = IFindChannelMod(SO, name);
            if(result && !dontCache) {
                ICacheChannelMod(result);
            }
        }
    }
    return result;
}

// CACHECHANNELMOD
plAGModifier * plAGMasterMod::ICacheChannelMod(plAGModifier *mod) const
{
    plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0);
    hsgResMgr::ResMgr()->SendRef(mod, msg, plRefFlags::kActiveRef);
    
    return mod;
}

// IFINDAGMOD (sceneObject)
// See if there's an ag modifier on this sceneobject.
// Doesn't check for multiples; just returns the first one.
plAGModifier * plAGMasterMod::IFindChannelMod(const plSceneObject *SO, const ST::string &name) const
{
    const plCoordinateInterface * CI = SO->GetCoordinateInterface();

    const plAGModifier * constMod = static_cast<const plAGModifier *>(FindModifierByClass(SO, plAGModifier::Index()));
    plAGModifier * mod = const_cast<plAGModifier *>(constMod);

    if(mod)
    {
        ST::string modName = mod->GetChannelName();
        if(modName.compare(name, ST::case_insensitive) == 0)
            return mod;
    }

    if(CI)
    {
        size_t childCount = CI->GetNumChildren();
        for (size_t i = 0; i < childCount; i++)
        {
            const plSceneObject * subChild = CI->GetChild(i)->GetOwner();
            plAGModifier * mod = IFindChannelMod(subChild, name);

            if(mod)
                return mod;
        }
    }
    return nullptr;
}

// ATTACHANIMATIONBLENDED(anim, blend)
plAGAnimInstance * plAGMasterMod::AttachAnimationBlended(plAGAnim *anim,
                                                         float blendFactor /* = 0 */,
                                                         uint16_t blendPriority /* plAGMedBlendPriority */,
                                                         bool cache /* = false */)
{
    plAGAnimInstance *instance = nullptr;
    plAnimVector::iterator i;
    if(anim)
    {
        fNeedCompile = true;    // need to recompile the graph since we're editing it...
        for (i = fPrivateAnims.begin(); i != fPrivateAnims.end(); i++) 
        {
            if (*i == anim)
                break;
        }
        if (i == fPrivateAnims.end()) // Didn't find it. Ref it!
        {
            plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPublicAnim);
            hsgResMgr::ResMgr()->SendRef(anim, msg, plRefFlags::kActiveRef);
        }
        instance = new plAGAnimInstance(anim, this, blendFactor, blendPriority, cache, false);
        fAnimInstances.push_back(instance);

        plATCAnim *atcAnim = plATCAnim::ConvertNoRef(anim);
        if (atcAnim)
        {
            fATCAnimInstances.push_back(instance);
            ISetupMarkerCallbacks(atcAnim, instance->GetTimeConvert());
        }
        IRegForEval(HasRunningAnims());
    }
    return instance;
}

// ATTACHANIMATIONBLENDED(name, blend)
plAGAnimInstance * plAGMasterMod::AttachAnimationBlended(const ST::string &name, float blendFactor /* = 0 */, uint16_t blendPriority, bool cache /* = false */)
{
    plAGAnimInstance *instance = nullptr;
    plAGAnim *anim = plAGAnim::FindAnim(name);

    if(anim)
    {
        instance = AttachAnimationBlended(anim, blendFactor, blendPriority, cache);
    }
    return instance;
}

void plAGMasterMod::PlaySimpleAnim(const ST::string &name)
{
    plATCAnim *anim = plATCAnim::ConvertNoRef(plAGAnim::FindAnim(name));
    plAGAnimInstance *instance = nullptr;
    if (anim)
    {
        if (FindAnimInstance(name))
            return;

        instance = AttachAnimationBlended(anim, 1.f, (uint16_t)kAGMaxBlendPriority, false);
    }

    if (instance)
    {
        instance->SetLoop(false);
        instance->Start();

        plAGDetachCallbackMsg *msg = new plAGDetachCallbackMsg(GetKey(), plEventCallbackMsg::kStop); 
        msg->SetAnimName(name);
        instance->GetTimeConvert()->AddCallback(msg);
        hsRefCnt_SafeUnRef(msg);
    }
}

// FINDANIMINSTANCE
// Look for an animation instance of the given name on the modifier.
// If we need this to be fast, should make it a map rather than a vector
plAGAnimInstance * plAGMasterMod::FindAnimInstance(const ST::string &name)
{
    plAGAnimInstance *result = nullptr;

    if (!name.empty())
    {
        for (int i = 0; i < fAnimInstances.size(); i++)
        {
            plAGAnimInstance *act = fAnimInstances[i];
            ST::string eachName = act->GetName();

            if( eachName.compare(name, ST::case_insensitive) == 0)
            {
                result = act;
                break;
            }
        }
    }
    return result;
}

// FINDORATTACHINSTANCE
plAGAnimInstance * plAGMasterMod::FindOrAttachInstance(const ST::string &name, float blendFactor)
{
    plAGAnimInstance *result = FindAnimInstance(name);
    if(result)
    {
        // if it's already attached, we need to set the blend
        result->SetBlend(blendFactor);
    } else  {
        result = AttachAnimationBlended(name, blendFactor);
    }
    return result;
}


// GETANIMINSTANCE
plAGAnimInstance * plAGMasterMod::GetAnimInstance(int i)
{
    return fAnimInstances[i];
}

// GETNUMANIMATIONS
int plAGMasterMod::GetNumAnimations()
{
    return fAnimInstances.size();
}

// GETNUMPRIVATEANIMATIONS
int plAGMasterMod::GetNumPrivateAnimations()
{
    return fPrivateAnims.size();
}

int plAGMasterMod::GetNumATCAnimations()
{
    return fATCAnimInstances.size();
}

plAGAnimInstance *plAGMasterMod::GetATCAnimInstance(int i)
{
    return fATCAnimInstances[i];
}

void plAGMasterMod::DetachAllAnimations()
{
    int nInstances = fAnimInstances.size();

    for (int i = nInstances - 1; i >= 0; i--)
    {
        plAGAnimInstance * instance = fAnimInstances[i];
        if(instance)
        {
            DetachAnimation(instance);
            // delete instance;
        }
    }
    fAnimInstances.clear();
    fPrivateAnims.clear();
    fATCAnimInstances.clear();
}

// DETACHANIMATION(plAGAnimInstance *)
void plAGMasterMod::DetachAnimation(plAGAnimInstance *anim)
{
    plInstanceVector::iterator i;
    plAnimVector::iterator j;
    
    fNeedCompile = true;    // need to recompile the graph since we're editing it...

    for ( i = fAnimInstances.begin(); i != fAnimInstances.end(); i++)
    {
        plAGAnimInstance *instance = *i;

        if(instance == anim)
        {
            // DetachAnimation(instance);
            instance->DetachChannels();

            // Need to release it if it's not a private anim
            const plAGAnim *agAnim = instance->GetAnimation();
            for (j = fPrivateAnims.begin(); j != fPrivateAnims.end(); j++) 
            {
                if (*j == agAnim)
                    break;
            }
            if (j == fPrivateAnims.end()) // We didn't find it
                GetKey()->Release(agAnim->GetKey());

            delete instance;
            i = fAnimInstances.erase(i);
            break;
        }
    }
    for ( i = fATCAnimInstances.begin(); i != fATCAnimInstances.end(); i++)
    {
        plAGAnimInstance *instance = *i;

        if(instance == anim)
        {
            i = fATCAnimInstances.erase(i);
            break;
        }
    }
}

// DETACHANIMATION(name)
void plAGMasterMod::DetachAnimation(const ST::string &name)
{
    plAGAnimInstance *anim = FindAnimInstance(name);
    if(anim) {
        DetachAnimation(anim);
    }
}

void plAGMasterMod::DumpCurrentAnims(const char *header)
{
    if(header)
        hsStatusMessage(ST::format("Dumping Armature Anim Stack: {}", header).c_str());
    int nAnims = fAnimInstances.size();
    for(int i = nAnims - 1; i >= 0; i--)
    {
        plAGAnimInstance *inst = fAnimInstances[i];
        ST::string name = inst->GetName();
        float blend = inst->GetBlend();

        hsStatusMessage(ST::format("{}: {} with blend of {}", i, name, blend).c_str());
    }
}

// MSGRECEIVE
// receive trigger messages
bool plAGMasterMod::MsgReceive(plMessage* msg)
{
    plSDLNotificationMsg* nMsg = plSDLNotificationMsg::ConvertNoRef(msg);
    if (nMsg)
    {
        // Force a single eval
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        return true;
    }

    plAnimCmdMsg* cmdMsg = plAnimCmdMsg::ConvertNoRef(msg);
    if (cmdMsg)
    {
        ST::string targetName = cmdMsg->GetAnimName();

        if (targetName.empty())
            targetName = ENTIRE_ANIMATION_NAME;

        plAGAnimInstance *inst = FindAnimInstance(targetName);
        if (inst != nullptr)
        {
            if (cmdMsg->CmdChangesAnimTime())
            {
                for (int i = 0; i < GetNumAnimations(); i++)
                {
                    plAGAnimInstance *currInst = GetAnimInstance(i);
                    if (currInst != inst && currInst->GetAnimation()->SharesPinsWith(inst->GetAnimation()))
                        currInst->SetBlend(0);
                }
                inst->SetBlend(1);
            }

            inst->HandleCmd(cmdMsg);
        }

        return true;
    }

    plAGCmdMsg* agMsg = plAGCmdMsg::ConvertNoRef(msg);
    if (agMsg)
    {
        if (agMsg->Cmd(plAGCmdMsg::kSetAnimTime))
        {
            for (int i = 0; i < fAnimInstances.size(); i++)
            {
                plAGAnimInstance *inst = fAnimInstances[i];
                inst->SetCurrentTime(agMsg->fAnimTime, true);
            }

            return true;
        }

        plAGAnimInstance *inst = FindAnimInstance(agMsg->GetAnimName());
        if (inst != nullptr)
        {
            if (agMsg->Cmd(plAGCmdMsg::kSetBlend))
                inst->Fade(agMsg->fBlend, agMsg->fBlendRate, plAGAnimInstance::kFadeBlend);
            if (agMsg->Cmd(plAGCmdMsg::kSetAmp))
                inst->Fade(agMsg->fAmp, agMsg->fAmpRate, plAGAnimInstance::kFadeAmp);
        }
        return true;
    }

    plAGInstanceCallbackMsg *agicMsg = plAGInstanceCallbackMsg::ConvertNoRef(msg);
    if (agicMsg)
    {
        if (agicMsg->fEvent == plEventCallbackMsg::kStart)
        {
            IRegForEval(true);
        }
        else if (agicMsg->fEvent == plEventCallbackMsg::kStop)
        {
            if (!HasRunningAnims())
                IRegForEval(false);
        }
        else // Just force a single eval
        {
            plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        }
        return true;
    }

    plAGDetachCallbackMsg *detachMsg = plAGDetachCallbackMsg::ConvertNoRef(msg);
    if (detachMsg)
    {
        DetachAnimation(detachMsg->GetAnimName());
    }

    plGenRefMsg *genRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (genRefMsg)
    {
        plAGAnim *anim = plAGAnim::ConvertNoRef(genRefMsg->GetRef());
        if (anim)
        {
            if (genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest))
            {
                if (genRefMsg->fType == kPrivateAnim)
                    fPrivateAnims.push_back(anim);
            }
            else
            {
                if (genRefMsg->fType == kPrivateAnim)
                {
                    plAnimVector::iterator i = fPrivateAnims.begin();
                    for ( ; i != fPrivateAnims.end(); i++)
                    {
                        plAGAnim *currAnim = *i;

                        if(currAnim == anim)
                        {
                            i = fPrivateAnims.erase(i);
                            break;
                        }
                    }
                }
            }

            return true;
        }

        plAGModifier *agmod = plAGModifier::ConvertNoRef(genRefMsg->GetRef());
        if (agmod)
        {
            if (genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest))
                fChannelMods[agmod->GetChannelName()] = agmod;
            else
                fChannelMods.erase(agmod->GetChannelName());

            return true;
        }

        plMsgForwarder *msgfwd = plMsgForwarder::ConvertNoRef(genRefMsg->GetRef());
        if (msgfwd)
        {
            if (genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest))
                fMsgForwarder = msgfwd;
            else
                fMsgForwarder = nullptr;

            return true;
        }
    }

    plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            AddTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));
        else
            RemoveTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));

        return true;
    }

    return plModifier::MsgReceive(msg);
}

void plAGMasterMod::IRegForEval(bool val)
{
    if (fNeedEval == val)
        return;

    fNeedEval = val;
    if (val)
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
    else
        plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
}

bool plAGMasterMod::HasRunningAnims()
{
    int i;
    bool needEval = false;
    for (i = 0; i < fAnimInstances.size(); i++)
    {
        if (!fAnimInstances[i]->IsFinished())
        {
            needEval = true;
            break;
        }
    }
    return needEval;
}

//
// Send SDL sendState msg to object's plAGMasterSDLModifier
//
bool plAGMasterMod::DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags)
{
    if(GetNumTargets() > 0 && (!fIsGrouped || fIsGroupMaster))
    {
        plSceneObject *sObj = GetTarget(0);
        if(sObj) 
            return sObj->DirtySynchState(SDLStateName, synchFlags);
    }
    return false;
}

void plAGMasterMod::SetIsGroupMaster(bool master, plMsgForwarder* msgForwarder)
{
    fIsGroupMaster = master;
    fMsgForwarder = msgForwarder;
}

void plAGMasterMod::SetIsGrouped(bool grouped)
{
    fIsGrouped = true;
}
