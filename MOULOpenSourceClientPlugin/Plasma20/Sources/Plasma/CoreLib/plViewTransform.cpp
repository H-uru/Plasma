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
#include "hsBounds.h"
#include "hsStream.h"
#include "plViewTransform.h"

const hsScalar plViewTransform::kMinHither = 0.25f;

plViewTransform::plViewTransform()
:	fFlags(kViewPortRelative),
	fWidth(0),
	fHeight(0)
{
	fCameraToWorld.Reset();
	fWorldToCamera.Reset();

	fViewPortX.Set(0,1.f,1.f);
	fViewPortY.Set(0,1.f,1.f);

	fMapMin.Set(0.f, 0.f, 0.f);
	fMapMax.Set(1.f, 1.f, 1.f);
}

void plViewTransform::Reset()
{
	fFlags = kViewPortRelative;
	fCameraToWorld.Reset();
	fWorldToCamera.Reset();

	fViewPortX.Set(0,1.f,1.f);
	fViewPortY.Set(0,1.f,1.f);
}

void plViewTransform::ISetCameraToNDC() const
{
	fCameraToNDC.Reset();
	fCameraToNDC.NotIdentity();

	if( GetOrthogonal() )
	{
		hsPoint3	worldSizeInv;

		worldSizeInv.fX = hsScalarInvert( fMax.fX - fMin.fX ) * 2.f;
		worldSizeInv.fY = hsScalarInvert( fMax.fY - fMin.fY ) * 2.f;
		worldSizeInv.fZ = hsScalarInvert( fMax.fZ - fMin.fZ );

		fCameraToNDC.fMap[0][0] = worldSizeInv.fX;
		fCameraToNDC.fMap[0][3] = -fMin.fX * worldSizeInv.fX - hsScalar1;

		fCameraToNDC.fMap[1][1] = worldSizeInv.fY;
		fCameraToNDC.fMap[1][3] = -fMin.fY * worldSizeInv.fY - hsScalar1;

		// Map Screen Z to range 0 (at hither) to 1 (at yon)
		fCameraToNDC.fMap[2][2] = worldSizeInv.fZ;
		fCameraToNDC.fMap[2][3] = -fMin.fZ * worldSizeInv.fZ;
	}
	else
	{

		fCameraToNDC.fMap[0][0] = 2.f / (fMax.fX - fMin.fX);
		fCameraToNDC.fMap[0][2] = (fMax.fX + fMin.fX) / (fMax.fX - fMin.fX);

		fCameraToNDC.fMap[1][1] = 2.f / (fMax.fY - fMin.fY);
		fCameraToNDC.fMap[1][2] = (fMax.fY + fMin.fY) / (fMax.fY - fMin.fY);

		fCameraToNDC.fMap[2][2] = fMax.fZ / (fMax.fZ - fMin.fZ);
		fCameraToNDC.fMap[2][3] = -fMax.fZ * fMin.fZ / (fMax.fZ - fMin.fZ);

		fCameraToNDC.fMap[3][2] = 1.f;
		fCameraToNDC.fMap[3][3] = 0.f;

	}
	ISetFlag(kCameraToNDCSet);
}

void plViewTransform::SetViewPort(const hsPoint2& mins, const hsPoint2& maxs, hsBool relative)
{ 
	fViewPortX.Set(mins.fX, maxs.fX, 1.f / (maxs.fX - mins.fX)); 
	fViewPortY.Set(mins.fY, maxs.fY, 1.f / (maxs.fY - mins.fY)); 
	ISetFlag(kViewPortRelative, relative);
}

hsScalarTriple plViewTransform::ScreenToNDC(const hsScalarTriple& scrP) const
{
	hsPoint2 vpMin, vpMax;
	GetViewPort(vpMin, vpMax);
	hsPoint3 ndc;

	ndc.fX = (scrP.fX - vpMin.fX) / (vpMax.fX - vpMin.fX) * 2.f - 1.f;

	ndc.fY = (vpMax.fY - scrP.fY) / (vpMax.fY - vpMin.fY) * 2.f - 1.f;

	ndc.fZ = scrP.fZ;
	
	return ndc;
}

