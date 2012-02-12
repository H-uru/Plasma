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
#ifndef __HSMATERIALCONVERTER_H
#define __HSMATERIALCONVERTER_H

#include "HeadSpin.h"
#include "hsTemplates.h"

#include "Max.h"

class Interface;

class hsStream;
class hsScene;
class hsGMaterial;
class plLayer;
class plLayerInterface;
class hsGBitmapClass;
class hsGQTLayer;
class hsGAVILayer;
class hsGBinkLayer;
class hsConverterUtils;
class hsGAnimLayer;
class plBitmap;
class plMipmap;
class plErrorMsg;
class plString;

class Mtl;
class Texmap;
class plMaxNode;
class StdUVGen;
class BitmapTex;
class StdMat;
class TSTR;
class Animatable;
class Bitmap;
class plLocation;


class plLayerTex;
class plBitmapData;
class plCubicRenderTarget;
class plStaticEnvLayer;
class plDynamicEnvLayer;
class plDynamicTextLayer;
class plPassMtlBase;
class plClothingItem;
class plClothingMtl;
class hsBitVector;

class plExportMaterialData
{
public:
    uint32_t fNumBlendChannels;
    hsGMaterial *fMaterial;
};

class hsMaterialConverter
{
private:
    hsMaterialConverter();

public:
    ~hsMaterialConverter();
    static hsMaterialConverter& Instance();

    void Init(hsBool save, plErrorMsg *msg);

    void FreeMaterialCache(const char* path);

    static void GetUsedMaterials(plMaxNode* node, hsBitVector& used);
    static hsBool IsTwoSided(Mtl* m, int iFace);
    static hsBool PreserveUVOffset(Mtl* m);
    static hsBool IsMultiMat(Mtl *m);
    static hsBool IsMultipassMat(Mtl *m);
    static hsBool IsHsMaxMat(Mtl *m);
    static hsBool IsDecalMat(Mtl *m);
    static hsBool IsCompositeMat(Mtl *m);
    static hsBool IsParticleMat(Mtl *m);
    static hsBool IsHsEnvironMapMat(Mtl *m);
    static hsBool IsClothingMat(Mtl *m);
//    static hsBool IsPortalMat(Mtl *m);
    static hsBool HasAnimatedTextures(Texmap* texMap);
    static hsBool HasAnimatedMaterial(plMaxNode* node);
    static hsBool IsAnimatedMaterial(Mtl* mtl);
    static hsBool HasMaterialDiffuseOrOpacityAnimation(plMaxNode* node, Mtl* mtl=nil);
    static hsBool HasEmissiveLayer(plMaxNode* node, Mtl* mtl=nil);
    static hsBool IsWaterLayer(plMaxNode* node, Texmap* texMap);
    static hsBool IsFireLayer(plMaxNode* node, Texmap* texMap);
    static hsBool IsAVILayer(Texmap*  texMap);
    static hsBool IsQTLayer(Texmap*  texMap);
    static hsBool IsBinkLayer(Texmap* texMap);
//  static hsBool IsEnvironMapped(plMaxNode *node);
//    static hsBool IsPortal(plMaxNode* node);
    static hsBool ForceNoUvsFlatten(plMaxNode* node);
//    static hsBool IsRenderProc(Mtl* mtl);
    static Mtl* GetBaseMtl(Mtl* mtl);
    static Mtl* GetBaseMtl(plMaxNode* node);
    static int GetCoordMapping(StdUVGen *uvgen);
    static void GetNodesByMaterial(Mtl *mtl, hsTArray<plMaxNode*> &out);

    static uint32_t     VertexChannelsRequestMask(plMaxNode* node, int iSubMtl, Mtl* mtl);
    static uint32_t     VertexChannelsRequiredMask(plMaxNode* node, int iSubMtl);
    static int          NumVertexOpacityChannelsRequired(plMaxNode* node, int iSubMtl);
    static uint32_t     ColorChannelsUseMask(plMaxNode* node, int iSubMtl);
    static int          MaxUsedUVWSrc(plMaxNode* node, Mtl* mtl);


