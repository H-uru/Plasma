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
#include "plResponderMtl.h"
#include "plResponderComponentPriv.h"
#include "resource.h"
#include "max.h"

#include "../MaxMain/plMaxNode.h"

#include "../MaxPlasmaMtls/Materials/plDecalMtl.h"
#include "../MaxPlasmaMtls/Materials/plPassMtl.h"

#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerAnimation.h"

#include "plMaxAnimUtils.h"
#include "plNotetrackAnim.h"

#include "plPickMaterialMap.h"
#include "../MaxMain/plMtlCollector.h"
#include "plPickNode.h"

// Needed for convert
#include "../plMessage/plAnimCmdMsg.h"

#include <set>
#include <vector>
#include <algorithm>

#include "../MaxMain/plPlasmaRefMsgs.h"

enum
{
	kMtlRef,
	kMtlAnim,
	kMtlLoop,
	kMtlType,
	kMtlOwner_DEAD,
	kMtlNode,
	kMtlNodeType,
};

enum MtlNodeType
{
	kNodePB,		// Use the node in the PB
	kNodeResponder	// Use the node the responder is attached to
};

class plResponderMtlProc;
extern plResponderMtlProc gResponderMtlProc;

ParamBlockDesc2 gResponderMtlBlock
(
	kResponderMtlBlk, _T("mtlCmd"), 0, NULL, P_AUTO_UI,

	IDD_COMP_RESPOND_MTL, IDS_COMP_CMD_PARAMS, 0, 0, &gResponderMtlProc,

	kMtlRef,	_T("mtl"),		TYPE_REFTARG,		0, 0,
		end,

	kMtlAnim,	_T("anim"),		TYPE_STRING,		0, 0,
		end,

	kMtlLoop,	_T("loop"),		TYPE_STRING,		0, 0,
		end,

	kMtlType,	_T("type"),		TYPE_INT,			0, 0,
		end,

	kMtlNode,	_T("node"),		TYPE_INODE,			0, 0,
		end,

	kMtlNodeType,	_T("nodeType"),	TYPE_INT,	0, 0,
		end,

	end
);

plResponderCmdMtl& plResponderCmdMtl::Instance()
{
	static plResponderCmdMtl theInstance;
	return theInstance;
}

ParamBlockDesc2 *plResponderCmdMtl::GetDesc()
{
	return &gResponderMtlBlock;
}

// Use old types for backward compatibility
enum
{
	kRespondPlayMat=12,
	kRespondStopMat,
	kRespondToggleMat,
	kRespondLoopMatOn,
	kRespondLoopMatOff,
	kRespondSetForeMat,
	kRespondSetBackMat,
	kRespondRewindMat,

	kNumTypes=8
};

static int IndexToOldType(int idx)
{
	static int oldTypes[] =
	{
		kRespondPlayMat,
		kRespondStopMat,
		kRespondToggleMat,
		kRespondLoopMatOn,
		kRespondLoopMatOff,
		kRespondSetForeMat,
		kRespondSetBackMat,
		kRespondRewindMat
	};

	hsAssert(idx < kNumTypes, "Bad index");
	return oldTypes[idx];
}

int plResponderCmdMtl::NumTypes()
{
	return kNumTypes;
}

const char *plResponderCmdMtl::GetCategory(int idx)
{
	return "Material";
}

const char *plResponderCmdMtl::GetName(int idx)
{
	int type = IndexToOldType(idx);

	switch (type)
	{
	case kRespondPlayMat:	return "Play";
	case kRespondStopMat:	return "Stop";
	case kRespondToggleMat:	return "Toggle";
	case kRespondLoopMatOn:	return "Set Looping On";
	case kRespondLoopMatOff:return "Set Looping Off";
	case kRespondSetForeMat:return "Set Forwards";
	case kRespondSetBackMat:return "Set Backwards";
	case kRespondRewindMat:	return "Rewind";
	}

	return nil;
}

static const char *GetShortName(int type)
{
	switch (type)
	{
	case kRespondPlayMat:	return "Mat Play";
	case kRespondStopMat:	return "Mat Stop";
	case kRespondToggleMat:	return "Mat Toggle";
	case kRespondLoopMatOn:	return "Mat Loop On";
	case kRespondLoopMatOff:return "Mat Loop Off";
	case kRespondSetForeMat:return "Mat Set Fore";
	case kRespondSetBackMat:return "Mat Set Back";
	case kRespondRewindMat:	return "Mat Rewind";
	}

	return nil;
}
const char *plResponderCmdMtl::GetInstanceName(IParamBlock2 *pb)
{
	static char name[256];

	const char *shortName = GetShortName(pb->GetInt(kMtlType));

	Mtl *mtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
	sprintf(name, "%s (%s)", shortName, mtl ? mtl->GetName() : "none");

	return name;
}

