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
#include "plShadowMaster.h"
#include "plShadowSlave.h"

#include "plLightInfo.h"
#include "plShadowCaster.h"

#include "../plIntersect/plVolumeIsect.h"
#include "../plMessage/plShadowCastMsg.h"
#include "../plMessage/plRenderMsg.h"

#include "../plDrawable/plDrawableSpans.h"

#include "../plScene/plVisMgr.h"

#include "hsBounds.h"
#include "plgDispatch.h"
#include "plPipeline.h"
#include "hsFastMath.h"

#include "plTweak.h"

UInt32 plShadowMaster::fGlobalMaxSize = 512;
hsScalar plShadowMaster::fGlobalMaxDist = 160.f; // PERSPTEST
// hsScalar plShadowMaster::fGlobalMaxDist = 100000.f; // PERSPTEST
hsScalar plShadowMaster::fGlobalVisParm = 1.f;

void plShadowMaster::SetGlobalShadowQuality(hsScalar s) 
{ 
	if( s < 0 )
		s = 0;
	else if( s > 1.f )
		s = 1.f;
	fGlobalVisParm = s;
}

void plShadowMaster::SetGlobalMaxSize(UInt32 s) 
{ 
	const UInt32 kMaxMaxGlobalSize = 512;
	const UInt32 kMinMaxGlobalSize = 32;

	// Make sure it's a power of two.
	if( ((s-1) & ~s) != (s-1) )
	{
		int i;
		for( i = 31; i >= 0; i-- )
		{
			if( (1 << i) & s )
				break;
		}
		s = 1 << i;
	}

	if( s > kMaxMaxGlobalSize )
		s = kMaxMaxGlobalSize;
	if( s < kMinMaxGlobalSize )
		s = kMinMaxGlobalSize;

	fGlobalMaxSize = s; 
}

plShadowMaster::plShadowMaster()
:	fAttenDist(0),
	fMaxDist(0),
	fMinDist(0),
	fMaxSize(256),
	fMinSize(256),
	fPower(1.f),
	fLightInfo(nil)
{
}

plShadowMaster::~plShadowMaster()
{
	Deactivate();

	fSlavePool.SetCount(fSlavePool.GetNumAlloc());
	int i;
	for( i = 0; i < fSlavePool.GetCount(); i++ )
		delete fSlavePool[i];
}

void plShadowMaster::Read(hsStream* stream, hsResMgr* mgr)
{
	plObjInterface::Read(stream, mgr);

	fAttenDist = stream->ReadSwapScalar();

	fMaxDist = stream->ReadSwapScalar();
	fMinDist = stream->ReadSwapScalar();

	fMaxSize = stream->ReadSwap32();
	fMinSize = stream->ReadSwap32();

	fPower = stream->ReadSwapScalar();

	Activate();
}

void plShadowMaster::Write(hsStream* stream, hsResMgr* mgr)
{
	plObjInterface::Write(stream, mgr);

	stream->WriteSwapScalar(fAttenDist);

	stream->WriteSwapScalar(fMaxDist);
	stream->WriteSwapScalar(fMinDist);

	stream->WriteSwap32(fMaxSize);
	stream->WriteSwap32(fMinSize);

	stream->WriteSwapScalar(fPower);
}

