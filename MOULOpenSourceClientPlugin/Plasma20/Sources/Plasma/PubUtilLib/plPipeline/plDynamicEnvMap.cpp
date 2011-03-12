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
#include "plDynamicEnvMap.h"

#include "plPipeline.h"
#include "plPipeDebugFlags.h"
#include "plDrawable.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "hsTimer.h"
#include "hsStream.h"

#include "../plMessage/plRenderRequestMsg.h"
#include "../plMessage/plDynamicEnvMapMsg.h"
#include "../pfCamera/plCameraModifier.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plLayRefMsg.h"
#include "../pnMessage/plPipeResMakeMsg.h"
#include "../pnMessage/plRefMsg.h"

#include "../plScene/plVisRegion.h"
#include "../plScene/plVisMgr.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plSurface/plLayer.h"

plDynamicEnvMap::plDynamicEnvMap()
:	fPos(0,0,0),
	fHither(0.3f),
	fYon(1000.f),
	fFogStart(1000.f),
	fRefreshRate(0.f),
	fLastRefresh(0.0),
	fLastRender(0),
	fOutStanding(0),
	fIncCharacters(false),
	fRootNode(nil)
{
	fColor.Set(0,0,0,1.f);
	int i;
	for( i = 0; i < 6; i++ )
		fReqMsgs[i] = TRACKED_NEW plRenderRequestMsg(nil, &fReqs[i]);;

	SetPosition(fPos);
}

plDynamicEnvMap::plDynamicEnvMap(UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth, UInt8 sDepth)
:	fPos(0,0,0),
	fHither(0.3f),
	fYon(0.f), // yon < hither means ignore and use current settings
	fFogStart(-1.f), // - fog start means use current settings
	fRefreshRate(0.f),
	fLastRefresh(0.0),
	fLastRender(0),
	fOutStanding(0),
	fIncCharacters(false),
	fRootNode(nil),
	plCubicRenderTarget(plRenderTarget::kIsTexture, width, height, bitDepth, zDepth, sDepth)
{
	fColor.Set(0,0,0,1.f);
	int i;
	for( i = 0; i < 6; i++ )
		fReqMsgs[i] = TRACKED_NEW plRenderRequestMsg(nil, &fReqs[i]);;

	SetPosition(fPos);
}

plDynamicEnvMap::~plDynamicEnvMap()
{
	SetDeviceRef(nil);

	int i;
	for( i = 0; i < 6; i++ )
		delete fReqMsgs[i];
}

void plDynamicEnvMap::Init()
{
	plgDispatch::Dispatch()->RegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());

	ISetupRenderRequests();
}

hsPoint3 plDynamicEnvMap::GetPosition() const
{
	if (fRootNode)
	{
		// This is to catch export issues where we've got a root node, but its iface
		// hasn't fully been set up yet.
		if (fRootNode->GetCoordinateInterface())
			return fRootNode->GetLocalToWorld().GetTranslate();
	}
	
	return fPos;
}

void plDynamicEnvMap::SetPosition(const hsPoint3& pos)
{
	hsAssert(fRootNode == nil, "Trying to override a cube map's root node.");
	fPos = pos;
	SetCameraMatrix(fPos);
}

void plDynamicEnvMap::IUpdatePosition()
{
	hsPoint3 pos = GetPosition();
	if (pos != fPos)
		SetCameraMatrix(fPos);
}

void plDynamicEnvMap::SetHither(hsScalar f)
{
	fHither = f;
}

void plDynamicEnvMap::SetYon(hsScalar f)
{
	fYon = f;
}

void plDynamicEnvMap::SetFogStart(hsScalar f)
{
	fFogStart = f;
}

void plDynamicEnvMap::SetColor(const hsColorRGBA& col)
{
	fColor = col;
}