    static hsBool       IsBumpLayer(Texmap* texMap);
    static hsBool       IsBumpMtl(Mtl* mtl);
    static hsBool       HasBumpLayer(plMaxNode* node, Mtl* mtl);
    static BitmapTex*   GetBumpLayer(plMaxNode* node, Mtl* mtl);

    static hsBool       HasVisDists(plMaxNode* node, Mtl* subMtl, float& minDist, float& maxDist);
    static hsBool       HasVisDists(plMaxNode* node, int iSubMtl, float& minDist, float& maxDist);

    static hsBool   IMustBeUniqueMaterial( Mtl *mtl );
    static hsBool   IMustBeUniqueLayer( Texmap *layer );

    static Mtl* FindSubMtlByName(TSTR& name, Animatable* anim);
    Mtl* FindSceneMtlByName(TSTR& name);

    hsTArray<plExportMaterialData> *CreateMaterialArray(Mtl *maxMaterial, plMaxNode *node, uint32_t multiIndex);

    // true if last material creation changed MAX time, invalidating current mesh
    hsBool ChangedTimes() { return fChangedTimes; }

    Texmap *GetUVChannelBase(plMaxNode *node, Mtl* mtl, int which);

    hsBool ClearDoneMaterials(plMaxNode* node);

    int GetMaterialArray(Mtl *mtl, plMaxNode* node, hsTArray<hsGMaterial*>& out, uint32_t multiIndex = 0 );
    int GetMaterialArray(Mtl *mtl, hsTArray<hsGMaterial*>& out, uint32_t multiIndex = 0);
    void CollectConvertedMaterials(Mtl *mtl, hsTArray<hsGMaterial *> &out);

    plClothingItem *GenerateClothingItem(plClothingMtl *mtl, const plLocation &loc);
    hsGMaterial*    AlphaHackVersion(plMaxNode* node, Mtl* mtl, int subIndex); // used by DynamicDecals
    hsGMaterial*    NonAlphaHackVersion(plMaxNode* node, Mtl* mtl, int subIndex);
    hsGMaterial*    AlphaHackPrint(plMaxNode* node, Texmap* baseTex, uint32_t blendFlags);
    hsGMaterial*    NonAlphaHackPrint(plMaxNode* node, Texmap* baseTex, uint32_t blendFlags);

    plMipmap* GetStaticColorTexture(Color c, plLocation &loc); // Creates a 4x4 texture of the specified solid color;

enum {
    kColorRedBlack          = 0x1,
    kColorRedGrey           = 0x2,
    kColorRedWhite          = 0x4,
    kColorRed               = kColorRedBlack | kColorRedGrey | kColorRedWhite,

    kColorGreenBlack        = 0x8,
    kColorGreenGrey         = 0x10,
    kColorGreenWhite        = 0x20,
    kColorGreen             = kColorGreenBlack | kColorGreenGrey | kColorGreenWhite,

    kColorBlueBlack         = 0x40,
    kColorBlueGrey          = 0x80,
    kColorBlueWhite         = 0x100,
    kColorBlue              = kColorBlueBlack | kColorBlueGrey | kColorBlueWhite,

    kColor                  = kColorRed | kColorGreen | kColorBlue,

    kIllumRedBlack          = 0x200,
    kIllumRedGrey           = 0x400,
    kIllumRedWhite          = 0x800,
    kIllumRed               = kIllumRedBlack | kIllumRedGrey | kIllumRedWhite,

    kIllumGreenBlack        = 0x1000,
    kIllumGreenGrey         = 0x2000,
    kIllumGreenWhite        = 0x4000,
    kIllumGreen             = kIllumGreenBlack | kIllumGreenGrey | kIllumGreenWhite,

    kIllumBlueBlack         = 0x8000,
    kIllumBlueGrey          = 0x10000,
    kIllumBlueWhite         = 0x20000,
    kIllumBlue              = kIllumBlueBlack | kIllumBlueGrey | kIllumBlueWhite,

    kIllum                  = kIllumRed | kIllumGreen | kIllumBlue,

    kAlphaBlack             = 0x40000,
    kAlphaGrey              = 0x80000,
    kAlphaWhite             = 0x100000,
    kAlpha                  = kAlphaBlack | kAlphaGrey | kAlphaWhite,

