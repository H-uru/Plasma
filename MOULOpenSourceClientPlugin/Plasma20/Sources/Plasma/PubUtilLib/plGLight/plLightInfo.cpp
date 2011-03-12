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
#include "plLightInfo.h"
#include "plLightKonstants.h"
#include "hsBounds.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "plgDispatch.h"
#include "../plIntersect/plVolumeIsect.h"
#include "../plDrawable/plSpaceTree.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plPipeline/plRenderTarget.h"
#include "hsFastMath.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../plGImage/plMipmap.h"
#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plRenderRequestMsg.h"
#include "../plScene/plRenderRequest.h"
#include "plPipeline.h"
#include "../plIntersect/plSoftVolume.h"
#include "../plPipeline/plPipeDebugFlags.h"
#include "../pnMessage/plPipeResMakeMsg.h"

#include "../plScene/plVisRegion.h"
#include "../plScene/plVisMgr.h"

// heinous
#include "../plNetClient/plNetClientMgr.h"
#include "../pnMessage/plEnableMsg.h"
static hsScalar kMaxYon = 1000.f;
static hsScalar kMinHither = 1.f;

#include "plLightProxy.h"

#include "../plDrawable/plDrawableGenerator.h"

plLightInfo::plLightInfo()
:	fSceneNode(nil),
	fDeviceRef(nil),
	fVolFlags(0),
	fProjection(nil),
	fSoftVolume(nil)
{
	fLightToWorld.Reset();
	fWorldToLight.Reset();

	fLocalToWorld.Reset();
	fWorldToLocal.Reset();

	fLightToLocal.Reset();
	fLocalToLight.Reset();

	fWorldToProj.Reset();

	fNextDevPtr = nil;
	fPrevDevPtr = nil;

	fProxyGen = TRACKED_NEW plLightProxy;
	fProxyGen->Init(this);

	fRegisteredForRenderMsg = false;

	fVisSet.SetBit(plVisMgr::kNormal);
}

plLightInfo::~plLightInfo()
{
	if( fNextDevPtr != nil || fPrevDevPtr != nil )
		Unlink();

	hsRefCnt_SafeUnRef( fDeviceRef );

	if( fRegisteredForRenderMsg )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
		plgDispatch::Dispatch()->UnRegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
		fRegisteredForRenderMsg = false;
	}

	delete fProxyGen;
}

void plLightInfo::SetDeviceRef( hsGDeviceRef *ref )
{
	hsRefCnt_SafeAssign( fDeviceRef, ref ); 
}

void plLightInfo::IRefresh()
{
	ICheckMaxStrength();
	if( fDeviceRef )
		fDeviceRef->SetDirty( true );
}

void plLightInfo::ICheckMaxStrength()
{
	hsScalar r = GetDiffuse().r >= 0 ? GetDiffuse().r : -GetDiffuse().r;
	hsScalar g = GetDiffuse().g >= 0 ? GetDiffuse().g : -GetDiffuse().g;
	hsScalar b = GetDiffuse().b >= 0 ? GetDiffuse().b : -GetDiffuse().b;
	fMaxStrength = 
		r > g 
		?	(
			r > b
				? r
				: b
			)
		:	(
			g > b
				? g
				: b
			);

	const hsScalar kMinMaxStrength = 1.e-2f;
	SetZero(fMaxStrength < kMinMaxStrength);
}

void plLightInfo::GetStrengthAndScale(const hsBounds3Ext& bnd, hsScalar& strength, hsScalar& scale) const
{
	if( IsIdle() )
	{
		strength = scale = 0.f;
		return;
	}

	strength = fMaxStrength;

	scale = 1.f;
	if( fSoftVolume )
	{
		scale = fSoftVolume->GetStrength(bnd.GetCenter());
		strength *= scale;

	}
	return;		
}

void plLightInfo::GetAffectedForced(const plSpaceTree* space, hsBitVector& list, hsBool charac)
{
	Refresh();

	if( IGetIsect() )
	{
		space->HarvestLeaves(IGetIsect(), list);
	}
	else
	{
		list.Set(space->GetNumLeaves());
	}
}

