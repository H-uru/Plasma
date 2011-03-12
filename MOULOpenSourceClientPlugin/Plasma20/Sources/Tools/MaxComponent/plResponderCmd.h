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
#ifndef plResponderCmd_h_inc
#define plResponderCmd_h_inc

#include "hsConfig.h"
#include <vector>
#include <string>
#include "../pnKeyedObject/plKey.h"

class ParamBlockDesc2;
class IParamBlock2;
class plMessage;
class plErrorMsg;
class plMaxNode;

class ResponderWaitInfo
{
public:
	const char* responderName;	// For error messages

	plMessage *msg;		// Message created by the responder command
	plKey receiver;		// Key to send the callback message to
	const char *point;	// Marker name to wait on (nil for end)
	int callbackUser;	// Value to set the callback user value to
};

class plResponderCmd
{
public:
	static plResponderCmd *Find(IParamBlock2 *pb);

	virtual ParamBlockDesc2 *GetDesc()=0;

	virtual int NumTypes()=0;
	virtual const char *GetCategory(int idx)=0;
	virtual const char *GetName(int idx)=0;
	virtual const char *GetInstanceName(IParamBlock2 *pb)=0;

	virtual IParamBlock2 *CreatePB(int idx);

	// In case the command needs to set any properties on a node
	virtual void SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb) {}
	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb)=0;

	typedef std::vector<std::string> WaitPoints;
	// Can other commands wait on you?
	virtual bool IsWaitable(IParamBlock2 *pb) { return false; }
	// The names of the points commands can wait on (or leave empty for only 'end')
	virtual void GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints) {}
	// Take your message and modify it to send a callback to 'receiver', with fUser set to callbackUser
	virtual void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo) {}
};

#endif // plResponderCmd_h_inc