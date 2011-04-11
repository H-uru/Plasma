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
#include "plMultistageBehComponent.h"
#include "plComponent.h"
#include "plComponentReg.h"

#include "plMultistageStage.h"

#include "hsStream.h"
#include "resource.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxMain/plMaxAccelerators.h"

#include "../plAvatar/plAnimStage.h"
#include "../plAvatar/plMultistageBehMod.h"
#include "hsResMgr.h"

#include <map>


void DummyCodeIncludeFuncMultistageBeh() {}

class plBaseStage;

class plMultistageBehComponent : public plComponent
{
protected:
	typedef std::multimap<plMaxNode*, plKey> ReceiverKeys;
	typedef std::pair<plMaxNode*, plKey> ReceiverKey;
	ReceiverKeys fReceivers;
	void IGetReceivers(plMaxNode* node, std::vector<plKey>& receivers);

	std::vector<plBaseStage*> fStages;
	bool fFreezePhys;
	bool fSmartSeek;
	bool fReverseFBOnRelease;

	// Dialog parameters, assume we'll only have one dialog open at a time
	static HWND fDlg;
	static int fCurStage;

	void IDeleteStages();

	static BOOL CALLBACK IStaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	void IInitDlg();
	void FixStageNames();

	void ICreateStageDlg();
	void IDestroyStageDlg();

	std::map<plMaxNode*, plMultistageBehMod*> fMods;

public:
	plMultistageBehComponent();
	~plMultistageBehComponent();

	plKey GetMultiStageBehKey(plMaxNode *node);

	hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual void AddReceiverKey(plKey pKey, plMaxNode* node=nil);

	virtual void CreateRollups();
	virtual void DestroyRollups();

	IOResult Save(ISave* isave);
	IOResult Load(ILoad* iload);

	RefTargetHandle Clone(RemapDir &remap);
};

HWND plMultistageBehComponent::fDlg = NULL;
int plMultistageBehComponent::fCurStage = -1;

//
// This is the access for other components to get the plKey of the MultiStageBeh modifier
//
plKey MultiStageBeh::GetMultiStageBehKey(plComponentBase *multiStageBehComp, plMaxNodeBase *target)
{
	if (multiStageBehComp->ClassID() == MULTISTAGE_BEH_CID)
	{
		plMultistageBehComponent *comp = (plMultistageBehComponent*)multiStageBehComp;
		return comp->GetMultiStageBehKey((plMaxNode*)target);
	}

	return nil;
}


CLASS_DESC(plMultistageBehComponent, gMultistageBehDesc, "Multistage Behavior", "MultiBeh", COMP_TYPE_AVATAR, MULTISTAGE_BEH_CID)

plMultistageBehComponent::plMultistageBehComponent()
: fFreezePhys(false),
  fSmartSeek(false),
  fReverseFBOnRelease(false)
{
	fClassDesc = &gMultistageBehDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plMultistageBehComponent::~plMultistageBehComponent()
{
	IDeleteStages();
}


hsBool plMultistageBehComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	node->SetForceLocal(true);
	fReceivers.clear();
	return true;
}

plKey plMultistageBehComponent::GetMultiStageBehKey(plMaxNode *node)
{
	if (fMods.find(node) != fMods.end())
		return fMods[node]->GetKey();

	return nil;
}

void plMultistageBehComponent::AddReceiverKey(plKey pKey, plMaxNode* node)
{
	fReceivers.insert(ReceiverKey(node, pKey));
}

void plMultistageBehComponent::IGetReceivers(plMaxNode* node, std::vector<plKey>& receivers)
{
	// Add the guys who want to be notified by all instances
	ReceiverKeys::iterator lowIt = fReceivers.lower_bound(nil);
	ReceiverKeys::iterator highIt = fReceivers.upper_bound(nil);
	for (; lowIt != highIt; lowIt++)
		receivers.push_back(lowIt->second);

	// Add the ones for just this instance
	lowIt = fReceivers.lower_bound(node);
	highIt = fReceivers.upper_bound(node);
	for (; lowIt != highIt; lowIt++)
		receivers.push_back(lowIt->second);
}