IParamBlock2 *plResponderCmdMtl::CreatePB(int idx)
{
	int type = IndexToOldType(idx);

	// Create the paramblock and save it's type
	IParamBlock2 *pb = CreateParameterBlock2(&gResponderMtlBlock, nil);
	pb->SetValue(kMtlType, 0, type);

	return pb;
}

Mtl *plResponderCmdMtl::GetMtl(IParamBlock2 *pb)
{
	return (Mtl*)pb->GetReferenceTarget(kMtlRef);
}

const char *plResponderCmdMtl::GetAnim(IParamBlock2 *pb)
{
	return pb->GetStr(kMtlAnim);
}

void ISearchLayerRecur(plLayerInterface *layer, const char *segName, hsTArray<plKey>& keys)
{
	if (!layer)
		return;

	plLayerAnimation *animLayer = plLayerAnimation::ConvertNoRef(layer);
	if (animLayer)
	{
		char *ID = animLayer->GetSegmentID();
		if (ID == nil)
			ID = "";
		if (!strcmp(ID, segName))
		{
			if( keys.kMissingIndex == keys.Find(animLayer->GetKey()) )
				keys.Append(animLayer->GetKey());
		}
	}

	ISearchLayerRecur(layer->GetAttached(), segName, keys);
}

int ISearchLayerRecur(hsGMaterial* mat, const char *segName, hsTArray<plKey>& keys)
{
	if (segName == nil)
		segName = "";
	int i;
	for( i = 0; i < mat->GetNumLayers(); i++ )
		ISearchLayerRecur(mat->GetLayer(i), segName, keys);
	return keys.GetCount();
}

int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char* segName, hsTArray<plKey>& keys)
{
	int retVal = 0;

	int i;

	//if( begin < 0 )
	//	begin = 0;

	if( mtl->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
	{
		for( i = 0; i < mtl->NumSubMtls(); i++ )
			retVal += GetMatAnimModKey(mtl->GetSubMtl(i), node, segName, keys);
	}
	else
	{
		hsTArray<hsGMaterial*> matList;

		if (node)
			hsMaterialConverter::Instance().GetMaterialArray(mtl, (plMaxNode*)node, matList);
		else
			hsMaterialConverter::Instance().CollectConvertedMaterials(mtl, matList);

		for( i = 0; i < matList.GetCount(); i++ )
		{
			retVal += ISearchLayerRecur(matList[i], segName, keys);
		}
	}

	return retVal;
}

void plResponderCmdMtl::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2* pb)
{
	plMaxNode* mtlNode;
	if (pb->GetInt(kMtlNodeType) == kNodeResponder)
		mtlNode = node;
	else
		mtlNode = (plMaxNode*)pb->GetINode(kMtlNode);

	if (mtlNode)
		mtlNode->SetForceMaterialCopy(true);
}

plMessage *plResponderCmdMtl::CreateMsg(plMaxNode* node, plErrorMsg *pErrMsg, IParamBlock2 *pb)
{
	Mtl *maxMtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
	if (!maxMtl)
		throw "No material specified";

	const char *animName = pb->GetStr(kMtlAnim);
	hsScalar begin=-1.f;
	hsScalar end = -1.f;

	SegmentMap *segMap = GetAnimSegmentMap(maxMtl, pErrMsg);

	hsTArray<plKey> keys;

	if( segMap )
	{
		GetSegMapAnimTime(animName, segMap, SegmentSpec::kAnim, begin, end);
	}

	plMaxNode* mtlNode;
	if (pb->GetInt(kMtlNodeType) == kNodeResponder)
		mtlNode = node;
	else
		mtlNode = (plMaxNode*)pb->GetINode(kMtlNode);

	GetMatAnimModKey(maxMtl, mtlNode, animName, keys);

	const char *loopName = nil;
	loopName = pb->GetStr(kMtlLoop);
	if (segMap && loopName)
		GetSegMapAnimTime(loopName, segMap, SegmentSpec::kLoop, begin, end);

	DeleteSegmentMap(segMap);

	if (!keys.GetCount())
	{
		// We need the check here because "physicals only" export mode means that
		// most of the materials won't be there, so we should ignore this warning. -Colin
		if (plConvert::Instance().GetConvertSettings()->fPhysicalsOnly)
			return nil;
		else
			throw "Material animation key(s) not found";
	}

	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg;
	msg->AddReceivers(keys);

	switch (pb->GetInt(kMtlType))
	{
	case kRespondPlayMat:
		msg->SetCmd(plAnimCmdMsg::kContinue);
		break;
	case kRespondStopMat:
		msg->SetCmd(plAnimCmdMsg::kStop);
		break;
	case kRespondToggleMat:
		msg->SetCmd(plAnimCmdMsg::kToggleState);
		break;

	case kRespondLoopMatOn:
		msg->SetCmd(plAnimCmdMsg::kSetLooping);

		// KLUDGE - We send the loop to play by name here, so anim grouped components
		// could have loops with different begin and end points.  However, apparently
		// that functionality was never implemented, whoops.  So, we'll take out the
		// stuff that actually tries to set the begin and end points for now, so that
		// anims with a loop set in advance will actually work with this. -Colin
		//
		// This KLUDGE has been copied from where Colin kludged it in plResponderAnim 
		// in the spirit of consistent hackage. Maybe when one gets unkludged, the
		// other one will too. -mf
//		msg->SetCmd(plAnimCmdMsg::kSetLoopBegin);
//		msg->fLoopBegin = begin;

//		msg->SetCmd(plAnimCmdMsg::kSetLoopEnd);
//		msg->fLoopEnd = end;
		break;

	case kRespondLoopMatOff:
		msg->SetCmd(plAnimCmdMsg::kUnSetLooping);
		break;

	case kRespondSetForeMat:
		msg->SetCmd(plAnimCmdMsg::kSetForewards);
		break;

	case kRespondSetBackMat:
		msg->SetCmd(plAnimCmdMsg::kSetBackwards);
		break;

	case kRespondRewindMat:
		msg->SetCmd(plAnimCmdMsg::kGoToBegin);
		break;

	default:
		delete msg;
		throw "Unknown material command";
	}

	return msg;
}