    kAllChannels            = kColor | kIllum | kAlpha      // Adjust if more channels added.
};

    // All this to catch duplicate mats with same name.  Sigh.
    struct DoneMaterialData
    {
        DoneMaterialData() : fHsMaterial(nil), fMaxMaterial(nil), fNode(nil), 
            fSubMultiMat(false), fOwnedCopy(false) { }

        hsGMaterial         *fHsMaterial;
        Mtl                 *fMaxMaterial;
        plMaxNode           *fNode;
        hsBool              fSubMultiMat;
        hsBool              fOwnedCopy;
        hsBool              fRuntimeLit;
        uint32_t            fSubMtlFlags;
        int                 fNumUVChannels;
        hsBool              fMakeAlphaLayer;
    };

private:
    enum {
        kWarnedNoMoreDub                = 0x1,
        kWarnedNoMoreMult               = 0x2,
        kWarnedNoMoreBitmapLoadErr      = 0x4,
        kWarnedSubMulti                 = 0x8,
        kWarnedCompMtlBadBlend          = 0x10,
        kWarnedNoLayers                 = 0x20,
        kWarnedTooManyUVs               = 0x40,
        kWarnedAlphaAddCombo            = 0x80,
        kWarnedNoBaseTexture            = 0x100,
        kWarnedNoUpperTexture           = 0x200,
        kWarnedUpperTextureMissing      = 0x400,
        kWarnedMissingClothingTexture   = 0x800,
        kWarnedBadAnimSDLVarName        = 0x1000,
    };

    DoneMaterialData* IFindDoneMaterial(DoneMaterialData& done);
    hsBool IClearDoneMaterial(Mtl* mtl, plMaxNode* node);

    hsGMaterial *IAddDefaultMaterial(plMaxNode *node);
    plMipmap *IGetUVTransTexture(plMaxNode *node, hsBool useU = true);
    void IInsertSingleBlendLayer(plMipmap *texture, hsGMaterial *mat, plMaxNode *node, 
                                 int layerIdx, int UVChan);

    hsGMaterial *ICreateMaterial(Mtl *mtl, plMaxNode *node, const plString &name, int subIndex, int numUVChannels, hsBool makeAlphaLayer);
    hsGMaterial *IProcessMaterial(Mtl *mtl, plMaxNode *node, const plString &name, int UVChan, int subMtlFlags = 0);

    // ... calls one of:
    hsGMaterial *IProcessMultipassMtl(Mtl *mtl, plMaxNode *node, const plString &name, int UVChan);
    hsGMaterial *IProcessCompositeMtl(Mtl *mtl, plMaxNode *node, const plString &name, int UVChan, int subMtlFlags);
    hsGMaterial *IProcessParticleMtl(Mtl *mtl, plMaxNode *node, const plString &name);
    hsBool IProcessPlasmaMaterial(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, const plString& namePrefix);

    hsGMaterial* IInsertDoneMaterial(Mtl *mtl, hsGMaterial *hMat, plMaxNode *node, hsBool isMultiMat, 
                             hsBool forceCopy, hsBool runtimeLit, uint32_t subMtlFlags, int numUVChannels, hsBool makeAlphaLayer);

    void        IInsertBumpLayers(plMaxNode* node, hsGMaterial* mat, int bumpLayerIdx);
    void        IInsertBumpLayers(plMaxNode* node, hsGMaterial* mat);
    plLayer*    IMakeBumpLayer(plMaxNode* node, const plString& nameBase, hsGMaterial* mat, uint32_t miscFlag);
    plMipmap*   IGetBumpLutTexture(plMaxNode* node);

    hsBool      IHasSubMtl(Mtl* base, Mtl* sub);
    int         IFindSubIndex(plMaxNode* node, Mtl* mtl);

    // NOTE: each insert function potentially modifies the layers, 
    // so make sure you own the material copy before calling these
    void IInsertAlphaBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int UVChan,
                                    hsBool makeAlphaLayer);
    void IInsertMultipassBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int UVChan,
                                        hsBool makeAlphaLayer);
    void IInsertCompBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int subMtlFlags, 
                                   int UVChan, hsBool makeAlphaLayer);

    
    void IAddLayerToMaterial(hsGMaterial *mat, plLayerInterface *layer);
