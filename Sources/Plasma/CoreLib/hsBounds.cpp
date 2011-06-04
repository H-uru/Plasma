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

#include "hsFastMath.h"

#if defined(__MWERKS__) && !defined(HS_DEBUGGING)
#pragma optimization_level 2
#endif

const hsScalar hsBounds::kRealSmall = 1.0e-5f;

///////////////////////////////////////////////////////////////////////////////////////
//
// hsBounds
//
/////////////////////////////////////////////////////////////////////////////////////////

void hsBounds::Read(hsStream *s)
{
	fType =(hsBoundsType) s->ReadSwap32();
}

void hsBounds::Write(hsStream *s) 
{
	s->WriteSwap32((Int32)fType);
}

///////////////////////////////////////////////////////////////////////////////////////
//
// hsBounds3
//
/////////////////////////////////////////////////////////////////////////////////////////

#if 0 // MESH_GEN_DEFER
void hsBounds3::Draw(hsGView3* v, hsG3DDevice* d, 
				hsScalar r, hsScalar g, hsScalar b, hsScalar a, 
				hsBool spheric) 
{

	hsGViewClipState* clipState = v->SaveClipDisabled();
	if( hsGClip3::kClipCulled & v->ClipTestBounds(this) )
	{
		v->RestoreClipDisabled(clipState);
		return;
	}

	// Setup Material
	hsGMaterial *mat = TRACKED_NEW hsGMaterial;
	hsGLayer* lay = mat->MakeBaseLayer();
	lay->SetAmbientColor(r,g,b,a);
	lay->SetMiscFlags(hsGMatState::kMiscWireFrame | hsGMatState::kMiscTwoSided);
	lay->SetShadeFlags(hsGMatState::kShadeNoShade);
	mat->SetLayer(lay, 0);

	// Setup tMesh
	hsGTriMesh tMesh;
	if( spheric )
		MakeTriMeshSphere(&tMesh);
	else
		MakeTriMesh(&tMesh, hsTriangle3::kTwoSided);
	

	tMesh.SetMaterial(mat);
	hsRefCnt_SafeUnRef(mat);

	tMesh.Render(v,d);

	v->RestoreClipDisabled(clipState);
}
#endif // MESH_GEN_DEFER

void hsBounds3::Transform(const hsMatrix44 *mat) 
{
#if 0 // IDENT
	if( mat->fFlags & hsMatrix44::kIsIdent )
		return;
#endif // IDENT

	hsAssert(fType != kBoundsUninitialized, "Can't transform an unitialized bound");
	if(fType == kBoundsNormal)
	{
		hsPoint3 corners[8];
		this->GetCorners(corners);
		
		mat->MapPoints(8, corners);
		this->Reset(8,corners);
		fBounds3Flags &= ~kCenterValid;
	}
}

void hsBounds3::Reset(const hsPoint3 *p)
{
	fType = kBoundsNormal;
	fMins = fMaxs = *p;
	fBounds3Flags |= kCenterValid;
	fCenter = *p;
}

void hsBounds3::Reset(const hsBounds3 *b)
{
	if( kBoundsNormal == b->fType )
	{
		fType = kBoundsNormal;
		fMins = b->fMins;
		fMaxs = b->fMaxs;
		if( b->fBounds3Flags & kCenterValid )
		{
			fBounds3Flags |= kCenterValid;
			fCenter = b->fCenter;
		}
		else
			fBounds3Flags &= ~kCenterValid;
	}
	else
		fType = b->fType;
}

void hsBounds3::Reset(int n, const hsPoint3 *p)
{
	fType = kBoundsNormal;
	fMins = fMaxs = *p;
	for(int i = 1; i < n ; i++)
		this->Union(&p[i]);
	fBounds3Flags &= ~kCenterValid;
}

void hsBounds3::Union(const hsPoint3 *p)
{
	if(fType == kBoundsNormal) // Add this point if bounds is normal
	{
		for (int i = 0; i < 3; i++)
		{
			if ((*p)[i] > fMaxs[i])
				fMaxs[i] =(*p)[i];
			else if ((*p)[i] < fMins[i])
				fMins[i] =(*p)[i];
		}
		fBounds3Flags &= ~kCenterValid;
	}
	else
	{
		if(fType != kBoundsFull)	// Otherwise re-init unless bounds is full already
			this->Reset(p);
	}	
}

void hsBounds3::Union(const hsVector3 *v)
{
	if(fType == kBoundsNormal) // Add this point if bounds is normal
	{
		for (int i = 0; i < 3; i++)
		{
			if( (*v)[i] > 0 )
				fMaxs[i] += (*v)[i];
			else
				fMins[i] += (*v)[i];
		}
		fBounds3Flags &= ~kCenterValid;
	}
}


void hsBounds3::Union(const hsBounds3 *p)
{
	if(fType == kBoundsNormal && p->GetType() == kBoundsNormal) // Add this point if bounds is normal
	{
		for (int i = 0; i < 3; i++)
		{
			if (p->fMaxs[i] > fMaxs[i])
				fMaxs[i] = p->fMaxs[i];
			if (p->fMins[i] < fMins[i])
				fMins[i] = p->fMins[i];
		}
		fBounds3Flags &= ~kCenterValid;
	}
	else if(fType == kBoundsEmpty || fType == kBoundsUninitialized)
	{
		*this = *p;
	}
	// If fType is kBoundsFull don't do anything
}

void hsBounds3::MakeSymmetric(const hsPoint3* p)
{
	if( fType != kBoundsNormal )
		return;

	hsScalar delMax = 0;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar delUp;

		delUp = fMaxs[i] - (*p)[i];
		delMax = hsMaximum(delMax, delUp);
		delUp = (*p)[i] - fMins[i];
		delMax = hsMaximum(delMax, delUp);
	}
	const hsScalar sqrtTwo = 1.41421f;
	delMax *= sqrtTwo;
	hsAssert((delMax > -1.e6f)&&(delMax < 1.e6f), "MakeSymmetric going out to sea");
	fCenter = *p;
	fMaxs.Set(delMax, delMax, delMax);
	fMaxs += fCenter;
	fMins.Set(-delMax, -delMax, -delMax);
	fMins += fCenter;
	fBounds3Flags |= kCenterValid;
}

void hsBounds3::InscribeSphere()
{
	if( fType != kBoundsNormal )
		return;

	const hsScalar ooSix = hsScalarInvert(2.f * 3.f);
	hsScalar a = GetMaxDim() * ooSix;
	hsPoint3 p = GetCenter();
	p.fX += a;
	p.fY += a;
	p.fZ += a;
	fMaxs = p;
	a *= -2.f;
	p.fX += a;
	p.fY += a;
	p.fZ += a;
	fMins = p;

	// Center still valid, type still normal
}

// neg, pos, zero == disjoint, I contain other, overlap
Int32 hsBounds3::TestBound(const hsBounds3& other) const
{
	Int32 retVal = 1;
	int i;
	for( i = 0; i < 3; i++ )
	{
		if( GetMins()[i] > other.GetMaxs()[i] )
			return -1;
		if( GetMaxs()[i] < other.GetMins()[i] )
			return -1;

		if( GetMaxs()[i] < other.GetMaxs()[i] )
			retVal = 0;
		if( GetMins()[i] > other.GetMins()[i] )
			retVal = 0;
	}
	return retVal;
}

hsBool hsBounds3::IsInside(const hsPoint3* pos) const
{
	hsAssert(fType != kBoundsUninitialized, "Invalid bounds type for hsBounds3::IsInside() ");
	if(fType == kBoundsEmpty)
		return false;
	if(fType == kBoundsFull)
		return true;
	return !(pos->fX>fMaxs.fX || pos->fY>fMaxs.fY || pos->fZ>fMaxs.fZ ||
			pos->fX<fMins.fX || pos->fY<fMins.fY || pos->fZ<fMins.fZ);
}

#if 0 // MESH_GEN_DEFER
void hsBounds3::MakeTriMeshSphere(hsGTriMesh* tMesh, hsPoint3* cornersIn) const
{
	hsPoint3 center = (*GetMaxs() + *GetMins()) * 0.5f;
	hsScalar radius = GetMaxDim() * 0.5f;

	const int nLong = 9;
	const int nLati = 5;

	int nPts = nLong * nLati + 3;
	int nFaces = nLong * 2 + nLong * (nLati - 1) * 2; // == nLong * nLati * 2
	tMesh->AllocatePointers(nFaces /*faces*/, nPts /*pts*/, 0 /*uvs*/, 0 /*colors*/);
	tMesh->SetNumTriVertex(nPts);

	int iCenter = nPts - 3;
	int iNorthPole = nPts - 2;
	int iSouthPole = nPts - 1;
	hsPoint3 pt;
	pt = center;
	tMesh->SetPoint(iCenter, &pt);
	pt.fZ += radius;
	tMesh->SetPoint(iNorthPole, &pt);
	pt.fZ -= 2.f * radius;
	tMesh->SetPoint(iSouthPole, &pt);

	int i, j;
	for( i = 0; i < nLong; i++ )
	{
		for( j = 0; j < nLati; j++ )
		{
			hsScalar theta = (hsScalar(i) / nLong) * 2.f * hsScalarPI;
			hsScalar cosTheta = hsCosine(theta);
			hsScalar sinTheta = hsSine(theta);

			hsScalar phi = (hsScalar(j+1) / (nLati+1)) * hsScalarPI;
			hsScalar cosPhi = hsCosine(phi);
			hsScalar sinPhi = hsSine(phi);

			pt.fX = center.fX + radius * sinPhi * cosTheta;
			pt.fY = center.fY + radius * sinPhi * sinTheta;
			pt.fZ = center.fZ + radius * cosPhi;
			tMesh->SetPoint(j + i * nLati, &pt);
		}
	}

	hsTriangle3* tri;
	int nTris = 0;
	int iNext;
	for( i = 0; i < nLong; i++ )
	{
		if( (iNext = i + 1) >= nLong )
			iNext = 0;

		tri = tMesh->GetTriFromPool(nTris);
		tri->Zero();
		tri->fFlags |= hsTriangle3::kTwoSided;
		tMesh->SetTriangle(nTris++, tri);
		tri->SetQuickMeshVerts(i * nLati, iNext * nLati, iNorthPole);

		tri = tMesh->GetTriFromPool(i);
		tri->Zero();
		tri->fFlags |= hsTriangle3::kTwoSided;
		tMesh->SetTriangle(nTris++, tri);
		tri->SetQuickMeshVerts(nLati-1 + iNext * nLati, nLati-1 + i * nLati, iSouthPole);

		int jNext;
		for( j = 0; j < nLati-1; j++ )
		{
			jNext = j + 1;
			
			tri = tMesh->GetTriFromPool(nTris);
			tri->Zero();
			tri->fFlags |= hsTriangle3::kTwoSided;
			tMesh->SetTriangle(nTris++, tri);
			tri->SetQuickMeshVerts(j + i * nLati, j + iNext * nLati, jNext + i * nLati);
			
			tri = tMesh->GetTriFromPool(nTris);
			tri->Zero();
			tri->fFlags |= hsTriangle3::kTwoSided;
			tMesh->SetTriangle(nTris++, tri);
			tri->SetQuickMeshVerts(jNext + iNext * nLati, jNext + i * nLati, j + iNext * nLati);
		}
	}
}

