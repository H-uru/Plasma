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
#include "plPhysXCooking.h"
#include "hsGeometry3.h"
#include "../plPhysX/plSimulationMgr.h"
#include "../plPhysX/plPXStream.h"
#include "../plPhysX/plPXConvert.h"
#include "hsSTLStream.h"

#include "Nx.h"
#include "NxStream.h"
#include "NxPhysics.h"
#include "NxCooking.h"
#include "NxPlane.h"
#include "NxUtilLib.h"
#include "NxMat33.h"
bool plPhysXCooking::fSkipErrors = false;

NxUtilLib* plPhysXCooking::fUtilLib =nil;
//assumes that the Vectors are normalized
hsBool ThreePlaneIntersect(const NxVec3& norm0, const NxVec3& point0, 
						 const NxVec3& norm1, const NxVec3& point1, 
						 const NxVec3& norm2, const NxVec3& point2, NxVec3& loc)
{
	//need to make sure these planes aren't parallel
	hsBool suc=0;
	NxVec3 cross=norm1.cross( norm2);
	hsScalar denom=norm0.dot(cross);
	if(abs(denom)<0.0001) return 0;//basically paralell
	// if we are here there must be	a point in 3 space
	try{
		hsScalar d1,d2,d3;
		d1=norm0.dot(point0);
		d2=norm1.dot(point1);
		d3=norm2.dot(point2);
		NxVec3 n1Xn2=norm1.cross(norm2);
		NxVec3 n2Xn0=norm2.cross(norm0);
		NxVec3 n0Xn1=norm0.cross(norm1);
		NxVec3 pos=(d1*n1Xn2+ d2*n2Xn0  + d3*n0Xn1)/(denom);
		loc.x=pos.x;
		loc.y=pos.y;
		loc.z=pos.z;
		suc= 1;
	}
	catch(...)
	{
		suc=0;
	}

	return suc;

}

void plPhysXCooking::Init()
{
	NxInitCooking();
	NxUtilLib* fUtilLib=NxGetUtilLib();
	NxCookingParams parms=NxGetCookingParams();
	parms.skinWidth=.05;
	NxSetCookingParams(parms);

}


void plPhysXCooking::Shutdown()
{
	NxCloseCooking();
	fUtilLib=nil;
	fSkipErrors = false;
}

hsVectorStream* plPhysXCooking::CookTrimesh(int nVerts, hsPoint3* verts, int nFaces, UInt16* faces)
{
	NxTriangleMeshDesc triDesc;
	triDesc.numVertices			= nVerts;
	triDesc.pointStrideBytes	= sizeof(hsPoint3);
	triDesc.points				= verts;
	triDesc.numTriangles		= nFaces;	
	triDesc.triangleStrideBytes	= sizeof(UInt16) * 3;
	triDesc.triangles			= faces;
	triDesc.flags				= NX_MF_16_BIT_INDICES;

	hsVectorStream* ram = TRACKED_NEW hsVectorStream;
	plPXStream buf(ram);
	bool status = NxCookTriangleMesh(triDesc, buf);
	hsAssert(status, "Trimesh failed to cook");

	if (status)
	{
		ram->Rewind();
		return ram;
	}
	else
	{
		delete ram;
		return nil;
	}
}

bool plPhysXCooking::IsPointInsideHull(hsPlane3* hull, int nPlanes, const hsPoint3& pos)
{
	int i;
	for( i = 0; i < nPlanes; i++ )
	{
		// add a fudge to the point so not to trip on the ever so slightly concave
		// ... so pull the point out in the direction of the normal of the plane we are testing.
		hsPoint3 fudgepos = pos + (hull[i].GetNormal()*0.005f);
		if (!ITestPlane(fudgepos, hull[i]))
			return false;
	}
	return true;
}

