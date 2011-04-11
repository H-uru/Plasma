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
#include "plAnimEventComponent.h"
#include "plComponentReg.h"
#include "resource.h"
#include "../MaxMain/plMaxNode.h"

#include "plAnimComponent.h"
#include "plNotetrackAnim.h"

#include "plAnimCompProc.h"
#include "plPickNode.h"

#include "../plModifier/plAnimEventModifier.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"

void DummyCodeIncludeFuncAnimDetector() {}

CLASS_DESC(plAnimEventComponent, gAnimEventDesc, "Anim Event", "AnimEvent", COMP_TYPE_DETECTOR, ANIMEVENT_CID)


class plAnimEventProc : public plAnimCompProc
{
protected:
	virtual void IPickComponent(IParamBlock2* pb);
	virtual void ILoadUser(HWND hWnd, IParamBlock2* pb);
	virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID);

public:
	plAnimEventProc();
};
static plAnimEventProc gAnimEventProc;


enum
{
	kAnimComp,
	kAnimNode,
	kAnimEvent_DEAD,
	kAnimName_DEAD,

	kAnimBegin,
	kAnimEnd,
	kAnimMarkers,
};

enum
{
	kAnimEventBegin,
	kAnimEventEnd,
	kAnimEventMarker,
};

ParamBlockDesc2 gAnimEventBlock
(
	plComponent::kBlkComp, _T("animEvent"), 0, &gAnimEventDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DETECTOR_ANIM, IDS_COMP_DETECTOR_ANIM, 0, 0, &gAnimEventProc,

	kAnimComp,	_T("animComp"),	TYPE_INODE,	0, 0,
		end,

	kAnimNode,	_T("animNode"),	TYPE_INODE,	0, 0,
		end,

	kAnimBegin,	_T("animBegin"),	TYPE_BOOL,	0, 0,
		end,
	kAnimEnd,	_T("animEnd"),		TYPE_BOOL,	0, 0,
		end,
	kAnimMarkers,	_T("animMarkers"),	TYPE_STRING_TAB, 0,		0, 0,
		end,

	end
);


plAnimEventComponent::plAnimEventComponent() : fCanExport(false)
{
	fClassDesc = &gAnimEventDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plAnimEventComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	plComponentBase* animComp;
	plMaxNode* animNode;
	if (!gAnimEventProc.GetCompAndNode(fCompPB, animComp, animNode))
	{
		pErrMsg->Set(true,
					"Anim Event Component",
					"Component %s does not have a valid anim component and node selected.\n"
					"It will not be exported.",
					GetINode()->GetName()).Show();
		pErrMsg->Set(false);
		fCanExport = false;
	}
	else
		fCanExport = true;

	return plActivatorBaseComponent::SetupProperties(node, pErrMsg);
}

hsBool plAnimEventComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if (!fCanExport)
		return false;

	plAnimEventModifier* mod = TRACKED_NEW plAnimEventModifier;
	plKey modKey = node->AddModifier(mod, IGetUniqueName(node));

	fLogicModKeys[node] = modKey;

	return true;
}

plEventCallbackMsg* CreateCallbackMsg(plAnimCmdMsg* animMsg, plKey modKey)
{
	plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
	eventMsg->AddReceiver(modKey);
	eventMsg->fRepeats = -1;

	animMsg->AddCallback(eventMsg);
	hsRefCnt_SafeUnRef(eventMsg);	// AddCallback adds it's own ref, so remove ours (the default of 1)

	return eventMsg;
}

