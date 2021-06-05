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

#ifndef plResponderLink_inc
#define plResponderLink_inc

#include "MaxMain/MaxAPI.h"

#include "plResponderCmd.h"
#include "pnKeyedObject/plKey.h"

class plErrorMsg;
class plMaxNode;
class IParamBlock2;
class ParamBlockDesc2;

class plResponderCmdLink : public plResponderCmd
{
public:
    static plResponderCmdLink& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Link"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    void SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb) override;
    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdEnable : public plResponderCmd
{
public:
    static plResponderCmdEnable& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return _T("Enable/Disable"); }
    const TCHAR* GetName(int idx) override { return _T("Responder"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdPhysEnable : public plResponderCmd
{
public:
    static plResponderCmdPhysEnable& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return _T("Enable/Disable"); }
    const TCHAR* GetName(int idx) override { return _T("Physical"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdOneShot : public plResponderCmd
{
public:
    static plResponderCmdOneShot& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("One Shot"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;

    bool IsWaitable(IParamBlock2 *pb) override { return true; }
    void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo) override;
};

class plResponderCmdNotify : public plResponderCmd
{
public:
    static plResponderCmdNotify& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Notify Triggerer"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override { return GetName(0); }

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdDetectorEnable : public plResponderCmd
{
public:
    static plResponderCmdDetectorEnable& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return _T("Enable/Disable"); }
    const TCHAR* GetName(int idx) override { return _T("Detector"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdXRegion : public plResponderCmd
{
public:
    static plResponderCmdXRegion& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override;
    const TCHAR* GetCategory(int idx) override;
    const TCHAR* GetName(int idx) override;
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    IParamBlock2 *CreatePB(int idx) override;
    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdCamTransition : public plResponderCmd
{
public:
    static plResponderCmdCamTransition& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Camera Transition"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdCamForce : public plResponderCmd
{
public:
    enum { kForce3rd, kResume1st };

    static plResponderCmdCamForce& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Camera Force 3rd"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdDelay : public plResponderCmd
{
public:
    static plResponderCmdDelay& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Delay"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;

    bool IsWaitable(IParamBlock2 *pb) override { return true; }
    void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo) override;
};

class plResponderCmdVisibility : public plResponderCmd
{
public:
    static plResponderCmdVisibility& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override;
    const TCHAR* GetCategory(int idx) override;
    const TCHAR* GetName(int idx) override;
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    IParamBlock2 *CreatePB(int idx) override;
    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdSubWorld : public plResponderCmd
{
public:
    static plResponderCmdSubWorld& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override;
    const TCHAR* GetCategory(int idx) override;
    const TCHAR* GetName(int idx) override;
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    IParamBlock2 *CreatePB(int idx) override;
    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdFootSurface : public plResponderCmd
{
public:
    static plResponderCmdFootSurface& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Footstep Surface"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

class plResponderCmdMultistage : public plResponderCmd
{
public:
    static plResponderCmdMultistage& Instance();
    ParamBlockDesc2 *GetDesc() override;

    int NumTypes() override { return 1; }
    const TCHAR* GetCategory(int idx) override { return nullptr; }
    const TCHAR* GetName(int idx) override { return _T("Trigger Multistage"); }
    const TCHAR* GetInstanceName(IParamBlock2 *pb) override;

    plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb) override;
};

#endif