//
// Allocate and create mesh from bounding box
//
void hsBounds3::MakeTriMesh(hsGTriMesh* tMesh, UInt32 triFlags, hsPoint3* cornersIn) const
{
	hsAssert(cornersIn || fType == kBoundsNormal, 
		"Invalid bounds type for hsBounds3::MakeTriMesh ");

	const int maxNew= 12;
	// Setup tMesh
	tMesh->AllocatePointers(maxNew /*faces*/, 8 /*pts*/, 0 /*uvs*/, 0 /*colors*/);
	tMesh->SetNumTriVertex(8);
	int i;
	hsPoint3 corners[8];
	// Set Points
	if( !cornersIn )
	{
		GetCorners(corners);
		cornersIn = corners;
	}
	for(i=0; i<8; i++)
	{
		tMesh->SetPoint(i, &cornersIn[i]);
	}
	tMesh->GetVertexPool()->SetCount(8);

	// Set faces
	hsTriangle3 *tri;
	int triNum=0;


	static int verts[maxNew * 3] = {		
	/* -Y */	6,2,3,
	/* -Y */	6,3,7,
	/* Y  */	5,1,0,
	/* Y  */	5,0,4,
	/* -X */	7,3,1,
	/* -X */	7,1,5,
	/* X  */	4,0,2,
	/* X  */	4,2,6,
	/* Z  */	3,0,1,
	/* Z  */	3,2,0,
	/* -Z */	7,4,6,
	/* -Z */	7,5,4
	};
	int v=0;
	for (;triNum < maxNew;triNum++)
	{
		tri = tMesh->GetTriFromPool(triNum);
		tri->Zero();
		tri->fFlags |= triFlags;
		tMesh->SetTriangle(triNum, tri);
		tri->SetQuickMeshVerts(verts[v + 0],verts[v + 1],verts[v + 2]);
		v += 3;
	}
	tMesh->SetTrianglePointers();
}
#endif // MESH_GEN_DEFER

void hsBounds3::TestPlane(const hsPlane3 *p, hsPoint2 &depth) const
{
	TestPlane(p->fN, depth);
}
void hsBounds3::TestPlane(const hsVector3 &n, hsPoint2 &depth) const
{
	hsAssert(fType == kBoundsNormal, "TestPlane only valid for kBoundsNormal filled bounds");

	hsScalar dmax = fMins.InnerProduct(n);
	hsScalar dmin = dmax;

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar dd;
		dd = fMaxs[i] - fMins[i];
		dd *= n[i];

		if( dd < 0 )
			dmin += dd;
		else
			dmax += dd;
	}

	depth.fX = dmin;
	depth.fY = dmax;
}

hsScalar hsBounds3::ClosestPointToLine(const hsPoint3 *p, const hsPoint3 *v0, const hsPoint3 *v1, hsPoint3 *out)
{
	hsVector3 del(v1, v0);
	hsScalar magSq = del.MagnitudeSquared();
	hsScalar t = 0.f;
	if( magSq < hsBounds::kRealSmall )
	{
		*out = *v0;
	}
	else
	{
		t = del.InnerProduct(hsVector3(p, v0)) * hsScalarInvert(magSq);
		if( t >= hsScalar1 )
			*out = *v1;
		else if( t <= 0 )
			*out = *v0;
		else
			*out = *v0 + del * t;
	}
	return t;
}

hsScalar hsBounds3::ClosestPointToInfiniteLine(const hsPoint3* p, const hsVector3* v, hsPoint3* out)
{
	hsScalar magSq = v->MagnitudeSquared();
	hsScalar t = 0.f;
	hsPoint3 origin(0,0,0);
	if( magSq < hsBounds::kRealSmall )
	{
		*out = origin;
	}
	else
	{
		t = v->InnerProduct(hsVector3(*p)) * hsScalarInvert(magSq);
		*out = hsPoint3(*v * t);
	}
	return t;
}

hsBool hsBounds3::ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const
{
	// Look for axis intervals p is within
	int nSect = 0;
	int i;
	for( i = 0; i < 3; i++ )
	{
		if( p[i] < fMins[i] )
		{
			inner[i] = fMins[i];
			outer[i] = fMaxs[i];
		}
		else if( p[i] > fMaxs[i] )
		{
			inner[i] = fMaxs[i];
			outer[i] = fMins[i];
		}
		else
		{
			inner[i] = p[i];
			outer[i] = (p[i] - fMins[i] > fMaxs[i] - p[i]) ? fMins[i] : fMaxs[i];
			nSect++;
		}
	}

	return nSect == 3;
}

void hsBounds3::Read(hsStream *stream)
{
	hsBounds::Read(stream);
	fMins.Read(stream);
	fMaxs.Read(stream);
	fBounds3Flags = 0;
}

void hsBounds3::Write(hsStream *stream) 
{
	hsBounds::Write(stream);
	fMins.Write(stream);
	fMaxs.Write(stream);
}

//////////////////////////////////
//////////////////////////////////////////////////
// Plane Bounds util class
//////////////////////////////////////////////////

hsPoint3  hsBoundsOriented::GetCenter() const
{ 
	hsAssert(fCenterValid==true, "Unset center for hsBoundsOriented::GetCenter()"); 
	return fCenter; 
}

void hsBoundsOriented::TestPlane(const hsVector3 &n, hsPoint2 &depth) const
{
	hsAssert(false, "TestPlane not a valid operation for hsBounsOriented");
}
//
// Return true if inside all the planes
//
hsBool hsBoundsOriented::IsInside(const hsPoint3* pos) const
{
	hsAssert(fType == kBoundsNormal, "Invalid bounds type for hsBounds3::IsInside() ");
	if(fType == kBoundsEmpty)
		return false;
	if(fType == kBoundsFull)
		return true;
	int i;
	for( i = 0; i < fNumPlanes; i++ )
	{
		hsScalar dis = fPlanes[i].fN.InnerProduct(pos);
		dis += fPlanes[i].fD;
		if( dis > 0.f )
			return false;
	}

	return true;
}

void hsBoundsOriented::SetNumberPlanes(UInt32 n)
{
	delete [] fPlanes;
	fPlanes = TRACKED_NEW hsPlane3[fNumPlanes = n];
}

void hsBoundsOriented::SetPlane(UInt32 i, hsPlane3 *pln)
{
	fType = kBoundsNormal;
	if( i >= fNumPlanes )
	{
		hsPlane3 *newPlanes = TRACKED_NEW hsPlane3[i+1];
		if( fPlanes )
		{
			int k;
			for( k = 0; k < fNumPlanes; k++ )
				*newPlanes++ = *fPlanes++;
			delete [] fPlanes;
		}
		fPlanes = newPlanes;
		fNumPlanes = i+1;
	}
	fPlanes[i] = *pln;
}

//
// Make mesh from bounds3. Make boundsOriented from mesh tris.
//
void hsBoundsOriented::Reset(const hsBounds3* bounds)
{
#if 0 // MESH_GEN_DEFER
	hsGTriMesh tMesh;
	bounds->MakeTriMesh(&tMesh, 0 /* triFlags */);
	Reset(&tMesh);
#endif // MESH_GEN_DEFER
}

#if 0
//
// Make mesh from bounds3. Make boundsOriented from mesh tris.
//
void	hsBoundsOriented::Union(const hsBounds3 *b)
{
#if 0 // MESH_GEN_DEFER
	hsGTriMesh tMesh;
	bounds->MakeTriMesh(&tMesh);
	int i;
	hsTriangle3 tri;
	for (i=0; i<tMesh.GetNumTriangles(); i++)
	{
		tMesh.GetTriangle(i, &tri);
		Union(&tri);
	}
#endif // MESH_GEN_DEFER
}
#endif

//
//
//
void hsBoundsOriented::Reset(hsGTriMesh *tMesh)
{
#if 0 // MESH_GEN_DEFER
	const float OBJCVT_ABOUT_ZERO = 1.0e-4f;
	const float OBJCVT_ABOUT_ONE = 1.0f - OBJCVT_ABOUT_ZERO;

	hsPlane3 *planes = TRACKED_NEW hsPlane3[tMesh->GetNumTriangles()];
	UInt32 nPlanes = 0;

	int i;
	for( i = 0; i < tMesh->GetNumTriangles(); i++ )
	{
		hsTriangle3 *tri;
		tri = tMesh->GetTriangle(i);
		hsPlane3 pln;
		tri->ComputePlane(&pln);
		
		hsScalar norm = hsFastMath::InvSqrRoot(pln.fN.MagnitudeSquared());
		pln.fN *= norm;

		int j;
		for( j = 0; j < nPlanes; j++ )
		{
			if( (pln.fN.InnerProduct(planes[j].fN)> OBJCVT_ABOUT_ONE)
				&&((pln.fD/planes[j].fD) >= 1.0-OBJCVT_ABOUT_ZERO)
				&&((pln.fD/planes[j].fD) <= 1.0+OBJCVT_ABOUT_ZERO) )
				break;
		}
		if( j == nPlanes )
			planes[nPlanes++] = pln;
	}

	SetNumberPlanes(nPlanes);
	for( i = 0; i < nPlanes; i++ )
		SetPlane(i, planes+i);

	delete [] planes;

	// Compute center
	hsPoint3 centroid(0,0,0);
	for(i=0; i<tMesh->GetNumPoints(); i++)
	{
		centroid = centroid + *tMesh->GetPoint(i);
	}
	centroid = centroid / (hsScalar)tMesh->GetNumPoints();
	SetCenter(&centroid);
#endif // MESH_GEN_DEFER
}


void hsBoundsOriented::Write(hsStream *stream)
{
	hsBounds::Write(stream);
	fCenter.Write(stream);
	stream->WriteSwap32(fCenterValid);
	stream->WriteSwap32(fNumPlanes);
	int i;
	for( i = 0; i < fNumPlanes; i++ )
	{
		fPlanes[i].Write(stream);
	}
}