bool plResponderCmdMtl::IsWaitable(IParamBlock2 *pb)
{
	int type = pb->GetInt(kMtlType);
	if (type == kRespondPlayMat ||
		type == kRespondToggleMat)
		return true;

	return false;
}

void plResponderCmdMtl::GetWaitPoints(IParamBlock2 *pb, WaitPoints& waitPoints)
{
	Mtl *mtl = GetMtl(pb);
	const char *animName = GetAnim(pb);

	if (mtl)
	{
		plNotetrackAnim notetrackAnim(mtl, nil);
		plAnimInfo info = notetrackAnim.GetAnimInfo(animName);
		while (const char *marker = info.GetNextMarkerName())
			waitPoints.push_back(marker);
	}
}

void plResponderCmdMtl::CreateWait(plMaxNode* node, plErrorMsg* pErrMsg, IParamBlock2 *pb, ResponderWaitInfo& waitInfo)
{
	plAnimCmdMsg *animMsg = plAnimCmdMsg::ConvertNoRef(waitInfo.msg);
	if (animMsg)
		animMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);

	plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
	eventMsg->AddReceiver(waitInfo.receiver);
	eventMsg->fRepeats = 0;
	eventMsg->fEvent = kStop;
	eventMsg->fUser = waitInfo.callbackUser;

	if (waitInfo.point)
	{
		// FIXME COLIN - Error checking here?
		Mtl *mtl = GetMtl(pb);
		const char *animName = GetAnim(pb);

		plNotetrackAnim notetrackAnim(mtl, nil);
		plAnimInfo info = notetrackAnim.GetAnimInfo(animName);

		eventMsg->fEvent = kTime;
		eventMsg->fEventTime = info.GetMarkerTime(waitInfo.point);
	}
	else
	{
		eventMsg->fEvent = kStop;
	}

	plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(waitInfo.msg);
	callbackMsg->AddCallback(eventMsg);
	hsRefCnt_SafeUnRef( eventMsg );
}

////////////////////////////////////////////////////////////////////////////////

#include "plAnimCompProc.h"

class plResponderMtlProc : public plMtlAnimProc
{
public:
	plResponderMtlProc();

protected:
	virtual void IOnInitDlg(HWND hWnd, IParamBlock2* pb);
	virtual void ILoadUser(HWND hWnd, IParamBlock2* pb);
	virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID);

	virtual void IPickNode(IParamBlock2* pb);

	virtual void ISetNodeButtonText(HWND hWnd, IParamBlock2* pb);
};
static plResponderMtlProc gResponderMtlProc;

plResponderMtlProc::plResponderMtlProc()
{
	fMtlButtonID		= IDC_MTL_BUTTON;
	fMtlParamID			= kMtlRef;
	fNodeButtonID		= IDC_NODE_BUTTON;
	fNodeParamID		= kMtlNode;
	fAnimComboID		= IDC_ANIM_COMBO;
	fAnimParamID		= kMtlAnim;
}

