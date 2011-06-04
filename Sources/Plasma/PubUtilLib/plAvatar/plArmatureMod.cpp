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
#include "plArmatureMod.h"

// local
#include "plAvBrain.h"
#include "plAvBrainUser.h"
#include "plAvatarMgr.h"
#include "plAGModifier.h"
#include "plAvatarClothing.h"
#include "plClothingSDLModifier.h"
#include "plAvatarSDLModifier.h"
#include "plAGAnim.h"
#include "plArmatureEffects.h"
#include "plAvBrainHuman.h"
#include "plMatrixChannel.h"
#include "plAvatarTasks.h"
#include "plAvCallbackAction.h"
#include "plAvBrainCritter.h"

// global
#include "plgDispatch.h"
#include "hsQuat.h"
#include "hsTimer.h"

// other
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../plInterp/plAnimTimeConvert.h"

#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../pnMessage/plAttachMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plCorrectionMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plPipeResMakeMsg.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plAvatarFootMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plLoadAgeMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plListenerMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plParticleUpdateMsg.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleSDLMod.h"

#include "../pfMessage/plArmatureEffectMsg.h"
#include "../pfMessage/pfKIMsg.h"
#include "../plVault/plVault.h"

#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plKeyImp.h" 

#include "../plDrawable/plInstanceDrawInterface.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plSurface/plLayerAnimation.h"
#include "../plSurface/hsGMaterial.h"

#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"			// for CCR stuff..
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plModifier/plSpawnModifier.h"
#include "../plPipeline/plDebugText.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plAudio/plWin32StaticSound.h"
#include "../plAudio/plAudioSystem.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plInputCore/plAvatarInputInterface.h"
#include "../plInputCore/plSceneInputInterface.h"
#include "../plInputCore/plInputDevice.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "../plScene/plRelevanceMgr.h"
#include "../plMessage/plSimStateMsg.h"

#include "../plGImage/plLODMipmap.h"
#include "plPipeline.h"
#include "plTweak.h"
#include "../plDrawable/plVisLOSMgr.h"

int plArmatureModBase::fMinLOD = 0;		// standard is 3 levels of LOD
double plArmatureModBase::fLODDistance = 50.0;


plArmatureModBase::plArmatureModBase() :
	fWaitFlags(kNeedMesh | kNeedPhysics | kNeedApplicator | kNeedBrainActivation),
	fController(nil),
	fCurLOD(-1),
	fRootAnimator(nil),
	fDisabledPhysics(0),
	fDisabledDraw(0)
{
}

plArmatureModBase::~plArmatureModBase()
{
	delete fController;

	while (fUnusedBones.size() > 0)
	{
		delete fUnusedBones.back();
		fUnusedBones.pop_back();
	}	
}

hsBool plArmatureModBase::MsgReceive(plMessage* msg)
{	
	hsBool result = false;
	
	plArmatureBrain *curBrain = nil;
	if (fBrains.size() > 0)
	{
		curBrain = fBrains.back();
		if(curBrain->MsgReceive(msg))
			return true;
	}
	
	return plAGMasterMod::MsgReceive(msg);
}	