//
// PreConvert done below
//
hsBool plMultistageBehComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	//create the modifier here so that other components can find it
	plMultistageBehMod *mod = TRACKED_NEW plMultistageBehMod;
	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), mod, node->GetLocation());
	fMods[node] = mod;

	return true;
}

hsBool plMultistageBehComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	// Create the stage vector
	plAnimStageVec* animStages = TRACKED_NEW plAnimStageVec;
	int numStages = fStages.size();
	animStages->reserve(numStages);

	// Convert the stages and add them to the vector
	for (int i = 0; i < numStages; i++)
	{
		plBaseStage* stage = fStages[i];
		plAnimStage* animStage = stage->CreateStage();

		animStages->push_back(animStage);
	}

	// re-find the mod and attach it
	plMultistageBehMod* mod = fMods[node];
	std::vector<plKey> receivers;
	IGetReceivers(node, receivers);
	mod->Init(animStages, fFreezePhys, fSmartSeek, fReverseFBOnRelease, &receivers);
	node->AddModifier(mod, IGetUniqueName(node));

	return true;
}

void plMultistageBehComponent::IDeleteStages()
{
	int numStages = fStages.size();
	for (int i = 0; i < numStages; i++)
	{
		plBaseStage *stage = fStages[i];
		delete [] stage;
	}
	fStages.clear();
}

void plMultistageBehComponent::ICreateStageDlg()
{
	if (fCurStage == -1)
		return;

	hsAssert(fCurStage < fStages.size(), "Current stage out of range");
	fStages[fCurStage]->CreateDlg();
}

void plMultistageBehComponent::IDestroyStageDlg()
{
	if (fCurStage == -1)
		return;

	hsAssert(fCurStage < fStages.size(), "Current stage out of range");
	fStages[fCurStage]->DestroyDlg();

	fCurStage = -1;
}

void plMultistageBehComponent::CreateRollups()
{
	plComponent::CreateRollups();

	fDlg = GetCOREInterface()->AddRollupPage(hInstance,
											MAKEINTRESOURCE(IDD_COMP_MULTIBEH),
											IStaticDlgProc,
											"Multistage Behavior",
											(LPARAM)this);
	IInitDlg();

	ICreateStageDlg();
}

void plMultistageBehComponent::DestroyRollups()
{
	IDestroyStageDlg();

	if (fDlg)
	{
		GetCOREInterface()->DeleteRollupPage(fDlg);
		fDlg = NULL;
	}

	plComponent::DestroyRollups();
}

int ListView_AddString(HWND hList, const char* str)
{
	LVITEM item = {0};
	item.mask = LVIF_TEXT;
	item.pszText = const_cast<char*>(str); // F*** you Windows
	item.iItem = ListView_GetItemCount(hList);
	return ListView_InsertItem(hList, &item);
}

void plMultistageBehComponent::IInitDlg()
{
	// Add a column.  We don't use it (graphically), but it has to be there.
	HWND hList = GetDlgItem(fDlg, IDC_STAGE_LIST);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = "Blah";
	ListView_InsertColumn(hList, 0, &lvc);

	FixStageNames();
	for (int i = 0; i < fStages.size(); i++)
	{
		plBaseStage* stage = fStages[i];
		ListView_AddString(hList, stage->GetName());
	}

	// Make sure the column is wide enough
	ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);

	CheckDlgButton(fDlg, IDC_SMART_SEEK, fSmartSeek ? BST_CHECKED : BST_UNCHECKED); 
	CheckDlgButton(fDlg, IDC_FREEZE_PHYS, fFreezePhys ? BST_CHECKED : BST_UNCHECKED); 
	CheckDlgButton(fDlg, IDC_MULTI_REVERSE_CTL, fReverseFBOnRelease ? BST_CHECKED : BST_UNCHECKED); 
}

// stages used to be named starting with "Stage 1", but in the code
// they're referred to as "Stage 0..n"
// here we're going to look for old stage names and, if present,
// rename them all to start with zero instead.
void plMultistageBehComponent::FixStageNames()
{
	if(fStages.size() > 0)
	{
		plBaseStage* stage = fStages[0];
		const char * stageName = stage->GetName();

		if(strcmp(stageName, "Stage 1") == 0)
		{
			for (int i = 0; i < fStages.size(); i++)
			{
				plBaseStage* stage = fStages[i];
				char buf[64];
				sprintf(buf, "Stage %d", i);
				stage->SetName(buf);
			}
		}
	}
}