void plLightInfo::GetAffected(const plSpaceTree* space, hsBitVector& list, hsBool charac)
{
	Refresh();

	if( IsIdle() )
		return;

	if( !GetProperty(kLPHasIncludes) || (GetProperty(kLPIncludesChars) && charac) )
	{
		if( IGetIsect() )
		{
			space->HarvestLeaves(IGetIsect(), list);
		}
		else
		{
			list.Set(space->GetNumLeaves());
		}
	}
}

const hsTArray<Int16>& plLightInfo::GetAffected(plSpaceTree* space, const hsTArray<Int16>& visList, hsTArray<Int16>& litList, hsBool charac)
{
	Refresh();

	if( !IsIdle() )
	{
		if( !GetProperty(kLPHasIncludes) || (GetProperty(kLPIncludesChars) && charac) )
		{
			if( IGetIsect() )
			{
				static hsBitVector cache;
				cache.Clear();
				space->EnableLeaves(visList, cache);

				space->HarvestEnabledLeaves(IGetIsect(), cache, litList);

				return litList;
			}
			else
			{
				return visList;
			}
		}
	}

	litList.SetCount(0);
	return litList;
}

//// Set/GetProperty /////////////////////////////////////////////////////////
//	Sets/gets a property just like the normal Set/GetNativeProperty, but the 
//	flag taken in is from plDrawInterface, not our props flags. So we have to 
//	translate...

void plLightInfo::SetProperty( int prop, hsBool on )
{
	plObjInterface::SetProperty(prop, on);
	if( kDisable == prop )
		fProxyGen->SetDisable(on);
}

//// SetSpecular /////////////////////////////////////////////////////////////
//	A bit more complicated here--make sure we set/clear the kLPHasSpecular
//	flag so we can test more easily for such a condition.

void plLightInfo::SetSpecular( const hsColorRGBA& c ) 
{ 
	fSpecular = c; 

	if( fSpecular.r == 0.f && fSpecular.g == 0.f && fSpecular.b == 0.f )
		SetProperty( kLPHasSpecular, false );
	else
		SetProperty( kLPHasSpecular, true );

	SetDirty(); 
}

void plLightInfo::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fLocalToWorld = l2w;
	fWorldToLocal = w2l;

	fLightToWorld = l2w * fLightToLocal; 
	fWorldToLight = fLocalToLight * w2l; 

	if( IGetIsect() )
		IGetIsect()->SetTransform(fLightToWorld, fWorldToLight);

	if( fDeviceRef != nil )
		fDeviceRef->SetDirty( true );

	fProxyGen->SetTransform(fLightToWorld, fWorldToLight);

	SetDirty(true);

	if( GetProjection() )
	{
		Refresh();
		hsMatrix44 w2proj = IGetWorldToProj();
		plLayer* lay = plLayer::ConvertNoRef(GetProjection()->BottomOfStack());
		if( lay )
		{
			lay->SetTransform(w2proj);
		}
	}
}

void plLightInfo::SetLocalToLight(const hsMatrix44& l2lt, const hsMatrix44& lt2l)
{
	fLocalToLight = l2lt;
	fLightToLocal = lt2l;
}

const hsMatrix44& plLightInfo::GetLocalToWorld() const
{
	return fLocalToWorld;
}

const hsMatrix44& plLightInfo::GetWorldToLocal() const
{
	return fWorldToLocal;
}

const hsMatrix44& plLightInfo::GetLightToWorld() const
{
	return fLightToWorld;
}

const hsMatrix44& plLightInfo::GetWorldToLight() const
{
	return fWorldToLight;
}

#include "plProfile.h"
plProfile_CreateTimer("Light Info", "RenderSetup", LightInfo);

