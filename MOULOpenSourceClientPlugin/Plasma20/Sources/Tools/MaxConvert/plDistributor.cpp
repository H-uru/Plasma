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

#include "hsWindows.h"
#include <commdlg.h>

#include "Max.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "iparamb2.h"
#include "meshdlib.h" 

#include "../MaxExport/plExportProgressBar.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

#include "hsMaterialConverter.h"

#include "hsTypes.h"
#include "hsGeometry3.h"
#include "hsUtils.h"
#include "../plMath/plTriUtils.h"

#include "plDistributor.h"
#include "../MaxMain/plMaxNode.h"

#include "plDistTree.h"

static const float kMaxHeight = 10.f;

// Inputs:
//	seed mesh		- geometric entity to replicate over the surface
//	surf mesh		- surface to cover with seed data
//	spacing			- nominal distance between replicants (before randomizing).
//	rnd pos radius	- replicants spread uniformly about nominal spacing by +- radius
//	alignment vector- preferred up direction for replicants
//	alignment weight- weight between alignment vector and interpolated face normal
//	normal range	- amount to randomize the weighted normal 
//	normal bunch	- amount randomized normals bunch around input normal
//	rotation range	- amount to randomize the azimuthal rotation about the randomized normal
//	prob bitmap		- describes probability that a seed planted here on the surface will sprout
//	prob uvws		- mapping from verts to prob bitmap, one per src span vertex
//	overall rnd		- overall probability that a candidate seed point (passing all other criteria) will be used
//

static inline hsPoint3& hsP3(const Point3& p) { return *(hsPoint3*)&p; }
static inline hsVector3& hsV3(const Point3& p) { return *(hsVector3*)&p; }
static inline Matrix3 Transpose(const Matrix3& m)
{
	return Matrix3(m.GetColumn3(0), m.GetColumn3(1), m.GetColumn3(2), Point3(0,0,0));
}

plDistributor::plDistributor()
{
	IClear();
	fRand.SetSeed(UInt32(this));
}

plDistributor::~plDistributor()
{
	Reset();
}

void plDistributor::Reset()
{
	// Let stuff go...
	if( fSurfObjToDelete )
		fSurfObjToDelete->DeleteThis();

	// Then clear
	IClear();
}

void plDistributor::IClear()
{
	fInterface = nil;

	fDistTree = nil;

	fMeshTree.Reset();

	fSurfNode = nil;
	fSurfMesh = nil;
	fSurfObjToDelete = nil;

	fRepNodes.ZeroCount();

	fIsolation = kIsoLow;
	fConformity = kConformNone;
	fFaceNormals = false;

	fMaxConform = 1.f;

	fSpacing = 5.f;
	fRndPosRadius = 2.5f;

	fAlignVec.Set(0,0,1.f);
	fAlignWgt = 0;

	fOffsetMin = fOffsetMax = 0;

	fPolarRange = 0;
	fTanPolarRange = 0;
	fAzimuthRange = hsScalarPI;

	fOverallProb = 1.f;

	fPolarBunch = 0;

	fScaleLock = kLockNone;
	fScaleLo.Set(1.f,1.f,1.f);
	fScaleHi.Set(1.f,1.f,1.f);

	fProbBitmapTex = nil;
	fProbLayerTex = nil;
	fProbColorChan = kRed;

	fProbRemapFromLo = 0;
	fProbRemapFromHi = 1.f;
	fProbRemapToLo = 0;
	fProbRemapToHi = 1.f;

	fAngProbVec.Set(0.f, 0.f, 1.f);
	fAngProbHi = fAngProbLo = 0;
	fAngProbTrans = 0;

	fAltProbHi = fAltProbLo = 0;
	fAltProbTrans = 0;

	fFade.pmin = fFade.pmax = Point3(0,0,0);

	fBone = nil;
}

BOOL plDistributor::IGetMesh(INode* node, TriObject*& objToDelete, Mesh*& retMesh) const
{
	if( objToDelete )
		objToDelete->DeleteThis();

	retMesh = nil;
	objToDelete = nil;

	// Get da object
	Object *obj = node->EvalWorldState(TimeValue(0)).obj;
	if( !obj )
		return false;

	if( !obj->CanConvertToType( triObjectClassID ) )
		return false;

	// Convert to triMesh object
	TriObject	*meshObj = (TriObject*)obj->ConvertToType(TimeValue(0), triObjectClassID);
	if( !meshObj )
		return false;

	if( meshObj != obj )
		objToDelete = meshObj;

	// Get the mesh
	Mesh* mesh = &(meshObj->mesh);
	if( !mesh->getNumFaces() )
	{
		if( objToDelete )
			objToDelete->DeleteThis();
		objToDelete = nil;
		return false;
	}

	retMesh = mesh;
	retMesh->checkNormals(true);
	
	return true;
}

void plDistributor::ISetAngProbCosines() const
{
	if( fAngProbHi == fAngProbLo )
		return;

	float maxAng, minAng;
	if( fAngProbHi > fAngProbLo )
	{
		maxAng = hsScalarDegToRad(fAngProbHi);
		minAng = hsScalarDegToRad(fAngProbLo);
	}
	else
	{
		maxAng = hsScalarDegToRad(fAngProbLo);
		minAng = hsScalarDegToRad(fAngProbHi);
	}

	float transAng = hsScalarDegToRad(fAngProbTrans);
	if( transAng > (maxAng - minAng) * 0.5f )
		transAng = (maxAng - minAng) * 0.5f;

	float transAngMax = maxAng < hsScalarPI ? transAng : 0;
	float transAngMin = minAng > 0 ? transAng : 0;

	fCosAngProbHi = hsCosine(minAng);
	fCosAngProbLo = hsCosine(maxAng);
	fCosAngProbHiTrans = hsCosine(minAng + transAngMin);
	fCosAngProbLoTrans = hsCosine(maxAng - transAngMax);
}

