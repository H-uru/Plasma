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
#include "plResponderAnim.h"
#include "plResponderComponentPriv.h"
#include "resource.h"
#include "max.h"

#include "../MaxMain/plMaxNode.h"

#include "plAnimComponent.h"
#include "plAudioComponents.h"

#include "plMaxAnimUtils.h"
#include <vector>

// Needed for anim msg creation
#include "../pnKeyedObject/plKey.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "plNotetrackAnim.h"

// Needed for sound msg creation
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "plAudible.h"
#include "../pnMessage/plSoundMsg.h"

#include "../MaxMain/plPlasmaRefMsgs.h"

enum
{
	kAnimComp,
	kAnimLoop,
	kAnimType,
	kAnimOwner,
	kAnimObject,
	kAnimObjectType,
};

class plResponderAnimProc;
extern plResponderAnimProc gResponderAnimProc;

ParamBlockDesc2 gResponderAnimBlock
(
	kResponderAnimBlk, _T("animCmd"), 0, NULL, P_AUTO_UI,

	IDD_COMP_RESPOND_ANIM, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderAnimProc,

	kAnimComp,	_T("comp"),		TYPE_REFTARG,		0, 0,
		end,

	kAnimObject, _T("object"),	TYPE_REFTARG,		0, 0,
		end,

	kAnimLoop,	_T("loop"),		TYPE_STRING,		0, 0,
		end,

	kAnimType,	_T("type"),		TYPE_INT,			0, 0,
		end,

	kAnimObjectType,	_T("objType"),	TYPE_INT,	0, 0,
		end,

	end
);

enum AnimObjectType
{
	kNodePB,		// Use the node in the PB
	kNodeResponder	// Use the node the responder is attached to
};

plResponderCmdAnim& plResponderCmdAnim::Instance()
{
	static plResponderCmdAnim theInstance;
	return theInstance;
}

ParamBlockDesc2 *plResponderCmdAnim::GetDesc()
{
	return &gResponderAnimBlock;
}

// Use the old types, for backwards compatibility
enum 
{
	kRespondPlayAnim, 
	kRespondStopAnim, 
	kRespondToggleAnim, 
	kRespondLoopAnimOn,
	kRespondLoopAnimOff, 
	kRespondSetForeAnim=7,
	kRespondSetBackAnim,

	kRespondPlaySound,
	kRespondStopSound,
	kRespondToggleSound,
	kRespondSyncedPlaySound,

	kRespondPlayRndSound=19,
	kRespondStopRndSound,
	kRespondToggleRndSound,

	kRespondRewindAnim,
	kRespondRewindSound,
	kRespondFastForwardAnim,
	
	kNumTypes = 16
};

int plResponderCmdAnim::NumTypes()
{
	return kNumTypes;
}

static int IndexToOldType(int idx)
{
	static int oldTypes[] =
	{
		kRespondPlayAnim,
		kRespondStopAnim, 
		kRespondToggleAnim, 
		kRespondLoopAnimOn,
		kRespondLoopAnimOff, 
		kRespondSetForeAnim,
		kRespondSetBackAnim,

		kRespondPlaySound,
		kRespondStopSound,
		kRespondToggleSound,
		kRespondSyncedPlaySound,

		kRespondPlayRndSound,
		kRespondStopRndSound,
		kRespondToggleRndSound,

		kRespondRewindAnim,
		kRespondRewindSound,
		kRespondFastForwardAnim,
	};

	hsAssert(idx < kNumTypes, "Bad index");
	return oldTypes[idx];
}