hsScalarTriple plViewTransform::NDCToScreen(const hsScalarTriple& ndc) const
{
	hsPoint2 vpMin, vpMax;
	GetViewPort(vpMin, vpMax);

	hsPoint3 scrP;
	scrP.fX = (ndc.fX + 1.f) * 0.5f * (vpMax.fX - vpMin.fX) + vpMin.fX;
	scrP.fY = (-ndc.fY + 1.f) * 0.5f * (vpMax.fY - vpMin.fY) + vpMin.fY;
	scrP.fZ = ndc.fZ;

	return scrP;
}

hsScalarTriple plViewTransform::NDCToCamera(const hsScalarTriple& ndc) const
{
	hsPoint3 camP;
	hsScalar w = ndc.fZ;

	const hsMatrix44& c2NDC = GetCameraToNDC();

	camP.fX = (ndc.fX - c2NDC.fMap[0][2]) * w / c2NDC.fMap[0][0];

	camP.fY = (ndc.fY - c2NDC.fMap[1][2]) * w / c2NDC.fMap[1][1];

	camP.fZ = ndc.fZ;

	return camP;
}

hsScalarTriple plViewTransform::CameraToNDC(const hsScalarTriple& camP) const
{
	const hsMatrix44& c2NDC = GetCameraToNDC();
	
#ifdef MF_FLIP_SPARSE
	// We count on the fact that we set up CameratToNDC, so we know where the
	// zeros are. Also, note that the proper "* camP.fZ"'s are missing off the
	// c2NDC.fMap[i][2] terms, because they just get cancelled out by the invW.

	hsPoint3 ndc;
	if( GetOrthogonal() )
	{
		ndc.fX = c2NDC.fMap[0][0] * camP.fX 
			+ c2NDC.fMap[0][2];

		ndc.fY = c2NDC.fMap[1][1] * camP.fY 
			+ c2NDC.fMap[1][2];

		ndc.fZ = c2NDC.fMap[2][2] * camP.fZ 
			+ c2NDC.fMap[2][3];
	}
	else
	{
		hsScalar invW = 1.f / camP.fZ;
		ndc.fX = c2NDC.fMap[0][0] * camP.fX * invW
			+ c2NDC.fMap[0][2];

		ndc.fY = c2NDC.fMap[1][1] * camP.fY * invW
			+ c2NDC.fMap[1][2];

		ndc.fZ = c2NDC.fMap[2][2] * camP.fZ 
			+ c2NDC.fMap[2][3];
		ndc.fZ *= invW;
	}
#else // MF_FLIP_SPARSE
	hsPoint3 ndc = c2NDC * hsPoint3(camP);
	if( !GetOrthogonal() )
	{
		hsScalar invW = 1.f / camP.fZ;
		ndc *= invW;
	}
#endif // MF_FLIP_SPARSE

	return ndc;
}

hsScalarTriple plViewTransform::NDCToMap(const hsScalarTriple& ndcP) const
{
	hsPoint3 map;
	map.fX = fMapMin.fX + (ndcP.fX + 1.f) * 0.5f * (fMapMax.fX - fMapMin.fX);
	map.fY = fMapMin.fY + (ndcP.fY + 1.f) * 0.5f * (fMapMax.fY - fMapMin.fY);
	map.fZ = fMapMin.fZ + (ndcP.fZ + 1.f) * 0.5f * (fMapMax.fZ - fMapMin.fZ);

	return map;
}

hsBool plViewTransform::SetProjection(const hsBounds3& bnd)
{
	hsPoint3 maxs;
	hsPoint3 mins;
	if( IGetMaxMinsFromBnd(bnd, mins, maxs) )
	{
		SetView(mins, maxs);
		return true;
	}
	return false;
}

hsBool plViewTransform::SetProjectionWorld(const hsBounds3& wBnd)
{
	hsBounds3Ext cBnd = wBnd;
	cBnd.Transform(&GetWorldToCamera());
	return SetProjection(cBnd);
}

