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
#ifndef __PLCOMPMTL__H
#define __PLCOMPMTL__H

#include "Max.h"
#include "../resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define COMP_MTL_CLASS_ID Class_ID(0x237c422d, 0x64ab6371)

class plCompositeMtlDlg;

class plCompositeMtl : public Mtl
{
protected:
    IParamBlock2    *fPassesPB;
    Interval        fIValid;
    plCompositeMtlDlg *fMtlDlg;

public:
    enum
    {
        kCompPasses,
        kCompOn,
        kCompBlend,
        kCompUVChannels,
        kCompLayerCounts
    };

    // Make sure to pair up each blend mode with an inverse after it
    // (This way we check for an inverse blend by doing an odd/even check.)
    enum BlendMethod // These should match up in order with the blend strings
    {
        kCompBlendVertexAlpha,
        kCompBlendInverseVtxAlpha,
        kCompBlendVertexIllumRed,
        kCompBlendInverseVtxIllumRed,
        kCompBlendVertexIllumGreen,
        kCompBlendInverseVtxIllumGreen,
        kCompBlendVertexIllumBlue,
        kCompBlendInverseVtxIllumBlue,

        kCompNumBlendMethods
    };  

    static const char *BlendStrings[];

    enum { kRefPasses };
    enum { kBlkPasses };

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
    void Update(TimeValue t, Interval& valid);
    Interval Validity(TimeValue t);
    void Reset();

    void NotifyChanged();

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

    // Shade and displacement calculation
    void Shade(ShadeContext& sc);
    float EvalDisplacement(ShadeContext& sc); 
    Interval DisplacementValidity(TimeValue t);     

    // SubTexmap access methods
    int NumSubMtls();
    Mtl* GetSubMtl(int i);
    void SetSubMtl(int i, Mtl *m);
    TSTR GetSubMtlSlotName(int i);
    TSTR GetSubMtlTVName(int i);
    
    BOOL SetDlgThing(ParamDlg* dlg);
    plCompositeMtl(BOOL loading);

    // Loading/Saving
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);

    //From Animatable
    Class_ID ClassID() { return COMP_MTL_CLASS_ID; }        
    SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
    void GetClassName(TSTR& s) { s = GetString(IDS_COMP_MTL); }

    RefTargetHandle Clone(RemapDir &remap);
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID, RefMessage message);

    int NumSubs();
    Animatable* SubAnim(int i); 
    TSTR SubAnimName(int i);

    int NumRefs();
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

    int NumParamBlocks();
    IParamBlock2* GetParamBlock(int i);
    IParamBlock2* GetParamBlockByID(BlockID id);

    void DeleteThis() { delete this; }

    void SetParamDlg(ParamDlg *dlg);

    int ComputeMaterialIndex(float opac[][2], int vertCount);
    int GetBlendStyle(int index);
    int CanWriteAlpha();
    bool IsInverseBlend(int blend) const { return blend & 1; }
    int RemoveInverse(int blend) { return blend - (blend & 1); }
    //void SetNumSubMtls(int num);
    void SetOpacityVal(float *pt, UVVert *alphas, UVVert *illums, int method);
    //DllExport int UVChannelsNeeded(bool makeAlphaLayer);
};

#endif // __PLCOMPMTL__H
