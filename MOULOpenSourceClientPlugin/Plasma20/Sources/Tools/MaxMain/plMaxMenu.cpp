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
#include "max.h"
#include "iMenuMan.h"

#include "plMaxMenu.h"
#include "plActionTableMgr.h"
#include "resource.h"
#include "plSaveSelected.h"
#include "plComponentDlg.h"
#include "plMaxCFGFile.h"
#include "plResCollector.h"
#include "plAgeDescInterface.h"
#include "../MaxSceneViewer/SceneViewer.h"
#include "plNodeLock.h"
#include "plResetXform.h"
#include "plTextureSearch.h"
#include "../MaxExport/plExportDlg.h"

//  ************************* action table
//

// random id to identify an action table and context
const ActionTableId kActionId = 0x54162b7c;


enum
{
	kActionSaveSel,
	kActionMerge,
	kActionComponent,
	kActionResCollect,
	kActionAgeDesc,
	kActionCompCopy,
	kActionSceneViewer,
	kActionLock,
	kActionUnlock,
	kActionTexSearch,
	kActionReset,
	kActionSelectNonRenderables,
	kActionExport,
};

static ActionDescription spActions[] = 
{
	{
		kActionSaveSel,			// ActionId identifies action for callback execution (within table)
		IDS_ACT1_DESC,			// Action description for display in ui customization
		IDS_ACT1_NAME,			// Action name for display on menu
		IDS_ACT_CAT				// Action category for ui customization
	},
	{
		kActionMerge,
		IDS_ACT2_DESC,
		IDS_ACT2_NAME,
		IDS_ACT_CAT
	},
	{
		kActionComponent,
		IDS_ACT3_NAME,
		IDS_ACT3_NAME,
		IDS_ACT_CAT
	},
	{
		kActionResCollect,
		IDS_ACT4_NAME,
		IDS_ACT4_NAME,
		IDS_ACT_CAT
	},
	{
		kActionAgeDesc,
		IDS_ACT5_NAME,
		IDS_ACT5_NAME,
		IDS_ACT_CAT
	},
	{
		kActionCompCopy,
		IDS_ACT6_NAME,
		IDS_ACT6_NAME,
		IDS_ACT_CAT
	},
	{
		kActionSceneViewer,
		IDS_ACT7_NAME,
		IDS_ACT7_NAME,
		IDS_ACT_CAT
	},
	{
		kActionLock,
		IDS_ACT8_DESC,
		IDS_ACT8_NAME,
		IDS_ACT_CAT
	},
	{
		kActionUnlock,
		IDS_ACT9_DESC,
		IDS_ACT9_NAME,
		IDS_ACT_CAT
	},
	{
		kActionTexSearch,
		IDS_ACT10_NAME,
		IDS_ACT10_NAME,
		IDS_ACT_CAT
	},
	{
		kActionReset,
		IDS_ACT11_DESC,
		IDS_ACT11_NAME,
		IDS_ACT_CAT
	},
	{
		kActionSelectNonRenderables,
		IDS_ACT12_DESC,
		IDS_ACT12_NAME,
		IDS_ACT_CAT
	},
	{
		kActionExport,
		IDS_PLASMA_EXPORT,
		IDS_PLASMA_EXPORT,
		IDS_ACT_CAT
	}
};

// callback for action exection
bool DoAction(int id)
{
	switch(id)
	{
	case kActionSaveSel:
		plSaveSelected();
		return true;

	case kActionMerge:
		plMerge();
		return true;

	case kActionComponent:
		plComponentDlg::Instance().Open();
		return true;

	case kActionResCollect:
		plResCollector::Collect();
		return true;

	case kActionAgeDesc:
		plAgeDescInterface::Instance().Open();
		return true;

	case kActionCompCopy:
		CopyComponents();
		return true;

	case kActionSceneViewer:
		SceneViewer::Instance().Show();
		return true;

	case kActionLock:
		plNodeLock().Lock();
		return true;

	case kActionUnlock:
		plNodeLock().Unlock();
		return true;

	case kActionTexSearch:
		plTextureSearch::Instance().Toggle();
		return true;

	case kActionReset:
		plResetXform().ResetSelected();
		return true;

	case kActionSelectNonRenderables:
		plSelectNonRenderables().SelectNonRenderables();
		return true;

	case kActionExport:
		plExportDlg::Instance().Show();
		return true;
	}

	return false;
}

static ActionTableInfo actionInfo(kActionId, "Plasma", spActions, sizeof(spActions) / sizeof(ActionDescription));

// static or global declaration of action table manager
plActionTableMgr theActionTableMgr(actionInfo, DoAction);


//
//  ************************* end action table

//////////////////////////////////////////////////////////////////////////////
// Menu Creation Junk

MenuContextId kMyMenuContextId=0xcff95f6c;  //<random number>
static char *kMenuName = "Plasma";
static int kMenuVersion = 10;	// Increment this number if you add an entry to the menu

extern TCHAR *GetString(int id);

