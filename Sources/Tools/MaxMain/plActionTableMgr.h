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
 plActionTableMgr.h

 Eric Ellis
******************************************************************************/

#ifndef __PLACTIONTABLEMGR_H
#define __PLACTIONTABLEMGR_H

#include "Max.h"
#include "notify.h"
#include <vector>
#include "hsTypes.h"

typedef bool(*ActionCallbackFunc)(int);


/******************************************************************************
 Helper classes for plActionTableMgr
******************************************************************************/

class ActionTableMgrCB : public ActionCallback
{
	ActionCallbackFunc fCallbackFunc;

public:
	ActionTableMgrCB(ActionCallbackFunc cbFunc) {fCallbackFunc = cbFunc;}

	BOOL ExecuteAction(int id) { return fCallbackFunc(id) ? TRUE : FALSE; }
};


class ActionTableInfo
{
	friend class plActionTableMgr;

private:
	ActionTableMgrCB* ActionCB;
	bool Created;

public:
	ActionTableId TableId;
	ActionContextId ContextId;
	TSTR Name;

	std::vector<ActionDescription> Actions;

	ActionTableInfo(ActionTableId actionId, TCHAR* name, ActionDescription actions[], int numActions)
	{
		TableId = actionId;
		ContextId = actionId;
		Name = name;
		Created = false;
		ActionCB = NULL;

		for(int i = 0; i < numActions; i++)
		{
			Actions.push_back(actions[i]);
		}
	}

	ActionTableInfo()
	{
		TableId = 0;
		ContextId = 0;
		Created = false;
		ActionCB = NULL;
	}

	virtual ~ActionTableInfo()
	{
		delete ActionCB;
	}
};



/******************************************************************************
 plActionTableMgr class defintion
******************************************************************************/

class plActionTableMgr
{
	std::vector<ActionTableInfo*> fActionTables;

public:
	plActionTableMgr(ActionTableInfo& actionTable, ActionCallbackFunc cbFunc);
	virtual ~plActionTableMgr();

	void AddActionTable(ActionTableInfo& actionTable, ActionCallbackFunc cbFunc);

	int NumActionTables() { return fActionTables.size(); }
	ActionTable* GetActionTable(int i);

private:
	static void SysStartup(void *param, NotifyInfo *info);
	static void SysShutdown(void *param, NotifyInfo *info);
};


#endif __PLACTIONTABLEMGR_H