BOOL plDistributor::ISetSurfaceNode(INode* surfNode) const
{
	if( !IGetMesh(surfNode, fSurfObjToDelete, fSurfMesh) )
	{
		return false;
	}
	fSurfNode = surfNode;

	fSurfToWorld = surfNode->GetObjectTM(TimeValue(0));
	fWorldToSurf = Inverse(fSurfToWorld);

	fSurfToWorldVec = Transpose(fWorldToSurf);
	fWorldToSurfVec = Transpose(fSurfToWorld);

	fSurfAngProbVec = FNormalize(fWorldToSurfVec * fAngProbVec);
	// This doesn't have anything to do with the surface node, but it
	// does have to do with the SurfAngProbVec, and this is as good a
	// place as any to do it.
	ISetAngProbCosines();

	fSurfAlignVec = FNormalize(fWorldToSurfVec * fAlignVec);

	if( INeedMeshTree() )
		IMakeMeshTree();

	return true;
}

BOOL plDistributor::INeedMeshTree() const
{
	switch( fConformity )
	{
	case kConformAll:
	case kConformHeight:
	case kConformCheck:
	case kConformBase:
		return true;
	}
	return false;
}

void plDistributor::IMakeMeshTree() const
{
	fMeshTree.Reset();

	const Box3 nonFade = fMeshTree.NonFade();
	int i;
	for( i = 0; i < fSurfMesh->getNumFaces(); i++ )
	{
		Point3 p0 = fSurfMesh->getVert(fSurfMesh->faces[i].getVert(0)) * fSurfToWorld;
		Point3 p1 = fSurfMesh->getVert(fSurfMesh->faces[i].getVert(1)) * fSurfToWorld;
		Point3 p2 = fSurfMesh->getVert(fSurfMesh->faces[i].getVert(2)) * fSurfToWorld;

		Box3 box(p0, p0);
		box += p1;
		box += p2;

		fMeshTree.AddBoxIData(box, nonFade, i);

	}
}

void plDistributor::IFindFaceSet(const Box3& box, Tab<Int32>& faces) const
{
	Tab<Int32> distNodes;
	fMeshTree.HarvestBox(box, distNodes);
	int i;
	for( i = 0; i < distNodes.Count(); i++ )
	{
		Int32 iFace = Int32(fMeshTree.GetBox(distNodes[i]).fIData);
		faces.Append(1, &iFace);
	}
}

BOOL plDistributor::IValidateSettings(INode* surfNode, plMeshCacheTab& cache) const
{
	if( !fInterface )
		return false;

	if( !ISetSurfaceNode(surfNode) )
		return false;

	if( !IReadyRepNodes(cache) )
		return false;

	return true;
}

BOOL plDistributor::Distribute(INode* surfNode, plDistribInstTab& reps, plMeshCacheTab& cache, plExportProgressBar& bar) const
{
	// Validate current settings.
	if( !IValidateSettings(surfNode, cache) )
		return false;


	return IDistributeOverMesh(reps, cache, bar);
}

BOOL plDistributor::IDistributeOverMesh(plDistribInstTab& reps, plMeshCacheTab& cache, plExportProgressBar& bar) const
{
	int iUpdate = (fSurfMesh->getNumFaces() >> 4) + 1;
	int i;
	for( i = 0; i < fSurfMesh->getNumFaces(); i++ )
	{
		IDistributeOverFace(i, reps, cache);

		if( ((i / iUpdate) * iUpdate) == i )
		{
			if( bar.Update(nil) )
				return false;
		}
	}
	return true;
}

Point3& plDistributor::IPerturbPoint(Point3& pt) const
{
	pt.x += fRand.RandMinusOneToOne() * fRndPosRadius;
	pt.y += fRand.RandMinusOneToOne() * fRndPosRadius;
	pt.z += fRand.RandMinusOneToOne() * fRndPosRadius;
	return pt;
}

Box3 plDistributor::ISetupGrid(const Point3& p0, const Point3& p1, const Point3& p2) const
{
	// Add half spacing to max's to protect against precision errors.
	Box3 box(p0, p0);
	box += p1;
	box += p2;

	Point3 mins, maxs;
	int i;
	for( i = 0; i < 3; i++ )
	{
		hsScalar t = box.Min()[i];
		t /= fSpacing;
		t = hsFloor(t);
		t *= fSpacing;
		mins[i] = t;

		t = box.Max()[i];
		t /= fSpacing;
		t = hsCeil(t);
		t *= fSpacing;
		maxs[i] = t + fSpacing*0.5f;
	}
	box = Box3(mins, maxs);

	return box;
}

