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
#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "plMultipassMtl.h"
#include "plPassMtl.h"
#include "plMultipassMtlPB.h"
#include "plMultipassMtlDlg.h"

class plMultipassClassDesc : public plMaxClassDesc<ClassDesc2>
{
public:
    int             IsPublic() override     { return TRUE; }
    void*           Create(BOOL loading) override { return new plMultipassMtl(loading); }
    const MCHAR*    ClassName() override    { return GetString(IDS_MULTI_MTL); }
    SClass_ID       SuperClassID() override { return MATERIAL_CLASS_ID; }
    Class_ID        ClassID() override      { return MULTIMTL_CLASS_ID; }
    const MCHAR*    Category() override     { return nullptr; }
    const MCHAR*    InternalName() override { return _T("PlasmaMultipass"); }
    HINSTANCE       HInstance() override    { return hInstance; }
};
static plMultipassClassDesc plMultipassMtlDesc;
ClassDesc2* GetMultiMtlDesc() { return &plMultipassMtlDesc; }

#include "plMultipassMtlPB.cpp"

plMultipassMtl::plMultipassMtl(BOOL loading) : fPassesPB()
{
    plMultipassMtlDesc.MakeAutoParamBlocks(this);

    if (!loading) 
        Reset();

    SetNumSubMtls(1);
}

void plMultipassMtl::IGetClassName(MSTR& s) const
{
    s = GetString(IDS_MULTI_MTL);
}

void plMultipassMtl::Reset() 
{
    fIValid.SetEmpty();
}

ParamDlg* plMultipassMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
    fMtlDlg = new plMultipassMtlDlg(hwMtlEdit, imp, this);

    return fMtlDlg; 
}

void plMultipassMtl::SetParamDlg(ParamDlg *dlg)
{
    fMtlDlg = (plMultipassMtlDlg*)dlg;
}

BOOL plMultipassMtl::SetDlgThing(ParamDlg* dlg)
{
    if (dlg == fMtlDlg)
    {
        fMtlDlg->SetThing(this);
        return TRUE;
    }

    return FALSE;
}

Interval plMultipassMtl::Validity(TimeValue t)
{
    Interval valid = FOREVER;       

/*  for (int i = 0; i < fSubTexmap.Count(); i++) 
    {
        if (fSubTexmap[i]) 
            valid &= fSubTexmap[i]->Validity(t);
    }
*/  
//  float u;
//  fPBlock->GetValue(pb_spin,t,u,valid);
    return valid;
}

/*===========================================================================*\
 |  Subanim & References support
\*===========================================================================*/

int plMultipassMtl::NumSubs()
{
    return NumSubMtls();
}

MSTR plMultipassMtl::ISubAnimName(int i)
{
    return GetSubMtlSlotName(i);
}

Animatable* plMultipassMtl::SubAnim(int i)
{
    return GetSubMtl(i);
}

int plMultipassMtl::NumRefs()
{
    return 1;
}

RefTargetHandle plMultipassMtl::GetReference(int i)
{
    if (i == kRefPasses)
        return fPassesPB;

    return nullptr;
}

void plMultipassMtl::SetReference(int i, RefTargetHandle rtarg)
{
    if (i == kRefPasses)
        fPassesPB = (IParamBlock2 *)rtarg;
}

int plMultipassMtl::NumParamBlocks()
{
    return 1;
}

IParamBlock2 *plMultipassMtl::GetParamBlock(int i)
{
    if (i == kRefPasses)
        return fPassesPB;

    return nullptr;
}

IParamBlock2 *plMultipassMtl::GetParamBlockByID(BlockID id)
{
    if (fPassesPB->ID() == id)
        return fPassesPB;

    return nullptr;
}