void plDynamicEnvMap::SetRefreshRate(hsScalar secs)
{
	fRefreshRate = secs / 6.f;
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plDynamicEnvMap::ISetupRenderRequests()
{
	UInt32 renderState 
		= plPipeline::kRenderNormal
		| plPipeline::kRenderClearColor
		| plPipeline::kRenderClearDepth;

	int i;
	for( i = 0; i < 6; i++ )
	{
		fReqs[i].SetRenderState(renderState);

		fReqs[i].SetDrawableMask(plDrawable::kNormal);
		fReqs[i].SetSubDrawableMask(plDrawable::kSubAllTypes);

		fReqs[i].SetHither(fHither);
		fReqs[i].SetYon(fYon);

		fReqs[i].SetFogStart(fFogStart);

		fReqs[i].SetFovX(90.f);
		fReqs[i].SetFovY(90.f);

		fReqs[i].SetClearColor(fColor);
		fReqs[i].SetClearDepth(1.f);

		fReqs[i].SetClearDrawable(nil);
		fReqs[i].SetRenderTarget(GetFace(i));

		fReqs[i].SetCameraTransform(GetWorldToCamera(i), GetCameraToWorld(i));

		fReqs[i].SetVisForce(fVisSet);

		fReqs[i].RequestAck(GetKey());
	}
}

void plDynamicEnvMap::ISubmitRenderRequest(int i)
{
	IUpdatePosition();
	fReqMsgs[i]->SendAndKeep();
	fLastRender = i;
	fOutStanding++;
}

void plDynamicEnvMap::ISubmitRenderRequests()
{
	IUpdatePosition();
	int i;
	for( i = 0; i < 6; i++ )
		fReqMsgs[i]->SendAndKeep();
	fLastRefresh = hsTimer::GetSysSeconds();
	fOutStanding += 6;
}

void plDynamicEnvMap::ICheckForRefresh(double t, plPipeline *pipe)
{
	if( fLastRefresh <= 0 )
	{
		ISubmitRenderRequests();
		return;
	}

	if( fRefreshRate <= 0 )
		return;

#ifndef PLASMA_EXTERNAL_RELEASE
	if (pipe->IsDebugFlagSet(plPipeDbg::kFlagNVPerfHUD) && hsTimer::GetDelSysSeconds() == 0)
	{
		ISubmitRenderRequests();
		return;
	}
#endif // PLASMA_EXTERNAL_RELEASE

	if( t > fLastRefresh + 6.f * fRefreshRate )
	{
		ISubmitRenderRequests();
		return;
	}
	while( t > fLastRefresh + fRefreshRate )
	{
		int nextRender = fLastRender+1;
		if( nextRender > 5 )
			nextRender = 0;
		ISubmitRenderRequest(nextRender);
		fLastRefresh += fRefreshRate;
	}
}

void plDynamicEnvMap::ReRender()
{
	ISetupRenderRequests();
	ISubmitRenderRequests();
}

hsBool plDynamicEnvMap::INeedReRender()
{
	fOutStanding = 0;
	fLastRefresh = 0;
	return true;
}

hsBool plDynamicEnvMap::MsgReceive(plMessage* msg)
{
	plRenderRequestAck* ack = plRenderRequestAck::ConvertNoRef(msg);
	if( ack )
	{
		fOutStanding--;
		return true;
	}
	plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
	if( rendMsg )
	{
		if( fOutStanding )
			INeedReRender();

		ICheckForRefresh(hsTimer::GetSysSeconds(), rendMsg->Pipeline());
		return true;
	}
	if( plPipeRTMakeMsg::ConvertNoRef(msg) )
	{
		INeedReRender();
		plCubicRenderTarget::MsgReceive(msg);
		return true;
	}
	plAgeLoadedMsg* ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg);
	if( ageLoaded && ageLoaded->fLoaded )
		return INeedReRender();

	if( plInitialAgeStateLoadedMsg::ConvertNoRef(msg) )
		return INeedReRender();

	plDynamicEnvMapMsg* cmd = plDynamicEnvMapMsg::ConvertNoRef(msg);
	if( cmd )
	{
		if( cmd->fCmd & plDynamicEnvMapMsg::kSetPosition )
			SetPosition(cmd->fPos);

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetHither )
			SetHither(cmd->fHither);

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetFogStart )
			SetFogStart(cmd->fFogStart);

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetYon )
			SetYon(cmd->fYon);

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetColor )
			SetColor(cmd->fColor);

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetRefresh )
			SetRefreshRate(cmd->fRefresh);

		// If we're going to ReRender, make sure we've gotten any
		// parameter changes first.
		if( cmd->fCmd & plDynamicEnvMapMsg::kReRender )
		{
			ISetupRenderRequests();
			INeedReRender();
		}
		
		return true;
	}
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( IOnRefMsg(refMsg) )
			return true;
	}

	return plCubicRenderTarget::MsgReceive(msg);
}

