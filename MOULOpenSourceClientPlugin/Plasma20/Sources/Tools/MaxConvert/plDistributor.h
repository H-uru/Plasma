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

#ifndef plDistributor_inc
#define plDistributor_inc

#include "../plMath/plRandom.h"
#include "plDistTree.h"

class INode;
class Mesh;
class TriObject;

class BitmapTex;
class plLayerTex;
class plExportProgressBar;
class plMaxNode;
class plDistTree;

// To Use:
//
// First you must set the interface
// Second, you must set the node(s) to be replicated.
//		If multiple replicants, they will be randomly selected at each plant site.
// Set the spacing. Default is 10 or so, but shouldn't be counted on.
// Set the spacing range. Set it to less than half the spacing to prevent
//		replicants piling up on each other.
// Rather than increasing the spacing range (which causes pileup), you can increase
//		the randomness of placement with the overall probability factor. Reducing it
//		means fewer grid points will be used, so placement seems more random
//
// Options:
//		PolarRange - defines the cone about the surface normal to randomly fill for up direction
//		PolarBunch - defines tendency to bunch around the center of PolarRange cone,
//			with zero being uniform distribution, and one being all on cone axis.
//		AlignmentVector - defines a world space preferred up orientation.
//		AlignmentWeight - blend factor between random normal (from polar stuff) and
//			above AlignmentVector.
//		ScaleRange - defines range of non-uniform scale to apply to each replicant.
//		ProbabilityTexmap - A BitmapTex which maps the probability of an instance
//			taking root at any point on the surface. Various interpretations of the
//			map available (see ProbabilityChan). If the surface mesh doesn't have
//			the appropriate uvw mapping for the map, the map will be ignored.
//		ProbabilityChan - defines interpretation of texel value from ProbTexmap
//			into a probability. ColorChan enum types are pretty self-explanatory.
//			In all cases, a higher texel channel value means more likely of a
//			replicant taking root there.
//
// Finally, call Distrubute() with a node containing a surface to be populated.
//
// Modifying. Were actually creating the new node instances, but then we just
// want to re-pack them into clusters anyway. So all we really need is to
// know which template (fRepNodes) and what transform to use for it. We can
// use that to make our clusters, without bogging Max down with adding and
// deleting a gazillion INodes.

class plDistribInstance
{
public:
	INode*			fNode;
	Matrix3			fNodeTM;
	Matrix3			fObjectTM;

	INode*			fBone;
	BOOL			fRigid;

	Box3			fFade;

	Point3			fFlex;

	Mesh*			fMesh;
};

class plDistribInstTab : public Tab<plDistribInstance>
{
};

class plMeshCache
{
public:
	Mesh*		fMesh;
	Point3		fFlex;

	plMeshCache() {}
};

class plMeshCacheTab : public Tab<plMeshCache>
{
};

class plDistributor
{
public:
	enum ColorChan
	{
		kRed						= 0x1,
		kGreen						= 0x2,
		kBlue						= 0x4,
		kAlpha						= 0x8,
		kAverageRedGreen			= kRed | kGreen,
		kAverageRedGreenTimesAlpha	= kRed | kGreen | kAlpha,
		kAverage					= kRed | kGreen | kBlue,
		kAverageTimesAlpha			= kAverage | kAlpha,
		kMax						= 0x100,
		kMaxColor					= kMax | kRed | kGreen | kBlue,
		kMaxColorTimesAlpha			= kMaxColor | kAlpha,
		kMaxRedGreen				= kMax | kRed | kGreen,
		kMaxRedGreenTimesAlpha		= kMaxRedGreen | kAlpha
	};
	enum 
	{
		kLockNone		= 0x0,
		kLockX			= 0x1,
		kLockY			= 0x2,
		kLockZ			= 0x4
	};
	enum
	{
		kWgtMapChan			= 66,
		kNormMapChan		= 67
	};
	enum IsoType
	{
		kIsoNone,
		kIsoLow,
		kIsoMedium,
		kIsoHigh,

		kIsoMax = kIsoHigh
	};
	enum ConformType
	{
		kConformNone,
		kConformAll,
		kConformHeight,
		kConformCheck,
		kConformBase
	};
protected:
	mutable INode*				fSurfNode;
	mutable Mesh*				fSurfMesh;
	mutable TriObject*			fSurfObjToDelete;

	mutable INodeTab			fRepNodes;

	mutable plDistTree*			fDistTree;
	mutable plDistTree			fMeshTree;

	Interface*					fInterface;

	IsoType				fIsolation;
	ConformType			fConformity;
	BOOL				fFaceNormals;
	
	float				fSpacing;
	float				fRndPosRadius;

	Point3				fAlignVec;
	float				fAlignWgt;

	float				fOffsetMin;
	float				fOffsetMax;

	Point3				fAngProbVec;
	float				fAngProbLo;
	float				fAngProbHi;
	float				fAngProbTrans;

	float				fAltProbLo;
	float				fAltProbHi;
	float				fAltProbTrans;

	float				fPolarRange;
	float				fTanPolarRange;
	float				fAzimuthRange;

