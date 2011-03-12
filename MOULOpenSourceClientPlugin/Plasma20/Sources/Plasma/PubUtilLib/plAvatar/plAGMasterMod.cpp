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
// singular
#include "plAGMasterMod.h"

// local
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plAGModifier.h"
// #include "plAvatarAnim.h"
#include "plAGMasterSDLModifier.h"
#include "plMatrixChannel.h"

// global
#include "hsResMgr.h"
#include "plgDispatch.h"

// other
// #include "../pnMessage/plRefMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../pnMessage/plSDLNotificationMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

////////////////
// PLAGMASTERMOD
////////////////
// Coordinates the activities of a bunch of plAGModifiers
// std::map<char *, plAGMasterMod *, stringISorter> plAGMasterMod::fInstances;

// CTOR
plAGMasterMod::plAGMasterMod()
: fTarget(nil),
  fNeedEval(false),
  fFirstEval(true),
  fAGMasterSDLMod(nil),
  fNeedCompile(false),
  fIsGrouped(false),
  fIsGroupMaster(false),
  fMsgForwarder(nil)
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
	stream->WriteSwap32(length); // backwards compatability. Nuke on next format change.
	stream->WriteSwap32(fPrivateAnims.size());
	int i;
	for (i = 0; i < fPrivateAnims.size(); i++)
	{
		mgr->WriteKey(stream, fPrivateAnims[i]->GetKey());
	}
	stream->Writebool(fIsGrouped);
	stream->Writebool(fIsGroupMaster);
	if (fIsGroupMaster)
		mgr->WriteKey(stream, fMsgForwarder->GetKey());

	// maybe later... WriteCachedMessages(stream, mgr);
}

void plAGMasterMod::Read(hsStream * stream, hsResMgr *mgr)
{
	plModifier::Read(stream, mgr);

	//////////////////////////////////////////
	int nameLength = stream->ReadSwap32();	// Unused. Nuke next format change.
	char *junk = TRACKED_NEW char[nameLength+1];	//
	stream->Read(nameLength, junk);			//
	junk[nameLength] = 0;					//
	delete [] junk;							//
	//////////////////////////////////////////

	int numPrivateAnims = stream->ReadSwap32();
	fPrivateAnims.reserve(numPrivateAnims);				// pre-allocate for performance
	int i;
	for (i = 0; i < numPrivateAnims; i++)
	{
		plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPrivateAnim);
		mgr->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
	}
	fIsGrouped = stream->Readbool();
	fIsGroupMaster = stream->Readbool();
	if (fIsGroupMaster)
	{
		plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0);
		mgr->ReadKeyNotifyMe(stream, msg, plRefFlags::kActiveRef);
	}

	// maybe later... ReadCachedMessages(stream, mgr);
}

