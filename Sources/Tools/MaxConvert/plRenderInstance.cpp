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
#include "Max.h"

#include "plRenderInstance.h"

class plNilView : public View 
{
	
public:
	
	Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
	
	plNilView() 
	{		
		projType = 1;
		fov = hsScalarPI * 0.25f;
		pixelSize = 1.f;
		affineTM.IdentityMatrix();
		worldToView.IdentityMatrix();
		screenW=640.0f; screenH = 480.0f;
	}
	
};

static plNilView nilView;

plRenderInstance::plRenderInstance()
:	fNext(nil),
	fNode(nil),
	fObject(nil),
	fDeleteMesh(false)
{
	mtl = nil;
	mesh = nil;
	flags = 0;
	wireSize = 1.f;
	vis = 1.f;
	nodeID = 0;
	objMotBlurFrame = 0;
	objBlurID = 0;
	objToWorld.IdentityMatrix();
	objToCam.IdentityMatrix();
	normalObjToCam.IdentityMatrix();
	camToObj.IdentityMatrix();
	obBox.Init();
	center = Point3(0,0,0);
	radsq = 0;
}

plRenderInstance::~plRenderInstance()
{
}

void plRenderInstance::Cleanup()
{
	if( mesh && fDeleteMesh )
	{
		mesh->DeleteThis();
		mesh = nil;
		fDeleteMesh = false;
	}
}

BOOL plRenderInstance::Update(TimeValue& t)
{
	fObject = fNode->EvalWorldState(t).obj;
	if( !fObject )
		return false;

	// this shouldn't happen, we shouldn't be trying to make 
	// renderinstances from non GEOMOBJECT's
	if( fObject->SuperClassID() != GEOMOBJECT_CLASS_ID )
		return false;

	if( mesh && fDeleteMesh )
	{
		mesh->DeleteThis();
		mesh = nil;
	}
	fDeleteMesh = false;
	mesh = ((GeomObject*)fObject)->GetRenderMesh(t, fNode, nilView, fDeleteMesh);
	if( !mesh )
		return false;

	vis = fNode->GetVisibility(t);
	if( vis < 0.0f ) 
	{
		vis = 0.0f;
		SetFlag(INST_HIDE, 1);
		return false;
	}
	if (vis > 1.0f) vis = 1.0f;
	SetFlag(INST_HIDE, 0);

	objMotBlurFrame = NO_MOTBLUR;
	objBlurID = 0;

	objToWorld = fNode->GetObjectTM(t);
	objToCam = objToWorld;
	camToObj = Inverse(objToCam);

	normalObjToCam.IdentityMatrix();
	Matrix3 inv = camToObj;
	int i;
	for( i = 0; i < 3; i++ )
		normalObjToCam.SetRow(i, inv.GetColumn3(i));

	obBox = mesh->getBoundingBox(nil);
	center = obBox.Center();
	radsq = LengthSquared(obBox.Width());

	return true;
}

BOOL plRenderInstance::GetFromNode(INode* node, TimeValue& t, int idx)
{
	fNext = nil;

	fValid = Interval(t, t);

	fNode = node;
	mtl = node->GetMtl();
	if( mtl )
	{
		wireSize = mtl->WireSize();
	}

	nodeID = idx;

	ClearLights();

	return Update(t);
}

BOOL plRenderInstance::CastsShadowsFrom(const ObjLightDesc& constLt)
{
	ObjLightDesc& lt = const_cast<ObjLightDesc&>(constLt);
	if( !fNode->CastShadows() )
		return false;

	if( !lt.ls.shadow )
		return false;

	if( lt.GetExclList() && lt.GetExclList()->TestFlag(NT_AFFECT_SHADOWCAST) )
	{
		int idx = lt.GetExclList()->FindNode(fNode);
		BOOL isInc = lt.GetExclList()->TestFlag(NT_INCLUDE); 

		if( idx >= 0 )
		{
			return isInc;
		}
		else
		{
			return !isInc;
		}
	}

	return true;
}

Point3 plRenderInstance::GetFaceNormal(int fnum)
{
	Face* f = &mesh->faces[fnum];
	Point3 a = GetCamVert(f->v[1]) - GetCamVert(f->v[0]);
	Point3 b = GetCamVert(f->v[2]) - GetCamVert(f->v[0]);

	Point3 n = CrossProd(a, b).Normalize();
	return n;
}

Point3 plRenderInstance::GetFaceVertNormal(int fnum, int vertNum)
{
	Point3 retNorm;

	int smGroup=0;

	Face* f = &mesh->faces[fnum];

	// Get the rendered vertex.  Don't use the device position, fPos. 
	RVertex &rv = mesh->getRVert(f->v[vertNum]);

	// Number of normals at the vertex
	int numNormalsAtVert = (rv.rFlags & NORCT_MASK);

	// Specified normal ?
	int specNrml = (rv.rFlags & SPECIFIED_NORMAL);

	if (specNrml || (numNormalsAtVert == 1))
    {
		// The normal case is one normal per vertex (a vertex not beng shared by more than one smoothing group)
		// We'll assign vertex normals here.
		// If the object is faceted, this will be the same as the face normal.
		retNorm = rv.rn.getNormal();
	}
	else
	{
		int found = 0;
		for(int j=0;j<numNormalsAtVert; j++)
		{
			smGroup = rv.ern[j].getSmGroup();
			// Since this vertex is shared by more than one smoothing group, it has multiple normals.
			// This is fairly rare and doesn't occur in faceted objects.
			// Just pick the first normal and use that as the vertex normal.
			int faceSmGroup = f->getSmGroup();
			if ((smGroup & faceSmGroup) == faceSmGroup)
			{
				retNorm = rv.ern[j].getNormal();
				found++;

				// NOTE: Remove this to really check smoothing groups
				break;
			}
		}
	}
	retNorm = retNorm * normalObjToCam;
	retNorm.Normalize();
	return retNorm;
}

Point3 plRenderInstance::GetCamVert(int vertnum)
{
	return objToCam*mesh->verts[vertnum];
}

void plRenderInstance::GetObjVerts(int fnum, Point3 obp[3])
{
	Face* f = &mesh->faces[fnum];
	obp[0] = mesh->verts[f->v[0]];
	obp[1] = mesh->verts[f->v[1]];
	obp[2] = mesh->verts[f->v[2]];
}

void plRenderInstance::GetCamVerts(int fnum, Point3 cp[3])
{
	Face* f = &mesh->faces[fnum];
	cp[0] = objToCam*mesh->verts[f->v[0]];
	cp[1] = objToCam*mesh->verts[f->v[1]];
	cp[2] = objToCam*mesh->verts[f->v[2]];
}

Mtl* plRenderInstance::GetMtl(int fnum)
{
	if( !mtl )
		return nil;

	if( TestFlag(INST_MTL_BYFACE) )
	{
		if( mtl->ClassID() != Class_ID(MULTI_CLASS_ID,0) )
			return mtl;

		Face* f = &mesh->faces[fnum];
		int matIndex = f->getMatID();
		return mtl->GetSubMtl(matIndex);

	}
	else
	{
		return mtl;
	}
}

ULONG plRenderInstance::MtlRequirements(int mtlNum, int faceNum)
{
	if( !mtl )
		return 0;

	if( TestFlag(INST_MTL_BYFACE) )
	{
		Mtl* faceMtl = GetMtl(faceNum);
		return faceMtl ? faceMtl->Requirements(mtlNum) : 0;
	}
	return mtl->Requirements(mtlNum);
}