void hsBoundsOriented::Read(hsStream *stream)
{
	hsBounds::Read(stream);
	fCenter.Read(stream);
	fCenterValid = (hsBool)stream->ReadSwap32();
	fNumPlanes = stream->ReadSwap32();
	if (fPlanes)
		delete [] fPlanes;
	fPlanes = TRACKED_NEW hsPlane3[fNumPlanes];
	int i;
	for( i = 0; i < fNumPlanes; i++ )
	{
		fPlanes[i].Read(stream);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

hsBounds3Ext::hsBounds3Ext(const hsBounds3 &b)
{
	Reset(&b);
}
hsBounds3Ext &hsBounds3Ext::operator=(const hsBounds3 &b)
{
	Reset(&b);
	return *this;
}

void hsBounds3Ext::IMakeMinsMaxs()
{
	hsAssert(!(fExtFlags & kAxisAligned), "Axis aligned box defined by min and max");
	fMins = fMaxs = fCorner;
	int i;
	for( i = 0; i < 3; i++ )
	{
		if(!IAxisIsZero(i) )
		{
			int j;
			for( j = 0; j < 3; j++ )
			{
				if( fAxes[i][j] < 0 )
					fMins[j] += fAxes[i][j];
				else
					fMaxs[j] += fAxes[i][j];
			}
		}
	}
}

void hsBounds3Ext::IMakeDists() const
{
	hsAssert(!(fExtFlags & kAxisAligned), "Dists only useful for transformed BB");

	int i;
	for( i = 0; i < 3; i++ )
	{
		fDists[i].fX = fCorner.InnerProduct(fAxes[i]);
		if( !IAxisIsZero(i) )
		{
			fDists[i].fY = fDists[i].fX + fAxes[i].InnerProduct(fAxes[i]);
			if( fDists[i].fX > fDists[i].fY )
			{
				hsScalar t = fDists[i].fX;
				fDists[i].fX = fDists[i].fY;
				fDists[i].fY = t;
			}
		}
		else
			fDists[i].fY = fDists[i].fX;
	}
	fExtFlags |= kDistsSet;
}

void hsBounds3Ext::IMakeSphere() const
{
	if(!(fBounds3Flags & kCenterValid) )
		ICalcCenter();
	if( fExtFlags & kAxisAligned )
	{
		if( fBounds3Flags & kIsSphere )
		{
			fRadius = fMaxs[0] - fMins[0];
			int i;
			for( i = 1; i < 3; i++ )
			{
				hsScalar dist = fMaxs[i] - fMins[i];
				if( dist < fRadius )
					fRadius = dist;
			}
			fRadius *= 0.5f;
		}
		else
		{
			fRadius = hsSquareRoot(hsVector3(&fMaxs, &fCenter).MagnitudeSquared());
		}
	}
	else
	{
		if( fBounds3Flags & kIsSphere )
		{
			hsScalar minMagSq = fAxes[0].MagnitudeSquared();
			hsScalar magSq = fAxes[1].MagnitudeSquared();
			if( magSq < minMagSq )
				magSq = minMagSq;
			magSq = fAxes[2].MagnitudeSquared();
			if( magSq < minMagSq )
				magSq = minMagSq;
			fRadius = hsSquareRoot(magSq);
		}
		else
		{
			hsVector3 accum;
			accum.Set(0,0,0);
			int i;
			for( i = 0; i < 3; i++ )
			{
				if( !IAxisIsZero(i) )
					accum += fAxes[i];
			}
			fRadius = hsSquareRoot((accum * 0.5f).MagnitudeSquared());
		}
	}
	fExtFlags |= kSphereSet;
}

void hsBounds3Ext::Reset(const hsBounds3 *b)
{
	fExtFlags = kAxisAligned;
	hsBounds3::Reset(b);
}

void hsBounds3Ext::Reset(const hsPoint3 *p)
{
	fExtFlags = kAxisAligned | kSphereSet;
	hsBounds3::Reset(p);
	fRadius = 0;
}
 
void hsBounds3Ext::Reset(const hsBounds3Ext *b)
{
	hsBounds3::Reset(b);
	fExtFlags	= b->fExtFlags;
	if (!(fExtFlags & kAxisAligned))
	{
		fCorner		= b->fCorner;
		fAxes[0]	= b->fAxes[0];
		fAxes[1]	= b->fAxes[1];
		fAxes[2]	= b->fAxes[2];
	}
	if (fExtFlags & kDistsSet)
	{
		fDists[0]	= b->fDists[0];
		fDists[1]	= b->fDists[1];
		fDists[2]	= b->fDists[2];
	}
	if (fExtFlags & kSphereSet)
		fRadius		= b->fRadius;
}


void hsBounds3Ext::GetCorners(hsPoint3 *b) const
{
	if( fExtFlags & kAxisAligned )
	{
		hsBounds3::GetCorners(b);
	}
	else
	{
		int i;
		for( i = 0; i < 8; i++ )
		{
			b[i] = fCorner;
			if( !(i & 0x1) && !(fExtFlags & kAxisZeroZero) )b[i] += fAxes[0];
			if( !(i & 0x2) && !(fExtFlags & kAxisOneZero) )b[i] += fAxes[1];
			if( !(i & 0x4) && !(fExtFlags & kAxisTwoZero) )b[i] += fAxes[2];
		}
	}
}

void hsBounds3Ext::GetAxes(hsVector3 *fAxis0, hsVector3 *fAxis1, hsVector3 *fAxis2) const
{
	if( !(fExtFlags & kAxisAligned) )
	{
		*fAxis0 = fAxes[0];
		*fAxis1 = fAxes[1];
		*fAxis2 = fAxes[2];
	}
	else
	{
		fAxis0->Set(fMaxs.fX - fMins.fX, 0, 0);
		fAxis1->Set(0, fMaxs.fY - fMins.fY, 0);
		fAxis2->Set(0, 0, fMaxs.fZ - fMins.fZ);
	}
}

void hsBounds3Ext::Reset(int n, const hsPoint3 *p)
{
	fExtFlags = kAxisAligned;
	hsBounds3::Reset(n, p);
}

// mf horse - could union in a point preserving axes...
void hsBounds3Ext::Union(const hsPoint3 *p)
{
	fExtFlags = kAxisAligned;
	hsBounds3::Union(p);
}
void hsBounds3Ext::Union(const hsVector3 *v)
{
#if 0 // smarter union
	fExtFlags = kAxisAligned;
	hsBounds3::Union(v);
#else // smarter union
	if( fExtFlags & kAxisAligned )
	{
		hsBounds3::Union(v);
	}
	else
	{
		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar dot = fAxes[i].InnerProduct(v);
			dot /= fAxes[i].MagnitudeSquared();
			if( dot > 0 )
			{
				fAxes[i] += dot * fAxes[i];
				fExtFlags &= ~(1 << (20+i)); // axis not zero no more
			}
			else if( dot < 0 )
			{
				hsVector3 del = dot * fAxes[i];
				fCorner += del;
				del = -del;
				fAxes[i] += del;
				fExtFlags &= ~(1 << (20+i)); // axis not zero no more
			}
		}
		fExtFlags &= ~(kSphereSet | kDistsSet);
		fBounds3Flags &= ~kCenterValid;
	}
#endif // smarter union
}

void hsBounds3Ext::Union(const hsBounds3 *b)
{
	fExtFlags = kAxisAligned;
	hsBounds3::Union(b);
}

void hsBounds3Ext::MakeSymmetric(const hsPoint3* p)
{
	if( fType != kBoundsNormal )
		return;

	if( fExtFlags & kAxisAligned )
	{
		fExtFlags = kAxisAligned;
		hsBounds3::MakeSymmetric(p);
		return;
	}

	// Can do this preserving axes, but may not be worth it.
	fExtFlags = kAxisAligned;
	hsBounds3::MakeSymmetric(p);
}

void hsBounds3Ext::InscribeSphere()
{
	fBounds3Flags |= kIsSphere;
	fExtFlags |= kAxisAligned;
	IMakeSphere();
	return;
#if 0
	if( fType != kBoundsNormal )
		return;

	if( fExtFlags & kAxisAligned )
	{
		hsBounds3::InscribeSphere();
		return;
	}

	const hsScalar oneThird = hsScalarInvert(3.f);
//	hsScalar a = GetMaxDim() * hsScalarInvert(6.f);
	hsScalar a = GetRadius() * oneThird;
	hsPoint3 p = GetCenter();
	p.fX += a;
	p.fY += a;
	p.fZ += a;
	fMaxs = p;
	a *= -2.f;
	p.fX += a;
	p.fY += a;
	p.fZ += a;
	fMins = p;

	// Center still valid, type still normal
	fExtFlags = kAxisAligned;
#endif
}

void hsBounds3Ext::Transform(const hsMatrix44 *m)
{
	if( fType != kBoundsNormal )
		return;

	if( fExtFlags & kAxisAligned )
	{
		fExtFlags = 0;

		fCorner = *m * fMins;
		hsVector3 v;
		hsScalar span;
		span = fMaxs.fX - fMins.fX;
		if( span < kRealSmall )
		{
			fExtFlags |= kAxisZeroZero;
			span = hsScalar1;
		}
		v.Set(span, 0, 0);
		fAxes[0] = *m * v;
		span = fMaxs.fY - fMins.fY;
		if( span < kRealSmall )
		{
			fExtFlags |= kAxisOneZero;
			span = hsScalar1;
		}
		v.Set(0, span, 0);
		fAxes[1] = *m * v;
		span = fMaxs.fZ - fMins.fZ;
		if( span < kRealSmall )
		{
			fExtFlags |= kAxisTwoZero;
			span = hsScalar1;
		}
		v.Set(0, 0, span);
		fAxes[2] = *m * v;

	}
	else
	{
#if 0 // IDENT
		if( m->fFlags & hsMatrix44::kIsIdent )
			return;
#endif // IDENT

		fCorner = *m * fCorner;
		fAxes[0] = *m * fAxes[0];
		fAxes[1] = *m * fAxes[1];
		fAxes[2] = *m * fAxes[2];

		fExtFlags &= kAxisZeroZero|kAxisOneZero|kAxisTwoZero;
	}
	IMakeMinsMaxs();
	fBounds3Flags &= ~kCenterValid;
}

void hsBounds3Ext::Translate(const hsVector3 &v)
{
	if( fType != kBoundsNormal )
		return;

	fMins += v;
	fMaxs += v;
	if( fBounds3Flags & kCenterValid )
		fCenter += v;
	if( !(fExtFlags & kAxisAligned) )
	{
		fCorner += v;
		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar d;
			d = fAxes[i].InnerProduct(v);
			fDists[i].fX += d;
			fDists[i].fY += d;
		}
	}
}

hsBool hsBounds3Ext::IsInside(const hsPoint3 *p) const
{
	if( fExtFlags & kAxisAligned )
		return hsBounds3::IsInside(p);

	if( !(fExtFlags & kDistsSet) )
		IMakeDists();

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar diss = p->InnerProduct(fAxes[i]);
		if( (diss < fDists[i].fX)
			||(diss > fDists[i].fY) )
			return false;
	}
	return true;

}

// neg, pos, zero == disjoint, I contain other, overlap
Int32 hsBounds3Ext::TestBound(const hsBounds3Ext& other) const
{
	if( fExtFlags & kAxisAligned )
		return hsBounds3::TestBound(other);

	if( !(fExtFlags & kDistsSet) )
		IMakeDists();

	Int32 retVal = 1;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsPoint2 depth;
		other.TestPlane(fAxes[i], depth);

		if( fDists[i].fX > depth.fY )
			return -1;
		if( fDists[i].fY < depth.fX )
			return -1;

		if( fDists[i].fY < depth.fY )
			retVal = 0;
		if( fDists[i].fX > depth.fX )
			retVal = 0;
	}
	return retVal;
}


void hsBounds3Ext::TestPlane(const hsVector3 &n, hsPoint2 &depth) const
{
	hsAssert(fType == kBoundsNormal, "TestPlane only valid for kBoundsNormal filled bounds");
	if( fExtFlags & kAxisAligned )
	{
		hsBounds3::TestPlane(n, depth);
	}
	else
	{
		hsScalar dmax = fCorner.InnerProduct(n);
		hsScalar dmin = dmax;

		int i;
		for( i = 0; i < 3; i++ )
		{
			if( !IAxisIsZero(i) )
			{
				hsScalar d;
				d = fAxes[i].InnerProduct(n);
				if( d < 0 )
					dmin += d;
				else
					dmax += d;
			}
		}

		depth.fX = dmin;
		depth.fY = dmax;
	}
}