BOOL plMultistageBehComponent::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			// Adding a new stage
			if (LOWORD(wParam) == IDC_ADD)
			{
				// Create the new stage and give it a default name.
				plBaseStage* stage = TRACKED_NEW plStandardStage;
				int count = fStages.size();
				fStages.push_back(stage);
				char buf[64];
				sprintf(buf, "Stage %d", count);
				stage->SetName(buf);

				// Add the new stage to the list and make sure the list is wide enough
				HWND hList = GetDlgItem(fDlg, IDC_STAGE_LIST);
				int idx = ListView_AddString(hList, stage->GetName());
				ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
				ListView_SetItemState(hList, idx, LVIS_SELECTED, LVIS_SELECTED);

				// Put up the new stages dialog
				IDestroyStageDlg();
				fCurStage = idx;
				ICreateStageDlg();

				SetSaveRequiredFlag();
			}
			// Removing the selected stage
			else if (LOWORD(wParam) == IDC_REMOVE)
			{
				HWND hList = GetDlgItem(fDlg, IDC_STAGE_LIST);

				int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
				if (sel != -1)
				{
					IDestroyStageDlg();

					plBaseStage* stage = fStages[sel];
					fStages.erase(fStages.begin()+sel);
					delete stage;
					ListView_DeleteItem(hList, sel);

					SetSaveRequiredFlag();
				}
			}
			else if (LOWORD(wParam) == IDC_FREEZE_PHYS)
			{
				fFreezePhys = (Button_GetCheck((HWND)lParam) == BST_CHECKED);
				SetSaveRequiredFlag();
			}
			else if (LOWORD(wParam) == IDC_SMART_SEEK)
			{
				fSmartSeek = (Button_GetCheck((HWND)lParam) == BST_CHECKED);
				SetSaveRequiredFlag();
			}
			else if (LOWORD(wParam) == IDC_MULTI_REVERSE_CTL)
			{
				fReverseFBOnRelease = (Button_GetCheck((HWND)lParam) == BST_CHECKED);
				SetSaveRequiredFlag();
			}
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *nmhdr = (NMHDR*)lParam;
			if (nmhdr->idFrom == IDC_STAGE_LIST)
			{
				switch (nmhdr->code)
				{
				// Stop Max from reading keypresses while the list has focus
				case NM_SETFOCUS:
					plMaxAccelerators::Disable();
					return TRUE;
				case NM_KILLFOCUS:
					plMaxAccelerators::Enable();
					return TRUE;

				// The edit box this creates kills the focus on the listbox,
				// so add an extra disable to ignore it
				case LVN_BEGINLABELEDIT:
					plMaxAccelerators::Disable();
					return TRUE;

				// Finishing changing the name of a stage
				case LVN_ENDLABELEDIT:
					{
						NMLVDISPINFO *di = (NMLVDISPINFO*)lParam;
						const char *name = di->item.pszText;

						// If the name was changed...
						if (name && *name != '\0')
						{
							plBaseStage* stage = fStages[fCurStage];
							stage->SetName(name);

							// Make sure the column is wide enough
							int width = ListView_GetStringWidth(nmhdr->hwndFrom, name)+10;
							if (width > ListView_GetColumnWidth(nmhdr->hwndFrom, 0))
							{
								ListView_SetColumnWidth(nmhdr->hwndFrom, 0, width);
							}

							// Return true to keep the changes
							SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
						}
						
						plMaxAccelerators::Enable();
					}
					return TRUE;

				case LVN_ITEMCHANGED:
					{
						int sel = ListView_GetNextItem(nmhdr->hwndFrom, -1, LVNI_SELECTED);
						IDestroyStageDlg();
						if (sel != -1 && sel != fCurStage)
						{
							fCurStage = sel;
							ICreateStageDlg();
						}
					}
					return TRUE;
				}
			}
		}
		break;
	}

	return FALSE;
}

