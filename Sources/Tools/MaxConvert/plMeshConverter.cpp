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
//  plMeshConverter Class Functions                                         //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  Created 4.18.2001 mcn                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "hsExceptionStack.h"
#include "hsResMgr.h"
#include "plTweak.h"

#include "MaxMain/MaxAPI.h"

#include "plMeshConverter.h"
#include "MaxMain/plMaxNode.h"
#include "MaxExport/plErrorMsg.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plDrawable/plGeometrySpan.h"
#include "hsConverterUtils.h"
#include "hsMaterialConverter.h"
#include "hsControlConverter.h"
#include "MaxPlasmaMtls/Materials/plCompositeMtl.h"
#include "MaxPlasmaMtls/Materials/plPassMtl.h"
#include "MaxPlasmaMtls/Materials/plCompositeMtlPB.h"
#include "MaxPlasmaMtls/Materials/plPassMtlBasicPB.h"
#include "plDrawable/plGBufferGroup.h"
#include "plParticleSystem/plConvexVolume.h"
#include "plDrawable/plGeoSpanDice.h"

#include "plDrawable/plAccessGeometry.h"
#include "plDrawable/plAccessSpan.h"
#include "plDrawable/plAccessTriSpan.h"
#include "plDrawable/plAccessVtxSpan.h"

#include "plStatusLog/plStatusLog.h"

//// Static Members //////////////////////////////////////////////////////////

bool    plMeshConverter::fWarnBadNormals = true;
char    plMeshConverter::fWarnBadNormalsMsg[] = "Bad normal autogeneration - please deliver Max file to QA";

bool    plMeshConverter::fWarnBadUVs = true;
char    plMeshConverter::fWarnBadUVsMsg[] = "The object \"{}\" does not have enough valid UVW mapping channels \
for the material(s) applied to it. This might produce unwanted rendering artifacts at runtime";

bool    plMeshConverter::fWarnSuspiciousUVs = true;
char    plMeshConverter::fWarnSuspiciousUVsMsg[] = "The object \"{}\" has suspicious UVW coordinates on it. \
You should apply an Unwrap UVW modifier to it.";

char    plMeshConverter::fTooManyVertsMsg[] = "The mesh \"{}\" has too many vertices to fit into a single buffer. \
Please break up the mesh into pieces with no more than {} vertices each\
or apply optimize terrain.";

char    plMeshConverter::fTooManyFacesMsg[] = "The mesh \"{}\" has too many faces to fit into a single buffer. \
Please break up the mesh into pieces with no more than {} faces each\
or apply optimize terrain.";


//// Local Helper Class Definitions //////////////////////////////////////////

class TempWeightInfo
{
    public:
        float   fWeights[ 4 ];
        uint32_t  fIndices;
};


class plMAXVertexAccNode
{
    public:
        hsPoint3    fPoint; // Inefficient space-wise, I know, but it makes this a lot simpler...
        hsVector3   fNormal;
        hsColorRGBA fColor, fIllum;
        uint32_t      fIndex;
        hsPoint3    fUVs[ plGeometrySpan::kMaxNumUVChannels ];
        uint32_t      fNumChannels;

        plMAXVertexAccNode  *fNext;

        plMAXVertexAccNode( const hsPoint3 *point, const hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, int numChannels, const hsPoint3 *uvs, uint32_t index );

        bool        IsEqual( const hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, const hsPoint3 *uvs );
};

typedef plMAXVertexAccNode      *plMAXVertexAccNodePtr;

class plMAXVertexAccumulator
{
    protected:
        
        int                 fNumPoints, fNumChannels, fNumVertices;
        plMAXVertexAccNode  **fPointList;

        std::vector<uint32_t> fIndices;
        std::vector<uint32_t> fInverseVertTable;

        void    IFindSkinWeights( ISkinContextData *skinData, int vertex, float *weights, uint32_t *indices );
        void    IFindUserSkinWeights( plMaxNode* node, Mesh* mesh, int vertex, float *weights, uint32_t *indices );
        void    IFindAllUserSkinWeights( plMaxNode* node, Mesh* mesh, TempWeightInfo weights[]);
    public:

        plMAXVertexAccumulator( int numOrigPoints, int numChannels );
        ~plMAXVertexAccumulator();

        void        AddVertex( int index, hsPoint3 *point, hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, hsPoint3 uvs[ plGeometrySpan::kMaxNumUVChannels ] );

        void        StuffMyData( plMaxNode* node, plGeometrySpan *span, Mesh* mesh, ISkinContextData* skinData );

        int         GetVertexCount();
        size_t      GetIndexCount() { return fIndices.size(); }
};

class plMAXVertNormal
{
    public:
        Point3  fNormal;
        DWORD   fSmGroup;
        bool    fInited;

        plMAXVertNormal *fNext;

        plMAXVertNormal() { fSmGroup = 0; fNext = nullptr; fInited = false; fNormal = Point3(0, 0, 0); }
        plMAXVertNormal(Point3 &n, DWORD s) { fNext = nullptr; fInited = true; fNormal = n; fSmGroup = s; }
        ~plMAXVertNormal() { /*delete fNext; */}
        void    DestroyChain() { if (fNext != nullptr) fNext->DestroyChain(); delete fNext; fNext = nullptr; }

        // Adding normalization of input n. Input is usually just crossproduct of face edges. Non-normalized,
        // large faces will overwhelm small faces on summation, which is the opposite of what we want, since
        // curvature is most accurately captured in the small faces. mf.
        void    AddNormal( Point3 &n, DWORD s )
        {
            if( !( s & fSmGroup ) && fInited )
            {
                if( fNext )
                    fNext->AddNormal( n, s );
                else
                    fNext = new plMAXVertNormal( ::Normalize(n), s );
            }
            else
            {
                fNormal += ::Normalize(n);
                fSmGroup |= s;
                fInited = true;
            }
        }

        Point3  &GetNormal( DWORD s )
        {
            if ((fSmGroup & s) || (fNext == nullptr))
                return fNormal;
            else
                return fNext->GetNormal( s );
        }

        /// Relies on static variable, values will be destroyed between calls
        hsVector3   &GetPlNormal( DWORD s )
        {
            Point3  n = GetNormal( s );
            static hsVector3    pt;
            pt.Set( n.x, n.y, n.z );
            return pt;
        }

        void    Normalize()
        {
            plMAXVertNormal *ptr = fNext, *prev = this;


            while (ptr != nullptr)
            {
                if( ptr->fSmGroup & fSmGroup )
                {
                    fNormal += ptr->fNormal;
                    prev->fNext = ptr->fNext;
                    delete ptr;
                    ptr = prev->fNext;
                }
                else
                {
                    prev = ptr;
                    ptr = ptr->fNext;
                }
            }

            fNormal = ::Normalize( fNormal );
            if( fNext )
                fNext->Normalize();
        }
};

//// Instance and Constructor/Destructor /////////////////////////////////////

plMeshConverter& plMeshConverter::Instance()
{
    static plMeshConverter the_instance;
    return the_instance;
}

plMeshConverter::plMeshConverter() :
                    fInterface(),
                    fConverterUtils(hsConverterUtils::Instance()),
                    fIsInitialized()
{
    hsGuardBegin("plMeshConverter::plMeshConverter");
    hsGuardEnd;
}

plMeshConverter::~plMeshConverter() noexcept(false)
{
    hsGuardBegin("plMeshConverter::~plMeshConverter");
    hsGuardEnd;
}

//// Init and DeInit /////////////////////////////////////////////////////////

void    plMeshConverter::Init( bool save, plErrorMsg *msg )
{
    hsGuardBegin( "plMeshConverter::Init" );

    if( fIsInitialized )
        DeInit( false );

    fIsInitialized = true;
    fInterface = GetCOREInterface();
    fErrorMsg = msg;
    fWarnBadUVs = true;
    fWarnSuspiciousUVs = true;
    fWarnBadNormals = true;

    hsGuardEnd;
}

void    plMeshConverter::DeInit( bool deInitLongRecur )
{
    hsGuardBegin( "plMeshConverter::DeInit" );

    fIsInitialized = false;

    hsGuardEnd;
}

void plMeshConverter::StuffPositionsAndNormals(plMaxNode *node, std::vector<hsPoint3> *pos, std::vector<hsVector3> *normals)
{
    hsGuardBegin( "plMeshConverter::BuildNormalsArray" );

    auto            dbgNodeName = node->GetName();
    Mesh            *mesh;
    hsMatrix44      l2wMatrix, vert2LMatrix, vertInvTransMatrix, tempMatrix;

    /// Get da mesh
    mesh = IGetNodeMesh( node );
    if (mesh == nullptr)
        return ;

    int numVerts = mesh->getNumVerts();

    /// Get transforms
    l2wMatrix = node->GetLocalToWorld44();
    vert2LMatrix = node->GetVertToLocal44();            // vert2LMatrix is the transform we apply 
                                                        // now to the verts, to get into *our* object-local-space
    vert2LMatrix.GetInverse( &tempMatrix );
    tempMatrix.GetTranspose( &vertInvTransMatrix ); // Inverse-transpose of the vert2Local matrix, 
                                                    // for xforming the normals
    mesh->buildNormals();

    normals->resize(numVerts);
    pos->resize(numVerts);
    for (int i = 0; i < numVerts; i++)
    {
        // positions
        hsPoint3 currPos;
        currPos.Set(mesh->verts[i].x, mesh->verts[i].y, mesh->verts[i].z);
        (*pos)[i] = vert2LMatrix * currPos;

        // normals
        RVertex &rv = mesh->getRVert(i);
        Point3& norm = rv.rn.getNormal();
        hsVector3 currNorm(norm.x, norm.y, norm.z);
        currNorm.Normalize();
        (*normals)[i] = vertInvTransMatrix * currNorm;
    }

    IDeleteTempGeometry();
    hsGuardEnd; 
}

