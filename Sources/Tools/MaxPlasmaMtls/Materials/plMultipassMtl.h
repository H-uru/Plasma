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
#ifndef __PLMAXMTL__H
#define __PLMAXMTL__H

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define MULTIMTL_CLASS_ID Class_ID(0x3f687a, 0x28c62bd7)

class plMultipassMtlDlg;

class plMultipassMtl : public Mtl
{
protected:
    IParamBlock2    *fPassesPB;
    Interval        fIValid;
    plMultipassMtlDlg *fMtlDlg;

public:
    enum { kRefPasses };
    enum { kBlkPasses };

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
    void Update(TimeValue t, Interval& valid) override;
    Interval Validity(TimeValue t) override;
    void Reset() override;

    void NotifyChanged();

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

    // Shade and displacement calculation
    void Shade(ShadeContext& sc) override;
    float EvalDisplacement(ShadeContext& sc) override;
    Interval DisplacementValidity(TimeValue t) override;

    // SubTexmap access methods
    int NumSubMtls() override;
    Mtl* GetSubMtl(int i) override;
    void SetSubMtl(int i, Mtl *m) override;
    MSTR GetSubMtlSlotName(int i MAX_NAME_LOCALIZED2 MAX_NAME_LOCALIZED_DEFAULT) override;
    MSTR GetSubMtlTVName(int i);
    
    BOOL SetDlgThing(ParamDlg* dlg);
    plMultipassMtl(BOOL loading);

    // Loading/Saving
    IOResult Load(ILoad *iload) override;
    IOResult Save(ISave *isave) override;

    //From Animatable
    Class_ID ClassID() override { return MULTIMTL_CLASS_ID; }
    SClass_ID SuperClassID() override { return MATERIAL_CLASS_ID; }
    void GetClassName(MSTR& s MAX_NAME_LOCALIZED2) MAX24_CONST override;

    RefTargetHandle Clone(RemapDir &remap) override;
    RefResult NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
        PartID& partID, RefMessage message MAX_REF_PROPAGATE) override;

    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    MSTR SubAnimName(int i MAX_NAME_LOCALIZED2) override;

    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;

    int NumParamBlocks() override;
    IParamBlock2* GetParamBlock(int i) override;
    IParamBlock2* GetParamBlockByID(BlockID id) override;

    void DeleteThis() override { delete this; }

    void SetParamDlg(ParamDlg *dlg);

    void SetNumSubMtls(int num);
};

#endif // __PLMAXMTL__H