// ADDTARGET
// Collect all the plAGModifiers from our children and attach private anims.
void plAGMasterMod::AddTarget(plSceneObject * object)
{
	plSynchEnabler p(false);	// turn off dirty tracking while in this function	
	
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
		masterIdx = autoIdx;		// If something autostarts, it wins.
	else if (initialIdx != -1)
		masterIdx = initialIdx;		// Otherwise, the fellow with the @initial point wins
	else
		masterIdx = timeIdx;		// Default case: the earliest anim wins

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
		fAGMasterSDLMod = TRACKED_NEW plAGMasterSDLModifier;
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
	fAGMasterSDLMod=nil;

	fTarget = nil;
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
hsBool plAGMasterMod::IEval(double secs, hsScalar del, UInt32 dirty)
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
void plAGMasterMod::ApplyAnimations(double time, hsScalar elapsed)
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
		if(!justThisChannel || stricmp(justThisChannel, mod->GetChannelName()) == 0)
		{
			plAGApplicator *app = mod->GetApplicator(kAGPinTransform);

			if(app) {
				plAGChannel *channel = app->GetChannel();
				if(channel)
				{
					plMatrixChannel *topChannel = plMatrixChannel::ConvertNoRef(channel);
					if(topChannel)
					{
						hsStatusMessageF("AGModifier: <%s>", mod->GetChannelName());
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
plAGModifier * plAGMasterMod::GetChannelMod(const char * name, hsBool dontCache ) const
{
	plAGModifier * result = nil;
	std::map<const char *, plAGModifier *, stringSorter>::const_iterator i = fChannelMods.find(name);

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
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0);
	hsgResMgr::ResMgr()->SendRef(mod, msg, plRefFlags::kActiveRef);
	
	return mod;
}

// IFINDAGMOD (sceneObject)
// See if there's an ag modifier on this sceneobject.
// Doesn't check for multiples; just returns the first one.
plAGModifier * plAGMasterMod::IFindChannelMod(const plSceneObject *SO, const char *name) const
{
	const plCoordinateInterface * CI = SO->GetCoordinateInterface();

	const plAGModifier * constMod = static_cast<const plAGModifier *>(FindModifierByClass(SO, plAGModifier::Index()));
	plAGModifier * mod = const_cast<plAGModifier *>(constMod);

	if(mod)
	{
		const char *modName = mod->GetChannelName();
		if(stricmp(name, modName) == 0)
			return mod;
	}

	if(CI)
	{
		int childCount = CI->GetNumChildren();
		for (int i = 0; i < childCount; i++)
		{
			const plSceneObject * subChild = CI->GetChild(i)->GetOwner();
			plAGModifier * mod = IFindChannelMod(subChild, name);

			if(mod)
				return mod;
		}
	}
	return nil;
}

// ATTACHANIMATIONBLENDED(anim, blend)
plAGAnimInstance * plAGMasterMod::AttachAnimationBlended(plAGAnim *anim,
														 hsScalar blendFactor /* = 0 */,
														 UInt16 blendPriority /* plAGMedBlendPriority */,
														 hsBool cache /* = false */)
{
	plAGAnimInstance *instance = nil;
	plAnimVector::iterator i;
	if(anim)
	{
		fNeedCompile = true;	// need to recompile the graph since we're editing it...
		for (i = fPrivateAnims.begin(); i != fPrivateAnims.end(); i++) 
		{
			if (*i == anim)
				break;
		}
		if (i == fPrivateAnims.end()) // Didn't find it. Ref it!
		{
			plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPublicAnim);
			hsgResMgr::ResMgr()->SendRef(anim, msg, plRefFlags::kActiveRef);
		}
		instance = TRACKED_NEW plAGAnimInstance(anim, this, blendFactor, blendPriority, cache, false);
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
plAGAnimInstance * plAGMasterMod::AttachAnimationBlended(const char *name, hsScalar blendFactor /* = 0 */, UInt16 blendPriority, hsBool cache /* = false */)
{
	plAGAnimInstance *instance = nil;
	plAGAnim *anim = plAGAnim::FindAnim(name);

	if(anim)
	{
		instance = AttachAnimationBlended(anim, blendFactor, blendPriority, cache);
	}
	return instance;
}

void plAGMasterMod::PlaySimpleAnim(const char *name)
{
	plATCAnim *anim = plATCAnim::ConvertNoRef(plAGAnim::FindAnim(name));
	plAGAnimInstance *instance = nil;
	if (anim)
	{
		if (FindAnimInstance(name))
			return;

		instance = AttachAnimationBlended(anim, 1.f, (UInt16)kAGMaxBlendPriority, false);
	}

	if (instance)
	{
		instance->SetLoop(false);
		instance->Start();

		plAGDetachCallbackMsg *msg = TRACKED_NEW plAGDetachCallbackMsg(GetKey(), kStop); 
		msg->SetAnimName(name);
		instance->GetTimeConvert()->AddCallback(msg);
		hsRefCnt_SafeUnRef(msg);
	}
}

// FINDANIMINSTANCE
// Look for an animation instance of the given name on the modifier.
// If we need this to be fast, should make it a map rather than a vector
plAGAnimInstance * plAGMasterMod::FindAnimInstance(const char *name)
{
	plAGAnimInstance *result = nil;

	if (name)
	{
		for (int i = 0; i < fAnimInstances.size(); i++)
		{
			plAGAnimInstance *act = fAnimInstances[i];
			const char *eachName = act->GetName();

			if( stricmp(eachName, name) == 0)
			{
				result = act;
				break;
			}
		}
	}
	return result;
}

// FINDORATTACHINSTANCE
plAGAnimInstance * plAGMasterMod::FindOrAttachInstance(const char *name, hsScalar blendFactor)
{
	plAGAnimInstance *result = FindAnimInstance(name);
	if(result)
	{
		// if it's already attached, we need to set the blend
		result->SetBlend(blendFactor);
	} else 	{
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
	
	fNeedCompile = true;	// need to recompile the graph since we're editing it...

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
void plAGMasterMod::DetachAnimation(const char *name)
{
	plAGAnimInstance *anim = FindAnimInstance(name);
	if(anim) {
		DetachAnimation(anim);
	}
}

void plAGMasterMod::DumpCurrentAnims(const char *header)
{
	if(header)
		hsStatusMessageF("Dumping Armature Anim Stack: %s", header);
	int nAnims = fAnimInstances.size();
	for(int i = nAnims - 1; i >= 0; i--)
	{
		plAGAnimInstance *inst = fAnimInstances[i];
		const char *name = inst->GetName();
		float blend = inst->GetBlend();

		hsStatusMessageF("%d: %s with blend of %f\n", i, name, blend);
	}
}

// MSGRECEIVE
// receive trigger messages
hsBool plAGMasterMod::MsgReceive(plMessage* msg)
{
	plAnimCmdMsg* cmdMsg;
	plAGCmdMsg* agMsg;
	int i;

	plSDLNotificationMsg* nMsg = plSDLNotificationMsg::ConvertNoRef(msg);
	if (nMsg)
	{
		// Force a single eval
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
		return true;
	}

	if (cmdMsg = plAnimCmdMsg::ConvertNoRef(msg))
	{
		const char *targetName = cmdMsg->GetAnimName();

		if (!targetName)
			targetName = ENTIRE_ANIMATION_NAME;

		plAGAnimInstance *inst = FindAnimInstance(targetName);
		if (inst != nil)
		{
			if (cmdMsg->CmdChangesAnimTime())
			{
				for (i = 0; i < GetNumAnimations(); i++)
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

	if (agMsg = plAGCmdMsg::ConvertNoRef(msg))
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
		if (inst != nil)
		{
			if (agMsg->Cmd(plAGCmdMsg::kSetBlend))
				inst->Fade(agMsg->fBlend, agMsg->fBlendRate, plAGAnimInstance::kFadeBlend);
			if (agMsg->Cmd(plAGCmdMsg::kSetAmp))
				inst->Fade(agMsg->fAmp, agMsg->fAmpRate, plAGAnimInstance::kFadeAmp);
		}
		return true;
	}

	plAGInstanceCallbackMsg *agicMsg = plAGInstanceCallbackMsg::ConvertNoRef(msg);
	if (agicMsg != nil)
	{
		if (agicMsg->fEvent == kStart)
		{
			IRegForEval(true);
		}
		else if (agicMsg->fEvent == kStop)
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
	if (detachMsg != nil)
	{
		DetachAnimation(detachMsg->GetAnimName());
	}

	plGenRefMsg *genRefMsg;
	if (genRefMsg = plGenRefMsg::ConvertNoRef(msg))
	{
		plAGAnim *anim;
		if (anim = plAGAnim::ConvertNoRef(genRefMsg->GetRef()))
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
					for (i; i != fPrivateAnims.end(); i++)
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
		plAGModifier *agmod;
		if (agmod = plAGModifier::ConvertNoRef(genRefMsg->GetRef()))
		{
			if (genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest))
				fChannelMods[agmod->GetChannelName()] = agmod;
			else
				fChannelMods.erase(agmod->GetChannelName());

			return true;
		}
		plMsgForwarder *msgfwd;
		if (msgfwd = plMsgForwarder::ConvertNoRef(genRefMsg->GetRef()))
		{
			if (genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest))
				fMsgForwarder = msgfwd;
			else
				fMsgForwarder = nil;

			return true;
		}
	}
	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			AddTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));
		else
			RemoveTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));

		return true;
	}

	return plModifier::MsgReceive(msg);
}

void plAGMasterMod::IRegForEval(hsBool val)
{
	if (fNeedEval == val)
		return;

	fNeedEval = val;
	if (val)
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
	else
		plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
}

hsBool plAGMasterMod::HasRunningAnims()
{
	int i;
	hsBool needEval = false;
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
hsBool plAGMasterMod::DirtySynchState(const char* SDLStateName, UInt32 synchFlags)
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