plConvexVolume *plMeshConverter::CreateConvexVolume(plMaxNode *node)
{
    hsGuardBegin( "plMeshConverter::CreateConvexVolume" );

    auto            dbgNodeName = node->GetName();
    Mesh            *mesh;
    int32_t         numFaces, i, j, numVerts;
    hsMatrix44      l2wMatrix, vert2LMatrix, vertInvTransMatrix, tempMatrix;
    bool            flipOrder, checkForOverflow = false;

    /// Get da mesh
    mesh = IGetNodeMesh( node );
    if (mesh == nullptr)
        return nullptr;

    numFaces = mesh->getNumFaces();
    numVerts = mesh->getNumVerts();

    plConvexVolume *bounds = new plConvexVolume();

    /// Get transforms
    l2wMatrix = node->GetLocalToWorld44();
    vert2LMatrix = node->GetVertToLocal44();
    flipOrder = vert2LMatrix.GetParity();               // vert2LMatrix is the transform we apply 
                                                        // now to the verts, to get into *our* object-local-space
    vert2LMatrix.GetInverse( &tempMatrix );
    tempMatrix.GetTranspose( &vertInvTransMatrix ); // Inverse-transpose of the vert2Local matrix, 
                                                    // for xforming the normals


    //mesh->buildNormals();

    for( i = 0; i < numFaces; i++ )
    {
        Face        *maxFace = &mesh->faces[ i ];
        TVFace      *maxColorFace = (mesh->vcFace != nullptr) ? &mesh->vcFace[i] : nullptr;
        hsPoint3    pos[ 3 ];
        hsPoint3    testPt1, testPt2, testPt3;
        hsVector3   normal;
        uint32_t      vertIdx[ 3 ];

        /// Add the 3 vertices to the correct vertex accumulator object

        // Get positions
        if( flipOrder )
        {
            for( j = 0; j < 3; j++ )
            {
                vertIdx[ j ] = maxFace->getVert( 2 - j );
                pos[ j ].fX = mesh->verts[ vertIdx[ j ] ].x;
                pos[ j ].fY = mesh->verts[ vertIdx[ j ] ].y;
                pos[ j ].fZ = mesh->verts[ vertIdx[ j ] ].z;
            }
        }
        else
        {
            for( j = 0; j < 3; j++ )
            {
                vertIdx[ j ] = maxFace->getVert( j );
                pos[ j ].fX = mesh->verts[ vertIdx[ j ] ].x;
                pos[ j ].fY = mesh->verts[ vertIdx[ j ] ].y;
                pos[ j ].fZ = mesh->verts[ vertIdx[ j ] ].z;
            }
        }

        // Look for degenerate triangles (why MAX even gives us these I have no clue...)
        testPt1 = pos[ 1 ] - pos[ 0 ];
        testPt2 = pos[ 2 ] - pos[ 0 ];
        testPt3 = pos[ 2 ] - pos[ 1 ];
        if( ( testPt1.fX == 0.0f && testPt1.fY == 0.0f && testPt1.fZ == 0.0f ) ||
            ( testPt2.fX == 0.0f && testPt2.fY == 0.0f && testPt2.fZ == 0.0f ) ||
            ( testPt3.fX == 0.0f && testPt3.fY == 0.0f && testPt3.fZ == 0.0f ) )
        {
            continue;
        }

        // Translate to local space
        for( j = 0; j < 3; j++ )
            pos[ j ] = vert2LMatrix * pos[ j ];

        // Calculate normal for face
        hsVector3   v1, v2;
        v1.Set( &pos[ 1 ], &pos[ 0 ] );
        v2.Set( &pos[ 2 ], &pos[ 0 ] );
        normal = (v1 % v2);
        normal.Normalize();

        hsPlane3 plane(&normal, normal.InnerProduct(pos[0]));
        bounds->AddPlane(plane); // auto-checks for redundant planes.
    }

    IDeleteTempGeometry();

    return bounds;
    hsGuardEnd; 
}

//
// Sometimes objects can have faces without UV coordinates.  These faces will
// export random UV values each time we export, changing the data and forcing
// patches when they aren't necessary.  To detect these, we put a Unwrap UVW mod
// on the object and see if it changes the UV values.
//
bool plMeshConverter::IValidateUVs(plMaxNode* node)
{
    if (node->GetObjectRef()->SuperClassID() != GEN_DERIVOB_CLASS_ID)
        return true;

    Mesh* mesh = IGetNodeMesh(node);
    if (!mesh)
        return true;

    if (mesh->getNumMaps() < 2)
        return true;

    // Cache the original UV verts
    int numVerts = mesh->getNumMapVerts(1);
    int vertBufSize = sizeof(UVVert)*numVerts;
    UVVert* origVerts = new UVVert[vertBufSize];
    memcpy(origVerts, mesh->mapVerts(1), vertBufSize);

    IDeleteTempGeometry();

    // Add an Unwrap UVW mod onto the stack
    IDerivedObject* derivedObject = (IDerivedObject*)node->GetObjectRef();
    #define UNWRAP_UVW_CID Class_ID(0x02df2e3a, 0x72ba4e1f)
    Modifier* mod = (Modifier*)GetCOREInterface()->CreateInstance(OSM_CLASS_ID, UNWRAP_UVW_CID);
    derivedObject->AddModifier(mod);

    mesh = IGetNodeMesh(node);

    bool uvsAreBad = false;

    UVVert* newVerts = mesh->mapVerts(1);
    for (int i = 0; i < numVerts; i++)
    {
        UVVert uvDiff = newVerts[i] - origVerts[i];
        float diff = uvDiff.Length();
        if (diff > 0.01)
        {
            uvsAreBad = true;
            break;
        }
    }

    delete [] origVerts;
    IDeleteTempGeometry();

    derivedObject->DeleteModifier();

    if (uvsAreBad)
    {
        plFileName logfile = ST::format("UV_{}.log", GetCOREInterface()->GetCurFileName().data());
        plStatusLog::AddLineSF(logfile, "{} has suspicious UVs", node->GetName());

        if (fWarnSuspiciousUVs)
        {
            /// We're missing some UV channels on our object. We'll handle it later; warn the user here
            if (fErrorMsg->Set(true, "UVW Warning", ST::format(fWarnSuspiciousUVsMsg, node->GetName())).CheckAskOrCancel())
                fWarnSuspiciousUVs = false;
            fErrorMsg->Set(false);
        }
    }

    return uvsAreBad;
}

static void SetWaterColor(plGeometrySpan* span);

//// CreateSpans /////////////////////////////////////////////////////////////
//  Main function. Takes a maxNode's object and creates geometrySpans from it
//  suitable for drawing as ice.