void hsBounds3Ext::TestPlane(const hsPlane3 *p, const hsVector3 &myVel, hsPoint2 &depth) const
{
	TestPlane(p->fN, myVel, depth);
}
void hsBounds3Ext::TestPlane(const hsVector3 &n, const hsVector3 &myVel, hsPoint2 &depth) const
{
	if( fExtFlags & kAxisAligned )
	{
		hsScalar dmax = fMins.InnerProduct(n);
		hsScalar dmin = dmax;
		hsScalar dvel = myVel.InnerProduct(n);
		if( dvel < 0 )
			dmin += dvel;
		else
			dmax += dvel;

		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar dd;
			dd = fMaxs[i] - fMins[i];
			dd *= n[i];

			if( dd < 0 )
				dmin += dd;
			else
				dmax += dd;
		}

		depth.fX = dmin;
		depth.fY = dmax;
	}
	else
	{
		hsScalar dmax = fCorner.InnerProduct(n);
		hsScalar dmin = dmax;
		hsScalar dvel = myVel.InnerProduct(n);
		if( dvel < 0 )
			dmin += dvel;
		else
			dmax += dvel;

		int i;
		for( i = 0; i < 3; i++ )
		{
			if( !IAxisIsZero(i) )
			{
				hsScalar d;
				d = fAxes[i].InnerProduct(n);
				if( d < 0 )
					dmin += d;
				else
					dmax += d;
			}
		}

		depth.fX = dmin;
		depth.fY = dmax;
	}
}

Int32 hsBounds3Ext::TestPoints(int n, const hsPoint3 *pList, const hsVector3 &ptVel) const
{
	if( fExtFlags & kAxisAligned )
	{
		Int32 retVal = -1;
		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar effMax = fMaxs[i];
			hsScalar effMin = fMins[i];
			if( ptVel[i] < 0 )
				effMax -= ptVel[i];
			else
				effMin -= ptVel[i];

			int j;
			const UInt32 low = 0x1, hi = 0x2;
			UInt32 mask = low | hi;
			for( j = 0; j < n; j++ )
			{
				if( pList[j][i] > effMin )
					mask &= ~low;
				if( pList[j][i] < effMax )
					mask &= ~hi;
				if( mask )
					retVal = 0;
			}
			if( mask )
				return 1;
		}
		return retVal;
	}
	else // non-axis aligned case
	{
		Int32 retVal = -1; // all inside
		if( !(fExtFlags & kDistsSet) )
			IMakeDists();
		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar diff = fAxes[i].InnerProduct(ptVel);
			hsBool someLow = false;
			hsBool someHi = false;
			hsBool someIn = false;
			int j;
			for( j = 0; j < n; j++ )
			{
				hsScalar d = fAxes[i].InnerProduct(pList[j]);
				hsScalar ddiff = d + diff;
				if( d < fDists[i].fX )
					someLow = true;
				else if( d > fDists[i].fY )
					someHi = true;
				else
					someIn = true;

				if( ddiff < fDists[i].fX )
					someLow = true;
				else if( ddiff > fDists[i].fY )
					someHi = true;
				else
					someIn = true;

				if( someIn &&(someHi || someLow) )
					break;
			}
			if( someHi && !(someLow || someIn) )
				return 1;
			if( someLow && !(someHi || someIn) )
				return 1;
			if( someHi || someLow )
				retVal = 0;
		}
		return retVal;
	}
}

Int32 hsBounds3Ext::TestPoints(int n, const hsPoint3 *pList) const
{
	hsBool someIn = false;
	hsBool someOut = false;
	int i;
	for( i = 0; i < n; i++ )
	{
		if( IsInside(pList+i) )
			someIn = true;
		else
			someOut = true;
		if( someIn && someOut )
			return 0;
	}
	if( someIn )
		return -1;
	return 1;
}

hsBool hsBounds3Ext::ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const
{
	if( fExtFlags & kAxisAligned )
		return hsBounds3::ClosestPoint(p, inner, outer);

	if( !(fExtFlags & kDistsSet) )
		IMakeDists();

	int nSect = 0;
	inner = outer = fCorner;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar dist = fAxes[i].InnerProduct(p);
		if( dist < fDists[i].fX )
		{
			outer += fAxes[i];
		}
		else if( dist > fDists[i].fY )
		{
			inner += fAxes[i];
		}
		else
		{
			hsScalar t = (dist - fDists[i].fX) / (fDists[i].fY - fDists[i].fX);
			inner += t * fAxes[i];
			if( t > 0.5f )
				outer += fAxes[i];

			nSect++;
		}
	}
	return nSect == 3;
}

hsBool hsBounds3Ext::ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel) const
{
	if( fExtFlags & kAxisAligned )
	{
		if( other.fExtFlags & kAxisAligned )
			return ISectABB(other, myVel);

		return other.ISectBB(*this, -myVel);
	}
	hsAssert(!(fExtFlags & kAxisAligned), "Other can be axis-aligned, but not me!");
	hsPoint2 depth;

	if( !(fExtFlags & kDistsSet) )
		IMakeDists();
	if( !(other.fExtFlags & (kDistsSet|kAxisAligned)) )
		other.IMakeDists();

	int i;
	for( i = 0; i < 3; i++ )
	{
		other.TestPlane(fAxes[i], -myVel, depth);
		if( (depth.fX > fDists[i].fY)
			||(depth.fY < fDists[i].fX) )
			return false;

		if( other.fExtFlags & kAxisAligned )
		{
			hsScalar myMin = fMins[i];
			hsScalar myMax = fMaxs[i];
			if( myVel[i] < 0 )
				myMin += myVel[i];
			else
				myMax += myVel[i];
			if( (other.fMins[i] > myMax)
				||(other.fMaxs[i] < myMin) )
				return false;
		}
		else
		{
			TestPlane(other.fAxes[i], myVel, depth);
			if( (depth.fX > other.fDists[i].fY)
				||(depth.fY < other.fDists[i].fX) )
				return false;
		}

		// still leaves the 3 axes of origAxis.Cross(myVel)
		hsVector3 ax = fAxes[i] % myVel;
		hsScalar dmax = fCorner.InnerProduct(ax);
		hsScalar dmin = dmax;
		int j = i+1;
		if( 3 == j )j = 0;
		hsScalar d;
		d = fAxes[j].InnerProduct(ax);
		if( d < 0 )
			dmin += d;
		else
			dmax += d;
		j = ( j == 2 ? 0 : j+1 );
		d = fAxes[j].InnerProduct(ax);
		if( d < 0 )
			dmin += d;
		else
			dmax += d;
		other.TestPlane(ax, depth);
		if( (depth.fX > dmax)
			||(depth.fY < dmin) )
			return false;
	}


	return true;
}


static hsBool ISectInterval(const hsPoint2& other, const hsPoint2& mine)
{
	if( other.fY - mine.fX <= 0 )
		return false;
	if( mine.fY - other.fX <= 0 )
		return false;

	return true;
}

static hsBool ITestDepth(const hsPoint2& other, const hsPoint2& mine, 
						  const hsVector3& inAx, 
						  hsVector3 &outAx, hsScalar& depth)
{
	depth = 0;
	hsScalar d0, d1;
	d0 = other.fY - mine.fX;
	if( d0 <= 0 )
		return false;
	d1 = mine.fY - other.fX;
	if( d1 <= 0 )
		return false;

	// if one interval is proper subset of other, skip
	if( (mine.fX < other.fX)^(mine.fY < other.fY) )
	{
		depth = 0;
		return true;
	}
	if( d0 < d1 )
	{
		outAx = inAx;
		depth = d0;
		return true;
	}

	outAx = -inAx;
	depth = d1;
	return true;
}

Int32 hsBounds3Ext::IClosestISect(const hsBounds3Ext& other, const hsVector3& myVel,
							  hsScalar* tClose, hsScalar* tImpact) const
{
	// Should assert both have their spheres set.

	hsVector3 meToOt(&other.GetCenter(), &GetCenter());

	// cTerm = (myCenter - otCenter)^2 - (myRad + otRad)^2
	hsScalar cTerm;

	cTerm = GetRadius() + other.GetRadius();
	cTerm *= -cTerm;

	hsScalar meToOtLen = meToOt.MagnitudeSquared();
	cTerm += meToOtLen;
	if( cTerm <= 0 )
	{
		*tClose = *tImpact = 0;
		return -1; // started off in contact
	}

	hsScalar ooATerm = myVel.InnerProduct(myVel);
	if( ooATerm < hsBounds::kRealSmall )
	{
		*tClose = *tImpact = 0;
		return 0;
	}
	ooATerm = hsScalarInvert(ooATerm);

	hsScalar bTerm = myVel.InnerProduct(meToOt);
	bTerm *= ooATerm;
	hsScalar bSqTerm = bTerm * bTerm;
	// bTerm is t for closest point to line

	hsScalar det = bSqTerm - ooATerm * cTerm;
	if( det < 0 )
	{
		*tClose = *tImpact = bTerm;
		return 0;
	}

	det = hsSquareRoot(det);
	*tClose = bTerm;
	*tImpact = bTerm - det;

	return 1;
}

void hsBounds3Ext::Unalign()
{
	fExtFlags = 0;

	fCorner = fMins;
	hsVector3 v;
	hsScalar span;
	span = fMaxs.fX - fMins.fX;
	if( span < kRealSmall )
	{
		fExtFlags |= kAxisZeroZero;
		span = hsScalar1;
	}
	fAxes[0].Set(span, 0, 0);
	span = fMaxs.fY - fMins.fY;
	if( span < kRealSmall )
	{
		fExtFlags |= kAxisOneZero;
		span = hsScalar1;
	}
	fAxes[1].Set(0, span, 0);
	span = fMaxs.fZ - fMins.fZ;
	if( span < kRealSmall )
	{
		fExtFlags |= kAxisTwoZero;
		span = hsScalar1;
	}
	fAxes[2].Set(0, 0, span);
}

