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

#include "plCompositeMtl.h"
#include "plCompositeMtlDlg.h"

struct LayerID
{
    int layerID;
    int activeID;
    int blendID;
};
static LayerID kLayerID[] =
{
    { IDC_TEX1, 0,          0 },
    { IDC_TEX2, IDC_TEXON2, IDC_COMBO2 },
    { IDC_TEX3, IDC_TEXON3, IDC_COMBO3 }
};

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------

plCompositeMtlDlg::plCompositeMtlDlg(HWND hwMtlEdit, IMtlParams *imp, plCompositeMtl *m)
{
    fDADMgr.Init(this);

    fhMtlEdit = hwMtlEdit;
    fhRollup = nullptr;
    fMtl = m;
    fPBlock = fMtl->GetParamBlockByID(plCompositeMtl::kBlkPasses);
    ip = imp;
    valid = FALSE;

    for (int i = 0; i < NSUBMTLS; i++)
        fLayerBtns[i] = nullptr;

    curTime = imp->GetTime();

    fhRollup = ip->AddRollupPage(
        hInstance,
        MAKEINTRESOURCE(IDD_COMPOSITE),
        ForwardProc,
        "Composite Parameters",
        (LPARAM)this);
}

plCompositeMtlDlg::~plCompositeMtlDlg()
{
    fMtl->SetParamDlg(nullptr);
    for (int i = 0; i < NSUBMTLS; i++)
    {
        ReleaseICustButton(fLayerBtns[i]);
        fLayerBtns[i] = nullptr;
    }

    SetWindowLongPtr(fhRollup, GWLP_USERDATA, 0L);
    ip->DeleteRollupPage(fhRollup);

    fhRollup = nullptr;
}

//-----------------------------------------------------------------------------
// Functions inheirited from ParamDlg
//-----------------------------------------------------------------------------

void plCompositeMtlDlg::SetThing(ReferenceTarget *m)
{
    assert(m->SuperClassID() == MATERIAL_CLASS_ID);
    assert(m->ClassID() == COMP_MTL_CLASS_ID);

    // Bad?
    if (fMtl) 
        fMtl->SetParamDlg(nullptr);
    fMtl = (plCompositeMtl *)m;
    if (fMtl)
        fMtl->SetParamDlg(this);

    LoadDialog();
    IUpdateMtlDisplay();
}

void plCompositeMtlDlg::SetTime(TimeValue t)
{
    if (t != curTime)
    {
        curTime = t;
        Interval v;
        fMtl->Update(ip->GetTime(),v);
        LoadDialog();
        IUpdateMtlDisplay();
    }
}

void plCompositeMtlDlg::ReloadDialog()
{
    Interval v;
    fMtl->Update(ip->GetTime(), v);
    LoadDialog();
}

void plCompositeMtlDlg::ActivateDlg(BOOL onOff)
{
}

int plCompositeMtlDlg::FindSubMtlFromHWND(HWND hwnd)
{
    for (int i = 0; i < NSUBMTLS; i++)
    {
        if (hwnd == fLayerBtns[i]->GetHwnd()) 
            return i;
    }

    return -1;
}

INT_PTR plCompositeMtlDlg::ForwardProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    plCompositeMtlDlg *theDlg;
    if (msg == WM_INITDIALOG)
    {
        theDlg = (plCompositeMtlDlg*)lParam;
        theDlg->fhRollup = hDlg;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
    }
    else
    {
        if (theDlg = (plCompositeMtlDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA); theDlg == nullptr)
            return FALSE; 
    }

    return theDlg->LayerPanelProc(hDlg,msg,wParam,lParam);
}



//----------------------------------------------------------------------------
// Layer panel processor
//----------------------------------------------------------------------------
INT_PTR plCompositeMtlDlg::LayerPanelProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int id = LOWORD(wParam);
    int code = HIWORD(wParam);
    int i;

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            for (i = 0; i < NSUBMTLS; i++)
            {
                fLayerBtns[i] = GetICustButton(GetDlgItem(hDlg, kLayerID[i].layerID));
                fLayerBtns[i]->SetDADMgr(&fDADMgr);

                if (i > 0) // the first material doesn't get one, nyah nyah!
                {
                    HWND cbox = nullptr;
                    int j;
                    for (j = 0; j < plCompositeMtl::kCompNumBlendMethods; j++)
                    {
                        cbox = GetDlgItem(hDlg, kLayerID[i].blendID);
                        SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plCompositeMtl::BlendStrings[j]);
                    }
                    SendMessage(cbox, CB_SETCURSEL, 0, 0);
                }
            }

            // TEMP testing
            UpdateLayerDisplay();

            IUpdateMtlDisplay();
        }
        return TRUE;

    case WM_DESTROY:
        for (i = 0; i < NSUBMTLS; i++)
        {
            ReleaseICustButton(fLayerBtns[i]);
            fLayerBtns[i] = nullptr;
        }
        break;

    case WM_COMMAND:  
        {
            for (i = 0; i < NSUBMTLS; i++)
            {
                if (id == kLayerID[i].activeID)
                {   
                    bool checked = SendMessage(GetDlgItem(hDlg, id), BM_GETCHECK, 0, 0) == BST_CHECKED;
                    fPBlock->SetValue(plCompositeMtl::kCompOn, curTime, checked, i - 1);
                    return TRUE;
                }
                if (id == kLayerID[i].layerID)
                {
                    PostMessage(fhMtlEdit, WM_SUB_MTL_BUTTON, i, (LPARAM)fMtl);
                    return TRUE;
                }
                if (id == kLayerID[i].blendID)
                {
                    fPBlock->SetValue(plCompositeMtl::kCompBlend, curTime, (int)SendMessage(GetDlgItem(hDlg, id), CB_GETCURSEL, 0, 0), i - 1);
                    return TRUE;
                }
            }

        }
//      IUpdateMtlDisplay();
        break;
    }

    return FALSE;
}

void plCompositeMtlDlg::UpdateLayerDisplay() 
{
    int i;
    for (i = 0; i < NSUBMTLS; i++)
    {
        Mtl *m = fPBlock->GetMtl(plCompositeMtl::kCompPasses, curTime, i);
        TSTR nm;
        if (m) 
            nm = m->GetName();
        else 
            nm = "None";
        fLayerBtns[i]->SetText(nm.data());
        
        ShowWindow(GetDlgItem(fhRollup, kLayerID[i].layerID), SW_SHOW);
        ShowWindow(GetDlgItem(fhRollup, kLayerID[i].activeID), SW_SHOW);
        if (i > 0)
        {
            int check = (fPBlock->GetInt(plCompositeMtl::kCompOn, curTime, i - 1) == 0 ? BST_UNCHECKED : BST_CHECKED);
            SendMessage(GetDlgItem(fhRollup, kLayerID[i].activeID), BM_SETCHECK, (WPARAM)check, 0);
            int selection = fPBlock->GetInt(plCompositeMtl::kCompBlend, curTime, i - 1);
            SendMessage(GetDlgItem(fhRollup, kLayerID[i].blendID), CB_SETCURSEL, (WPARAM)selection, 0);
        }
    }
}

void plCompositeMtlDlg::LoadDialog()
{
    if (fMtl)
    {
        fPBlock = fMtl->GetParamBlockByID(plCompositeMtl::kBlkPasses);

        if (fhRollup)
            UpdateLayerDisplay();
    }
}