#if 0   // Out for now...
    void IInitAttrSurface(hsGLayer *hLay, StdMat *stdMtl, plMaxNode *node);
    void IInitAttrTexture(plMaxNode *node, Mtl *mtl, hsGLayer* hLay, Texmap *texMap, char *nodeName);
    void IInitAttrLayer(hsGLayer* hLay, Mtl *mtl, Texmap* layer, plMaxNode* node);
#endif
    // ... and so forth
    hsBool IUVGenHasDynamicScale(plMaxNode* node, StdUVGen *uvGen);
    void IScaleLayerOpacity(plLayer* hLay, float scale);

    hsGMaterial *ICheckForProjectedTexture(plMaxNode *node);
    hsGMaterial *IWrapTextureInMaterial(Texmap *texMap, plMaxNode *node);

    BMM_Color_64 ICubeSample(Bitmap *bitmap[6], double phi, double theta);
    void IBuildSphereMap(Bitmap *bitmap[6], Bitmap *bm);
#if 0 // DEFER_ANIM_MAT
    void IProcessAnimMaterial(BitmapTex *bitmapTex, hsGAnimLayer* at, uint32_t texFlags, uint32_t procFlags);
#endif // DEFER_ANIM_MAT
    static hsBool ITextureTransformIsAnimated(Texmap *texmap);
    static hsBool IHasAnimatedControllers(Animatable* anim);
    static hsBool IIsAnimatedTexmap(Texmap* texmap);

    static uint32_t       ICheckPoints(const Point3& p0, const Point3& p1, const Point3& p2, const Point3& p3,
                                             int chan,
                                             uint32_t mBlack, uint32_t mGrey, uint32_t mWhite);
    static uint32_t       ICheckPoints(const Point3& p0, const Point3& p1, const Point3& p2,
                                             int chan,
                                             uint32_t mBlack, uint32_t mGrey, uint32_t mWhite);

    void                    IAppendWetLayer(plMaxNode* node, hsGMaterial* mat);
    static plBitmap*        IGetFunkyRamp(plMaxNode* node, uint32_t funkyType);
    static void             IAppendFunkyLayer(plMaxNode* node, Texmap* texMap, hsGMaterial* mat);
    static hsBool           IHasFunkyOpacity(plMaxNode* node, Texmap* texMap);
    static uint32_t         IGetFunkyType(Texmap* texMap);
    static uint32_t         IGetOpacityRanges(plMaxNode* node, Texmap* texMap, float& tr0, float& op0, float& op1, float& tr1);

    Interface           *fInterface;
    hsConverterUtils&   fConverterUtils;
    hsBool              fSave;
    plErrorMsg          *fErrorMsg;

    int32_t             fSubIndex;
    hsBool              fChangedTimes;

    char                *fNodeName;
    uint32_t            fWarned;


    DoneMaterialData            fLastMaterial;
    hsTArray<DoneMaterialData>  fDoneMaterials;

    hsBool IsMatchingDoneMaterial(DoneMaterialData *dmd, 
                                  Mtl *mtl, hsBool isMultiMat, uint32_t subMtlFlags, hsBool forceCopy, hsBool runtimeLit,
                                  plMaxNode *node, int numUVChannels, hsBool makeAlphaLayer);

    void        ISortDoneMaterials(hsTArray<DoneMaterialData*>& doneMats);
    hsBool      IEquivalent(DoneMaterialData* one, DoneMaterialData* two);
    void        IPrintDoneMat(hsStream* stream, const char* prefix, DoneMaterialData* doneMat);
    void        IPrintDoneMaterials(const char* path, hsTArray<DoneMaterialData*>& doneMats);
    void        IGenMaterialReport(const char* path);

public:
    // Apologies all around, but I need this list for dumping some export warnings. mf
    const hsTArray<struct DoneMaterialData>& DoneMaterials() { return fDoneMaterials; }

    //hsBool             CheckValidityOfSDLVarAnim(plPassMtlBase *mtl, char *varName, plMaxNode *node);
};

extern hsMaterialConverter gMaterialConverter;

#endif