hsBool plDistributor::IFailsProbBitmap(int iFace, const Point3& bary) const
{
	// If we don't have a probability map, or we don't have
	// valid coordinates into it, just return false. That is,
	// with no valid probability map, everything goes.
	int uvwChan = 1;
	Matrix3 uvtrans(true);
	Bitmap* bm = nil;
	UINT filtType = BMM_FILTER_PYRAMID;
	if( fProbBitmapTex )
	{
		uvwChan = fProbBitmapTex->GetMapChannel();
		fProbBitmapTex->GetUVTransform(uvtrans);

		bm = fProbBitmapTex->GetBitmap(TimeValue(0));

		if( bm && !bm->HasFilter() )
		{
			switch( fProbBitmapTex->GetFilterType() )
			{
			default:
			case FILTER_PYR:
				filtType = BMM_FILTER_PYRAMID;
				break;
			case FILTER_SAT:
				filtType = BMM_FILTER_SUM;
				break;
			case FILTER_NADA:
				filtType = BMM_FILTER_NONE;
				break;
			}
		}
	}
	else if( fProbLayerTex )
	{
		uvwChan = fProbLayerTex->GetMapChannel();
		fProbLayerTex->GetUVTransform(uvtrans);

		bm = fProbLayerTex->GetBitmap(TimeValue(0));

	}

	if( !bm )
		return false;

	if( !bm->HasFilter() )
		bm->SetFilter(filtType);

	bm->PrepareGChannels(&bm->Storage()->bi);

	if( !fSurfMesh->mapSupport(uvwChan) )
		return false;

	if( !fSurfMesh->mapFaces(uvwChan) || !fSurfMesh->mapVerts(uvwChan) )
		return false;

	// Lookup the appropriate texel value
	Point3 uvw;
	uvw = fSurfMesh->mapVerts(uvwChan)[fSurfMesh->mapFaces(uvwChan)[iFace].getTVert(0)] * bary[0];
	uvw += fSurfMesh->mapVerts(uvwChan)[fSurfMesh->mapFaces(uvwChan)[iFace].getTVert(1)] * bary[1];
	uvw += fSurfMesh->mapVerts(uvwChan)[fSurfMesh->mapFaces(uvwChan)[iFace].getTVert(2)] * bary[2];

	uvw = uvw * uvtrans;

	float fu = uvw.x - int(uvw.x);
	if( fu < 0 )
		fu += 1.f;
	float fv = 1.0f - (uvw.y - int(uvw.y));
	if( fv < 0 )
		fv += 1.f;
	float du = 1.f / bm->Width();
	float dv = 1.f / bm->Height();

	BMM_Color_fl evCol;
	bm->GetFiltered(fu, fv, du, dv, &evCol);
	
	float frac;
	switch( fProbColorChan )
	{
	case kRed:
		frac = evCol.r;
		break;
	case kGreen:
		frac = evCol.g;
		break;
	case kBlue:
		frac = evCol.b;
		break;
	case kAlpha:
		frac = evCol.a;
		break;
	case kAverageRedGreen:
		frac = (evCol.r + evCol.g) / 2.f;
		break;
	case kAverageRedGreenTimesAlpha:
		frac = (evCol.r + evCol.g) / 2.f * evCol.a;
		break;
	case kAverage:
		frac = (evCol.r + evCol.g + evCol.b ) / 3.f;
		break;
	case kAverageTimesAlpha:
		frac = (evCol.r + evCol.g + evCol.b ) / 3.f * evCol.a;
		break;
	case kMax:
	case kMaxColor:
		frac = hsMaximum(evCol.r, hsMaximum(evCol.g, evCol.b));
		break;
	case kMaxColorTimesAlpha:
		frac = hsMaximum(evCol.r, hsMaximum(evCol.g, evCol.b)) * evCol.a;
		break;
	case kMaxRedGreen:
		frac = hsMaximum(evCol.r, evCol.g);
		break;
	case kMaxRedGreenTimesAlpha:
		frac = hsMaximum(evCol.r, evCol.g) * evCol.a;
		break;
	}

	if( fProbRemapFromHi != fProbRemapFromLo )
		frac = fProbRemapToLo + (frac - fProbRemapFromLo) / (fProbRemapFromHi - fProbRemapFromLo) * (fProbRemapToHi - fProbRemapToLo);
	else
		frac = frac > fProbRemapFromHi ? fProbRemapToHi : fProbRemapToLo;

	return frac < fRand.RandZeroToOne();
}

Point3 plDistributor::IGetSurfaceNormal(int iFace, const Point3& bary) const
{
	fSurfMesh->checkNormals(true);

	if( !fFaceNormals )
	{
		Face& face = fSurfMesh->faces[iFace];
		Point3 norm = FNormalize(fSurfMesh->getNormal(face.getVert(0))) * bary[0];
		norm += FNormalize(fSurfMesh->getNormal(face.getVert(1))) * bary[1];
		norm += FNormalize(fSurfMesh->getNormal(face.getVert(2))) * bary[2];

		return norm;
	}

	Point3 faceNorm = fSurfMesh->getFaceNormal(iFace);
	return FNormalize(faceNorm);
}

hsBool plDistributor::IFailsAngProb(int iFace, const Point3& bary) const
{
	if( fAngProbLo == fAngProbHi )
		return false;

	Point3 norm = IGetSurfaceNormal(iFace, bary);

	float dot = DotProd(norm, fSurfAngProbVec);

	if( dot > fCosAngProbHi )
		return true;
	if( dot < fCosAngProbLo )
		return true;

	if( dot > fCosAngProbHiTrans )
	{
		float prob = fCosAngProbHi - dot;
		prob /= fCosAngProbHi - fCosAngProbHiTrans;
		return fRand.RandZeroToOne() >= prob;
	}

	if( dot < fCosAngProbLoTrans )
	{
		float prob = dot - fCosAngProbLo;
		prob /= fCosAngProbLoTrans - fCosAngProbLo;
		return fRand.RandZeroToOne() >= prob;
	}

	return false;

}

hsBool plDistributor::IFailsAltProb(int iFace, const Point3& bary) const
{
	if( fAltProbLo == fAltProbHi )
		return false;

	Face& face = fSurfMesh->faces[iFace];
	Point3 pos = fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(0)) * bary[0];
	pos += fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(1)) * bary[1];
	pos += fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(2)) * bary[2];

	pos = pos * fSurfToWorld;

	if( pos.z > fAltProbHi )
		return true;
	if( pos.z < fAltProbLo )
		return true;
	if( pos.z < fAltProbLo + fAltProbTrans )
	{
		float prob = (pos.z - fAltProbLo) / fAltProbTrans;
		return fRand.RandZeroToOne() >= prob;
	}
	if( pos.z > fAltProbHi - fAltProbTrans )
	{
		float prob = (fAltProbHi - pos.z) / fAltProbTrans;
		return fRand.RandZeroToOne() >= prob;
	}

	return false;

}

