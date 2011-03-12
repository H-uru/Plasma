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
#include "plResponderCmd.h"
#include "../pnKeyedObject/plKey.h"

class plMessage;
class IParamBlock2;
class ClassDesc2;
class plMaxNode;
class plErrorMsg;
class plComponentBase;
class Mtl;
class plMaxNodeBase;
template <class T> class hsTArray;

int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char *segName, hsTArray<plKey>& keys);

class plResponderCmdMtl : public plResponderCmd
{
public:
	static plResponderCmdMtl& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes();
	virtual const char *GetCategory(int type);
	virtual const char *GetName(int type);
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual IParamBlock2 *CreatePB(int type);
	virtual void SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb);
	virtual plMessage* CreateMsg(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb);

	virtual bool IsWaitable(IParamBlock2 *pb);
	virtual void GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints);
	virtual void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo);
	
	Mtl *GetMtl(IParamBlock2 *pb);
	const char *GetAnim(IParamBlock2 *pb);
};

int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char* segName, hsTArray<plKey>& keys);