	float				fOverallProb;

	float				fPolarBunch;

	ULONG				fScaleLock;				
	Point3				fScaleLo;
	Point3				fScaleHi;

	BitmapTex*			fProbBitmapTex;
	plLayerTex*			fProbLayerTex;
	ColorChan			fProbColorChan;

	float				fProbRemapFromLo;
	float				fProbRemapFromHi;
	float				fProbRemapToLo;
	float				fProbRemapToHi;

	float				fMaxConform; // in feet

	Box3				fFade;
	INode*				fBone;
	BOOL				fRigid;

	// Temps used during processing.
	mutable Matrix3		fSurfToWorld;
	mutable Matrix3		fWorldToSurf;
	mutable Matrix3		fSurfToWorldVec;
	mutable Matrix3		fWorldToSurfVec;
	mutable	Point3		fSurfAlignVec;
	mutable Point3		fSurfAngProbVec;
	mutable plRandom	fRand;
	mutable float		fCosAngProbHi;
	mutable float		fCosAngProbHiTrans;
	mutable float		fCosAngProbLo;
	mutable float		fCosAngProbLoTrans;

	void				ISetAngProbCosines() const;
	BOOL				ISetSurfaceNode(INode* node) const;
	BOOL				IGetMesh(INode* node, TriObject*& objToDelete, Mesh*& retMesh) const;