bool plMeshConverter::CreateSpans(plMaxNode *node, std::vector<plGeometrySpan *> &spanArray, bool doPreshading)
{
    hsGuardBegin( "plMeshConverter::CreateSpans" );

    auto            dbgNodeName = node->GetName();
    Mesh            *mesh;
    int32_t         numFaces, i, numVerts, maxNumBones, maxUVWSrc;
    int32_t         numMaterials = 1, numSubMaterials = 1;
    hsMatrix44      l2wMatrix, vert2LMatrix, vertInvTransMatrix, tempMatrix;
    Mtl             *maxMaterial = nullptr;
    bool            isComposite, isMultiMat, flipOrder, checkForOverflow = false, includesComp;
    uint8_t         ourFormat, numChannels, maxBlendChannels;
    hsColorRGBA     *colorArray = nullptr;
    hsColorRGBA     *illumArray = nullptr;
    uint32_t        sharedSpanProps = 0;
    hsBitVector     usedSubMtls;
    bool            makeAlphaLayer = node->VtxAlphaNotAvailable();

    ISkinContextData    *skinData;

    std::vector<std::vector<plExportMaterialData> *>        ourMaterials;
    std::vector<std::vector<plMAXVertexAccumulator *> *>    ourAccumulators;

    std::vector<plMAXVertNormal>    vertNormalCache;
    std::vector<plMAXVertNormal>*   vertDPosDuCache = nullptr;
    std::vector<plMAXVertNormal>*   vertDPosDvCache = nullptr;

    //// Setup ///////////////////////////////////////////////////////////////
    plLocation nodeLoc = node->GetLocation(); 
    
    TimeValue timeVal = fConverterUtils.GetTime(fInterface);
    Class_ID cid = node->EvalWorldState(timeVal).obj->ClassID();
    if( node->EvalWorldState(timeVal).obj->ClassID() == BONE_OBJ_CLASSID )
        return false;

    IValidateUVs(node);

    /// Get da mesh
    mesh = IGetNodeMesh( node );
    if (mesh == nullptr)
        return false;
    numFaces = mesh->getNumFaces();
    numVerts = mesh->getNumVerts();

    /// Get the material
    maxMaterial = hsMaterialConverter::Instance().GetBaseMtl( node );
    isMultiMat = hsMaterialConverter::Instance().IsMultiMat( maxMaterial );

    const bool smoothAll = node->GetSmoothAll();

    includesComp = false;
    if (isMultiMat)
    {
        for (i = 0; i < numFaces; i++)
        {
            int index = mesh->faces[i].getMatID();
            if (index >= maxMaterial->NumSubMtls())
                index = 0;

            usedSubMtls.SetBit(index);
            if (hsMaterialConverter::Instance().IsCompositeMat(maxMaterial->GetSubMtl(index)))
                includesComp = true;
        }
    }
    else
        includesComp = hsMaterialConverter::Instance().IsCompositeMat(maxMaterial);
    
    try
    {

        /// Check vert count
        if( numVerts >= plGBufferGroup::kMaxNumVertsPerBuffer || numFaces * 3 >= plGBufferGroup::kMaxNumIndicesPerBuffer )
        {
            /// Possible overflow, but not sure. Only check for overflow if this is set
            checkForOverflow = true;
        }

        /// Get transforms
        l2wMatrix = node->GetLocalToWorld44();
        vert2LMatrix = node->GetVertToLocal44();
        flipOrder = vert2LMatrix.GetParity();               // vert2LMatrix is the transform we apply 
                                                            // now to the verts, to get into *our* object-local-space
        vert2LMatrix.GetInverse( &tempMatrix );
        tempMatrix.GetTranspose( &vertInvTransMatrix ); // Inverse-transpose of the vert2Local matrix, 
                                                        // for xforming the normals

        // OTM used in generating normals.
        Matrix3 otm = node->GetOTM();
        Matrix3 invOtm = Inverse(otm);
        invOtm.SetTrans(Point3(0,0,0));
        invOtm.ValidateFlags();

        // If we use a composite on this object, we don't want to export the illumination channel.
        UVVert *illumMap = mesh->mapVerts(MAP_SHADING);
        int numIllumVerts = mesh->getNumMapVerts(MAP_SHADING);

        UVVert *alphaMap = mesh->mapVerts(MAP_ALPHA);   
        int numAlphaVerts = mesh->getNumMapVerts(MAP_ALPHA);

        if( node->GetRunTimeLight() )
        {
            sharedSpanProps |= plGeometrySpan::kPropRunTimeLight;
        }
        if( node->GetNoPreShade() )
        {
            sharedSpanProps |= plGeometrySpan::kPropNoPreShade;
        }
        float waterHeight = 0;
        if( node->GetHasWaterHeight() )
        {
            sharedSpanProps |= plGeometrySpan::kWaterHeight;
            waterHeight = node->GetWaterHeight();
        }
        /// Which lighting equation?
        if( node->NonVtxPreshaded() )
        {
            /// OK, we can go with kLiteVtxNonPreshaded, so we get vertex alpha. Yipee!!!
            sharedSpanProps |= plGeometrySpan::kLiteVtxNonPreshaded;
        }
        
        //// Vertex Colors / Illumination ////////////////////////////////////////

        /// If there are colors, pre-convert them 
        hsColorRGBA white, black;
        white.Set(1.f, 1.f, 1.f, 1.f);
        black.Set(0, 0, 0, 1.f);
        bool allWhite = true, allBlack = true;

        if( mesh->numCVerts > 0)
        {
            if (mesh->vertCol != nullptr)
            {
                colorArray = new hsColorRGBA[ mesh->numCVerts ];
                for( i = 0; i < mesh->numCVerts; i++ )  
                {   
                    colorArray[i].Set(mesh->vertCol[ i ].x, mesh->vertCol[ i ].y, mesh->vertCol[ i ].z, 1.f);
                    if (colorArray[ i ] != black)
                        allBlack = false;
                }

                // XXX Sometimes 3DS reports that all colors have been set black (when they haven't been touched).
                // We set them white here, so that they don't affect the shader when multiplied in.
                // (Sometimes it reports them as all white too, but hey, that's the value we'd use anyway...)
                if (allBlack)
                    for( i = 0; i < mesh->numCVerts; i++ )
                        colorArray[ i ] = white;
            }
        }

        if (illumMap != nullptr)
        {
            // MF_HORSE CARNAGE
            illumArray = new hsColorRGBA[numIllumVerts];
            for( i = 0; i < numIllumVerts; i++ )
            {
                illumArray[i].Set(illumMap[ i ].x, illumMap[ i ].y, illumMap[ i ].z, 1.f);
                if (illumArray[ i ] != white)
                    allWhite = false;
            }

            // XXX Same hack as with colorArray above, except illumination values are added in, so we set them black
            // in order to not affect the shader.
            if (allWhite)
                for( i = 0; i < numIllumVerts; i++ )
                    illumArray[ i ] = black;
            // MF_HORSE CARNAGE
        }
        
        //// Materials / Mapping Channels Setup //////////////////////////////////

        numChannels = node->NumUVWChannels();

        maxBlendChannels = 0;
        
        if( isMultiMat )
        {
            numMaterials = maxMaterial->NumSubMtls();

            ourMaterials.resize(numMaterials);
            for( i = 0; i < numMaterials; i++ )
            {
                if (usedSubMtls.IsBitSet(i)) // Only export the sub materials actually used
                    ourMaterials[i] = hsMaterialConverter::Instance().CreateMaterialArray( maxMaterial->GetSubMtl(i), node, i);
                else
                    ourMaterials[i] = nullptr;
            }
        }
        else // plPassMtl, plDecalMat, plMultiPassMtl
        {
            numMaterials = 1;
            
            ourMaterials = { hsMaterialConverter::Instance().CreateMaterialArray(maxMaterial, node, 0) };
        }

        /// UV check on the layers
        for( i = 0, maxUVWSrc = -1; i < numMaterials; i++ )
        {
            std::vector<plExportMaterialData> *subMats = ourMaterials[i];
            if (subMats == nullptr)
                continue;
            for (const plExportMaterialData& currData : *subMats)
            {
                if (currData.fMaterial == nullptr)
                    continue;
                
                for (size_t k = 0; k < currData.fMaterial->GetNumLayers(); k++)
                {
                    plLayerInterface    *layer = currData.fMaterial->GetLayer( k );

                    int uvwSrc = layer->GetUVWSrc() & plLayerInterface::kUVWIdxMask;

                    if (maxUVWSrc < uvwSrc && layer->GetTexture() != nullptr)
                        maxUVWSrc = uvwSrc;
                    if( maxBlendChannels < currData.fNumBlendChannels)
                        maxBlendChannels = currData.fNumBlendChannels;
                }
            }
        }
        // If this node is a water decal set to environment map, then there's only 1 layer, but
        // we'll need an extra 2 uvw channels for the tangent space basis vectors.
        if( node->GetWaterDecEnv() )
            maxUVWSrc = 2;

        if( numChannels + maxBlendChannels < ( maxUVWSrc + 1 ) && fWarnBadUVs )
        {
            /// We're missing some UV channels on our object. We'll handle it later; warn the user here
            if( fErrorMsg->Set(true, "UVW Channel Warning", ST::format(fWarnBadUVsMsg, node->GetName())).CheckAskOrCancel() )
                fWarnBadUVs = false;
            fErrorMsg->Set( false );
        }
        else if( numChannels > ( maxUVWSrc + 1 ) )
        {
            // Make sure we allocate enough for all the channel data, even if the materials don't use them (yet...)
            // (trick is, make sure those extra channels are valid first)
            for( i = maxUVWSrc + 1; i < numChannels; i++ )
            {
                if (mesh->mapFaces(i + 1) == nullptr)
                {
                    numChannels = i;
                    break;
                }
            }

            maxUVWSrc = numChannels - 1;    
        }
        
        //maxUVWSrc += maxBlendChannels;
        if (maxUVWSrc > plGeometrySpan::kMaxNumUVChannels - 1) maxUVWSrc = plGeometrySpan::kMaxNumUVChannels - 1;

        /// Our buffer format...
/*      ourFormat = ( maxUVWSrc == -1 ) ? plGeometrySpan::kNoUVChannels :
                    ( maxUVWSrc == 0 ) ? plGeometrySpan::k1UVChannel :
                    ( maxUVWSrc == 1 ) ? plGeometrySpan::k2UVChannels :
                    ( maxUVWSrc == 2 ) ? plGeometrySpan::k3UVChannels :
                    plGeometrySpan::k4UVChannels;
*/
        ourFormat = plGeometrySpan::UVCountToFormat( maxUVWSrc + 1 );
        /// NOW allocate our accumulators, since maxUVWSrc was just calculated...
        ourAccumulators.resize(numMaterials);
        for( i = 0; i < numMaterials; i++ )
        {           
            if (ourMaterials[i] == nullptr)
            {
                ourAccumulators[i] = nullptr;
                continue;
            }

            auto currAccum = new std::vector<plMAXVertexAccumulator *>;
            ourAccumulators[i] = currAccum;
            for (size_t j = 0; j < ourMaterials[i]->size(); j++)
            {
                currAccum->emplace_back(new plMAXVertexAccumulator(mesh->getNumVerts(), maxUVWSrc + 1));
            }
        }
        

        //// Skinning ////////////////////////////////////////////////////////////

        /// Check for skinning
        ISkin* skin = node->FindSkinModifier();
        if( skin )
        {
            skinData = skin->GetContextInterface(node);
            int skinNumPoints = skinData->GetNumPoints();
            if(skinNumPoints != numVerts)
            {
                fErrorMsg->Set(true, "Skinning Error", ST::format("Invalid point count on ISkin data on node {}", dbgNodeName)).Show();
                fErrorMsg->Set();
                throw (bool)false;
                //hsAssert( skinData->GetNumPoints() == numVerts, "Invalid point count on ISkin data" );

            }

            
            /// Loop through the skin verts and find the max # of bones
            for( i = 0, maxNumBones = 0; i < numVerts; i++ )
            {
                if( skinData->GetNumAssignedBones( i ) > maxNumBones )
                    maxNumBones = skinData->GetNumAssignedBones( i );
            }
            maxNumBones++;
            if( maxNumBones > 4 )
                maxNumBones = 4;
            //hsAssert( maxNumBones >= 2, "Invalid skin (not enough bones)" );
            if( maxNumBones < 2)
            {
                fErrorMsg->Set(true, "Skinning Error", ST::format("Invalid skin (no bones) on node {}", dbgNodeName)).Show();
                fErrorMsg->Set();
                throw (bool)false;
            }


            if (node->GetBoneMap() && maxNumBones == 2)
                maxNumBones++;

            /// Change format to match
            ourFormat |= ( maxNumBones == 2 ) ? plGeometrySpan::kSkin1Weight : 
                         ( maxNumBones == 3 ) ? plGeometrySpan::kSkin2Weights : plGeometrySpan::kSkin3Weights;

            if( skin->GetNumBones() > 1 || node->GetBoneMap())
                ourFormat |= plGeometrySpan::kSkinIndices;
        }
        else
        {
            skinData = nullptr;

            if( node->NumBones() )
            {
                maxNumBones = 2;
                ourFormat |= plGeometrySpan::kSkin1Weight;
            }
        }


        //// Build Vertex Normal Cache ///////////////////////////////////////////

        vertNormalCache.resize(mesh->getNumVerts());
        for( i = 0; i < mesh->getNumFaces(); i++ )
        {
            Face        *maxFace = &mesh->faces[ i ];
            Point3      v0, v1, v2, norm;

            uint32_t smGroup = smoothAll ? 1 : maxFace->getSmGroup();

            v0 = mesh->verts[ maxFace->v[ 0 ] ];
            v1 = mesh->verts[ maxFace->v[ 1 ] ];
            v2 = mesh->verts[ maxFace->v[ 2 ] ];

            norm = ( v1 - v0 ) ^ ( v2 - v1 );
            for (int j = 0; j < 3; j++)
                vertNormalCache[ maxFace->v[ j ] ].AddNormal( norm, maxFace->smGroup );
        }
        for (plMAXVertNormal& norm : vertNormalCache)
            norm.Normalize();

        vertDPosDuCache = new std::vector<plMAXVertNormal>[numMaterials];
        vertDPosDvCache = new std::vector<plMAXVertNormal>[numMaterials];

        std::vector<int16_t>    bumpLayIdx;
        std::vector<int16_t>    bumpLayChan;
        std::vector<int16_t>    bumpDuChan;
        std::vector<int16_t>    bumpDvChan;
        ISetBumpUvSrcs(ourMaterials, bumpLayIdx, bumpLayChan, bumpDuChan, bumpDvChan);
        if( node->GetWaterDecEnv() )
            ISetWaterDecEnvUvSrcs(ourMaterials, bumpLayIdx, bumpLayChan, bumpDuChan, bumpDvChan);

        ISmoothUVGradients(node, mesh, ourMaterials, bumpLayIdx, bumpLayChan, vertDPosDuCache, vertDPosDvCache);
        
        //// Main Conversion Loop ////////////////////////////////////////////////

        // Loop through the faces and stuff them into spans
        spanArray.clear();

        mesh->buildNormals();

        for( i = 0; i < numFaces; i++ )
        {
            Face        *maxFace = &mesh->faces[ i ];
            TVFace      *maxColorFace = (mesh->vcFace != nullptr) ? &mesh->vcFace[i] : nullptr;
            hsPoint3    pos[ 3 ];
            hsPoint3    testPt1, testPt2, testPt3;
            hsVector3   normals[ 3 ];
            hsColorRGBA colors[ 3 ], illums[ 3 ];
            uint32_t      smGroup, vertIdx[ 3 ];
            hsPoint3    uvs1[ plGeometrySpan::kMaxNumUVChannels + 1];
            hsPoint3    uvs2[ plGeometrySpan::kMaxNumUVChannels + 1];
            hsPoint3    uvs3[ plGeometrySpan::kMaxNumUVChannels + 1];
            hsPoint3    temp;
            Mtl         *currMaxMtl;

            // The main index is how a multi-material keeps track of which sub material is involved. The sub material
            // may actually create multiple materials (like composites), hence the second index, but it most cases it
            // will be zero as well.

            int mainMatIndex = 0; 
            int subMatIndex = 0; 

            // Get span index
            if( isMultiMat )
            {
                mainMatIndex = maxFace->getMatID();
                if( mainMatIndex >= numMaterials )
                    mainMatIndex = 0;
                currMaxMtl = maxMaterial->GetSubMtl(mainMatIndex);
            }   
            else
                currMaxMtl = maxMaterial;

            int numBlendChannels = 0;
            std::vector<plExportMaterialData> *subMtls = ourMaterials[mainMatIndex];
            hsAssert(subMtls != nullptr, "Face is assigned a material that we think is unused.");

            for (const plExportMaterialData& subMtl : *subMtls)
            {
                int currBlend = subMtl.fNumBlendChannels;
                if (numBlendChannels < currBlend)
                    numBlendChannels = currBlend;
            }
            isComposite = hsMaterialConverter::Instance().IsCompositeMat( currMaxMtl );

            /// Add the 3 vertices to the correct vertex accumulator object

            // Get positions
            if( flipOrder )
            {
                for (int j = 0; j < 3; j++)
                {
                    vertIdx[ j ] = maxFace->getVert( 2 - j );
                    pos[ j ].fX = mesh->verts[ vertIdx[ j ] ].x;
                    pos[ j ].fY = mesh->verts[ vertIdx[ j ] ].y;
                    pos[ j ].fZ = mesh->verts[ vertIdx[ j ] ].z;
                }
            }
            else
            {
                for (int j = 0; j < 3; j++)
                {
                    vertIdx[ j ] = maxFace->getVert( j );
                    pos[ j ].fX = mesh->verts[ vertIdx[ j ] ].x;
                    pos[ j ].fY = mesh->verts[ vertIdx[ j ] ].y;
                    pos[ j ].fZ = mesh->verts[ vertIdx[ j ] ].z;
                }
            }

            // Look for degenerate triangles (why MAX even gives us these I have no clue...)
            testPt1 = pos[ 1 ] - pos[ 0 ];
            testPt2 = pos[ 2 ] - pos[ 0 ];
            testPt3 = pos[ 2 ] - pos[ 1 ];
            if( ( testPt1.fX == 0.0f && testPt1.fY == 0.0f && testPt1.fZ == 0.0f ) ||
                ( testPt2.fX == 0.0f && testPt2.fY == 0.0f && testPt2.fZ == 0.0f ) ||
                ( testPt3.fX == 0.0f && testPt3.fY == 0.0f && testPt3.fZ == 0.0f ) )
            {
                continue;
            }

            // If we're expanding the UVW channel list, fill out the rest with zeros
            for (int32_t j = numChannels; j < maxUVWSrc; j++)
            {
                uvs1[ j ].Set( 0, 0, 0 );
                uvs2[ j ].Set( 0, 0, 0 );
                uvs3[ j ].Set( 0, 0, 0 );
            }

            // Now for each vertex, get the UVs, calc color, and add
            if( numChannels > 0 )
            {
                // Just go ahead and always generate the opacity into the uvs, because we're
                // going to look for it there on composites whether they actually use the
                // alpha hack texture or not.
                IGenerateUVs( node, currMaxMtl, mesh, i, numChannels, 
                              1, uvs1, uvs2, uvs3 );
                if( flipOrder )
                {
                    for (int j = 0; j < 3; j++)
                    {
                        temp = uvs1[ j ];
                        uvs1[ j ] = uvs3[ j ];
                        uvs3[ j ] = temp;
                    }
                }
            }

            // Handle colors
            if (maxColorFace == nullptr)
            {
                colors[2] = colors[1] = colors[0] = white;
            }
            else
            {
                colors[ 0 ] = colorArray[ maxColorFace->t[ flipOrder ? 2 : 0 ] ];
                colors[ 1 ] = colorArray[ maxColorFace->t[ flipOrder ? 1 : 1 ] ];
                colors[ 2 ] = colorArray[ maxColorFace->t[ flipOrder ? 0 : 2 ] ];
            }
            
            // Don't want to write illum values to the vertex for composite materials
            if (illumArray == nullptr || includesComp)
            {
                illums[ 0 ] = illums[ 1 ] = illums[ 2 ] = black;
            }
            else
            {
                // MF_HORSE CARNAGE
                TVFace* tvFace = &mesh->mapFaces(MAP_SHADING)[i];
                for (int j = 0; j < 3; j++)
                    illums[j] = illumArray[ tvFace->getTVert(flipOrder ? 2 - j : j) ];
                // MF_HORSE CARNAGE
            }
            
            if (alphaMap != nullptr) // if it IS nil, then alpha values are all at the default 1.0
            {
                // MF_HORSE CARNAGE
                TVFace* tvFace = &mesh->mapFaces(MAP_ALPHA)[i];
                for (int j = 0; j < 3; j++)
                    colors[j].a = alphaMap[ tvFace->getTVert(flipOrder ? 2 - j : j) ].x;
                // MF_HORSE CARNAGE
            }

            if (isComposite && !makeAlphaLayer) 
            {
                int index = ((plCompositeMtl *)currMaxMtl)->CanWriteAlpha();
                int j;
                TVFace* tvFaces = mesh->mapFaces(MAP_SHADING);
                for (j = 0; j < 3; j++)
                {
                    switch(index)
                    {
                    case plCompositeMtl::kCompBlendVertexAlpha:
                        break;
                    case plCompositeMtl::kCompBlendVertexIllumRed:
                        colors[j].a = (tvFaces != nullptr ? illumMap[tvFaces[i].getTVert(flipOrder ? 2 - j : j)].x : 1.0f);
                        break;
                    case plCompositeMtl::kCompBlendVertexIllumGreen:
                        colors[j].a = (tvFaces != nullptr ? illumMap[tvFaces[i].getTVert(flipOrder ? 2 - j : j)].y : 1.0f);
                        break;
                    case plCompositeMtl::kCompBlendVertexIllumBlue:
                        colors[j].a = (tvFaces != nullptr ? illumMap[tvFaces[i].getTVert(flipOrder ? 2 - j : j)].z : 1.0f);
                        break;
                    default: // Different channels, thus we flush the alpha to 100 and do alpha through a 2nd layer.
                        colors[j].a = 1.0f;
                        break;
                    }
                }
            }
            // Calculate normal for face
            if( node->HasNormalChan() )
            {
                // Someone has stuffed a requested normal into a map channel.
                // Ignore common sense and use it as is.
                int normChan = node->GetNormalChan();
                TVFace* mapFaces = mesh->mapFaces(normChan);
                if( mapFaces )
                {
                    TVFace* normFace = mapFaces + i;
                    int ii;
                    for( ii = 0; ii < 3; ii++ )
                    {
                        Point3 norm = mesh->mapVerts(normChan)[normFace->getTVert(ii)];
                        normals[ii].Set(norm.x, norm.y, norm.z);
                    }
                }
                else
                {
                    if( fErrorMsg->Set(fWarnBadNormals, node->GetName(), fWarnBadNormalsMsg).CheckAskOrCancel() )
                        fWarnBadNormals = false;
                    fErrorMsg->Set( false );
                    normals[0].Set(0,0,1.f);
                    normals[1] = normals[2] = normals[0];
                }

            }
            else if( node->GetRadiateNorms() )
            {
                int ii;
                for( ii = 0; ii < 3; ii++ )
                {
                    Point3 pos = mesh->getVert(vertIdx[ii]) * otm;
                    pos = pos * invOtm;

                    normals[ii].Set(pos.x, pos.y, pos.z);
                }
            }
            else
            {
                smGroup = smoothAll ? 1 : maxFace->getSmGroup();
                if( smGroup == 0 )
                {
                    hsVector3   v1, v2;
                    v1.Set( &pos[ 1 ], &pos[ 0 ] );
                    v2.Set( &pos[ 2 ], &pos[ 1 ] ); // Hey, MAX does it...see normalCache building above
                    // Note: if flipOrder is set, we have to reverse the order of the cross product, since
                    // we already flipped the order of the points, to match what MAX would get
                    normals[ 0 ] = normals[ 1 ] = normals[ 2 ] = flipOrder ? ( v2 % v1 ) : ( v1 % v2 );
                }
                else
                {
                    normals[ 0 ] = vertNormalCache[ vertIdx[ 0 ] ].GetPlNormal( smGroup );
                    normals[ 1 ] = vertNormalCache[ vertIdx[ 1 ] ].GetPlNormal( smGroup );
                    normals[ 2 ] = vertNormalCache[ vertIdx[ 2 ] ].GetPlNormal( smGroup );
                }
            }
            normals[ 0 ] = vertInvTransMatrix * normals[ 0 ];
            normals[ 1 ] = vertInvTransMatrix * normals[ 1 ];
            normals[ 2 ] = vertInvTransMatrix * normals[ 2 ];

            // Adding normalization here, because we're going to compare them when searching for
            // this vertex to share. mf.
            normals[0].Normalize();
            normals[1].Normalize();
            normals[2].Normalize();

            // The above section of code has just set any bump uv channels incorrectly,
            // but at least they are there. Now we just need to correct the values.
            if( bumpLayIdx[mainMatIndex] >= 0 )
            {
                TVFace* tvFace = mesh->mapFaces(bumpLayChan[mainMatIndex]+1) + i;
                ISetBumpUvs(bumpDuChan[mainMatIndex], vertDPosDuCache[mainMatIndex], tvFace, smGroup, uvs1, uvs2, uvs3);
                ISetBumpUvs(bumpDvChan[mainMatIndex], vertDPosDvCache[mainMatIndex], tvFace, smGroup, uvs1, uvs2, uvs3);
            }

            // Do this here, cause if we do it before we calculate the normals on smoothing group #0, 
            // the normals will be wrong
            for (int j = 0; j < 3; j++)
                pos[ j ] = vert2LMatrix * pos[ j ];

/* We already compute the index, this looks like redundant code - 7/26/01 Bob
            // Get span index
            if( isMultiMat )
            {
                mainMatIndex = maxFace->getMatID();
                if( mainMatIndex >= numMaterials )
                    mainMatIndex = 0;
            }
*/
            if (isComposite)
            {
                // I don't care about flipOrder here... it doesn't affect the index
                float opac[][2] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};

                opac[0][0] = uvs1[numChannels].fX;
                opac[1][0] = uvs2[numChannels].fX;
                opac[2][0] = uvs3[numChannels].fX;
                opac[0][1] = uvs1[numChannels].fY;
                opac[1][1] = uvs2[numChannels].fY;
                opac[2][1] = uvs3[numChannels].fY;
                
                subMatIndex = ((plCompositeMtl *)currMaxMtl)->ComputeMaterialIndex(opac, 3) - 1;
            }
            // Add!
            hsAssert(ourAccumulators[mainMatIndex] != nullptr, "Trying to add a face with an unused sub-material.");
            (*ourAccumulators[mainMatIndex])[subMatIndex]->AddVertex(vertIdx[0], &pos[0], &normals[0], colors[0], illums[0], uvs1);
            (*ourAccumulators[mainMatIndex])[subMatIndex]->AddVertex(vertIdx[1], &pos[1], &normals[1], colors[1], illums[1], uvs2);
            (*ourAccumulators[mainMatIndex])[subMatIndex]->AddVertex(vertIdx[2], &pos[2], &normals[2], colors[2], illums[2], uvs3);

        }


        /// Now go through each accumulator, take any data created and stuff it into a new span
        for( i = 0; i < numMaterials; i++ )
        {
            std::vector<plExportMaterialData> *subMats = ourMaterials[i];
            // A sub material of a MultiMat that never gets used will have a nil value, signifying no spans to export
            if (subMats == nullptr)
                continue;
            for (size_t j = 0; j < subMats->size(); j++)
            {
                plMAXVertexAccumulator *accum = (*ourAccumulators[i])[j];

                // With composite materials, not every accumulator will have faces. Only create spans for the ones that do.
                if (accum->GetVertexCount() == 0)
                    continue;

                plGeometrySpan      *span = new plGeometrySpan;

                span->BeginCreate((*subMats)[j].fMaterial, l2wMatrix, ourFormat);
                span->fLocalToOBB = node->GetLocalToOBB44();
                span->fOBBToLocal = node->GetOBBToLocal44();

                accum->StuffMyData( node, span, mesh, skinData );
                span->fProps |= sharedSpanProps;

                span->fWaterHeight = waterHeight;

                if( (bumpDuChan[i] >= 0) && (bumpDvChan[i] > 0) )
                    span->fLocalUVWChans = (bumpDuChan[i] << 8) | bumpDvChan[i];

                if ((span->fMaterial != nullptr)
                    && (span->fMaterial->GetNumLayers() > 0) 
                    && (span->fMaterial->GetLayer( 0 )->GetState().fBlendFlags & hsGMatState::kBlendMask) ) 
                {
                    span->fProps |= plGeometrySpan::kRequiresBlending;
                }
                if( node->GetForceSortable() )
                    span->fProps |= plGeometrySpan::kRequiresBlending;

                span->EndCreate();

                float minDist, maxDist;
                if( hsMaterialConverter::HasVisDists(node, i, minDist, maxDist) )
                {
                    span->fMinDist = (minDist);
                    span->fMaxDist = (maxDist);
                }

                // If we're not doing preshading later, make sure everything is illuminated so you can see
                if (doPreshading)
                {
                    hsColorRGBA gray;
                    gray.Set(0.5, 0.5, 0.5, 0.0);
                    for( int iVert = 0; iVert < span->fNumVerts; iVert++ )
                        span->StuffVertex( iVert, &white, &gray );
                }

                if( span->fNumVerts > 0 )
                    spanArray.emplace_back(span);
                else
                    delete span;
            }
        }

        // A bit of test hack here. Remind me to nuke it.
        if (node->GetCalcEdgeLens() || node->UserPropExists(_M("XXXWaterColor"))) {
            for (plGeometrySpan* span : spanArray)
                SetWaterColor(span);
        }


        // Now that we have our nice spans, see if they need to be diced up a bit
        int maxFaces, minFaces;
        float maxSize;
        if( node->GetGeoDice(maxFaces, maxSize, minFaces) )
        {
            plGeoSpanDice dice;
            dice.SetMaxFaces(maxFaces);
            dice.SetMaxSize(hsPoint3(maxSize, maxSize, maxSize));
            dice.SetMinFaces(minFaces);
            dice.Dice(spanArray);
        }

        /// Check for overflow (only do it if our initial tests showed there could be overflow; this
        /// is so these tests don't slow down the loop unless absolutely needed).
        // We're going to go ahead and quietly break up the mesh if it needs breaking,
        // because we can probably do a better job of it than production anyway.
        if( checkForOverflow )
        {
            bool needMoreDicing = false;
            for (plGeometrySpan* span : spanArray)
            {
                plAccessGeometry accGeom;
                plAccessSpan accSpan;
                accGeom.AccessSpanFromGeometrySpan(accSpan, span);
                bool destroySpan = false;
                if( accSpan.HasAccessVtx() )
                {
                    if( accSpan.AccessVtx().VertCount() >= plGBufferGroup::kMaxNumVertsPerBuffer )
                    {
                        needMoreDicing = true;
                    }
                }
                if( accSpan.HasAccessTri() )
                {
                    if( accSpan.AccessTri().TriCount() * 3 >= plGBufferGroup::kMaxNumIndicesPerBuffer )
                    {
                        needMoreDicing = true;
                    }
                }
                accGeom.Close(accSpan);
            }
            if( needMoreDicing )
            {
                // Could just dice the ones that need it, but whatever. mf.
                plConst(int) kAutoMaxFaces(5000);
                plConst(float) kAutoMaxSize(10000.f);
                plConst(int) kAutoMinFaces(1000);
                plGeoSpanDice dice;
                dice.SetMaxFaces(kAutoMaxFaces);
                dice.SetMaxSize(hsPoint3(kAutoMaxSize,kAutoMaxSize,kAutoMaxSize));
                dice.SetMinFaces(kAutoMinFaces);
                dice.Dice(spanArray);
            }
        }
        throw (bool)true;
    }
    catch( bool retVal )
    {
        /// Cleanup!
        for (plMAXVertNormal& norm : vertNormalCache)
            norm.DestroyChain();

        for( i = 0; i < numMaterials; i++ )
        {
            if (vertDPosDuCache != nullptr)
            {
                for (plMAXVertNormal& norm : vertDPosDuCache[i])
                    norm.DestroyChain();
            }
            if (vertDPosDvCache != nullptr)
            {
                for (plMAXVertNormal& norm : vertDPosDvCache[i])
                    norm.DestroyChain();
            }

            if (ourAccumulators[i] == nullptr)
                continue;
            for (plMAXVertexAccumulator* accum : *ourAccumulators[i])
            {
                delete accum;
            }
            delete ourMaterials[ i ];
            delete ourAccumulators[ i ];
        }
        delete [] vertDPosDuCache;
        delete [] vertDPosDvCache;
        delete [] colorArray;
        delete [] illumArray;
    
        IDeleteTempGeometry();

        return retVal;
    }

    return true;
    hsGuardEnd; 
}

