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
#include "plResponderWait.h"
#include "plResponderComponentPriv.h"
#include "resource.h"
#include "../plModifier/plResponderModifier.h"

#include "plResponderLink.h"

class plResponderWaitProc;
extern plResponderWaitProc gResponderWaitProc;

enum
{
	kWaitWhoOld,
	kWaitPointOld,
	kWaitMe,
	kWaitWho,
	kWaitPoint,
};

ParamBlockDesc2 gResponderWaitBlock
(
	kResponderWaitBlk, _T("waitCmd"), 0, NULL, P_AUTO_UI,

	IDD_COMP_RESPOND_WAIT, IDS_COMP_WAIT, 0, 0, &gResponderWaitProc,

	kWaitWhoOld,	_T("whoOld"),		TYPE_INT_TAB, 0,		0, 0,
		end,

	kWaitPointOld,	_T("pointOld"),	TYPE_STRING_TAB, 0,		0, 0,
		end,

	kWaitMe,	_T("me"),		TYPE_BOOL,				0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_WAIT_ON_ME_CHECK,
		end,

	kWaitWho, _T("who"),		TYPE_INT,				0, 0,
		p_default,	-1,
		end,

	kWaitPoint, _T("point"),	TYPE_STRING,			0, 0,
		end,

	end
);
void ResponderWait::SetDesc(ClassDesc2 *desc) { gResponderWaitBlock.SetClassDesc(desc); }

void ResponderWait::FixupWaitBlock(IParamBlock2 *waitPB)
{
	if (waitPB->Count(kWaitWhoOld) > 0)
	{
		int who = waitPB->GetInt(kWaitWhoOld, 0, 0);
		waitPB->SetValue(kWaitWho, 0, who);
		waitPB->Delete(kWaitWhoOld, 0, 1);
	}

	if (waitPB->Count(kWaitPointOld) > 0)
	{
		TCHAR* point = waitPB->GetStr(kWaitPointOld, 0, 0);
		waitPB->SetValue(kWaitPoint, 0, point);
		waitPB->Delete(kWaitPointOld, 0, 1);
	}
}

IParamBlock2 *ResponderWait::CreatePB()
{
	return CreateParameterBlock2(&gResponderWaitBlock, nil);
}

bool ResponderWait::GetWaitOnMe(IParamBlock2* waitPB)
{
	return (waitPB->GetInt(kWaitMe) != 0);
}

int ResponderWait::GetWaitingOn(IParamBlock2* waitPB)
{
	return waitPB->GetInt(kWaitWho);
}

const char*	ResponderWait::GetWaitPoint(IParamBlock2* waitPB)
{
	const char* point = waitPB->GetStr(kWaitPoint);
	if (point && *point == '\0')
		return nil;
	return point;
}

class plResponderWaitProc : public ParamMap2UserDlgProc
{
protected:
	IParamBlock2 *fStatePB;
	IParamBlock2 *fWaitPB;

	int fCurCmd;
	HWND fhDlg;
	HWND fhList;

public:
	void Init(IParamBlock2 *curStatePB, int curCmd, HWND hList) { fStatePB = curStatePB; fCurCmd = curCmd; fhList = hList; }

	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	void DeleteThis() {}

protected:
	void LoadWho(bool setDefault=false);
	void LoadPoint(bool force=false);

	IParamBlock2 *GetCmdParams(int cmdIdx);
};
static plResponderWaitProc gResponderWaitProc;

void ResponderWait::InitDlg(IParamBlock2 *curStatePB, int curCmd, HWND hList)
{
	gResponderWaitProc.Init(curStatePB, curCmd, hList);
}

IParamBlock2 *plResponderWaitProc::GetCmdParams(int cmdIdx)
{
	return (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdParams, 0, cmdIdx);
}