bool plPhysXCooking::TestIfConvex(NxConvexMesh* convexMesh, int nVerts, hsPoint3* verts)
{
	bool retVal = true;

	// build planes from the convex mesh
	NxConvexMeshDesc desc;
	convexMesh->saveToDesc(desc);

	hsPlane3* planes = TRACKED_NEW hsPlane3[desc.numTriangles];

	int i;
	for ( i = 0; i < desc.numTriangles; i++)
	{
		UInt32* triangle = (UInt32*)(((char*)desc.triangles) + desc.triangleStrideBytes*i);
		float* vertex1 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[0]);
		float* vertex2 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[1]);
		float* vertex3 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[2]);
		hsPoint3 pt1(vertex1[0],vertex1[1],vertex1[2]);
		hsPoint3 pt2(vertex2[0],vertex2[1],vertex2[2]);
		hsPoint3 pt3(vertex3[0],vertex3[1],vertex3[2]);

		planes[i] = hsPlane3(&pt1,&pt2,&pt3);
	}
	// now see if any of the points from the mesh are inside the hull
	for (int j=0; j<nVerts && retVal; j++)
	{
		if ( IsPointInsideHull(planes,desc.numTriangles,verts[j]) )
			retVal = false;
	}

	delete [] planes;

	return retVal;
}