hsBool plDynamicEnvMap::IOnRefMsg(plGenRefMsg* refMsg)
{
	switch( refMsg->fType)
	{
	case kRefVisSet:
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
		{
			plVisRegion* reg = plVisRegion::ConvertNoRef(refMsg->GetRef());
			int idx = fVisRegions.Find(reg);
			if( reg && (fVisRegions.kMissingIndex == idx) )
			{
				fVisRegions.Append(reg);
				fVisSet.SetBit(reg->GetIndex());
			}
			ISetupRenderRequests();
			return true;
		}
		else
		{
			plVisRegion* reg = plVisRegion::ConvertNoRef(refMsg->GetRef());
			int idx = fVisRegions.Find(reg);
			if( reg && (fVisRegions.kMissingIndex != idx) )
			{
				fVisRegions.Remove(idx);
				fVisSet.ClearBit(reg->GetIndex());
			}
			ISetupRenderRequests();
			return true;
		}
		break;

	case kRefRootNode:
		plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			fRootNode = so;
		else
			fRootNode = nil;
		return true;
	}

	return false;
}

void plDynamicEnvMap::SetIncludeCharacters(hsBool b) 
{ 
	fIncCharacters = b; 
	if( b )
		fVisSet.SetBit(plVisMgr::kCharacter);
	else
		fVisSet.ClearBit(plVisMgr::kCharacter);
}

void plDynamicEnvMap::AddVisRegion(plVisRegion* reg)
{
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefVisSet);
	hsgResMgr::ResMgr()->AddViaNotify(reg->GetKey(), msg, plRefFlags::kActiveRef);
}

void plDynamicEnvMap::Read(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Read(s, mgr);
	UInt32 sz = plCubicRenderTarget::Read(s);

	fPos.Read(s);
	fHither = s->ReadSwapScalar();
	fYon = s->ReadSwapScalar();
	fFogStart = s->ReadSwapScalar();
	fColor.Read(s);

	fRefreshRate = s->ReadSwapScalar();

	SetPosition(fPos);

	sz += sizeof(fPos) + sizeof(fHither) + sizeof(fYon) + sizeof(fFogStart) + sizeof(fColor) + sizeof(fRefreshRate);

	fIncCharacters = s->ReadByte();
	SetIncludeCharacters(fIncCharacters);
	int nVis = s->ReadSwap32();
	int i;
	for( i = 0; i < nVis; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefVisSet), plRefFlags::kActiveRef);

	nVis = s->ReadSwap32();
	for( i = 0; i < nVis; i++)
	{
		char *name = s->ReadSafeString();
		plKey key = plKeyFinder::Instance().StupidSearch(nil, nil, plVisRegion::Index(), name);
		delete[] name;
		if (key)
			hsgResMgr::ResMgr()->AddViaNotify(key, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefVisSet), plRefFlags::kActiveRef);
	}

	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefRootNode), plRefFlags::kActiveRef);

	Init();
}