// If |projGridPt - GridPt| < gridCubeRadius
// and probBitmap->GetPixel(src->UVW(bary)) < RandomZeroToOne()
// Also a generic random factor.
hsBool plDistributor::IProbablyDoIt(int iFace, Point3& del, const Point3& bary) const
{
	if( fRand.RandZeroToOne() >= fOverallProb )
	{
		return false;
	}

	if( (kIsoNone == fIsolation) || (kIsoLow == fIsolation) )
	{
		if( LengthSquared(del) >= fSpacing*fSpacing )
		{
			return false;
		}

		Point3 faceNorm = fSurfMesh->FaceNormal(iFace, FALSE);
		if( DotProd(del, faceNorm) < 0 )
		{
			return false;
		}
	}

	if( IFailsAngProb(iFace, bary) )
	{
		return false;
	}

	if( IFailsAltProb(iFace, bary) )
	{
		return false;
	}

	if( IFailsProbBitmap(iFace, bary) )
	{
		return false;
	}

	return true;
}

Point3 plDistributor::IPerpAxis(const Point3& p) const
{
	const hsScalar kMinLengthSquared = 1.e-1f;

	int minAx = p.MinComponent();
	Point3 ax(0,0,0);
	ax[minAx] = 1.f;

	Point3 perp = p ^ ax;
	if( perp.LengthSquared() < kMinLengthSquared )
	{
		// hmm, think we might be screwed, but this shouldn't happen.
	}

	return perp = perp.FNormalize();
}

// Generate local to world from face info (pos, normal, etc)
Matrix3 plDistributor::IGenerateTransform(int iRepNode, int iFace, const Point3& pt, const Point3& bary) const
{
	const float kMinVecLengthSq = 1.e-6f;
	Matrix3 l2w(true);

	// First, set the scale
	Point3 scale;
	switch( fScaleLock )
	{
	case kLockX | kLockY:
		scale.x = fRand.RandRangeF(fScaleLo.x, fScaleHi.x);
		scale.y = scale.x;
		scale.z = fRand.RandRangeF(fScaleLo.z, fScaleHi.z);
		break;
	case kLockX | kLockY | kLockZ:
		scale.x = fRand.RandRangeF(fScaleLo.x, fScaleHi.x);
		scale.y = scale.z = scale.x;
		break;
	default:
		scale.x = fRand.RandRangeF(fScaleLo.x, fScaleHi.x);
		scale.y = fRand.RandRangeF(fScaleLo.y, fScaleHi.y);
		scale.z = fRand.RandRangeF(fScaleLo.z, fScaleHi.z);
		break;
	}

	l2w.Scale(scale);

	// Next up, get the rotation.
	// First we'll randomly rotate about local Z
	float azimRot = fRand.RandMinusOneToOne() * fAzimuthRange;
	Matrix3 azimMat;
	azimMat.SetRotateZ(azimRot);

	l2w = l2w * azimMat;

	// Now align with the surface.
	// Get the interpolated surface normal.
	Point3 surfNorm = IGetSurfaceNormal(iFace, bary);

	Matrix3 repNodeTM = fRepNodes[iRepNode]->GetNodeTM(TimeValue(0));

	Point3 alignVec = repNodeTM.GetRow(2);
	alignVec = alignVec * fWorldToSurfVec;
	alignVec = FNormalize(alignVec);

	Point3 norm = surfNorm + (alignVec - surfNorm) * fAlignWgt;
	// The norm can come out of this zero length, if the surace normal
	// is directly opposite the "natural" up direction and the weight
	// is 50% (for example). In that case, this is just a bad place
	// to drop this replicant.
	if( norm.LengthSquared() < kMinVecLengthSq )
	{
		l2w.IdentityMatrix();
		return l2w;
	}
	norm = norm.Normalize();

	// Randomize through the cone around that.
	Point3 rndNorm = norm;
	Point3 rndDir = IPerpAxis(norm);
	Point3 rndOut = rndDir ^ norm;
	rndDir *= fRand.RandMinusOneToOne();
	float len = hsSquareRoot(1.f - rndDir.LengthSquared());
	rndOut *= len;
	if( fRand.RandMinusOneToOne() < 0 )
		rndOut *= -1.f;
	Point3 rndPol = rndDir + rndOut;

	float polScale = fRand.RandZeroToOne() * fTanPolarRange;

	// Combine using the bunching factor
	polScale = polScale * (1.f - fPolarBunch) + polScale * polScale * fPolarBunch;

	rndPol *= polScale;
	rndNorm += rndPol;
	norm = rndNorm.Normalize();

	// Have "up" alignment, now just generate random dir vector perpindicular to up
	Point3 dir = repNodeTM.GetRow(1);
	dir = dir * fWorldToSurfVec;
	Point3 out = dir ^ norm;
	if( out.LengthSquared() < kMinVecLengthSq )
	{
		if( fAzimuthRange < hsScalarPI * 0.5f )
		{
			l2w.IdentityMatrix();
			return l2w;
		}
		else
		{
			dir = IPerpAxis(norm);
			out = dir ^ norm;
		}
	}
	out = FNormalize(out);
	dir = norm ^ out;

	// If our "up" direction points into the surface, return an "up" direction
	// tangent to the surface. Also, make the "dir" direction point out from
	// the surface. So if the alignVec/fAlignWgt turns the replicant around
	// to penetrate the surface, it just lies down instead.
	//
	// There's an early out here, for the case where the surface normal is
	// exactly opposed to the destination normal. This usually means the
	// surface normal is directly opposite the alignVec. In that
	// case, we just want to bag it.
	if( DotProd(norm, surfNorm) < 0 )
	{
		dir = surfNorm;
		dir = dir.Normalize();
		out = dir ^ norm;
		if( out.LengthSquared() < kMinVecLengthSq )
		{
			l2w.IdentityMatrix();
			return l2w;
		}
		out = out.Normalize();
		norm = out ^ dir;
	}

	Matrix3 align;
	align.Set(out, dir, norm, Point3(0,0,0));

	l2w = l2w * align;


	// Lastly, set the position.
	Point3 pos = pt;
	const float offset = fRand.RandRangeF(fOffsetMin, fOffsetMax);
	pos += norm * offset;
	l2w.Translate(pos);

	l2w = l2w * fSurfNode->GetObjectTM(TimeValue(0));

	return l2w;
}