void plLightInfo::IAddVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fVisRegions.Find(reg);
		if( fVisRegions.kMissingIndex == idx )
		{
			fVisRegions.Append(reg);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.SetBit(reg->GetIndex());
			else
			{
				fVisSet.SetBit(reg->GetIndex());
				if( reg->ReplaceNormal() )
					fVisSet.ClearBit(plVisMgr::kNormal);
			}
		}
	}
}

void plLightInfo::IRemoveVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fVisRegions.Find(reg);
		if( fVisRegions.kMissingIndex != idx )
		{
			fVisRegions.Remove(idx);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.ClearBit(reg->GetIndex());
			else
				fVisSet.ClearBit(reg->GetIndex());
		}
	}
}


hsBool plLightInfo::MsgReceive(plMessage* msg)
{
	plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
	if( rendMsg )
	{
		plProfile_BeginLap(LightInfo, this->GetKey()->GetUoid().GetObjectName());

		if( !fDeviceRef && !GetProperty(kLPShadowOnly) )
		{
			rendMsg->Pipeline()->RegisterLight( this );
		}

		ICheckMaxStrength();

		plProfile_EndLap(LightInfo, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
		{
			switch( refMsg->fType )
			{
			case kProjection:
				fProjection = plLayerInterface::ConvertNoRef(refMsg->GetRef());
				{
					if( GetKey() && GetKey()->GetName() && !strncmp(GetKey()->GetName(), "RTPatternLight", strlen("RTPatternLight")) )
						SetProperty(kLPForceProj, true);
				}
				break;
			case kSoftVolume:
				fSoftVolume = plSoftVolume::ConvertNoRef(refMsg->GetRef());
				break;
			case kVisRegion:
				IAddVisRegion(plVisRegion::ConvertNoRef(refMsg->GetRef()));
				break;
			}
		}
		else if( refMsg->GetContext() & (plRefMsg::kOnRemove | plRefMsg::kOnDestroy) )
		{
			switch( refMsg->fType )
			{
			case kProjection:
				fProjection = nil;
				break;
			case kSoftVolume:
				fSoftVolume = nil;
				break;
			case kVisRegion:
				IRemoveVisRegion(plVisRegion::ConvertNoRef(refMsg->GetRef()));
				break;
			}
		}
		return true;
	}
	plPipeRTMakeMsg* rtMake = plPipeRTMakeMsg::ConvertNoRef(msg);
	if( rtMake )
	{
		// Make sure we're registered with the pipeline
		// If we're only here to cast shadows, just don't tell anyone
		// about us.
		if( !fDeviceRef && !GetProperty(kLPShadowOnly) )
		{
			rtMake->Pipeline()->RegisterLight( this );
		}
		return true;
	}
	plEnableMsg* enaMsg = plEnableMsg::ConvertNoRef(msg);
	if( enaMsg )
	{
		SetProperty(kDisable, enaMsg->Cmd(plEnableMsg::kDisable));
		return true;
	}
	


	return plObjInterface::MsgReceive(msg);
}


void plLightInfo::Read(hsStream* s, hsResMgr* mgr)
{
	hsRefCnt_SafeUnRef( fDeviceRef );
	fDeviceRef = nil;

	plObjInterface::Read(s, mgr);

	fAmbient.Read(s);
	fDiffuse.Read(s);
	fSpecular.Read(s);

	fLightToLocal.Read(s);
	fLocalToLight.Read(s);

	fLightToWorld.Read(s);
	fWorldToLight.Read(s);

	fLocalToWorld = fLightToWorld * fLocalToLight;
	fWorldToLocal = fLightToLocal * fWorldToLight;

	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kProjection), plRefFlags::kActiveRef);

	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kSoftVolume), plRefFlags::kActiveRef);

	// Let our sceneNode know we're here.
	plKey nodeKey = mgr->ReadKey(s);
	ISetSceneNode(nodeKey);

	int n = s->ReadSwap32();
	fVisRegions.SetCountAndZero(n);
	int i;
	for( i = 0; i < n; i++ )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kVisRegion), plRefFlags::kActiveRef);

	SetDirty(true);
}

