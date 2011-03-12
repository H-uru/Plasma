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
#include "plMaxMeshExtractor.h"
#include "Max.h"
#include "plMaxNode.h"
#include "dummy.h"

static Mesh* ExtractMesh(INode* pNode, TriObject** ppDeleteMe)
{
	Object *obj = pNode->EvalWorldState(0).obj;
	Mesh *pRetMesh = nil;

	if( obj ) {

		if( obj->CanConvertToType(triObjectClassID) ) {

			// Convert to triangle object
			TriObject *tri = (TriObject*)obj->ConvertToType(0, triObjectClassID);

			if (tri != obj) *ppDeleteMe = tri;	// if Convert allocated, pass back so caller can delete.

			if( tri ) {
				Mesh *pTMesh = &tri->mesh;

				if( pTMesh->getNumFaces() ) {
					pRetMesh = pTMesh;
				}
			}
		}
	}
	return pRetMesh;
}

// Return the two points defining the bounding box for the given vertices.
static void MeshMinMax(hsPoint3& min, hsPoint3& max, int numVerts, hsPoint3* pVerts)
{
	for (int i = 0; i < numVerts; i++)
	{
		hsPoint3* vert = &pVerts[i];
		min.fX = hsMinimum(vert->fX, min.fX);
		min.fY = hsMinimum(vert->fY, min.fY);
		min.fZ = hsMinimum(vert->fZ, min.fZ);
		max.fX = hsMaximum(vert->fX, max.fX);
		max.fY = hsMaximum(vert->fY, max.fY);
		max.fZ = hsMaximum(vert->fZ, max.fZ);
	}
}

static bool MakeNormalMesh(plMaxNode *node, plMaxMeshExtractor::NeutralMesh& mesh, Matrix3* w2l)
{
	TriObject *pDeleteMe = nil;
	Mesh *pMesh = ExtractMesh(node, &pDeleteMe);	// allocates *sometimes*; check pDeleteMe

	if (!pMesh)
		return false;

	Matrix3 fullTM = node->GetObjectTM(0);
	int parity = fullTM.Parity();

	mesh.fNumVerts = pMesh->numVerts;
	mesh.fVerts = TRACKED_NEW hsPoint3[mesh.fNumVerts];

	for (int i = 0; i < mesh.fNumVerts; i++)
	{
		// convert the vertex to global coordinates
		Point3 newVert = fullTM * pMesh->verts[i];
		// convert the vertex to the new (requested) coordinate system
		if (w2l)
			newVert = (*w2l) * newVert;

		mesh.fVerts[i].Set(newVert.x, newVert.y, newVert.z);
	}

	mesh.fNumFaces = pMesh->numFaces;
	mesh.fFaces = TRACKED_NEW UInt16[mesh.fNumFaces*3];
	for (int i = 0; i < mesh.fNumFaces; i++)
	{
		Face* pFace = &pMesh->faces[i];
		UInt16* pNFace = &mesh.fFaces[i * 3];

		pNFace[0] = pFace->v[ parity ? 2 : 0 ];	// reverse winding if parity backwards
		pNFace[1] = pFace->v[1];
		pNFace[2] = pFace->v[ parity ? 0 : 2 ];	// ''
	}

	if (pDeleteMe)
		delete pDeleteMe;

	return true;
}