const char *plResponderCmdAnim::GetCategory(int idx)
{
	int type = IndexToOldType(idx);

	switch (type)
	{
	case kRespondPlayAnim:
	case kRespondStopAnim:
	case kRespondToggleAnim:
	case kRespondLoopAnimOn:
	case kRespondLoopAnimOff:
	case kRespondSetForeAnim:
	case kRespondSetBackAnim:
	case kRespondRewindAnim:
	case kRespondFastForwardAnim:
		return "Animation";

	case kRespondPlaySound:
	case kRespondStopSound:
	case kRespondToggleSound:
	case kRespondRewindSound:
	case kRespondSyncedPlaySound:
		return "Sound";

	case kRespondPlayRndSound:
	case kRespondStopRndSound:
	case kRespondToggleRndSound:
		return "Random Sound";
	}

	return nil;
}

const char *plResponderCmdAnim::GetName(int idx)
{
	int type = IndexToOldType(idx);

	switch (type)
	{
	case kRespondPlayAnim:
	case kRespondPlaySound:
	case kRespondPlayRndSound:
		return "Play";

	case kRespondStopAnim:
	case kRespondStopSound:
	case kRespondStopRndSound:
		return "Stop";

	case kRespondToggleAnim:
	case kRespondToggleSound:
	case kRespondToggleRndSound:
		return "Toggle";

	case kRespondLoopAnimOn:
		return "Set Looping On";
	case kRespondLoopAnimOff:
		return "Set Looping Off";
	case kRespondSetForeAnim:
		return "Set Forwards";
	case kRespondSetBackAnim:
		return "Set Backwards";

	case kRespondRewindAnim:
	case kRespondRewindSound:
		return "Rewind";
	case kRespondFastForwardAnim:
		return "Fast Forward";
	case kRespondSyncedPlaySound:
		return "Synched Play";
	}

	return nil;
}

static const char *GetShortName(int type)
{
	switch (type)
	{
	case kRespondPlayAnim:			return "Anim Play";
	case kRespondStopAnim:			return "Anim Stop";
	case kRespondToggleAnim:		return "Anim Toggle";
	case kRespondLoopAnimOn:		return "Anim Loop On";
	case kRespondLoopAnimOff:		return "Anim Loop Off";
	case kRespondSetForeAnim:		return "Anim Set Fore";
	case kRespondSetBackAnim:		return "Anim Set Back";
	case kRespondPlaySound:			return "Snd Play";
	case kRespondSyncedPlaySound:	return "Snd Synched Play";
	case kRespondStopSound:			return "Snd Stop";
	case kRespondToggleSound:		return "Snd Toggle";
	case kRespondPlayRndSound:		return "Rnd Snd Play";
	case kRespondStopRndSound:		return "Rnd Snd Stop";
	case kRespondToggleRndSound:	return "Rnd Snd Toggle";
	case kRespondRewindAnim:		return "Anim Rewind";
	case kRespondRewindSound:		return "Snd Rewind";
	case kRespondFastForwardAnim:	return "Anim FFwd";
	}

	return nil;
}
const char *plResponderCmdAnim::GetInstanceName(IParamBlock2 *pb)
{
	static char name[256];

	const char *shortName = GetShortName(pb->GetInt(kAnimType));
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(kAnimComp);
	sprintf(name, "%s (%s)", shortName, node ? node->GetName() : "none");

	return name;
}

static bool IsSoundMsg(int type)
{
	if (type == kRespondPlaySound ||
		type == kRespondSyncedPlaySound ||
		type == kRespondStopSound ||
		type == kRespondToggleSound ||
		type == kRespondPlayRndSound ||
		type == kRespondStopRndSound ||
		type == kRespondToggleRndSound ||
		type == kRespondRewindSound)
		return true;
	return false;
}

IParamBlock2 *plResponderCmdAnim::CreatePB(int idx)
{
	int type = IndexToOldType(idx);

	// Create the paramblock and save it's type
	IParamBlock2 *pb = CreateParameterBlock2(&gResponderAnimBlock, nil);
	pb->SetValue(kAnimType, 0, type);

	return pb;
}

plComponentBase *plResponderCmdAnim::GetComponent(IParamBlock2 *pb)
{
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(kAnimComp);
	if (node)
		return node->ConvertToComponent();
	else
		return nil;
}

