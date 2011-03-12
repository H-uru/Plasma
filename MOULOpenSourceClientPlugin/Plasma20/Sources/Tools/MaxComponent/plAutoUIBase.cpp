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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "HeadSpin.h"
#include "plAutoUIBase.h"

#include "max.h"
#include "iparamb2.h"

#include "plAutoUIParams.h"
#include "hsUtils.h"
#include "resource.h"

#include "plGUICompClassIDs.h"

plAutoUIBase::plAutoUIBase() :
	fhDlg(nil), fDesc(nil), fPBlock(nil), fName(nil), fhRollup(nil)
{
}

plAutoUIBase::~plAutoUIBase()
{
	if (fDesc)
	{
		// The internal names are just pointers Max keeps to OUR copy of the string.
		// We'll free them here to prevent leaks
		UInt16 count = fDesc->Count();
		for (UInt16 i = 0; i < count; i++)
		{
			ParamID id = fDesc->IndextoID(i);
			ParamDef& def = fDesc->GetParamDef(id);

			char *name = def.int_name;
			def.int_name = nil;
			delete [] name;

			if (def.type == TYPE_STRING)
			{
				char *defVal = def.def.s;
				def.def.s = nil;
				delete [] defVal;
			}
		}

		delete fDesc;
		fDesc = nil;
	}

	UInt32 count = fParams.size();
	for (UInt32 i = 0; i < count; i++)
		delete fParams[i];
	fParams.clear();

	delete [] fName;
	fName = nil;
}

char *plAutoUIBase::IMakeScriptName(const char *fullName)
{
	if (!fullName)
		return nil;

	char buf[256];
	buf[0] = '\0';
	char *bufptr = buf;

	while (*fullName != '\0')
	{
		if (isalpha(*fullName) || isdigit(*fullName))
		{
			*bufptr = *fullName;
			bufptr++;
		}

		fullName++;
	}

	*bufptr = '\0';

	if (buf[0] == '\0')
		return nil;

	return hsStrcpy(buf);
}

////////////////////////////////////////////////////////////////////////////////
// Setup control
//

void plAutoUIBase::AddCheckBox(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, hsBool def)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_BOOL, 0, 0,
		p_default, def, end,
		end);
	plAutoUIParam* param = TRACKED_NEW plCheckBoxParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddFloatSpinner(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, hsScalar def, hsScalar min, hsScalar max)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_FLOAT, 0, 0,
		p_default, def,
		p_range, min, max,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plSpinnerParam(id, name, true);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddIntSpinner(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, int def, int min, int max)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INT, 0, 0,
		p_default, def,
		p_range, min, max,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plSpinnerParam(id, name, false);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddEditBox(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, const char *def, int lines)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_STRING, 0, 0,
		p_default, def ? hsStrcpy(def) : nil, end,
		end);
	plAutoUIParam* param = TRACKED_NEW plEditParam(id, name, lines);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickNodeList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<Class_ID>* filter)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE_TAB, 0, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickListParam(id, name, filter);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickNodeButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<Class_ID>* filter, bool canConvertToType)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickButtonParam(id, name, filter, canConvertToType);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<Class_ID>* filter, bool canConvertToType)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickComponentButtonParam(id, name, filter, canConvertToType);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickComponentList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<Class_ID>* filter)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE_TAB, 0, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickComponentListParam(id, name, filter);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickActivatorButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickActivatorButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickActivatorList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE_TAB, 0, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickActivatorListParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickDynamicTextButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_REFTARG, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickDynamicTextButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickGUIDialogButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUIDialog,GUI_DIALOG_COMP_CLASS_ID);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickExcludeRegionButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickExcludeRegionButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickWaterComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickWaterComponentButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickSwimCurrentInterfaceButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickSwimCurrentInterfaceButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickClusterComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickClusterComponentButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickAnimationButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickAnimationButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickBehaviorButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickBehaviorButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickMaterialButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_REFTARG, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickMaterialButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickMaterialAnimationButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_REFTARG, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickMaterialAnimationButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickGUIPopUpMenuButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUIPopUpMenu,GUI_MENUANCHOR_CLASSID);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickGUISkinButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickSingleComponentButtonParam(id, name,plAutoUIParam::kTypeGUISkin,GUI_SKIN_CLASSID);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddDropDownList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<std::string>* options)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_STRING, 0, 0,
		p_default, nil, end,
		end);
	plAutoUIParam* param = TRACKED_NEW plDropDownListParam(id, name, options);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

