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
#include "plResetXform.h"


#include "../MaxMain/plMaxNode.h"
#include "../MaxComponent/plComponent.h"


void plResetXform::ResetSelected() const
{
	ResetSelectedRecur(GetCOREInterface()->GetRootNode());
}

void plResetXform::ResetSelectedRecur(INode* node) const
{
	if( !node )
		return;

	if( node->Selected() )
	{
		IResetNode(node);
	}
	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
		ResetSelectedRecur(node->GetChildNode(i));
}

void plResetXform::IResetNode(INode* node) const
{
	const char* dbgNodeName = node->GetName();

	BOOL deleteIt = false;
	TriObject* oldObj = IGetTriObject(node, deleteIt);
	if( !oldObj )
		return;

	Mesh& oldMesh = oldObj->mesh;

	TimeValue t(0);

	Matrix3 nodeTM = node->GetNodeTM(t);

	Matrix3 objectTM = node->GetObjectTM(t);
	Matrix3 otm = objectTM * Inverse(nodeTM);

	Point3 u = nodeTM.GetRow(0);
	Point3 v = nodeTM.GetRow(1);
	Point3 w = CrossProd(u, v);
	v = CrossProd(w, u);

	u = Normalize(u);
	v = Normalize(v);
	w = Normalize(w);


	Matrix3 newXform(u, v, w, nodeTM.GetTrans());
	Matrix3 vtxXform = otm * nodeTM * Inverse(newXform) * Inverse(otm);

	TriObject* newObj = CreateNewTriObject();
	Mesh& newMesh = newObj->mesh;

	newMesh = oldMesh;

	int i;
	for( i = 0; i < newMesh.getNumVerts(); i++ )
	{
		Point3 p = vtxXform * newMesh.getVert(i);
		newMesh.getVert(i) = p;
	}

	if( nodeTM.Parity() )
	{
		for( i = 0; i < newMesh.getNumFaces(); i++ )
		{
			newMesh.FlipNormal(i);
		}

	}
	node->SetObjectRef(newObj);

	node->SetNodeTM(t, newXform);

	if( deleteIt )
		oldObj->DeleteThis();
}

TriObject* plResetXform::IGetTriObject(INode* node, BOOL& deleteIt) const
{
	Object *obj = node->EvalWorldState(TimeValue(0)).obj;
	if( !obj )
		return NULL;

	if( !obj->CanConvertToType( triObjectClassID ) )
		return NULL;

	// Convert to triMesh object
	TriObject	*meshObj = (TriObject*)obj->ConvertToType(TimeValue(0), triObjectClassID);
	if( !meshObj )
		return NULL;

	deleteIt = meshObj != obj;

	return meshObj;
}

void plSelectNonRenderables::ICollectNonDrawablesRecur(plMaxNode* node, INodeTab& nodeTab) const
{
	plComponentBase* comp = node->ConvertToComponent();
	if( comp )
		comp->CollectNonDrawables(nodeTab);

	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
		ICollectNonDrawablesRecur((plMaxNode*)node->GetChildNode(i), nodeTab);
}

void plSelectNonRenderables::SelectNonRenderables() const
{
	INodeTab nodeTab;

	ICollectNonDrawablesRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), nodeTab);

	INodeTab unhidden;
	int i;
	for( i = 0; i < nodeTab.Count(); i++ )
	{
		INode* node = nodeTab[i];
		if( !node->IsHidden() )
			unhidden.Append(1, &node);
	}

	theHold.Begin();


	TSTR undostr; 
	undostr.printf("SelNonRend");

	GetCOREInterface()->SelectNodeTab(unhidden, true, true);

	theHold.Accept(undostr);
}