//// ICreateHexColor /////////////////////////////////////////////////////////

uint32_t  plMeshConverter::ICreateHexColor( float r, float g, float b )
{
    uint32_t      ru, gu, bu, au;


    au = 0xff000000;
    ru = (uint32_t)(r * 255.0f);
    gu = (uint32_t)(g * 255.0f);
    bu = (uint32_t)(b * 255.0f);
    return au | ( ru << 16 ) | ( gu << 8 ) | ( bu );
}

uint32_t  plMeshConverter::ISetHexAlpha( uint32_t color, float alpha)
{
    uint32_t alphaBits = uint32_t(alpha * 255);
    alphaBits <<= 24;
    return color & 0x00ffffff | alphaBits;
}

// Delete temp geo stuff allocated (either directly or indirectly)
// via IGetNodeMesh().
void plMeshConverter::IDeleteTempGeometry()
{
    if (fTriObjToDelete != nullptr)
    {
        fTriObjToDelete->DeleteMe();
        fTriObjToDelete = nullptr;
    }
    if( fMeshToDelete )
    {
        delete fMeshToDelete;
        fMeshToDelete = nullptr;
    }
}

//// IGetNodeMesh ////////////////////////////////////////////////////////////
//  Get the Mesh object attached to a node. Returns nullptr if the node
//  is not a triMesh object