void plShadowMaster::Activate() const
{
	plgDispatch::Dispatch()->RegisterForExactType(plShadowCastMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowMaster::Deactivate() const
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plShadowCastMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowMaster::SetMaxDist(hsScalar f)
{
	fMaxDist = f;
	fMinDist = f * 0.75f;
}


#include "plProfile.h"
plProfile_CreateTimer("ShadowMaster", "RenderSetup", ShadowMaster);
hsBool plShadowMaster::MsgReceive(plMessage* msg)
{
	plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
	if( rendMsg )
	{
		plProfile_BeginLap(ShadowMaster, this->GetKey()->GetUoid().GetObjectName());
		IBeginRender();
		plProfile_EndLap(ShadowMaster, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}

	plShadowCastMsg* castMsg = plShadowCastMsg::ConvertNoRef(msg);
	if( castMsg )
	{
		IOnCastMsg(castMsg);
		return true;
	}

	return plObjInterface::MsgReceive(msg);
}

void plShadowMaster::IBeginRender()
{
	fSlavePool.SetCount(0);
	if( ISetLightInfo() ) 
		fLightInfo->ClearSlaveBits();
}

hsBool plShadowMaster::IOnCastMsg(plShadowCastMsg* castMsg)
{
//	// HACKTEST
//	return false;

	if( !fLightInfo )
		return false;

	if( !fLightInfo->InVisSet(plGlobalVisMgr::Instance()->GetVisSet())
		|| fLightInfo->InVisNot(plGlobalVisMgr::Instance()->GetVisNot()) )
		return false;

	const UInt8 shadowQuality = UInt8(plShadowMaster::GetGlobalShadowQuality() * 3.9f);
	if( !GetKey()->GetUoid().GetLoadMask().MatchesQuality(shadowQuality) )
		return false;

	plShadowCaster* caster = castMsg->Caster();

	if( !caster->Spans().GetCount() )
		return false;

	hsBounds3Ext casterBnd;
	IComputeCasterBounds(caster, casterBnd);

	hsScalar power = IComputePower(caster, casterBnd);

	static hsScalar kVisShadowPower = 1.e-1f;
	static hsScalar kMinShadowPower = 2.e-1f;
	static hsScalar kKneeShadowPower = 3.e-1f;
	if( power < kMinShadowPower )
		return false;
	if( power < kKneeShadowPower )
	{
		power -= kMinShadowPower;
		power /= kKneeShadowPower - kMinShadowPower;
		power *= kKneeShadowPower - kVisShadowPower;
		power += kVisShadowPower;
	}

	// Create ShadowSlave focused on ShadowCaster
	// ShadowSlave extent just enough to cover ShadowCaster (including nearplane)
	plShadowSlave* slave = ICreateShadowSlave(castMsg, casterBnd, power);
	if( !slave )
		return false;

	// !!!IMPORTANT
	// ShadowMaster contains 2 values for yon.
	// First value applies to ShadowMaster. Any ShadowCaster beyond this distance
	//		won't cast a shadow
	// Second value applies to ShadowSlaves. This is the distance beyond the ShadowCaster
	//		(NOT FROM SHADOW SOURCE) over which the shadow attenuates to zero
	// The effective yon for the ShadowSlave is ShadowSlaveYon + DistanceToFarthestPointOnShadowCasterBound
	//		That's the distance used for culling ShadowReceivers
	// The ShadowSlaveYon is used directly in the 

	slave->fIndex = UInt32(-1);
	castMsg->Pipeline()->SubmitShadowSlave(slave);
	
	if( slave->fIndex == UInt32(-1) )
	{
		IRecycleSlave(slave);
		return false;
	}
	
	fLightInfo->SetSlaveBit(slave->fIndex);
	slave->SetFlag(plShadowSlave::kObeysLightGroups, fLightInfo->GetProperty(plLightInfo::kLPShadowLightGroup) && fLightInfo->GetProperty(plLightInfo::kLPHasIncludes));
	slave->SetFlag(plShadowSlave::kIncludesChars, fLightInfo->GetProperty(plLightInfo::kLPIncludesChars));

	return true;
}


plLightInfo* plShadowMaster::ISetLightInfo()
{
	fLightInfo = nil;
	plSceneObject* owner = IGetOwner();
	if( owner )
	{
		fLightInfo = plLightInfo::ConvertNoRef(owner->GetGenericInterface(plLightInfo::Index()));
	}
	return fLightInfo;
}

void plShadowMaster::IComputeCasterBounds(const plShadowCaster* caster, hsBounds3Ext& casterBnd)
{
	casterBnd.MakeEmpty();
	const hsTArray<plShadowCastSpan>& castSpans = caster->Spans();
	int i;
	for( i = 0; i < castSpans.GetCount(); i++ )
	{
		plDrawableSpans* dr = castSpans[i].fDraw;
		UInt32 index = castSpans[i].fIndex;

		// Right now, the generic world bounds seems close enough, even when skinned.
		// It gets a little off on the lower LODs, but, hey, they're the lower LODs.
		// Getting something more precise will probably involve finding a cagey place
		// to stash the accurate world bounds, because we only compute them when the
		// skinned objects are visible, and we need them here before the object is
		// visibility tested.
		casterBnd.Union(&dr->GetSpan(index)->fWorldBounds);
	}
}

plShadowSlave* plShadowMaster::INextSlave(const plShadowCaster* caster)
{
	int iSlave = fSlavePool.GetCount();
	fSlavePool.ExpandAndZero(iSlave+1);
	plShadowSlave* slave = fSlavePool[iSlave];
	if( !slave )
	{
		fSlavePool[iSlave] = slave = INewSlave(caster);
	}
	return slave;
}

plShadowSlave* plShadowMaster::ICreateShadowSlave(plShadowCastMsg* castMsg, const hsBounds3Ext& casterBnd, hsScalar power)
{
	const plShadowCaster* caster = castMsg->Caster();

	plShadowSlave* slave = INextSlave(caster);

	slave->Init();
	//HACKTEST
//	slave->SetFlag(plShadowSlave::kReverseCull, true);

	slave->fPower = power;
	slave->fCaster = caster;
	slave->fCasterWorldBounds = casterBnd;
	slave->fAttenDist = fAttenDist * caster->GetAttenScale();
	slave->fBlurScale = caster->GetBlurScale();

	slave->SetFlag(plShadowSlave::kSelfShadow, GetProperty(kSelfShadow) || caster->GetSelfShadow());

	// Order of these matters, since values calculated in one are
	// used by later functions. Rearrange at your own risk.
	IComputeWorldToLight(casterBnd, slave);

	IComputeBounds(casterBnd, slave);

	IComputeWidthAndHeight(castMsg, slave);

	IComputeProjections(castMsg, slave);

	IComputeLUT(castMsg, slave);

	IComputeISect(casterBnd, slave);

	// Make sure we really need this shadow. If we decide we
	// don't, the returned slave will be nil;
	slave = ILastChanceToBail(castMsg, slave);

	return slave;
}

plShadowSlave* plShadowMaster::IRecycleSlave(plShadowSlave* slave)
{
	fSlavePool.SetCount(fSlavePool.GetCount()-1);
	return nil;
}

plShadowSlave* plShadowMaster::ILastChanceToBail(plShadowCastMsg* castMsg, plShadowSlave* slave)
{
	const hsBounds3Ext& wBnd = slave->fWorldBounds;

	// If the bounds of the cast shadow aren't visible, forget it.
	if( !castMsg->Pipeline()->TestVisibleWorld(wBnd) )
		return IRecycleSlave(slave);

	hsScalar maxDist = fMaxDist > 0
		? (fGlobalMaxDist > 0
			? hsMinimum(fMaxDist, fGlobalMaxDist)
			: fMaxDist)
		: fGlobalMaxDist;

	plConst(hsScalar) kMinFrac(0.6f);
	maxDist *= kMinFrac + GetGlobalShadowQuality() * (1.f - kMinFrac);

	// If we haven't got a max distance at which the shadow stays visible
	// then we just need to go with it.
	if( maxDist <= 0 )
		return slave;

	plConst(hsScalar) kMinFadeFrac(0.90f);
	plConst(hsScalar) kMaxFadeFrac(0.75f);
	const hsScalar fadeFrac = kMinFadeFrac + GetGlobalShadowQuality() * (kMaxFadeFrac - kMinFadeFrac);
	hsScalar minDist = maxDist * fadeFrac;

	// So we want to fade out the shadow as it gets farther away, hopefully
	// pitching it in the distance when we couldn't see it anyway. 
	hsPoint2 depth;
	// We've been testing based on the view direction, which shows unfortunate
	// camera facing dependency (because it is camera facing dependent) with
	// large shadow casters and low shadow quality. Let's try using the vector
	// connecting the caster center with the view position. It's just as wrong,
	// but at least nothing will change when you swing the camera around.
#if 0
	wBnd.TestPlane(castMsg->Pipeline()->GetViewDirWorld(), depth);
	hsScalar eyeDist = castMsg->Pipeline()->GetViewDirWorld().InnerProduct(castMsg->Pipeline()->GetViewPositionWorld());
#else
	hsVector3 dir(&wBnd.GetCenter(), &castMsg->Pipeline()->GetViewPositionWorld());
	hsFastMath::NormalizeAppr(dir);
	wBnd.TestPlane(dir, depth);
	hsScalar eyeDist = dir.InnerProduct(castMsg->Pipeline()->GetViewPositionWorld());
#endif
	hsScalar dist = depth.fX - eyeDist;

	// If it's not far enough to be fading, just go with it as is.
	dist -= minDist;
	if( dist < 0 )
		return slave;

	dist /= maxDist - minDist;
	dist = 1.f - dist;

	// If it's totally faded out, recycle the slave and return nil;
	if( dist <= 0 )
		return IRecycleSlave(slave);

	slave->fPower *= dist;

	return slave;
}

// compute ShadowSlave power influenced by SoftRegion, current light intensity, and ShadowCaster.fMaxOpacity;
hsScalar plShadowMaster::IComputePower(const plShadowCaster* caster, const hsBounds3Ext& casterBnd) const
{
	hsScalar power = 0;
	if( fLightInfo && !fLightInfo->IsIdle() )
	{
		power = caster->fMaxOpacity;
		hsScalar strength, scale;
		fLightInfo->GetStrengthAndScale(casterBnd, strength, scale);
		power *= strength;
	}
	power *= fPower;
	power *= caster->GetBoost();

	return power;
}

void plShadowMaster::IComputeWidthAndHeight(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{
	slave->fWidth = fMaxSize;
	slave->fHeight = fMaxSize;

	if( GetGlobalShadowQuality() <= 0.5f )
	{
		slave->fWidth >>= 1;
		slave->fHeight >>= 1;
	}
	if( castMsg->Caster()->GetLimitRes() )
	{
		slave->fWidth >>= 1;
		slave->fHeight >>= 1;
	}

	const hsBounds3Ext& wBnd = slave->fWorldBounds;

	hsPoint2 depth;
	wBnd.TestPlane(castMsg->Pipeline()->GetViewDirWorld(), depth);
	hsScalar eyeDist = castMsg->Pipeline()->GetViewDirWorld().InnerProduct(castMsg->Pipeline()->GetViewPositionWorld());
	hsScalar dist = depth.fX - eyeDist;
	if( dist < 0 )
		dist = 0;

	slave->fPriority = dist; // Might want to boost the local players priority.

	plConst(hsScalar) kShiftDist = 50.f; // PERSPTEST
//	plConst(hsScalar) kShiftDist = 5000.f; // PERSPTEST
	int iShift = int(dist / kShiftDist);
	slave->fWidth >>= iShift;
	slave->fHeight >>= iShift;

	if( slave->fWidth > fGlobalMaxSize )
		slave->fWidth = fGlobalMaxSize;
	if( slave->fHeight > fGlobalMaxSize )
		slave->fHeight = fGlobalMaxSize;

	const int kMinSize = 32;
	if( slave->fWidth < kMinSize )
		slave->fWidth = kMinSize;
	if( slave->fHeight < kMinSize )
		slave->fHeight = kMinSize;
}

void plShadowMaster::IComputeLUT(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{
	// This needs to go from camera space z to lightspace z, and then translate that
	// into a lookup on U from our LUT.
	// First to get into lightspace, we transform our point by
	// worldToLight * cameraToWorld.
	// Then we want to map like
	// Map 0 => (closest = CasterBnd.Closest), 1 => (CasterBnd.Closest + FalloffDist = farthest)
	// which means a matrix looking like:
	//	0.0, 0.0, 1/(farthest - closest), -closest / (farthest - closest),
	//	0.0, 0.0, 0.0, 0.0,
	//	0.0, 0.0, 0.0, 0.0,
	//	0.0, 0.0, 0.0, 0.0,


	hsBounds3Ext bnd = slave->fCasterWorldBounds;
	bnd.Transform(&slave->fWorldToLight);
	hsScalar farthest = bnd.GetCenter().fZ + slave->fAttenDist;
	hsScalar closest = bnd.GetMins().fZ;

	// Shouldn't this always be negated?
	static hsMatrix44 lightToLut; // Static ensures initialized to all zeros.
	lightToLut.fMap[0][2] = 1/(farthest - closest); 
	lightToLut.fMap[0][3] = -closest / (farthest - closest);

	// This full matrix multiply is a little overkill. Could simplify it quite a bit...
	slave->fRcvLUT = lightToLut * slave->fWorldToLight;

	// For caster, we'll be rendering in light space, so we just need to lut off
	// cameraspace z
	// Can put bias here if needed (probably) by adding small 
	// bias to ShadowSlave.LUTXfm.fMap[0][3]. Bias magnitude would probably be at
	// least 0.5f/256.f to compensate for quantization.

	plConst(hsScalar) kSelfBias = 2.f / 256.f;
	plConst(hsScalar) kOtherBias = -0.5 / 256.f;
#if 0 // MF_NOSELF
	lightToLut.fMap[0][3] += slave->HasFlag(plShadowSlave::kSelfShadow) ? kSelfBias : kOtherBias;
#else // MF_NOSELF
	lightToLut.fMap[0][3] += kSelfBias;
#endif // MF_NOSELF

	if( slave->CastInCameraSpace() )
	{
		slave->fCastLUT = lightToLut * slave->fWorldToLight;
	}
	else
	{
		slave->fCastLUT = lightToLut;
	}

#ifdef MF_HACK_SKIPLUT
	hsMatrix44 hack;
	hack.Reset();
	hack.NotIdentity();
	hack.fMap[0][0] = 0;
	hack.fMap[0][3] = 1.f;
	slave->fCastLUT = hack;
#endif // MF_HACK_SKIPLUT
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// SOURCE CODE ENDS HERE!!!!
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Some notes from when I was working this out, in case I ever need to figure
// out what I was thinking when I set this up.
#if 0 // Notes



MasterShadow

On RenderMsg
	
	Harvest all shadow casters within influence from CullTree

	Assign Shadow (shadow == plLightInfo) to each shadow caster

		Ideally, we want a shadow caster to be a conceptual object
		(like one Avatar), rather than individual spans. Shadow Group ID?

	Each shadow renders it's caster into rendertarget, with:

		ClearAlpha to 255 (alphatest will neutralize any texels not written to with shadowcaster)
		ClearColor to 0 - then a blur will bleed black in, darkening edges, making for softer effect
			around edge of image, which means a softer shadow.
			

		Camera Matrix is from=lightPos, at=casterCenter, up=lightUp

		ViewTransform = framed around casterBnd

		color = (camZ - nearPointOfCasterBound) / (lightYon - nearPointOfCasterBnd)
		alpha = color

	Add all active shadows to pipeline lights (or just enable them)

	During render, if a shadow is affecting the current object, as a final pass:

		T0 = texture from shadow renders caster (UV = projection of pos by shadow Xform
		
		T1 = LUT on vtxPos (same LUT as for color and alpha above).

		Color = T1 - T0
		Alpha = T1 - T0

		Texture blend is Subtract (color and alpha)

		FB AlphaTest = Greater
		FB Blend = Mult

	Gives a linear falloff of shadow intensity from nearPointOfCasterBnd to lightYon

	Can be softened (just blur T0)


	Big problem? On a two TMU system, we're screwed on alpha textures.

	On a 3 (or greater) TMU system, we could:

		// Select first texture
		Stage0
			Color/Alpha
			Arg1 = T0
			Op = SelectArg1

		// Subtract first texture from second (T1 - T0)
		Stage1
			Color/Alpha
			Arg1 = T1
			Arg2 = Current
			Op	= Subtract

		// Add the complement of the orig texture's alpha to the color, so where the texture
		// is transparent, we add 255 and neutralize the shadow, where texture is opaque we
		// add 0 and process normally, and stuff in between.
		Stage2
			Color
			Arg1 = Current
			Arg2 = origTex | D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE;
			Op = Add

				

		Stage0	= T0
		Stage1	= T1
		Stage2	= Original texture

		ColorOp0_1	= Subtract (T0 - T1, inverse of above)
		ColorOp1_2	= Select Current

		AlphaOp0_1	= Subtract (T0 - T1, inverse of above)
		AlphaOp1_2	= Multiply (OrigTex.a * (T0 - T1))


	Okay, time for the bonus round:

		We have 4 cases;

			a) 4 TMU system, base layer opaque
			b) 4 TMU system, base layer has alpha (god help us on base layer has add)
			c) 2 TMU system, base layer opaque
			d) 2 TMU system, base layer has alpha

		If the base layer is opaque, we can do Stage0 and Stage1 as above, whether
			we have 2 or 4 TMU's at our disposal
		If the base layer has alpha, and we have 4 TMU's, we can do the above
		If the base layer has alpha and we have 2 TMU's, we skip it (early out in Apply)

		So, we have the following set up (from above):

			Stage0 = T0
			Stage1 = T1, subtract
			[Stage2 = origTex, add] - only if 4 TMU and origTex has alpha

		In any case, we can add one more stage as long as it's just diffuse (we are
			out of textures on 2 TMU system). So we use the diffuse to modulate the
			effect as follows

			Stage3
				ColorArg1 = Diffuse
				ColorArg2 = current | D3DTA_COMPLEMENT
				ColorOp = Modulate

				AlphaOp = Disable

			The Diffuse contains the value by which to scale the effect, e.g. by SoftRegion
				or artist input.

			Now the alpha coming out is still fine (make sure you set up the alphatest),
				but the color the inverse of what we want. That's okay, our framebuffer
				blend now becomes

				SrcBlend = ZERO
				DstBlend = INVSRCCOLOR

				That means we need to be sure to set the fog color to black

	And that's that. Uh-huh.


Shadow Plan 9

classes:

class plShadowCaster : public plMultiModifier
{
protected:
	class plDrawSpan
	{
	public:
		plDrawableSpans*	fDraw;
		plSpan*				fSpan;
		UInt32				fIndex;
	};

	hsTArray<plDrawSpan> fSpans;

	hsBounds3Ext		fTotalWorldBounds;
	hsScalar			fMaxOpacity;


	On RenderMsg
	{
		// Don't really like having to gather these guys up every frame,
		// but with the avatar customization, it's all pretty volatile,
		// subject to infrequent change, but change without warning.
		// The number of actual targets (and hence shadow casting spans)
		// for any ShadowCasterModifier should always be on the order of
		// 10, so chances are we can get away with this. If not, we can
		// figure some way of caching, like a broadcast message warning us
		// that an avatar customization event has occurred.
		ICollectSpans();

		// Max opacity used to fade out shadows during link

		//find max opacity of all spans
		//clear shadowBits of all spans
		hsScalar fMaxOpacity = 0.f;
		int i;
		for( i = 0; i < fSpans.GetCount(); i++ )
		{
			plLayer* baseLay = fSpans[i].fDraw->GetSubMaterial(fSpans[i].fSpan->fMaterialIdx)->GetLayer(0);
			if( baseLay->GetOpacity() > maxOpacity )
				fMaxOpacity = baseLay->GetOpacity();

			fSpans[i].fSpan->ClearShadowBits();
		}


		if( fMaxOpacity > 0 )
		{
			Broadcast ShadowCastMsg containing
				this (ShadowCaster)
		}
	}

	void ICollectAllSpans()
	{
		fSpans.SetCount(0);
		int i;
		for( i = 0; i < GetNumTargets(); i++ )
		{
			plSceneObject* so = GetTarget(i);
			// Nil target? Shouldn't happen.
			if( so )
			{
				plDrawInterface* di = so->GetDrawInterface();
				// Nil di- either it hasn't loaded yet, or we've been applied to something that isn't visible (oops).
				if( di )
				{
					int j;
					for( j = 0; j < di->GetNumDrawables(); j++ )
					{
						plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
						// Nil dr - it hasn't loaded yet.
						if( dr )
						{
							plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
							if( !diIndex.IsMatrixOnly() )
							{
								int k;
								for( k = 0; k < diIndex.GetCount(); k++ )
								{
									fSpans.Append(dr, dr->GetSpan(diIndex[k]), diIndex[k]);
								}
							}
						}
					}
				}
			}
		}
	}
public:
	plShadowCaster();
	virtual ~plShadowCaster();

	CLASSNAME_REGISTER( plShadowCaster );
	GETINTERFACE_ANY( plShadowCaster, plMultiModifier );
	
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) {}

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
}

class plShadowMaster
{
protected:
	hsTArray<plShadowSlave*>		fSlavePool;

	plVolumeIsect*					fIsect;

	hsScalar						fAttenDist;

	plSoftVolume*					fSoftVolume;

	virtual void IComputeWidthAndHeight(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeProjections(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeLUT(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeISect(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeBounds(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;

	// Override if you want to attenuate (e.g. dist for omni, cone angle for spot).
	// But make sure you factor in base class power.
	virtual hsScalar IComputePower(const plShadowCaster* caster);

public:
	plVolumeIsect* GetIsect() const { return fIsect; }

	hsBool CanSee(const hsBounds3Ext& bnd)
	{
		switch( fType )
		{
		case kSpot:
			return GetIsect().Test(bnd) != kVolumeCulled;
		case kDirectional:
			return true;
		case kLtdDirection:
			return GetIsect().Test(bnd) != kVolumeCulled;
		case kOmni:
			return GetIsect().Test(bnd) != kVolumeCulled;
		default:
			return false;
		}
	}

	virtual void CreateShadowSlave(const hsBounds3Ext& bnd, hsScalar power)
	{
		int iSlave = fSlavePool.GetCount();
		fSlavePool.ExpandAndZero(iSlave+1);
		plShadowSlave* slave = fSlavePool[iSlave];
		if( !slave )
		{
			fSlavePool[iSlave] = slave = TRACKED_NEW plShadowSlave;
			fISectPool[iSlave] = INewISect();
		}

		slave.SetIndex(iSlave); // To be used in not shadowing our own casters

		slave->fPower = power;

		IComputeWidthAndHeight(bnd, slave);

		IComputeWorldToLight(bnd, slave);

		IComputeProjections(bnd, slave);

		IComputeLUT(bnd, slave);

		IComputeISect(bnd, slave, iSlave);

		IComputeBounds(bnd, slave);
	}
};

// compute ShadowSlave power influenced by SoftRegion and ShadowCaster.fMaxOpacity;
hsScalar plShadowMaster::ComputePower(const plShadowCaster* caster)
{
	hsScalar power = caster->fMaxOpacity;
	if( fSoftVolume )
	{
		power *= fSoftVolume->GetStrength(caster->fTotalWorldBounds.GetCenter());
	}
	return power;
}

class OmniShadowMaster : public plShadowMaster
{
protected:
	hsTArray<plVolumeIsect*>		fIsectPool;

	virtual void IComputeWidthAndHeight(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
	virtual void IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
	virtual void IComputeProjections(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
	virtual void IComputeLUT(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
	virtual void IComputeISect(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
	virtual void IComputeBounds(const hsBounds3Ext& bnd, plShadowSlave* slave) const;
};

void plOmniShadowMaster::IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const
{
	hsPoint3 from = fPosition;
	hsPoint3 at = bnd.GetCenter();
	hsVector3 up = fLastUp;
	if( (up % (at - from)).MagnitudeSqaured() < kMinMag )
	{
		up.Set(0, 1.f, 0);
		if( (up % (at - from)).MagnitudeSqaured() < kMinMag )
		{
			up.Set(0, 0, 1.f);
		}
		fLastUp = up;
	}
	slave->fWorldToLight.MakeCamera(&from, &at, &up);
}

void plOmniShadowMaster::IComputeProjections(const hsBounds3Ext& wBnd, plShadowSlave* slave) const
{
	hsBounds3Ext bnd = wBnd;
	bnd.Transform(slave->fWorldToLight);

	hsScalar minZ = bnd.GetMins().fZ;
	hsScalar maxZ = bnd.GetCenter().fZ + fAttenDist;

	if( minZ < kMinMinZ )
		minZ = kMinMinZ;

	hsScalar cotX, cotY;
	if( -bnd.GetMins().fX > bnd.GetMaxs().fX )
	{
		hsAssert(bnd.GetMins().fX < 0, "Empty shadow caster bounds?");
		cotX = -minZ / bnd.GetMins().fX;
	}
	else
	{
		hsAssert(bnd.GetMaxs().fX > 0, "Empty shadow caster bounds?");
		cotX = minZ / bnd.GetMaxs().fX;
	}

	if( -bnd.GetMins().fY > bnd.GetMaxs().fY )
	{
		hsAssert(bnd.GetMins().fY < 0, "Empty shadow caster bounds?");
		cotY = -minZ / bnd.GetMins().fY;
	}
	else
	{
		hsAssert(bnd.GetMaxs().fY > 0, "Empty shadow caster bounds?");
		cotY = minZ / bnd.GetMaxs().fY;
	}


	hsMatrix44 proj;
	proj.Reset();
	proj.NotIdentity();

	// First the LightToTexture, which uses the above pretty much as is.
	// Note the remapping to range [0.5..width-0.5] etc. Also, the perspective
	// divide is by the 3rd output (not the fourth), so we make the 3rd
	// output be W (instead of Z).
	proj.fMap[0][0] = cotX * 0.5f;
	proj.fMap[0][3] = 0.5f * (1.f + 1.f/slave->fWidth);
	proj.fMap[1][1] = -cotY * 0.5f;
	proj.fMap[1][3] = 0.5f * (1.f + 1.f/slave->fHeight);
#if 0 // This computes correct Z, but we really just want W in 3rd component.
	proj.fMap[2][2] = maxZ / (maxZ - minZ);
	proj.fMap[2][3] = minZ * maxZ / (maxZ - minZ);
#else
	proj.fMap[2][2] = 1.f;
	proj.fMap[2][3] = 0;
#endif
	proj.fMap[3][3] = 0;
	proj.fMap[3][2] = 1.f;

	slave->fLightToTexture = proj;
	slave->fCameraToTexture = slave->fLightToTexture * slave->fWorldToLight * pipe->GetCameraToWorld();

	// Now the LightToNDC. This one's a little trickier, because we want to compensate for
	// having brought in the viewport to keep our border constant, so we can clamp the 
	// projected texture and not have the edges smear off to infinity.
	cotX -= cotX / (slave->fWidth * 0.5f);
	cotY -= cotY / (slave->fHeight * 0.5f);

	proj.fMap[0][0] = cotX;
	proj.fMap[0][3] = 0.f;
	proj.fMap[1][1] = cotY;
	proj.fMap[1][3] = 0.f;
	proj.fMap[2][2] = maxZ / (maxZ - minZ);
	proj.fMap[2][3] = minZ * maxZ / (maxZ - minZ);
	proj.fMap[3][3] = 0;
	proj.fMap[3][2] = 1.f;

	slave->fLightToNDC = proj;
}

class plShadowSlave
{
public:

	hsMatrix44			fWorldToLight;
	hsMatrix44			fLightToNDC;
	hsMatrix44			fLightToTexture;
	hsMatrix44			fCastLUT;
	hsMatrix44			fRcvLUT;

	hsScalar			fPower;

	plVolumeIsect*		fISect;

	UInt32				fWidth;
	UInt32				fHeight;
};

BeginScene (on EvalMsg?)
{
	ShadowMasters ClearShadowSlaves(); // fSlavePool.SetCount(0); fISectPool.SetCount(0);
}

EndScene
{
	pipeline->ClearShadowSlaves();
}

Harvest
{
	ShadowMasters wait for ShadowCastMsg broadcast

	On ShadowCastMsg

		if( !ShadowMaster.CanSee(ShadowCaster.fTotalWorldBounds) )
			forget it;

		hsScalar power = ComputePower(ShadowCaster);

		if( power == 0 )
			forget it;

		// Create ShadowSlave focused on ShadowCaster
		// ShadowSlave extent just enough to cover ShadowCaster (including nearplane)
		CreateShadowSlave(ShadowCaster.fTotalWorldBounds, power);

		// !!!IMPORTANT
		// ShadowMaster contains 2 values for yon.
		// First value applies to ShadowMaster. Any ShadowCaster beyond this distance
		//		won't cast a shadow
		// Second value applies to ShadowSlaves. This is the distance beyond the ShadowCaster
		//		(NOT FROM SHADOW SOURCE) over which the shadow attenuates to zero
		// The effective yon for the ShadowSlave is ShadowSlaveYon + DistanceToFarthestPointOnShadowCasterBound
		//		That's the distance used for culling ShadowReceivers
		// The ShadowSlaveYon is used directly in the 

		if ShadowSlave extent not visible to current camera
			forget it;

		ShadowSlave.Generate

		Submit to pipeline

	endOnMsg

	endfor
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
Pipeline functions
/////////////////////////////////////////////////////////////////////////////////////////////

// Puts the slave in a list valid for this frame only. The list will
// be preprocessed at BeginRender. See IPreprocessShadows.
void SubmitShadowSlave(plShadowSlave* slave);

// rendering all the associated spans into
// a rendertarget of the correct size

// We have a (possibly empty) list of shadows submitted for this frame.
// At BeginRender, we need to accomplish:
//	Find render targets for each shadow request of the requested size.
//	Render the associated spans into the render targets. Something like the following:
void IPreprocessShadows()
{

	SetupShadowCastTextureStages - see below

	for each shadowCaster.fSpans
	{
		render shadowcaster.fSpans[i] to rendertarget

		shadowCaster.fSpans[i]->SetShadowBit(shadowCaster.fIndex); //index set in CreateShadowSlave
	}

	Blur rendertarget (optional);

	// Must ensure we have an alpha border of 255 (for clamping the effect)
	//SetBorderTo255(); we don't have to do this if we can set the viewport
	// to leave a border and compensate the fov so LightToNDC and LightToTexture match up.
}

// After doing the usual render for a span (all passes), we call the following.
// If the span accepts shadows, this will loop over all the shadows active this
// frame, and apply the ones that intersect this spans bounds. See below for details.
void IRenderSpanShadows();

// At EndRender(), we need to clear our list of shadow slaves. They are only valid for one frame.
void IClearShadowSlaves();

// We don't have the depth resolution to even think about self shadowing, so we just don't
// let a slave shadow any of the spans that were rendered into it.
hsBool AcceptsShadow(plSpan* span, plShadowSlave* slave)
{
	return !span->IsShadowBitSet(slave->fIndex);
}

// Want artists to be able to just disable shadows for spans where they'll either
// look goofy, or won't contribute.
// Also, if we have less than 3 simultaneous textures, we want to skip anything with
// an alpha'd base layer, unless it's been overriden.
hsBool ReceivesShadows(plSpan* span, hsGMaterial* mat)
{
	if( span.fProps & plSpan::kPropNoShadow )
		return false;

	if( span.Props & plSpan::kPropForceShadow )
		return true;

	if( (fMaxLayersAtOnce < 3) && (mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha) )
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

SetBorderTo255()
{
	Set FB blend to Add;

	render a texture the same size as the render target to the render target
	as a single quad. Texture has black as color, 0 as alpha except the border
	which is black with alpha=255.
}

Apply
{
	render all passes of span

	if( ShadowSlaveListNotEmpty() )
		RenderSpanShadows
}

RenderSpanShadows
{
	hsBool first = true;
	if receivesShadows(span)
	{
		for each ShadowSlave
		{

			if AcceptsShadow(span, ShadowSlave) && (ShadowSlave->fIsect->Test(span.fBounds) != kVolumeCulled)
			{
				if( first )
				{
					SetupShadowRcvTextureStages();
					first = false;
				}

				SetupShadowSlaveTextures()

				render span

			}
		}
	}
}

SetupShadowSlaveTextures()
{
	// See below
}

TRANSFORMS
==========

Summary -
	ShadowSlave.W2Light - world space to space of shadow slave
	ShadowSlave.Light2W - unused

	ShadowSlave.LightToNDC - normal projection matrix, maps to 
		[-1,1], [-1,1], [0,1] (after divide)
		AND
		has fov decreased slightly to compensate for the viewport being
			brought down to preserve the border

	ShadowSlave.LightToTexture - like LightToNDC, but maps to
		[0.5 / width, 1 - 0.5/width], [0.5/height, 1 - 0.5/height, [0,1]
		fov NOT brought down for border

	ShadowSlave.CameraToTexture = ShadowSlave.LightToTexture * ShadowSlave.W2Light * pipe->GetCameraToWorld();

	ShadowSlave.CasterLUTXfm - see below

	ShadowSlave.ViewPort = {1, 1, width-2, height-2, 0, 1}

To Compensate FOV for border
{
	if( perspective ) // spots and omnis
	{
		delX = fovX / (txtWidth/2);
		delY = fovY / (txtHeight/2);
		fovX -= delX;
		fovY -= delY;
	}
	else // directional
	{
		delX = width / (txtWidth/2);
		delY = height / (txtHeight/2);

		minX += delX;
		minY += delY;
		maxX -= delX;
		maxY -= delY;
	}
}

To Render ShadowCaster
{
	render transform to ShadowSlave.W2Light * ShadowCaster.L2W

	projection transform to ShadowSlave.LightToNDC

	viewPort to ShadowSlave.ViewPort

	Stage0 -
		UVWSrc = CameraSpacePos

		UVWXfm = ShadowSlave.CasterLUTXfm * ShadowSlave.W2L * CameraToWorld

		Texture = U_LUT

	Stage1 -
		Disable
}

ShadowSlave.LUTXfm
{
	// Map 0 => (closest = CasterBnd.Closest), 1 => (CasterBnd.Closest + FalloffDist = farthest)
	0.0, 0.0, 1/(farthest - closest), -closest / (farthest - closest),
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,

	// FOR CASTER ONLY
	// Can put bias here if needed (probably) by adding small NEGATIVE
	// bias to ShadowSlave.LUTXfm.fMap[0][3]. Bias magnitude would probably be at
	// least 0.5f/256.f to compensate for quantization.
}

To Project onto Shadow Receiver // SetupShadowSlaveTexture
{
	render transform = current;
	project transform = current;
	viewport = current;

	Stage0 - 
		UVWSrc = CameraSpacePos

		UVWXfm = ShadowSlave.LightToTexture * ShadowSlave.W2L * CameraToWorld

		Texture = ShadowMap

	Stage1 -
		UVWSrc = CameraSpacePos

		UVWXfm = ShadowSlave.RcvLUTXfm * ShadowSlave.W2L * CameraToWorld

		Texture = U_LUT

	[ // Optional for when have > 2 TMUs AND base texture is alpha
	Stage2 -
		Process base texture normally normally
	]
	Stage2/3
		No texture - setup as in ShadowNotes.h
}

ShadowSlave.Offset
{
	Offset =
	{
		0.5, 0.0, 0.0, 0.5 + 0.5 * ShadowSlave.Width,
		0.0, 0.5, 0.0, 0.5 + 0.5 * ShadowSlave.Height,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	}
}

#endif // Notes
