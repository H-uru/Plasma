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
#include "plPipeline.h"
#include "plTweak.h"
#include "hsFastMath.h"

#include "plPerspDirSlave.h"

#include <float.h>

void plPerspDirSlave::Init() 
{ 
	plShadowSlave::Init();
	fFlags |= kCastInCameraSpace; 
}

hsPoint3 plPerspDirSlave::IProject(const hsMatrix44& world2NDC, const hsPoint3& pos, hsScalar w) const
{
	hsPoint3 retVal;
	retVal.fX = world2NDC.fMap[0][0] * pos.fX
		+ world2NDC.fMap[0][1] * pos.fY
		+ world2NDC.fMap[0][2] * pos.fZ
		+ world2NDC.fMap[0][3] * w;

	retVal.fY = world2NDC.fMap[1][0] * pos.fX
		+ world2NDC.fMap[1][1] * pos.fY
		+ world2NDC.fMap[1][2] * pos.fZ
		+ world2NDC.fMap[1][3] * w;

	retVal.fZ = world2NDC.fMap[2][0] * pos.fX
		+ world2NDC.fMap[2][1] * pos.fY
		+ world2NDC.fMap[2][2] * pos.fZ
		+ world2NDC.fMap[2][3] * w;

	hsScalar invW = 1.f / (
		world2NDC.fMap[3][0] * pos.fX
		+ world2NDC.fMap[3][1] * pos.fY
		+ world2NDC.fMap[3][2] * pos.fZ
		+ world2NDC.fMap[3][3] * w);

	retVal *= invW;

	return retVal;
}

hsBounds3Ext plPerspDirSlave::IGetPerspCasterBound(const hsMatrix44& world2NDC) const
{
	hsPoint3 corners[8];
	fCasterWorldBounds.GetCorners(corners);
	hsPoint3 perspCorners[8];

	int i;
	for( i = 0; i < 8; i++ )
	{
		perspCorners[i] = IProject(world2NDC, corners[i]);

		if( perspCorners[i].fX < -1.f )
			perspCorners[i].fX = -1.f;
		else if( perspCorners[i].fX > 1.f )
			perspCorners[i].fX = 1.f;

		if( perspCorners[i].fY < -1.f )
			perspCorners[i].fY = -1.f;
		else if( perspCorners[i].fY > 1.f )
			perspCorners[i].fY = 1.f;

		if( perspCorners[i].fZ < 0 )
			perspCorners[i].Set(0.f, 0.f, 0.f);
	}
	hsBounds3Ext bnd;
	bnd.MakeEmpty();
	for( i = 0; i < 8; i++ )
		bnd.Union(&perspCorners[i]);

	return bnd;
}