Mesh    *plMeshConverter::IGetNodeMesh( plMaxNode *node )
{
    hsGuardBegin( "plMeshConverter::IGetNodeMesh" );

    auto dbgNodeName = node->GetName();

    fTriObjToDelete = nullptr;
    fMeshToDelete = nullptr;

    // Get da object
    Object *obj = node->EvalWorldState( fConverterUtils.GetTime( fInterface ) ).obj;
    if (obj == nullptr)
        return nullptr;

    if( !obj->CanConvertToType( triObjectClassID ) )
        return nullptr;

    // Convert to triMesh object
    TriObject   *meshObj = (TriObject *)obj->ConvertToType( fConverterUtils.GetTime( fInterface ), triObjectClassID );
    if (meshObj == nullptr)
        return nullptr;

    if( meshObj != obj )
        fTriObjToDelete = meshObj;

    // Get the mesh
    Mesh    *mesh = &(meshObj->mesh);
    if( mesh->getNumFaces() == 0 )
        return nullptr;

    if( node->GetDup2Sided() )
    {
        mesh = IDuplicate2Sided(node, mesh);
        
        IDeleteTempGeometry();

        fMeshToDelete = mesh;
    }

    return mesh;
    hsGuardEnd; 
}

Mesh* plMeshConverter::IDuplicate2Sided(plMaxNode* node, Mesh* mesh)
{
    mesh = new Mesh(*mesh);

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
        return mesh;

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

    return mesh;
}

//// IGenerateUVs ////////////////////////////////////////////////////////////
//  Generates the UV coordinates for the three vertices of a given face. 
//  Returns the number of UV channels. Ripped off of SetUVs() from the old
//  hsMeshConverter.

int     plMeshConverter::IGenerateUVs( plMaxNode *node, Mtl *maxMtl, Mesh *mesh, int faceIdx, int numChan, int numBlend,
                                       hsPoint3 *uvs1, hsPoint3 *uvs2, hsPoint3 *uvs3 )
{
    hsGuardBegin( "plMeshConverter::IGenerateUVs" );

    if( !( maxMtl && ( hsMaterialConverter::Instance().IsMultiMat( maxMtl ) || ( maxMtl->Requirements(-1) & MTLREQ_UV ) ) ) )
    {
        return 0;
    }


    // To avoid transforming the shared UVs while rendering, we will
    // sometimes pretransform them and not share.

    int     j, k;
    Face    *face = &mesh->faces[ faceIdx ];



    if( hsMaterialConverter::Instance().IsMultiMat( maxMtl ) )
    {
        int faceMtlIndex = face->getMatID();
        if( faceMtlIndex >= maxMtl->NumSubMtls() ) // we'll warn in createtrimeshrecur
            faceMtlIndex = 0;
    }

    bool firstWarn = true;
    hsPoint3    pt;

    /// Loop through the vertices
    for( j = 0; j < 3; j++ )
    {
        int chan;
        for( chan = 0; chan < numChan; chan++ )
        {
            if( mesh->mapFaces( chan + 1 ) )
            {
                TVFace* tvFace = &mesh->mapFaces( chan + 1 )[ faceIdx ];
                UVVert* tVerts = mesh->mapVerts( chan + 1 );

                if( firstWarn && fErrorMsg->Set( !tvFace, node->GetName(), "Check mapping on textured objects" ).CheckAndAsk() )
                {
                    firstWarn = false;
                }
                fErrorMsg->Set( false );
                if( !tvFace )
                {
                    continue;
                }

                int tvIdx = tvFace->getTVert( j );

                if( tvIdx >= mesh->getNumMapVerts( chan + 1 ) )
                {
                    static int muteWarn = false;
                    if( !muteWarn )
                    {
                        muteWarn = fErrorMsg->Set(true, node->GetName(), ST::format("Check mapping on channel {}!!!", chan + 1)).CheckAskOrCancel();
                        fErrorMsg->Set( false );
                    }
                    tvIdx = 0;
                }

                UVVert      uv = tVerts[tvIdx];

                // The artists set the 3rd coordinate to help create the mapping,
                // but we never need it at runtime, so let's set it to zero on
                // export and then the vert coder can detect that it doesn't 
                // even need to write it.
                pt.Set( uv.x, 1.0f - uv.y, 0.f );

                if( _isnan( (double)pt.fX ) || _isnan( (double)pt.fY ) || _isnan( (double)pt.fZ ) )
                    pt.Set( 0, 0, 0 );

                switch( j )
                {
                    case 0: uvs1[ chan ] = pt; break;
                    case 1: uvs2[ chan ] = pt; break;
                    case 2: uvs3[ chan ] = pt; break;
                }           
            }
            
        }

        // That takes care of the UVs MAX gives us. Do we need some leftover channels to store our blending info?
        for (k = numChan; k < numChan + numBlend; k++)
        {
            UVVert *alphas = mesh->mapVerts(MAP_ALPHA);
            UVVert *illums = mesh->mapVerts(MAP_SHADING);
            UVVert pt;
            pt.z = 0.0f;

            TVFace* alphaFace = mesh->mapFaces(MAP_ALPHA) ? &mesh->mapFaces(MAP_ALPHA)[faceIdx] : nullptr;
            TVFace* illumFace = mesh->mapFaces(MAP_SHADING) ? &mesh->mapFaces(MAP_SHADING)[faceIdx] : nullptr;

            if (hsMaterialConverter::Instance().IsCompositeMat( maxMtl ))
            {
                plCompositeMtl *compMtl = (plCompositeMtl *)maxMtl;
                IParamBlock2 *pb = maxMtl->GetParamBlockByID(kCompPasses);

                // MF_HORSE CARNAGE


                compMtl->SetOpacityVal(&pt.x, 
                        (alphas == nullptr
                            ? nullptr
                            : &alphas[ alphaFace->getTVert(j) ]),
                        (illums == nullptr
                            ? nullptr
                            : &illums[ illumFace->getTVert(j) ]),
                        pb->GetInt(kCompBlend, 0, 0));
                
                compMtl->SetOpacityVal(&pt.y, 
                    (alphas == nullptr
                        ? nullptr
                        : &alphas[ alphaFace->getTVert(j) ]),
                    (illums == nullptr
                        ? nullptr
                        : &illums[ illumFace->getTVert(j) ]),
                    pb->GetInt(kCompBlend, 0, 1));
                // MF_HORSE CARNAGE
            }
            else // all other materials
            {       
                pt.y = 0.0;
                pt.z = 1.0;
                if (alphas == nullptr || alphaFace == nullptr)
                    pt.x = 1.0;
                else
                    pt.x = alphas[alphaFace->getTVert(j)].x;
            }

            switch( j )
            {
                case 0: uvs1[ k ].fX = pt.x; uvs1[ k ].fY = pt.y; uvs1[ k ].fZ = pt.z; break;
                case 1: uvs2[ k ].fX = pt.x; uvs2[ k ].fY = pt.y; uvs2[ k ].fZ = pt.z; break;
                case 2: uvs3[ k ].fX = pt.x; uvs3[ k ].fY = pt.y; uvs3[ k ].fZ = pt.z; break;
            }           
        }
    }

    return numChan;
    hsGuardEnd;
}

