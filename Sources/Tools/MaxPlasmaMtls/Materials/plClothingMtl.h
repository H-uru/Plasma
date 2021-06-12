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
#ifndef PL_CLOTHINGMTL_H
#define PL_CLOTHINGMTL_H

#include <vector>

class Bitmap;
class plClothingItem;
class plMaxNode;
class plClothingElement;
class Texmap;
namespace ST { class string; }

#define CLOTHING_MTL_CLASS_ID Class_ID(0x792c6de4, 0x1f952b65)

extern TCHAR *GetString(int id);

class plClothingTileset
{
public:
    char *fName;
    std::vector<plClothingElement *> fElements;

    plClothingTileset();
    ~plClothingTileset();

    void SetName(char *name);
    void AddElement(plClothingElement *element);
};

class plClothingMtl : public Mtl
{
protected:
    IParamBlock2    *fBasicPB;
    Interval        fIValid;

public:
    IMtlParams *fIMtlParams;

    static const char *LayerStrings[];
    static const uint8_t LayerToPBIdx[];

    enum
    {
        kRefBasic,
    };
    enum
    {
        kBlkBasic,
    };

    enum // Param block indicies
    {
        kTileset,
        kTexmap,
        kDescription,
        kThumbnail, 
        kLayer,
        kTexmapSkin,
        kTexmap2,
        kDefault,
        kCustomTextSpecs,
        kTexmapBase,
        kTexmapSkinBlend1,
        kTexmapSkinBlend2,
        kTexmapSkinBlend3,
        kTexmapSkinBlend4,
        kTexmapSkinBlend5,
        kTexmapSkinBlend6,
        kDefaultTint1,
        kDefaultTint2,
        kForcedAcc,
    };

    enum
    {
        kMaxTiles = 4,
    };

    enum
    {
        kBlendNone,
        kBlendAlpha,
        //kBlendAdd,
    };

    static const UINT32 ButtonConstants[kMaxTiles];
    static const UINT32 TextConstants[kMaxTiles * 2];

    std::vector<plClothingTileset *> fTilesets;
    std::vector<plClothingElement *> fElements;
    virtual void InitTilesets();
    virtual void ReleaseTilesets();
    plClothingElement *FindElementByName(const ST::string &name) const;

    int GetTilesetIndex() { return fBasicPB->GetInt(ParamID(kTileset)); }
    Texmap *GetTexmap(int index, int layer);
    Texmap *GetThumbnail() { return fBasicPB->GetTexmap(ParamID(kThumbnail)); }
    const MCHAR* GetDescription() { return fBasicPB->GetStr(ParamID(kDescription)); }
    const MCHAR* GetCustomText() { return fBasicPB->GetStr(ParamID(kCustomTextSpecs)); }
    bool GetDefault() { return fBasicPB->GetInt(ParamID(kDefault)) != 0; }
    Color GetDefaultTint1() { return fBasicPB->GetColor(plClothingMtl::kDefaultTint1); }
    Color GetDefaultTint2() { return fBasicPB->GetColor(plClothingMtl::kDefaultTint2); }
    const MCHAR* GetForcedAccessoryName() { return fBasicPB->GetStr(ParamID(kForcedAcc)); }

    plClothingMtl(BOOL loading);
    void DeleteThis() override { delete this; }

    //From Animatable
    Class_ID ClassID() override { return CLOTHING_MTL_CLASS_ID; }
    SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }
    void GetClassName(TSTR& s) override;

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
    void Update(TimeValue t, Interval& valid) override;
    Interval Validity(TimeValue t) override;
    void Reset() override;

    void NotifyChanged();

    BOOL SupportsMultiMapsInViewport() override { return FALSE; }
    void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override;

    // Shade and displacement calculation
    void Shade(ShadeContext& sc) override;
    void ShadeWithBackground(ShadeContext &sc, Color background);
    float EvalDisplacement(ShadeContext& sc) override;
    Interval DisplacementValidity(TimeValue t) override;

    // SubTexmap access methods
    int NumSubTexmaps() override;
    Texmap* GetSubTexmap(int i) override;
    void SetSubTexmap(int i, Texmap *m) override;
    MSTR GetSubTexmapSlotName(int i) override;
    MSTR GetSubTexmapTVName(int i);
    
    BOOL SetDlgThing(ParamDlg* dlg) override;

    // Loading/Saving
    IOResult Load(ILoad *iload) override;
    IOResult Save(ISave *isave) override;

    RefTargetHandle Clone(RemapDir &remap) override;
    RefResult NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
        PartID& partID, RefMessage message MAX_REF_PROPAGATE) override;

    int NumSubs() override { return 0; }
    Animatable* SubAnim(int i) override { return nullptr; }
    MSTR SubAnimName(int i) override { return _M(""); }

    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;

    int NumParamBlocks() override;
    IParamBlock2* GetParamBlock(int i) override;
    IParamBlock2* GetParamBlockByID(BlockID id) override;


    // From MtlBase and Mtl
    void SetAmbient(Color c, TimeValue t) override;
    void SetDiffuse(Color c, TimeValue t) override;
    void SetSpecular(Color c, TimeValue t) override;
    void SetShininess(float v, TimeValue t) override;
    Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE) override;
    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) override;
    Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE) override;
    float GetXParency(int mtlNum=0, BOOL backFace=FALSE) override;
    float GetShininess(int mtlNum=0, BOOL backFace=FALSE) override;
    float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) override;
    float WireSize(int mtlNum=0, BOOL backFace=FALSE) override;

    ULONG   Requirements(int subMtlNum) override;
};

#endif //PL_ClothingMTL_H