void plDynamicEnvMap::Write(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Write(s, mgr);

	UInt32 sz = plCubicRenderTarget::Write(s);

	fPos.Write(s);
	s->WriteSwapScalar(fHither);
	s->WriteSwapScalar(fYon);
	s->WriteSwapScalar(fFogStart);
	fColor.Write(s);

	s->WriteSwapScalar(fRefreshRate);

	sz += sizeof(fPos) + sizeof(fHither) + sizeof(fYon) + sizeof(fFogStart) + sizeof(fColor) + sizeof(fRefreshRate);

	s->WriteByte(fIncCharacters);
	s->WriteSwap32(fVisRegions.GetCount());
	int i;
	for( i = 0; i < fVisRegions.GetCount(); i++ )
		mgr->WriteKey(s, fVisRegions[i]);

	s->WriteSwap32(fVisRegionNames.Count());
	for( i = 0; i < fVisRegionNames.Count(); i++)
	{
		s->WriteSafeString(fVisRegionNames[i]);
	}

	mgr->WriteKey(s, fRootNode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

UInt8 plDynamicCamMap::fFlags = kReflectionEnabled | kReflectionCapable;

plDynamicCamMap::plDynamicCamMap() :
fHither(0.3f),
fYon(500.f),
fFogStart(1000.f),
fRefreshRate(0.f),
fLastRefresh(0.0),
fOutStanding(0),
fCamera(nil),
fRootNode(nil),
fIncCharacters(false),
fDisableTexture(nil)
{
	fColor.Set(0,0,0,1.f);
	fReqMsg = TRACKED_NEW plRenderRequestMsg(nil, &fReq);
}

plDynamicCamMap::plDynamicCamMap(UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth, UInt8 sDepth) :
fHither(0.3f),
fYon(-1.f),
fFogStart(-1.f),
fRefreshRate(0.f),
fLastRefresh(0.0),
fOutStanding(0),
fCamera(nil),
fRootNode(nil),
fIncCharacters(false),
fDisableTexture(nil),
plRenderTarget(plRenderTarget::kIsTexture, width, height, bitDepth, zDepth, sDepth)
{
	fColor.Set(0,0,0,1.f);
	fReqMsg = TRACKED_NEW plRenderRequestMsg(nil, &fReq);
}

plDynamicCamMap::~plDynamicCamMap()
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());

	SetDeviceRef(nil);

	delete fReqMsg;
}

