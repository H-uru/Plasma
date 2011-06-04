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
/******************************************************************************
 plActionTableMgr.cpp

 Eric Ellis
******************************************************************************/

#include "HeadSpin.h"
#include "plActionTableMgr.h"


plActionTableMgr::plActionTableMgr(ActionTableInfo& actionTable, ActionCallbackFunc cbFunc)
{
	AddActionTable(actionTable, cbFunc);

	RegisterNotification(SysStartup, this, NOTIFY_SYSTEM_STARTUP);
	RegisterNotification(SysShutdown, this, NOTIFY_SYSTEM_SHUTDOWN);
}


plActionTableMgr::~plActionTableMgr()
{
	UnRegisterNotification(SysStartup, this, NOTIFY_SYSTEM_STARTUP);
	UnRegisterNotification(SysShutdown, this, NOTIFY_SYSTEM_SHUTDOWN);
}


void plActionTableMgr::AddActionTable(ActionTableInfo& actionTable, ActionCallbackFunc cbFunc)
{
	actionTable.ActionCB = new ActionTableMgrCB(cbFunc);
	fActionTables.push_back(&actionTable);
}

CoreExport void *__cdecl MAX_new(size_t size);
CoreExport void __cdecl MAX_delete(void* mem);

class plActionTable : public ActionTable
{
public:
	plActionTable(ActionTableId id, ActionContextId contextId, TSTR& name, HACCEL hDefaults, int numIds, ActionDescription* pOps, HINSTANCE hInst)
		: ActionTable(id, contextId, name, hDefaults, numIds, pOps, hInst) {}
	plActionTable(ActionTableId id, ActionContextId contextId, TSTR& name)
		: ActionTable(id, contextId, name) {}

	void *operator new (size_t)
	{
		return MAX_new(sizeof(plActionTable));
	}
	void operator delete (void * mem)
	{
		MAX_delete(mem);
	}
};

ActionTable* plActionTableMgr::GetActionTable(int i)
{
	if(i > this->NumActionTables())
	{
		return NULL;
	}

	ActionTableInfo* pTableInfo = fActionTables[i];

	if(pTableInfo->Created)
	{
		return GetCOREInterface()->GetActionManager()->FindTable(pTableInfo->TableId);
	}


    ActionTable *pTab = new plActionTable(
		pTableInfo->TableId, 
		pTableInfo->ContextId, 
		pTableInfo->Name, NULL, 
		pTableInfo->Actions.size(), 
		&pTableInfo->Actions[0], 
		hInstance
		);

	// register the action table with the system before we hand it off
    GetCOREInterface()->GetActionManager()->RegisterActionContext(pTableInfo->ContextId, pTableInfo->Name);

	pTableInfo->Created = true;

	return pTab;
}


void plActionTableMgr::SysStartup(void *param, NotifyInfo *info) 
{
	plActionTableMgr* pActionTableMgr = (plActionTableMgr*)param;

//		((MenuTestUtil*)param)->CreateMenu();  //setup menus

	IActionManager* pActionMgr = GetCOREInterface()->GetActionManager();

	for(int i = 0; i < pActionTableMgr->NumActionTables(); i++)
	{
		ActionTableInfo* pTableInfo = pActionTableMgr->fActionTables[i];

		pActionMgr->ActivateActionTable(pTableInfo->ActionCB, pTableInfo->TableId);
	}
}


void plActionTableMgr::SysShutdown(void *param, NotifyInfo *info) 
{
	plActionTableMgr* pActionTableMgr = (plActionTableMgr*)param;

	IActionManager* pActionMgr = GetCOREInterface()->GetActionManager();

	for(int i = 0; i < pActionTableMgr->NumActionTables(); i++)
	{
		ActionTableInfo* pTableInfo = pActionTableMgr->fActionTables[i];

		pActionMgr->DeactivateActionTable(pTableInfo->ActionCB, pTableInfo->TableId);
	}
}
