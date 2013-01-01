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

#include "hsTemplates.h"

class Bitmap;
class plClothingItem;
class plMaxNode;
class plClothingElement;
class Texmap;

#define CLOTHING_MTL_CLASS_ID Class_ID(0x792c6de4, 0x1f952b65)

extern TCHAR *GetString(int id);

class plClothingTileset
{
public:
    char *fName;
    hsTArray<plClothingElement *> fElements;

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

    hsTArray<plClothingTileset *> fTilesets;
    hsTArray<plClothingElement *> fElements;
    virtual void InitTilesets();
    virtual void ReleaseTilesets();
    plClothingElement *FindElementByName(char *name);

    int GetTilesetIndex() { return fBasicPB->GetInt(ParamID(kTileset)); }
    DllExport Texmap *GetTexmap(int index, int layer);
    Texmap *GetThumbnail() { return fBasicPB->GetTexmap(ParamID(kThumbnail)); }
    const char *GetDescription() { return fBasicPB->GetStr(ParamID(kDescription)); }
    const char *GetCustomText() { return fBasicPB->GetStr(ParamID(kCustomTextSpecs)); }
    bool GetDefault() { return fBasicPB->GetInt(ParamID(kDefault)) != 0; }
    Color GetDefaultTint1() { return fBasicPB->GetColor(plClothingMtl::kDefaultTint1); }
    Color GetDefaultTint2() { return fBasicPB->GetColor(plClothingMtl::kDefaultTint2); }
    const char *GetForcedAccessoryName() { return fBasicPB->GetStr(ParamID(kForcedAcc)); }

    plClothingMtl(BOOL loading);
    void DeleteThis() { delete this; }

    //From Animatable
    Class_ID ClassID() { return CLOTHING_MTL_CLASS_ID; }        
    SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
    void GetClassName(TSTR& s);

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
    void Update(TimeValue t, Interval& valid);
    Interval Validity(TimeValue t);
    void Reset();

    void NotifyChanged();

    BOOL SupportsMultiMapsInViewport() { return FALSE; }
    void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);

    // Shade and displacement calculation
    void Shade(ShadeContext& sc);
    void ShadeWithBackground(ShadeContext &sc, Color background);
    float EvalDisplacement(ShadeContext& sc); 
    Interval DisplacementValidity(TimeValue t);     

    // SubTexmap access methods
    int NumSubTexmaps();
    Texmap* GetSubTexmap(int i);
    void SetSubTexmap(int i, Texmap *m);
    TSTR GetSubTexmapSlotName(int i);
    TSTR GetSubTexmapTVName(int i);
    
    BOOL SetDlgThing(ParamDlg* dlg);

    // Loading/Saving
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);

    RefTargetHandle Clone( RemapDir &remap );
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID,  RefMessage message);

    int NumSubs() { return 0; }
    Animatable* SubAnim(int i) { return nil; } 
    TSTR SubAnimName(int i) { return ""; }

    int NumRefs();
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

    int NumParamBlocks();
    IParamBlock2* GetParamBlock(int i);
    IParamBlock2* GetParamBlockByID(BlockID id);


    // From MtlBase and Mtl
    void SetAmbient(Color c, TimeValue t);      
    void SetDiffuse(Color c, TimeValue t);      
    void SetSpecular(Color c, TimeValue t);
    void SetShininess(float v, TimeValue t);
    Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
    Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
    float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
    float GetShininess(int mtlNum=0, BOOL backFace=FALSE);      
    float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
    float WireSize(int mtlNum=0, BOOL backFace=FALSE);

    ULONG   Requirements( int subMtlNum );
};

#endif //PL_ClothingMTL_H