void plMeshConverter::ISetWaterDecEnvUvSrcs(std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                                            std::vector<int16_t>& bumpLayIdx,
                                            std::vector<int16_t>& bumpLayChan,
                                            std::vector<int16_t>& bumpDuChan,
                                            std::vector<int16_t>& bumpDvChan)
{
    bumpLayIdx.resize(ourMaterials.size());
    bumpLayChan.resize(ourMaterials.size());
    bumpDuChan.resize(ourMaterials.size());
    bumpDvChan.resize(ourMaterials.size());

    for (size_t i = 0; i < ourMaterials.size(); i++)
    {
        bumpLayIdx[i] = (int16_t)i;
        bumpLayChan[i] = 0;
        bumpDuChan[i] = 1;
        bumpDvChan[i] = 2;
    }
}

void plMeshConverter::ISetBumpUvSrcs(std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                                     std::vector<int16_t>& bumpLayIdx,
                                     std::vector<int16_t>& bumpLayChan,
                                     std::vector<int16_t>& bumpDuChan,
                                     std::vector<int16_t>& bumpDvChan)
{
    bumpLayIdx.resize(ourMaterials.size());
    bumpLayChan.resize(ourMaterials.size());
    bumpDuChan.resize(ourMaterials.size());
    bumpDvChan.resize(ourMaterials.size());

    for (size_t i = 0; i < ourMaterials.size(); i++)
    {
        bumpLayIdx[i] = -1;
        bumpLayChan[i] = -1;
        bumpDuChan[i] = -1;
        bumpDvChan[i] = -1;

        // The following two lines pretty much rule out composites with bump maps.
        if( !ourMaterials[i] )
            continue;

        if (ourMaterials[i]->size() != 1)
            continue;


        hsGMaterial* ourMat = ourMaterials[i]->front().fMaterial;
        for (size_t j = 0; j < ourMat->GetNumLayers(); j++)
        {
            if( ourMat->GetLayer(j)->GetMiscFlags() & hsGMatState::kMiscBumpLayer )
            {
                bumpLayIdx[i] = (int16_t)j;
                bumpLayChan[i] = ourMat->GetLayer(j)->GetUVWSrc();
            }

            if( ourMat->GetLayer(j)->GetMiscFlags() & hsGMatState::kMiscBumpDu )
                bumpDuChan[i] = ourMat->GetLayer(j)->GetUVWSrc();

            if( ourMat->GetLayer(j)->GetMiscFlags() & hsGMatState::kMiscBumpDv )
                bumpDvChan[i] = ourMat->GetLayer(j)->GetUVWSrc();
        }
    }
}

void plMeshConverter::ISetBumpUvs(int16_t uvChan, std::vector<plMAXVertNormal>& vertDPosDuvCache, TVFace* tvFace, uint32_t smGroup,
                                  hsPoint3* uvs1, hsPoint3* uvs2, hsPoint3* uvs3)
{
    if( uvChan < 0 )
        return;

    uvs1[uvChan] = *(hsPoint3*)&vertDPosDuvCache[tvFace->getTVert(0)].GetPlNormal(smGroup);
    uvs2[uvChan] = *(hsPoint3*)&vertDPosDuvCache[tvFace->getTVert(1)].GetPlNormal(smGroup);
    uvs3[uvChan] = *(hsPoint3*)&vertDPosDuvCache[tvFace->getTVert(2)].GetPlNormal(smGroup);
}

// Determine if we're going to need a uv gradient channel.
// If we do need one, determine which uvw channel needs the gradient.
// Finally, make the gradients, smoothing according to smooth groups (just like vertex normals).
//
// If we decided we needed them, they are in the output arrays, otherwise the output arrays are made empty.
void plMeshConverter::ISmoothUVGradients(plMaxNode* node, Mesh* mesh,
                                         std::vector<std::vector<plExportMaterialData> *>& ourMaterials,
                                         std::vector<int16_t>& bumpLayIdx, std::vector<int16_t>& bumpLayChan,
                                         std::vector<plMAXVertNormal>* vertDPosDuCache, std::vector<plMAXVertNormal>* vertDPosDvCache)
{
    auto dbgNodeName = node->GetName();

    Mtl* mainMtl = hsMaterialConverter::Instance().GetBaseMtl( node );

    bool needsGradientUvs = hsMaterialConverter::Instance().HasBumpLayer(node, mainMtl) || node->GetWaterDecEnv();

    if( needsGradientUvs )
    {
        for (size_t matIdx = 0; matIdx < ourMaterials.size(); matIdx++)
        {
            if( bumpLayIdx[matIdx] >= 0 )
            {
                uint32_t uvwSrc = bumpLayChan[matIdx];
                if( mesh->getNumMapVerts(uvwSrc+1) && mesh->mapVerts(uvwSrc+1) )
                {
                    vertDPosDuCache[matIdx].resize(mesh->getNumMapVerts(uvwSrc+1));
                    vertDPosDvCache[matIdx].resize(mesh->getNumMapVerts(uvwSrc+1));
                }
                else
                {
                    // Ooops. This is probably an error somewhere.
                    hsAssert(false, "Thought we had a valid bump map, but we don't.");
                    bumpLayIdx[matIdx] = -1;
                }
            }
        }

        bool isMultiMat = hsMaterialConverter::Instance().IsMultiMat(mainMtl);
        for (int i = 0; i < mesh->getNumFaces(); i++)
        {
            const plLayerInterface* layer = nullptr;
            size_t matIdx;
            if( isMultiMat )
            {
                int index = mesh->faces[i].getMatID();
                if (index >= mainMtl->NumSubMtls())
                    index = 0;

                matIdx = (size_t)index;
            }
            else
            {
                matIdx = 0;
            }
            if (bumpLayIdx[matIdx] >= 0)
                layer = ourMaterials[matIdx]->front().fMaterial->GetLayer(bumpLayIdx[matIdx]);
            if( layer )
            {
                Point3 dPosDu = IGetUvGradient(node, layer->GetTransform(), layer->GetUVWSrc(),
                                mesh, i, 
                                0);
                Point3 dPosDv = IGetUvGradient(node, layer->GetTransform(), layer->GetUVWSrc(),
                                mesh, i, 
                                1);

// #define MF_BUMP_CHECK_DUXDV
#ifdef MF_BUMP_CHECK_DUXDV
                Point3 duXdv = ::Normalize(dPosDu) ^ ::Normalize(dPosDv);

                Point3 v0 = mesh->verts[ mesh->faces[i].v[ 0 ] ];
                Point3 v1 = mesh->verts[ mesh->faces[i].v[ 1 ] ];
                Point3 v2 = mesh->verts[ mesh->faces[i].v[ 2 ] ];

                Point3 norm = ::Normalize(( v1 - v0 ) ^ ( v2 - v1 ));
                
                Point3 diff = duXdv - norm;

                static int doAgain = false;
                if( doAgain )
                {
                    dPosDu = IGetUvGradient(node, layer->GetTransform(), layer->GetUVWSrc(),
                                    mesh, i, 
                                    0);
                    dPosDv = IGetUvGradient(node, layer->GetTransform(), layer->GetUVWSrc(),
                                    mesh, i, 
                                    1);
                }
#endif // MF_BUMP_CHECK_DUXDV

                if( node->GetWaterDecEnv() )
                {
                    dPosDu.z = dPosDv.z = 0.f;
                }

                // Flip the direction of dPosDv, because we flip textures about V for histerical reasons.
                dPosDv = -dPosDv;

                TVFace* tvFace = &mesh->mapFaces(layer->GetUVWSrc() + 1)[i];
                int j;
                for( j = 0; j < 3; j++ )
                {
                    vertDPosDuCache[matIdx][tvFace->getTVert(j)].AddNormal(dPosDu, mesh->faces[i].smGroup);
                    vertDPosDvCache[matIdx][tvFace->getTVert(j)].AddNormal(dPosDv, mesh->faces[i].smGroup);
                }
            }
        }
        for (size_t matIdx = 0; matIdx < ourMaterials.size(); matIdx++)
        {
            for (size_t i = 0; i < vertDPosDuCache[matIdx].size(); i++)
            {
                vertDPosDuCache[matIdx][i].Normalize();
                vertDPosDvCache[matIdx][i].Normalize();
            }
        }
    }
}

// Get dPos/du into uvws[0], and dPos/dv into uvws[1]. dPos should be in the object's local space.
Point3 plMeshConverter::IGetUvGradient(plMaxNode* node, 
                                        const hsMatrix44& uvXform44, int16_t bmpUvwSrc, // Transform and uvwSrc of layer to gradient
                                        Mesh *mesh, int faceIdx, 
                                        int iUV) // 0 = uvw.x, 1 = uv2.y
{
    Point3 uvwOut(0,0,0);
    if( bmpUvwSrc < 0 )
        return uvwOut; // Not Error.

    if( bmpUvwSrc >= mesh->getNumMaps() )
        return uvwOut; // Error?

    TVFace* tvFace = &mesh->mapFaces(bmpUvwSrc + 1)[faceIdx];
    UVVert* tVerts = mesh->mapVerts(bmpUvwSrc + 1);
    if( !tvFace )
        return uvwOut; // Error?
    if( !tVerts )
        return uvwOut; // Error?

    Matrix3 v2l = node->GetVertToLocal();

    bool flipOrder = v2l.Parity();
    int vtxIdx = 0;
    int vtxNext = flipOrder ? 2 : 1;
    int vtxLast = flipOrder ? 1 : 2;

    // Get the three verts, v0-v2, where v0 is the corner in question.
    Face* face = &mesh->faces[faceIdx];             
    Point3 v0 = v2l * mesh->verts[face->getVert(vtxIdx)];
    Point3 v1 = v2l * mesh->verts[face->getVert(vtxNext)];
    Point3 v2 = v2l * mesh->verts[face->getVert(vtxLast)];

    // Get the three uvs, uv0-uv2, matching above verts.
    if( tvFace->getTVert(vtxIdx) >= mesh->getNumMapVerts(bmpUvwSrc + 1) )
        return uvwOut; // Error?
    if( tvFace->getTVert(vtxNext) >= mesh->getNumMapVerts(bmpUvwSrc + 1) )
        return uvwOut; // Error?
    if( tvFace->getTVert(vtxLast) >= mesh->getNumMapVerts(bmpUvwSrc + 1) )
        return uvwOut; // Error?

    Matrix3 uvwXform = plMaxNodeBase::Matrix44ToMatrix3(uvXform44);

    Point3 uv0 = uvwXform * tVerts[tvFace->getTVert(vtxIdx)];
    Point3 uv1 = uvwXform * tVerts[tvFace->getTVert(vtxNext)];
    Point3 uv2 = uvwXform * tVerts[tvFace->getTVert(vtxLast)];


    const float kRealSmall = 1.e-6f;
    // First, look for degenerate cases. 
    // If (uvn - uvm)[!iUV] == 0
    //      then (vn - vm) is tangent in  iUV dimension
    // Just be careful about direction, since (vn-vm) may be opposite direction from
    // increasing iUV.
    int iNotUV = !iUV;
    float del = uv0[iNotUV] - uv1[iNotUV];
    if( fabs(del) < kRealSmall )
    {
        if( uv0[iUV] - uv1[iUV] < 0 )
            uvwOut = v1 - v0;
        else
            uvwOut = v0 - v1;
        return uvwOut;
    }
    del = uv2[iNotUV] - uv1[iNotUV];
    if( fabs(del) < kRealSmall )
    {
        if( uv2[iUV] - uv1[iUV] < 0 )
            uvwOut = v1 - v2;
        else
            uvwOut = v2 - v1;
        return uvwOut;
    }
    del = uv2[iNotUV] - uv0[iNotUV];
    if( fabs(del) < kRealSmall )
    {
        if( uv2[iUV] - uv0[iUV] < 0 )
            uvwOut = v0 - v2;
        else
            uvwOut = v2 - v0;
        return uvwOut;
    }

    // Okay, none of the edges are along the dU gradient. That's good, because
    // it means we don't have to worry about divides by zero in what we're about
    // to do.
    del = uv0[iNotUV] - uv1[iNotUV];
    del = 1.f / del;
    Point3 v0Mv1 = v0 - v1;
    v0Mv1 *= del;
    float v0uv = (uv0[iUV] - uv1[iUV]) * del;

    del = uv2[iNotUV] - uv1[iNotUV];
    del = 1.f / del;
    Point3 v2Mv1 = v2 - v1;
    v2Mv1 *= del;
    float v2uv = (uv2[iUV] - uv1[iUV]) * del;

    if( v0uv > v2uv )
        uvwOut = v0Mv1 - v2Mv1;
    else
        uvwOut = v2Mv1 - v0Mv1;

    return uvwOut;
}

