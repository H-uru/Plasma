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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plMeshConverter Class Header                                            //
//  Static class that converts a Max triMesh object into the geometrySpans  //
//  necessary for a plDrawableIce object.                                   //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  Created 4.18.2001 mcn                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plMeshConverter_h
#define _plMeshConverter_h

#include <vector>

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

    static bool     fWarnBadNormals;
    static char     fWarnBadNormalsMsg[];
    static bool     fWarnBadUVs;
    static char     fWarnBadUVsMsg[];
    static bool     fWarnSuspiciousUVs;
    static char     fWarnSuspiciousUVsMsg[];
    static char     fTooManyVertsMsg[];
    static char     fTooManyFacesMsg[];

public:
    ~plMeshConverter() noexcept(false);
    static plMeshConverter& Instance();

    void    Init( bool save, plErrorMsg *msg );
    void    DeInit( bool deInitLongRecur = true );

    void StuffPositionsAndNormals(plMaxNode *node, std::vector<hsPoint3> *pos, std::vector<hsVector3> *normals);
    plConvexVolume *CreateConvexVolume( plMaxNode *node );
    // doPreshading - If true, do crappy flat shading now (since we won't do any shading later)
    bool    CreateSpans(plMaxNode *node, std::vector<plGeometrySpan *> &spanArray, bool doPreshading);

private:
    bool IValidateUVs(plMaxNode* node);

    void    ISetBumpUvs(int16_t uvChan, std::vector<plMAXVertNormal>& vertDPosDuvCache, TVFace* tvFace, uint32_t smGroup,
                        hsPoint3* uvs1, hsPoint3* uvs2, hsPoint3* uvs3);
    void    ISetBumpUvSrcs(std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                           std::vector<int16_t>& bumpLayIdx, std::vector<int16_t>& bumpLayChan,
                           std::vector<int16_t>& bumpDuChan, std::vector<int16_t>& bumpDvChan);
    void    ISetWaterDecEnvUvSrcs(std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                                  std::vector<int16_t>& bumpLayIdx, std::vector<int16_t>& bumpLayChan,
                                  std::vector<int16_t>& bumpDuChan, std::vector<int16_t>& bumpDvChan);
    void    ISmoothUVGradients(plMaxNode* node, Mesh* mesh, 
                               std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                               std::vector<int16_t>& bumpLayIdx, std::vector<int16_t>& bumpLayChan,
                               std::vector<plMAXVertNormal>* vertDPosDuCache, std::vector<plMAXVertNormal>* vertDPosDvCache);
    Point3  IGetUvGradient(plMaxNode* node, const hsMatrix44& uvXform44, int16_t bmpUvwSrc,
                                        Mesh *mesh, int faceIdx, 
                                        int iUV);

    int     IGenerateUVs( plMaxNode *node, Mtl *maxMtl, Mesh *mesh, int faceIdx, int numChan, int numBlend,
                          hsPoint3 *uvs1, hsPoint3 *uvs2, hsPoint3 *uvs3 );
    void    IGetUVTransform( plMaxNode *node, Mtl *mtl, Matrix3 *uvTransform, int which );

    uint32_t  ICreateHexColor( float r, float g, float b );
    uint32_t  ISetHexAlpha( uint32_t color, float alpha );

    Mesh*       IGetNodeMesh(plMaxNode *node);
    void        IDeleteTempGeometry();
    Mesh*       IDuplicate2Sided(plMaxNode *node, Mesh* mesh);
        
    Interface           *fInterface;
    hsConverterUtils&   fConverterUtils;
    plErrorMsg          *fErrorMsg;
    bool                fIsInitialized;

    // Non-nil if we converted the MAX object and have to delete it when we're done
    TriObject           *fTriObjToDelete;
    // Non-nil if we made a copy to mess with that we need to delete when we're done
    Mesh                *fMeshToDelete;
};

#endif  // _plMeshConverter_h
