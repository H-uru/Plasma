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
#ifndef PL_COMPONENTMGR_H
#define PL_COMPONENTMGR_H

#include "hsTypes.h"
#include "max.h"
#include "utilapi.h"

#include <vector>
using namespace std;

class ClassDesc2;
class Class_ID;

#define COMPONENT_MGR_CID Class_ID(0x5b870ba2, 0xb7b1da2)

//
// Manages the ClassDesc's for all components.  Each component calls a macro that
// registers it with the component mgr.  Then when Max loads the plugin the plugin
// enumeration functions call this to get all the registered ones.  This saves us
// from having to go change those functions every time we add a new component.
//
// 8/28/01: Added globals -Colin
//
class hsResMgr;
class plFactory;
class plTimerCallbackManager;
class plTimerShare;

class plComponentMgr : public UtilityObj
{
private:
	vector<ClassDesc*> fDescs;

	plComponentMgr() {};

public:
	static plComponentMgr &Inst();

	// Required Max functions
	virtual void BeginEditParams(Interface *ip,IUtil *iu) {}
	virtual void EndEditParams(Interface *ip,IUtil *iu) {}
	virtual void SelectionSetChanged(Interface *ip,IUtil *iu) {}
	virtual void DeleteThis() {}

	virtual UInt32 Count();
	virtual ClassDesc *Get(UInt32 i);

	virtual UInt32 FindClassID(Class_ID id);

	// Registers a component.  Only used by the classdesc macro.
	virtual void Register(ClassDesc *desc);
};

#endif //PL_COMPONENTMGR_H