int plDistributor::ISelectRepNode() const
{
	if( fRepNodes.Count() < 2 )
		return 0;

	int i = fRand.RandRangeI(0, fRepNodes.Count()-1);

	return i;
}

BOOL plDistributor::ISetupNormals(plMaxNode* node, Mesh* mesh, BOOL radiateNorm) const
{
	const char* dbgNodeName = node->GetName();

	UVVert *normMap = mesh->mapVerts(kNormMapChan);	
	int numNormVerts = mesh->getNumMapVerts(kNormMapChan);
	if( !mesh->mapSupport(kNormMapChan) || !mesh->mapVerts(kNormMapChan) || !mesh->mapFaces(kNormMapChan) )
	{
		mesh->setMapSupport(kNormMapChan);

		mesh->setNumMapVerts(kNormMapChan, mesh->getNumVerts());
		mesh->setNumMapFaces(kNormMapChan, mesh->getNumFaces());
	}

	int i;
	if( radiateNorm )
	{
		Matrix3 otm = node->GetOTM();
		Matrix3 invOtm = Inverse(otm);
		invOtm.SetTrans(Point3(0,0,0));
		invOtm.ValidateFlags();

		for( i = 0; i < mesh->getNumVerts(); i++ )
		{
			Point3 pos = mesh->getVert(i) * otm;
			pos = pos * invOtm;

			mesh->setMapVert(kNormMapChan, i, pos);
		}
	}
	else
	{
		mesh->checkNormals(true);

		for( i = 0; i < mesh->getNumVerts(); i++ )
		{
			Point3 norm = mesh->getNormal(i);

			mesh->setMapVert(kNormMapChan, i, norm);
		}
	}

	TVFace* mapFaces = mesh->mapFaces(kNormMapChan);
	Face* faces = mesh->faces;
	for( i = 0; i < mesh->getNumFaces(); i++ )
	{
		mapFaces[i].setTVerts(faces[i].getVert(0), faces[i].getVert(1), faces[i].getVert(2));
	}

	return true;
}


BOOL plDistributor::ISetupSkinWeights(plMaxNode* node, Mesh* mesh, const Point3& flex) const
{
	const char* dbgNodeName = node->GetName();

	Matrix3 otm = node->GetOTM();

	Box3 bnd = mesh->getBoundingBox() * otm;

	float meshHeight = bnd.Max().z;
	float maxHeight = kMaxHeight;
	if( meshHeight > maxHeight )
		maxHeight = meshHeight;
	float maxNorm = meshHeight / maxHeight;

	float flexibility = flex[0];

	UVVert *wgtMap = mesh->mapVerts(kWgtMapChan);	
	int numWgtVerts = mesh->getNumMapVerts(kWgtMapChan);
	if( !mesh->mapSupport(kWgtMapChan) || !mesh->mapVerts(kWgtMapChan) || !mesh->mapFaces(kWgtMapChan) )
	{
		mesh->setMapSupport(kWgtMapChan);

		mesh->setNumMapVerts(kWgtMapChan, mesh->getNumVerts());
		mesh->setNumMapFaces(kWgtMapChan, mesh->getNumFaces());
	}

	int i;
	for( i = 0; i < mesh->getNumVerts(); i++ )
	{
		Point3 pos = mesh->getVert(i) * otm;
		float wgt = pos.z / meshHeight;
		wgt *= wgt > 0 ? wgt : 0;
		wgt *= maxNorm;
		wgt *= flexibility;

		pos.x = wgt;
		pos.y = wgt;
		pos.z = wgt;

		mesh->setMapVert(kWgtMapChan, i, pos);
	}

	TVFace* mapFaces = mesh->mapFaces(kWgtMapChan);
	Face* faces = mesh->faces;
	for( i = 0; i < mesh->getNumFaces(); i++ )
	{
		mapFaces[i].setTVerts(faces[i].getVert(0), faces[i].getVert(1), faces[i].getVert(2));
	}

	return true;
}

BOOL plDistributor::IDuplicate2Sided(plMaxNode* node, Mesh* mesh) const
{
	Mtl* mtl = node->GetMtl();

	BitArray faces(mesh->getNumFaces());

	int num2Sided = 0;

	int origNumFaces = mesh->getNumFaces();

	int i;
	for( i = 0; i < mesh->getNumFaces(); i++ )
	{
		if( hsMaterialConverter::IsTwoSided(mtl, mesh->faces[i].getMatID()) )
		{
			num2Sided++;
			faces.Set(i);
		}
	}

	if( !num2Sided )
		return false;

	MeshDelta meshDelta(*mesh);
	meshDelta.CloneFaces(*mesh, faces);
	meshDelta.Apply(*mesh);

	BitArray verts(mesh->getNumVerts());
	verts.SetAll();
	const float kWeldThresh = 0.1f;
	meshDelta.WeldByThreshold(*mesh, verts, kWeldThresh);
	meshDelta.Apply(*mesh);

	hsAssert(origNumFaces + num2Sided == mesh->getNumFaces(), "Whoa, lost or gained, unexpected");

	for( i = origNumFaces; i < mesh->getNumFaces(); i++ )
	{
		meshDelta.FlipNormal(*mesh, i);
	}
	meshDelta.Apply(*mesh);

	return true;
}

BOOL plDistributor::IReadyRepNodes(plMeshCacheTab& cache) const
{
	int i;
	for( i = 0; i < fRepNodes.Count(); i++ )
	{
		Mesh* mesh = nil;
		TriObject* obj = nil;
		if( IGetMesh(fRepNodes[i], obj, mesh) )
		{
			plMaxNode* repNode = (plMaxNode*)fRepNodes[i];

			int iCache = cache.Count();
			cache.SetCount(iCache + 1);

			cache[iCache].fMesh = TRACKED_NEW Mesh(*mesh);
			cache[iCache].fFlex = repNode->GetFlexibility();
			
			if( obj )
				obj->DeleteThis();

			BOOL hasXImp = nil != repNode->GetXImposterComp();

			ISetupNormals(repNode, cache[iCache].fMesh, hasXImp);

			ISetupSkinWeights(repNode, cache[iCache].fMesh, cache[iCache].fFlex);
		}
		else
		{
			fRepNodes.Delete(i, 1);
			i--;
		}
	}

	return fRepNodes.Count() > 0;
}

