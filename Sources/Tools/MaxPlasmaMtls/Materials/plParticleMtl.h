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
#ifndef PL_PARTICLEMTL_H
#define PL_PARTICLEMTL_H

class Bitmap;

#define PARTICLE_MTL_CLASS_ID Class_ID(0x26df05ff, 0x60660749)

extern TCHAR *GetString(int id);

class plParticleMtl : public Mtl
{
protected:
    IParamBlock2    *fBasicPB;
    Interval        fIValid;

public:
    IMtlParams *fIMtlParams;

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
        kOpacity,
        kColor,
        kWidth,
        kHeight,
        kXTiles,
        kYTiles,
        kNormal,
        kBlend,
        kOrientation,
        kBitmap,
        kTexmap,
        kColorAmb,
        kNoFilter
    };
    enum
    {
        kBlendNone,
        kBlendAlpha,
        kBlendAdd
    };
    enum
    {
        kOrientVelocity,
        kOrientUp,
        kOrientVelStretch,
        kOrientVelFlow
    };

    enum
    {
        kNormalViewFacing,
        kNormalUp,
        kNormalNearestLight,
        kNormalFromCenter,
        kNormalVelUpVel,
        kEmissive,
        kNumNormalOptions
    };

    static const char *NormalStrings[];

    plParticleMtl(BOOL loading);
    void DeleteThis() override { delete this; }

    //From Animatable
    Class_ID ClassID() override { return PARTICLE_MTL_CLASS_ID; }
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
        PartID& partID,  RefMessage message MAX_REF_PROPAGATE) override;

    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    MSTR SubAnimName(int i) override;

    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;

    int NumParamBlocks() override;
    IParamBlock2* GetParamBlock(int i) override;
    IParamBlock2* GetParamBlockByID(BlockID id) override;


//  void SetParamDlg(ParamDlg *dlg);

//  void SetNumSubTexmaps(int num);

    Control *GetAmbColorController();
    Control *GetColorController();
    Control *GetOpacityController();
    Control *GetWidthController();
    Control *GetHeightController();

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

#endif //PL_PARTICLEMTL_H
