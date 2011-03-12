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

#ifndef plRenderInstance_inc
#define plRenderInstance_inc

#include "hsTemplates.h"

class plRenderInstance : public RenderInstance
{
protected:
	plRenderInstance*			fNext;
	Interval					fValid;

	INode*						fNode;
	Object*						fObject;

	BOOL						fDeleteMesh;

	hsTArray<LightDesc*>		fLights;
public:
	plRenderInstance();
	virtual ~plRenderInstance();

	BOOL GetFromNode(INode* node, TimeValue& t, int idx);
	BOOL Update(TimeValue& t);
	void SetNext(plRenderInstance* n) { fNext = n; }
	void Cleanup();

	virtual RenderInstance *Next() { return fNext; }	// next in list

	virtual Interval MeshValidity() { return fValid; }

	virtual int NumLights() { return fLights.GetCount(); }
	virtual LightDesc *Light(int n) { return fLights[n]; }
	virtual void AddLight(LightDesc* l) { fLights.Append(l); }
	virtual void ClearLights() { fLights.SetCount(0); }

	virtual BOOL CastsShadowsFrom(const ObjLightDesc& lt); // is lt shadowed by this instance?

	virtual INode *GetINode() { return fNode; }  						 // get INode for instance
	virtual Object *GetEvalObject() { return fObject; } 					 // evaluated object for instance

	virtual Point3 GetFaceNormal(int faceNum);         // geometric normal in camera coords
	virtual Point3 GetFaceVertNormal(int faceNum, int vertNum);  // camera coords
	virtual void GetFaceVertNormals(int faceNum, Point3 n[3])   // camera coords
	{
		n[0] = GetFaceVertNormal(faceNum, 0);
		n[1] = GetFaceVertNormal(faceNum, 1);
		n[2] = GetFaceVertNormal(faceNum, 2);
	}	
	virtual Point3 GetCamVert(int vertnum); 			 // coord for vertex in camera coords		
	virtual void GetObjVerts(int fnum, Point3 obp[3]); // vertices of face in object coords
	virtual void GetCamVerts(int fnum, Point3 cp[3]); // vertices of face in camera(view) coords

	// Material-by-face access
	// Objects can provide a material as a function of face number via the IChkMtlAPI interface (chkmtlapi.h).
	// This method will return RenderInstance::mtl if flag INST_MTL_BYFACE is not set. If INST_MTL_BYFACE is
	// set it will return the proper by-face mtl. // DS 4/3/00
	virtual Mtl *GetMtl(int faceNum);  
	virtual ULONG MtlRequirements(int mtlNum, int faceNum);  	 // node's mtl requirements. DS 3/31/00: added faceNum to support mtl-per-face objects
};

#endif // plRenderInstance_inc