// BUILDBOXMESH
// Build the minimum bounding box (triangles and all) enclosing the given vertices
// DELETES the given vertex and face array
// ALLOCATES a new vertex and face array
// MODIFIES *all* the input parameters.
static void MakeBoxMesh(plMaxNode* node, plMaxMeshExtractor::NeutralMesh& mesh, hsPoint3& minV, hsPoint3& maxV)
{
	hsPoint3* newVerts = TRACKED_NEW hsPoint3[8];
	UInt16* newFaces = TRACKED_NEW UInt16[12 * 3];

	newVerts[0].Set(minV.fX, minV.fY, minV.fZ);
	newVerts[1].Set(maxV.fX, minV.fY, minV.fZ);
	newVerts[2].Set(minV.fX, maxV.fY, minV.fZ);
	newVerts[3].Set(maxV.fX, maxV.fY, minV.fZ);
	newVerts[4].Set(minV.fX, minV.fY, maxV.fZ);
	newVerts[5].Set(maxV.fX, minV.fY, maxV.fZ);
	newVerts[6].Set(minV.fX, maxV.fY, maxV.fZ);
	newVerts[7].Set(maxV.fX, maxV.fY, maxV.fZ);

	UInt16 standardFaces[] = { 0, 2, 1,
		1, 2, 3,
		0, 1, 4,
		1, 5, 4,
		0, 4, 2,
		2, 4, 6,
		1, 3, 7,
		7, 5, 1,
		3, 2, 7,
		2, 6, 7,
		4, 7, 6,
		4, 5, 7 };

	memcpy(newFaces, standardFaces, sizeof(standardFaces));

	delete [] mesh.fVerts;
	mesh.fVerts = newVerts;
	delete [] mesh.fFaces;
	mesh.fFaces = newFaces;
	mesh.fNumVerts = 8;
	mesh.fNumFaces = 12;
}

static void MakeDummyMesh(plMaxNode* node, plMaxMeshExtractor::NeutralMesh& mesh)
{
	hsPoint3 minV, maxV;

	Object* thisObj = node->GetObjectRef();
	DummyObject* thisDummy = (DummyObject*)thisObj;
	Box3 thisBoundSurface = thisDummy->GetBox();
	minV.fX = thisBoundSurface.Min().x;
	minV.fY = thisBoundSurface.Min().y;
	minV.fZ = thisBoundSurface.Min().z;
	maxV.fX = thisBoundSurface.Max().x;
	maxV.fY = thisBoundSurface.Max().y;
	maxV.fZ = thisBoundSurface.Max().z;

	MakeBoxMesh(node, mesh, minV, maxV);
}

// CREATEPLHKPHYSICALFROMMESHEASY
// Convenience function for getting from a max node to a plHKPhysical and the requisite
// Havok objects.
// The node and the scene object don't have to correspond to the same Max object.
// If the sAltNode is supplied, the node will be moved into the coordinate system of the
bool plMaxMeshExtractor::Extract(plMaxMeshExtractor::NeutralMesh& mesh, plMaxNode* node, bool makeAABB, plMaxNode* sOwningNode)
{
	mesh.fNumVerts = mesh.fNumFaces = 0;
	mesh.fVerts = nil;
	mesh.fFaces = nil;

	// if an alternate node was supplied, get its scene object. otherwise don't...
	plMaxNode* masterNode = sOwningNode ? sOwningNode : node;

	mesh.fL2W = masterNode->GetLocalToWorld44();

	//
	// Create the arrays of verts and faces
	//
	hsBool isDummy = (node->EvalWorldState(0).obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0));
	if (isDummy)
	{
		hsMatrix44 w2l = masterNode->GetWorldToLocal44();
		MakeDummyMesh(node, mesh);
		// Localize the verts
		//for (int i = 0; i < mesh.fNumVerts; i++)
		//	mesh.fVerts[i] = w2l * mesh.fVerts[i];
	}
	else
	{
		// only get the max world-to-local transform if the node is moveable or instanced. otherwise verts stay global.
		Matrix3 w2l = masterNode->GetWorldToLocal();
//		Matrix3 *localizer = nil;
//		if (masterNode->IsMovable() || masterNode->GetForceLocal() || masterNode->GetInstanced())
//			localizer = &w2l;

		if (!MakeNormalMesh(node, mesh, &w2l))
			return false;

		if (makeAABB)
		{
			hsPoint3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			MeshMinMax(minV, maxV, mesh.fNumVerts, mesh.fVerts);
			MakeBoxMesh(node, mesh, minV, maxV);
		}
	}

	return true;
}