void plArmatureModBase::AddTarget(plSceneObject* so)
{
	plAGMasterMod::AddTarget(so);
	
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plArmatureModBase::RemoveTarget(plSceneObject* so)
{
	int count = fBrains.size();
	for(int i = count - 1; i >= 0; i--)
	{
		plArmatureBrain *brain = fBrains[i];
		if (brain)
			brain->Deactivate();
		delete brain;
		fBrains.pop_back();
	}
	
	plAGMasterMod::RemoveTarget(so);
}

hsBool plArmatureModBase::IEval(double time, hsScalar elapsed, UInt32 dirty)
{
	if (IsFinal())
	{
		if (fBrains.size())
		{
			plArmatureBrain *curBrain = fBrains.back();
			if (curBrain)
			{
				hsBool result = curBrain->Apply(time, elapsed);
				if (!result)
				{
					PopBrain();
					delete curBrain;
				}
			}
		}
		AdjustLOD();		
	}
	else
		IFinalize();
	
	return true;
}

void plArmatureModBase::Read(hsStream * stream, hsResMgr *mgr)
{
	plAGMasterMod::Read(stream, mgr);

	int i;
	int meshKeyCount = stream->ReadSwap32();
	for(i = 0; i < meshKeyCount; i++)
	{
		plKey meshKey = mgr->ReadKey(stream);
		fMeshKeys.push_back(meshKey);
		
		plKeyVector *vec = TRACKED_NEW plKeyVector;
		int boneCount = stream->ReadSwap32();
		for(int j = 0; j < boneCount; j++)
			vec->push_back(mgr->ReadKey(stream));
		fUnusedBones.push_back(vec);
	}

	int nBrains = stream->ReadSwap32();
	for (i = 0; i < nBrains; i++)
	{
		plArmatureBrain * brain = (plArmatureBrain *)mgr->ReadCreatable(stream);
		this->PushBrain(brain);
	}
}

void plArmatureModBase::Write(hsStream *stream, hsResMgr *mgr)
{
	plAGMasterMod::Write(stream, mgr);

	int i;
	int meshKeyCount = fMeshKeys.size();
	stream->WriteSwap32(meshKeyCount);	
	for (i = 0; i < meshKeyCount; i++)
	{
		plKey meshKey = fMeshKeys[i];
		mgr->WriteKey(stream, meshKey);
		
		// Should be a list per mesh key
		stream->WriteSwap32(fUnusedBones[i]->size());
		for(int j = 0; j < fUnusedBones[i]->size(); j++)
			mgr->WriteKey(stream, (*fUnusedBones[i])[j]);
	}
	
	int nBrains = fBrains.size();
	stream->WriteSwap32(nBrains);
	for (i = 0; i < nBrains; i++)
	{
		mgr->WriteCreatable(stream, fBrains[i]);
	}
}

void plArmatureModBase::AddressMessageToDescendants(const plCoordinateInterface * CI, plMessage *msg)
{
	if (!CI)
		return;
	
	msg->AddReceiver(CI->GetOwnerKey());
	for (int i = 0; i < CI->GetNumChildren(); i++)
		AddressMessageToDescendants(CI->GetChild(i), msg);
}

void plArmatureModBase::EnableDrawingTree(const plSceneObject *object, hsBool status)
{
	if (!object)
		return;
	
	plEnableMsg *msg = TRACKED_NEW plEnableMsg;
	if (status)
		msg->SetCmd( plEnableMsg::kEnable );
	else
		msg->SetCmd( plEnableMsg::kDisable );
	msg->SetCmd( plEnableMsg::kDrawable );
	
	const plCoordinateInterface *pCI = object->GetCoordinateInterface();
	AddressMessageToDescendants(pCI, msg);
	plgDispatch::MsgSend(msg);
}

plKey plArmatureModBase::GetWorldKey() const
{
	if (fController)
		return fController->GetSubworld();
	else
		return nil;
}

hsBool plArmatureModBase::ValidatePhysics()
{
	if (!fTarget)
		return false;

	if (fController)
	{
		EnablePhysics(true);
		fWaitFlags &= ~kNeedPhysics;
	}
	
	return !(fWaitFlags & kNeedPhysics);
}

hsBool plArmatureModBase::ValidateMesh()
{
	if (fWaitFlags & kNeedMesh)
	{
		fWaitFlags &= ~kNeedMesh;
		int n = fMeshKeys.size();
		
		for(int i = 0; i < n; i++)
		{
			plKey meshKey = fMeshKeys[i];
			plSceneObject *meshObj = (plSceneObject *)meshKey->GetObjectPtr();
			
			if (!meshObj)
			{
				fWaitFlags |= kNeedMesh;
				break;
			}
			hsBool visible = (i == fCurLOD ? true : false);
			EnableDrawingTree(meshObj, visible);
		}	
	}
	
	return !(fWaitFlags & kNeedMesh);
}

void plArmatureModBase::PushBrain(plArmatureBrain *brain)
{
	plArmatureBrain *oldBrain = GetCurrentBrain();
	if (oldBrain)
		oldBrain->Suspend();

	
	fBrains.push_back(brain);
	// don't activate brains until we are attached to our target
	// addTarget will do activation...
	if (GetTarget(0))
		brain->Activate(this);

	DirtySynchState(kSDLAvatar, 0);
}

void plArmatureModBase::PopBrain()
{
	plArmatureBrain *oldBrain = nil;
	if (fBrains.size() > 0)
	{
		oldBrain = fBrains.back();
		oldBrain->Deactivate();
		fBrains.pop_back();
	}
	
	plArmatureBrain *newBrain = GetCurrentBrain();
	if (newBrain)
		newBrain->Resume();
	
	DirtySynchState(kSDLAvatar, 0);
}

plArmatureBrain *plArmatureModBase::GetCurrentBrain() const
{
	plArmatureBrain *result = nil;
	if (fBrains.size() > 0)
		result = fBrains.back();
	
	return result;
}

plDrawable *plArmatureModBase::FindDrawable() const
{
	if (fMeshKeys[0] == nil)
		return nil;
	
	plSceneObject *so = plSceneObject::ConvertNoRef(fMeshKeys[0]->ObjectIsLoaded());
	if (so == nil)
		return nil;
	
	const plDrawInterface *di = so->GetDrawInterface();
	if (di && di->GetNumDrawables() > 0)
	{
		plDrawable *spans = plDrawable::ConvertNoRef(di->GetDrawable(0));
		if (spans)
			return spans;
	}
	
	const plInstanceDrawInterface *idi = plInstanceDrawInterface::ConvertNoRef(di);
	if (idi)
		return idi->GetInstanceDrawable();
	
	return nil;
}

void plArmatureModBase::LeaveAge()
{
	int nBrains = fBrains.size();
	for (int i = nBrains - 1; i >= 0; i--)
	{
		plArmatureBrain * curBrain = fBrains[i];
		if (curBrain->LeaveAge())
		{
			if (curBrain == GetCurrentBrain())
			{
				PopBrain();
				delete curBrain;
			}
		}
	}
}	

hsBool plArmatureModBase::IsFinal()
{
	return !fWaitFlags;
}

void plArmatureModBase::AdjustLOD()
{
	if (!IsDrawEnabled())
		return;

	hsPoint3 camPos = plVirtualCam1::Instance()->GetCameraPos();
		
	plSceneObject * SO = GetTarget(0);
	if (SO)
	{
		hsMatrix44  l2w = SO->GetLocalToWorld();
		hsPoint3 ourPos = l2w.GetTranslate();
		hsPoint3 delta = ourPos - camPos;
		hsScalar distanceSquared = delta.MagnitudeSquared();
		if (distanceSquared < fLODDistance * fLODDistance)
			SetLOD(__max(0, fMinLOD));
		else if (distanceSquared < fLODDistance * fLODDistance * 4.0) 
			SetLOD(__max(1, fMinLOD));
		else 
			SetLOD(2);
	}
}

// Should always be called from AdjustLOD
hsBool plArmatureModBase::SetLOD(int iNewLOD)
{
	if (iNewLOD >= fMeshKeys.size())
		iNewLOD = fMeshKeys.size() - 1;
	
	int oldLOD = fCurLOD;
	if (iNewLOD != fCurLOD)
	{
		int oldLOD = fCurLOD;
		if(fMeshKeys.size() > iNewLOD)
		{
			if (fCurLOD != -1)
			{
				plSceneObject * oldMesh = (plSceneObject *)fMeshKeys[oldLOD]->GetObjectPtr();
				EnableDrawingTree(oldMesh, false);
			} 
			else 
			{
				// just starting up; turn all except current off
				for (int i = 0; i < fMeshKeys.size(); i++)
				{
					if (i != iNewLOD)
					{
						plKey offKey = fMeshKeys[i];
						plSceneObject * offMesh = (plSceneObject *)offKey->GetObjectPtr();
						EnableDrawingTree(offMesh, false);
					}
				}
			}
			
			fCurLOD = iNewLOD;
			plSceneObject * newMesh = (plSceneObject *)fMeshKeys[fCurLOD]->GetObjectPtr();
			EnableDrawingTree(newMesh, true);
			
			int boneLOD;
			for (boneLOD = 0; boneLOD < fMeshKeys.size(); boneLOD++)
				IEnableBones(boneLOD, boneLOD <= iNewLOD ? false: true);
		}
	}
	return oldLOD;
}

void plArmatureModBase::RefreshTree()
{
	fCurLOD = -1;
	AdjustLOD();
}

int plArmatureModBase::AppendMeshKey(plKey meshKey)
{
	fMeshKeys.push_back(meshKey);
	return fMeshKeys.size() - 1;
}

int plArmatureModBase::AppendBoneVec(plKeyVector *boneVec)
{
	fUnusedBones.push_back(boneVec);
	return fUnusedBones.size() - 1;
}

UInt8 plArmatureModBase::GetNumLOD() const
{
	return fMeshKeys.size();
}

void plArmatureModBase::EnablePhysics(hsBool status, UInt16 reason /* = kDisableReasonUnknown */)
{
	if (status)
		fDisabledPhysics &= ~reason;
	else
		fDisabledPhysics |= reason;
	
	hsBool newStatus = !fDisabledPhysics;

	if (fController)
		fController->Enable(newStatus);
}

//
// Enabling Kinematics (state=true) will have the affect of:
//   - disabling outside forces (gravity and other physics objects pushing us)
//   - but leave on collisions with detector regions
// Disabling Kinematics (state=false) means:
//   - all outside forces will affect us and collisions on
//   i.e. normal enabled physical
void plArmatureModBase::EnablePhysicsKinematic(hsBool status)
{
	if (fController)
		fController->Kinematic(status);
}

void plArmatureModBase::EnableDrawing(hsBool status, UInt16 reason /* = kDisableReasonUnknown */)
{
	hsBool oldStatus = !fDisabledDraw;
	if (status)
		fDisabledDraw &= ~reason;
	else
		fDisabledDraw |= reason;
	
	hsBool newStatus = !fDisabledDraw;
	if (oldStatus == newStatus)
		return;
	
	int i;
	for (i = 0; i < fMeshKeys.size(); i++)
	{
		plSceneObject *obj = plSceneObject::ConvertNoRef(fMeshKeys[i]->ObjectIsLoaded());
		if (obj)
			EnableDrawingTree(obj, newStatus);
	}
	
	if (status)
		fCurLOD = -1; // We just enabled all LOD. Need to force ourselves to recompute current LOD	
}

void plArmatureModBase::IFinalize()
{
	if (fWaitFlags & kNeedMesh)
		ValidateMesh();
	if (fWaitFlags & kNeedPhysics)
		ValidatePhysics();
	if (fWaitFlags & kNeedApplicator)
		ICustomizeApplicator();
	if (fWaitFlags & kNeedBrainActivation)
	{
		int nBrains = fBrains.size();
		for (int i = 0; i < nBrains; i++)
		{
			// every brain gets activated, but all except the top brain
			// also get suspended when the one above them is activated.
			if (i > 0)
				fBrains[i - 1]->Suspend();

			fBrains[i]->Activate(this);
		}	
		fWaitFlags &= ~kNeedBrainActivation;
	}
}

void plArmatureModBase::ICustomizeApplicator()
{
	const plAGModifier *agMod = plAGModifier::ConvertNoRef(FindModifierByClass(GetTarget(0), plAGModifier::Index()));
	if (agMod)
	{
		plAGApplicator *app = agMod->GetApplicator(kAGPinTransform);
		if (app)
		{
			plMatrixDifferenceApp *differ = plMatrixDifferenceApp::ConvertNoRef(app);
			
			if (differ)
			{
				fRootAnimator = differ;
				fWaitFlags &= ~kNeedApplicator;
				return; // already there
			}
		}
		plAGModifier *volAGMod = const_cast<plAGModifier *>(agMod);
		plMatrixDifferenceApp *differ = TRACKED_NEW plMatrixDifferenceApp();
		
		fRootAnimator = differ;
		volAGMod->SetApplicator(differ);		
		differ->Enable(false);
		fWaitFlags &= ~kNeedApplicator;
	}
}

void plArmatureModBase::IEnableBones(int lod, hsBool enable)
{
	if (lod < fUnusedBones.size())
	{
		plKeyVector *vec = fUnusedBones[lod];
		int i;
		for (i = 0; i < vec->size(); i++)
			((plAGModifier *)(*vec)[i]->GetObjectPtr())->Enable(enable);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

const char *plArmatureMod::BoneStrings[] = {"Male", "Female", "Critter", "Actor"};

const hsScalar plArmatureMod::kAvatarInputSynchThreshold = 10.f;
hsScalar plArmatureMod::fMouseTurnSensitivity = 1.f;
hsBool plArmatureMod::fClickToTurn = true;

void plArmatureMod::IInitDefaults()
{
	fBoneRootAnimator = nil;
	fRootAGMod = nil;
	fFootSoundSOKey = nil;
	fLinkSoundSOKey = nil;
	fBodyType = kBoneBaseMale;
	fClothingOutfit = nil;
	fClothingSDLMod = nil;
	fAvatarSDLMod = nil;
	fAvatarPhysicalSDLMod = nil;
	fEffects = nil;
	fDebugOn = false;
	fBoneMap = nil;
	fStealthMode = plAvatarStealthModeMsg::kStealthVisible;
	fStealthLevel = 0;
	fMouseFrameTurnStrength = 0.f;
	fSuspendInputCount = 0;
	fMidLink = false;
	fAlreadyPanicLinking = false;
	fReverseFBOnIdle = false;
	fFollowerParticleSystemSO = nil;
	fPendingSynch = false;
	fLastInputSynch = 0;
	fOpaque = true;
	fPhysHeight = 0.f;
	fPhysWidth = 0.f;
	fUpdateMsg = nil;
	fRootName = nil;
	fDontPanicLink = false;
	fBodyAgeName = "GlobalAvatars";
	fBodyFootstepSoundPage = "Audio";
	fAnimationPrefix = "Male";
	fUserStr = "";
}

plArmatureMod::plArmatureMod() : plArmatureModBase()
{
	IInitDefaults();
	fWaitFlags |= kNeedAudio | kNeedCamera | kNeedSpawn;
}

plArmatureMod::~plArmatureMod()
{
	delete fBoneMap;
	delete [] fRootName;

	if (fUpdateMsg)
		fUpdateMsg->UnRef();
}

void plArmatureMod::SetPositionAndRotationSim(const hsPoint3* position, const hsQuat* rotation)
{
	const plCoordinateInterface* subworldCI = nil;
	if (fController)
		subworldCI = fController->GetSubworldCI();

	hsMatrix44 l2w, w2l;

	// If we need the position or rotation, grab it from the avatar
	if (!position || !rotation)
	{
		// Get the current position of the avatar in sim space
		hsMatrix44 avatarL2S = GetTarget(0)->GetLocalToWorld();
		if (subworldCI)
			avatarL2S = subworldCI->GetWorldToLocal() * avatarL2S;

		if (!rotation)
			l2w = avatarL2S;
		else
			rotation->MakeMatrix(&l2w);

		if (!position)
		{
			hsPoint3 simPos;
			avatarL2S.GetTranslate(&simPos);
			l2w.SetTranslate(&simPos);
		}
		else
			l2w.SetTranslate(position);
	}
	else
	{
		rotation->MakeMatrix(&l2w);
		l2w.SetTranslate(position);
	}

	// We've got the requested position of the avatar in subworld space, convert
	// it to world space if necessary
	if (subworldCI)
		l2w = subworldCI->GetLocalToWorld() * l2w;
	l2w.GetInverse(&w2l);

	GetTarget(0)->SetTransform(l2w, w2l);
	GetTarget(0)->FlushTransform();
}

void plArmatureMod::GetPositionAndRotationSim(hsPoint3* position, hsQuat* rotation)
{
	hsMatrix44 l2s = GetTarget(0)->GetLocalToWorld();

	if (fController)
	{
		const plCoordinateInterface* subworldCI = fController->GetSubworldCI();
		if (subworldCI)
			l2s = subworldCI->GetWorldToLocal() * l2s;

		if (position)
			l2s.GetTranslate(position);
		if (rotation)
			rotation->SetFromMatrix(&l2s);
	}
}

const plSceneObject *plArmatureMod::FindBone(const char * name) const
{
	plSceneObject *result = nil;

	plAGModifier * mod = GetChannelMod(name);

	if (mod)
		result = mod->GetTarget(0);

	return result;
}

const plSceneObject *plArmatureMod::FindBone(UInt32 id) const
{
	if(fBoneMap)
		return fBoneMap->FindBone(id);
	else
		return nil;
}

void plArmatureMod::AddBoneMapping(UInt32 id, const plSceneObject *bone)
{
	if(!fBoneMap)
		fBoneMap = TRACKED_NEW plAvBoneMap();

	fBoneMap->AddBoneMapping(id, bone);
}

void plArmatureMod::WindowActivate(bool active)
{
	if (!active) // We don't want the avatar to move while we don't have focus
	{
		if (!plAvatarMgr::GetInstance())
			return;

		plArmatureMod *localAv = plAvatarMgr::GetInstance()->GetLocalAvatar();
		if (localAv)
			localAv->ClearInputFlags(true, true);
	}
}

char	*plArmatureMod::fSpawnPointOverride = nil;

void	plArmatureMod::SetSpawnPointOverride( const char *overrideObjName )
{
	delete [] fSpawnPointOverride;
	if( overrideObjName == nil )
		fSpawnPointOverride = nil;
	else
	{
		fSpawnPointOverride = hsStrcpy( overrideObjName );
		strlwr( fSpawnPointOverride );
	}
}

int	plArmatureMod::IFindSpawnOverride( void )
{
	if( fSpawnPointOverride == nil || fSpawnPointOverride[ 0 ] == 0 )
		return -1;
	int		i;
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	for( i = 0; i < mgr->NumSpawnPoints(); i++ )
	{
		char	str2[ 256 ];
		strcpy(str2, mgr->GetSpawnPoint( i )->GetTarget(0)->GetKeyName());
		strlwr(str2);

		if (strstr(str2, fSpawnPointOverride) != nil)
			return i; // Found it!
	}
	return -1;
}

void plArmatureMod::Spawn(double timeNow)
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int numSpawnPoints = mgr->NumSpawnPoints();

	int spawnNum = IFindSpawnOverride();
	if( spawnNum == -1 )
	{
		spawnNum = plAvatarMgr::GetInstance()->FindSpawnPoint( "LinkInPointDefault" );
		if( spawnNum == -1 )
		{
			spawnNum = plNetClientApp::GetInstance()->GetJoinOrder();
		}
	}
	
	hsAssert(numSpawnPoints, "No spawn points! You are about to crash!");
	
	if ( numSpawnPoints )
		spawnNum %= numSpawnPoints;
	
	ValidatePhysics();
	
	SpawnAt(spawnNum, timeNow);
}

void plArmatureMod::SpawnAt(int spawnNum, double time)
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	const plSpawnModifier * spawn = mgr->GetSpawnPoint(spawnNum);
	
	hsMatrix44 l2w;
	hsMatrix44 w2l;
	if ( spawn )
	{
		const plSceneObject * spawnObj = spawn->GetTarget(0);
		l2w = spawnObj->GetLocalToWorld();
		w2l = spawnObj->GetWorldToLocal();
	}
	
	if (fController)
		fController->ResetAchievedLinearVelocity();
	plCoordinateInterface* ci = (plCoordinateInterface*)GetTarget(0)->GetCoordinateInterface();
	l2w.RemoveScale();
	w2l.RemoveScale();
	ci->SetTransform(l2w, w2l);
	ci->FlushTransform();
	
	if (plVirtualCam1::Instance())
		plVirtualCam1::Instance()->SetCutNextTrans();
	
	if (GetFollowerParticleSystemSO())
	{
		// Since particles are in world space, if we've got some surrounding us, we've got to translate them to compensate for our warp.
		plParticleSystem *sys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(GetFollowerParticleSystemSO()->GetModifierByType(plParticleSystem::Index())));
		if (sys)
		{
			hsPoint3 trans = l2w.GetTranslate() - GetTarget(0)->GetLocalToWorld().GetTranslate();
			sys->TranslateAllParticles(trans);
		}
	}
	
	int nBrains = fBrains.size();
	for (int i = 0; i < nBrains; i++)
	{
		plArmatureBrain * curBrain = fBrains[i];	
		curBrain->Spawn(time);
	}
	fWaitFlags &= ~kNeedSpawn;
	
	plAvatarSpawnNotifyMsg *notify = TRACKED_NEW plAvatarSpawnNotifyMsg();
	notify->SetTimeStamp(hsTimer::GetSysSeconds() + 0.1);
	notify->SetBCastFlag(plMessage::kBCastByExactType);
	notify->Send();

	EnablePhysics(true);
}

