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
#ifndef PL_DECALMTL_H
#define PL_DECALMTL_H

#include "plPassMtlBase.h"

#define DECAL_MTL_CLASS_ID  Class_ID(0x691d2257, 0x419d629e)

extern TCHAR *GetString(int id);

class plDecalMtl : public plPassMtlBase
{
protected:

    void        ICloneRefs(plPassMtlBase *target, RemapDir &remap) override;

public:

    enum
    {
        kRefBasic,
        kRefLayers,
        kRefAdv,
        kRefAnim,
    };
    
    enum Blocks
    {
        kBlkBasic,
        kBlkLayers,
        kBlkAdv,
        kBlkAnim,
    };

    plDecalMtl(BOOL loading);
    void DeleteThis() override { delete this; }

    //From Animatable
    Class_ID ClassID() override { return DECAL_MTL_CLASS_ID; }
    SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }
    void GetClassName(MSTR& s) override;

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
    void Update(TimeValue t, Interval& valid) override;
    Interval Validity(TimeValue t) override;

    void NotifyChanged();

    BOOL SupportsMultiMapsInViewport() override { return FALSE; }
    void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) override;

    // Shade and displacement calculation
    void Shade(ShadeContext& sc) override;
    void ShadeWithBackground(ShadeContext &sc, Color background);
    float EvalDisplacement(ShadeContext& sc) override;
    Interval DisplacementValidity(TimeValue t) override;

    RefTargetHandle GetReference(int i) override;
    void            SetReference(int i, RefTargetHandle rtarg) override;

    // SubTexmap access methods
    int NumSubTexmaps() override;
    Texmap* GetSubTexmap(int i) override;
    void SetSubTexmap(int i, Texmap *m) override;
    MSTR GetSubTexmapSlotName(int i) override;
    MSTR GetSubTexmapTVName(int i);
    int SubTexmapOn(int i) override;
      
    BOOL SetDlgThing(ParamDlg* dlg) override;

    RefTargetHandle Clone(RemapDir &remap) override;

    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    MSTR SubAnimName(int i) override;

    int NumParamBlocks() override;
    IParamBlock2* GetParamBlock(int i) override;
    IParamBlock2* GetParamBlockByID(BlockID id) override;


//  void SetParamDlg(ParamDlg *dlg);

//  void SetNumSubTexmaps(int num);

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

    bool    HasAlpha() override;
    // Massive list of inherited accessor functions for ParamBlock data

    // Advanced Block
    int     GetBasicWire() override;
    int     GetMeshOutlines() override;
    int     GetTwoSided() override;
    int     GetSoftShadow() override;
    int     GetNoProj() override;
    int     GetVertexShade() override;
    int     GetNoShade() override;
    int     GetNoFog() override;
    int     GetWhite() override;
    int     GetZOnly() override;
    int     GetZClear() override;
    int     GetZNoRead() override;
    int     GetZNoWrite() override;
    int     GetZInc() override;
    int     GetAlphaTestHigh() override;

    // Animation block
    const MCHAR*  GetAnimName() override;
    int     GetAutoStart() override;
    int     GetLoop() override;
    const MCHAR*  GetAnimLoopName() override;
    int     GetEaseInType() override;
    float   GetEaseInMinLength() override;
    float   GetEaseInMaxLength() override;
    float   GetEaseInNormLength() override;
    int     GetEaseOutType() override;
    float   GetEaseOutMinLength() override;
    float   GetEaseOutMaxLength() override;
    float   GetEaseOutNormLength() override;
    int     GetUseGlobal() override;
    const MCHAR* GetGlobalVarName() override;

    // Basic block
    int     GetColorLock() override;
    Color   GetAmbColor() override;
    Color   GetColor() override;
    int     GetOpacity() override;
    int     GetEmissive() override;
    int     GetUseSpec() override;
    int     GetShine() override;
    Color   GetSpecularColor() override;
    Control *GetPreshadeColorController() override;
    Control *GetAmbColorController() override;
    Control *GetOpacityController() override;
    Control *GetSpecularColorController() override;
    int     GetDiffuseColorLock() override;
    Color   GetRuntimeColor() override;
    Control *GetRuntimeColorController() override;
    
    // Layer block
    Texmap *GetBaseLayer() override;
    int     GetTopLayerOn() override;
    Texmap *GetTopLayer() override;
    int     GetLayerBlend() override;
    int     GetOutputAlpha() override;
    int     GetOutputBlend() override;
};

#endif //PL_DECALMTL_H
