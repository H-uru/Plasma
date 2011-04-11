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
#include "HeadSpin.h"

#include "max.h"
#include "meshdlib.h" 

#include "plDicer.h"

static const int kDefFaces = 200;
static const float kDefSize = 1000.f;

plDicer::plDicer()
:	fMaxSize(kDefSize, kDefSize, kDefSize),
	fMaxFaces(kDefFaces)
{
	
}

plDicer::~plDicer()
{
}

void plDicer::SetMaxSize(const Point3& size)
{
	fMaxSize = size;
}

Point3 plDicer::GetMaxSize() const
{
	return fMaxSize;
}

void plDicer::SetMaxFaces(int n)
{
	fMaxFaces = n;
}

int plDicer::GetMaxFaces() const
{
	return fMaxFaces;
}

BOOL plDicer::Dice(INode* node, INodeTab& out)
{
	Object *obj = node->EvalWorldState(TimeValue(0)).obj;
	if( !obj )
	{
		out.Append(1, &node);
		return false;
	}

	if( !obj->CanConvertToType( triObjectClassID ) )
	{
		out.Append(1, &node);
		return false;
	}

	// Convert to triMesh object
	TriObject	*meshObj = (TriObject*)obj->ConvertToType(TimeValue(0), triObjectClassID);
	if( !meshObj )
	{
		out.Append(1, &node);
		return false;
	}

	TriObject* newObj = CreateNewTriObject();
	newObj->mesh = meshObj->mesh;

	plTriObjectTab triList;
	if( !IDiceIter(newObj, triList) )
	{
		delete newObj;
		out.Append(1, &node);
		return false;
	}

	IMakeIntoNodes(node, triList, out);
	
	node->Delete(TimeValue(0), true);

	return true;
}

BOOL plDicer::IDetach(TriObject* triObj, BitArray& faces, plTriObjectTab& triList)
{
	TriObject* newObj = CreateNewTriObject();
	Mesh* newMesh = &newObj->mesh;

	MeshDelta meshDelta(triObj->mesh);

	// meshDelta.Detach(Mesh & m, Mesh *out, BitArray fset, BOOL faces, BOOL del, BOOL elem);
	meshDelta.Detach(triObj->mesh, newMesh, faces, true, true, false);

	meshDelta.Apply(triObj->mesh);

	if( newObj->mesh.getNumFaces() )
		triList.Append(1, &newObj);
	else
		delete newObj;

	if( triObj->mesh.getNumFaces() )
	{
		newObj = CreateNewTriObject();
		newObj->mesh = triObj->mesh;
		triList.Append(1, &newObj);
	}

	delete triObj;

	return true;
}

BOOL plDicer::IHalf(TriObject* triObj, plTriObjectTab& triList)
{
	Mesh& mesh = triObj->mesh;
	BitArray faces(mesh.getNumFaces());

	int iAxis = 0;
	float maxDim = mesh.getBoundingBox().Width()[0];
	if( mesh.getBoundingBox().Width()[1] > maxDim )
	{
		maxDim = mesh.getBoundingBox().Width()[1];
		iAxis = 1;
	}
	if( mesh.getBoundingBox().Width()[2] > maxDim )
	{
		maxDim = mesh.getBoundingBox().Width()[2];
		iAxis = 2;
	}
	float middle = mesh.getBoundingBox().Center()[iAxis];

	int numHi = 0;
	int i;
	for( i = 0; i < mesh.getNumFaces(); i++ )
	{
		Point3 p[3];
		p[0] = mesh.getVert(mesh.faces[i].getVert(0));
		p[1] = mesh.getVert(mesh.faces[i].getVert(1));
		p[2] = mesh.getVert(mesh.faces[i].getVert(2));

		if( (p[0][iAxis] > middle)
			&&(p[1][iAxis] > middle)
			&&(p[2][iAxis] > middle) )
		{
			numHi++;
			faces.Set(i);
		}
	}
	if( !numHi || (numHi == mesh.getNumFaces()) )
		return false;

	return IDetach(triObj, faces, triList);
}

BOOL plDicer::IDice(TriObject* triObj, plTriObjectTab& triList)
{
	int oneBigger = -1;
	int oneSmaller = -1;
	BOOL doChop = false;
	// First, does he need chopping?
	Mesh& mesh = triObj->mesh;
	if( mesh.getNumFaces() > GetMaxFaces() )
	{
		doChop = true;
	}
	else
	{
		Box3 bnd = mesh.getBoundingBox();
		Point3 wid = bnd.Width();
		
		if( wid.x > GetMaxSize().x )
			doChop = true;
		if( wid.y > GetMaxSize().y )
			doChop = true;
		if( wid.z > GetMaxSize().z )
			doChop = true;
	}

	if( !doChop )
		return false;

	// Okay, we got to chop.
	return IHalf(triObj, triList);
}

BOOL plDicer::IDiceIter(TriObject* triObj, plTriObjectTab& triList)
{
	triList.ZeroCount();

	plTriObjectTab inList;
	plTriObjectTab cutList;
	
	inList.Append(1, &triObj);

	while( inList.Count() )
	{
		int i;
		for( i = 0; i < inList.Count(); i++ )
		{
			if( !IDice(inList[i], cutList) )
			{
				triList.Append(1, &inList[i]);
			}
		}
		inList = cutList;
		cutList.ZeroCount();
	}

	return triList.Count() > 0;
}

BOOL plDicer::IMakeIntoNodes(INode* node, plTriObjectTab& triList, INodeTab& out)
{
	NameMaker *nn = GetCOREInterface()->NewNameMaker();
	TSTR nodeName(node->GetName());

	int i;
	for( i = 0; i < triList.Count(); i++ )
	{
		INode* outNode = GetCOREInterface()->CreateObjectNode(triList[i]);

		outNode->SetNodeTM(TimeValue(0), node->GetNodeTM(TimeValue(0)));
		outNode->CopyProperties(node);
		outNode->SetMtl(node->GetMtl());
		outNode->SetObjOffsetPos(node->GetObjOffsetPos());
		outNode->SetObjOffsetRot(node->GetObjOffsetRot());
		outNode->SetObjOffsetScale(node->GetObjOffsetScale());

		nn->MakeUniqueName(nodeName);
		outNode->SetName(nodeName);

		out.Append(1, &outNode);
	}
	return true;
}