void plArmatureMod::SetFollowerParticleSystemSO(plSceneObject *follower)
{
	// TODO: Check for old one and clean up.
	hsPoint3 trans = GetTarget(0)->GetLocalToWorld().GetTranslate() - follower->GetLocalToWorld().GetTranslate();
	
	plWarpMsg *warp = TRACKED_NEW plWarpMsg(GetKey(), follower->GetKey(), plWarpMsg::kFlushTransform | plWarpMsg::kZeroVelocity,
		GetTarget(0)->GetLocalToWorld());
	warp->Send();
	hsgResMgr::ResMgr()->AddViaNotify(follower->GetKey(), TRACKED_NEW plAttachMsg(GetTarget(0)->GetKey(), nil, plRefMsg::kOnRequest), plRefFlags::kActiveRef);
	fFollowerParticleSystemSO = follower;

	plParticleSystem *sys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(follower->GetModifierByType(plParticleSystem::Index())));
	if (sys)
	{
		sys->fMiscFlags |= plParticleSystem::kParticleSystemAlwaysUpdate;
		sys->TranslateAllParticles(trans);
		sys->SetAttachedToAvatar(true);
	}
}

plSceneObject *plArmatureMod::GetFollowerParticleSystemSO()
{
	return fFollowerParticleSystemSO;
}

void plArmatureMod::RegisterForBehaviorNotify(plKey key)
{
	if (fNotifyKeys.Find(key) == fNotifyKeys.kMissingIndex)
		fNotifyKeys.Append(key);
}

void plArmatureMod::UnRegisterForBehaviorNotify(plKey key)
{
	fNotifyKeys.RemoveItem(key);
}

void plArmatureMod::IFireBehaviorNotify(UInt32 type, hsBool behaviorStart)
{
	if (fNotifyKeys.GetCount() > 0)
	{
		plAvatarBehaviorNotifyMsg *msg = TRACKED_NEW plAvatarBehaviorNotifyMsg();
		msg->SetSender(GetKey());
		msg->AddReceivers(fNotifyKeys);
		msg->fType = type;
		msg->state = behaviorStart;
		msg->Send();
	}
}

void plArmatureMod::EnterAge(hsBool reSpawn)
{
	fMidLink = false;
	fAlreadyPanicLinking = false;
	EnablePhysics(true, kDisableReasonLinking);
	ValidatePhysics();
	
	// force regions to send an update whenever we enter an age
	fOldRegionsICareAbout.Clear();
	fOldRegionsImIn.Clear();
	
	if (GetFollowerParticleSystemSO())
	{
		const plParticleSystem *sys = plParticleSystem::ConvertNoRef(GetFollowerParticleSystemSO()->GetModifierByType(plParticleSystem::Index()));
		hsAssert(sys, "We have a particle system SO, but no system?");
		if (sys)
		{
			// Need to tell other clients about this
			plLoadCloneMsg *clone = TRACKED_NEW plLoadCloneMsg(GetFollowerParticleSystemSO()->GetKey(), plAvatarMgr::GetInstance()->GetKey(), GetKey()->GetUoid().GetClonePlayerID(), true);
			clone->SetBCastFlag(plMessage::kLocalPropagate, false);
			clone->Send();

			GetFollowerParticleSystemSO()->DirtySynchState(plParticleSDLMod::kStrNumParticles, plSynchedObject::kBCastToClients);
		}
	}

	ClearInputFlags(true, false);
	if (reSpawn)
		fWaitFlags |= kNeedSpawn;
	
	// In case we personal age linked out of a situation and didn't properly
	// re-enable input...
	plAvatarInputInterface::GetInstance()->EnableForwardMovement(true);
	plAvatarInputInterface::GetInstance()->EnableJump(true);
	while (fSuspendInputCount > 0)
		ResumeInput();
	plAvatarInputInterface::GetInstance()->EnableMouseMovement();
}

void plArmatureMod::LeaveAge()
{
	plArmatureModBase::LeaveAge();
	
	fMidLink = true;

	if (fController)
	{
		fController->LeaveAge();
	}
	
	if (GetFollowerParticleSystemSO())
	{
		// Need to tell other clients to remove this
		plLoadCloneMsg *clone = TRACKED_NEW plLoadCloneMsg(GetFollowerParticleSystemSO()->GetKey(), plAvatarMgr::GetInstance()->GetKey(), GetKey()->GetUoid().GetClonePlayerID(), false);
		clone->SetBCastFlag(plMessage::kLocalPropagate, false);
		clone->Send();
	}

	GetArmatureEffects()->ResetEffects();
	ClearInputFlags(true, false);	
}

void plArmatureMod::PanicLink(hsBool playLinkOutAnim /* = true */)
{
	// console override... just go back to the beginning
	if (fDontPanicLink)
	{
		Spawn(0.f);
		return;
	}

	if (fAlreadyPanicLinking)
		return;
	fAlreadyPanicLinking = true;


	plNetApp::StaticDebugMsg("plArmatureMod::PanicLink()");

	// make the player book blink as they are linking out
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kStartBookAlert );
	plgDispatch::MsgSend( msg );

	// Can't depend on the anim to link if the human brain isn't ready to deal with it
	plAvBrainHuman *brain = plAvBrainHuman::ConvertNoRef(GetCurrentBrain());
	// If things break then uncomment the code below
	if (!brain)//  || brain->IsRunningTask())
		playLinkOutAnim = false;

	if (playLinkOutAnim)
	{
		plAvOneShotLinkTask *task = TRACKED_NEW plAvOneShotLinkTask;

		char *animName = MakeAnimationName("FallingLinkOut");
		task->SetAnimName(animName);
		task->SetMarkerName("touch");
	
		plAvTaskMsg *taskMsg = TRACKED_NEW plAvTaskMsg(GetKey(), GetKey(), task);
		taskMsg->Send();

		delete [] animName;
	}
	else
	{
		EnablePhysics(false, plArmatureMod::kDisableReasonLinking);
		ILinkToPersonalAge();
	}
}	

void plArmatureMod::PersonalLink()
{
	// Can't depend on the anim to link if the human brain isn't ready to deal with it
	plAvBrainHuman *brain = plAvBrainHuman::ConvertNoRef(GetCurrentBrain());
	if (!brain || brain->IsRunningTask())
		ILinkToPersonalAge();
	else
	{
		plAvOneShotLinkTask *task = TRACKED_NEW plAvOneShotLinkTask;
		char *animName = MakeAnimationName("PersonalLink");	
		task->SetAnimName(animName);
		task->SetMarkerName("touch");
		delete [] animName;
		
		plAvTaskMsg *taskMsg = TRACKED_NEW plAvTaskMsg(GetKey(), GetKey(), task);
		taskMsg->SetBCastFlag(plMessage::kNetPropagate);	
		taskMsg->Send();
	}
}