void plDistributor::ClearReplicateNodes() 
{ 
	fRepNodes.ZeroCount(); 
}

void plDistributor::AddReplicateNode(INode* node) 
{ 
	fRepNodes.Append(1, &node); 
}

BOOL plDistributor::IProjectVertex(const Point3& pt, const Point3& dir, float maxDist, Tab<Int32>&faces, Point3& projPt) const
{
	BOOL retVal = false;
	plTriUtils triUtil;
	int i;
	for( i = 0; i < faces.Count(); i++ )
	{
		int iFace = faces[i];
		const hsPoint3& p0 = hsP3(fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(0)) * fSurfToWorld);
		const hsPoint3& p1 = hsP3(fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(1)) * fSurfToWorld);
		const hsPoint3& p2 = hsP3(fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(2)) * fSurfToWorld);

		Point3 plnPt = pt;
		if( triUtil.ProjectOntoPlaneAlongVector(p0, p1, p2, hsV3(dir), hsP3(plnPt)) )
		{
			Point3 bary = plnPt;
			plTriUtils::Bary baryVal = triUtil.ComputeBarycentric(p0, p1, p2, hsP3(plnPt), hsP3(bary));
			if( (plTriUtils::kOutsideTri != baryVal) && (plTriUtils::kDegenerateTri != baryVal) )
			{
				float dist = DotProd((pt - plnPt), dir);
				if( (dist <= maxDist) && (dist >= -maxDist) )
				{
					projPt = plnPt;
					maxDist = dist >= 0 ? dist : -dist;
					retVal = true;
				}
			}
		}

	}
	return retVal;
}