bool plPerspDirSlave::SetupViewTransform(plPipeline* pipe)
{
	plViewTransform pipeView = pipe->GetViewTransform();

	plConst(hsScalar) kYon(100.f);
	plConst(hsScalar) kHither(30.f);
	pipeView.SetHither(kHither);
	pipeView.SetYon(kYon);

	hsMatrix44 cam2NDC = pipeView.GetCameraToNDC();
	hsMatrix44 world2NDC = cam2NDC * pipeView.GetWorldToCamera();
	
	fLightDir = fLightToWorld.GetAxis(hsMatrix44::kUp);
	hsVector3 worldLiDir = fLightDir;
	hsPoint3 pWorldLiDir(worldLiDir.fX, worldLiDir.fY, worldLiDir.fZ);
	hsPoint3 perspLiPos = IProject(world2NDC, pWorldLiDir, 0);

	hsBool reverseZ = fLightDir.InnerProduct(pipe->GetViewDirWorld()) > 0;
	SetFlag(kReverseZ, reverseZ);
	SetFlag(kReverseCull, reverseZ);

	hsPoint3 lookAt;
	plConst(hsBool) kUsePerspCenter(true);
	plConst(hsBool) kUseFrustCenter(true);
	if( kUsePerspCenter )
	{
		hsPoint3 lookAtCam = pipeView.GetWorldToCamera() * fCasterWorldBounds.GetCenter();
		hsPoint3 lookAtNDC = IProject(pipeView.GetCameraToNDC(), lookAtCam);
		lookAt = IProject(world2NDC, fCasterWorldBounds.GetCenter());
	}
	else if( kUseFrustCenter )
	{
		plConst(hsScalar) kDist(50.f);
		hsPoint3 camFrustCenter(0.f, 0.f, kDist);
		lookAt = IProject(cam2NDC, camFrustCenter);
	}
	else
	{
		hsBounds3Ext ndcBnd(IGetPerspCasterBound(world2NDC));
		lookAt = ndcBnd.GetCenter();
	}

	hsMatrix44 camNDC2Li;
	hsMatrix44 li2CamNDC;
	IComputeCamNDCToLight(perspLiPos, lookAt, camNDC2Li, li2CamNDC);

	hsScalar minZ, maxZ;
	hsScalar cotX, cotY;

	plConst(hsBool) kFixedPersp(true);
	if( !kFixedPersp )
	{
		hsBounds3Ext bnd(IGetPerspCasterBound(camNDC2Li * world2NDC));
		hsBounds3Ext bnd2(IGetPerspCasterBound(world2NDC));
		bnd2.Transform(&camNDC2Li);
		plConst(hsBool) kUseBnd2(false);
		if( kUseBnd2 )
			bnd = bnd2;

		minZ = bnd.GetMins().fZ;
		maxZ = bnd.GetMaxs().fZ; // THIS IS WRONG

		// EAP
		// This is my hack to get the Nexus age working.  The real problem
		// is probably data-side.  I take full responsibility for this
		// hack-around breaking the entire system, loosing data, causing
		// unauthorized credit card transactions, etc.		
		if (_isnan(bnd.GetMins().fX) || _isnan(bnd.GetMins().fY))
			return false;
		if (_isnan(bnd.GetMaxs().fX) || _isnan(bnd.GetMaxs().fY))
			return false;

		// THIS IS EVEN MORE WRONG
		plConst(hsBool) kFakeDepth(false);
		if( kFakeDepth )
		{
			plConst(hsScalar) kMin(1.f);
			plConst(hsScalar) kMax(30.f);
			minZ = kMin;
			maxZ = kMax;
		}

		plConst(hsScalar) kMinMinZ(1.f);
		if( minZ < kMinMinZ )
			minZ = kMinMinZ;

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
	}
	else
	{
		plConst(hsScalar) kHi(1.f);
		hsBounds3Ext bnd;
		const hsPoint3 lo(-1.f, -1.f, 0.f);
		const hsPoint3 hi(1.f, 1.f, kHi);
		bnd.MakeEmpty();
		bnd.Union(&lo);
		bnd.Union(&hi);

		bnd.Transform(&camNDC2Li);

		minZ = bnd.GetMins().fZ;
		maxZ = bnd.GetMaxs().fZ; // THIS IS WRONG

		// EAP
		// This is my hack to get the Nexus age working.  The real problem
		// is probably data-side.  I take full responsibility for this
		// hack-around breaking the entire system, loosing data, causing
		// unauthorized credit card transactions, etc.		
		if (_isnan(bnd.GetMins().fX) || _isnan(bnd.GetMins().fY))
			return false;
		if (_isnan(bnd.GetMaxs().fX) || _isnan(bnd.GetMaxs().fY))
			return false;

		plConst(hsScalar) kMinMinZ(1.f);
		if( minZ < kMinMinZ )
			minZ = kMinMinZ;

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
	}


	hsMatrix44 proj;
	proj.Reset();
	proj.NotIdentity();

	// LightToTexture

	// First the LightToTexture, which uses the above pretty much as is.
	// Note the remapping to range [0.5..width-0.5] etc. Also, the perspective
	// divide is by the 3rd output (not the fourth), so we make the 3rd
	// output be W (instead of Z).
	// This also means that our translate goes into [i][2] instead of [i][3].
	proj.fMap[0][0] = cotX * 0.5f;
	proj.fMap[0][2] = 0.5f * (1.f + 0.5f/fWidth);
	proj.fMap[1][1] = -cotY * 0.5f;
	proj.fMap[1][2] = 0.5f * (1.f + 0.5f/fHeight);
#if 1 // This computes correct Z, but we really just want W in 3rd component. HACKFISH
	proj.fMap[2][2] = maxZ / (maxZ - minZ);
	proj.fMap[2][3] = -minZ * maxZ / (maxZ - minZ);
#elif 1
	proj.fMap[2][2] = 1.f;
	proj.fMap[2][3] = 0;
#endif
	proj.fMap[3][2] = 1.f;
	proj.fMap[3][3] = 0;

//	fWorldToTexture = proj * camNDC2Li * pipeView.GetWorldToNDC();
	fWorldToTexture = proj * camNDC2Li * world2NDC;
	fWorldToTexture.fMap[2][0] = fWorldToTexture.fMap[3][0];
	fWorldToTexture.fMap[2][1] = fWorldToTexture.fMap[3][1];
	fWorldToTexture.fMap[2][2] = fWorldToTexture.fMap[3][2];
	fWorldToTexture.fMap[2][3] = fWorldToTexture.fMap[3][3];

	// Now the LightToNDC. This one's a little trickier, because we want to compensate for
	// having brought in the viewport to keep our border constant, so we can clamp the 
	// projected texture and not have the edges smear off to infinity.
	cotX -= cotX / (fWidth * 0.5f);
	cotY -= cotY / (fHeight * 0.5f);

	hsScalar tanX = 1.f / cotX;
	hsScalar tanY = 1.f / cotY;
	fView.SetScreenSize((UInt16)fWidth, (UInt16)fHeight);
	fView.SetCameraTransform(pipe->GetViewTransform().GetWorldToCamera(), pipe->GetViewTransform().GetCameraToWorld());
	fView.SetPerspective(true);
	fView.SetViewPort(0, 0, (float)fWidth, (float)fHeight, false);
	fView.SetView(hsPoint3(-tanX, -tanY, minZ), hsPoint3(tanX, tanY, maxZ));

	hsMatrix44 cam2Light = camNDC2Li * pipeView.GetCameraToNDC();
	fView.PostMultCameraToNDC(cam2Light);

	fLightPos = fLightToWorld.GetTranslate();
	SetFlag(kPositional, false);
	
	return true;
}