void plLightInfo::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);

	fAmbient.Write(s);
	fDiffuse.Write(s);
	fSpecular.Write(s);

	fLightToLocal.Write(s);
	fLocalToLight.Write(s);

	fLightToWorld.Write(s);
	fWorldToLight.Write(s);

	mgr->WriteKey(s, GetProjection());

	mgr->WriteKey(s, fSoftVolume);

	mgr->WriteKey(s, fSceneNode);

	s->WriteSwap32(fVisRegions.GetCount());
	int i;
	for( i = 0; i < fVisRegions.GetCount(); i++ )
		mgr->WriteKey(s, fVisRegions[i]);
}

// These two should only be called by the SceneObject
void plLightInfo::ISetSceneNode(plKey node)
{
	if( node != fSceneNode )
	{
		if( node )
		{
			plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(node, plRefMsg::kOnCreate, -1, plNodeRefMsg::kLight);
			hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kPassiveRef);
		}
		if( fSceneNode )
		{
			fSceneNode->Release(GetKey());
		}
	}
	fSceneNode = node;

	if( fSceneNode != nil )
	{
		if( !fRegisteredForRenderMsg )
		{
			plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
			plgDispatch::Dispatch()->RegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
			fRegisteredForRenderMsg = true;
		}
	}
	else if( fRegisteredForRenderMsg )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
		plgDispatch::Dispatch()->UnRegisterForExactType(plPipeRTMakeMsg::Index(), GetKey());
		fRegisteredForRenderMsg = false;
	}
}

plKey plLightInfo::GetSceneNode() const
{
	return fSceneNode;
}

//// Link & Unlink ///////////////////////////////////////////////////////

void	plLightInfo::Unlink( void )
{
	hsAssert( fPrevDevPtr, "Light info not in list" );
	if( fNextDevPtr )
		fNextDevPtr->fPrevDevPtr = fPrevDevPtr;
	*fPrevDevPtr = fNextDevPtr;

	fNextDevPtr = nil;
	fPrevDevPtr = nil;
}