void AddPlasmaExportMenu()
{
	IMenuManager* pMenuMan = GetCOREInterface()->GetMenuManager();
	IMenu* fileMenu = pMenuMan->FindMenu("&File");

	int i;

	bool plasmaExportFound = false;

	// Make sure our action isn't already in the menu
	TSTR ourName = GetString(IDS_PLASMA_EXPORT);
	for (i = 0; i < fileMenu->NumItems(); i++)
	{
		IMenuItem* fileMenuItem = fileMenu->GetItem(i);
		const TSTR& title = fileMenuItem->GetTitle();
		if (title == ourName)
			plasmaExportFound = true;

		// KLUDGE - MaxAss didn't define the file submenu with an accelerator.
		// This fixes it.
		if (title == "MAX File Operations")
		{
			fileMenuItem->SetUseCustomTitle(true);
			bool custom = fileMenuItem->GetUseCustomTitle();
			fileMenuItem->SetTitle("MAX File Opera&tions");

			pMenuMan->UpdateMenuBar();
		}
	}

	if (!plasmaExportFound)
	{
		// Menu item isn't there, add it
		for (i = 0; i < fileMenu->NumItems(); i++)
		{
			IMenuItem* fileMenuItem = fileMenu->GetItem(i);
			const TSTR& title = fileMenuItem->GetTitle();
			// We want to add it after the "Export Selected" menu item
			if (title == "Export Selected...")
			{
				ActionTable* pActionTable = GetCOREInterface()->GetActionManager()->FindTable(kActionId);
				if (!pActionTable)
				{
					hsAssert(0, "Action table not found");
					return;
				}

				IMenuItem* menuItem = GetIMenuItem();
				menuItem->SetActionItem(pActionTable->GetAction(kActionExport));
				fileMenu->AddItem(menuItem, i+1);

				pMenuMan->UpdateMenuBar();

				return;
			}
		}
	}
}

void plCreateMenu()
{
	AddPlasmaExportMenu();

	IMenuManager* pMenuMan = GetCOREInterface()->GetMenuManager();
	bool newlyRegistered = pMenuMan->RegisterMenuBarContext(kMyMenuContextId, kMenuName);

	// Is the Max menu version the most recent?
	bool wrongVersion = GetPrivateProfileInt("Menu", "Version", 0, plMaxConfig::GetPluginIni()) < kMenuVersion;
	if (wrongVersion)
	{
		// Delete the old version of the menu
		IMenu *oldMenu = pMenuMan->FindMenu(kMenuName);
		if (oldMenu)
			pMenuMan->UnRegisterMenu(oldMenu);

		// Update the menu version
		char buf[30];
		WritePrivateProfileString("Menu", "Version", itoa(kMenuVersion, buf, 10), plMaxConfig::GetPluginIni());
	}
	
	if (wrongVersion || newlyRegistered)
	{
		IMenu *pMainMenu = pMenuMan->GetMainMenuBar();
		if (!pMainMenu)
		{
			hsAssert(0, "Main menu not found");
			return;
		}

		// Get our action table
		ActionTable* pActionTable = GetCOREInterface()->GetActionManager()->FindTable(kActionId);
		if (!pActionTable)
		{
			hsAssert(0, "Action table not found");
			return;
		}

		// Create the Plasma menu
		IMenu* pPlasmaMenu = GetIMenu();
		pPlasmaMenu->SetTitle(kMenuName);

		// Register the new menu with the system
		pMenuMan->RegisterMenu(pPlasmaMenu, 0);

		/////////////////////////////////////////////////
		// Add the menu items
		//

		// Add the save selected action to the menu
		IMenuItem* pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionSaveSel));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the merge action to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionMerge));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the component copy action to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionCompCopy));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add a separator
		pMenuItem = GetIMenuItem();
		pMenuItem->ActAsSeparator();
		pPlasmaMenu->AddItem(pMenuItem);
	
		// Add the component manager to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionComponent));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the resource collector to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionResCollect));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the texture search to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionTexSearch));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the age description to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionAgeDesc));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add a separator
		pMenuItem = GetIMenuItem();
		pMenuItem->ActAsSeparator();
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the SceneViewer to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionSceneViewer));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add a separator
		pMenuItem = GetIMenuItem();
		pMenuItem->ActAsSeparator();
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the Lock Selected to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionLock));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the Unlock Selected to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionUnlock));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the Reset Selected to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionReset));
		pPlasmaMenu->AddItem(pMenuItem);

		// Add the SelectNonRenderables to the menu
		pMenuItem = GetIMenuItem();
		pMenuItem->SetActionItem(pActionTable->GetAction(kActionSelectNonRenderables));
		pPlasmaMenu->AddItem(pMenuItem);

        // Create a new menu item to hold the sub-menu
		IMenuItem* pSubMenuItem1 = GetIMenuItem();   //menu in menu bar...
		pSubMenuItem1->SetSubMenu(pPlasmaMenu);
		pMainMenu->AddItem(pSubMenuItem1);

		pMenuMan->UpdateMenuBar();

		// Save the dang menu, in case Max crashes
		const char *uiDir = GetCOREInterface()->GetDir(APP_UI_DIR);
		char path[MAX_PATH];
		sprintf(path, "%s\\%s", uiDir, "MaxMenus.mnu");
		
		pMenuMan->SaveMenuFile(path);
	}

}

// End Menu Creation Junk
//////////////////////////////////////////////////////////////////////////////