BOOL plResponderWaitProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			fhDlg = hDlg;

			fWaitPB = pm->GetParamBlock();

			ResponderWait::FixupWaitBlock(fWaitPB);

			IParamBlock2 *pb = GetCmdParams(fCurCmd);
			plResponderCmd *cmd = plResponderCmd::Find(pb);
			pm->Enable(kWaitMe, cmd->IsWaitable(pb));

			LoadWho();
			LoadPoint();
			return TRUE;
		}

		case WM_CUSTEDIT_ENTER:
			if (wParam == IDC_MARKER_EDIT)
			{
				ICustEdit *edit = GetICustEdit((HWND)lParam);
				char buf[256];
				edit->GetText(buf, sizeof(buf));
				fWaitPB->SetValue(kWaitPoint, 0, buf);

				return TRUE;
			}
			break;

		case WM_COMMAND:
		{
			int code = HIWORD(wParam);
			int id = LOWORD(wParam);

			if (id == IDC_CHECK_WAIT && code == BN_CLICKED)
			{
				BOOL checked = (IsDlgButtonChecked(hDlg, IDC_CHECK_WAIT) == BST_CHECKED);
				if (!checked)
				{
					fWaitPB->SetValue(kWaitWho, 0, -1);
					fWaitPB->SetValue(kWaitPoint, 0, "");

					LoadPoint();

					HWND hWho = GetDlgItem(hDlg, IDC_WAIT_WHO);
					EnableWindow(hWho, FALSE);
					ComboBox_ResetContent(hWho);
				}
				else
				{
					LoadWho(true);
					LoadPoint();
				}

				return TRUE;
			}
			else if (id == IDC_WAIT_WHO && code == CBN_SELCHANGE)
			{
				HWND hWho = (HWND)lParam;
				int who = ComboBox_GetCurSel(hWho);
				int idx = ComboBox_GetItemData(hWho, who);
				fWaitPB->SetValue(kWaitWho, 0, idx);

				LoadPoint();
				return TRUE;
			}
			else if (id == IDC_RADIO_FINISH && code == BN_CLICKED)
			{
				fWaitPB->SetValue(kWaitPoint, 0, "");
				LoadPoint();
				return TRUE;
			}
			else if (id == IDC_RADIO_POINT && code == BN_CLICKED)
			{
				LoadPoint(true);
				return TRUE;
			}
			else if (id == IDC_WAIT_POINT && code == CBN_SELCHANGE)
			{
				HWND hPoint = (HWND)lParam;
				if (ComboBox_GetCurSel(hPoint) != CB_ERR)
				{
					char buf[256];
					ComboBox_GetText(hPoint, buf, sizeof(buf));
					fWaitPB->SetValue(kWaitPoint, 0, buf);
				}
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

void plResponderWaitProc::LoadWho(bool setDefault)
{
	HWND hWho = GetDlgItem(fhDlg, IDC_WAIT_WHO);
	int who = fWaitPB->GetInt(kWaitWho);

	ComboBox_ResetContent(hWho);

	int numFound = 0;

	// Copy all the commands before this one to the 'who' combo box
	for (int i = 0; i < fCurCmd; i++)
	{
		IParamBlock2 *pb = GetCmdParams(i);
		plResponderCmd *cmd = plResponderCmd::Find(pb);

		if (cmd->IsWaitable(pb))
		{
			int idx = ComboBox_AddString(hWho, cmd->GetInstanceName(pb));
			ComboBox_SetItemData(hWho, idx, i);

			// If the saved 'who' is valid, select it and check the wait checkbox
			if (who == i)
			{
				ComboBox_SetCurSel(hWho, idx);
				CheckDlgButton(fhDlg, IDC_CHECK_WAIT, BST_CHECKED);
				EnableWindow(hWho, TRUE);
			}

			numFound++;
		}
	}

	// Pick the last item in the who combo as the default
	if (setDefault && numFound > 0)
	{
		HWND hWho = GetDlgItem(fhDlg, IDC_WAIT_WHO);
		int idx = ComboBox_GetItemData(hWho, numFound-1);
		fWaitPB->SetValue(kWaitWho, 0, idx);

		ComboBox_SetCurSel(hWho, numFound-1);
		CheckDlgButton(fhDlg, IDC_CHECK_WAIT, BST_CHECKED);
		EnableWindow(hWho, TRUE);
	}

	// Disable the wait checkbox if there are no waitable commands behind this one
	EnableWindow(GetDlgItem(fhDlg, IDC_CHECK_WAIT), (numFound > 0));
}

void plResponderWaitProc::LoadPoint(bool force)
{
	int who = fWaitPB->GetInt(kWaitWho);
	const char *point = fWaitPB->GetStr(kWaitPoint);
	if (point && *point == '\0')
		point = nil;

	CheckRadioButton(fhDlg, IDC_RADIO_FINISH, IDC_RADIO_POINT, point || force ? IDC_RADIO_POINT : IDC_RADIO_FINISH);

	BOOL enableAll = (who != -1);
	EnableWindow(GetDlgItem(fhDlg, IDC_RADIO_FINISH), enableAll);
	EnableWindow(GetDlgItem(fhDlg, IDC_RADIO_POINT), enableAll);

	BOOL enablePoint = ((point != nil) || force) && enableAll;
	EnableWindow(GetDlgItem(fhDlg, IDC_WAIT_POINT), enablePoint);
	ComboBox_ResetContent(GetDlgItem(fhDlg, IDC_WAIT_POINT));

	if (enableAll)
	{
		IParamBlock2 *pb = (IParamBlock2*)fStatePB->GetReferenceTarget(kStateCmdParams, 0, who);
		plResponderCmd *cmd = plResponderCmd::Find(pb);

		// KLUDGE - stupid one-shot needs editable box
		if (cmd == &(plResponderCmdOneShot::Instance()))
		{
			ShowWindow(GetDlgItem(fhDlg, IDC_WAIT_POINT), SW_HIDE);

			HWND hEdit = GetDlgItem(fhDlg, IDC_MARKER_EDIT);
			ShowWindow(hEdit, SW_SHOW);
			ICustEdit *custEdit = GetICustEdit(hEdit);
			custEdit->SetText(point ? (char*)point : "");
		}
		else
		{
			ShowWindow(GetDlgItem(fhDlg, IDC_WAIT_POINT), SW_SHOW);

			HWND hEdit = GetDlgItem(fhDlg, IDC_MARKER_EDIT);
			ShowWindow(hEdit, SW_HIDE);

			plResponderCmd::WaitPoints waitPoints;
			cmd->GetWaitPoints(pb, waitPoints);

			HWND hCombo = GetDlgItem(fhDlg, IDC_WAIT_POINT);
			ComboBox_ResetContent(hCombo);

			if (waitPoints.size() == 0)
			{
				EnableWindow(GetDlgItem(fhDlg, IDC_RADIO_POINT), FALSE);
				EnableWindow(GetDlgItem(fhDlg, IDC_WAIT_POINT), FALSE);
			}
			else
			{
				for (int i = 0; i < waitPoints.size(); i++)
				{
					const char *marker = waitPoints[i].c_str();
					int idx = ComboBox_AddString(hCombo, marker);
					if (point && !strcmp(point, marker))
						ComboBox_SetCurSel(hCombo, idx);
				}
			}
		}
	}
}

static IParamBlock2 *GetWaitBlk(IParamBlock2 *state, int idx)
{
	return (IParamBlock2*)state->GetReferenceTarget(kStateCmdWait, 0, idx);
}

void ResponderWait::CmdRemoved(IParamBlock2 *state, int delIdx)
{
	int numCmds = state->Count(kStateCmdParams);
	for (int i = delIdx; i < numCmds; i++)
	{
		IParamBlock2 *pb = GetWaitBlk(state, i);

		int who = pb->GetInt(kWaitWho);

		if (who == delIdx)
			pb->SetValue(kWaitWho, 0, -1);
		if (who > delIdx)
			pb->SetValue(kWaitWho, 0, who-1);
	}
}

// A command was moved from oldIdx to newIdx.  Fix all the wait commands.
void ResponderWait::CmdMoved(IParamBlock2 *state, int oldIdx, int newIdx)
{
	int numCmds = state->Count(kStateCmdParams);

	// Moved forward
	if (oldIdx < newIdx)
	{
		// Patch up the commands that were ahead of this one
		for (int i = oldIdx; i < numCmds; i++)
		{
			if (i == newIdx)
				continue;

			IParamBlock2 *pb = GetWaitBlk(state, i);

			int who = pb->GetInt(kWaitWho);

			// If the command was waiting on the moved one, and is now behind it, invalidate it
			if (who == oldIdx && i < newIdx)
				pb->SetValue(kWaitWho, 0, -1);
			//
			else if (who == oldIdx)
				pb->SetValue(kWaitWho, 0, newIdx);
			// If it was waiting on one ahead of it, correct the index
			else if (who > oldIdx && who <= newIdx)
				pb->SetValue(kWaitWho, 0, who-1);
		}
	}
	// Moved backward
	else
	{
		// If this command was waiting on any of the commands now ahead of it, invalidate it
		IParamBlock2 *movedPB = GetWaitBlk(state, newIdx);
		int who = movedPB->GetInt(kWaitWho);
		if (who >= newIdx)
			movedPB->SetValue(kWaitWho, 0, -1);

		for (int i = newIdx+1; i < numCmds; i++)
		{
			// Is this command waiting on any of the commands it is moving in back of?
			IParamBlock2 *pb = GetWaitBlk(state, i);
			int who = pb->GetInt(kWaitWho);
			if (who == oldIdx)
				pb->SetValue(kWaitWho, 0, newIdx);
			if (who >= newIdx && who < oldIdx)
				pb->SetValue(kWaitWho, 0, who+1);
		}
	}
}

// Determine if any of the wait commands will be invalidated by this move
bool ResponderWait::ValidateCmdMove(IParamBlock2 *state, int oldIdx, int newIdx)
{
	// Moving forward
	if (oldIdx < newIdx)
	{
		// Are any of the commands ahead of this one waiting on it?
		for (int i = oldIdx+1; i <= newIdx; i++)
		{
			IParamBlock2 *pb = GetWaitBlk(state, i);

			if (pb->GetInt(kWaitWho) == oldIdx)
			{
				int ret = hsMessageBox("You are moving this command ahead of another command that waits on it.\nAre you sure you want to do that?", "Warning", hsMessageBoxYesNo);
				if (ret == hsMBoxYes)
					return true;
				else
					return false;
			}
		}
	}
	// Moving backward
	else
	{
		// Is this command waiting on any of the commands it is moving in back of?
		IParamBlock2 *pb = GetWaitBlk(state, oldIdx);

		int who = pb->GetInt(kWaitWho);
		if (who >= newIdx && who < oldIdx)
		{
			int ret = hsMessageBox("You are moving this command behind another command that it is waiting on.\nAre you sure you want to do that?", "Warning", hsMessageBoxYesNo);
			if (ret == hsMBoxYes)
				return true;
			else
				return false;
		}
	}

	return true;
}
