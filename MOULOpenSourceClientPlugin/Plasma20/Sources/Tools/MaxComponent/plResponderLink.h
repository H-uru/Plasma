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
#include "hsTypes.h"
#include "plResponderCmd.h"
#include "../pnKeyedObject/plKey.h"

class plResponderCmdLink : public plResponderCmd
{
public:
	static plResponderCmdLink& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Link"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual void SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb);
	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdEnable : public plResponderCmd
{
public:
	static plResponderCmdEnable& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return "Enable/Disable"; }
	virtual const char *GetName(int idx) { return "Responder"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdPhysEnable : public plResponderCmd
{
public:
	static plResponderCmdPhysEnable& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return "Enable/Disable"; }
	virtual const char *GetName(int idx) { return "Physical"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdOneShot : public plResponderCmd
{
public:
	static plResponderCmdOneShot& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "One Shot"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);

	virtual bool IsWaitable(IParamBlock2 *pb) { return true; }
	virtual void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo);
};

class plResponderCmdNotify : public plResponderCmd
{
public:
	static plResponderCmdNotify& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Notify Triggerer"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb) { return GetName(0); }

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdDetectorEnable : public plResponderCmd
{
public:
	static plResponderCmdDetectorEnable& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return "Enable/Disable"; }
	virtual const char *GetName(int idx) { return "Detector"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdXRegion : public plResponderCmd
{
public:
	static plResponderCmdXRegion& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes();
	virtual const char *GetCategory(int idx);
	virtual const char *GetName(int idx);
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual IParamBlock2 *CreatePB(int idx);
	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdCamTransition : public plResponderCmd
{
public:
	static plResponderCmdCamTransition& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Camera Transition"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdCamForce : public plResponderCmd
{
public:
	enum { kForce3rd, kResume1st };

	static plResponderCmdCamForce& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Camera Force 3rd"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdDelay : public plResponderCmd
{
public:
	static plResponderCmdDelay& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Delay"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);

	virtual bool IsWaitable(IParamBlock2 *pb) { return true; }
	virtual void CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo);
};

class plResponderCmdVisibility : public plResponderCmd
{
public:
	static plResponderCmdVisibility& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes();
	virtual const char *GetCategory(int idx);
	virtual const char *GetName(int idx);
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual IParamBlock2 *CreatePB(int idx);
	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdSubWorld : public plResponderCmd
{
public:
	static plResponderCmdSubWorld& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes();
	virtual const char *GetCategory(int idx);
	virtual const char *GetName(int idx);
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual IParamBlock2 *CreatePB(int idx);
	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdFootSurface : public plResponderCmd
{
public:
	static plResponderCmdFootSurface& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Footstep Surface"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};

class plResponderCmdMultistage : public plResponderCmd
{
public:
	static plResponderCmdMultistage& Instance();
	virtual ParamBlockDesc2 *GetDesc();

	virtual int NumTypes() { return 1; }
	virtual const char *GetCategory(int idx) { return nil; }
	virtual const char *GetName(int idx) { return "Trigger Multistage"; }
	virtual const char *GetInstanceName(IParamBlock2 *pb);

	virtual plMessage *CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb);
};