//// IGetUVTransform /////////////////////////////////////////////////////////
//  Gets the UV transform matrix for the given channel.

void    plMeshConverter::IGetUVTransform( plMaxNode *node, Mtl *mtl, Matrix3 *uvTransform, int which )
{
    hsGuardBegin( "plMeshConverter::IGetUVTransform" );

    uvTransform->IdentityMatrix();

    if( !mtl )
        return;

    Texmap* texMap = hsMaterialConverter::Instance().GetUVChannelBase(node, mtl, which);

    if( !texMap )
        return;

    BitmapTex *bitmapTex = (BitmapTex *)texMap;

#ifndef NDEBUG
    MSTR className;
    texMap->GetClassName(className);
    if (className != MSTR(_M("Bitmap")) && className != MSTR(_M("Plasma Layer")) && className != MSTR(_M("Plasma Layer Dbg.")))
        return;
#endif // NDEBUG

    StdUVGen *uvGen = bitmapTex->GetUVGen();

    uvGen->GetUVTransform(*uvTransform);

    // We're going to munge the internals of the matrix here, but we won't need to worry about
    // the flags (ident etc.) because we're not changing the character of the matrix (unless
    // it's a pure translation by an integer amount, which is a no-op).
    // We also don't have to worry about preserving unit offsets for animation, because the
    // output of this function is used as a static matrix (for uv generation). We could check
    // and not do this on animated transforms or something. mf
    // Note that we can only do this if the texture wraps (not clamps)
    if( !hsMaterialConverter::Instance().PreserveUVOffset(mtl) )
    {
        MRow* data = uvTransform->GetAddr();
        int i;
        for( i = 0; i < 2; i++ )
        {
            if( fabsf(data[3][i]) >= 1.f )
            {
                data[3][i] -= float(int(data[3][i]));
            }
        }
    }
    hsGuardEnd;
}

//////////////////////////////////////////////////////////////////////////////
//// Helper Class Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// plMAXVertexAccNode Constructor //////////////////////////////////////////

plMAXVertexAccNode::plMAXVertexAccNode( const hsPoint3 *point, const hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, int numChannels, const hsPoint3 *uvs, uint32_t index )
{
    int     i;


    fPoint = *point;
    fNormal = *normal;
    fColor = color;
    fIllum = illum;
    for( i = 0; i < numChannels; i++ )
        fUVs[ i ] = uvs[ i ];
    fNumChannels = numChannels;
    fIndex = index;

    fNext = nullptr;
}

//// IsEqual /////////////////////////////////////////////////////////////////
//  Determines whether the node matches the values given.

bool        plMAXVertexAccNode::IsEqual( const hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, const hsPoint3 *uvs )
{
    int     i;


    if( color != fColor || !( *normal == fNormal ) || illum != fIllum )
        return false;

    for( i = 0; i < fNumChannels; i++ )
    {
        if( !( uvs[ i ] == fUVs[ i ] ) )
            return false;
    }

    return true;
}

//// plMAXVertexAccumulator Constructor & Destructor /////////////////////////

plMAXVertexAccumulator::plMAXVertexAccumulator( int numOrigPoints, int numChannels )
{
    fNumPoints = numOrigPoints;
    fNumChannels = numChannels;

    fPointList = new plMAXVertexAccNodePtr[ fNumPoints ];
    memset( fPointList, 0, sizeof( plMAXVertexAccNode * ) * fNumPoints );

    fIndices.clear();
    fInverseVertTable.clear();
    fNumVertices = 0;
}

plMAXVertexAccumulator::~plMAXVertexAccumulator()
{
    int                 i;
    plMAXVertexAccNode  *node;

    
    for( i = 0; i < fNumPoints; i++ )
    {
        while (fPointList[i] != nullptr)
        {
            node = fPointList[ i ]->fNext;
            delete fPointList[ i ];
            fPointList[ i ] = node;
        }
    }

    delete [] fPointList;
}

//// AddVertex ///////////////////////////////////////////////////////////////
//  Adds a vertex to this accumulator. If it already exists, adds the normals
//  together and stores the old index; if not, creates a new node and adds
//  an index for it. In the end, fIndices will be our final index buffer,
//  while fInverseVertTable is basically an array telling us where to get
//  our vertices when building the vertex buffer.

void    plMAXVertexAccumulator::AddVertex( int index, hsPoint3 *point, hsVector3 *normal, const hsColorRGBA& color, const hsColorRGBA& illum, hsPoint3 *uvs )
{
    plMAXVertexAccNode      *node;


    // See if one exists in this list
    for (node = fPointList[index]; node != nullptr; node = node->fNext)
    {
        if( node->IsEqual( normal, color, illum, uvs ) )
        {
            // Found match! Accumulate
            fIndices.emplace_back(node->fIndex);
            return;
        }
    }


    /// Adding new
    node = new plMAXVertexAccNode( point, normal, color, illum, fNumChannels, uvs, fNumVertices );
    fInverseVertTable.emplace_back(index);
    fIndices.emplace_back(fNumVertices++);

    node->fNext = fPointList[ index ];
    fPointList[ index ] = node;
}

//// StuffMyData /////////////////////////////////////////////////////////////
//  Stuffs the data into the given span. Assumes the span has already begun 
//  creation.

void    plMAXVertexAccumulator::StuffMyData( plMaxNode* maxNode, plGeometrySpan *span, Mesh* mesh, ISkinContextData* skinData )
{
    int                     i, j, origIdx;
    plMAXVertexAccNode      *node;
    hsPoint3                *uvs[ plGeometrySpan::kMaxNumUVChannels ];  

    auto dbgNodeName = maxNode->GetName();
    TempWeightInfo          *weights = nullptr;

    /// Precalculate the weights if necessary
    weights = new TempWeightInfo[ fNumPoints ];
    if (skinData != nullptr)
    {
        for( i = 0; i < fNumPoints; i++ )
        {
            IFindSkinWeights( skinData, i, weights[ i ].fWeights, &weights[ i ].fIndices );

            /// Debug checking stuff, for testing only
            // Wrong. Geometry span depends on this
            switch( span->fFormat & plGeometrySpan::kSkinWeightMask )
            {
            case plGeometrySpan::kSkin1Weight:
                weights[ i ].fWeights[1] = -1.f;
                break;
            case plGeometrySpan::kSkin2Weights:
                weights[ i ].fWeights[2] = -1.f;
                break;
            case plGeometrySpan::kSkin3Weights:
                weights[ i ].fWeights[3] = -1.f;
                break;
            default:
                hsAssert(false, "Shouldn't have gotten here");
                
                break;
            }
        }
    }
    else if( maxNode->NumBones() )
    {
        IFindAllUserSkinWeights(maxNode, mesh, weights);
    }
    else
    {
        for( i = 0; i < fNumPoints; i++ )
        {
            weights[ i ].fWeights[ 0 ] = weights[ i ].fWeights[ 1 ] = 
                                        weights[ i ].fWeights[ 2 ] = weights[ i ].fWeights[ 3 ] = -1.0f;
            weights[ i ].fIndices = 0;
        }
    }
    
    plMaxBoneMap *boneMap = maxNode->GetBoneMap();
    if (boneMap)
    {
        for (i = 0; i < fNumPoints; i++)
        {
            uint8_t indices[4];
            indices[0] = (weights[i].fIndices) & 0xff;
            indices[1] = (weights[i].fIndices >> 8) & 0xff;
            indices[2] = (weights[i].fIndices >> 16) & 0xff;
            indices[3] = (weights[i].fIndices >> 24) & 0xff;
            
            for (j = 0; j < 4; j++)
            {
                //if (weights[i].fWeights[j] >= 0)
                //{
                    if (indices[j] != 0)
                    {
                        plMaxNodeBase *bone = maxNode->GetBone(indices[j] - 1);
                        auto dbgBoneName = bone->GetName();
                        indices[j] = boneMap->GetIndex(bone) + 1;
                    }
                //}
            }
            weights[i].fIndices = (indices[0]) |
                                  (indices[1] << 8) |
                                  (indices[2] << 16) |
                                  (indices[3] << 24);
        }
    }
    
    float maxWgt = 0;
    float penWgt = 0;
    int16_t maxIdx = -1;
    int16_t penIdx = -1;
    // Find the highest two weighted bones. We'll use just these two to calculate our bounds.
    for( i = 0; i < fNumPoints; i++ )
    {
        if( weights[i].fIndices )
        {
            for( j = 0; j < 4; j++ )
            {
                if( weights[i].fWeights[j] < 0 )
                    break;
                if( weights[i].fWeights[j] > maxWgt )
                {
                    penWgt = maxWgt;
                    penIdx = maxIdx;

                    maxWgt = weights[i].fWeights[j];
                    maxIdx = (weights[i].fIndices >> (j*8)) & 0xff;
                }
                else
                if( weights[i].fWeights[j] > penWgt )
                {
                    penWgt = weights[i].fWeights[j];
                    penIdx = (weights[i].fIndices >> (j*8)) & 0xff;
                }
            }
        }
    }
    if( maxIdx < 0 )
        maxIdx = 0;
    if( penIdx < 0 )
        penIdx = maxIdx;
    span->fMaxBoneIdx = maxIdx;
    span->fPenBoneIdx = penIdx;

    /// Stuff the verts
    for( i = 0; i < plGeometrySpan::kMaxNumUVChannels; i++)
        uvs[i] = nullptr;

    for( i = 0; i < fNumVertices; i++ )
    {
        origIdx = fInverseVertTable[ i ];

        // origIdx gets us the list, but we need to know which node in the list
        for (node = fPointList[origIdx]; node != nullptr; node = node->fNext)
        {
            if( node->fIndex == i )
            {
                // Found it! output this one
                hsPoint3        normal;
            
                node->fNormal.Normalize();
                normal.Set( node->fNormal.fX, node->fNormal.fY, node->fNormal.fZ );
                for( j = 0; j < fNumChannels; j++ )
                    uvs[ j ] = &node->fUVs[ j ];
    
                /// Add!
                span->AddVertex( &node->fPoint, &normal, node->fColor, node->fIllum, uvs,
                                   weights[ origIdx ].fWeights[ 0 ], weights[ origIdx ].fWeights[ 1 ], 
                                   weights[ origIdx ].fWeights[ 2 ], weights[ origIdx ].fIndices );
                break;
            }
        }
        hsAssert(node != nullptr, "Invalid accumulator table when stuffing buffers!");
    }

    /// Now stuff the indices
    for (uint32_t index : fIndices)
        span->AddIndex(index);

    if (weights != nullptr)
        delete [] weights;
}