void plDynamicCamMap::Init()
{
	plgDispatch::Dispatch()->RegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plDynamicCamMap::SetRefreshRate(hsScalar secs)
{
	fRefreshRate = secs;
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plDynamicCamMap::ISetupRenderRequest(plPipeline *pipe)
{
	fReq.SetRenderState(plPipeline::kRenderNormal | plPipeline::kRenderClearColor | plPipeline::kRenderClearDepth);
	fReq.SetDrawableMask(plDrawable::kNormal);
	fReq.SetSubDrawableMask(plDrawable::kSubAllTypes);
	fReq.SetHither(fHither);
	fReq.SetYon(fYon);
	fReq.SetFogStart(fFogStart);

	// For a reflection map, this must match the camera FOV, or else the camera based 
	// texture coordinates for the reflection texture will be off.
	// 
	// For a fixed camera, you might want to use the height in both params, so that
	// you're rendering a square FOV into your square texture. In practice, the artists
	// don't mind the visual results when just scaling their UVs, so I'll leave it the
	// same for both cases.
	fReq.SetFovX(fCamera ? fCamera->GetFOVw() : plVirtualCam1::Instance()->GetFOVw());
	fReq.SetFovY(fCamera ? fCamera->GetFOVh() : plVirtualCam1::Instance()->GetFOVh());

	fReq.SetClearColor(fColor);
	fReq.SetClearDepth(1.f);
	fReq.SetClearDrawable(nil);
	fReq.SetRenderTarget(this);
	fReq.SetVisForce(fVisSet);
	fReq.SetIgnoreOccluders(true);
	fReq.RequestAck(GetKey());

	hsMatrix44 w2c, c2w;
	if (fCamera)
	{
		w2c.MakeCamera(&fCamera->GetTargetPos(), &fCamera->GetTargetPOA(), &hsVector3(0.f, 0.f, 1.f));
		w2c.GetInverse(&c2w);
	}
	else
	{
		if (!fRootNode)
			return;

		// Could be optimized, but the matrix construction work here seems cheap relative to the cost
		// of rerendering all this stuff to a separate target, so I doubt we'd notice.
		hsMatrix44 invert;
		invert.MakeScaleMat(&(hsVector3(1.f, 1.f, -1.f)));
		w2c = pipe->GetWorldToCamera();
		c2w = pipe->GetCameraToWorld();

		w2c = w2c * fRootNode->GetLocalToWorld() * invert * fRootNode->GetWorldToLocal();
		c2w = fRootNode->GetWorldToLocal() * invert * fRootNode->GetLocalToWorld() * c2w;
	}
	fReq.SetCameraTransform(w2c, c2w);
}

void plDynamicCamMap::ISubmitRenderRequest(plPipeline *pipe)
{
	ISetupRenderRequest(pipe);
	fReqMsg->SendAndKeep();
	fOutStanding++;
	fLastRefresh = hsTimer::GetSysSeconds();
}

void plDynamicCamMap::ICheckForRefresh(double t, plPipeline *pipe)
{
	int i;

	hsBool useRefl = (fFlags & kReflectionMask) == kReflectionMask;
	if (!fCamera)
	{
		if ((useRefl && fMatLayers[0]->GetTexture() != this) || (!useRefl && fMatLayers[0]->GetTexture() != fDisableTexture))
			IPrepTextureLayers();
	}

	// So no one's using us, eh? Hitting this condition is likely a bug, 
	// but an assert every frame would be annoying. We'll notice when
	// the render target never updates.
	if (fTargetNodes.GetCount() == 0)
		return; 

	// If dynamic planar reflections are disabled and we're using our substitute
	// texture, don't update. Otherwise, this particular reflection is forced to
	// always display.
	if (!useRefl && fDisableTexture)
		return;

	hsBool inView = false;
	for (i = 0; i < fTargetNodes.GetCount(); i++)
	{
		if (pipe->TestVisibleWorld(fTargetNodes[i]))
		{
			inView = true;
			break;
		}
	}

	if (!inView)
		return;

	if( fLastRefresh <= 0 )
	{
		ISubmitRenderRequest(pipe);
		return;
	}

	if( fRefreshRate <= 0 )
		return;

#ifndef PLASMA_EXTERNAL_RELEASE
	if (pipe->IsDebugFlagSet(plPipeDbg::kFlagNVPerfHUD) && hsTimer::GetDelSysSeconds() == 0)
	{
		ISubmitRenderRequest(pipe);
	}
#endif // PLASMA_EXTERNAL_RELEASE

	if (t > fLastRefresh + fRefreshRate)
	{
		ISubmitRenderRequest(pipe);
		return;
	}
}

hsBool plDynamicCamMap::INeedReRender()
{
	fOutStanding = 0;
	fLastRefresh = 0;
	return true;
}

hsBool plDynamicCamMap::MsgReceive(plMessage* msg)
{
	plRenderRequestAck* ack = plRenderRequestAck::ConvertNoRef(msg);
	if( ack )
	{
		fOutStanding--;
		return true;
	}
	plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
	if( rendMsg )
	{
		if( fOutStanding )
			INeedReRender();

		ICheckForRefresh(hsTimer::GetSysSeconds(), rendMsg->Pipeline());
		return true;
	}
	if( plPipeRTMakeMsg::ConvertNoRef(msg) )
	{
		INeedReRender();
		plRenderTarget::MsgReceive(msg);
		return true;
	}
	plAgeLoadedMsg* ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg);
	if( ageLoaded && ageLoaded->fLoaded )
		return INeedReRender();

	if( plInitialAgeStateLoadedMsg::ConvertNoRef(msg) )
		return INeedReRender();

	plDynamicEnvMapMsg* cmd = plDynamicEnvMapMsg::ConvertNoRef(msg);
	if( cmd )
	{
		if( cmd->fCmd & plDynamicEnvMapMsg::kSetFogStart )
			fFogStart = cmd->fFogStart;

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetColor )
			fColor = cmd->fColor;

		if( cmd->fCmd & plDynamicEnvMapMsg::kSetRefresh )
			SetRefreshRate(cmd->fRefresh);

		return true;
	}
	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( IOnRefMsg(refMsg) )
			return true;
	}

	return plRenderTarget::MsgReceive(msg);
}