// A simple wrapper so the Max save/load stuff can be used with the hsStream interface
class MaxStream : public hsStream
{
protected:
	ISave* fSave;
	ILoad* fLoad;

public:
	MaxStream(ISave* isave) : fSave(isave), fLoad(nil) {}
	MaxStream(ILoad* iload) : fSave(nil), fLoad(iload) {}

	// Don't support any of this
	virtual hsBool Open(const char *, const char * = "rb") { hsAssert(0, "Not supported"); return false; }
	virtual hsBool Open(const wchar *, const wchar * = L"rb") { hsAssert(0, "Not supported"); return false; }
	virtual hsBool Close() {  hsAssert(0, "Not supported"); return false; }
	virtual void   Skip(UInt32 deltaByteCount) { hsAssert(0, "Not supported"); }
	virtual void   Rewind() { hsAssert(0, "Not supported"); }

	virtual UInt32	GetEOF() { return fLoad->CurChunkLength(); }

	virtual UInt32 Read(UInt32 byteCount, void * buffer)
	{
		UInt32 numRead = 0;
		hsAssert(fLoad, "No Max ILoad!");
		if (fLoad)
			fLoad->Read(buffer, byteCount, &numRead);
		fPosition += numRead;
		return numRead;
	}
	virtual UInt32 Write(UInt32 byteCount, const void* buffer)
	{
		UInt32 numWritten;
		hsAssert(fSave, "No Max ISave!");
		if (fSave)
			fSave->Write(buffer, byteCount, &numWritten);
		return numWritten;
	}
};


IOResult plMultistageBehComponent::Save(ISave* isave)
{
	isave->BeginChunk(kMultiStage);
	MaxStream multiChunk(isave);
	multiChunk.WriteSwap32(3);
	multiChunk.Writebool(fFreezePhys);
	multiChunk.Writebool(fSmartSeek);
	multiChunk.Writebool(fReverseFBOnRelease);
	isave->EndChunk();

	int numStages = fStages.size();
	for (int i = 0; i < numStages; i++)
	{
		plBaseStage *stage = fStages[i];
		if (stage)
		{
			isave->BeginChunk(stage->GetType());
			MaxStream stageChunk(isave);
			stage->Write(&stageChunk);
			isave->EndChunk();
		}
	}

	return IO_OK;
}

IOResult plMultistageBehComponent::Load(ILoad* iload)
{
	IDeleteStages();

	while (iload->OpenChunk() == IO_OK)
	{
		plBaseStage *stage = nil;

		switch (iload->CurChunkID())
		{
		case kMultiStage:
			{
				MaxStream multiChunk(iload);
				// all versions do this
				int version = multiChunk.ReadSwap32();
				fFreezePhys = multiChunk.Readbool();

				if(version > 1)
					// version 1 adds smart seek
					fSmartSeek = multiChunk.Readbool();
				else
					fSmartSeek = false;

				if(version > 2)
					fReverseFBOnRelease = multiChunk.Readbool();
				else
					fReverseFBOnRelease = false;
			}
			break;

		case kStandard:
			stage = TRACKED_NEW plStandardStage;
			break;
		}

		if (stage)
		{
			MaxStream stageChunk(iload);
			stage->Read(&stageChunk);
			fStages.push_back(stage);
		}

		iload->CloseChunk();
	}

	return IO_OK;
}

RefTargetHandle plMultistageBehComponent::Clone(RemapDir &remap)
{
	plMultistageBehComponent* clone = (plMultistageBehComponent*)plComponent::Clone(remap);

	clone->fFreezePhys = fFreezePhys;
	clone->fSmartSeek = fSmartSeek;
	clone->fReverseFBOnRelease = fReverseFBOnRelease;

	int numStages = fStages.size();
	clone->fStages.reserve(numStages);
	for (int i = 0; i < numStages; i++)
	{
		plBaseStage* cloneStage = fStages[i]->Clone();
		clone->fStages.push_back(cloneStage);
	}

	return clone;
}

BOOL plMultistageBehComponent::IStaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

	plMultistageBehComponent *multi = (plMultistageBehComponent*)GetWindowLong(hDlg, GWL_USERDATA);

	if (!multi)
		return FALSE;

	return multi->IDlgProc(hDlg, msg, wParam, lParam);
}