BOOL plDistributor::IConformCheck(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const
{
	Matrix3 OTM = IOTM(iRepNode);
	Mesh* mesh = cache[iRepNode].fMesh;

	Point3 dir = l2w.VectorTransform(Point3(0.f, 0.f, 1.f));
	dir = FNormalize(dir);

	const float kOneOverSqrt2 = 0.707107f;
	Point3 scalePt(kOneOverSqrt2, kOneOverSqrt2, 0.f);
	scalePt = l2w.VectorTransform(scalePt);
	float maxScaledDist = fMaxConform * scalePt.Length();

	Box3 bnd = mesh->getBoundingBox() * OTM;
	bnd = Box3(Point3(bnd.Min().x, bnd.Min().y, -bnd.Max().z), bnd.Max());
	bnd = bnd * l2w;
	Tab<Int32> faces;
	IFindFaceSet(bnd, faces);

	int i;
	for( i = 0; i < mesh->getNumVerts(); i++ )
	{
		Point3 pt = mesh->getVert(i) * OTM;
		pt.z = 0;

		pt = pt * l2w;

		Point3 projPt;
		if( !IProjectVertex(pt, dir, maxScaledDist, faces, projPt) )
			return false;
	}
	return true;
}

BOOL plDistributor::IConformAll(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const
{
	Matrix3 OTM = IOTM(iRepNode);
	Mesh* mesh = cache[iRepNode].fMesh;

	Point3 dir = l2w.VectorTransform(Point3(0.f, 0.f, 1.f));
	dir = FNormalize(dir);

	const float kOneOverSqrt2 = 0.707107f;
	Point3 scalePt(kOneOverSqrt2, kOneOverSqrt2, 0.f);
	scalePt = l2w.VectorTransform(scalePt);
	float maxScaledDist = fMaxConform * scalePt.Length();

	Box3 bnd = mesh->getBoundingBox() * OTM;
	bnd = Box3(Point3(bnd.Min().x, bnd.Min().y, -bnd.Max().z), bnd.Max());
	bnd = bnd * l2w;
	Tab<Int32> faces;
	IFindFaceSet(bnd, faces);

	// l2w, iRepNode, cache, &iCache, maxScaledDist, dir
	iCache = cache.Count();
	cache.SetCount(iCache + 1);
	cache[iCache] = cache[iRepNode];
	cache[iCache].fMesh = TRACKED_NEW Mesh(*mesh);

	mesh = cache[iCache].fMesh;

	Matrix3 v2w = OTM * l2w;
	Matrix3 w2v = Inverse(v2w);

	BOOL retVal = true;
	int i;
	for( i = 0; i < mesh->getNumVerts(); i++ )
	{
		Point3 pt = mesh->getVert(i) * OTM;
		pt.z = 0;

		pt = pt * l2w;

		Point3 projPt;
		if( !IProjectVertex(pt, dir, maxScaledDist, faces, projPt) )
		{
			retVal = false;
			break;
		}

		Point3 del = w2v.VectorTransform(projPt - pt);
		mesh->getVert(i) += del;
	}
	if( !retVal )
	{
//		delete cache[iCache].fMesh;
		delete mesh;
		cache.SetCount(iCache);
		iCache = iRepNode;
	}
	return retVal;
}

BOOL plDistributor::IConformHeight(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const
{
	Matrix3 OTM = IOTM(iRepNode);
	Mesh* mesh = cache[iRepNode].fMesh;

	Point3 dir = l2w.VectorTransform(Point3(0.f, 0.f, 1.f));
	dir = FNormalize(dir);

	const float kOneOverSqrt2 = 0.707107f;
	Point3 scalePt(kOneOverSqrt2, kOneOverSqrt2, 0.f);
	scalePt = l2w.VectorTransform(scalePt);
	float maxScaledDist = fMaxConform * scalePt.Length();

	Box3 bnd = mesh->getBoundingBox() * OTM;
	bnd = Box3(Point3(bnd.Min().x, bnd.Min().y, -bnd.Max().z), bnd.Max());
	bnd = bnd * l2w;
	Tab<Int32> faces;
	IFindFaceSet(bnd, faces);

	// l2w, iRepNode, cache, &iCache, maxScaledDist, dir
	iCache = cache.Count();
	cache.SetCount(iCache + 1);
	cache[iCache] = cache[iRepNode];
	cache[iCache].fMesh = TRACKED_NEW Mesh(*mesh);

	mesh = cache[iCache].fMesh;

	Matrix3 v2w = OTM * l2w;
	Matrix3 w2v = Inverse(v2w);


	float maxZ = (mesh->getBoundingBox() * OTM).Max().z;

	BOOL retVal = true;
	int i;
	for( i = 0; i < mesh->getNumVerts(); i++ )
	{
		Point3 pt = mesh->getVert(i) * OTM;

		float conScale = 1.f - pt.z / maxZ;
		if( conScale > 1.f )
			conScale = 1.f;
		else if( conScale < 0 )
			conScale = 0;

		pt.z = 0;

		pt = pt * l2w;

		Point3 projPt;
		if( !IProjectVertex(pt, dir, maxScaledDist, faces, projPt) )
		{
			retVal = false;
			break;
		}

		Point3 del = w2v.VectorTransform(projPt - pt);
		del *= conScale;
		mesh->getVert(i) += del;
	}
	if( !retVal )
	{
		delete cache[iCache].fMesh;
		cache.SetCount(iCache);
		iCache = iRepNode;
	}
	return retVal;
}

BOOL plDistributor::IConformBase(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const
{
	Matrix3 OTM = IOTM(iRepNode);
	Mesh* mesh = cache[iRepNode].fMesh;

	Point3 dir = l2w.VectorTransform(Point3(0.f, 0.f, 1.f));
	dir = FNormalize(dir);

	const float kOneOverSqrt2 = 0.707107f;
	Point3 scalePt(kOneOverSqrt2, kOneOverSqrt2, 0.f);
	scalePt = l2w.VectorTransform(scalePt);
	float maxScaledDist = fMaxConform * scalePt.Length();

	Box3 bnd = mesh->getBoundingBox() * OTM;
	bnd = Box3(Point3(bnd.Min().x, bnd.Min().y, -bnd.Max().z), bnd.Max());
	bnd = bnd * l2w;
	Tab<Int32> faces;
	IFindFaceSet(bnd, faces);

	// l2w, iRepNode, cache, &iCache, maxScaledDist, dir
	iCache = cache.Count();
	cache.SetCount(iCache + 1);
	cache[iCache] = cache[iRepNode];
	cache[iCache].fMesh = TRACKED_NEW Mesh(*mesh);

	mesh = cache[iCache].fMesh;

	Matrix3 v2w = OTM * l2w;
	Matrix3 w2v = Inverse(v2w);


	float maxZ = (mesh->getBoundingBox() * OTM).Max().z;

	BOOL retVal = true;
	int i;
	for( i = 0; i < mesh->getNumVerts(); i++ )
	{
		Point3 pt = mesh->getVert(i) * OTM;

		const float kMaxConformZ = 0.5f;
		if( pt.z < kMaxConformZ )
		{
			pt.z = 0;

			pt = pt * l2w;

			Point3 projPt;
			if( !IProjectVertex(pt, dir, maxScaledDist, faces, projPt) )
			{
				retVal = false;
				break;
			}

			Point3 del = w2v.VectorTransform(projPt - pt);
			mesh->getVert(i) += del;
		}
	}
	if( !retVal )
	{
		delete cache[iCache].fMesh;
		cache.SetCount(iCache);
		iCache = iRepNode;
	}
	return retVal;
}

BOOL plDistributor::IConform(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const
{
	iCache = iRepNode;
	switch( fConformity )
	{
	case kConformAll:
		return IConformAll(l2w, iRepNode, cache, iCache);
	case kConformHeight:
		return IConformHeight(l2w, iRepNode, cache, iCache);
	case kConformCheck:
		return IConformCheck(l2w, iRepNode, cache, iCache);
	case kConformBase:
		return IConformBase(l2w, iRepNode, cache, iCache);
	}
	return true;
}


// Clone the replicant and set its transform appropriately. Should set the plMaxNode property
// that the node's spans are collapseable? No, we'll make a separate component for that.
void plDistributor::IReplicate(Matrix3& l2w, int iRepNode, plDistribInstTab& reps, plMeshCache& mCache) const
{
	INode* repNode = fRepNodes[iRepNode];
	plDistribInstance inst;
	inst.fNode = repNode;
	inst.fNodeTM = l2w;
	inst.fObjectTM = repNode->GetObjectTM(TimeValue(0)) * Inverse(repNode->GetNodeTM(TimeValue(0))) * l2w;

	inst.fMesh = mCache.fMesh;

	inst.fFlex = mCache.fFlex;

	inst.fFade = fFade;

	inst.fBone = fBone;

	inst.fRigid = fRigid;

	reps.Append(1, &inst);
}

void plDistributor::IDistributeOverFace(int iFace, plDistribInstTab& reps, plMeshCacheTab& cache) const
{
	Point3 p0 = fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(0));
	Point3 p1 = fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(1));
	Point3 p2 = fSurfMesh->getVert(fSurfMesh->faces[iFace].getVert(2));

	Box3 grid = ISetupGrid(p0, p1, p2);

	hsScalar delta = fSpacing;

	hsScalar x, y, z;
	for( x = grid.Min().x; x < grid.Max().x; x += delta )
	{
		for( y = grid.Min().y; y < grid.Max().y; y += delta )
		{
			for( z = grid.Min().z; z < grid.Max().z; z += delta )
			{
				Point3 pt(x, y, z);

				pt = IPerturbPoint(pt);

				Point3 plnPt = pt;
				Point3 bary;

				// Get Barycentric coord of projection of grid pt onto face
				plTriUtils triUtil;
				plTriUtils::Bary baryVal = triUtil.ComputeBarycentricProjection(hsP3(p0), hsP3(p1), hsP3(p2), hsP3(plnPt), hsP3(bary));
				if( !(baryVal & (plTriUtils::kOutsideTri | plTriUtils::kDegenerateTri)) )
				{
					int iRepNode = ISelectRepNode();

					Matrix3 l2w = IGenerateTransform(iRepNode, iFace, plnPt, bary);

					// l2w as ident means this position turned out to be not so good afterall.
					if( l2w.IsIdentity() )
					{
						continue;
					}

					Box3 clearBox;
					if( !ISpaceClear(iRepNode, l2w, clearBox, cache) )
					{
						continue;
					}

					int iCacheNode = iRepNode;
					if( !IConform(l2w, iRepNode, cache, iCacheNode) )
					{
						continue;
					}

					// If |projGridPt - GridPt| < gridCubeRadius
					// and probBitmap->GetPixel(src->UVW(bary)) < RandomZeroToOne()
					// Also a generic random factor.
					if( IProbablyDoIt(iFace, pt - plnPt, bary) )
					{
						IReplicate(l2w, iRepNode, reps, cache[iCacheNode]);

						IReserveSpace(clearBox);
					}
				}
			}
		}
	}
}

Matrix3 plDistributor::IInvOTM(int iRepNode) const
{
	// objectTM = otm * nodeTM
	// invOTM * objectTM = nodeTM
	// invOTM = nodeTM * invObjectTM
	INode* repNode = fRepNodes[iRepNode];
	Matrix3 objectTM = repNode->GetObjectTM(TimeValue(0));
	Matrix3 nodeTM = repNode->GetNodeTM(TimeValue(0));
	Matrix3 invOTM = nodeTM * Inverse(objectTM);
	return invOTM;
}

Matrix3 plDistributor::IOTM(int iRepNode) const
{
	// objectTM = otm * nodeTM
	// objectTM * Inverse(nodeTM) = otm
	INode* repNode = fRepNodes[iRepNode];
	Matrix3 objectTM = repNode->GetObjectTM(TimeValue(0));
	Matrix3 nodeTM = repNode->GetNodeTM(TimeValue(0));
	Matrix3 OTM = objectTM * Inverse(nodeTM);
	return OTM;
}

BOOL plDistributor::ISpaceClear(int iRepNode, const Matrix3& l2w, Box3& clearBox, plMeshCacheTab& cache) const
{
	if( !fDistTree )
		return true;

	// If we have high isolation,
	//		clearBox = Box3(Point3(-fSpacing*0.5f, -fSpacing*0.5f, 0), Point3(fSpacing*0.5f, fSpacing*0.5f, fSpacing));
	// Else if we have medium isolation
	//		clearBox = cache[iRepNode]->getBoundingBox(); // The mesh's bounds
	// Else if we have low isolation or None isolation
	//		clearBox = Box3(Point3(-kSmallSpace, -kSmallSpace, 0), Point3(kSmallSpace, kSmallSpace, kSmallSpace)); // kSmallSpace ~= 0.5f or one or something

	// We want to set up the box (for high, low and none) in Post OTM space. So instead of multiplying
	// by l2w, we want to multiply box = box * invOTM * l2w (because l2w already has OTM folded in). When using
	// the mesh bounds (Medium), l2w is the right transform.
	// objectTM = otm * nodeTM
	// invOTM * objectTM = nodeTM
	// invOTM = nodeTM * invObjectTM
	const float kSmallSpace = 0.5f;
	switch( fIsolation )
	{
	case kIsoHigh:
		{
			INode* repNode = fRepNodes[iRepNode];
			Matrix3 objectTM = repNode->GetObjectTM(TimeValue(0));
			Matrix3 nodeTM = repNode->GetNodeTM(TimeValue(0));
			Matrix3 invOTM = nodeTM * Inverse(objectTM);
			clearBox = Box3(Point3(-fSpacing*0.5f, -fSpacing*0.5f, 0.f), Point3(fSpacing*0.5f, fSpacing*0.5f, fSpacing));
			clearBox = clearBox * invOTM;
		}
		break;
	case kIsoMedium:
		clearBox = cache[iRepNode].fMesh->getBoundingBox(); // The mesh's bounds
		break;
	case kIsoLow:
	case kIsoNone:
	default:
		{
			INode* repNode = fRepNodes[iRepNode];
			Matrix3 objectTM = repNode->GetObjectTM(TimeValue(0));
			Matrix3 nodeTM = repNode->GetNodeTM(TimeValue(0));
			Matrix3 invOTM = nodeTM * Inverse(objectTM);
			clearBox = Box3(Point3(-kSmallSpace, -kSmallSpace, 0.f), Point3(kSmallSpace, kSmallSpace, kSmallSpace)); 
			clearBox = clearBox * invOTM;
		}
		break;
	}

	clearBox = clearBox * l2w;

	return fDistTree->BoxClear(clearBox, fFade);
}

void plDistributor::IReserveSpace(const Box3& clearBox) const
{
	// if isolation isn't None, add the box.
	if( fDistTree && (fIsolation != kIsoNone) )
		fDistTree->AddBox(clearBox, fFade);
}

UInt32 plDistributor::GetRandSeed() const
{
	return fRand.GetSeed();
}

void plDistributor::SetRandSeed(int seed)
{
	fRand.SetSeed(seed);
}

void plDistributor::SetPolarRange(float deg) 
{ 
	fPolarRange = hsScalarDegToRad(deg); 
	fTanPolarRange = tan(fPolarRange); 
}

void plDistributor::SetProbabilityBitmapTex(BitmapTex* t)
{
	fProbLayerTex = nil;
	fProbBitmapTex = t;
}

void plDistributor::SetProbabilityLayerTex(plLayerTex* t)
{
	fProbBitmapTex = nil;
	fProbLayerTex = t;
}