plMessage *plResponderCmdAnim::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
	if (IsSoundMsg(pb->GetInt(kAnimType)))
		return ICreateSndMsg(node, pErrMsg, pb);
	else
		return ICreateAnimMsg(node, pErrMsg, pb);
}

bool GetCompAndNode(IParamBlock2* pb, plMaxNode* node, plComponentBase*& comp, plMaxNode*& targNode)
{
	plMaxNode *compNode = (plMaxNode*)pb->GetReferenceTarget(kAnimComp);
	if (!compNode)
		return false;

	comp = compNode->ConvertToComponent();

	// KLUDGE: Anim group components don't need target nodes
	if (comp->ClassID() == ANIM_GROUP_COMP_CID)
		return true;

	if (pb->GetInt(kAnimObjectType) == kNodeResponder)
		targNode = node;
	else
		targNode = (plMaxNode*)pb->GetReferenceTarget(kAnimObject);

	if (!targNode)
		return false;

	if (!comp->IsTarget(targNode))
		return false;

	return true;
}

plMessage *plResponderCmdAnim::ICreateAnimMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
	plAnimComponentBase *comp = nil;
	plMaxNode *targNode = nil;
	if (!GetCompAndNode(pb, node, (plComponentBase*&)comp, targNode))
		throw "A valid animation component and node were not found";

	// Get the anim modifier keys for all nodes this comp is attached to
	plKey animKey = comp->GetModKey(targNode);
	if (!animKey)
		throw "Animation component didn't convert";

	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg;
	msg->AddReceiver(animKey);

	const char *tempAnimName = comp->GetAnimName();
	msg->SetAnimName(tempAnimName);

	// Create and initialize a message for the command
	switch (pb->GetInt(kAnimType))
	{
	case kRespondPlayAnim:
		msg->SetCmd(plAnimCmdMsg::kContinue);
		break;
	case kRespondStopAnim:
		msg->SetCmd(plAnimCmdMsg::kStop);
		break;
	case kRespondToggleAnim:
		msg->SetCmd(plAnimCmdMsg::kToggleState);
		break;
	case kRespondSetForeAnim:
		msg->SetCmd(plAnimCmdMsg::kSetForewards);
		break;
	case kRespondSetBackAnim:
		msg->SetCmd(plAnimCmdMsg::kSetBackwards);
		break;
	case kRespondLoopAnimOn:
		{
			msg->SetCmd(plAnimCmdMsg::kSetLooping);
			// KLUDGE - We send the loop to play by name here, so anim grouped components
			// could have loops with different begin and end points.  However, apparently
			// that functionality was never implemented, whoops.  So, we'll take out the
			// stuff that actually tries to set the begin and end points for now, so that
			// anims with a loop set in advance will actually work with this. -Colin
//			msg->SetCmd(plAnimCmdMsg::kSetLoopBegin);
//			msg->SetCmd(plAnimCmdMsg::kSetLoopEnd);
			const char *loopName = pb->GetStr(kAnimLoop);
			msg->SetLoopName(loopName);
		}
		break;
	case kRespondLoopAnimOff:
		msg->SetCmd(plAnimCmdMsg::kUnSetLooping);
		break;

	case kRespondRewindAnim:
		msg->SetCmd(plAnimCmdMsg::kGoToBegin);
		break;
	case kRespondFastForwardAnim:
		msg->SetCmd(plAnimCmdMsg::kGoToEnd);
		break;
	}

	return msg;
}