hsBool plAnimEventComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!fCanExport)
		return false;

	plKey modKey = fLogicModKeys[node];
	plAnimEventModifier* mod = plAnimEventModifier::ConvertNoRef(modKey->GetObjectPtr());

	plAnimComponentBase* animComp;
	plMaxNode* animNode;
	gAnimEventProc.GetCompAndNode(fCompPB, (plComponentBase*&)animComp, animNode);

	//
	// Create and setup the callback message
	//
	plKey animKey = animComp->GetModKey(animNode);
	const char* animName = animComp->GetAnimName();

	plAnimCmdMsg *animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
	animMsg->SetSender(modKey);
	animMsg->SetAnimName(animName);
	animMsg->AddReceiver(animKey);

	if (fCompPB->GetInt(kAnimBegin))
	{
		plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
		eventMsg->fEvent = kBegin;
	}
	if (fCompPB->GetInt(kAnimEnd))
	{
		plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
		eventMsg->fEvent = kEnd;
	}
	if (fCompPB->Count(kAnimMarkers) > 0)
	{
		plNotetrackAnim anim(animComp, nil);
		plAnimInfo info = anim.GetAnimInfo(animName);

		int numMarkers = fCompPB->Count(kAnimMarkers);
		for (int i = 0; i < numMarkers; i++)
		{
			const char* marker = fCompPB->GetStr(kAnimMarkers, 0, i);
			float time = info.GetMarkerTime(marker);

			plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
			eventMsg->fEvent = kTime;
			eventMsg->fEventTime = time;
		}
	}

	mod->SetCallback(animMsg);
	
	hsTArray<plKey> receivers;
	IGetReceivers(node, receivers);
	mod->SetReceivers(receivers);

	return true;
}


////////////////////////////////////////////////////////////////////////////////


plAnimEventProc::plAnimEventProc()
{
	fCompButtonID	= IDC_ANIM_BUTTON;
	fCompParamID	= kAnimComp;
	fNodeButtonID	= IDC_NODE_BUTTON;
	fNodeParamID	= kAnimNode;
}

void plAnimEventProc::IPickComponent(IParamBlock2* pb)
{
	std::vector<Class_ID> cids;
	cids.push_back(ANIM_COMP_CID);
	cids.push_back(ANIM_GROUP_COMP_CID);

	plPick::Node(pb, kAnimComp, &cids, true, false);
}

static int ListBox_AddStringData(HWND hList, const char* text, int data)
{
	int idx = ListBox_AddString(hList, text);
	ListBox_SetItemData(hList, idx, data);
	return idx;
}

static bool IsMarkerSelected(IParamBlock2* pb, int paramID, const char* marker, bool remove=false)
{
	int numMarkers = pb->Count(paramID);
	for (int i = 0; i < numMarkers; i++)
	{
		if (hsStrEQ(marker, pb->GetStr(paramID, 0, i)))
		{
			if (remove)
				pb->Delete(paramID, i, 1);
			return true;
		}
	}

	return false;
}

//
// Remove markers we had saved that aren't in the object's notetrack any more
//
static void RemoveDeadMarkers(IParamBlock2* pb, int paramID, plAnimInfo& info)
{
	int numMarkers = pb->Count(paramID);
	for (int i = numMarkers-1; i >= 0; i--)
	{
		float time = info.GetMarkerTime(pb->GetStr(paramID, 0, i));
		if (time == -1)
		{
			pb->Delete(paramID, i, 1);
		}
	}
}

void plAnimEventProc::ILoadUser(HWND hWnd, IParamBlock2* pb)
{
	HWND hList = GetDlgItem(hWnd, IDC_EVENT_LIST);
	ListBox_ResetContent(hList);

	plAnimComponentBase* comp;
	plMaxNode* node;

	//
	// If we don't have a valid comp and node, we should be disabled
	//
	if (!GetCompAndNode(pb, (plComponentBase*&)comp, node))
	{
		EnableWindow(hList, FALSE);
		return;
	}
	else
		EnableWindow(hList, TRUE);

	//
	// Load the events
	//
	int idx;

	idx = ListBox_AddStringData(hList, "(Begin)", kAnimEventBegin);
	if (pb->GetInt(kAnimBegin))
		ListBox_SetSel(hList, TRUE, idx);

	idx = ListBox_AddStringData(hList, "(End)", kAnimEventEnd);
	if (pb->GetInt(kAnimEnd))
		ListBox_SetSel(hList, TRUE, idx);

	if (comp)
	{
		// Get the shared animations for all the nodes this component is applied to
		plNotetrackAnim anim(comp, nil);
		plAnimInfo info = anim.GetAnimInfo(comp->GetAnimName());

		RemoveDeadMarkers(pb, kAnimMarkers, info);

		// Get all the markers in this animation
		while (const char* marker = info.GetNextMarkerName())
		{
			idx = ListBox_AddStringData(hList, marker, kAnimEventMarker);

			if (IsMarkerSelected(pb, kAnimMarkers, marker))
				ListBox_SetSel(hList, TRUE, idx);
		}
	}
}