void plDynamicCamMap::IPrepTextureLayers()
{
	if (fDisableTexture)
	{
		int i;
		for (i = 0; i < fMatLayers.GetCount(); i++)
		{
			if ((fFlags & kReflectionMask) == kReflectionMask)
			{
				fMatLayers[i]->SetUVWSrc(plLayerInterface::kUVWPosition);
				fMatLayers[i]->SetMiscFlags(hsGMatState::kMiscCam2Screen | hsGMatState::kMiscPerspProjection);
				hsgResMgr::ResMgr()->SendRef(GetKey(), TRACKED_NEW plGenRefMsg(fMatLayers[i]->GetKey(), plRefMsg::kOnRequest, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
			}
			else
			{
				fMatLayers[i]->SetUVWSrc(0);
				fMatLayers[i]->SetMiscFlags(0);
				hsgResMgr::ResMgr()->SendRef(fDisableTexture->GetKey(), TRACKED_NEW plGenRefMsg(fMatLayers[i]->GetKey(), plRefMsg::kOnRequest, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
			}
		}
	}
}

hsBool plDynamicCamMap::IOnRefMsg(plRefMsg* refMsg)
{
	plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(refMsg);

	if (genRefMsg)
	{
		if (genRefMsg->fType == kRefVisSet)
		{
			plVisRegion* reg = plVisRegion::ConvertNoRef(refMsg->GetRef());
			if (reg)
			{
				int idx = fVisRegions.Find(reg);
				if ((refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace)) && fVisRegions.kMissingIndex == idx)
				{
					fVisRegions.Append(reg);
					fVisSet.SetBit(reg->GetIndex());
				}
				else if (!(refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace)) && fVisRegions.kMissingIndex != idx)
				{
					fVisRegions.Remove(idx);
					fVisSet.ClearBit(reg->GetIndex());
				}
				return true;
			}
		}
		else if (genRefMsg->fType == kRefCamera)
		{
			plCameraModifier1 *cam = plCameraModifier1::ConvertNoRef(refMsg->GetRef());
			if (cam)
			{
				if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
					fCamera = cam;
				else 
					fCamera = nil;

				return true;
			}
		}
		else if (genRefMsg->fType == kRefRootNode)
		{
			plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
			if (so)
			{
				if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
					fRootNode = so;
				else 
					fRootNode = nil;

				return true;
			}
		}
		else if (genRefMsg->fType == kRefTargetNode)
		{
			plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
			if (so)
			{
				if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				{
					if (fTargetNodes.Find(so) == fTargetNodes.kMissingIndex)
						fTargetNodes.Append(so);
				}
				else 
				{
					fTargetNodes.RemoveItem(so);
				}

				return true;
			}
		}
		else if (genRefMsg->fType == kRefDisableTexture)
		{
			plBitmap *bitmap = plBitmap::ConvertNoRef(refMsg->GetRef());
			if (bitmap)
			{
				if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
					fDisableTexture = bitmap;
				else
					fDisableTexture = nil;

				return true;
			}
		}
		else if (genRefMsg->fType == kRefMatLayer)
		{
			plLayer *lay = plLayer::ConvertNoRef(refMsg->GetRef());
			if (lay)
			{
				if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				{
					if (fMatLayers.Find(lay) == fMatLayers.kMissingIndex)
						fMatLayers.Append(lay);
				}
				else
				{
					fMatLayers.RemoveItem(lay);
				}

				return true;
			}
		}
	}

	return false;
}

void plDynamicCamMap::SetIncludeCharacters(hsBool b) 
{ 
	fIncCharacters = b; 
	if( b )
		fVisSet.SetBit(plVisMgr::kCharacter);
	else
		fVisSet.ClearBit(plVisMgr::kCharacter);
}

void plDynamicCamMap::AddVisRegion(plVisRegion* reg)
{
	hsgResMgr::ResMgr()->AddViaNotify( reg->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plGenRefMsg::kOnReplace, -1, kRefVisSet ), plRefFlags::kActiveRef );
}

void plDynamicCamMap::SetEnabled(hsBool enable)
{
	if (enable)
		fFlags |= kReflectionEnabled;
	else
		fFlags &= ~kReflectionEnabled;
}

void plDynamicCamMap::SetCapable(hsBool capable)
{
	if (capable)
		fFlags |= kReflectionCapable;
	else
		fFlags &= ~kReflectionCapable;
}

void plDynamicCamMap::Read(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Read(s, mgr);
	plRenderTarget::Read(s);

	fHither = s->ReadSwapScalar();
	fYon = s->ReadSwapScalar();
	fFogStart = s->ReadSwapScalar();
	fColor.Read(s);

	fRefreshRate = s->ReadSwapScalar();
	fIncCharacters = s->ReadBool();
	SetIncludeCharacters(fIncCharacters);
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefCamera), plRefFlags::kPassiveRef);
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefRootNode), plRefFlags::kActiveRef);

	int numTargs = s->ReadByte();
	int i;
	for (i = 0; i < numTargs; i++)
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kRefTargetNode), plRefFlags::kPassiveRef);

	int nVis = s->ReadSwap32();
	for( i = 0; i < nVis; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefVisSet), plRefFlags::kActiveRef);

	nVis = s->ReadSwap32();
	for( i = 0; i < nVis; i++)
	{
		char *name = s->ReadSafeString();
		plKey key = plKeyFinder::Instance().StupidSearch(nil, nil, plVisRegion::Index(), name);
		delete[] name;
		if (key)
			hsgResMgr::ResMgr()->AddViaNotify(key, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefVisSet), plRefFlags::kActiveRef);
	}

	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefDisableTexture), plRefFlags::kActiveRef);
	
	UInt8 numLayers = s->ReadByte();
	for (i = 0; i < numLayers; i++)
	{
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatLayer), plRefFlags::kPassiveRef);
	}

	Init();
}