hsBool plViewTransform::IGetMaxMinsFromBnd(const hsBounds3& bnd, hsPoint3& mins, hsPoint3& maxs) const
{
	if( bnd.GetMaxs().fZ <= kMinHither )
		return false;

	hsPoint3 minBnd = bnd.GetMins();
	hsPoint3 maxBnd = bnd.GetMaxs();
	// If the box intersects the hither plane, we'll need to chop it
	// off.
	if( minBnd.fZ < kMinHither )
	{
		minBnd.fZ = kMinHither;		
	}
	mins.Set(minBnd.fX / minBnd.fZ, minBnd.fY / minBnd.fZ, minBnd.fZ);
	maxs.Set(maxBnd.fX / minBnd.fZ, maxBnd.fY / minBnd.fZ, maxBnd.fZ);

	return true;
}

hsBool plViewTransform::Intersect(const plViewTransform& view)
{
	hsPoint3 mins;
	hsPoint3 maxs;

	hsBool retVal = true;
	int i;
	for( i = 0; i < 3; i++ )
	{
		mins[i] = hsMaximum(fMin[i], view.fMin[i]);

		maxs[i] = hsMinimum(fMax[i], view.fMax[i]);

		if( mins[i] >= maxs[i] )
		{
			mins[i] = maxs[i] = (mins[i] + maxs[i]) * 0.5f;
			retVal = false;
		}
	}
	SetView(mins, maxs);
	return retVal;
}

hsBool plViewTransform::Union(const plViewTransform& view)
{
	hsPoint3 mins;
	hsPoint3 maxs;

	int i;
	for( i = 0; i < 3; i++ )
	{
		mins[i] = hsMinimum(fMin[i], view.fMin[i]);

		maxs[i] = hsMaximum(fMax[i], view.fMax[i]);

	}
	SetView(mins, maxs);
	return true;
}

hsScalar plViewTransform::GetFovX() const
{
	hsScalar minAng = hsATan2(fMin.fX, 1.f);
	hsScalar maxAng = hsATan2(fMax.fX, 1.f);

	return maxAng - minAng;
}

hsScalar plViewTransform::GetFovY() const
{
	hsScalar minAng = hsATan2(fMin.fY, 1.f);
	hsScalar maxAng = hsATan2(fMax.fY, 1.f);

	return maxAng - minAng;
}

void plViewTransform::GetViewPort(hsPoint2& mins, hsPoint2& maxs) const 
{ 
	if( GetViewPortRelative() )
	{
		mins.Set(fViewPortX.fX * fWidth, fViewPortY.fX * fHeight); 
		maxs.Set(fViewPortX.fY * fWidth, fViewPortY.fY * fHeight); 
	}
	else
	{
		mins.Set(fViewPortX.fX, fViewPortY.fX); 
		maxs.Set(fViewPortX.fY, fViewPortY.fY); 
	}
}

void plViewTransform::GetViewPort(int& loX, int& loY, int& hiX, int& hiY) const 
{ 
	if( GetViewPortRelative() )
	{
		loX = int(fViewPortX.fX * fWidth); 
		loY = int(fViewPortY.fX * fHeight); 
		hiX = int(fViewPortX.fY * fHeight); 
		hiY = int(fViewPortY.fY * fWidth); 
	}
	else
	{
		loX = int(fViewPortX.fX); 
		loY = int(fViewPortY.fX); 
		hiX = int(fViewPortX.fY); 
		hiY = int(fViewPortY.fY); 
	}
}

void plViewTransform::Read(hsStream* s)
{
	fFlags = s->ReadSwap32();
	fFlags &= ~kSetMask;

	fCameraToWorld.Read(s);
	fWorldToCamera.Read(s);

	fMin.Read(s);
	fMax.Read(s);

	fWidth = s->ReadSwap16();
	fHeight = s->ReadSwap16();

	fViewPortX.Read(s);
	fViewPortY.Read(s);

	fMapMin.Read(s);
	fMapMin.Read(s);
}

void plViewTransform::Write(hsStream* s)
{
	s->WriteSwap32(fFlags & ~kSetMask);

	fCameraToWorld.Write(s);
	fWorldToCamera.Write(s);

	fMin.Write(s);
	fMax.Write(s);

	s->WriteSwap16(fWidth);
	s->WriteSwap16(fHeight);

	fViewPortX.Write(s);
	fViewPortY.Write(s);

	fMapMin.Write(s);
	fMapMin.Write(s);
}
