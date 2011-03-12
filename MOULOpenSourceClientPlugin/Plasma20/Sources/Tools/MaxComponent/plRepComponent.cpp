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

#include "max.h"
#include "dummy.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "plMiscComponents.h"

#include "../MaxMain/plPlasmaRefMsgs.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxExport/plExportErrorMsg.h"

#include "hsResMgr.h"

#include "plLoadMask.h"

#include "plPickNode.h"


void DummyCodeIncludeFuncRepComp()
{
}

const Class_ID REPCOMP_CID(0x157c29f3, 0x18fa54cd);
const Class_ID REPGROUP_CID(0x20ce74a2, 0x471b2386);

static const int kNumQualities = 4;
static const char* kQualityStrings[kNumQualities] = {
	"Low",
	"Medium",
	"High",
	"Ultra"
};

class plRepresentComp : public plComponent
{
public:
	enum {
		kQuality
	};
public:
	plRepresentComp();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

	int GetQuality();
	int GetCapability();

	void SetLoadMask(const plLoadMask& m);

	static plRepresentComp* GetComp(INode* node);
};

#define WM_ROLLOUT_OPEN WM_USER+1

class plRepresentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
				PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

				HWND cbox = GetDlgItem(hWnd, IDC_COMP_REPRESENT_QUALITY);
				int i;
				for( i = 0; i < kNumQualities; i++ )
				{
					SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kQualityStrings[i]);
				}
				SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(plRepresentComp::kQuality), 0);

			}
			return true;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
			case IDC_COMP_REPRESENT_QUALITY:
				map->GetParamBlock()->SetValue(plRepresentComp::kQuality, t, SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plRepresentProc gRepresentProc;


CLASS_DESC(plRepresentComp, gRepresentDesc, "Representation",  "Rep", COMP_TYPE_GRAPHICS, REPCOMP_CID)

ParamBlockDesc2 gRepresentBk
(	
	plComponent::kBlkComp, _T("Represent"), 0, &gRepresentDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_REPRESENT, IDS_COMP_REPRESENT, 0, 0, &gRepresentProc,

	plRepresentComp::kQuality,	_T("Quality"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	end
);

plRepresentComp::plRepresentComp()
{
	fClassDesc = &gRepresentDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plRepresentComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepresentComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepresentComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepresentComp::DeInit(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

int plRepresentComp::GetQuality()
{
	return fCompPB->GetInt(kQuality);
}

int plRepresentComp::GetCapability()
{
	int maxCap = 0;
	int numTarg = NumTargets();
	int iTarg;
	for( iTarg = 0; iTarg < numTarg; iTarg++ )
	{
		plMaxNodeBase* node = GetTarget(iTarg);
		if( node )
		{
			const char* name = node->GetName();
			int numComp = node->NumAttachedComponents();
			int iComp;
			for( iComp = 0; iComp < numComp; iComp++ )
			{
				plComponentBase* comp = node->GetAttachedComponent(iComp);
				if( comp )
				{
					const char* compName = comp->GetINode()->GetName();
					int cap = comp->GetMinCap();
					if( cap > maxCap )
						maxCap = cap;
				}
			}
		}
	}
	return maxCap;
}

void plRepresentComp::SetLoadMask(const plLoadMask& m)
{
	int numTarg = NumTargets();
	int iTarg;
	for( iTarg = 0; iTarg < numTarg; iTarg++ )
	{
		plMaxNodeBase* node = GetTarget(iTarg);
		if( node )
		{
			const char* nodeName = node->GetName();
			node->AddLoadMask(m);
			plLoadMask x = node->GetLoadMask();
			x |= m;
		}
	}
}

plRepresentComp* plRepresentComp::GetComp(INode* node)
{
	if( node == nil )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( comp == nil )
		return nil;

	if( comp->ClassID() == REPCOMP_CID )
		return (plRepresentComp*) comp;

	return nil;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class plRepGroupComp : public plComponent
{
public:
	enum {
		kReps
	};

	void	IGetQC(int quals[], int caps[]);

	hsBool ComputeAndValidate(plErrorMsg* pErrMsg, int quals[], int caps[], plLoadMask masks[]);

	void CleanDeadNodes();
public:
	plRepGroupComp();
	void DeleteThis() { delete this; }
	
	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool Validate(plErrorMsg* pErrMsg);
};

void plRepGroupComp::CleanDeadNodes()
{
	int i = fCompPB->Count(kReps) - 1;
	while( i >= 0 )
	{
		if( !fCompPB->GetINode(kReps, TimeValue(0), i) )
			fCompPB->Delete(kReps, i, 1);
		i--;
	}
}

class plRepGroupProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			{
			}
			return true;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				switch( LOWORD(wParam) )
				{
				case IDC_ADD_REPS:
					{
						std::vector<Class_ID> cids;
						cids.push_back(REPCOMP_CID);
						IParamBlock2 *pb = map->GetParamBlock();
						plPick::Node(pb, plRepGroupComp::kReps, &cids, false, false);

						map->Invalidate(plRepGroupComp::kReps);
					}
					return TRUE;
				case IDC_UP_REPS:
					{
						HWND hNode = GetDlgItem(hWnd, IDC_LIST_REPS);
						int idx = ListBox_GetCurSel(hNode);
						if( (idx != LB_ERR) && (idx > 0) )
						{
							IParamBlock2 *pb = map->GetParamBlock();
							INode* node = pb->GetINode(plRepGroupComp::kReps, TimeValue(0), idx);
							pb->Delete(plRepGroupComp::kReps, idx, 1);
							if( node )
								pb->Insert(plRepGroupComp::kReps, idx-1, 1, &node);
							ListBox_SetCurSel(hNode, idx-1);

							map->Invalidate(plRepGroupComp::kReps);
						}
					}
					return TRUE;
				case IDC_DOWN_REPS:
					{
						HWND hNode = GetDlgItem(hWnd, IDC_LIST_REPS);
						IParamBlock2 *pb = map->GetParamBlock();
						int idx = ListBox_GetCurSel(hNode);
						if( (idx != LB_ERR) && (idx < pb->Count(plRepGroupComp::kReps)-1) )
						{
							INode* node = pb->GetINode(plRepGroupComp::kReps, TimeValue(0), idx);
							pb->Delete(plRepGroupComp::kReps, idx, 1);
							if( node )
								pb->Insert(plRepGroupComp::kReps, idx+1, 1, &node);
							ListBox_SetCurSel(hNode, idx+1);

							map->Invalidate(plRepGroupComp::kReps);
						}
					}
					return TRUE;
				case IDC_VAL_REPS:
					{
						plRepGroupComp* repGroup = (plRepGroupComp*)map->GetParamBlock()->GetOwner();
						plExportErrorMsg errMsg;
						repGroup->Validate(&errMsg);
					}
					return TRUE;
				}
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plRepGroupProc gRepGroupProc;

CLASS_DESC(plRepGroupComp, gRepGroupDesc, "Representation Group",  "RepGroup", COMP_TYPE_GRAPHICS, REPGROUP_CID)

ParamBlockDesc2 gRepGroupBk
(	
	plComponent::kBlkComp, _T("RepGroup"), 0, &gRepGroupDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_REPGROUP, IDS_COMP_REPGROUP, 0, 0, &gRepGroupProc,

	plRepGroupComp::kReps,	_T("Reps"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_REPS, 0, 0, IDC_DEL_REPS,
		end,

	end
);

plRepGroupComp::plRepGroupComp()
{
	fClassDesc = &gRepGroupDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

void plRepGroupComp::IGetQC(int quals[], int caps[])
{
	const int numReps = fCompPB->Count(kReps);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		plRepresentComp* rep = plRepresentComp::GetComp(fCompPB->GetINode(kReps, TimeValue(0), i));
		quals[i] = rep->GetQuality();
		caps[i] = rep->GetCapability();
	}
}
hsBool plRepGroupComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	const int numReps = fCompPB->Count(kReps);
	hsTArray<int> quals(numReps);
	hsTArray<int> caps(numReps);
	hsTArray<plLoadMask> masks(numReps);

	IGetQC(quals.AcquireArray(), caps.AcquireArray());

	ComputeAndValidate(pErrMsg, quals.AcquireArray(), caps.AcquireArray(), masks.AcquireArray());

	int i;
	for( i = 0; i < numReps; i++ )
	{
		plRepresentComp* rep = plRepresentComp::GetComp(fCompPB->GetINode(kReps, TimeValue(0), i));
		rep->SetLoadMask(masks[i]);
	}

	return true;
}

hsBool plRepGroupComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepGroupComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepGroupComp::DeInit(plMaxNode* node, plErrorMsg* pErrMsg)
{
	return true;
}

hsBool plRepGroupComp::ComputeAndValidate(plErrorMsg* pErrMsg, int quals[], int caps[], plLoadMask masks[])
{
	const int numReps = fCompPB->Count(kReps);
	UInt32 preVal = plLoadMask::ValidateReps(numReps, quals, caps);

	if( preVal )
	{
		int i;
		for( i = 0; i < 32; i++ )
		{
			if( preVal & (1 << i) )
			{
				char buff[256];
				INode* rep = fCompPB->GetINode(kReps, TimeValue(0), i);
				sprintf(buff, "Rep %d - %s is obscured by an earlier representation in preVal", i, rep ? rep->GetName() : "Unknown");
				pErrMsg->Set(true, GetINode()->GetName(), buff).Show();
				pErrMsg->Set(false);
			}
		}
	}

	hsBool val = plLoadMask::ComputeRepMasks(numReps, quals, caps, masks);

	UInt32 postVal = plLoadMask::ValidateMasks(numReps, masks);

	if( postVal )
	{
		int i;
		for( i = 0; i < 32; i++ )
		{
			if( !(preVal & (1 << i)) && (postVal & (1 << i)) )
			{
				char buff[256];
				INode* rep = fCompPB->GetINode(kReps, TimeValue(0), i);
				sprintf(buff, "Rep %d - %s is obscured by an earlier representation in postVal", i, rep ? rep->GetName() : "Unknown");
				pErrMsg->Set(true, GetINode()->GetName(), buff).Show();
				pErrMsg->Set(false);
			}
		}
	}

	return !(preVal || val || postVal);
}

hsBool plRepGroupComp::Validate(plErrorMsg* pErrMsg)
{
	CleanDeadNodes();

	const int numReps = fCompPB->Count(kReps);
	hsTArray<int> quals(numReps);
	hsTArray<int> caps(numReps);
	hsTArray<plLoadMask> masks(numReps);

	IGetQC(quals.AcquireArray(), caps.AcquireArray());

	return ComputeAndValidate(pErrMsg, quals.AcquireArray(), caps.AcquireArray(), masks.AcquireArray());
}