static inline hsVector3 CrossProd(const hsVector3& a, const hsPoint3& b)
{
	return hsVector3(a.fY*b.fZ - a.fZ*b.fY, a.fZ*b.fX - a.fX*b.fZ, a.fX*b.fY - a.fY*b.fX);
}

static inline void InverseOfPureRotTran(const hsMatrix44& src, hsMatrix44& inv)
{
	inv = src;

	// We know this is a pure rotation and translation matrix, so
	// we won't have to do a full inverse. Okay kids, don't try this
	// at home.
	inv.fMap[0][1] = src.fMap[1][0];
	inv.fMap[0][2] = src.fMap[2][0];

	inv.fMap[1][0] = src.fMap[0][1];
	inv.fMap[1][2] = src.fMap[2][1];

	inv.fMap[2][0] = src.fMap[0][2];
	inv.fMap[2][1] = src.fMap[1][2];

	hsPoint3 newTran(-src.fMap[0][3], -src.fMap[1][3], -src.fMap[2][3]);
	inv.fMap[0][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[0][0]);
	inv.fMap[1][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[1][0]);
	inv.fMap[2][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[2][0]);
}

void plPerspDirSlave::IComputeCamNDCToLight(const hsPoint3& from, const hsPoint3& at, hsMatrix44& camNDC2Li, hsMatrix44& li2CamNDC)
{

	hsVector3 atToFrom(&from, &at);
	hsScalar distSq = atToFrom.MagnitudeSquared();
	atToFrom *= hsFastMath::InvSqrtAppr(distSq);

	const hsScalar kMinMag = 0.5f;
	hsVector3 up(0,0,1.f);
	if( CrossProd(up, (at - from)).MagnitudeSquared() < kMinMag )
	{
		up.Set(0, 1.f, 0);
	}
	hsMatrix44 w2light;
	w2light.MakeCamera(&from, &at, &up);

	hsMatrix44 light2w;
	InverseOfPureRotTran(w2light, light2w);

#ifdef CHECK_INVERSE
	hsMatrix44 inv;
	w2light.GetInverse(&inv);
#endif // CHECK_INVERSE

	camNDC2Li = w2light;
	li2CamNDC = light2w;
}