hsBool plArmatureMod::MsgReceive(plMessage* msg)
{	
	hsBool result = false;
	
	plArmatureBrain *curBrain = nil;
	if (fBrains.size() > 0)
	{
		curBrain = fBrains.back();
		if(curBrain->MsgReceive(msg))
			return true;
	}	

	plAvatarInputStateMsg *aisMsg = plAvatarInputStateMsg::ConvertNoRef(msg);
	if (aisMsg)
	{
		IHandleInputStateMsg(aisMsg);
		return true;
	}
	
	plControlEventMsg *control = plControlEventMsg::ConvertNoRef(msg);
	if(control)
	{
		IHandleControlMsg(control);
		return true;
	}

	plCorrectionMsg *corMsg = plCorrectionMsg::ConvertNoRef(msg);
	if (corMsg)
	{
		hsMatrix44 &correction = corMsg->fWorldToLocal * GetTarget(0)->GetLocalToWorld();
		if (fBoneRootAnimator)
			fBoneRootAnimator->SetCorrection(correction);
	}
		
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
	if (refMsg) 
	{
		plClothingOutfit *outfit = plClothingOutfit::ConvertNoRef(refMsg->GetRef());
		if (outfit) 
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fClothingOutfit = outfit;
				fClothingOutfit->fAvatar = this;
				plgDispatch::Dispatch()->RegisterForExactType(plPipeRTMakeMsg::Index(), fClothingOutfit->GetKey());
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				if (fClothingOutfit)
					plgDispatch::Dispatch()->UnRegisterForExactType(plPipeRTMakeMsg::Index(), fClothingOutfit->GetKey());
				fClothingOutfit = nil;
				outfit->fAvatar = nil;
			}
			return true;
		}
		plArmatureEffectsMgr *effects = plArmatureEffectsMgr::ConvertNoRef(refMsg->GetRef());
		if (effects)
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fEffects = effects;
				fEffects->fArmature = this;
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				fEffects->fArmature = nil;
				fEffects = nil;
			}
			return true;		
		}
		plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
		if (so)
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fClothToSOMap.ExpandAndZero(refMsg->fWhich + 1);
				fClothToSOMap[refMsg->fWhich] = so;
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				fClothToSOMap[refMsg->fWhich] = nil;
			}
			return true;
		}
	}
	
	if (fEffects)
	{
		plArmatureEffectMsg *aeMsg = plArmatureEffectMsg::ConvertNoRef(msg);
		plArmatureEffectStateMsg *aesMsg = plArmatureEffectStateMsg::ConvertNoRef(msg);
		if (aeMsg || aesMsg)
			return fEffects->MsgReceive(msg);
	}
	
	plAttachMsg *aMsg = plAttachMsg::ConvertNoRef(msg);
	if (aMsg)
	{
		GetTarget(0)->MsgReceive(aMsg);
		return true;
	}
	
	plEnableMsg *enMsg = plEnableMsg::ConvertNoRef(msg);	
	if(enMsg && enMsg->Type(plEnableMsg::kDrawable))
	{
		hsBool enable = enMsg->Cmd(plEnableMsg::kEnable);
		hsBool disable = enMsg->Cmd(plEnableMsg::kDisable);
		
		hsAssert(enable != disable, "Conflicting or missing commands to enable message");
		
		if(enable != disable)
			EnableDrawing(enable);
		return true;
	}
	if(enMsg && enMsg->Cmd(plEnableMsg::kPhysical))
	{
		EnablePhysics( enMsg->Cmd(plEnableMsg::kEnable));
	}
	
	plAvatarOpacityCallbackMsg *opacMsg = plAvatarOpacityCallbackMsg::ConvertNoRef(msg);
	if (opacMsg)
	{
		plLayerLinkAnimation *linkAnim = IFindLayerLinkAnim();
		if (linkAnim)
		{
			bool trans = (linkAnim->GetTimeConvert().CurrentAnimTime() != 0.f);
			ISetTransparentDrawOrder(trans);
		}
		return true;
	}

	plAvatarPhysicsEnableCallbackMsg *epMsg = plAvatarPhysicsEnableCallbackMsg::ConvertNoRef(msg);
	if (epMsg)
	{
		EnablePhysics(true);
		return true;
	}

	plAvatarStealthModeMsg *stealthMsg = plAvatarStealthModeMsg::ConvertNoRef(msg);
	if (stealthMsg && stealthMsg->GetSender() == GetTarget(0)->GetKey() &&
		(stealthMsg->fLevel != fStealthLevel || stealthMsg->fMode != fStealthMode))
	{
		fStealthMode = stealthMsg->fMode;
		fStealthLevel = (stealthMsg->fMode == plAvatarStealthModeMsg::kStealthVisible) ? 0 : stealthMsg->fLevel;

		if (fStealthMode == plAvatarStealthModeMsg::kStealthCloaked)
		{
			if (fUpdateMsg)
				fUpdateMsg->SetInvis(true);
		}
		else
		{
			if (fUpdateMsg)
				fUpdateMsg->SetInvis(false);
		}		
		
		if (fEffects)
			fEffects->MsgReceive(stealthMsg);
		
		if (stealthMsg->fMode == plAvatarStealthModeMsg::kStealthCloaked)
			EnableDrawing(false, kDisableReasonCCR);
		else
			EnableDrawing(true, kDisableReasonCCR);

		DirtySynchState(kSDLAvatar, 0);		// changed invisibility state
		plNetApp::StaticDebugMsg("ArmatureMod: rcvd avatarStealth msg, cloaked=%d", stealthMsg->fMode == plAvatarStealthModeMsg::kStealthCloaked);
		return true;
	}

	plParticleTransferMsg *partMsg = plParticleTransferMsg::ConvertNoRef(msg);
	if (partMsg)
	{
		// First, do we have the system?
		plSceneObject *dstSysSO = GetFollowerParticleSystemSO();
		if (!dstSysSO || ((plKeyImp*)dstSysSO->GetKey())->GetCloneOwner() != partMsg->fSysSOKey) 
		{
			// Need to clone and resend.
			if (plNetClientApp::GetInstance()->GetLocalPlayer() != GetTarget(0))
				return true; // Only the local player can create the clone.

			// Clone is sent to all players.
			plLoadCloneMsg *cloneMsg = TRACKED_NEW plLoadCloneMsg(partMsg->fSysSOKey->GetUoid(), plAvatarMgr::GetInstance()->GetKey(), GetKey()->GetUoid().GetClonePlayerID());
			cloneMsg->SetTriggerMsg(partMsg);
			cloneMsg->SetBCastFlag(plMessage::kNetForce);
			cloneMsg->Send();

			// Expect to receive a clone message later. Return for now.
			return true;
		}
		else
		{	
			plParticleSystem *dstSys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(dstSysSO->GetModifierByType(plParticleSystem::Index())));
			if (dstSys)
			{
				// Got the system. Time to steal particles!
				plParticleSystem *srcSys = nil;
				plSceneObject *srcSysSO = plSceneObject::ConvertNoRef(partMsg->fSysSOKey->ObjectIsLoaded());
				if (srcSysSO)
					srcSys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(srcSysSO->GetModifierByType(plParticleSystem::Index())));
				
				// A nil source system is ok. It just won't copy anything.
				int numToGen = partMsg->fNumToTransfer - dstSys->StealParticlesFrom(srcSys, partMsg->fNumToTransfer);
				if (numToGen > 0)
				{
					dstSys->GenerateParticles(numToGen);
				}
			}
			return true;
		}
	}

	plLoadAvatarMsg *avLoadMsg = plLoadAvatarMsg::ConvertNoRef(msg);
	if (avLoadMsg)
	{
		hsBool isPlayer = avLoadMsg->GetIsPlayer();
		if (!isPlayer)
			plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
		
		// see if we're being spawned explictly
		plKey spawnPoint = avLoadMsg->GetSpawnPoint();
		
		if (spawnPoint)
		{
			hsKeyedObject * spawnKO = spawnPoint->ObjectIsLoaded();
			if (spawnKO)
			{
				plSceneObject * spawnSO = plSceneObject::ConvertNoRef(spawnKO);
				if(spawnSO)
				{
					hsMatrix44 l2w = spawnSO->GetLocalToWorld();
					plWarpMsg *warpM = TRACKED_NEW plWarpMsg(nil, GetTarget(0)->GetKey(), 0, l2w);
					warpM->Send();
					fWaitFlags &= ~kNeedSpawn;
				}
			}
		}

		// copy the user string over
		const char* userStr = avLoadMsg->GetUserStr();
		if (userStr)
			fUserStr = userStr;
		else
			fUserStr = "";

		return true;
	}	

	plLoadCloneMsg *cloneMsg = plLoadCloneMsg::ConvertNoRef(msg);
	if (cloneMsg)
	{
		if (cloneMsg->GetIsLoading())
		{
			plSceneObject *so = plSceneObject::ConvertNoRef(cloneMsg->GetCloneKey()->ObjectIsLoaded());
			if (so)
			{
				so->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
				plParticleSystem *sys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(so->GetModifierByType(plParticleSystem::Index())));
				hsAssert(sys, "Modifier not loaded yet.");
				if (sys)
				{
					sys->DisableGenerators();
					sys->MsgReceive(cloneMsg); // Let the system know to finish its init stuff
					SetFollowerParticleSystemSO(so);
					if (!plNetClientApp::GetInstance()->IsLoadingInitialAgeState())
					{
						// Just happened. Redirect the trigger and transfer the particles
						MsgReceive(cloneMsg->GetTriggerMsg());
					}
					// otherwise we'll get SDL state about it and handle it in plParticleSDLMod

					return true;
				}
			}
		}
		else
		{
			if (cloneMsg->GetCloneKey() == GetFollowerParticleSystemSO())
			{
				SetFollowerParticleSystemSO(nil);
				return true;
			}
		}
	}

	plLinkEffectBCMsg *linkBCMsg = plLinkEffectBCMsg::ConvertNoRef(msg);
	if (linkBCMsg)
	{
		if (GetTarget(0)->GetKey() == linkBCMsg->fLinkKey)
		{
			if (linkBCMsg->HasLinkFlag(plLinkEffectBCMsg::kLeavingAge))
			{
				fMidLink = true;
				IFireBehaviorNotify(plHBehavior::kBehaviorTypeLinkOut, true);
			}
			else
				IFireBehaviorNotify(plHBehavior::kBehaviorTypeLinkIn, true);
		}
		return true;
	}

	plLinkInDoneMsg *doneMsg = plLinkInDoneMsg::ConvertNoRef(msg);
	if (doneMsg)
	{
		IFireBehaviorNotify(plHBehavior::kBehaviorTypeLinkIn, false);
		return true;
	}
	
	plAgeLoadedMsg *ageLoadMsg = plAgeLoadedMsg::ConvertNoRef(msg);
	if (ageLoadMsg && ageLoadMsg->fLoaded) 
	{
		// only the local player gets these
		NetworkSynch(hsTimer::GetSysSeconds(), true);
		EnablePhysics(true);
		return true;
	}
	
	plAnimCmdMsg *cmdMsg = plAnimCmdMsg::ConvertNoRef(msg);
	if (cmdMsg)
	{
		hsAssert(false, "Illegal use of plAnimCmdMsg on an avatar.");
		return true; 
	}

	plSubWorldMsg* subMsg = plSubWorldMsg::ConvertNoRef(msg);
	if (subMsg)
	{
		if (fController)
		{
			fController->SetSubworld(subMsg->fWorldKey);
			DirtySynchState(kSDLAvatar, plSynchedObject::kBCastToClients);
		}
		return true;
	}

	return plAGMasterMod::MsgReceive(msg);
}

hsBool plArmatureMod::IHandleControlMsg(plControlEventMsg* pMsg)
{
	// Slight change in design here...
	// Avatar input control messages are only sent locally.
	// When things change (that a remote client would care about),
	// we call SynchInputState and that sends off the bit vector
	// for the state of all avatar input controls.
	// 
	// This means a remote player will only know which controls are
	// active at a certain point. It might miss a particular keypress.
	// Don't rely on them receiving it.
	
	if (fSuspendInputCount > 0)
	{
		fQueuedCtrlMessages.push_back(pMsg);
		pMsg->Ref();
	}
	else 
	{
		ControlEventCode moveCode = pMsg->GetControlCode();

		hsBool flagChanged = false;
		if(pMsg->ControlActivated())
		{
			if (moveCode == B_CONTROL_TURN_TO && fClickToTurn)
			{
#if 0
				// This will do an LOS and call TurnToPoint()
				plSceneInputInterface::GetInstance()->RequestAvatarTurnToPointLOS();
#else
				plVisHit hit;
				if( plVisLOSMgr::Instance()->CursorCheck(hit) )
					TurnToPoint(hit.fPos);
#endif
				return true;
			}
			
			// This control is intended for turning while walking/running.
			// It is never net propagated. We just flag our physics state
			// as dirty, and let the physics synch (with dead reckoning) handle it.
			if (moveCode == A_CONTROL_TURN)
			{
				// Filter out the messages that are just the mouse recentering
				if (pMsg->GetPct() < 0.4 && pMsg->GetPct() > -0.4)
				{
					fMouseFrameTurnStrength += pMsg->GetPct() * fMouseTurnSensitivity;
					SynchIfLocal(hsTimer::GetSysSeconds(), false);
				}
			}

			if (!GetInputFlag( moveCode ) )
			{
				SetInputFlag( moveCode, true );
				flagChanged = true;

				if (moveCode == B_CONTROL_JUMP)
					SetInputFlag(B_CONTROL_CONSUMABLE_JUMP, true);
				
				if(plNetClientMgr::GetInstance()->AmCCR())
				{
					// special case for clipping: synch every key change
					SynchIfLocal( hsTimer::GetSysSeconds(), false);
				}
			}
		} else {
			if ( GetInputFlag( moveCode ) )
			{
				SetInputFlag( moveCode, false );
				flagChanged = true;

				if (fReverseFBOnIdle && (moveCode == B_CONTROL_MOVE_FORWARD || moveCode == B_CONTROL_MOVE_BACKWARD))
				{
					if (!GetInputFlag(B_CONTROL_MOVE_FORWARD) && !GetInputFlag(B_CONTROL_MOVE_BACKWARD))
						SetInputFlag(B_CONTROL_LADDER_INVERTED, true);
				}
			}
		}
		if (flagChanged && plAvatarInputStateMsg::IsCodeInMap(moveCode))
			SynchInputState();
	}
	return true;
}