plMessage* plResponderCmdAnim::ICreateSndMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
	plComponentBase *comp;
	plMaxNode *targNode;
	if (!GetCompAndNode(pb, node, comp, targNode))
		throw "A valid sound component and node were not found";

	int type = pb->GetInt(kAnimType);
	switch (type)
	{
		case kRespondPlaySound:
		case kRespondStopSound:
		case kRespondToggleSound:
		case kRespondRewindSound:
		case kRespondSyncedPlaySound:
		{
			if (!targNode->GetSceneObject())
				throw "Sound emitter didn't export";

			int soundIdx = plAudioComp::GetSoundModIdx(comp, targNode);

			// Changed 8.26.2001 mcn - The audioInterface should be the message receiver,
			// not the audible itself.
			const plAudioInterface *ai = targNode->GetSceneObject()->GetAudioInterface();
			plKey key = ai->GetKey();

			plSoundMsg* msg = TRACKED_NEW plSoundMsg;
			msg->AddReceiver(key);
			msg->fIndex = soundIdx;

			if (type == kRespondPlaySound)
				msg->SetCmd(plSoundMsg::kPlay);
			else if (type == kRespondStopSound)
				msg->SetCmd(plSoundMsg::kStop);
			else if (type == kRespondToggleSound)
				msg->SetCmd(plSoundMsg::kToggleState);
			else if (type == kRespondRewindSound)
			{
				msg->fTime = 0;
				msg->SetCmd(plSoundMsg::kGoToTime);
			}
			else if(type == kRespondSyncedPlaySound)
				msg->SetCmd(plSoundMsg::kSynchedPlay);

			if( plAudioComp::IsLocalOnly( comp ) )
				msg->SetCmd( plSoundMsg::kIsLocalOnly );

			return msg;
		}

		case kRespondPlayRndSound:
		case kRespondStopRndSound:
		case kRespondToggleRndSound:
		{
			plKey key = plAudioComp::GetRandomSoundKey(comp, targNode);
			if (key)
			{
				plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg;
				msg->AddReceiver(key);

				if (type == kRespondPlayRndSound)
					msg->SetCmd(plAnimCmdMsg::kContinue);
				else if (type == kRespondStopRndSound)
					msg->SetCmd(plAnimCmdMsg::kStop);
				else if (type == kRespondToggleRndSound)
					msg->SetCmd(plAnimCmdMsg::kToggleState);

				return msg;
			}
		}
	}

	throw "Unknown sound command";
}

bool plResponderCmdAnim::IsWaitable(IParamBlock2 *pb)
{
	int type = pb->GetInt(kAnimType);
	if (type == kRespondPlayAnim ||
		type == kRespondToggleAnim ||
		type == kRespondStopAnim ||
		type == kRespondPlaySound ||
		type == kRespondSyncedPlaySound ||
		type == kRespondToggleSound)
		return true;

	return false;
}

void plResponderCmdAnim::GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints)
{
	int type = pb->GetInt(kAnimType);
	
	// Don't try and get points for the stop anim, it can only stop at a stop point
	if (type == kRespondStopAnim || IsSoundMsg(type))
		return;

	plAnimComponent *animComp = (plAnimComponent*)GetComponent(pb);
	const char *animName = animComp->GetAnimName();

	if (animComp)
	{
		plNotetrackAnim notetrackAnim(animComp, nil);
		plAnimInfo info = notetrackAnim.GetAnimInfo(animName);
		while (const char *marker = info.GetNextMarkerName())
			waitPoints.push_back(marker);
	}
}

void plResponderCmdAnim::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
	plAnimCmdMsg *animMsg = plAnimCmdMsg::ConvertNoRef(waitInfo.msg);
	if (animMsg)
		animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);

	plSoundMsg *soundMsg = plSoundMsg::ConvertNoRef(waitInfo.msg);
	if (soundMsg)
		soundMsg->SetCmd(plSoundMsg::kAddCallbacks);

	plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
	eventMsg->AddReceiver(waitInfo.receiver);
	eventMsg->fRepeats = 0;
	eventMsg->fUser = waitInfo.callbackUser;

	if (waitInfo.point)
	{
		// FIXME COLIN - Error checking here?
		plAnimComponent *animComp = (plAnimComponent*)GetComponent(pb);
		const char *animName = animComp->GetAnimName();

		plNotetrackAnim notetrackAnim(animComp, nil);
		plAnimInfo info = notetrackAnim.GetAnimInfo(animName);

		eventMsg->fEvent = kTime;
		eventMsg->fEventTime = info.GetMarkerTime(waitInfo.point);
	}
	else
		eventMsg->fEvent = kStop;

	plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(waitInfo.msg);
	callbackMsg->AddCallback(eventMsg);
	// AddCallback adds it's own ref, so remove ours (the default of 1)
	hsRefCnt_SafeUnRef(eventMsg);
}