hsVectorStream* plPhysXCooking::CookHull(int nVerts, hsPoint3* verts, bool inflate)
{
	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices          = nVerts;
	convexDesc.pointStrideBytes     = sizeof(hsPoint3);
	convexDesc.points               = verts;
	convexDesc.flags                = NX_CF_COMPUTE_CONVEX;

	if(inflate) 
	{

		convexDesc.flags|= NX_CF_INFLATE_CONVEX ;
	}
	hsVectorStream* ram = TRACKED_NEW hsVectorStream;
	plPXStream buf(ram);
	bool status = NxCookConvexMesh(convexDesc, buf);
	hsAssert(status, "Convex mesh failed to cook");

	if (status)
	{
		ram->Rewind();
		return ram;
	}
	else
	{
		delete ram;
		return nil;
	}
}
/*
NxTriangleMesh* ReadExplicit(hsStream* stream)
{
	const int nVertices = stream->ReadSwap32();
	hsPoint3* pVertices = TRACKED_NEW hsPoint3[nVertices];
	stream->ReadSwapScalar(nVertices*3, (float*)pVertices);

	const int nFaces = stream->ReadSwap32();
	unsigned short* pTriangles = TRACKED_NEW unsigned short[nFaces * 3];
	stream->ReadSwap16(nFaces * 3, pTriangles);

	NxTriangleMeshDesc triDesc;
	triDesc.numVertices			= nVertices;
	triDesc.pointStrideBytes	= sizeof(hsPoint3);
	triDesc.points				= pVertices;
	triDesc.numTriangles		= nFaces;	
	triDesc.triangleStrideBytes	= sizeof(UInt16) * 3;
	triDesc.triangles			= pTriangles;
	triDesc.flags				= NX_MF_16_BIT_INDICES;// | NX_MF_FLIPNORMALS;

	hsRAMStream ram;
	plNxStream buf(&ram);
	NxInitCooking();
	bool status = NxCookTriangleMesh(triDesc, buf);
	hsAssert(status, "Trimesh failed to cook");
	NxCloseCooking();

	delete[] pVertices;
	delete[] pTriangles;

	if (status)
	{
		ram.Rewind();
		return plSimulationMgr::GetInstance()->GetSDK()->createTriangleMesh(buf);
	}

	return nil;
}

NxConvexMesh* ReadConvexHull(hsStream* stream)
{
	const int nVertices = stream->ReadSwap32();
	hsPoint3* pVertices = TRACKED_NEW hsPoint3[nVertices];
	stream->ReadSwapScalar(nVertices*3, (float*)pVertices);

	NxConvexMeshDesc convexDesc;
	convexDesc.numVertices          = nVertices;
	convexDesc.pointStrideBytes     = sizeof(hsPoint3);
	convexDesc.points               = pVertices;
	convexDesc.flags                = NX_CF_COMPUTE_CONVEX;

	hsRAMStream ram;
	plNxStream buf(&ram);
	NxInitCooking();
	bool status = NxCookConvexMesh(convexDesc, buf);
	hsAssert(status, "Convex mesh failed to cook");
	NxCloseCooking();

	delete[] pVertices;

	if (status)
	{
		ram.Rewind();
		return plSimulationMgr::GetInstance()->GetSDK()->createConvexMesh(buf);
	}

	return nil;
}

void ReadBoxFromHull(hsStream* stream, NxBoxShapeDesc& box)
{
	const int nVertices = stream->ReadSwap32();
	hsPoint3* pVertices = TRACKED_NEW hsPoint3[nVertices];
	stream->ReadSwapScalar(nVertices*3, (float*)pVertices);

	hsScalar minX, minY, minZ, maxX, maxY, maxZ;
	minX = minY = minZ = FLT_MAX;
	maxX = maxY = maxZ = -FLT_MAX;
	for (int i = 0; i < nVertices; i++)
	{
		hsPoint3& vec = pVertices[i];
		minX = hsMinimum(minX, vec.fX);
		minY = hsMinimum(minY, vec.fY);
		minZ = hsMinimum(minZ, vec.fZ);
		maxX = hsMaximum(maxX, vec.fX);
		maxY = hsMaximum(maxY, vec.fY);
		maxZ = hsMaximum(maxZ, vec.fZ);
	}

	delete[] pVertices;

	float xWidth = maxX - minX;
	float yWidth = maxY - minY;
	float zWidth = maxZ - minZ;
	box.dimensions.x = xWidth / 2;
	box.dimensions.y = yWidth / 2;
	box.dimensions.z = zWidth / 2;

// 	hsMatrix44 mat;
// 	box.localPose.getRowMajor44(&mat.fMap[0][0]);
// 	hsPoint3 trans(minX + (xWidth / 2), minY + (yWidth / 2), minY + (yWidth / 2));
// 	mat.SetTranslate(&trans);
// 	box.localPose.setRowMajor44(&mat.fMap[0][0]);
}
*/
hsBool ProjectPointOnToPlane(const hsVector3& planeNormal,hsScalar& d0, 
		const hsVector3 pointToProject, hsPoint3& res)
{

	NxVec3 vec=plPXConvert::Vector(planeNormal);
	NxVec3 orig,projected;
	orig=plPXConvert::Vector(pointToProject);
	NxPlane* pl=new NxPlane(vec,d0);
	projected=pl->project(orig);
	res.fX=projected.x;
	res.fY=projected.y;
	res.fZ=projected.z;
	return 1;
}
void plPhysXCooking::PCA(const NxVec3* points,int numPoints, NxMat33& out)
{
	NxVec3 mean(0.f,0.f,0.f);
	hsScalar Cov[3][3];
	memset(Cov,0,9* sizeof hsScalar);
	for(int i=0; i<numPoints;i++)
	{
		mean+=points[i];
	}
	mean=mean/(hsScalar)numPoints;
	for(int i=0;i<numPoints;i++)
	{
		Cov[0][0]+=pow(points[i].x-mean.x ,2.0f)/(hsScalar)(numPoints);
		Cov[1][1]+=pow(points[i].y-mean.y ,2.0f)/(hsScalar)(numPoints);
		Cov[2][2]+=pow(points[i].z-mean.z ,2.0f)/(hsScalar)(numPoints);
		Cov[0][1]+=(points[i].x-mean.x)*(points[i].y-mean.y)/(hsScalar)(numPoints);
		Cov[0][2]+=(points[i].x-mean.x)*(points[i].z-mean.z)/(hsScalar)(numPoints);
		Cov[1][2]+=(points[i].y-mean.y)*(points[i].z-mean.z)/(hsScalar)(numPoints);
	}
	Cov[2][0]=Cov[0][2];
	Cov[1][0]=Cov[0][1];
	Cov[2][1]=Cov[1][2];
	NxF32 Covun[9];
	for(int i=0;i<3;i++)
	{
		for(int j=0; j<3;j++)
		{
			Covun[3*i +j]=Cov[i][j];	
		}
	}

	NxVec3 eigenVals;
	NxMat33 CovNx,Rot;
	CovNx.setRowMajor(Covun);
	if(fUtilLib==nil)Init();
	NxDiagonalizeInertiaTensor(CovNx,eigenVals,out);

	

}
hsVectorStream* plPhysXCooking::IMakePolytope(const plMaxMeshExtractor::NeutralMesh& inMesh)
{
	hsBool success=0;
	std::vector<hsPoint3> outCloud;
	hsPoint3 offset;
	int numPlanes=26;
	float planeMax[26];
	int indexMax[26];
	hsPoint3 AABBMin(FLT_MAX,FLT_MAX,FLT_MAX);
	hsPoint3 AABBMax(-FLT_MAX,-FLT_MAX,-FLT_MAX);
	//prep
	NxVec3* vectors = TRACKED_NEW NxVec3[26];
	
	int curvec=0;
	for(int xcomp= -1;xcomp<2;xcomp++)
	{
		for(int ycomp= -1;ycomp<2;ycomp++)
		{
			for(int zcomp= -1;zcomp<2;zcomp++)
			{
				if(!((xcomp==0)&&(ycomp==0)&&(zcomp==0)))
				{
					vectors[curvec].set((hsScalar)(xcomp),(hsScalar)(ycomp),(hsScalar)(zcomp));
					vectors[curvec].normalize();
					planeMax[curvec]=(-FLT_MAX);
					//indexMax[curvec]=0;
					curvec++;
				}
			}
		}
	}
	/*
	for(int i=0;i<26;i++)
	{//make your max and mins
		planeMax[i]=(-FLT_MAX);
	}
	*/
	hsPoint3 centroid(0.0f,0.0f,0.0f);
	for(int i=0;i<inMesh.fNumVerts;i++) centroid+=inMesh.fVerts[i];
	centroid=centroid/(float)inMesh.fNumVerts;
	//temp
	NxVec3* nxLocs=new NxVec3[inMesh.fNumVerts];
	NxVec3* nxLocs2=new NxVec3[inMesh.fNumVerts];
	for(int i=0;i<inMesh.fNumVerts;i++)
	{
		hsPoint3 temppt=inMesh.fVerts[i] - centroid;
		nxLocs[i]=plPXConvert::Point(temppt);
	}
	NxMat33 rot;
	NxVec3 eigen;
	PCA(nxLocs,inMesh.fNumVerts,rot);
	NxMat33 invrot;
	rot.getInverse(invrot);
	for(int i=0; i<inMesh.fNumVerts;i++)
	{
		nxLocs2[i]=invrot*nxLocs[i];
	}
	for(int i=0;i<inMesh.fNumVerts;i++)
	{
		for(int plane=0;plane<26;plane++)
		{
			float dist=nxLocs2[i].dot(vectors[plane]);
			if(dist>=planeMax[plane])
			{
				planeMax[plane]=dist;
				indexMax[plane]=i;
			}
		}
	}
   	for(int i=0;i<inMesh.fNumVerts;i++)
	{
		AABBMin.fX = hsMinimum(nxLocs2[i].x, AABBMin.fX);
		AABBMin.fY = hsMinimum(nxLocs2[i].y, AABBMin.fY);
		AABBMin.fZ = hsMinimum(nxLocs2[i].z, AABBMin.fZ);
		AABBMax.fX = hsMaximum(nxLocs2[i].x, AABBMax.fX);
		AABBMax.fY = hsMaximum(nxLocs2[i].y, AABBMax.fY);
		AABBMax.fZ = hsMaximum(nxLocs2[i].z, AABBMax.fZ);
	}
	
	int resultingPoints=0;
	for(int i=0;i<26;i++)
	{
		for(int j=0;j<26;j++)
		{
			for(int k=0;k<26;k++)
			{	
				NxVec3 res;
				if(ThreePlaneIntersect(vectors[i],nxLocs2[indexMax[i]],vectors[j],nxLocs2[indexMax[j]],	vectors[k],nxLocs2[indexMax[k]],res))
				{
					//check it is within all slabs
					bool within=true;
					int curplane=0;
					do
					{
						float intersecdist=res.dot(vectors[curplane]);
						if((intersecdist-planeMax[curplane])>0.0001)

						{
							within=false;
					
						}
						curplane++;
					}
					while((curplane<26)&&within);
					if(within)
//					if((res.x>=AABBMin.fX)&&(res.x<=AABBMax.fX)&&
//						(res.y>=AABBMin.fY)&&(res.y<=AABBMax.fY)&&
//						(res.z>=AABBMin.fZ)&&(res.z<=AABBMax.fZ))
					{
						NxVec3 reverted;
						reverted=rot*res;
						reverted.x=reverted.x +centroid.fX;
						reverted.y=reverted.y +centroid.fY;
						reverted.z=reverted.z +centroid.fZ;
						hsPoint3 out;
						out=plPXConvert::Point(reverted);
						outCloud.push_back(out);
					}
				}
			}
		}
	}
	
	//planes discovered
	//this is'nt  right
	//cleanup
	offset=centroid;

	delete[] vectors;
		hsPoint3* pointages=TRACKED_NEW hsPoint3[outCloud.size()];
	for(int x=0;x<outCloud.size();x++)pointages[x]=outCloud[x];
	hsVectorStream* vectorstrm;
	vectorstrm= CookHull(outCloud.size(),pointages,true);
	delete[] pointages;	
	delete[] nxLocs;
	delete[] nxLocs2;
	return vectorstrm;
}
