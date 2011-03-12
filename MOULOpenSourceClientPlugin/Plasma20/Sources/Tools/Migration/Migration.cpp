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
// Migration.cpp : Defines the entry point for the application.
//
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <commctrl.h>
#include "resource.h"
#include "Migration.h"

HINSTANCE gInstance;
HWND gDlg;
unsigned int gThreadID;

int gTaskItem = -1;
bool gTasksRunning = false;

LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	InitCommonControls();

	HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, (DLGPROC) DlgProc);

	gInstance = hInstance;
	gDlg = hWnd;

	ShowWindow(hWnd, SW_SHOW);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!IsWindow(hWnd) || !IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}


// Mesage handler for dlg box.
LRESULT CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = FALSE;
	switch (message)
	{
		case WM_INITDIALOG:
			{
				HWND hListView = GetDlgItem(hDlg,IDC_TASKLIST);
				ListView_SetExtendedListViewStyleEx(hListView, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
				LoadTasks(hListView);
				LoadTasks(hDlg);
				Button_SetCheck(GetDlgItem(hDlg,IDC_RADIOTEST),BST_CHECKED);
				ret = TRUE;
			}
			break;
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDC_START:
					// Start the migration Tasks
					SetCursor(LoadCursor(NULL,IDC_WAIT));
					Button_Enable(GetDlgItem(hDlg,IDC_START),FALSE);
					Button_Enable(GetDlgItem(hDlg,IDC_STOP),TRUE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOTEST),FALSE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOLAST),FALSE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOBRANCH),FALSE);
					_beginthreadex(NULL,0,RunTasks,NULL,0,&gThreadID);
					gTasksRunning = true;
					ret = TRUE;
					break;
				case IDC_STOP:
					// Stop the migration Tasks
					SetCursor(LoadCursor(NULL,IDC_ARROW));
					Button_Enable(GetDlgItem(hDlg,IDC_START),TRUE);
					Button_Enable(GetDlgItem(hDlg,IDC_STOP),FALSE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOTEST),TRUE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOLAST),TRUE);
					Button_Enable(GetDlgItem(hDlg,IDC_RADIOBRANCH),TRUE);
					ListBox_SetCurSel(GetDlgItem(hDlg,IDC_TASKLIST),gTaskItem);
					gTasksRunning = false;
					ret = TRUE;
					break;
				default:
					break;
				}
			}
			break;
		case WM_NOTIFY:
			{
				switch ((int)wParam)
				{
				case IDC_TASKLIST:
					{
						NMHDR* hdr = (NMHDR*)lParam;
						switch (hdr->code)
						{
						case LVN_ITEMCHANGED:
							{
								NMLISTVIEW* note = (NMLISTVIEW*)lParam;
								if (note->iItem != -1)
								{
									Static_SetText(GetDlgItem(hDlg,IDC_DESCRIPTION),(*(MigrationTaskList::GetInstance()->GetList()))[note->iItem]->GetDescription());
									(*(MigrationTaskList::GetInstance()->GetList()))[note->iItem]->SetEnabled(ListView_GetCheckState(hdr->hwndFrom, note->iItem) != 0);
									ret = true;
								}
							}
							break;
						}
					}
					break;
				}
			}
			break;
		case WM_CLOSE:
			PostQuitMessage(-1);
			break;
	}
    return ret;
}


MigrationTaskList::MigrationTaskList()
{
	static MigrationTask_Backup backup;
	static MigrationTask_CleanUp cleanUp;
	static MigrationTask_PatchBuilder patchBuilder;
	static MigrationTask_DataMigration dataMigration;
	static MigrationTask_InstallClient installClient;
	static MigrationTask_GenerateClientManifest generateClientManifest;
	static MigrationTask_DropStoredGames dropStoredGames;
	static MigrationTask_InstallAges installAges;
	static MigrationTask_CopyTestServers copyTestServers;
	static MigrationTask_StartLiveServers startLiveServers;

	fList.push_back(&backup);
	fList.push_back(&cleanUp);
	fList.push_back(&patchBuilder);
	fList.push_back(&dataMigration);
	fList.push_back(&installClient);
	fList.push_back(&generateClientManifest);
	fList.push_back(&dropStoredGames);
	fList.push_back(&installAges);
	fList.push_back(&copyTestServers);
	fList.push_back(&startLiveServers);
}

MigrationTaskList* MigrationTaskList::GetInstance()
{
	static MigrationTaskList mlist;
	return &mlist;
}

void LoadTasks(HWND hListView)
{
	MigrationTaskList::TaskList* tasktlist = MigrationTaskList::GetInstance()->GetList();

	MigrationTaskList::TaskList::iterator it = tasktlist->begin();
	int index = 0;
	while (it != tasktlist->end())
	{
		LVITEM item;
		ZeroMemory(&item,sizeof(item));
		item.pszText = (*it)->GetName();
		item.mask = LVIF_TEXT;
		item.iItem = index;
		ListView_InsertItem(hListView,&item);
		it++; index++;
	}
}

unsigned int __stdcall RunTasks(void* args)
{
	gTaskItem = 0;
	MigrationTaskList::TaskList* tasktlist = MigrationTaskList::GetInstance()->GetList();

	while(gTasksRunning && gTaskItem < tasktlist->size())
	{
		if ((*tasktlist)[gTaskItem]->GetEnabled())
		{
			if (Button_GetCheck(GetDlgItem(gDlg,IDC_RADIOTEST)) == BST_CHECKED)
				(*tasktlist)[gTaskItem]->SetServer(MigrationTask::kTest);
			if (Button_GetCheck(GetDlgItem(gDlg,IDC_RADIOLAST)) == BST_CHECKED)
				(*tasktlist)[gTaskItem]->SetServer(MigrationTask::kLast);
			if (Button_GetCheck(GetDlgItem(gDlg,IDC_RADIOBRANCH)) == BST_CHECKED)
				(*tasktlist)[gTaskItem]->SetServer(MigrationTask::kBranch);

			ListBox_SetCurSel(GetDlgItem(gDlg,IDC_TASKLIST),gTaskItem);
			Static_SetText(GetDlgItem(gDlg,IDC_DESCRIPTION),(*tasktlist)[gTaskItem]->GetDescription());
			gTasksRunning = (*tasktlist)[gTaskItem]->Run(gInstance,gDlg) == 0;
		}
		gTaskItem++;
	}

	gTaskItem = -1;
	SendMessage(gDlg,WM_COMMAND,MAKEWPARAM(IDC_STOP,BN_CLICKED),WPARAM(GetDlgItem(gDlg,IDC_STOP)));

	return 0;
}