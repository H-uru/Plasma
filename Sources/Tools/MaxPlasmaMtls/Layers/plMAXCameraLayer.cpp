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

#include "plMAXCameraLayer.h"

#include "../plBMSampler.h"
#include "MaxMain/plPlasmaRefMsgs.h"

const TCHAR* kUVStrings[] = { _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8") };

class plMAXCameraLayerClassDesc : public ClassDesc2
{
public:
    int             IsPublic() override     { return TRUE; }
    void*           Create(BOOL loading = FALSE) override { return new plMAXCameraLayer(); }
    const TCHAR*    ClassName() override    { return GetString(IDS_MAX_CAMERA_LAYER); }
    SClass_ID       SuperClassID() override { return TEXMAP_CLASS_ID; }
    Class_ID        ClassID() override      { return MAX_CAMERA_LAYER_CLASS_ID; }
    const TCHAR*    Category() override     { return TEXMAP_CAT_COLMOD; }
    const TCHAR*    InternalName() override { return _T("PlasmaMAXCameraLayer"); }
    HINSTANCE       HInstance() override    { return hInstance; }
};
static plMAXCameraLayerClassDesc plMAXCameraLayerDesc;
ClassDesc2* GetMAXCameraLayerDesc() { return &plMAXCameraLayerDesc; }

class MAXCameraLayerDlgProc : public ParamMap2UserDlgProc
{
public:
    MAXCameraLayerDlgProc() {}
    ~MAXCameraLayerDlgProc() {}

    void UpdateDisplay(IParamMap2 *pmap)
    {
        HWND hWnd = pmap->GetHWnd();
        IParamBlock2 *pb = pmap->GetParamBlock();
        HWND cbox;
        cbox = GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC);
        SendMessage(cbox, CB_SETCURSEL, pb->GetInt(plMAXCameraLayer::kUVSource), 0);
        bool reflect = (pb->GetInt(ParamID(plMAXCameraLayer::kExplicitCam)) == 0);
        EnableWindow(GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC), !reflect);
    }

    void Update(TimeValue t, Interval& valid, IParamMap2* pmap) override { UpdateDisplay(pmap); }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        IParamBlock2 *pb = map->GetParamBlock();
        HWND cbox = nullptr;

        switch (msg)
        {
        case WM_INITDIALOG:
            int i;
            for (const TCHAR* uvString : kUVStrings)
            {
                cbox = GetDlgItem(hWnd, IDC_CAM_LAYER_UV_SRC);
                SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)uvString);
            }
            UpdateDisplay(map);
            return TRUE;

        case WM_COMMAND:
            if (id == IDC_CAM_LAYER_UV_SRC)
            {
                pb->SetValue(plMAXCameraLayer::kUVSource, t, (int)SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0));
                return TRUE;
            }
            else if (id == IDC_CAM_LAYER_EXPLICIT_CAM)
            {
                UpdateDisplay(map);
                return TRUE;
            }
            break;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static MAXCameraLayerDlgProc gMAXCameraLayerDlgProc;

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 gMAXCameraLayerParamBlk
(
    plMAXCameraLayer::kBlkMain, _T("CamLayer"),  0, GetMAXCameraLayerDesc(),
    P_AUTO_CONSTRUCT + P_AUTO_UI, plMAXCameraLayer::kRefMain,
    IDD_MAX_CAMERA_LAYER, IDS_MAX_CAMERA_LAYER_PROPS, 0, 0, &gMAXCameraLayerDlgProc,

    plMAXCameraLayer::kCamera, _T("camera"),    TYPE_INODE, P_CAN_CONVERT, 0,
        p_ui, TYPE_PICKNODEBUTTON, IDC_CAM_LAYER_CAMERA,
        p_classID, Class_ID(LOOKAT_CAM_CLASS_ID, 0),
        p_prompt, IDS_CAM_LAYER_CAMERA,
        p_end,

    plMAXCameraLayer::kUVSource, _T("UVSource"),    TYPE_INT,   0, 0,
        p_default, 0,
        p_end,

    plMAXCameraLayer::kExplicitCam, _T("explicitCam"),  TYPE_BOOL,      0, 0,
        p_ui, TYPE_SINGLECHEKBOX, IDC_CAM_LAYER_EXPLICIT_CAM,
        p_default, false,
        p_enable_ctrls, 1, plMAXCameraLayer::kCamera,
        p_end,

    plMAXCameraLayer::kRootNode, _T("rootNode"),    TYPE_INODE, 0, 0,
        p_ui, TYPE_PICKNODEBUTTON, IDC_CAM_LAYER_ROOT_NODE,
        p_prompt, IDS_CAM_LAYER_ROOT_NODE,
        p_end,

    plMAXCameraLayer::kDisableColor, _T("disableColor"), TYPE_RGBA, 0, 0,
        p_ui,           TYPE_COLORSWATCH, IDC_CAM_LAYER_DISABLE_COLOR,
        p_default,      Color(0,0,0),
        p_end,

    plMAXCameraLayer::kForce, _T("force"),  TYPE_BOOL,      0, 0,
        p_ui, TYPE_SINGLECHEKBOX, IDC_CAM_LAYER_FORCE,
        p_default, false,
        p_end,

    p_end
);

/////////////////////////////////////////////////////////////////////////////

plMAXCameraLayer::plMAXCameraLayer() :
fParmsPB(),
fIValid(NEVER)
{
    plMAXCameraLayerDesc.MakeAutoParamBlocks(this);
}

plMAXCameraLayer::~plMAXCameraLayer()
{
}