hsBool hsBounds3Ext::ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const
{
	if( fExtFlags & kAxisAligned )
	{
		hsBounds3Ext meUnalign(*this);
		meUnalign.Unalign();
		return meUnalign.ISectBB(other, myVel, hit);
	}
	hsAssert(!(fExtFlags & kAxisAligned), "Other can be axis-aligned, but not me!");
	hsPoint2 depth;

	if( !(fExtFlags & kDistsSet) )
		IMakeDists();
	if( !(other.fExtFlags & (kDistsSet|kAxisAligned)) )
		other.IMakeDists();

	const hsScalar kRealBig = 1.e30f;
	hsScalar tstDepths[9];
	hsVector3 tstAxes[9];
	hsScalar totDepth = 0;
	int nDeep = 0;
	int i;
	for( i = 0; i < 3; i++ )
	{
		const hsScalar kFavorConstant = 0.01f; // smaller is favored

		other.TestPlane(fAxes[i], -myVel, depth);

		if( !ITestDepth(depth, fDists[i], fAxes[i], tstAxes[i], tstDepths[i]) )
			return false;

		other.TestPlane(fAxes[i], depth);
		if( !ISectInterval(depth, fDists[i]) )
			tstDepths[i] *= kFavorConstant;

		if( tstDepths[i] > 0 )
		{
			totDepth += tstDepths[i];
			nDeep++;
		}

		if( other.fExtFlags & kAxisAligned )
		{
			hsPoint2 mine;
			mine.fX = fMins[i];
			mine.fY = fMaxs[i];
			if( myVel[i] > 0 )mine.fY += myVel[i];
			else mine.fX += myVel[i];
			depth.fX = other.fMins[i];
			depth.fY = other.fMaxs[i];

			hsVector3 ax;
			ax.Set( 0 == i ? hsScalar1 : 0,
					1 == i ? hsScalar1 : 0,
					2 == i ? hsScalar1 : 0);

			if( !ITestDepth(depth, mine, ax, tstAxes[i+3], tstDepths[i+3]) )
				return false;

			mine.fX = fMins[i];
			mine.fY = fMaxs[i];
			if( !ISectInterval(depth, mine) )
				tstDepths[i+3] *= kFavorConstant;

			if( tstDepths[i+3] )
			{
				totDepth += tstDepths[i+3];
				nDeep++;
			}
		}
		else
		{
			TestPlane(other.fAxes[i], myVel, depth);

			if( !ITestDepth(other.fDists[i], depth, other.fAxes[i], tstAxes[i+3], tstDepths[i+3]) )
				return false;

			TestPlane(other.fAxes[i], depth);

			if( !ISectInterval(other.fDists[i], depth) )
				tstDepths[i+3] *= kFavorConstant;

			if( tstDepths[i+3] )
			{
				totDepth += tstDepths[i+3];
				nDeep++;
			}
	}

#if 0
		// still leaves the 3 axes of origAxis.Cross(myVel)
		hsVector3 ax = fAxes[i] % myVel;
		if( ax.MagnitudeSquared() > kRealSmall )
		{
			hsPoint2 myDepth;
			myDepth.fX = myDepth.fY = fCorner.InnerProduct(ax);
			hsScalar d;
			int j = i == 2 ? 0 : i+1;
			if( !IAxisIsZero(j) )
			{
				d = fAxes[j].InnerProduct(ax);
				if( d < 0 )
					myDepth.fX += d;
				else
					myDepth.fY += d;
			}
			j = ( j == 2 ? 0 : j+1 );
			if( !IAxisIsZero(j) )
			{
				d = fAxes[j].InnerProduct(ax);
				if( d < 0 )
					myDepth.fX += d;
				else
					myDepth.fY += d;
			}
			other.TestPlane(ax, depth);

			if( !ITestDepth(depth, myDepth, ax, tstAxes[i+6], tstDepths[i+6]) )
				return false;
			totDepth += tstDepths[i+6];
		}
		else
			tstDepths[i+6] = 0;
#endif;
	}

	hsVector3 norm;
	if( totDepth <= 0 )
	{
		hsScalar t, tIgnore;
		IClosestISect(other, myVel, &tIgnore, &t);
		if( t < 0 )
			t = 0;
		else if( t > 1.f )
			t = 1.f;
		hsPoint3 hitPt = GetCenter() + myVel * t;
		norm.Set(&hitPt, &other.GetCenter());
	}
	else
	{
		// now do a weighted average of the axes
		hsAssert(totDepth > 0, "nobody home");
		norm.Set(0,0,0);
		for( i =0; i < 6; i++ )
		{
			if( tstDepths[i] > 0 )
				norm += tstAxes[i] / tstDepths[i];
//				norm += tstAxes[i] * (1.f - tstDepths[i] / totDepth);
		}
	}
	hsPoint2 otherDepth;
	norm.Normalize();
	other.TestPlane(norm, otherDepth);
	TestPlane(norm, myVel, depth);

	hit->Set(this, &other, norm, otherDepth.fY - depth.fX);

	// mf horse hack test
	if( hit->fDepth < 0 )
		return false;
	hsAssert(hit->fDepth >= 0, "Negative Depth");

	return true;
}

hsBool hsBounds3Ext::ISectABB(const hsBounds3Ext &other, const hsVector3 &myVel) const
{
	hsPoint3 effMaxs = fMaxs;
	hsPoint3 effMins = fMins;

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar effMax = fMaxs[i];
		hsScalar effMin = fMins[i];
		if( myVel[i] > 0 )
			effMax += myVel[i];
		else
			effMin += myVel[i];

		if( (effMax < other.fMins[i])
			||(effMin > other.fMaxs[i]) )
			return false;
	}
	return true;
}

hsBool hsBounds3Ext::ISectBS(const hsBounds3Ext &other, const hsVector3 &myVel) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();
	if( !(other.fExtFlags & kSphereSet) )
		other.IMakeSphere();

	hsPoint3 closestPt = GetCenter();
	// we should know whether we have a useful velocity or not...
	// having the speed cached away would get rid of several
	// such uglies...
	if( myVel.MagnitudeSquared() > 0 )
	{
		hsScalar parm = hsVector3(&other.GetCenter(), &fCenter).InnerProduct(myVel) 
			/ myVel.InnerProduct(myVel);
		if( parm > 0 )
		{
			if( parm > hsScalar1 )
				parm = hsScalar1;
			closestPt += myVel * parm;
		}
	}

	hsScalar combRad = fRadius + other.fRadius;

	return hsVector3(&closestPt, &other.GetCenter()).MagnitudeSquared() < combRad*combRad;
}

#if 0 // Commenting out this which will be made redundant and/or obsolete by Havok integration
hsBool hsBounds3Ext::ISectTriABB(hsBounds3Tri &tri, const hsVector3 &myVel) const
{
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar effMax = fMaxs[i];
		hsScalar effMin = fMins[i];
		if( myVel[i] < 0 )
			effMin += myVel[i];
		else
			effMax += myVel[i];

		int j;
		const UInt32 low = 0x1, hi = 0x2;
		UInt32 mask = low | hi;
		for( j = 0; j < 3; j++ )
		{
			if( tri.fVerts[j][i] > effMin )
				mask &= ~low;
			if( tri.fVerts[j][i] < effMax )
				mask &= ~hi;
		}
		if( mask )
			return false;
	}
	return true;
}

hsBool hsBounds3Ext::TriBSHitInfo(hsBounds3Tri& tri, const hsVector3& myVel, hsHitInfoExt* hit) const
{
	hsPoint3 myPt = GetCenter();
	myPt += myVel;

	hsPoint3 closePt;
	hsBool onTri = tri.ClosestTriPoint(&myPt, &closePt);

	hsVector3 repel;
	repel.Set(&myPt, &closePt);

	hsScalar myDepth;
	hsScalar repelMagSq = repel.MagnitudeSquared();
	if( repelMagSq < hsBounds::kRealSmall )
	{
		repel = tri.fNormal;
		myDepth = GetRadius();
	}
	else
	{
		myDepth = hsFastMath::InvSqrt(repelMagSq);
		repel *= myDepth;
		myDepth = 1.f / myDepth;
		myDepth = GetRadius() - myDepth;
		if( myDepth < 0 )
			myDepth = 0;
	}

	if( tri.fNormal.InnerProduct(myPt) < tri.fDist )
	{
		repel += tri.fNormal * (-2.f * repel.InnerProduct(tri.fNormal));
		myDepth = GetRadius() * 2.f - myDepth;
		if( myDepth < 0 )
			myDepth = 0;
	}

	hit->Set(this, &tri, &repel, myDepth);

	return true;
}

#if 0 // TOCENTER
hsBool hsBounds3Ext::TriBBHitInfo(hsBounds3Tri& tri, const hsVector3& myVel, hsHitInfoExt* hit) const
{
	// Find our closest point (after movement)
	hsPoint3 myPt = fCorner;
	myPt += myVel;

	const hsScalar kMinDist = 1.f; // Huge min dist because world is really big right now. mf horse
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar axDot = fAxes[i].InnerProduct(tri.fNormal);
		if( axDot < -kMinDist )
		{
			// moving towards
			myPt += fAxes[i];
		}
		else if( axDot < kMinDist )
		{
			// need to interp
			axDot /= -(kMinDist*2.f);
			axDot += 0.5f;
			myPt += fAxes[i] * axDot;
		}
		// else moving away, skip it
	}

	// Find closest point on tri to our closest corner
	hsPoint3 closePt;
	hsBool onTri = tri.ClosestTriPoint(&myPt, &closePt);

	// Repel vector is from closest corner to closest point on tri
	hsVector3 repel;
	repel.Set(&myPt, &closePt);
	repel += (-2.f * repel.InnerProduct(tri.fNormal)) * tri.fNormal;

	hsScalar repelMag = hsFastMath::InvSqrt(repel.MagnitudeSquared());
	
	if( repelMag < hsBounds::kRealSmall )
	{
		hsPoint2 faceDepth;
		TestPlane(tri.fNormal, myVel, faceDepth);
		hit->Set(this, &tri, &tri.fNormal, tri.fDist - faceDepth.fX);
		return true;
	}

	repel *= repelMag;
	repelMag = 1.f / repelMag;

	hit->Set(this, &tri, &repel, repelMag);
	
	// Return true of our closest corner projects on to tri (along normal or myVel?)
	return onTri;
}
#else // TOCENTER

hsBool hsBounds3Ext::TriBBHitInfo(hsBounds3Tri& tri, const hsVector3& myVel, hsHitInfoExt* hit) const
{
	hsPoint3 myPt = GetCenter();
	myPt += myVel;

	hsPoint3 closePt;
	hsBool onTri = tri.ClosestTriPoint(&myPt, &closePt);

	hsVector3 repel;
	repel.Set(&myPt, &closePt);
	hsScalar repelDotNorm = repel.InnerProduct(tri.fNormal);
	if( repelDotNorm < 0 )
	{
		repel += (-2.f * repelDotNorm) * tri.fNormal;
	}

	hsScalar repelMagSq = repel.MagnitudeSquared();
	if( repelMagSq < hsBounds::kRealSmall )
		repel = tri.fNormal;
	else
	{
		hsScalar repelMag = hsFastMath::InvSqrt(repelMagSq);
		repel *= repelMag;
	}


	hsPoint2 triDepth;
	tri.TestPlane(repel, triDepth);

	hsPoint2 myDepth;
	TestPlane(repel, myVel, myDepth);

	hit->Set(this, &tri, &repel, triDepth.fY - myDepth.fX);

	return true;
}

#endif // TOCENTER

hsBool hsBounds3Ext::ISectTriBB(hsBounds3Tri &tri, const hsVector3 &myVel) const
{
	hsPoint2 faceDepth;
	// first test box against the triangle plane
	TestPlane(tri.fNormal, myVel, faceDepth);

	if( (tri.fDist > faceDepth.fY)
		||(tri.fDist < faceDepth.fX) )
		return false;

	// now test tri against box planes
	if( TestPoints(3, tri.fVerts, -myVel) > 0 )
		return false;

	if( !(tri.fTriFlags & hsBounds3Tri::kAxesSet) )
		tri.SetAxes();

	hsScalar depth = tri.fDist - faceDepth.fX;
	hsVector3 norm = tri.fNormal;

	// that only leaves the planes of triEdge.Cross(vel)
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsPoint2 depths;
		TestPlane(tri.fPerpAxes[i], myVel, depths);
		if( (tri.fPerpDists[i].fY < depths.fX)
			||(tri.fPerpDists[i].fX > depths.fY) )
			return false;

#if 0
		hsScalar testDepth = tri.fPerpDists[i].fY - depths.fX;
		if( testDepth < depth )
		{
			depth = testDepth;
			norm = tri.fPerpAxes[i];
		}
#endif
	}
	hsScalar vDotN = myVel.InnerProduct(tri.fNormal);
	if( vDotN > 0 )
		depth -= vDotN;

	if( depth <= 0 )
		return false;

	return true;
}