void plDynamicCamMap::Write(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Write(s, mgr);
	plRenderTarget::Write(s);

	s->WriteSwapScalar(fHither);
	s->WriteSwapScalar(fYon);
	s->WriteSwapScalar(fFogStart);
	fColor.Write(s);

	s->WriteSwapScalar(fRefreshRate);
	s->WriteByte(fIncCharacters);
	mgr->WriteKey(s, (fCamera ? fCamera->GetKey() : nil));
	mgr->WriteKey(s, (fRootNode ? fRootNode->GetKey() : nil));

	s->WriteByte(fTargetNodes.GetCount());
	int i;
	for (i = 0; i < fTargetNodes.GetCount(); i++)
		mgr->WriteKey(s, fTargetNodes[i]);

	s->WriteSwap32(fVisRegions.GetCount());
	for( i = 0; i < fVisRegions.GetCount(); i++ )
		mgr->WriteKey(s, fVisRegions[i]);

	s->WriteSwap32(fVisRegionNames.Count());
	for( i = 0; i < fVisRegionNames.Count(); i++)
	{
		s->WriteSafeString(fVisRegionNames[i]);
	}

	mgr->WriteKey(s, fDisableTexture ? fDisableTexture->GetKey() : nil);
	
	s->WriteByte(fMatLayers.GetCount());
	for (i = 0; i < fMatLayers.GetCount(); i++)
	{
		mgr->WriteKey(s, fMatLayers[i]->GetKey());
	}
}
