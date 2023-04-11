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

#include "resource.h"

#include "plAutoUIBase.h"
#include "plAutoUIParams.h"

#include "plGUICompClassIDs.h"

plAutoUIBase::plAutoUIBase() :
    fhDlg(), fDesc(), fPBlock(), fhRollup()
{
}

plAutoUIBase::~plAutoUIBase()
{
    delete fDesc;
    fDesc = nullptr;

    uint32_t count = fParams.size();
    for (uint32_t i = 0; i < count; i++)
        delete fParams[i];
    fParams.clear();
}

ST::string plAutoUIBase::IMakeScriptName(const ST::string& fullName)
{
    if (fullName.empty())
        return {};

    ST::string_stream ss;
    for (auto chr : fullName) {
        if (isalpha(static_cast<unsigned char>(chr)) || isdigit(static_cast<unsigned char>(chr)))
            ss.append_char(chr);
    }
    return ss.to_string();
}

////////////////////////////////////////////////////////////////////////////////
// Setup control
//

void plAutoUIBase::AddCheckBox(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, bool def)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_BOOL, 0, 0,
        p_default, def, p_end,
        p_end);
    plAutoUIParam* param = new plCheckBoxParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddFloatSpinner(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, float def, float min, float max)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_FLOAT, 0, 0,
        p_default, def,
        p_range, min, max,
        p_end,
        p_end);
    plAutoUIParam* param = new plSpinnerParam(id, name, true);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddIntSpinner(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, int def, int min, int max)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INT, 0, 0,
        p_default, def,
        p_range, min, max,
        p_end,
        p_end);
    plAutoUIParam* param = new plSpinnerParam(id, name, false);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddEditBox(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, const ST::string& def, int lines)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_STRING, 0, 0,
        p_default, ST2M(def), p_end,
        p_end);
    plAutoUIParam* param = new plEditParam(id, name, lines);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickNodeList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<Class_ID>* filter)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE_TAB, 0, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickListParam(id, name, filter);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickNodeButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<Class_ID>* filter, bool canConvertToType)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickButtonParam(id, name, filter, canConvertToType);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<Class_ID>* filter, bool canConvertToType)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickComponentButtonParam(id, name, filter, canConvertToType);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickComponentList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<Class_ID>* filter)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE_TAB, 0, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickComponentListParam(id, name, filter);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickActivatorButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickActivatorButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickActivatorList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE_TAB, 0, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickActivatorListParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickDynamicTextButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_REFTARG, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickDynamicTextButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickGUIDialogButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUIDialog,GUI_DIALOG_COMP_CLASS_ID);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickExcludeRegionButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickExcludeRegionButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickWaterComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickWaterComponentButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickSwimCurrentInterfaceButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickSwimCurrentInterfaceButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickClusterComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickClusterComponentButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickAnimationButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickAnimationButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickBehaviorButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickBehaviorButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickMaterialButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_REFTARG, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickMaterialButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickMaterialAnimationButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_REFTARG, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickMaterialAnimationButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickGUIPopUpMenuButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUIPopUpMenu,GUI_MENUANCHOR_CLASSID);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickGUISkinButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUISkin,GUI_SKIN_CLASSID);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddDropDownList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<ST::string> options)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_STRING, 0, 0,
        p_default, nullptr, p_end,
        p_end);
    plAutoUIParam* param = new plDropDownListParam(id, name, std::move(options));
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

void plAutoUIBase::AddPickGrassComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates)
{
    ST::string scriptNameNew = !scriptName.empty() ? scriptName : IMakeScriptName(name);

    fDesc->AddParam(id, ST2M(scriptNameNew), TYPE_INODE, 0, 0,
        p_end,
        p_end);
    plAutoUIParam* param = new plPickGrassComponentButtonParam(id, name);
    param->SetVisInfo(vid, std::move(vstates));
    fParams.push_back(param);
}

INT_PTR CALLBACK plAutoUIBase::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    plAutoUIBase *pthis = nullptr;
    if (msg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
        pthis = (plAutoUIBase*)lParam;
    }
    else
        pthis = (plAutoUIBase*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

    return pthis->DlgProc(hDlg, msg, wParam, lParam);
}

#define WM_SIZE_PANEL WM_APP+1