hsBool hsBounds3Ext::ISectTriBB(hsBounds3Tri &tri, const hsVector3 &myVel, hsHitInfoExt *hit) const
{
	hsPoint2 faceDepth;
	// first test box against the triangle plane
	TestPlane(tri.fNormal, myVel, faceDepth);

	if( (tri.fDist > faceDepth.fY)
		||(tri.fDist < faceDepth.fX) )
		return false;

	hsScalar centDist = tri.fNormal.InnerProduct(hit->fRootCenter);
	if( centDist < tri.fDist )
		return false;

	// now test tri against box planes
	if( TestPoints(3, tri.fVerts, -myVel) > 0 )
		return false;

	if( !(tri.fTriFlags & hsBounds3Tri::kAxesSet) )
		tri.SetAxes();

	hsScalar depth = tri.fDist - faceDepth.fX;
	hsVector3 norm = tri.fNormal;

	// that only leaves the planes of triEdge.Cross(vel)
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsPoint2 depths;
		TestPlane(tri.fPerpAxes[i], myVel, depths);
		if( (tri.fPerpDists[i].fY < depths.fX)
			||(tri.fPerpDists[i].fX > depths.fY) )
			return false;

#if 0
		hsScalar testDepth = tri.fPerpDists[i].fY - depths.fX;
		if( testDepth < depth )
		{
			depth = testDepth;
			norm = tri.fPerpAxes[i];
		}
#endif
	}
	hsScalar vDotN = myVel.InnerProduct(tri.fNormal);
	if( vDotN > 0 )
		depth -= vDotN;

	if( (tri.fTriFlags & hsBounds3Tri::kDoubleSide) )
	{
		if( tri.fNormal.InnerProduct(hit->fRootCenter) - tri.fDist < 0 )
		{
			depth = -tri.fDist + faceDepth.fY;
			if( vDotN < 0 )
				depth += vDotN;

			tri.fNormal = -tri.fNormal;
			tri.fDist = -tri.fDist;
		}
	}
	if( depth <= 0 )
		return false;

	//	printf("ATTRIBUTE triBnd addr %x\n",&tri.fNormal);	/* Takashi Nakata TEST Add */
	hit->Set(this, &tri, &norm, depth);

	return hit->fDepth > hsBounds::kRealSmall;
}

hsBool hsBounds3Ext::ISectTriBS(hsBounds3Tri &tri, const hsVector3 &myVel) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();

	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (TriBS)");
	hsScalar radScaled = fRadius * tri.fNormal.Magnitude();
	hsScalar centerDist = tri.fNormal.InnerProduct(fCenter);
	hsScalar velDist = tri.fNormal.InnerProduct(myVel);
	hsScalar effMin = centerDist;
	hsScalar effMax = centerDist;

	if( velDist > 0 )
		effMax += velDist;
	else
		effMin += velDist;
	effMax += radScaled;
	effMin -= radScaled;

	if( tri.fDist <= effMin )
		return false;
	if( tri.fDist >= effMax )
		return false;

	// mf horse
	hsScalar normDepth = tri.fDist - (centerDist - radScaled + velDist);
	if( normDepth <= 0 )
	{
		// we'll report a depth of zero to (hopefully) neutralize any effects
		if( tri.fTriFlags & hsBounds3Tri::kDoubleSide )
		{
			normDepth = -tri.fDist + (centerDist + radScaled + velDist);
			if( normDepth > 0 )
			{	
				tri.fDist = -tri.fDist;
				tri.fNormal = -tri.fNormal;
			}
			else
				normDepth = 0;
		}
		else
			normDepth = 0;
	}
	hsAssert(normDepth >= 0, "NegativeDepth");

	if( !(tri.fTriFlags & hsBounds3Tri::kAxesSet) )
		tri.SetAxes();

	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (TriBS)");
	int i;
	for( i = 0; i < 3; i++ )
	{
		centerDist = tri.fPerpAxes[i].InnerProduct(fCenter);
		velDist = tri.fPerpAxes[i].InnerProduct(myVel);
		effMin = centerDist;
		effMax = centerDist;

		if( velDist > 0 )
			effMax += velDist;
		else
			effMin += velDist;

		hsScalar radScale = fRadius * tri.fPerpAxes[i].Magnitude();
		effMax += radScale;
		effMin -= radScale;
		if( tri.fPerpDists[i].fY <= effMin )
			return false;
		if( tri.fPerpDists[i].fX >= effMax )
			return false;

	}

	return true;
}

hsBool hsBounds3Ext::ISectTriBS(hsBounds3Tri &tri, const hsVector3 &myVel, hsHitInfoExt *hit) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();

	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (TriBS)");
	hsScalar radScaled = fRadius * tri.fNormal.Magnitude();
	hsScalar centerDist = tri.fNormal.InnerProduct(fCenter);
	hsScalar velDist = tri.fNormal.InnerProduct(myVel);
	hsScalar effMin = centerDist;
	hsScalar effMax = centerDist;

	if( velDist > 0 )
		effMax += velDist;
	else
		effMin += velDist;
	effMax += radScaled;
	effMin -= radScaled;

	if( tri.fDist <= effMin )
		return false;
	if( tri.fDist >= effMax )
		return false;

	// mf horse
	hsScalar normDepth = tri.fDist - (centerDist - radScaled + velDist);
	if( normDepth <= 0 )
	{
#if 0 // need to report the collision even if the object is leaving the tri
		// we'll report a depth of zero to (hopefully) neutralize any effects
		if(!(tri.fTriFlags & hsBounds3Tri::kDoubleSide) )
			return false;
		normDepth = -tri.fDist + (centerDist + radScaled + velDist);
		if( normDepth <= 0 )
			return false;
		tri.fDist = -tri.fDist;
		tri.fNormal = -tri.fNormal;
#else
		// we'll report a depth of zero to (hopefully) neutralize any effects
		if( tri.fTriFlags & hsBounds3Tri::kDoubleSide )
		{
			normDepth = -tri.fDist + (centerDist + radScaled + velDist);
			if( normDepth > 0 )
			{	
				tri.fDist = -tri.fDist;
				tri.fNormal = -tri.fNormal;
			}
			else
				normDepth = 0;
		}
		else
			normDepth = 0;
#endif
	}
	hsAssert(normDepth >= 0, "NegativeDepth");

	if( !(tri.fTriFlags & hsBounds3Tri::kAxesSet) )
		tri.SetAxes();

	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (TriBS)");
	int i;
	for( i = 0; i < 3; i++ )
	{
		centerDist = tri.fPerpAxes[i].InnerProduct(fCenter);
		velDist = tri.fPerpAxes[i].InnerProduct(myVel);
		effMin = centerDist;
		effMax = centerDist;

		if( velDist > 0 )
			effMax += velDist;
		else
			effMin += velDist;

		hsScalar radScale = fRadius * tri.fPerpAxes[i].Magnitude();
		effMax += radScale;
		effMin -= radScale;
		if( tri.fPerpDists[i].fY <= effMin )
			return false;
		if( tri.fPerpDists[i].fX >= effMax )
			return false;

	}

	hsScalar invLen = hsScalarInvert(tri.fNormal.Magnitude());
	hit->Set(this, &tri, &tri.fNormal, normDepth);

	// mf horse - move this into Set()?
	hit->fNormal *= invLen;
	hit->fDepth *= invLen;

	return true;
}

#endif // Commenting out this which will be made redundant and/or obsolete by Havok integration

hsBool hsBounds3Ext::ISectBSBS(const hsBounds3Ext& other, const hsVector3& myVel, hsHitInfoExt *hit) const
{
	if(!(fExtFlags & kSphereSet) )
		IMakeSphere();
	if(!(other.fExtFlags & kSphereSet) )
		other.IMakeSphere();

	hsScalar tClose, tImpact;
	if( !IClosestISect(other, myVel, &tClose, &tImpact) )
		return false;
	if( (tImpact < 0) || (tImpact > 1.f) )
		return false;
	if( tClose < 0 )
		tClose = 0;
	if( tClose > 1.f )
		tClose = 1.f;

	hsPoint3 closePt = GetCenter() + myVel * tClose;
	hsVector3 del;
	del.Set(&closePt, &other.GetCenter());

	hsScalar mag = del.Magnitude();
	hsScalar depth = GetRadius() + other.GetRadius() - mag;
	if( depth <= 0 )
		return false;

	hsPoint3 hitPt = GetCenter() + myVel * tImpact;
	hsVector3 norm;
	norm.Set(&hitPt, &other.GetCenter());
	norm.Normalize();

	hit->Set(this, &other, norm, depth);
	return true;
}

hsBool hsBounds3Ext::ISectBSBox(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const
{
	hit->fDelPos = -myVel;
	if( other.ISectBoxBS(*this, hit->fDelPos, hit) )
	{
		hit->fNormal = -hit->fNormal;
		hit->fBoxBnd = this;
		hit->fOtherBoxBnd = &other;
		hit->fDelPos = myVel;

		return true;
	}

	hit->fDelPos = myVel;
	return false;
}

hsBool hsBounds3Ext::ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const
{
	if(!(fExtFlags & kSphereSet) )
		IMakeSphere();
	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (BoxBS(vel))");

	hsVector3 minAxis;
	hsScalar minDepth;
	hsBool haveAxis = false;
	hsVector3 tstAxis;
	hsScalar tstDepth;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsBool tryAxis;
		if( other.fExtFlags & kAxisAligned )
		{
			// first try the other box axes
			hsScalar effMin = fCenter[i];
			hsScalar effMax = effMin;
			hsScalar velDist = myVel[i];
			if( velDist > 0 )
				effMax += velDist;
			else
				effMin += velDist;
			effMax += fRadius;
			effMin -= fRadius;
			
			if( effMax < other.fMins[i] )
				return false;
			if( effMin > other.fMaxs[i] )
				return false;
			
			if( (other.fMins[i] <= effMin)
				&&(other.fMaxs[i] <= effMax) )
			{
				tstDepth = other.fMaxs[i] - effMin;
				hsAssert(tstDepth > -kRealSmall, "Late to be finding sep axis");
				tstAxis.Set(i == 0 ? hsScalar1 : 0, i & 1 ? hsScalar1 : 0, i & 2 ? hsScalar1 : 0);
				tryAxis = true;
			}
			else
			if( (other.fMins[i] >= effMin)
				&&(other.fMaxs[i] >= effMax) )
			{
				tstDepth = effMax - other.fMins[i];
				hsAssert(tstDepth > -kRealSmall, "Late to be finding sep axis");
				tstAxis.Set(i == 0 ? -hsScalar1 : 0, i & 1 ? -hsScalar1 : 0, i & 2 ? -hsScalar1 : 0);
				tryAxis = true;
			}
			else 
				tryAxis = false;
		}
		else
		{
			// first try the other box axes
			hsScalar radScaled = fRadius * other.fAxes[i].Magnitude();
			hsScalar centerDist = other.fAxes[i].InnerProduct(fCenter);
			hsScalar effMin = centerDist;
			hsScalar effMax = centerDist;
			hsScalar velDist = other.fAxes[i].InnerProduct(myVel);
			if( velDist > 0 )
				effMax += velDist;
			else
				effMin += velDist;
			effMax += radScaled;
			effMin -= radScaled;
			
			if( !(other.fExtFlags & kDistsSet) )
				other.IMakeDists();

			if( effMax < other.fDists[i].fX )
				return false;
			if( effMin > other.fDists[i].fY )
				return false;
			
			if( centerDist <= other.fDists[i].fX )
			{
				tstDepth = effMax - other.fDists[i].fX;
				tstAxis = -other.fAxes[i];
				hsAssert(tstDepth > -kRealSmall, "Late to be finding sep axis");
			}
			else
			if( centerDist >= other.fDists[i].fY )
			{
				tstDepth = other.fDists[i].fY - effMin;
				tstAxis = other.fAxes[i];
				hsAssert(tstDepth > -kRealSmall, "Late to be finding sep axis");
			}
			else 
				tryAxis = false;
			
		}
		if( tryAxis )
		{
			hsScalar magSq = tstAxis.MagnitudeSquared();
			if( magSq > kRealSmall )
			{
				tstDepth *= tstDepth * hsScalarInvert(magSq);
				if( !haveAxis||(tstDepth < minDepth) )
				{
					minDepth = tstDepth;
					minAxis = tstAxis;
					haveAxis = true;
				}
				hsAssert(!haveAxis || (minAxis.MagnitudeSquared() > kRealSmall), "Bogus");
			}
		}
	}
	// now try the axis between the center of sphere and center of other box
	hsVector3 diag(&fCenter, &other.GetCenter());
	if( !haveAxis && (diag.MagnitudeSquared() < kRealSmall) )
		diag.Set(1.f, 0, 0);
	hsScalar effMin = diag.InnerProduct(fCenter);
	hsScalar effMax = effMin;
	hsScalar velDist = diag.InnerProduct(myVel);
	if( velDist > 0 )
		effMax += velDist;
	else
		effMin += velDist;
	hsScalar radDist = fRadius * diag.Magnitude();
	effMax += radDist;
	effMin -= radDist;
	hsPoint2 otherDepth;
	other.TestPlane(diag, otherDepth);
	if( effMax < otherDepth.fX )
		return false;
	if( effMin > otherDepth.fY )
		return false;

	tstAxis = diag;
	tstDepth = otherDepth.fY - effMin;
	hsScalar magSq = tstAxis.MagnitudeSquared();
	if( magSq > 0 )
	{
		tstDepth *= tstDepth * hsScalarInvert(magSq);
		if( !haveAxis ||(tstDepth < minDepth) )
		{
			minDepth = tstDepth;
			minAxis = tstAxis;
		}
	}

	hsScalar invMag = hsScalarInvert(minAxis.Magnitude());
	minAxis *= invMag;
	hsAssert(minDepth >= 0, "Late to find sep plane");
	minDepth = hsSquareRoot(minDepth);
	hit->Set(this, &other, minAxis, minDepth);

	return true;
}