#include "plAnimCompProc.h"
#include "plPickNode.h"
#include "plResponderGetComp.h"

class plResponderAnimProc : public plAnimCompProc
{
public:
	plResponderAnimProc();
	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	virtual void IPickComponent(IParamBlock2* pb);
	virtual void IPickNode(IParamBlock2* pb, plComponentBase* comp);

	virtual void ILoadUser(HWND hWnd, IParamBlock2* pb);
	virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID);

	virtual void IUpdateNodeButton(HWND hWnd, IParamBlock2* pb);
};
static plResponderAnimProc gResponderAnimProc;

plResponderAnimProc::plResponderAnimProc()
{
	fCompButtonID	= IDC_ANIM_BUTTON;
	fCompParamID	= kAnimComp;
	fNodeButtonID	= IDC_OBJ_BUTTON;
	fNodeParamID	= kAnimObject;
}

BOOL plResponderAnimProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			IParamBlock2 *pb = pm->GetParamBlock();

			int type = pb->GetInt(kAnimType);

			// Only show the loop control if this is a loop command
			int show = (type == kRespondLoopAnimOn) ? SW_SHOW : SW_HIDE;
			ShowWindow(GetDlgItem(hWnd, IDC_LOOP_COMBO), show);
			ShowWindow(GetDlgItem(hWnd, IDC_LOOP_TEXT), show);
			// Resize the dialog if we're not using the loop control
			if (type != kRespondLoopAnimOn)
			{
				RECT itemRect, clientRect;
				GetWindowRect(GetDlgItem(hWnd, IDC_LOOP_TEXT), &itemRect);
				GetWindowRect(hWnd, &clientRect);
				SetWindowPos(hWnd, NULL, 0, 0, clientRect.right-clientRect.left,
					itemRect.top-clientRect.top, SWP_NOMOVE | SWP_NOZORDER);
			}

			if (IsSoundMsg(type))
				SetDlgItemText(hWnd, IDC_COMP_TEXT, "Sound Component");
		}
		break;
	}

	return plAnimCompProc::DlgProc(t, pm, hWnd, msg, wParam, lParam);
}

bool plResponderAnimProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
	if (cmd == CBN_SELCHANGE && resID == IDC_LOOP_COMBO)
	{
		HWND hCombo = GetDlgItem(hWnd, IDC_LOOP_COMBO);
		int sel = ComboBox_GetCurSel(hCombo);

		// If this is an actual loop (not the entire animation) get its name and save it
		if (sel != CB_ERR)
		{
			if (ComboBox_GetItemData(hCombo, sel) == 1)
			{
				char buf[256];
				ComboBox_GetText(hCombo, buf, sizeof(buf));
				pb->SetValue(kAnimLoop, 0, buf);
			}
			else
				pb->SetValue(kAnimLoop, 0, "");
		}

		return true;
	}

	return false;
}