// IFindAllUserSkinWeights
// Like IFindSkinWeights, but doesn't use Max's native skinning (e.g. ISkinContextData).
// Rather, someone has put a bone (currently only support one) on this plMaxNode,
// and told us what vertex channel to find the weight for that bone in.
void plMAXVertexAccumulator::IFindAllUserSkinWeights( plMaxNode* node, Mesh* mesh, TempWeightInfo weights[])
{
    auto dbgNodeName = node->GetName();

    int iMap = MAP_ALPHA; // FISH HACK, till we stuff the src channel into the max node.
    iMap = 66;

    int iChan = 1; // FISH HACK, get this one stuffed too. Could probably or them into the same
    //thing or something, but who cares. Gotta stop with the ethers.

    UVVert *wgtMap = mesh->mapVerts(iMap);  
    int numWgtVerts = mesh->getNumMapVerts(iMap);

    TVFace* mapFaces = mesh->mapFaces(iMap);

    if( wgtMap && mapFaces )
    {
        Face* faces = mesh->faces;

        int i;
        for( i = 0; i < mesh->getNumFaces(); i++ )
        {
            int j;
            for( j = 0; j < 3; j++ )
            {
                int iVtx = faces[i].getVert(j);
                int iTvtx = mapFaces[i].getTVert(j);

                weights[iVtx].fWeights[2] = weights[iVtx].fWeights[3] = 0;
                weights[iVtx].fWeights[0] = 1.f - wgtMap[iTvtx][iChan];
                if( weights[iVtx].fWeights[0] > 1.f )
                    weights[iVtx].fWeights[0] = 1.f;
                else if( weights[iVtx].fWeights[0] < 0 )
                    weights[iVtx].fWeights[0] = 0;
                weights[iVtx].fWeights[1] = 1.f - weights[iVtx].fWeights[0];
                weights[iVtx].fIndices = 1 << 8;
            }
        }
    }
    else
    {
        int i;
        for( i = 0; i < mesh->getNumVerts(); i++ )
        {
            weights[i].fWeights[1] = weights[i].fWeights[2] = weights[i].fWeights[3] = 0;
            weights[i].fWeights[0] = 1.f;
            weights[i].fIndices = 1 << 8;
        }
    }
}

//// IFindSkinWeights ////////////////////////////////////////////////////////
//  Finds the biggest weights (up to 4) for the given vertex index and returns
//  them, along with a uint32_t specifying the indices for each.

void    plMAXVertexAccumulator::IFindSkinWeights( ISkinContextData *skinData, 
                                                  int vertex, 
                                                  float *weights, uint32_t *indices )
{
    float   tempWs[ 4 ], tempW, t;
    uint32_t  idxs[ 4 ], tempIdx, tI;
    int     i, j;

    tempWs[ 0 ] = tempWs[ 1 ] = tempWs[ 2 ] = tempWs[ 3 ] = 0;
    idxs[ 0 ] = idxs[ 1 ] = idxs[ 2 ] = idxs[ 3 ] = 0;

    int boneCount = skinData->GetNumAssignedBones( vertex);

    if( boneCount )
    {
        float defWgt = 1.f;
        for( i = 0; i < boneCount; i++ )
        {
            /// Grab the weight and index for this bone
            tempW = skinData->GetBoneWeight( vertex, i );
            defWgt -= tempW;

            // GetAssignedBone will assert unpredictably if the weight is 0.0f (exactly)
            // It will usually then return 0 for the bone index, but sometimes 1
            // In any case, the bone index should not matter at that point.
            // Without walking through all the downstream code, seems to work ok.
            if(tempW > 0.0f)
                tempIdx = skinData->GetAssignedBone( vertex, i ) + 1;
            else
                tempIdx = 0;

    //  float hi = skinData->GetBoneWeight( vertex, tempIdx );

            /// Slide it in to our list
            for( j = 0; j < 4; j++ )
            {
                if( tempWs[ j ] < tempW )
                {
                    t = tempWs[ j ];
                    tempWs[ j ] = tempW;
                    tempW = t;

                    tI = idxs[ j ];
                    idxs[ j ] = tempIdx;
                    tempIdx = tI;
                }
            }
        }

        // This isn't really what we want. If the weights add up to less than
        // 1.f, the remainder is the un-skinned, un-boned Transform.
        // If the weights add up to more than 1.f, someone probably screwed up,
        // but we'll deal with it gracefully by normalizing.
        if( defWgt > 0 )
        {
            tempW = defWgt;
            tempIdx = 0;

            /// Slide it in to our list
            for( j = 0; j < 4; j++ )
            {
                if( tempWs[ j ] < tempW )
                {
                    t = tempWs[ j ];
                    tempWs[ j ] = tempW;
                    tempW = t;

                    tI = idxs[ j ];
                    idxs[ j ] = tempIdx;
                    tempIdx = tI;
                }
            }
        }

        t = tempWs[ 0 ] + tempWs[ 1 ] + tempWs[ 2 ] + tempWs[ 3 ];
        t = 1.0f / t;

        weights[ 0 ] = tempWs[ 0 ] * t;
        weights[ 1 ] = tempWs[ 1 ] * t;
        weights[ 2 ] = tempWs[ 2 ] * t;
        weights[ 3 ] = tempWs[ 3 ] * t;
    }
    else
    {
        weights[ 0 ] = 1.f;
        idxs[ 0 ] = 0;

        weights[1] = weights[2] = weights[3] = 0;
        idxs[1] = idxs[2] = idxs[3] = 0;
    }

    if( skinData->GetNumAssignedBones( vertex ) < 2 )
    {
        if( idxs[0] )
        {
            float tWgt = weights[0];
            int tIdx = idxs[0];

            weights[0] = weights[1];
            idxs[0] = idxs[1];

            weights[1] = tWgt;
            idxs[1] = tIdx;
        }
    }
    
    *indices = ( idxs[ 0 ] & 0xff ) | ( ( idxs[ 1 ] & 0xff ) << 8 ) | 
               ( ( idxs[ 2 ] & 0xff ) << 16 ) | ( ( idxs[ 3 ] & 0xff ) << 24 );
}

int plMAXVertexAccumulator::GetVertexCount()
{
//  return fIndices.GetCount();
    return fNumVertices;
}

static void SetWaterColor(plGeometrySpan* span)
{
    plAccessGeometry accGeom;
    // First, set up our access, iterators and that mess.
    plAccessSpan acc;
    accGeom.AccessSpanFromGeometrySpan(acc, span);
    if( !acc.HasAccessTri() )
    {
        plAccessGeometry::Instance()->Close(acc);
        return;
    }
    plAccessTriSpan& tri = acc.AccessTri();
    plAccTriIterator triIter(&tri);

    const int nVerts = tri.VertCount();
    // Now, set up our accumulators
    std::vector<float> lens;
    lens.resize(nVerts);
    std::vector<float> wgts;
    wgts.resize(nVerts);

    // For each triangle
    for( triIter.Begin(); triIter.More(); triIter.Advance() )
    {
        // This area thing seems like a good robust idea, but it doesn't really
        // take into account the fact that the sampling frequency is really determined
        // by the weakest link, or in this case the longest link. Experimenting
        // with alternatives.
        // Actually, I just realized that the area way kind of sucks, because, 
        // as a parallelogram gets less and less rectangular, the area goes down
        // even as the longest edge (the diagonal) gets longer.
        float lenSq20 = hsVector3(&triIter.Position(2), &triIter.Position(0)).MagnitudeSquared();
        float lenSq10 = hsVector3(&triIter.Position(1), &triIter.Position(0)).MagnitudeSquared();
        float lenSq21 = hsVector3(&triIter.Position(2), &triIter.Position(1)).MagnitudeSquared();
        float len = lenSq20;
        if( len < lenSq10 )
            len = lenSq10;
        if( len < lenSq21 )
            len = lenSq21;
        len = sqrt(len);

        lens[triIter.RawIndex(0)] += len;
        wgts[triIter.RawIndex(0)] += 1.f;

        lens[triIter.RawIndex(1)] += len;
        wgts[triIter.RawIndex(1)] += 1.f;

        lens[triIter.RawIndex(2)] += len;
        wgts[triIter.RawIndex(2)] += 1.f;

    }
    // For each vert
    int iVert;
    for( iVert = 0; iVert < nVerts; iVert++ )
    {
        if( wgts[iVert] > 0.f )
            lens[iVert] /= wgts[iVert];
        
        wgts[iVert] = 0.f; // We'll use them again on smoothing.
    }
    // Now we might want to smooth this out some
    // This can be repeated for any degree of smoothing
    std::vector<float> smLens;
    smLens.resize(nVerts);
    // For each triangle
    for( triIter.Begin(); triIter.More(); triIter.Advance() )
    {
        int i;
        // For each edge
        for( i = 0; i < 3; i++ )
        {
            int iVert = triIter.RawIndex(i);
            int iVertNext = triIter.RawIndex(i < 2 ? i+1 : 0);
            int iVertLast = triIter.RawIndex(i ? i-1 : 2);
            smLens[iVert] += lens[iVert];
            wgts[iVert] += 1.f;

            const float kSmooth(8.f);
            smLens[iVertNext] += lens[iVert] * kSmooth;
            wgts[iVertNext] += kSmooth;

            smLens[iVertLast] += lens[iVert] * kSmooth;
            wgts[iVertLast] += kSmooth;
        }
    }
    lens.swap(smLens);
    // For each vert
    for( iVert = 0; iVert < nVerts; iVert++ )
    {
        if( wgts[iVert] > 0.f )
            lens[iVert] /= wgts[iVert];
        
        wgts[iVert] = 0.f; // We'll use them again on smoothing.
    }

    plConst(float) kNumLens(4.f);
    // Okay, we have smoothed lengths. We just need to 
    // iterate over the vertices and stuff 1/len into the alpha channel
    // For each vert
    plAccDiffuseIterator colIter(&tri);
    for( iVert = 0, colIter.Begin(); colIter.More(); iVert++, colIter.Advance() )
    {
        hsColorRGBA multCol;
        hsColorRGBA addCol;
        span->ExtractInitColor(iVert, &multCol, &addCol);

        // Get the vert color
        hsColorRGBA col = colIter.DiffuseRGBA();
        col = multCol;
        
        col.a = lens[iVert] > 0.f ? 1.f / (kNumLens * lens[iVert]) : 1.f;

        // Stuff color back in.
        *colIter.Diffuse32() = col.ToARGB32();
    }

    // Close up the access span.
    accGeom.Close(acc);
}