hsBool hsBounds3Ext::ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();
	hsAssert(fBounds3Flags & kCenterValid, "Sphere set but not center (BoxBS)");

	if( other.fExtFlags & kAxisAligned )
	{
		// first try the other box axes
		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar effMin = fCenter[i];
			hsScalar effMax = effMin;
			hsScalar velDist = myVel[i];
			if( velDist > 0 )
				effMax += velDist;
			else
				effMin += velDist;
			effMax += fRadius;
			effMin -= fRadius;

			if( effMax < other.fMins[i] )
				return false;
			if( effMin > other.fMaxs[i] )
				return false;
		}
	}
	else
	{
		// first try the other box axes
		if( !(other.fExtFlags & kDistsSet) )
			other.IMakeDists();

		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar effMin = other.fAxes[i].InnerProduct(fCenter);
			hsScalar effMax = effMin;
			hsScalar velDist = other.fAxes[i].InnerProduct(myVel);
			if( velDist > 0 )
				effMax += velDist;
			else
				effMin += velDist;
			hsScalar radScaled = fRadius * other.fAxes[i].Magnitude();
			effMax += radScaled;
			effMin -= radScaled;

			if( effMax < other.fDists[i].fX )
				return false;
			if( effMin > other.fDists[i].fY )
				return false;
		}
	}

	// now try the axis between the center of sphere and center of other box
	hsVector3 diag(&fCenter, &other.GetCenter());
	hsScalar effMin = diag.InnerProduct(fCenter);
	hsScalar effMax = effMin;
	hsScalar velDist = diag.InnerProduct(myVel);
	if( velDist > 0 )
		effMax += velDist;
	else
		effMin += velDist;
	hsScalar radDist = fRadius * diag.Magnitude();
	effMax += radDist;
	effMin -= radDist;
	hsPoint2 otherDepth;
	other.TestPlane(diag, otherDepth);
	if( effMax < otherDepth.fX )
		return false;
	if( effMin > otherDepth.fY )
		return false;

	return true;
}

hsBool hsBounds3Ext::ISectLine(const hsPoint3* from, const hsPoint3* at) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();

	hsPoint3 onLine;
	hsScalar z = ClosestPointToLine(&fCenter, from, at, &onLine);
	
	hsScalar distSq = hsVector3(&onLine, &fCenter).MagnitudeSquared();
	if( distSq >= fRadius*fRadius )
		return false;

	if( fExtFlags & kAxisAligned )
	{
		int i;
		for( i = 0; i < 3; i++ )
		{
			if( ((*from)[i] < fMins[i])&&((*at)[i] < fMins[i]) )
				return false;
			if( ((*from)[i] > fMaxs[i])&&((*at)[i] > fMaxs[i]) )
				return false;
		}
	}
	else
	{
		if( !(fExtFlags & kDistsSet) )
			IMakeDists();

		int i;
		for( i = 0; i < 3; i++ )
		{
			hsScalar d0 = fAxes[i].InnerProduct(from);
			hsScalar d1 = fAxes[i].InnerProduct(at);
			if( d0 < d1 )
			{
				if( d1 < fDists[i].fX )
					return false;
				if( d0 > fDists[i].fY )
					return false;
			}
			else
			{
				if( d0 < fDists[i].fX )
					return false;
				if( d1 > fDists[i].fY )
					return false;
			}
		}
	}
	return true;
}

hsBool hsBounds3Ext::ISectCone(const hsPoint3* from, const hsPoint3* at, hsScalar radius) const
{
	if( !(fExtFlags & kSphereSet) )
		IMakeSphere();

	// expensive
	hsPoint3 onLine;
	ClosestPointToLine(&fCenter, from, at, &onLine);

	hsScalar distSq = hsVector3(&onLine, &fCenter).MagnitudeSquared();
	hsScalar radiusSq = fRadius * fRadius;
	if (distSq - radius*radius >= radiusSq)
		return false;

	hsScalar dist = hsVector3(from, &onLine).Magnitude();
	hsScalar len = hsVector3(from, at).Magnitude();
	hsScalar partRadius = radius/len * dist;
	if (distSq - fRadius*fRadius - partRadius*partRadius >= 0)
	{
		hsVector3 rayToCenter(&fCenter,&onLine);
		rayToCenter.Normalize();

		hsPoint3 atEdge = *at + rayToCenter*radius;

		ClosestPointToLine(&fCenter, from, &atEdge, &onLine);

		distSq = hsVector3(&onLine, &fCenter).MagnitudeSquared();
		if( distSq >= radiusSq )
			return false;
	}

	// incorrect
	if( fExtFlags & kAxisAligned )
	{
		int i;
		for( i = 0; i < 3; i++ )
		{
			if( ((*from)[i] < fMins[i])&&((*at)[i]+radius < fMins[i]) )
				return false;
			if( ((*from)[i] > fMaxs[i])&&((*at)[i]-radius > fMaxs[i]) )
				return false;
		}
	}
	else
	{
		if( !(fExtFlags & kDistsSet) )
			IMakeDists();

		int i;
		for( i = 0; i < 3; i++ )
		{
			ClosestPointToInfiniteLine(at, &fAxes[i], &onLine);
			hsVector3 atLine(&onLine,at);
			atLine.Normalize();
			hsPoint3 atEdge = *at + atLine * radius;

			hsScalar d0 = fAxes[i].InnerProduct(*from);
			hsScalar d1 = fAxes[i].InnerProduct(atEdge);
			if( d0 < d1 )
			{
				if( d1 < fDists[i].fX )
					return false;
				if( d0 > fDists[i].fY )
					return false;
			}
			else
			{
				if( d0 < fDists[i].fX )
					return false;
				if( d1 > fDists[i].fY )
					return false;
			}
		}
	}
	return true;
}


hsBool hsBounds3Ext::ISectRayBS(const hsPoint3& from, const hsPoint3& to, hsPoint3& at) const
{
	hsVector3 c2f(&from,&GetCenter());
	hsVector3 f2t(&to,&from);
	hsScalar a = f2t.MagnitudeSquared();
	hsScalar b = 2 * (c2f.InnerProduct(f2t));
	hsScalar c = c2f.MagnitudeSquared() - GetRadius()*GetRadius();
		
	hsScalar disc = b*b - 4*a*c;
	if (disc < 0)
		return false;
	else
	{
		hsScalar discSqrt = hsSquareRoot(disc);
		hsScalar denom = 1.f/(2*a);
		hsScalar t = (-b - discSqrt) * denom;

		if (t<1 && t>0)
			at = from + (f2t * t);
		else
			return false;
#if 0
		{
			t = (-b + discSqrt) * denom;
			if (t > 1)
				return false;
			at = from + (f2t * t);
		}
#endif
		return true;
	}
}

void hsBounds3Ext::Read(hsStream *s)
{
	fExtFlags = s->ReadSwap32();
	hsBounds3::Read(s);
	if( !(fExtFlags & kAxisAligned) )
	{
		fCorner.Read(s);
		int i;
		for( i = 0; i < 3; i++ )
		{
			fAxes[i].Read(s);
			fDists[i].fX = s->ReadSwapScalar();
			fDists[i].fY = s->ReadSwapScalar();
		}
		IMakeMinsMaxs();
		IMakeDists();
	}
	IMakeSphere();
}
void hsBounds3Ext::Write(hsStream *s)
{
	s->WriteSwap32(fExtFlags);
	hsBounds3::Write(s);
	if( !(fExtFlags & kAxisAligned) )
	{
		fCorner.Write(s);
		int i;
		for( i = 0; i < 3; i++ )
		{
			fAxes[i].Write(s);
			if( fExtFlags & kDistsSet )
			{
				s->WriteSwapScalar(fDists[i].fX);
				s->WriteSwapScalar(fDists[i].fY);
			}
			else
			{
				// Playing nice with binary patches--writing uninited values BAD!
				s->WriteSwapScalar( 0.f );
				s->WriteSwapScalar( 0.f );
			}
		}
	}
}