void	plLightInfo::Link( plLightInfo **back )
{
	hsAssert( fNextDevPtr == nil && fPrevDevPtr == nil, "Trying to link a lightInfo that's already linked" );

	fNextDevPtr = *back;
	if( *back )
		(*back)->fPrevDevPtr = &fNextDevPtr;
	fPrevDevPtr = back;
	*back = this;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///// Standard light types
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Directional

plDirectionalLightInfo::plDirectionalLightInfo()
{
}

plDirectionalLightInfo::~plDirectionalLightInfo()
{
}

void plDirectionalLightInfo::GetStrengthAndScale(const hsBounds3Ext& bnd, hsScalar& strength, hsScalar& scale) const
{
	plLightInfo::GetStrengthAndScale(bnd, strength, scale);
}

void plDirectionalLightInfo::Read(hsStream* s, hsResMgr* mgr)
{
	plLightInfo::Read(s, mgr);
}

void plDirectionalLightInfo::Write(hsStream* s, hsResMgr* mgr)
{
	plLightInfo::Write(s, mgr);
}


hsVector3 plDirectionalLightInfo::GetWorldDirection() const
{ 
	return -fLightToWorld.GetAxis( hsMatrix44::kUp );
}

//////////////////////////////////////////////////////////////////////////
// Limited Directional
plLimitedDirLightInfo::plLimitedDirLightInfo()
:	fParPlanes(nil)
{
}

plLimitedDirLightInfo::~plLimitedDirLightInfo()
{
	delete fParPlanes;
}

void plLimitedDirLightInfo::IRefresh()
{
	plLightInfo::IRefresh();

	if( !IGetIsect() )
		IMakeIsect();

	if( GetProjection() )
	{
		hsMatrix44 l2ndc;
		l2ndc.Reset();

		hsScalar width = fWidth;
		hsScalar height = fHeight;

		l2ndc.fMap[0][0] = 1.f / width;
		l2ndc.fMap[0][3] = 0.5f;

		l2ndc.fMap[1][1] = -1.f / height;
		l2ndc.fMap[1][3] = 0.5f;

		// Map Screen Z to range 0 (at hither) to 1 (at yon)
		// No, map z dead on to 1.f
		l2ndc.fMap[2][2] = 1.f / fDepth;
		l2ndc.fMap[2][3] = 0;

		l2ndc.fMap[3][3] = 1.f;
		l2ndc.NotIdentity();

		fWorldToProj = l2ndc * fWorldToLight;
	}
}

void plLimitedDirLightInfo::GetStrengthAndScale(const hsBounds3Ext& bnd, hsScalar& strength, hsScalar& scale) const
{
	// If we haven't culled the object, return that we're full strength.
	plLightInfo::GetStrengthAndScale(bnd, strength, scale);
}

void plLimitedDirLightInfo::Read(hsStream* s, hsResMgr* mgr)
{
	plDirectionalLightInfo::Read(s, mgr);

	fWidth = s->ReadSwapScalar();
	fHeight = s->ReadSwapScalar();
	fDepth = s->ReadSwapScalar();
}

void plLimitedDirLightInfo::Write(hsStream* s, hsResMgr* mgr)
{
	plDirectionalLightInfo::Write(s, mgr);

	s->WriteSwapScalar(fWidth);
	s->WriteSwapScalar(fHeight);
	s->WriteSwapScalar(fDepth);
}

void plLimitedDirLightInfo::IMakeIsect()
{
	if( !fParPlanes )
		fParPlanes = TRACKED_NEW plParallelIsect;

	fParPlanes->SetNumPlanes(3);
	
	hsPoint3 p0, p1;

	hsScalar width = fWidth;
	hsScalar height = fHeight;
	p0.Set(-width*0.5f, 0, 0);
	p1.Set(width*0.5f, 0, 0);
	fParPlanes->SetPlane(0, p0, p1);

	p0.Set(0, -height * 0.5f, 0);
	p1.Set(0, height * 0.5f, 0);
	fParPlanes->SetPlane(1, p0, p1);

	p0.Set(0, 0, 0);
	p1.Set(0, 0, -fDepth);
	fParPlanes->SetPlane(2, p0, p1);

	fParPlanes->SetTransform(fLightToWorld, fWorldToLight);
}

//// ICreateProxy //////////////////////////////////////////////////////
//	Creates a new box drawable for showing the light's
//	influence.

plDrawableSpans* plLimitedDirLightInfo::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	hsPoint3 corner;
	corner.Set(-fWidth*0.5f, -fHeight*0.5f, -fDepth);
	hsVector3 vecs[3];
	vecs[0].Set(fWidth, 0, 0);
	vecs[1].Set(0, fHeight, 0);
	vecs[2].Set(0, 0, fDepth);
		// Generate a rectangular drawable based on a corner and three vectors
	plDrawableSpans* draw = plDrawableGenerator::GenerateBoxDrawable( corner, vecs[0], vecs[1], vecs[2], 
															mat, 
															fLightToWorld, 
															true,
															nil,
															&idx, 
															addTo );

	return draw;
}



//////////////////////////////////////////////////////////////////////////
// Omni
plOmniLightInfo::plOmniLightInfo()
:	fAttenConst(0),
	fAttenLinear(1.f),
	fAttenQuadratic(0),
	fAttenCutoff(0),
	fSphere(nil)
{
}

plOmniLightInfo::~plOmniLightInfo()
{
	delete fSphere;
}

void plOmniLightInfo::IMakeIsect()
{
	fSphere = TRACKED_NEW plSphereIsect;
	fSphere->SetTransform(fLightToWorld, fWorldToLight);
}

