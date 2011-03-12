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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plMeshConverter Class Header   											//
//	Static class that converts a Max triMesh object into the geometrySpans	//
//	necessary for a plDrawableIce object.									//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	Created 4.18.2001 mcn													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plMeshConverter_h
#define _plMeshConverter_h

#include "Max.h"
#include "HeadSpin.h"
#include "hsTemplates.h"

class plMaxNode;
class plErrorMsg;
class hsConverterUtils;
class plMaxNode;
class plGeometrySpan;
struct hsPoint3;
struct hsVector3;
struct hsMatrix44;
class ISkin;
class plConvexVolume;
class plMAXVertNormal;
class BitmapTex;
class hsGMaterial;
class plExportMaterialData;



//// Class Definition ////////////////////////////////////////////////////////

class plMeshConverter
{
private:
    plMeshConverter();

	static hsBool	fWarnBadNormals;
	static char		fWarnBadNormalsMsg[];
	static hsBool	fWarnBadUVs;
	static char		fWarnBadUVsMsg[];
	static hsBool	fWarnSuspiciousUVs;
	static char		fWarnSuspiciousUVsMsg[];
	static char		fTooManyVertsMsg[];
	static char		fTooManyFacesMsg[];

public:
    ~plMeshConverter();
	static plMeshConverter& Instance();

    void	Init( hsBool save, plErrorMsg *msg );
    void	DeInit( hsBool deInitLongRecur = true );

	void StuffPositionsAndNormals(plMaxNode *node, hsTArray<hsPoint3> *pos, hsTArray<hsVector3> *normals);
	plConvexVolume *CreateConvexVolume( plMaxNode *node );
	// doPreshading - If true, do crappy flat shading now (since we won't do any shading later)
	hsBool	CreateSpans( plMaxNode *node, hsTArray<plGeometrySpan *> &spanArray, bool doPreshading );

private:
	bool IValidateUVs(plMaxNode* node);

	void	ISetBumpUvs(Int16 uvChan, hsTArray<plMAXVertNormal>& vertDPosDuvCache, TVFace* tvFace, UInt32 smGroup, 
									  hsPoint3* uvs1, hsPoint3* uvs2, hsPoint3* uvs3);
	void	ISetBumpUvSrcs(hsTArray<hsTArray<plExportMaterialData> *>& ourMaterials, 
										hsTArray<Int16>& bumpLayIdx, hsTArray<Int16>& bumpLayChan, hsTArray<Int16>& bumpDuChan, hsTArray<Int16>& bumpDvChan);
	void	ISetWaterDecEnvUvSrcs(hsTArray<hsTArray<plExportMaterialData> *>& ourMaterials, 
										hsTArray<Int16>& bumpLayIdx, hsTArray<Int16>& bumpLayChan, hsTArray<Int16>& bumpDuChan, hsTArray<Int16>& bumpDvChan);
	void	ISmoothUVGradients(plMaxNode* node, Mesh* mesh, 
										hsTArray<hsTArray<plExportMaterialData> *>& ourMaterials, 
										hsTArray<Int16>& bumpLayIdx, hsTArray<Int16>& bumpLayChan,
										hsTArray<plMAXVertNormal>* vertDPosDuCache, hsTArray<plMAXVertNormal>* vertDPosDvCache);
	Point3	IGetUvGradient(plMaxNode* node, const hsMatrix44& uvXform44, Int16 bmpUvwSrc,
										Mesh *mesh, int faceIdx, 
										int iUV);

	int		IGenerateUVs( plMaxNode *node, Mtl *maxMtl, Mesh *mesh, int faceIdx, int numChan, int numBlend,
						  hsPoint3 *uvs1, hsPoint3 *uvs2, hsPoint3 *uvs3 );
	void	IGetUVTransform( plMaxNode *node, Mtl *mtl, Matrix3 *uvTransform, int which );

	UInt32	ICreateHexColor( float r, float g, float b );
	UInt32	ISetHexAlpha( UInt32 color, float alpha );

    Mesh*		IGetNodeMesh(plMaxNode *node);
	void		IDeleteTempGeometry();
	Mesh*		IDuplicate2Sided(plMaxNode *node, Mesh* mesh);
        
    Interface           *fInterface;
	hsConverterUtils&	fConverterUtils;
    plErrorMsg          *fErrorMsg;
	hsBool				fIsInitialized;

	// Non-nil if we converted the MAX object and have to delete it when we're done
	TriObject			*fTriObjToDelete;
	// Non-nil if we made a copy to mess with that we need to delete when we're done
	Mesh				*fMeshToDelete;
};

#endif  // _plMeshConverter_h