#if 0 // Commenting out this which will be made redundant and/or obsolete by Havok integration
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void hsBounds3Tri::TestPlane(const hsVector3 &n, hsPoint2 &depth) const
{
	depth.fX = depth.fY = n.InnerProduct(fVerts[0]);

	hsScalar d1, d2;

	d1 = n.InnerProduct(fVerts[1]);
	d2 = n.InnerProduct(fVerts[2]);

	if( d1 > d2 )
	{
		if( d1 > depth.fY )
			depth.fY = d1;
		if( d2 < depth.fX )
			depth.fX = d2;
	}
	else
	{
		if( d2 > depth.fY )
			depth.fY = d2;
		if( d1 < depth.fX )
			depth.fX = d1;
	}
}
hsBool hsBounds3Tri::ClosestTriPoint(const hsPoint3 *p, hsPoint3 *out, const hsVector3 *ax) const
{
	// project point onto tri plane
	hsPoint3 pPln;
	if( ax )
	{
		hsScalar t;

		t =  fNormal.InnerProduct(fVerts[0] - *p);
		hsScalar s = fNormal.InnerProduct(ax);
		if( (s > hsBounds::kRealSmall)||(s < -hsBounds::kRealSmall) )
		{
			t /= s;

			pPln = *p;
			pPln += *ax * t;
		}
		else
		{
			return ClosestTriPoint(p, out);
		}
	}
	else
	{
		hsScalar t;

		t =  fNormal.InnerProduct(fVerts[0] - *p);
		t /= fNormal.MagnitudeSquared();

		pPln = *p;
		pPln += fNormal * t;
	}

	if( !(fTriFlags & kAxesSet) )
		SetAxes();
	
	int nIn = 0;
	int firstIn, secondIn;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar tst = fPerpAxes[i].InnerProduct(pPln);
		hsBool in = false;
		if( fOnIsMax & (1 << i) )
		{
			if( tst <= fPerpDists[i].fY )
				in = true;
		}
		else
		{
			if( tst >= fPerpDists[i].fX )
				in = true;
		}
		if( in )
		{
			if( nIn++ )
				secondIn = i;
			else
				firstIn = i;
		}
	}
	switch( nIn )
	{
	case 3:
		*out = pPln;
		break;
	case 1:
		{
			int k, kPlus;
			k = firstIn == 2 ? 0 : firstIn+1;
			kPlus = k == 2 ? 0 : k+1;

			hsPoint3 pTmp;
			hsScalar z;
			z = hsBounds3::ClosestPointToLine(&pPln, fVerts+k, fVerts+kPlus, &pTmp);
			if( z <= hsScalar1 )
				*out = pTmp;
			else
			{
				k = kPlus;
				kPlus = k == 2 ? 0 : k+1;
				z = hsBounds3::ClosestPointToLine(&pPln, fVerts+k, fVerts+kPlus, out);
			}
		}
		break;
	case 2:
		{
			int k, kPlus;
			k = secondIn == 2 ? 0 : secondIn+1;
			if( k == firstIn )
				k++;
			kPlus = k == 2 ? 0 : k+1;

			hsBounds3::ClosestPointToLine(&pPln, fVerts+k, fVerts+kPlus, out);
			break;
		}

	case 0:
		hsAssert(false, "Extreme bogosity, inverted tri?!?");
		*out = pPln;
		return false;
	}

#ifdef HS_DEBUGGING // mf horse testing
#if 0
	if( 0 )
	{
		hsVector3 ndeb = hsVector3(fVerts+1, fVerts) % hsVector3(fVerts+2, fVerts);
		hsScalar dis;
		dis = fNormal.InnerProduct(pPln) - fDist;
		if( (fDist > hsBounds::kRealSmall)||(fDist < -hsBounds::kRealSmall) )
			dis /= fDist;
		hsAssert((dis < hsBounds::kRealSmall)&&(dis > -hsBounds::kRealSmall), "Non-planar pPln");
		dis = hsVector3(&pPln, out).MagnitudeSquared();
		hsScalar vDis;
		vDis = hsVector3(&pPln, fVerts+0).MagnitudeSquared();
		hsAssert( vDis - dis > -hsBounds::kRealSmall, "Bad closest point");
		vDis = hsVector3(&pPln, fVerts+1).MagnitudeSquared();
		hsAssert( vDis - dis > -hsBounds::kRealSmall, "Bad closest point");
		vDis = hsVector3(&pPln, fVerts+2).MagnitudeSquared();
		hsAssert( vDis - dis > -hsBounds::kRealSmall, "Bad closest point");
		hsBool dork = false;
		if( dork )
		{
			hsScalar zn[3];
			hsScalar zf[3];
			hsScalar z[3];
			int i;
			for( i = 0; i < 3; i++ )
			{
				z[i] = fPerpAxes[i].InnerProduct(fVerts[i]);
				int j;
				j = i == 0 ? 2 : i-1;
				zf[i] = fPerpAxes[i].InnerProduct(fVerts[j]);
				j = i == 2 ? 0 : i+1;
				zn[i] = fPerpAxes[i].InnerProduct(fVerts[j]);
			}
			return ClosestTriPoint(p, out, ax);
		}
	}
#endif
#endif

	return 3 == nIn;
}

void hsBounds3Tri::SetAxes() const
{
	fOnIsMax = 0;

	hsVector3 edge[3];
	edge[0].Set(fVerts, fVerts+1);
	edge[1].Set(fVerts+1, fVerts+2);
	edge[2].Set(fVerts+2, fVerts);
	hsVector3 perp = edge[2] % edge[0];

	int i;
	for( i = 0; i < 3; i++ )
	{
		int j = i == 2 ? 0 : i+1;
		int k = j == 2 ? 0 : j+1;
		fPerpAxes[i] = edge[i] % perp;
		fPerpAxes[i].Normalize();

		fPerpDists[i].fX = fPerpAxes[i].InnerProduct(fVerts[i]);
		fPerpDists[i].fY = fPerpAxes[i].InnerProduct(fVerts[k]);
		if( fPerpDists[i].fX > fPerpDists[i].fY )
		{
			fOnIsMax |= 1 << i;
			hsScalar d = fPerpDists[i].fX;
			fPerpDists[i].fX = fPerpDists[i].fY;
			fPerpDists[i].fY = d;
		}
	}
	fTriFlags |= kAxesSet;
}

hsBounds3Tri* hsBounds3Tri::Transform(const hsMatrix44& x)
{
#if 0 // IDENT
	if( x.fFlags & hsMatrix44::kIsIdent )
		return this;
#endif // IDENT

	fVerts[0] = x * fVerts[0];
	fVerts[1] = x * fVerts[1];
	fVerts[2] = x * fVerts[2];

	hsVector3 v1, v2;
	v1.Set(&fVerts[1], &fVerts[0]);
	v2.Set(&fVerts[2], &fVerts[0]);
	fNormal = v1 % v2;
	// mf horse - do we need to normalize here?
	// fNormal.Normalize();

	fDist = fNormal.InnerProduct(fVerts[0]);

	fTriFlags &= ~kAxesSet;

	SetAxes();

	return this;
}

hsBounds3Tri* hsBounds3Tri::Translate(const hsVector3& v)
{
	fVerts[0] += v;
	fVerts[1] += v;
	fVerts[2] += v;

	fDist = fNormal.InnerProduct(fVerts[0]);

	int i;
	for( i = 0; i < 3; i++ )
	{
		int j = i == 2 ? 0 : i+1;
		int k = j == 2 ? 0 : j+1;

		hsScalar del = fPerpAxes[i].InnerProduct(v);
		fPerpDists[i].fX += del;
		fPerpDists[i].fY += del;
	}

	return this;
}

void hsBounds3Tri::Set(const hsPoint3& v0, 
						   const hsPoint3& v1, 
						   const hsPoint3& v2,
						   hsTriangle3* t,
						   const hsMatrix44& x)
{
	fVerts[0] = v0;
	fVerts[1] = v1;
	fVerts[2] = v2;

	fOnIsMax = 0;
	fTriangle = t;

	if( t->fFlags & hsTriangle3::kTwoSided )
		fTriFlags |= kDoubleSide;

#if 0 // IDENT
	if( x.fFlags & hsMatrix44::kIsIdent )
	{
		hsVector3 v1, v2;
		v1.Set(&fVerts[1], &fVerts[0]);
		v2.Set(&fVerts[2], &fVerts[0]);
		fNormal = v1 % v2;
		// mf horse - do we need to normalize here?
		// fNormal.Normalize();

		fDist = fNormal.InnerProduct(fVerts[0]);

		fTriFlags &= ~kAxesSet;

		SetAxes();
	}
	else
#endif // IDENT
		Transform(x);
}

hsBounds3Tri::hsBounds3Tri(const hsPoint3& v0, 
						   const hsPoint3& v1, 
						   const hsPoint3& v2,
						   hsTriangle3* t,
						   const hsMatrix44& x)
{
	Set(v0, v1, v2, t, x);
}

hsBounds3Tri::hsBounds3Tri(hsTriangle3* t, const hsMatrix44& x)
{
	Set(t->fVert[0]->fVtx->fLocalPos, 
		t->fVert[2]->fVtx->fLocalPos, 
		t->fVert[2]->fVtx->fLocalPos, 
		t, x);
}

void hsBounds3Tri::Set(hsPoint3 *v0, hsPoint3 *v1, hsPoint3 *v2, hsVector3 *n, UInt32 triFlags, hsTriangle3 *t)
{
	fTriFlags = 0;

	if( triFlags & hsTriangle3::kTwoSided )
		fTriFlags |= kDoubleSide;

	fNormal = *n;
	fVerts[0] = *v0;
	fVerts[1] = *v1;
	fVerts[2] = *v2;

	fOnIsMax = 0;
	fTriangle = t;

	fDist = fNormal.InnerProduct(fVerts[0]);
}

hsBounds3Tri::hsBounds3Tri(hsPoint3 *v0, hsPoint3 *v1, hsPoint3 *v2, hsVector3 *n, UInt32 triFlags, hsTriangle3 *t)
{
	Set(v0, v1, v2, n, triFlags, t);
}

hsBounds3Tri::hsBounds3Tri(hsTriangle3* t)
{
	Set(&t->fVert[0]->fVtx->fLocalPos, 
		&t->fVert[1]->fVtx->fLocalPos, 
		&t->fVert[2]->fVtx->fLocalPos, 
		&t->fNormal, t->fFlags, t);
}

hsBounds3Tri::~hsBounds3Tri()
{
}


// Finds closest intersection vertex or triangle/center-line intersection
hsBool hsBounds3Tri::ISectCone(const hsPoint3& from, const hsPoint3& to, hsScalar cosThetaSq, hsBool ignoreFacing, hsPoint3& at, hsBool& backSide) const
{
	hsScalar d0 = from.InnerProduct(fNormal);
	hsScalar d1 = at.InnerProduct(fNormal);
	hsScalar dt = fNormal.InnerProduct(fVerts[0]);
	backSide = d0 < dt;
	if( !ignoreFacing && backSide )
		return false;
	if ( (d0 < dt || d1 < dt) && 
		 (d0 > dt || d1 > dt) &&
		 ClosestTriPoint(&from, &at, &hsVector3(&to,&from)) )
		 return true;

	hsVector3 av(&to,&from);
	hsScalar distASq = av.MagnitudeSquared();
	hsScalar radiusSq = distASq * (1-cosThetaSq)/cosThetaSq;

	hsScalar minDistSq = 0;
	Int32 minVert = 0;
	hsBool sect = false;
	for (Int32 i=0; i<3; i++)
	{
		hsPoint3 onLine;
		hsScalar t = hsBounds3::ClosestPointToLine(&fVerts[i], &from, &to, &onLine);

		// outside the cap of the cylinder
		if (t<0 || t>1)
			continue;

		// outside the edge of the cylinder
		if (hsVector3(&onLine, &fVerts[i]).MagnitudeSquared() >= radiusSq)
			continue;

		hsVector3 bv(&fVerts[i],&from);

		hsScalar distBSq = bv.MagnitudeSquared();

		hsScalar cosMuSquared = (av * bv) / (distASq * distBSq);

		// outside the angle of the cone
		if (cosMuSquared > cosThetaSq)
			continue;
		
		if (!sect || distBSq < minDistSq)
		{
			minVert = i;
			minDistSq = distBSq;
			sect = true;
		}
	}
	at = fVerts[minVert];
	return sect;
}
#endif // Commenting out this which will be made redundant and/or obsolete by Havok integration