void plOmniLightInfo::GetStrengthAndScale(const hsBounds3Ext& bnd, hsScalar& strength, hsScalar& scale) const
{
	plLightInfo::GetStrengthAndScale(bnd, strength, scale);

	// Volume - Want to base this on the closest point on the bounds, instead of just the center.
	const hsPoint3& pos = bnd.GetCenter();

	hsScalar dist = hsVector3(&pos, &GetWorldPosition()).MagnitudeSquared();
	dist = 1.f / hsFastMath::InvSqrtAppr(dist);
	if( fAttenQuadratic > 0 )
	{
		strength /= (fAttenConst + fAttenLinear * dist + fAttenQuadratic * dist * dist);
	}
	else if( fAttenLinear > 0 )
	{
		strength /= (fAttenConst + fAttenLinear * dist);
	}
	else if( fAttenConst > 0 )
	{
		strength /= fAttenConst;
	}
	else if( fAttenCutoff > 0 )
	{
		if( dist > fAttenCutoff )
			strength = 0;
	}
}

hsScalar plOmniLightInfo::GetRadius() const
{
	hsScalar radius = 0;
	if( fAttenQuadratic > 0 )
	{
		hsScalar mult = fDiffuse.a >= 0 ? fDiffuse.a : -fDiffuse.a;
		hsScalar det = fAttenLinear*fAttenLinear - 4.f * fAttenQuadratic * fAttenConst * (1.f - mult * plSillyLightKonstants::GetFarPowerKonst());
		if( det > 0 )
		{
			det = hsSquareRoot(det);

			radius = -fAttenLinear + det;
			radius /= fAttenQuadratic * 2.f;
			if( radius < 0 )
				radius = 0;
		}
	}
	else if( fAttenLinear > 0 )
	{
		hsScalar mult = fDiffuse.a >= 0 ? fDiffuse.a : -fDiffuse.a;
		radius = (mult * plSillyLightKonstants::GetFarPowerKonst() - 1.f ) * fAttenConst / fAttenLinear;
	}
	else if( fAttenCutoff > 0 )
	{
		radius = fAttenCutoff;
	}

	return radius;
}

void plOmniLightInfo::IRefresh()
{
	plLightInfo::IRefresh();

	if( IsAttenuated() )
	{
		if( !fSphere )
			IMakeIsect();
		fSphere->SetRadius(GetRadius());
	}
	else
	{
		delete fSphere;
		fSphere = nil;
	}
}

hsVector3 plOmniLightInfo::GetNegativeWorldDirection(const hsPoint3& pos) const
{
	return hsFastMath::NormalizeAppr(hsVector3(&GetWorldPosition(), &pos));
}

void plOmniLightInfo::Read(hsStream* s, hsResMgr* mgr)
{
	plLightInfo::Read(s, mgr);

	fAttenConst = s->ReadSwapScalar();
	fAttenLinear = s->ReadSwapScalar();
	fAttenQuadratic = s->ReadSwapScalar();
	fAttenCutoff = s->ReadSwapScalar();
}

void plOmniLightInfo::Write(hsStream* s, hsResMgr* mgr)
{
	plLightInfo::Write(s, mgr);

	s->WriteSwapScalar(fAttenConst);
	s->WriteSwapScalar(fAttenLinear);
	s->WriteSwapScalar(fAttenQuadratic);
	s->WriteSwapScalar( fAttenCutoff );
}


//// ICreateProxy //////////////////////////////////////////////////////
//	Creates a new sphere drawable for showing the omnilight's
//	sphere (haha) of influence.

plDrawableSpans* plOmniLightInfo::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	float	rad = GetRadius();
	if( rad == 0 )
		rad = 50;

	plDrawableSpans* draw = plDrawableGenerator::GenerateSphericalDrawable(hsPoint3(0,0,0), 
														rad, 
														mat, 
														fLightToWorld, 
														true,
														nil,
														&idx,
														addTo);

	return draw;
}


//////////////////////////////////////////////////////////////////////////
// Spot
plSpotLightInfo::plSpotLightInfo()
:	fFalloff(1.f),
	fSpotInner(hsScalarPI * 0.125f),
	fSpotOuter(hsScalarPI * 0.25f),
	fCone(nil)
{
}

plSpotLightInfo::~plSpotLightInfo()
{
	delete fCone;
}