RefResult plMultipassMtl::NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
   PartID& partID, RefMessage message MAX_REF_PROPAGATE)
{
    switch (message)
    {
    case REFMSG_CHANGE:
        fIValid.SetEmpty();
        if (hTarget == fPassesPB)
        {
            ParamID changingParam = fPassesPB->LastNotifyParamID();
            fPassesPB->GetDesc()->InvalidateUI(changingParam);
        }
        break;
    }

    return REF_SUCCEED;
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plMultipassMtl::NumSubMtls()
{
    return fPassesPB->GetInt(kMultCount);
}

Mtl *plMultipassMtl::GetSubMtl(int i)
{
    if (i < NumSubMtls())
        return fPassesPB->GetMtl(kMultPasses, 0, i);

    return nullptr;
}

void plMultipassMtl::SetSubMtl(int i, Mtl *m)
{
    if (i < NumSubMtls())
        fPassesPB->SetValue(kMultPasses, 0, m, i);
}

MSTR plMultipassMtl::IGetSubMtlSlotName(int i)
{
    MSTR str;
    str.printf(_M("Pass %d"), i+1);
    return str;
}

MSTR plMultipassMtl::GetSubMtlTVName(int i)
{
    return GetSubMtlSlotName(i);
}


/*===========================================================================*\
 |  Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult plMultipassMtl::Save(ISave *isave)
{ 
    IOResult res;
    isave->BeginChunk(MTL_HDR_CHUNK);
    res = MtlBase::Save(isave);
    if (res!=IO_OK) return res;
    isave->EndChunk();

    return IO_OK;
}   

IOResult plMultipassMtl::Load(ILoad *iload)
{
    IOResult res;
    int id;
    while (IO_OK==(res=iload->OpenChunk()))
    {
        switch(id = iload->CurChunkID())
        {
            case MTL_HDR_CHUNK:
                res = MtlBase::Load(iload);
                break;
        }
        iload->CloseChunk();
        if (res!=IO_OK) 
            return res;
    }

    return IO_OK;
}


/*===========================================================================*\
 |  Updating and cloning
\*===========================================================================*/

RefTargetHandle plMultipassMtl::Clone(RemapDir &remap)
{
    plMultipassMtl *mnew = new plMultipassMtl(FALSE);
    *((MtlBase*)mnew) = *((MtlBase*)this); 
    mnew->ReplaceReference(kRefPasses, remap.CloneRef(fPassesPB));

    mnew->fIValid.SetEmpty();   
    BaseClone(this, mnew, remap);

    return (RefTargetHandle)mnew;
}

void plMultipassMtl::NotifyChanged() 
{
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plMultipassMtl::Update(TimeValue t, Interval& valid) 
{   
    if (!fIValid.InInterval(t))
    {
        fIValid.SetInfinite();
//      fPassesPB->GetValue(kMtlLayLayer1On, t, fMapOn[0], fIValid);

        for (int i = 0; i < NumSubMtls(); i++)
        {
            if (GetSubMtl(i))
                GetSubMtl(i)->Update(t, fIValid);
        }
    }

    valid &= fIValid;
}

/*===========================================================================*\
 |  Determine the characteristics of the material
\*===========================================================================*/
void plMultipassMtl::SetAmbient(Color c, TimeValue t) {}        
void plMultipassMtl::SetDiffuse(Color c, TimeValue t) {}        
void plMultipassMtl::SetSpecular(Color c, TimeValue t) {}
void plMultipassMtl::SetShininess(float v, TimeValue t) {}
                
Color plMultipassMtl::GetAmbient(int mtlNum, BOOL backFace) { return Color(0,0,0); }
Color plMultipassMtl::GetDiffuse(int mtlNum, BOOL backFace) { return Color(0,0,0); }
Color plMultipassMtl::GetSpecular(int mtlNum, BOOL backFace)    { return Color(0,0,0); }
float plMultipassMtl::GetXParency(int mtlNum, BOOL backFace)    { return 0.0f; }
float plMultipassMtl::GetShininess(int mtlNum, BOOL backFace)   { return 0.0f; }
float plMultipassMtl::GetShinStr(int mtlNum, BOOL backFace) { return 0.0f; }
float plMultipassMtl::WireSize(int mtlNum, BOOL backFace)       { return 0.0f; }

/*===========================================================================*\
 |  Actual shading takes place
\*===========================================================================*/

void plMultipassMtl::Shade(ShadeContext& sc) 
{
    // Get the background color
    Color backColor, backTrans;
    backColor.Black();
    backTrans.White();

    int count = NumSubMtls();
    for (int i = 0; i < count; i++)
    {
        if (fPassesPB->GetInt(kMultOn, 0, i) == 0)
            continue;
        
        // Call each pass' shade function with the previous color
        Mtl *mtl = GetSubMtl(i);
        //backTrans = Color(0,0,0);
        if (mtl->ClassID() == PASS_MTL_CLASS_ID)
        {
            plPassMtl *passMtl = (plPassMtl*)mtl;
            passMtl->ShadeWithBackground(sc, backColor);
            backTrans *= sc.out.t;
            backColor = backColor * sc.out.t + sc.out.c;
        }
    }

    sc.out.t = backTrans;
    sc.out.c = backColor;
}

float plMultipassMtl::EvalDisplacement(ShadeContext& sc)
{
    return 0.0f;
}

Interval plMultipassMtl::DisplacementValidity(TimeValue t)
{
    Interval iv;
    iv.SetInfinite();

    return iv;  
}

void plMultipassMtl::SetNumSubMtls(int num)
{
    TimeValue t = GetCOREInterface()->GetTime();
    int curNum = fPassesPB->GetInt(kMultCount);

    fPassesPB->SetValue(kMultCount, 0, num);

    fPassesPB->SetCount(kMultPasses, num);
    fPassesPB->SetCount(kMultOn, num);
    fPassesPB->SetCount(kMultLayerCounts, num);

    for (int i = curNum; i < num; i++)
    {
        plPassMtl *newMtl = new plPassMtl(false);
        fPassesPB->SetValue(kMultPasses, t, newMtl, i);
        fPassesPB->SetValue(kMultOn, t, TRUE, i);
        GetCOREInterface()->AssignNewName(fPassesPB->GetMtl(kMultPasses, t, i));
    }
}