void plResponderAnimProc::IPickComponent(IParamBlock2* pb)
{
	std::vector<Class_ID> cids;

	int type = pb->GetInt(kAnimType);
	if (type == kRespondPlaySound ||
		type == kRespondStopSound ||
		type == kRespondToggleSound ||
		type == kRespondRewindSound || 
		type == kRespondSyncedPlaySound)
	{
		cids.push_back(SOUND_3D_COMPONENT_ID);
		cids.push_back(BGND_MUSIC_COMPONENT_ID);
		cids.push_back(GUI_SOUND_COMPONENT_ID);
	}
	else if (type == kRespondPlayRndSound ||
			type == kRespondStopRndSound ||
			type == kRespondToggleRndSound)
	{
		cids.push_back(RANDOM_SOUND_COMPONENT_ID);
	}
	else
	{
		cids.push_back(ANIM_COMP_CID);
		cids.push_back(ANIM_GROUP_COMP_CID);
	}

	plPick::NodeRefKludge(pb, kAnimComp, &cids, true, false);
}

#include "plPickNodeBase.h"

static const char* kResponderNodeName = "(Responder Node)";

class plPickRespNode : public plPickCompNode
{
protected:
	int fTypeID;

	void IAddUserType(HWND hList)
	{
		int idx = ListBox_AddString(hList, kResponderNodeName);

		int type = fPB->GetInt(fTypeID);
		if (type == kNodeResponder)
			ListBox_SetCurSel(hList, idx);
	}

	void ISetUserType(plMaxNode* node, const char* userType)
	{
		if (userType && !strcmp(userType, kResponderNodeName))
		{
			ISetNodeValue(nil);
			fPB->SetValue(fTypeID, 0, kNodeResponder);
		}
		else
			fPB->SetValue(fTypeID, 0, kNodePB);
	}

public:
	plPickRespNode(IParamBlock2* pb, int nodeParamID, int typeID, plComponentBase* comp) :
	  plPickCompNode(pb, nodeParamID, comp), fTypeID(typeID)
	{
	}
};

void plResponderAnimProc::IPickNode(IParamBlock2* pb, plComponentBase* comp)
{
	plPickRespNode pick(pb, kAnimObject, kAnimObjectType, comp);
	pick.DoPick();
}

#include "plNotetrackAnim.h"

void plResponderAnimProc::ILoadUser(HWND hWnd, IParamBlock2 *pb)
{
	// Premptive strike.  If this isn't a loop, don't bother!
	int type = pb->GetInt(kAnimType);
	if (type != kRespondLoopAnimOn)
		return;

	HWND hLoop = GetDlgItem(hWnd, IDC_LOOP_COMBO);

	const char *savedName = pb->GetStr(kAnimLoop);
	if (!savedName)
		savedName = "";

	// Reset the combo and add the default selection
	ComboBox_ResetContent(hLoop);
	int sel = ComboBox_AddString(hLoop, ENTIRE_ANIMATION_NAME);
	ComboBox_SetCurSel(hLoop, sel);

	// FIXME
	plComponentBase *comp = plResponderCmdAnim::Instance().GetComponent(pb);
	if (comp && comp->ClassID() == ANIM_COMP_CID)
	{
		const char *animName = ((plAnimComponent*)comp)->GetAnimName();

		// Get the shared animations for all the nodes this component is applied to
		plNotetrackAnim anim(comp, nil);
		plAnimInfo info = anim.GetAnimInfo(animName);
		// Get all the loops in this animation
		while (const char *loopName = info.GetNextLoopName())
		{
			int idx = ComboBox_AddString(hLoop, loopName);
			ComboBox_SetItemData(hLoop, idx, 1);

			if (!strcmp(loopName, savedName))
				ComboBox_SetCurSel(hLoop, idx);
		}

		EnableWindow(hLoop, TRUE);
	}
	else
	{
		EnableWindow(hLoop, FALSE);
	}
}

void plResponderAnimProc::IUpdateNodeButton(HWND hWnd, IParamBlock2* pb)
{
	if (pb->GetInt(kAnimObjectType) == kNodeResponder)
	{
		HWND hButton = GetDlgItem(hWnd, IDC_OBJ_BUTTON);
		SetWindowText(hButton, kResponderNodeName);
	}
	else
		plAnimCompProc::IUpdateNodeButton(hWnd, pb);
}