void plSpotLightInfo::GetStrengthAndScale(const hsBounds3Ext& bnd, hsScalar& strength, hsScalar& scale) const
{
	plOmniLightInfo::GetStrengthAndScale(bnd, strength, scale);

	// Volume - Want to base this on the closest point on the bounds, instead of just the center.
	const hsPoint3& pos = bnd.GetCenter();
	
	hsVector3 del;
	del.Set(&pos, &GetWorldPosition());
	hsScalar invDist = del.MagnitudeSquared();
	invDist = hsFastMath::InvSqrtAppr(invDist);

	hsScalar dot = del.InnerProduct(GetWorldDirection());
	dot *= invDist;

	hsScalar cosInner, cosOuter, t;
	hsFastMath::SinCosInRangeAppr(fSpotInner, t, cosInner);
	hsFastMath::SinCosInRangeAppr(fSpotOuter, t, cosOuter);
	if( dot < cosOuter )
		strength = 0;
	else if( dot < cosInner )
		strength *= (dot - cosOuter) / (cosInner - cosOuter);
}

void plSpotLightInfo::IMakeIsect()
{
	fCone = TRACKED_NEW plConeIsect;
	fCone->SetTransform(fLightToWorld, fWorldToLight);
}

void plSpotLightInfo::IRefresh()
{
	plLightInfo::IRefresh();

	if( !fCone )
		IMakeIsect();

	hsScalar effFOV = fSpotOuter;
	fCone->SetAngle(effFOV);
	

	if( IsAttenuated() )
	{
		fCone->SetLength(GetRadius());
		fCone->SetTransform(fLightToWorld, fWorldToLight);
	}

	if( GetProjection() )
	{
		hsScalar yon = GetRadius();
		if( yon < kMinHither )
			yon = kMaxYon;
		hsScalar hither = hsMinimum(kMinHither, yon * 0.5f);

		hsScalar sinFOV, cosFOV;
		hsFastMath::SinCos(effFOV, sinFOV, cosFOV);

		hsMatrix44 l2ndc;
		l2ndc.Reset();
		l2ndc.fMap[0][0] =   cosFOV / sinFOV * 0.5f;
		l2ndc.fMap[0][2] = -0.5f;
		l2ndc.fMap[1][1] =  -cosFOV / sinFOV * 0.5f;
		l2ndc.fMap[1][2] = -0.5f;
		l2ndc.fMap[2][2] =	-yon / (yon - hither);
		l2ndc.fMap[3][3] =	0;
		l2ndc.fMap[3][2] =	-1.f;

		l2ndc.NotIdentity();

		fWorldToProj = l2ndc * fWorldToLight;
	}
}

void plSpotLightInfo::Read(hsStream* s, hsResMgr* mgr)
{
	plOmniLightInfo::Read(s, mgr);

	fFalloff = s->ReadSwapScalar();
	fSpotInner = s->ReadSwapScalar();
	fSpotOuter = s->ReadSwapScalar();
}

void plSpotLightInfo::Write(hsStream* s, hsResMgr* mgr)
{
	plOmniLightInfo::Write(s, mgr);

	s->WriteSwapScalar(fFalloff);
	s->WriteSwapScalar(fSpotInner);
	s->WriteSwapScalar(fSpotOuter);
}


hsVector3 plSpotLightInfo::GetWorldDirection() const
{ 
	return -fLightToWorld.GetAxis( hsMatrix44::kUp );
}

//// ICreateProxy //////////////////////////////////////////////////////
//	Generates a new drawable for showing the spotlight's
//	sphere of influence.

plDrawableSpans* plSpotLightInfo::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	float	rad = GetRadius();
	float	x, y;


	if( rad == 0 )
		rad = 80;

	hsFastMath::SinCosAppr( GetSpotOuter(), x, y );

	plDrawableSpans* draw = plDrawableGenerator::GenerateConicalDrawable(rad * x / y, -rad, 
														mat, 
														fLightToWorld, 
														true,
														nil,
														&idx,
														addTo);
	return draw;
}