void plResponderMtlProc::IOnInitDlg(HWND hWnd, IParamBlock2* pb)
{
	int type = pb->GetInt(kMtlType);

	// Show the loop control only if this is a loop
	int show = (type == kRespondLoopMatOn) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem(hWnd, IDC_LOOP_COMBO), show);
	ShowWindow(GetDlgItem(hWnd, IDC_LOOP_TEXT), show);

	// Resize the dialog if we're not using the loop control
	if (type != kRespondLoopMatOn)
	{
		RECT itemRect, clientRect;
		GetWindowRect(GetDlgItem(hWnd, IDC_LOOP_TEXT), &itemRect);
		GetWindowRect(hWnd, &clientRect);
		SetWindowPos(hWnd, NULL, 0, 0, clientRect.right-clientRect.left,
			itemRect.top-clientRect.top, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void plResponderMtlProc::ILoadUser(HWND hWnd, IParamBlock2 *pb)
{
	HWND hLoop = GetDlgItem(hWnd, IDC_LOOP_COMBO);

	const char *savedName = pb->GetStr(kMtlLoop);
	if (!savedName)
		savedName = "";

	ComboBox_ResetContent(hLoop);
	int sel = ComboBox_AddString(hLoop, ENTIRE_ANIMATION_NAME);
	ComboBox_SetCurSel(hLoop, sel);
	
	// Get the NoteTrack animations off the selected material
	Mtl *mtl = (Mtl*)pb->GetReferenceTarget(kMtlRef);
	if (!mtl)
	{
		ComboBox_Enable(hLoop, FALSE);
		return;
	}

	ComboBox_Enable(hLoop, TRUE);

	plNotetrackAnim anim(mtl, nil);
	const char *animName = pb->GetStr(kMtlAnim);
	plAnimInfo info = anim.GetAnimInfo(animName);

	while (const char *loopName = info.GetNextLoopName())
	{
		sel = ComboBox_AddString(hLoop, loopName);
		if (!strcmp(loopName, savedName))
			ComboBox_SetCurSel(hLoop, sel);
	}
}

bool plResponderMtlProc::IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)
{
	if (cmd == CBN_SELCHANGE && resID == IDC_LOOP_COMBO)
	{
		HWND hCombo = GetDlgItem(hWnd, IDC_LOOP_COMBO);
		int idx = ComboBox_GetCurSel(hCombo);

		if (idx != CB_ERR)
		{
			if (ComboBox_GetItemData(hCombo, idx) == 0)
				pb->SetValue(kMtlLoop, 0, "");
			else
			{
				// Get the name of the animation and save it
				char buf[256];
				ComboBox_GetText(hCombo, buf, sizeof(buf));
				pb->SetValue(kMtlLoop, 0, buf);
			}
		}

		return true;
	}

	return false;
}


#include "plPickNodeBase.h"

static const char* kUserTypeAll = "(All)";
static const char* kResponderNodeName = "(Responder Node)";

class plPickRespMtlNode : public plPickMtlNode
{
protected:
	int fTypeID;

	void IAddUserType(HWND hList)
	{
		int type = fPB->GetInt(fTypeID);

		int idx = ListBox_AddString(hList, kUserTypeAll);
		if (type == kNodePB && !fPB->GetINode(fNodeParamID))
			ListBox_SetCurSel(hList, idx);


		idx = ListBox_AddString(hList, kResponderNodeName);
		if (type == kNodeResponder)
			ListBox_SetCurSel(hList, idx);
	}

	void ISetUserType(plMaxNode* node, const char* userType)
	{
		if (hsStrEQ(userType, kUserTypeAll))
		{
			ISetNodeValue(nil);
			fPB->SetValue(fTypeID, 0, kNodePB);
		}
		else if (hsStrEQ(userType, kResponderNodeName))
		{
			ISetNodeValue(nil);
			fPB->SetValue(fTypeID, 0, kNodeResponder);
		}
		else
			fPB->SetValue(fTypeID, 0, kNodePB);
	}

public:
	plPickRespMtlNode(IParamBlock2* pb, int nodeParamID, int typeID, Mtl* mtl) :
	  plPickMtlNode(pb, nodeParamID, mtl), fTypeID(typeID)
	{
	}
};

void plResponderMtlProc::IPickNode(IParamBlock2* pb)
{
	plPickRespMtlNode pick(pb, kMtlNode, kMtlNodeType, IGetMtl(pb));
	pick.DoPick();
}

void plResponderMtlProc::ISetNodeButtonText(HWND hWnd, IParamBlock2* pb)
{
	int type = pb->GetInt(kMtlNodeType);
	HWND hButton = GetDlgItem(hWnd, IDC_NODE_BUTTON);

	if (type == kNodeResponder)
		SetWindowText(hButton, kResponderNodeName);
	else if (type == kNodePB && !pb->GetINode(kMtlNode))
		SetWindowText(hButton, kUserTypeAll);
	else
		plMtlAnimProc::ISetNodeButtonText(hWnd, pb);
}