INT_PTR plAutoUIBase::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_INITDIALOG)
    {
        fhDlg = hDlg;
        ICreateControls();

        PostMessage(fhDlg, WM_SIZE_PANEL, 0, 0);
    }

    uint32_t count = fParams.size();
    for (uint32_t i = 0; i < count; i++)
    {
        if (fParams[i]->IsMyMessage(msg, wParam, lParam, fPBlock))
        {
            if (fParams[i]->GetParamType() == plAutoUIParam::kTypeDropDownList && HIWORD(wParam) == CBN_SELENDOK)
            {
                plDropDownListParam* ddl = (plDropDownListParam*)fParams[i];
                ParamID id = ddl->GetID();
                ST::string str = M2ST(ddl->GetString(fPBlock));
                int yOffset = 10;

                // We now have the id and current state of the drop-down list that changed
                // so now we need to update the visible state of the controls
                for (uint32_t idx = 0; idx < fParams.size(); idx++)
                {
                    plAutoUIParam* par = fParams[idx];

                    if (par->CheckVisibility(id, str))
                    {
                        par->Show(yOffset);
                        yOffset += par->GetHeight() + 5;
                    }
                    else
                    {
                        par->Hide();
                    }
                }

                IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
                int index = rollup->GetPanelIndex(fhDlg);
                
                if (index >= 0)
                    rollup->SetPageDlgHeight(index, yOffset);
                
                InvalidateRect(fhDlg, nullptr, TRUE);
            }
            return TRUE;
        }
    }

    // During init but after everything else we want to update the visibility
    if (msg == WM_SIZE_PANEL)
    {
        // Ok, this sucks but I don't know of a better way at this point
        // We need to intialize the visible state of the controls, and the only way to do this
        // is by looping through and finding all of the drop-down lists
        for (uint32_t i = 0; i < fParams.size(); i++)
        {
            if (fParams[i]->GetParamType() == plAutoUIParam::kTypeDropDownList)
            {
                plDropDownListParam* ddl = (plDropDownListParam*)fParams[i];
                ParamID id = ddl->GetID();
                ST::string str = M2ST(ddl->GetString(fPBlock));
                int yOffset = 10;


                // We now have the id and current state of the drop-down list that changed
                // so now we need to update the visible state of the controls
                for (uint32_t idx = 0; idx < fParams.size(); idx++)
                {
                    if (fParams[idx]->CheckVisibility(id, str))
                    {
                        fParams[idx]->Show(yOffset);
                        yOffset += fParams[idx]->GetHeight() + 5;
                    }
                    else
                    {
                        fParams[idx]->Hide();
                    }
                }

                IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
                int index = rollup->GetPanelIndex(fhDlg);
                
                if (index >= 0)
                    rollup->SetPageDlgHeight(index, yOffset);

                InvalidateRect(fhDlg, nullptr, TRUE);
            }
        }

        return TRUE;
    }
    
    return FALSE;
}

void plAutoUIBase::ICreateControls()
{
    int yOffset = 10;

    uint32_t count = fParams.size();
    for (uint32_t i = 0; i < count; i++)
        yOffset = fParams[i]->Create(fhDlg, fPBlock, yOffset)+5;
        //yOffset = fParams[i]->CreateControls(fhDlg, fPBlock, yOffset)+5;

    RECT rect;
    GetWindowRect(fhDlg, &rect);

    // This used to use MoveWindow() to resize the rollup, but in Max 2022,
    // that does not seem to work anymore. So, we now do the same thing
    // that WM_SIZE_PANEL does.
    IRollupWindow* rollup = GetCOREInterface()->GetCommandPanelRollup();
    int index = rollup->GetPanelIndex(fhDlg);

    if (index >= 0)
        rollup->SetPageDlgHeight(index, yOffset + 5);

    InvalidateRect(fhDlg, nullptr, TRUE);
}

void plAutoUIBase::CreateAutoRollup(IParamBlock2 *pb)
{
    fPBlock = pb;

    // Don't bother putting up a rollup if there are no params
    if (pb->NumParams() == 0)
        return;

    fhRollup = GetCOREInterface()->AddRollupPage(hInstance,
                                        MAKEINTRESOURCE(IDD_COMP_AUTO),
                                        ForwardDlgProc,
                                        const_cast<MCHAR*>(!fName.empty() ? ST2M(fName) : fDesc->cd->ClassName()),
                                        (LPARAM)this);
}

void plAutoUIBase::DestroyAutoRollup()
{
    if (fhDlg)
    {
        GetCOREInterface()->DeleteRollupPage(fhDlg);
        fhDlg = nullptr;
    }

    fPBlock = nullptr;
}