void plMAXCameraLayer::GetClassName(TSTR& s)
{
    s = GetString(IDS_MAX_CAMERA_LAYER);
}

//From MtlBase
void plMAXCameraLayer::Reset()
{
    GetMAXCameraLayerDesc()->Reset(this, TRUE); // reset all pb2's
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

    fIValid.SetEmpty();
}

void plMAXCameraLayer::Update(TimeValue t, Interval& valid)
{
    if (!fIValid.InInterval(t))
    {
        fIValid.SetInfinite();
    }

    valid &= fIValid;
}

Interval plMAXCameraLayer::Validity(TimeValue t)
{
    Interval v = FOREVER;
    return v;
}

ParamDlg* plMAXCameraLayer::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
    IAutoMParamDlg* masterDlg = plMAXCameraLayerDesc.CreateParamDlgs(hwMtlEdit, imp, this);

    return masterDlg;
}

BOOL plMAXCameraLayer::SetDlgThing(ParamDlg* dlg)
{
    return FALSE;
}

int plMAXCameraLayer::NumRefs()
{
    return 1;
}

//From ReferenceMaker
RefTargetHandle plMAXCameraLayer::GetReference(int i)
{
    switch (i)
    {
    case kRefMain:      return fParmsPB;
    default:            return nullptr;
    }
}

void plMAXCameraLayer::SetReference(int i, RefTargetHandle rtarg)
{
    Interval    garbage;

    switch (i)
    {
    case kRefMain:
        fParmsPB = (IParamBlock2 *)rtarg;
        break;
    }
}

int plMAXCameraLayer::NumParamBlocks()
{
    return 1;
}

IParamBlock2* plMAXCameraLayer::GetParamBlock(int i)
{
    switch (i)
    {
    case 0: return fParmsPB;
    default: return nullptr;
    }
}

IParamBlock2* plMAXCameraLayer::GetParamBlockByID(BlockID id)
{
    if (fParmsPB->ID() == id)
        return fParmsPB;
    else
        return nullptr;
}

//From ReferenceTarget
RefTargetHandle plMAXCameraLayer::Clone(RemapDir &remap)
{
    plMAXCameraLayer *mnew = new plMAXCameraLayer();
    *((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
    mnew->ReplaceReference(kRefMain, remap.CloneRef(fParmsPB));
    BaseClone(this, mnew, remap);
    return (RefTargetHandle)mnew;
}

int plMAXCameraLayer::NumSubs()
{
    return 1;
}

Animatable* plMAXCameraLayer::SubAnim(int i)
{
    switch (i)
    {
        case kRefMain:      return fParmsPB;
        default:            return nullptr;
    }
}

MSTR plMAXCameraLayer::SubAnimName(int i)
{
    switch (i)
    {
        case kRefMain:      return _M("Main");
        default: return _M("");
    }
}

RefResult plMAXCameraLayer::NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
                                             PartID& partID, RefMessage message MAX_REF_PROPAGATE)
{
    switch (message)
    {
    case REFMSG_CHANGE:
        {
            fIValid.SetEmpty();

            if (hTarget == fParmsPB)
            {
                // see if this message came from a changing parameter in the pblock,
                // if so, limit rollout update to the changing item
                ParamID changingParam = fParmsPB->LastNotifyParamID();
                fParmsPB->GetDesc()->InvalidateUI(changingParam);

                if (changingParam != -1 && MAX_REF_PROPAGATE_VALUE)
                    IChanged();
            }
        }
        break;

    }

    return REF_SUCCEED;
}

void plMAXCameraLayer::IChanged()
{
    // Cut and paste insanity from DynamicTextLayer.
    // Texture wasn't getting updated in the viewports, and this fixes it.
    // Don't know if it's the right way though.
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

    // And this is so the SceneWatcher gets notified that the material on some of it's
    // referenced objects changed.
    NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_MAT);
}

#define TEX_HDR_CHUNK 0x5000

IOResult plMAXCameraLayer::Save(ISave *isave)
{
    IOResult res;

    isave->BeginChunk(TEX_HDR_CHUNK);
    res = MtlBase::Save(isave);
    if (res != IO_OK)
        return res;
    isave->EndChunk();

    return IO_OK;
}

IOResult plMAXCameraLayer::Load(ILoad *iload)
{
    IOResult res;
    while (IO_OK == (res = iload->OpenChunk()))
    {
        if (iload->CurChunkID() == TEX_HDR_CHUNK)
        {
            res = MtlBase::Load(iload);
        }
        iload->CloseChunk();
        if (res != IO_OK)
            return res;
    }

    return IO_OK;
}


AColor plMAXCameraLayer::EvalColor(ShadeContext& sc)
{
    return AColor(0.0f, 0.0f, 0.0f, 1.0f);
}

float plMAXCameraLayer::EvalMono(ShadeContext& sc)
{
    return Intens(EvalColor(sc));
}

Point3 plMAXCameraLayer::EvalNormalPerturb(ShadeContext& sc)
{
    // Return the perturbation to apply to a normal for bump mapping
    return Point3(0, 0, 0);
}

ULONG plMAXCameraLayer::LocalRequirements(int subMtlNum)
{
    return MTLREQ_VIEW_DEP | MTLREQ_TRANSP;
}

void plMAXCameraLayer::ActivateTexDisplay(BOOL onoff)
{
}

BITMAPINFO *plMAXCameraLayer::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH)
{
    return nullptr;
}

DWORD_PTR plMAXCameraLayer::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
    return 0;
}

const char *plMAXCameraLayer::GetTextureName( int which )
{
    return nullptr;
}