	BOOL				INeedMeshTree() const;
	void				IMakeMeshTree() const;
	void				IFindFaceSet(const Box3& box, Tab<Int32>& faces) const;
	BOOL				IProjectVertex(const Point3& pt, const Point3& dir, float maxDist, Tab<Int32>&faces, Point3& projPt) const;
	BOOL				IConform(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const;
	BOOL				IConformHeight(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const;
	BOOL				IConformAll(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const;
	BOOL				IConformCheck(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const;
	BOOL				IConformBase(Matrix3& l2w, int iRepNode, plMeshCacheTab& cache, int& iCache) const;

	Matrix3				IOTM(int iRepNode) const;
	Matrix3				IInvOTM(int iRepNode) const;

	Matrix3				IGenerateTransform(int iRepNode, int iFace, const Point3& pt, const Point3& bary) const;
	hsBool				IProbablyDoIt(int iFace, Point3& del, const Point3& bary) const;
	hsBool				IFailsAltProb(int iFace, const Point3& bary) const;
	hsBool				IFailsAngProb(int iFace, const Point3& bary) const;
	hsBool				IFailsProbBitmap(int iFace, const Point3& bary) const;
	Box3				ISetupGrid(const Point3& p0, const Point3& p1, const Point3& p2) const;
	Point3&				IPerturbPoint(Point3& pt) const;
	int 				ISelectRepNode() const;
	Point3				IPerpAxis(const Point3& p) const;
	Point3				IGetSurfaceNormal(int iFace, const Point3& bary) const;

	BOOL				ISpaceClear(int iRepNode, const Matrix3& l2w, Box3& clearBox, plMeshCacheTab& cache) const;
	void				IReserveSpace(const Box3& clearBox) const;

	void				IReplicate(Matrix3& l2w, int iRep, plDistribInstTab& reps, plMeshCache& mCache) const;
	void				IDistributeOverFace(int iFace, plDistribInstTab& reps, plMeshCacheTab& cache) const;
	BOOL				IDistributeOverMesh(plDistribInstTab& reps, plMeshCacheTab& cache, plExportProgressBar& bar) const;

	void				IClear();
	BOOL				IValidateSettings(INode* surfNode, plMeshCacheTab& cache) const;

	BOOL				ISetupNormals(plMaxNode* node, Mesh* mesh, BOOL radiateNorm) const;
	BOOL				IDuplicate2Sided(plMaxNode* node, Mesh* mesh) const;
	BOOL				ISetupSkinWeights(plMaxNode* node, Mesh* mesh, const Point3& flex) const;
	BOOL				IReadyRepNodes(plMeshCacheTab& cache) const;
public:

	plDistributor();
	virtual ~plDistributor();

	void				SetTheInterface(Interface* i) { fInterface = i; }
	Interface*			GetTheInterface() const { return fInterface; }

	BOOL				Distribute(INode* surfNode, plDistribInstTab& replicants, plMeshCacheTab& cache, plExportProgressBar& bar) const;

	void				Reset();
	UInt32				GetRandSeed() const;
	void				SetRandSeed(int seed);

	void				ClearReplicateNodes();
	void				AddReplicateNode(INode* node);
	int					GetNumReplicateNodes() const { return fRepNodes.Count(); }
	INode*				GetReplicateNode(int i) const { return fRepNodes[i]; }

	INode*				GetSurfaceNode() const { return fSurfNode; }

	void				SetSpacing(float f) { fSpacing = f; }
	float				GetSpacing() const { return fSpacing; }

	void				SetSpacingRange(float f) { fRndPosRadius = f; }
	float				GetSpacingRange() const { return fRndPosRadius; }

	void				SetAlignmentVec(const Point3& v) { fAlignVec = v; }
	Point3				GetAlignmentVec() const { return fAlignVec; }

	void				SetAlignmentWeight(float w) { fAlignWgt = w / 100.f; }
	float				GetAlignmentWeight() const { return fAlignWgt * 100.f; }

	void				SetPolarRange(float deg);
	float				GetPolarRange() const { return hsScalarRadToDeg(fPolarRange); }

	void				SetAzimuthRange(float deg) { fAzimuthRange = hsScalarDegToRad(deg); }
	float				GetAzimuthRange() const { return hsScalarRadToDeg(fAzimuthRange); }

	void				SetOverallProb(float percent) { fOverallProb = percent/100.f; }
	float				GetOverallProb() const { return fOverallProb * 100.f; }

	void				SetAngleProbVec(const Point3& v) { fAngProbVec = v; }
	Point3				GetAngleProbVec() const { return fAngProbVec; }

	void				SetAngleProbHi(float deg) { fAngProbHi = deg; }
	float				GetAngleProbHi() const { return fAngProbHi; }

	void				SetAngleProbLo(float deg) { fAngProbLo = deg; }
	float				GetAngleProbLo() const { return fAngProbLo; }

	void				SetAngleProbTransition(float deg) { fAngProbTrans = deg; }
	float				GetAngleProbTransition() const { return fAngProbTrans; }

	void				SetMinAltitude(float feet) { fAltProbLo = feet; }
	float				GetMinAltitude() const { return fAltProbLo; }

	void				SetMaxAltitude(float feet) { fAltProbHi = feet; }
	float				GetMaxAltitude() const { return fAltProbHi; }

	void				SetAltitudeTransition(float feet) { fAltProbTrans = feet; }
	float				GetAltitudeTransition() const { return fAltProbTrans; }

	void				SetPolarBunch(float b) { fPolarBunch = b/100.f; }
	float				GetPolarBunch() const { return fPolarBunch * 100.f; }

	void				SetScaleRange(const Point3& lo, const Point3& hi) { fScaleLo = lo; fScaleHi = hi; }
	Point3				GetScaleRangeMin() const { return fScaleLo; }
	Point3				GetScaleRangeMax() const { return fScaleHi; }

	void				SetProbabilityBitmapTex(BitmapTex* t);
	BitmapTex*			GetProbabilityBitmapTex() const { return fProbBitmapTex; }

	void				SetProbabilityLayerTex(plLayerTex* t);
	plLayerTex*			GetProbabilityLayerTex() const { return fProbLayerTex; }

	void				SetProbabilityChan(ColorChan c) { fProbColorChan = c; }
	ColorChan			GetProbabilityChan() const { return fProbColorChan; }

	void				SetProbabilityRemapFromLo(float f) { fProbRemapFromLo = f / 255.f; }
	float				GetProbabilityRemapFromLo() const { return fProbRemapFromLo * 255.f; }

	void				SetProbabilityRemapFromHi(float f) { fProbRemapFromHi = f / 255.f; }
	float				GetProbabilityRemapFromHi() const { return fProbRemapFromHi * 255.f; }

	void				SetProbabilityRemapToLo(float f) { fProbRemapToLo = f / 255.f; }
	float				GetProbabilityRemapToLo() const { return fProbRemapToLo * 255.f; }

	void				SetProbabilityRemapToHi(float f) { fProbRemapToHi = f / 255.f; }
	float				GetProbabilityRemapToHi() const { return fProbRemapToHi * 255.f; }

	// We don't really know what fades are, they're just something we're handed that
	// we stamp on every distribInstance we generate. See plDistribComponent.h.
	void				SetFade(const Box3& fade) { fFade = fade; }
	Box3				GetFade() const { return fFade; }

	void				SetBone(INode* b) { fBone = b; }
	INode*				GetBone() const { return fBone; }

	void				SetRigid(BOOL b) { fRigid = b; }
	BOOL				GetRigid() const { return fRigid; }

	void				SetScaleLock(ULONG f) { fScaleLock = f; }
	ULONG				GetScaleLock() const { return fScaleLock; }

	void				SetDistTree(plDistTree* dt) { fDistTree = dt; }
	plDistTree*			GetDistTree() const { return fDistTree; }

	void				SetIsolation(IsoType t) { fIsolation = t; }
	IsoType				GetIsolation() const { return fIsolation; }

	void				SetConformity(ConformType t) { fConformity = t; }
	ConformType			GetConformity() const { return fConformity; }

	void				SetMaxConform(float feet) { fMaxConform = feet; }
	float				GetMaxConform() const { return fMaxConform; }

	void				SetMinOffset(float feet) { fOffsetMin = feet; }
	float				GetMinOffset() const { return fOffsetMin; }

	void				SetMaxOffset(float feet) { fOffsetMax = feet; }
	float				GetMaxOffset() const { return fOffsetMax; }

	void				SetFaceNormals(BOOL on=true) { fFaceNormals = on; }
	BOOL				GetFaceNormals() const { return fFaceNormals; }
};

#endif // plDistributor_inc
