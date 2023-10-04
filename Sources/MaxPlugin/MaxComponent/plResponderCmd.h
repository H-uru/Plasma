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
#ifndef plResponderCmd_h_inc
#define plResponderCmd_h_inc

#include <vector>
#include <string>
#include "pnKeyedObject/plKey.h"

class plErrorMsg;
class plMaxNode;
class plMessage;
class ParamBlockDesc2;
class IParamBlock2;

class ResponderWaitInfo
{
public:
    ST::string responderName;  // For error messages

    plMessage *msg;     // Message created by the responder command
    plKey receiver;     // Key to send the callback message to
    ST::string point;   // Marker name to wait on (nil for end)
    int callbackUser;   // Value to set the callback user value to
};

class plResponderCmd
{
public:
    static plResponderCmd *Find(IParamBlock2 *pb);

    virtual ParamBlockDesc2 *GetDesc()=0;

    virtual int NumTypes()=0;
    virtual const TCHAR* GetCategory(int idx)=0;
    virtual const TCHAR* GetName(int idx)=0;
    virtual const TCHAR* GetInstanceName(IParamBlock2 *pb)=0;

    virtual IParamBlock2 *CreatePB(int idx);

    // In case the command needs to set any properties on a node
    virtual void SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb) {}
    virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb)=0;

    typedef std::vector<ST::string> WaitPoints;
    // Can other commands wait on you?
    virtual bool IsWaitable(IParamBlock2 *pb) { return false; }
    // The names of the points commands can wait on (or leave empty for only 'end')
    virtual void GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints) {}
    // Take your message and modify it to send a callback to 'receiver', with fUser set to callbackUser
    virtual void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo) {}
};

#endif // plResponderCmd_h_inc