bool plAnimEventProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
	if (cmd == LBN_SELCHANGE && resID == IDC_EVENT_LIST)
	{
		HWND hList = GetDlgItem(hWnd, IDC_EVENT_LIST);
		int idx = ListBox_GetCurSel(hList);
		BOOL selected = ListBox_GetSel(hList, idx);
		int eventType = ListBox_GetItemData(hList, idx);

		if (eventType == kAnimEventBegin)
			pb->SetValue(kAnimBegin, 0, selected);
		else if (eventType == kAnimEventEnd)
			pb->SetValue(kAnimEnd, 0, selected);
		else if (eventType == kAnimEventMarker)
		{
			char buf[256];
			ListBox_GetText(hList, idx, buf);
			if (selected)
			{
				if (!IsMarkerSelected(pb, kAnimMarkers, buf))
				{
					TCHAR* name = buf;
					pb->Append(kAnimMarkers, 1, &name);
				}
			}
			else
				IsMarkerSelected(pb, kAnimMarkers, buf, true);
		}

		return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


CLASS_DESC(plMtlEventComponent, gMtlEventDesc, "Mtl Event", "MtlEvent", COMP_TYPE_DETECTOR, MTLEVENT_CID)

class plMtlEventProc : public plMtlAnimProc
{
public:
	plMtlEventProc();

protected:
	virtual void ILoadUser(HWND hWnd, IParamBlock2* pb);
	virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID);
};
static plMtlEventProc gMtlEventProc;

enum
{
	kMtlMtl,
	kMtlNode,
	kMtlAnim,

	kMtlBegin,
	kMtlEnd,
	kMtlMarkers,
};

ParamBlockDesc2 gMtlEventBlock
(
	plComponent::kBlkComp, _T("mtlEvent"), 0, &gMtlEventDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DETECTOR_MTL, IDS_COMP_DETECTOR_MTL, 0, 0, &gMtlEventProc,

	kMtlMtl,	_T("mtl"),			TYPE_MTL,			0, 0,
		end,

	kMtlNode,	_T("node"),			TYPE_INODE,			0, 0,
		end,

	kMtlAnim,	_T("anim"),			TYPE_STRING,		0, 0,
		end,

	kMtlBegin,	_T("animBegin"),	TYPE_BOOL,			0, 0,
		end,
	kMtlEnd,	_T("animEnd"),		TYPE_BOOL,			0, 0,
		end,
	kMtlMarkers,_T("markers"),		TYPE_STRING_TAB, 0,	0, 0,
		end,

	end
);


plMtlEventComponent::plMtlEventComponent() : fCanExport(false)
{
	fClassDesc = &gMtlEventDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plMtlEventComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	Mtl* mtl = fCompPB->GetMtl(kMtlMtl);

	if (!mtl)
	{
		pErrMsg->Set(true,
					"Mtl Event Component",
					"Component %s does not have a valid material selected.\n"
					"It will not be exported.",
					GetINode()->GetName()).Show();
		pErrMsg->Set(false);
		fCanExport = false;
	}
	else
		fCanExport = true;

	return plActivatorBaseComponent::SetupProperties(node, pErrMsg);
}

hsBool plMtlEventComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if (!fCanExport)
		return false;

	plAnimEventModifier* mod = TRACKED_NEW plAnimEventModifier;
	plKey modKey = node->AddModifier(mod, IGetUniqueName(node));

	fLogicModKeys[node] = modKey;

	return true;
}

// KLUDGE - The material animation key getter is here, so we have to include all this crap
#include "plResponderMtl.h"