void plAutoUIBase::AddPickGrassComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates)
{
	char *scriptNameNew = scriptName ? hsStrcpy(scriptName) : IMakeScriptName(name);

	fDesc->AddParam(id, scriptNameNew, TYPE_INODE, 0, 0,
		end,
		end);
	plAutoUIParam* param = TRACKED_NEW plPickGrassComponentButtonParam(id, name);
	param->SetVisInfo(vid, vstates);
	fParams.push_back(param);
}

BOOL CALLBACK plAutoUIBase::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	plAutoUIBase *pthis = NULL;
	if (msg == WM_INITDIALOG)
	{
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		pthis = (plAutoUIBase*)lParam;
	}
	else
		pthis = (plAutoUIBase*)GetWindowLong(hDlg, GWL_USERDATA);

	return pthis->DlgProc(hDlg, msg, wParam, lParam);
}

#define WM_SIZE_PANEL WM_APP+1

BOOL plAutoUIBase::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		fhDlg = hDlg;
		ICreateControls();

		PostMessage(fhDlg, WM_SIZE_PANEL, 0, 0);
	}

	UInt32 count = fParams.size();
	for (UInt32 i = 0; i < count; i++)
	{
		if (fParams[i]->IsMyMessage(msg, wParam, lParam, fPBlock))
		{
			if (fParams[i]->GetParamType() == plAutoUIParam::kTypeDropDownList && HIWORD(wParam) == CBN_SELENDOK)
			{
				plDropDownListParam* ddl = (plDropDownListParam*)fParams[i];
				ParamID id = ddl->GetID();
				std::string str = ddl->GetString(fPBlock);
				int yOffset = 10;

				// We now have the id and current state of the drop-down list that changed
				// so now we need to update the visible state of the controls
				for (UInt32 idx = 0; idx < fParams.size(); idx++)
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
				
				InvalidateRect(fhDlg, NULL, TRUE);
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
		for (UInt32 i = 0; i < fParams.size(); i++)
		{
			if (fParams[i]->GetParamType() == plAutoUIParam::kTypeDropDownList)
			{
				plDropDownListParam* ddl = (plDropDownListParam*)fParams[i];
				ParamID id = ddl->GetID();
				std::string str = "";
				const char* cstr = ddl->GetString(fPBlock);
				int yOffset = 10;

				if (cstr)
					str = cstr;

				// We now have the id and current state of the drop-down list that changed
				// so now we need to update the visible state of the controls
				for (UInt32 idx = 0; idx < fParams.size(); idx++)
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

				InvalidateRect(fhDlg, NULL, TRUE);
			}
		}

		return TRUE;
	}
	
	return FALSE;
}

void plAutoUIBase::ICreateControls()
{
	int yOffset = 10;

	UInt32 count = fParams.size();
	for (UInt32 i = 0; i < count; i++)
		yOffset = fParams[i]->Create(fhDlg, fPBlock, yOffset)+5;
		//yOffset = fParams[i]->CreateControls(fhDlg, fPBlock, yOffset)+5;

	RECT rect;
	GetWindowRect(fhDlg, &rect);
	MoveWindow(fhDlg, rect.left, rect.top, rect.right - rect.left, yOffset+5, FALSE);
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
										(fName ? fName : (char*)fDesc->cd->ClassName()),
										(LPARAM)this);
}

void plAutoUIBase::DestroyAutoRollup()
{
	if (fhDlg)
	{
		GetCOREInterface()->DeleteRollupPage(fhDlg);
		fhDlg = nil;
	}

	fPBlock = nil;
}