void plArmatureMod::IHandleInputStateMsg(plAvatarInputStateMsg *msg)
{
	int i;
	UInt32 curBit;
	for (i = 0, curBit = 0x1; i < plAvatarInputStateMsg::fMapSize; i++, curBit <<= 1)
	{
		SetInputFlag(msg->fCodeMap[i], msg->fState & curBit);
	}
	
}

void plArmatureMod::SynchInputState(UInt32 rcvID /* = kInvalidPlayerID */)
{
	if (plAvatarMgr::GetInstance()->GetLocalAvatar() != this)
		return;
	
	plAvatarInputStateMsg *msg = TRACKED_NEW plAvatarInputStateMsg();
	int i;
	UInt32 curBit;
	for (i = 0, curBit = 0x1; i < plAvatarInputStateMsg::fMapSize; i++, curBit <<= 1)
	{
		if (GetInputFlag(msg->fCodeMap[i]))
			msg->fState |= curBit;
	}
	msg->AddReceiver(GetKey());
	msg->SetBCastFlag(plMessage::kNetPropagate);
	msg->SetBCastFlag(plMessage::kNetUseRelevanceRegions);
	msg->SetBCastFlag(plMessage::kLocalPropagate, false);
	msg->SetBCastFlag(plMessage::kNetSendUnreliable, true);
	if (rcvID != kInvalidPlayerID)
		msg->AddNetReceiver(rcvID);
	
	msg->Send();
	
	fLastInputSynch = hsTimer::GetSysSeconds();
}

void plArmatureMod::ILinkToPersonalAge()
{
	plNetClientMgr * nc = plNetClientMgr::GetInstance();
	
	plAgeLinkStruct link;
	link.GetAgeInfo()->SetAgeFilename( kPersonalAgeFilename );
	link.GetAgeInfo()->SetAgeInstanceName( kPersonalAgeInstanceName );
	
	plSpawnPointInfo hutSpawnPoint;
	hutSpawnPoint.SetName(kPersonalAgeLinkInPointCloset);
	link.SetSpawnPoint(hutSpawnPoint);
	
	link.SetLinkingRules( plNetCommon::LinkingRules::kOriginalBook );
	plLinkToAgeMsg* pMsg = TRACKED_NEW plLinkToAgeMsg( &link );
	pMsg->SetLinkInAnimName("PersonalBookEnter");
	pMsg->AddReceiver(nc->GetKey());
	pMsg->Send();	
}

hsBool plArmatureMod::IEval(double time, hsScalar elapsed, UInt32 dirty)
{
	if (IsFinal())
	{
		bool noOverlap = false;

		const plArmatureMod *localPlayer = plAvatarMgr::GetInstance()->GetLocalAvatar();
		if (plRelevanceMgr::Instance()->GetEnabled() && (localPlayer != nil))
		{
			// (May decide to update this elsewhere instead.)
			plRelevanceMgr::Instance()->SetRegionVectors(GetTarget(0)->GetLocalToWorld().GetTranslate(), fRegionsImIn, fRegionsICareAbout);	
			if (localPlayer != this)
			{
				if (!fRegionsImIn.Overlap(localPlayer->fRegionsICareAbout))
				{
					noOverlap = true;
				}
			}
			else // Bookkeeping for the local player...
			{
				hsBool update = false;
				if (fOldRegionsICareAbout != fRegionsICareAbout)
				{
					update = true;
					fOldRegionsICareAbout = fRegionsICareAbout;
				}
				if (fOldRegionsImIn != fRegionsImIn)
				{
					update = true;
					fOldRegionsImIn = fRegionsImIn;
				}
				if (update)
				{
					// Send message to the server here.
					plNetMsgRelevanceRegions relRegionsNetMsg;
					relRegionsNetMsg.SetNetProtocol(kNetProtocolCli2Game);
					relRegionsNetMsg.SetRegionsICareAbout(fRegionsICareAbout);
					relRegionsNetMsg.SetRegionsImIn(fRegionsImIn);
					plNetClientApp::GetInstance()->SendMsg(&relRegionsNetMsg);
				}
			}
		}
	
		if (localPlayer == this)
		{
			if (time - fLastInputSynch > kAvatarInputSynchThreshold)
				SynchInputState();			
		}
		
		if (noOverlap)
		{
			EnablePhysics(false, kDisableReasonRelRegion);
			EnableDrawing(false, kDisableReasonRelRegion);
		}
		else
		{
			EnablePhysics(true, kDisableReasonRelRegion);
			EnableDrawing(true, kDisableReasonRelRegion);
		}
		
		if (fMouseFrameTurnStrength == 0 && GetInputFlag(A_CONTROL_TURN))
			SetInputFlag(A_CONTROL_TURN, false);
		
		if (!fMidLink)
			plArmatureModBase::IEval(time, elapsed, dirty);
		
		fUpdateMsg->Ref();
		fUpdateMsg->Send();

		if (fPendingSynch)
			NetworkSynch(time, false);
		if (fDebugOn)
			RefreshDebugDisplay();
	
		fMouseFrameTurnStrength = 0.f; // Processed this frame. Clear it.

		// update our attached particle system if necessary
		if (GetFollowerParticleSystemSO())
		{
			plSceneObject* follower = GetFollowerParticleSystemSO();
			hsPoint3 trans = GetTarget(0)->GetLocalToWorld().GetTranslate() - follower->GetLocalToWorld().GetTranslate();
			if (trans.MagnitudeSquared() > 1) // we can be a bit fuzzy about this, since the particle system is rather large and people won't notice it being off
			{
				plWarpMsg *warp = TRACKED_NEW plWarpMsg(GetKey(), follower->GetKey(), plWarpMsg::kFlushTransform | plWarpMsg::kZeroVelocity,
					GetTarget(0)->GetLocalToWorld());
				warp->Send();

				plParticleSystem *sys = const_cast<plParticleSystem*>(plParticleSystem::ConvertNoRef(follower->GetModifierByType(plParticleSystem::Index())));
				if (sys)
				{
					sys->fMiscFlags |= plParticleSystem::kParticleSystemAlwaysUpdate;
					sys->TranslateAllParticles(trans);
				}
			}
		}
	}
	else
		IFinalize();

	return true;
}

void plArmatureMod::AddTarget(plSceneObject* so)
{
	plArmatureModBase::AddTarget(so);

	plAvatarMgr::GetInstance()->AddAvatar(this);			// register for easy lookup
	plgDispatch::Dispatch()->RegisterForExactType(plAvatarMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plLinkEffectBCMsg::Index(), GetKey());

	// all avatars will register for the age loaded message.
	// only players need it, but we don't know that we're a player until it's too late to get it.
	// non-players will unregister when they learn the truth.
	if (IsLocallyOwned())
		plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
	
	// attach a clothingSDLModifier to handle clothing saveState
	delete fClothingSDLMod;
	fClothingSDLMod = TRACKED_NEW plClothingSDLModifier;
	fClothingSDLMod->SetClothingOutfit(GetClothingOutfit());	// ok if clothingOutfit is nil at this point
	so->AddModifier(fClothingSDLMod);

	// add avatar sdl modifier
	delete fAvatarSDLMod;
	fAvatarSDLMod = TRACKED_NEW plAvatarSDLModifier;
	so->AddModifier(fAvatarSDLMod);

	delete fAvatarPhysicalSDLMod;
	fAvatarPhysicalSDLMod = TRACKED_NEW plAvatarPhysicalSDLModifier;
	so->AddModifier(fAvatarPhysicalSDLMod);

	// At export time, this key will be nil. This is important, or else we'll overwrite the page the key comes from.
	if (fFootSoundSOKey != nil)
		hsgResMgr::ResMgr()->AddViaNotify(fFootSoundSOKey, TRACKED_NEW plAttachMsg(so->GetKey(), nil, plRefMsg::kOnRequest), plRefFlags::kActiveRef);
	if (fLinkSoundSOKey != nil)
		hsgResMgr::ResMgr()->AddViaNotify(fLinkSoundSOKey, TRACKED_NEW plAttachMsg(so->GetKey(), nil, plRefMsg::kOnRequest), plRefFlags::kActiveRef);

	if (fUpdateMsg)
		fUpdateMsg->UnRef(); // delete an old one.
	
	fUpdateMsg = TRACKED_NEW plArmatureUpdateMsg(GetKey(), so->IsLocallyOwned(), true, this);		
}

void plArmatureMod::RemoveTarget(plSceneObject* so)
{
	if (so)
	{
		if (fClothingSDLMod)
			so->RemoveModifier(fClothingSDLMod);
		if (fAvatarSDLMod)
			so->RemoveModifier(fAvatarSDLMod);
		if (fAvatarPhysicalSDLMod)
			so->RemoveModifier(fAvatarPhysicalSDLMod);
	}
	delete fClothingSDLMod;
	fClothingSDLMod = nil;
	delete fAvatarSDLMod;
	fAvatarSDLMod = nil;
	delete fAvatarPhysicalSDLMod;
	fAvatarPhysicalSDLMod = nil;

	plArmatureModBase::RemoveTarget(so);
}

void plArmatureMod::Write(hsStream *stream, hsResMgr *mgr)
{
	// Temporarily going around plArmatureModBase so that we don't
	// break format
	plAGMasterMod::Write(stream, mgr);

	mgr->WriteKey(stream, fMeshKeys[0]);
	stream->WriteSafeString(fRootName);
	int nBrains = fBrains.size();
	stream->WriteSwap32(nBrains);
	for (int i = 0; i < nBrains; i++)
		mgr->WriteCreatable(stream, fBrains[i]);

	if (fClothingOutfit == nil)
	{
		stream->WriteBool( false );
	}
	else
	{
		stream->WriteBool( true );
		mgr->WriteKey(stream, fClothingOutfit->GetKey());
	}

	stream->WriteSwap32(fBodyType);
	if( fEffects == nil )
		stream->WriteBool( false );
	else
	{
		stream->WriteBool( true );
		mgr->WriteKey(stream, fEffects->GetKey());
	}

	stream->WriteSwapFloat(fPhysHeight);
	stream->WriteSwapFloat(fPhysWidth);

	stream->WriteSafeString(fAnimationPrefix.c_str());
	stream->WriteSafeString(fBodyAgeName.c_str());
	stream->WriteSafeString(fBodyFootstepSoundPage.c_str());
}