hsBool plMtlEventComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!fCanExport)
		return false;

	plKey modKey = fLogicModKeys[node];
	plAnimEventModifier* mod = plAnimEventModifier::ConvertNoRef(modKey->GetObjectPtr());

	Mtl* mtl = fCompPB->GetMtl(kMtlMtl);
	plMaxNodeBase* mtlNode = (plMaxNodeBase*)fCompPB->GetINode(kMtlNode);
	const char* mtlAnim = fCompPB->GetStr(kMtlAnim);

	//
	// Create and setup the callback message
	//
	hsTArray<plKey> animKeys;
	GetMatAnimModKey(mtl, mtlNode, mtlAnim, animKeys);

	plAnimCmdMsg *animMsg = TRACKED_NEW plAnimCmdMsg;
	animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
	animMsg->SetSender(modKey);
	animMsg->SetAnimName(mtlAnim);
	animMsg->AddReceivers(animKeys);

	if (fCompPB->GetInt(kMtlBegin))
	{
		plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
		eventMsg->fEvent = kBegin;
	}
	if (fCompPB->GetInt(kMtlEnd))
	{
		plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
		eventMsg->fEvent = kEnd;
	}
	if (fCompPB->Count(kMtlMarkers) > 0)
	{
		plNotetrackAnim anim(mtl, nil);
		plAnimInfo info = anim.GetAnimInfo(mtlAnim);

		int numMarkers = fCompPB->Count(kMtlMarkers);
		for (int i = 0; i < numMarkers; i++)
		{
			const char* marker = fCompPB->GetStr(kMtlMarkers, 0, i);
			float time = info.GetMarkerTime(marker);

			plEventCallbackMsg *eventMsg = CreateCallbackMsg(animMsg, modKey);
			eventMsg->fEvent = kTime;
			eventMsg->fEventTime = time;
		}
	}

	mod->SetCallback(animMsg);

	hsTArray<plKey> receivers;
	IGetReceivers(node, receivers);
	mod->SetReceivers(receivers);

	return true;
}


////////////////////////////////////////////////////////////////////////////////


plMtlEventProc::plMtlEventProc()
{
	fMtlButtonID		= IDC_MTL_BUTTON;
	fMtlParamID			= kMtlMtl;
	fNodeButtonID		= IDC_NODE_BUTTON;
	fNodeParamID		= kMtlNode;
	fAnimComboID		= IDC_ANIM_COMBO;
	fAnimParamID		= kMtlAnim;
}

void plMtlEventProc::ILoadUser(HWND hWnd, IParamBlock2* pb)
{
	HWND hList = GetDlgItem(hWnd, IDC_EVENT_LIST);
	ListBox_ResetContent(hList);

	//
	// If we don't have a valid material, we should be disabled
	//
	Mtl* mtl = pb->GetMtl(kMtlMtl);
	if (!mtl)
	{
		EnableWindow(hList, FALSE);
		return;
	}
	else
		EnableWindow(hList, TRUE);

	//
	// Load the events
	//
	int idx;

	idx = ListBox_AddStringData(hList, "(Begin)", kAnimEventBegin);
	if (pb->GetInt(kMtlBegin))
		ListBox_SetSel(hList, TRUE, idx);

	idx = ListBox_AddStringData(hList, "(End)", kAnimEventEnd);
	if (pb->GetInt(kMtlEnd))
		ListBox_SetSel(hList, TRUE, idx);

	if (mtl)
	{
		const char* mtlAnim = pb->GetStr(kMtlAnim);

		// Get the shared animations for all the nodes this component is applied to
		plNotetrackAnim anim(mtl, nil);
		plAnimInfo info = anim.GetAnimInfo(mtlAnim);

		RemoveDeadMarkers(pb, kMtlMarkers, info);

		// Get all the markers in this animation
		while (const char* marker = info.GetNextMarkerName())
		{
			idx = ListBox_AddStringData(hList, marker, kAnimEventMarker);

			if (IsMarkerSelected(pb, kMtlMarkers, marker))
				ListBox_SetSel(hList, TRUE, idx);
		}
	}
}

bool plMtlEventProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
	if (cmd == LBN_SELCHANGE && resID == IDC_EVENT_LIST)
	{
		HWND hList = GetDlgItem(hWnd, IDC_EVENT_LIST);
		int idx = ListBox_GetCurSel(hList);
		BOOL selected = ListBox_GetSel(hList, idx);
		int eventType = ListBox_GetItemData(hList, idx);

		if (eventType == kAnimEventBegin)
			pb->SetValue(kMtlBegin, 0, selected);
		else if (eventType == kAnimEventEnd)
			pb->SetValue(kMtlEnd, 0, selected);
		else if (eventType == kAnimEventMarker)
		{
			char buf[256];
			ListBox_GetText(hList, idx, buf);
			if (selected)
			{
				if (!IsMarkerSelected(pb, kMtlMarkers, buf))
				{
					TCHAR* name = buf;
					pb->Append(kMtlMarkers, 1, &name);
				}
			}
			else
				IsMarkerSelected(pb, kMtlMarkers, buf, true);
		}

		return true;
	}

	return false;
}