void plArmatureMod::Read(hsStream * stream, hsResMgr *mgr)
{
	// Temporarily going around plArmatureModBase so that we don't
	// break format
	plAGMasterMod::Read(stream, mgr);

	fMeshKeys.push_back(mgr->ReadKey(stream));

	// read the root name string
	fRootName = stream->ReadSafeString();

	// read in the brains
	int nBrains = stream->ReadSwap32();
	for (int i = 0; i < nBrains; i++)
	{
		plArmatureBrain * brain = (plArmatureBrain *)mgr->ReadCreatable(stream);
		this->PushBrain(brain);
	}
		
	if( stream->ReadBool() )
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // plClothingBase		
	else
		fClothingOutfit = nil;

	fBodyType = stream->ReadSwap32();

	if( stream->ReadBool() )
	{
		plKey effectMgrKey = mgr->ReadKey(stream);
		mgr->AddViaNotify(effectMgrKey, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // plArmatureEffects		

		// Attach the Footstep emitter scene object
		hsResMgr *mgr = hsgResMgr::ResMgr();
		const char *age = fBodyAgeName.c_str();
		const char *page = fBodyFootstepSoundPage.c_str();
		const plLocation &gLoc = plKeyFinder::Instance().FindLocation(age, page);
		
		if (gLoc.IsValid())
		{
			const plUoid &myUoid = GetKey()->GetUoid();
			plUoid SOUoid(gLoc, plSceneObject::Index(), "FootstepSoundObject");
			fFootSoundSOKey = mgr->FindKey(SOUoid);
			if (fFootSoundSOKey)
			{
				// So it exists... but FindKey won't properly create our clone. So we do.
				SOUoid.SetClone(myUoid.GetClonePlayerID(), myUoid.GetCloneID());
				fFootSoundSOKey = mgr->ReRegister(nil, SOUoid);
			}

			// Add the effect to our effects manager
			plUoid effectUoid(gLoc, plArmatureEffectFootSound::Index(), "FootstepSounds" );
			plKey effectKey = mgr->FindKey(effectUoid);
			if (effectKey)
			{
				effectUoid.SetClone(myUoid.GetClonePlayerID(), myUoid.GetCloneID());
				effectKey = mgr->ReRegister(nil, effectUoid);
			}
			if (effectKey != nil)
				mgr->AddViaNotify(effectKey, TRACKED_NEW plGenRefMsg(effectMgrKey, plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);

			// Get the linking sound
			plUoid LinkUoid(gLoc, plSceneObject::Index(), "LinkSoundSource");
			fLinkSoundSOKey = mgr->FindKey(LinkUoid);
			if (fLinkSoundSOKey)
			{
				LinkUoid.SetClone(myUoid.GetClonePlayerID(), myUoid.GetCloneID());
				fLinkSoundSOKey = mgr->ReRegister(nil, LinkUoid);
			}
		}
	}
	else
		fEffects = nil;

	fPhysHeight = stream->ReadSwapFloat();
	fPhysWidth = stream->ReadSwapFloat();

	char* temp = stream->ReadSafeString();
	fAnimationPrefix = temp;
	delete [] temp;

	temp = stream->ReadSafeString();
	fBodyAgeName = temp;
	delete [] temp;

	temp = stream->ReadSafeString();
	fBodyFootstepSoundPage = temp;
	delete [] temp;
	
	plgDispatch::Dispatch()->RegisterForExactType(plAvatarStealthModeMsg::Index(), GetKey());
}

hsBool plArmatureMod::DirtySynchState(const char* SDLStateName, UInt32 synchFlags)
{
	// skip requests to synch non-avatar state
	if (SDLStateName && stricmp(SDLStateName, kSDLAvatar))
	{
		return false;
	}
	
	synchFlags |= plSynchedObject::kForceFullSend;	// TEMP
	
	if(GetNumTargets() > 0)
	{
		plSceneObject *sObj = GetTarget(0);
		if(sObj) 
			sObj->DirtySynchState(SDLStateName, synchFlags);
	}
	return false;
}

hsBool plArmatureMod::DirtyPhysicalSynchState(UInt32 synchFlags)
{
	synchFlags |= plSynchedObject::kForceFullSend;	// TEMP
	synchFlags |= plSynchedObject::kBCastToClients;
	
	if(GetNumTargets() > 0)
	{
		plSceneObject *sObj = GetTarget(0);
		if(sObj)
			sObj->DirtySynchState(kSDLAvatarPhysical, synchFlags);
	}
	return false;
}

void plArmatureMod::IFinalize()
{
	plArmatureModBase::IFinalize();
	
	if (fWaitFlags & kNeedAudio)
	{
		plSetListenerMsg *msg = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVelocity, GetTarget(0)->GetKey(), true );
		msg->Send();
		fWaitFlags &= ~kNeedAudio;
	}
	
	if (fWaitFlags & kNeedCamera)
	{
		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
		pMsg->SetCmd(plCameraMsg::kCreateNewDefaultCam);
		pMsg->SetCmd(plCameraMsg::kSetSubject);
		pMsg->SetSubject(GetTarget(0));
		pMsg->SetBCastFlag( plMessage::kBCastByExactType );
		pMsg->Send();
		fWaitFlags &= ~kNeedCamera;
	}	

	if (fWaitFlags & kNeedSpawn)
	{
		Spawn(hsTimer::GetSysSeconds());
		fWaitFlags &= ~kNeedSpawn;
	}
}

void plArmatureMod::ICustomizeApplicator()
{
	plArmatureModBase::ICustomizeApplicator();

	const plAGModifier *agMod = GetChannelMod("Bone_Root", true);
	if (agMod)
	{
		// are there any applicators that manipulate the transform?
		plAGApplicator *app = agMod->GetApplicator(kAGPinTransform);
		if(app)
		{
			plMatrixDelayedCorrectionApplicator *corApp = plMatrixDelayedCorrectionApplicator::ConvertNoRef(app);
			if (corApp)
			{
				fBoneRootAnimator = corApp;
				fWaitFlags &= ~kNeedApplicator;
				return;	// already there
			}
		}
		plAGModifier *volAGMod = const_cast<plAGModifier *>(agMod);
		fBoneRootAnimator = TRACKED_NEW plMatrixDelayedCorrectionApplicator();
		volAGMod->SetApplicator(fBoneRootAnimator);
		fWaitFlags &= ~kNeedApplicator;
	}		
}	

const plSceneObject *plArmatureMod::GetClothingSO(UInt8 lod) const 
{
	if (fClothToSOMap.GetCount() <= lod)
		return nil;

	return fClothToSOMap[lod];
}

#define kSynchInterval 1	// synch once per second

void plArmatureMod::NetworkSynch(double timeNow, int force)
{
	if (force || ((timeNow - fLastSynch) > kSynchInterval))
	{
		// make sure state change gets sent out over the network
		// avatar state should use relevance region filtering
		UInt32 flags = kBCastToClients | kUseRelevanceRegions;
		if (force)
			flags |= kForceFullSend;
		DirtyPhysicalSynchState(flags);
		fLastSynch = timeNow;
		fPendingSynch = false;
	}
	else
		fPendingSynch = true;
}

hsBool plArmatureMod::IsLocalAvatar()
{
	return plAvatarMgr::GetInstance()->GetLocalAvatar() == this;
}

hsBool plArmatureMod::IsLocalAI()
{
	plAvBrainCritter* ai = plAvBrainCritter::ConvertNoRef(FindBrainByClass(plAvBrainCritter::Index()));
	if (ai)
		return ai->LocallyControlled();
	return false; // not an AI, obviously not local
}

void plArmatureMod::SynchIfLocal(double timeNow, int force)
{
	if (IsLocalAvatar() || IsLocalAI())
	{
		NetworkSynch(timeNow, force);
	}
}

plLayerLinkAnimation *plArmatureMod::IFindLayerLinkAnim()
{
	int i;
	hsGMaterial *mat = fClothingOutfit->fMaterial;
	if (!mat)
		return nil;

	for (i = 0; i < mat->GetNumLayers(); i++)
	{
		plLayerInterface *li = mat->GetLayer(i);
		while (li != nil)
		{
			plLayerLinkAnimation *anim = plLayerLinkAnimation::ConvertNoRef(li);
			if (anim)
				return anim;

			li = li->GetUnderLay();
		}
	}
	return nil;
}

hsBool plArmatureMod::ValidatePhysics()
{
	if (!fTarget)
		return false;

	if (!fController)
		fController = plPhysicalControllerCore::Create(GetTarget(0)->GetKey(), fPhysHeight, fPhysWidth);

	if (fController)
	{
		if (GetTarget(0)->GetKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
		{
			// local avatars get added to a special LOS db just for them.
			fController->SetLOSDB(plSimDefs::kLOSDBLocalAvatar);
		}
		else
		{
			// non-local avatars are in the same LOS db as clickables
			fController->SetLOSDB(plSimDefs::kLOSDBUIItems);
		}

		hsClearBits(fWaitFlags, kNeedPhysics);
		return true;
	}
	else
	{
		return false;
	}
	
}

hsBool plArmatureMod::ValidateMesh()
{
	if (fWaitFlags & kNeedMesh)
	{
		fWaitFlags &= ~kNeedMesh;
		int n = fMeshKeys.size();
		
		for(int i = 0; i < n; i++)
		{
			plKey meshKey = fMeshKeys[i];
			plSceneObject * meshObj = (plSceneObject *)meshKey->GetObjectPtr();
			
			if( ! meshObj)
			{
				fWaitFlags |= kNeedMesh;
				break;
			}
			hsBool visible = (i == fCurLOD) ? true : false;
			
			EnableDrawingTree(meshObj, visible);
			
			// If we haven't created the mapping yet...
			if (fClothToSOMap.GetCount() <= i || fClothToSOMap[i] == nil)
			{
				plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, i, 0);
				hsgResMgr::ResMgr()->SendRef(meshObj->GetKey(), refMsg, plRefFlags::kPassiveRef); 
			}
		}
		if (!strcmp(GetTarget(0)->GetKeyName(), "Yeesha"))
			ISetTransparentDrawOrder(true);
		else
			ISetTransparentDrawOrder(false);
	}
		
	return !(fWaitFlags & kNeedMesh);
}

plArmatureBrain * plArmatureMod::GetNextBrain(plArmatureBrain *brain)
{
	plArmatureBrain * result = nil;
	bool passedTarget = false;

	int count = fBrains.size();
	for(int i = count - 1; i >= 0; i--)
	{
		plArmatureBrain *curBrain = fBrains.at(i);
		if(passedTarget)
			result = curBrain;
		else {
			if(curBrain == brain)
				passedTarget = true;
		}
	}

	return result;
}

int plArmatureMod::GetBrainCount()
{
	return fBrains.size();
}

plArmatureBrain * plArmatureMod::FindBrainByClass(UInt32 classID) const
{
	int n = fBrains.size();

	for(int i = 0; i < n; i++)
	{
		plArmatureBrain *brain = fBrains.at(i);

		if(brain->ClassIndex() == classID)
		{
			return brain;
		}
	}
	return nil;
}

void plArmatureMod::TurnToPoint(hsPoint3 &point)
{
	plAvBrainHuman *brain = plAvBrainHuman::ConvertNoRef(FindBrainByClass(plAvBrainHuman::Index()));
	if (brain)
		brain->TurnToPoint(point);
}

void plArmatureMod::SuspendInput()
{
	if (fSuspendInputCount == 0 && plNetClientApp::GetInstance()->GetLocalPlayer() == GetTarget(0))
	{
		plAvatarInputInterface::GetInstance()->SuspendMouseMovement();
		PreserveInputState();
		ClearInputFlags(false, false);
	}

	fSuspendInputCount++;
}

void plArmatureMod::ResumeInput()
{
	if (fSuspendInputCount > 0) // decrementing an unsigned variable set to 0 is BAD
		fSuspendInputCount--;
	if(fSuspendInputCount == 0)
	{
		RestoreInputState();
		IProcessQueuedInput();
		if (plNetClientApp::GetInstance()->GetLocalPlayer() == GetTarget(0))
			plAvatarInputInterface::GetInstance()->EnableMouseMovement();		
	}
}

void plArmatureMod::IProcessQueuedInput()
{
	CtrlMessageVec::iterator i = fQueuedCtrlMessages.begin();
	CtrlMessageVec::iterator end = fQueuedCtrlMessages.end();
	while(i != end)
	{
		plControlEventMsg *ctrlMsg = *i;
		IHandleControlMsg(ctrlMsg);
		ctrlMsg->UnRef();
		(*i) = nil;
		i++;
	}
	fQueuedCtrlMessages.clear();
}

void plArmatureMod::PreserveInputState()
{
	fMoveFlagsBackup = fMoveFlags;
}

void plArmatureMod::RestoreInputState()
{
	fMoveFlags = fMoveFlagsBackup;

}

hsBool plArmatureMod::GetInputFlag (int which) const
{
	return fMoveFlags.IsBitSet(which);
}

void plArmatureMod::SetInputFlag(int which, hsBool status)
{
	if(status)
	{
		fMoveFlags.SetBit(which);
	} else {
		fMoveFlags.ClearBit(which);
	}
}

hsBool plArmatureMod::HasMovementFlag() const
{
	return (fMoveFlags.IsBitSet(B_CONTROL_MOVE_FORWARD) ||
		    fMoveFlags.IsBitSet(B_CONTROL_MOVE_BACKWARD) ||
		    fMoveFlags.IsBitSet(B_CONTROL_ROTATE_LEFT) ||
		    fMoveFlags.IsBitSet(B_CONTROL_ROTATE_RIGHT) ||
			fMoveFlags.IsBitSet(A_CONTROL_TURN) ||
			fMoveFlags.IsBitSet(B_CONTROL_STRAFE_LEFT) ||
			fMoveFlags.IsBitSet(B_CONTROL_STRAFE_RIGHT));
}

bool plArmatureMod::ForwardKeyDown() const
{
	return GetInputFlag(B_CONTROL_MOVE_FORWARD) ? true : false;
}

bool plArmatureMod::BackwardKeyDown() const
{
	return GetInputFlag(B_CONTROL_MOVE_BACKWARD) ? true : false;
}

bool plArmatureMod::StrafeLeftKeyDown() const
{
	return GetInputFlag(B_CONTROL_STRAFE_LEFT) ? true : false;
}

bool plArmatureMod::StrafeRightKeyDown() const
{
	return GetInputFlag(B_CONTROL_STRAFE_RIGHT) ? true : false;
}

bool plArmatureMod::FastKeyDown() const
{
	return ((GetInputFlag(B_CONTROL_MODIFIER_FAST) && !GetInputFlag(B_CONTROL_ALWAYS_RUN)) ||
			(!GetInputFlag(B_CONTROL_MODIFIER_FAST) && GetInputFlag(B_CONTROL_ALWAYS_RUN)));
}

bool plArmatureMod::StrafeKeyDown() const
{
	return GetInputFlag(B_CONTROL_MODIFIER_STRAFE) ? true : false;
}

bool plArmatureMod::TurnLeftKeyDown() const
{
	return GetInputFlag(B_CONTROL_ROTATE_LEFT) ? true : false;
}

bool plArmatureMod::TurnRightKeyDown() const
{
	return GetInputFlag(B_CONTROL_ROTATE_RIGHT) ? true : false;
}

bool plArmatureMod::JumpKeyDown() const
{
	return GetInputFlag(B_CONTROL_JUMP) ? true : false;
}

bool plArmatureMod::ExitModeKeyDown() const
{
	return GetInputFlag(B_CONTROL_EXIT_MODE) ? true : false;
}

void plArmatureMod::SetForwardKeyDown()
{
	SetInputFlag(B_CONTROL_MOVE_FORWARD, true);
}

void plArmatureMod::SetBackwardKeyDown()
{
	SetInputFlag(B_CONTROL_MOVE_BACKWARD, true);
}

void plArmatureMod::SetStrafeLeftKeyDown(bool status)
{
	SetInputFlag(B_CONTROL_STRAFE_LEFT, status);
}

void plArmatureMod::SetStrafeRightKeyDown(bool status)
{
	SetInputFlag(B_CONTROL_STRAFE_RIGHT, status);
}

void plArmatureMod::SetFastKeyDown()
{
	SetInputFlag(B_CONTROL_MODIFIER_FAST, true);
}

void plArmatureMod::SetTurnLeftKeyDown(bool status)
{
	SetInputFlag(B_CONTROL_ROTATE_LEFT, status);
}

void plArmatureMod::SetTurnRightKeyDown(bool status)
{
	SetInputFlag(B_CONTROL_ROTATE_RIGHT, status);
}

void plArmatureMod::SetJumpKeyDown()
{
	SetInputFlag(B_CONTROL_JUMP, true);
}

hsScalar plArmatureMod::GetTurnStrength() const
{
	return GetKeyTurnStrength() + GetAnalogTurnStrength();
}

hsScalar plArmatureMod::GetKeyTurnStrength() const
{
	if (StrafeKeyDown())
		return 0.f;	

	return (TurnLeftKeyDown() ? 1.f : 0.f) + (TurnRightKeyDown() ? -1.f: 0.f);
}

hsScalar plArmatureMod::GetAnalogTurnStrength() const
{
	if (StrafeKeyDown())
		return 0.f;

	hsScalar elapsed = hsTimer::GetDelSysSeconds();
	if (elapsed > 0)
		return fMouseFrameTurnStrength / elapsed;
	else
		return 0;
}	

void plArmatureMod::SetReverseFBOnIdle(bool val)
{
	fReverseFBOnIdle = val;
	if (val)
		SetInputFlag(B_CONTROL_LADDER_INVERTED, !(ForwardKeyDown() || BackwardKeyDown()));
	else
		SetInputFlag(B_CONTROL_LADDER_INVERTED, false);
}
		
hsBool plArmatureMod::IsFBReversed()
{ 
	return GetInputFlag(B_CONTROL_LADDER_INVERTED); 
}

void plArmatureMod::ClearInputFlags(bool saveAlwaysRun, bool clearBackup)
{
	bool alwaysRun = (fMoveFlags.IsBitSet(B_CONTROL_ALWAYS_RUN) != 0);
	bool fast = (fMoveFlags.IsBitSet(B_CONTROL_MODIFIER_FAST) != 0);
	fMoveFlags.Clear();

	if (saveAlwaysRun)
	{
		if (alwaysRun)
			fMoveFlags.SetBit(B_CONTROL_ALWAYS_RUN);
		if (fast)
			fMoveFlags.SetBit(B_CONTROL_MODIFIER_FAST);
	}

	if (clearBackup)
	{
		alwaysRun = (fMoveFlagsBackup.IsBitSet(B_CONTROL_ALWAYS_RUN) != 0);
		fast = (fMoveFlagsBackup.IsBitSet(B_CONTROL_MODIFIER_FAST) != 0);
		fMoveFlagsBackup.Clear();

		if (saveAlwaysRun)
		{
			if (alwaysRun)
				fMoveFlagsBackup.SetBit(B_CONTROL_ALWAYS_RUN);
			if (fast)
				fMoveFlagsBackup.SetBit(B_CONTROL_MODIFIER_FAST);
		}
	}
}

plAGModifier * plArmatureMod::GetRootAGMod()
{
	if(fRootAGMod)
		return fRootAGMod;

	if(fTarget)
	{
		const plAGModifier * shit = plAGModifier::ConvertNoRef(FindModifierByClass(fTarget, plAGModifier::Index()));
		fRootAGMod = const_cast<plAGModifier *>(shit);
	}

	return fRootAGMod;
}

int plArmatureMod::GetCurrentGenericType()
{
	plAvBrainGeneric *brain = plAvBrainGeneric::ConvertNoRef(GetCurrentBrain());

	if (!brain)
		return plAvBrainGeneric::kNonGeneric;
	else
		return brain->GetType();
}

bool plArmatureMod::FindMatchingGenericBrain(const char *names[], int count)
{
	int i;
	for (i = 0; i < GetBrainCount(); i++)
	{
		plAvBrainGeneric *brain = plAvBrainGeneric::ConvertNoRef(GetBrain(i));
		if (brain && brain->MatchAnimNames(names, count))
			return true;
	}
	return false;
}

char *plArmatureMod::MakeAnimationName(const char *baseName) const
{
	std::string temp = fAnimationPrefix + baseName;
	char *result = hsStrcpy(temp.c_str()); // why they want a new string I'll never know... but hey, too late to change it now
	return result;
}

char *plArmatureMod::GetRootName()
{
	return fRootName;
}

void plArmatureMod::SetRootName(const char *name)
{
	delete [] fRootName;
	fRootName = hsStrcpy(name);
}

plAGAnim *plArmatureMod::FindCustomAnim(const char *baseName) const
{
	char *customName = MakeAnimationName(baseName);
	plAGAnim *result = plAGAnim::FindAnim(customName);
	delete[] customName;
	return result;
}

void plArmatureMod::ISetupMarkerCallbacks(plATCAnim *anim, plAnimTimeConvert *atc)
{
	std::vector<char*> markers;
	anim->CopyMarkerNames(markers);

	int i;
	for (i = 0; i < markers.size(); i++)
	{
		
		hsScalar time = -1;
		hsBool isLeft = false;
		if (strstr(markers[i], "SndLeftFootDown") == markers[i])
		{
			isLeft = true;		
			time = anim->GetMarker(markers[i]);
		}
		if (strstr(markers[i], "SndRightFootDown") == markers[i])
			time = anim->GetMarker(markers[i]);

		if (time >= 0)
		{
			plEventCallbackInterceptMsg *iMsg;

			plArmatureEffectMsg *msg = TRACKED_NEW plArmatureEffectMsg(fEffects->GetKey(), kTime);
			msg->fEventTime = time;
			msg->fTriggerIdx = AnimNameToIndex(anim->GetName());
			
			iMsg = TRACKED_NEW plEventCallbackInterceptMsg();
			iMsg->AddReceiver(fEffects->GetKey());
			iMsg->fEventTime = time;
			iMsg->fEvent = kTime;
			iMsg->SetMessage(msg);
			atc->AddCallback(iMsg);
			hsRefCnt_SafeUnRef(msg);
			hsRefCnt_SafeUnRef(iMsg);

			plAvatarFootMsg* foot = TRACKED_NEW plAvatarFootMsg(GetKey(), this, isLeft);
			foot->fEventTime = time;

			iMsg = TRACKED_NEW plEventCallbackInterceptMsg();
			iMsg->AddReceiver(fEffects->GetKey());
			iMsg->fEventTime = time;
			iMsg->fEvent = kTime;
			iMsg->SetMessage(foot);
			atc->AddCallback(iMsg);
			hsRefCnt_SafeUnRef(foot);
			hsRefCnt_SafeUnRef(iMsg);
		}

		delete [] markers[i]; // done with this string, nuke it
	}
}

const char *plArmatureMod::GetAnimRootName(const char *name)
{
	return name + fAnimationPrefix.length();
}

Int8 plArmatureMod::AnimNameToIndex(const char *name)
{
	const char *rootName = GetAnimRootName(name);
	Int8 result = -1;
	
	if (!strcmp(rootName, "Walk") || !strcmp(rootName, "WalkBack") ||
		!strcmp(rootName, "LadderDown") || !strcmp(rootName, "LadderDownOn") ||
		!strcmp(rootName, "LadderDownOff") || !strcmp(rootName, "LadderUp") ||
		!strcmp(rootName, "LadderUpOn") || !strcmp(rootName, "LadderUpOff") ||
		!strcmp(rootName, "SwimSlow") || !strcmp(rootName, "SwimBackward") ||
		!strcmp(rootName, "BallPushWalk"))
		result = kWalk;
	else if (!strcmp(rootName, "Run") || !strcmp(rootName, "SwimFast"))
		result = kRun;
	else if (!strcmp(rootName, "TurnLeft") || !strcmp(rootName, "TurnRight") ||
			 !strcmp(rootName, "StepLeft") || !strcmp(rootName, "StepRight") ||
			 !strcmp(rootName, "SideSwimLeft") || !strcmp(rootName, "SideSwimRight") ||
			 !strcmp(rootName, "TreadWaterTurnLeft") || !strcmp(rootName, "TreadWaterTurnRight"))
		result = kTurn;
	else if (!strcmp(rootName, "GroundImpact") || !strcmp(rootName, "RunningImpact"))
		result = kImpact;
	else if (strstr(rootName, "Run")) // Critters
		result = kRun;
	else if (strstr(rootName, "Idle")) // Critters
		result = kWalk;
		
	return result;
}

hsBool plArmatureMod::IsInStealthMode() const 
{ 
	return (fStealthMode != plAvatarStealthModeMsg::kStealthVisible);
}

bool plArmatureMod::IsOpaque()
{
	return fOpaque;
}

bool plArmatureMod::IsMidLink()
{
	return fMidLink;
}

hsBool plArmatureMod::ConsumeJump()
{
	if (!GetInputFlag(B_CONTROL_CONSUMABLE_JUMP))
		return false;

	SetInputFlag(B_CONTROL_CONSUMABLE_JUMP, false);
	return true;
}

void plArmatureMod::ISetTransparentDrawOrder(bool val)
{
	if (fOpaque != val)
		return;

	fOpaque = !val;

	plDrawableSpans *spans = plDrawableSpans::ConvertNoRef(FindDrawable());
	if (spans)
	{
		spans->SetNativeProperty(plDrawable::kPropPartialSort, true);
		spans->SetNativeProperty(plDrawable::kPropSortAsOne, true);
		if (val)
		{
			spans->SetNativeProperty(plDrawable::kPropSortSpans, true);
			spans->SetRenderLevel(plRenderLevel(plRenderLevel::kBlendRendMajorLevel, plRenderLevel::kDefRendMinorLevel));
		}
		else
		{
			spans->SetNativeProperty(plDrawable::kPropSortSpans, false);
			spans->SetRenderLevel(plRenderLevel(0, plRenderLevel::kAvatarRendMinorLevel));			
		}
	}
}

bool plArmatureMod::IsKILowestLevel()
{
	if ( GetKILevel() == pfKIMsg::kNanoKI )
		return true;
	else
		return false;
}

int  plArmatureMod::GetKILevel()
{
	return VaultGetKILevel();
}

void plArmatureMod::SetLinkInAnim(const char *animName)
{
	if (animName)
	{
		plAGAnim *anim = FindCustomAnim(animName);
		fLinkInAnimKey = (anim ? anim->GetKey() : nil);
	}
	else
		fLinkInAnimKey = nil;
}

plKey plArmatureMod::GetLinkInAnimKey() const
{
	return fLinkInAnimKey;
}


//////////////////////
//
//  PLARMATURELODMOD
//
//////////////////////

plArmatureLODMod::plArmatureLODMod()
{
}

// CTOR (physical, name)
plArmatureLODMod::plArmatureLODMod(const char* root_name)
: plArmatureMod()
{
	fRootName = hsStrcpy(root_name);
}

plArmatureLODMod::~plArmatureLODMod()
{
}

void plArmatureLODMod::Read(hsStream *stream, hsResMgr *mgr)
{
	plArmatureMod::Read(stream, mgr);

	fMeshKeys.clear();
	int meshKeyCount = stream->ReadSwap32();
	for(int i = 0; i < meshKeyCount; i++)
	{
		plKey meshKey = mgr->ReadKey(stream);
		fMeshKeys.push_back(meshKey);
		
		plKeyVector *vec = TRACKED_NEW plKeyVector;
		int boneCount = stream->ReadSwap32();
		for(int j = 0; j < boneCount; j++)
			vec->push_back(mgr->ReadKey(stream));
		fUnusedBones.push_back(vec);
	}
}

// WRITE
void plArmatureLODMod::Write(hsStream *stream, hsResMgr *mgr)
{
	plArmatureMod::Write(stream, mgr);
	
	int meshKeyCount = fMeshKeys.size();
	stream->WriteSwap32(meshKeyCount);
	
	for(int i = 0; i < meshKeyCount; i++)
	{
		plKey meshKey = fMeshKeys[i];
		mgr->WriteKey(stream, meshKey);
		
		// Should be a list per mesh key
		stream->WriteSwap32(fUnusedBones[i]->size());
		for(int j = 0; j < fUnusedBones[i]->size(); j++)
			mgr->WriteKey(stream, (*fUnusedBones[i])[j]);
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG SUPPORT

int plArmatureMod::RefreshDebugDisplay()
{
	plDebugText		&debugTxt = plDebugText::Instance();
	char			strBuf[ 2048 ];
	int				lineHeight = debugTxt.GetFontSize() + 4;
	UInt32			scrnWidth, scrnHeight;

	debugTxt.GetScreenSize( &scrnWidth, &scrnHeight );
	int	y = 10;
	int x = 10;

	DumpToDebugDisplay(x, y, lineHeight, strBuf, debugTxt);
	return y;
}

void plArmatureMod::DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	sprintf(strBuf, "Armature <%s>:", fRootName);
	debugTxt.DrawString(x, y, strBuf, 255, 128, 128);
	y += lineHeight;

	plSceneObject * SO = GetTarget(0);
	if(SO)
	{
		// global location
		hsMatrix44  l2w = SO->GetLocalToWorld();
		hsPoint3 worldPos = l2w.GetTranslate();

		char *opaque = IsOpaque() ? "yes" : "no"; 
		sprintf(strBuf, "position(world): %.2f, %.2f, %.2f Opaque: %3s", 
				worldPos.fX, worldPos.fY, worldPos.fZ, opaque);
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;

		hsPoint3 kPos;
		char *kinematic = "n.a.";
		const char* frozen = "n.a.";
		if (fController)
		{
			fController->GetKinematicPosition(kPos);
			kinematic = fController->IsKinematic() ? "on" : "off";
		}
		sprintf(strBuf, "kinematc(world): %.2f, %.2f, %.2f  Kinematic: %3s",
			kPos.fX, kPos.fY, kPos.fZ,kinematic);
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;

		if (fController)
			frozen = fController->IsEnabled() ? "no" : "yes";

		// are we in a subworld?
		plKey world = nil;
		if (fController)
			world = fController->GetSubworld();
		sprintf(strBuf, "In world: %s  Frozen: %s", world ? world->GetName() : "nil", frozen);
		debugTxt.DrawString(x,y, strBuf);
		y+= lineHeight;

		if (fController)
		{
			hsPoint3 physPos;
			GetPositionAndRotationSim(&physPos, nil);
			const hsVector3& vel = fController->GetLinearVelocity();
			sprintf(strBuf, "position(physical): <%.2f, %.2f, %.2f> velocity: <%5.2f, %5.2f, %5.2f>", physPos.fX, physPos.fY, physPos.fZ, vel.fX, vel.fY, vel.fZ);
		}
		else
		{
			sprintf(strBuf, "position(physical): no controller");
		}
 		debugTxt.DrawString(x, y, strBuf);
 		y += lineHeight;
	}

	DebugDumpMoveKeys(x, y, lineHeight, strBuf, debugTxt);

	int i;
	for(i = 0; i < fBrains.size(); i++)
	{
		plArmatureBrain *brain = fBrains[i];
		brain->DumpToDebugDisplay(x, y, lineHeight, strBuf, debugTxt);
	}
	
	if (fClothingOutfit)
	{
		y += lineHeight;

		debugTxt.DrawString(x, y, "ItemsWorn:");
		y += lineHeight;
		strBuf[0] = '\0';
		int itemCount = 0; 
		for (i = 0; i < fClothingOutfit->fItems.GetCount(); i++)
		{
			if (itemCount == 0)
				strcat(strBuf, "    ");

			strcat(strBuf, fClothingOutfit->fItems[i]->fName);
			itemCount++;

			if (itemCount == 4)
			{
				debugTxt.DrawString(x, y, strBuf);
				itemCount = 0;
				strBuf[0] = '\0';
				y += lineHeight;
			}

			if (itemCount > 0)
				strcat(strBuf, ", ");
		}
		if (itemCount > 0)
		{
			debugTxt.DrawString(x, y, strBuf);
			y += lineHeight;
		}
	}

	if (plRelevanceMgr::Instance()->GetEnabled())
	{
		y += lineHeight;

		debugTxt.DrawString(x, y, "Relevance Regions:");
		y += lineHeight;
		sprintf(strBuf, "          In: %s", plRelevanceMgr::Instance()->GetRegionNames(fRegionsImIn).c_str());
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;
		sprintf(strBuf, "  Care about: %s", plRelevanceMgr::Instance()->GetRegionNames(fRegionsICareAbout).c_str());
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;
	}
}

class plAvBoneMap::BoneMapImp 
{
public:	
	typedef std::map<UInt32, const plSceneObject *> id2SceneObjectMap;
	id2SceneObjectMap fMap;
};

plAvBoneMap::plAvBoneMap()
{
	fImp = TRACKED_NEW BoneMapImp;
}

plAvBoneMap::~plAvBoneMap()
{
	delete fImp;
}

const plSceneObject * plAvBoneMap::FindBone(UInt32 boneID)
{
	BoneMapImp::id2SceneObjectMap::iterator i = fImp->fMap.find(boneID);
	const plSceneObject *result = nil;

	if(i != fImp->fMap.end())
	{
		result = (*i).second;
	}
	return result;
}

void plAvBoneMap::AddBoneMapping(UInt32 boneID, const plSceneObject *SO)
{
	(fImp->fMap)[boneID] = SO;
}

void plArmatureMod::DebugDumpMoveKeys(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	char buff[256];
	sprintf(buff, "Mouse Input Map: %s", plAvatarInputInterface::GetInstance()->GetInputMapName());
	debugTxt.DrawString(x, y, buff);
	y += lineHeight;
	
	sprintf(buff, "Turn strength: %.2f (key: %.2f, analog: %.2f)", GetTurnStrength(), GetKeyTurnStrength(), GetAnalogTurnStrength());
	debugTxt.DrawString(x, y, buff);
	y += lineHeight;
	
	GetMoveKeyString(buff);
	debugTxt.DrawString(x, y, buff);
	y += lineHeight;
}

void plArmatureMod::GetMoveKeyString(char *buff)
{
	sprintf(buff, "Move keys: ");

	if(FastKeyDown())
		strcat(buff, "FAST ");
	if(StrafeKeyDown())
		strcat(buff, "STRAFE ");
	if(ForwardKeyDown())
		strcat(buff, "FORWARD ");
	if(BackwardKeyDown())
		strcat(buff, "BACKWARD ");
	if(TurnLeftKeyDown())
		strcat(buff, "TURNLEFT ");
	if(TurnRightKeyDown())
		strcat(buff, "TURNRIGHT ");
	if(StrafeLeftKeyDown())
		strcat(buff, "STRAFELEFT ");
	if(StrafeRightKeyDown())
		strcat(buff, "STRAFERIGHT ");
	if(JumpKeyDown())
		strcat(buff, "JUMP ");
}
