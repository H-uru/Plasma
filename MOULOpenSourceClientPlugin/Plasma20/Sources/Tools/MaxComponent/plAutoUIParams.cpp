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
#include "plAutoUIParams.h"

#include "../MaxMain/plMaxAccelerators.h"
#include "hsUtils.h"
#include <algorithm>
#include "../MaxMain/plMaxNode.h"
#include "plComponentBase.h"

plAutoUIParam::plAutoUIParam(ParamID id, const char *name) :
	fID(id), fName(hsStrcpy(name)), fVisID(-1), fHeight(0)
{
}

plAutoUIParam::~plAutoUIParam()
{
	delete [] fName;
}

int plAutoUIParam::Create(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	int size = fControlVec.size();
	if (size > 0)
		fControlVec.clear();

	int initialOffset = yOffset;
	int finalOffset = CreateControls(hDlg, pb, yOffset);
	
	fHeight = finalOffset - initialOffset;
	fhDlg = hDlg;

	return finalOffset;
}

int plAutoUIParam::ISizeControl(HWND hDlg, HWND hControl, int w, int h, int y, int x)
{
	// Convert the dialog units to screen units
	RECT rect;
	SetRect(&rect, x, 0, w, h);
	MapDialogRect(hDlg, &rect);
	// Y is already in screen units
	rect.top = y;

	// Resize the window
	MoveWindow(hControl, rect.left, rect.top, rect.right, rect.bottom, FALSE);

	return rect.bottom;
}

HWND plAutoUIParam::ICreateControl(HWND hDlg, const char *className, const char *wndName, DWORD style, DWORD exStyle)
{
	HWND hwnd = CreateWindowEx(exStyle, className, wndName, WS_VISIBLE | WS_CHILD | style,
				0, 0, 0, 0, hDlg, 0/*(HMENU)fDlgItemID*/, hInstance, NULL);

	fControlVec.push_back(hwnd);

	return hwnd;
}

// By default the controls use a font that is too big.  This fixes that.
void plAutoUIParam::ISetControlFont(HWND hControl)
{
	SendMessage(hControl, WM_SETFONT, (WPARAM)GetCOREInterface()->GetAppHFont(), TRUE);
}

int plAutoUIParam::IAddStaticText(HWND hDlg, int y, const char *text)
{
	HWND hStatic = ICreateControl(hDlg, "Static", text, SS_LEFT);
	int height = ISizeControl(hDlg, hStatic, 100, 8, y) + 2;
	ISetControlFont(hStatic);

	return height;
}

int plAutoUIParam::GetParamType()
{
	return kTypeNone;
}

void plAutoUIParam::Show(int yOffset)
{
	HWND tmp;

	int ctrlOffset = 0;
	RECT rect;

	for (int i = 0; i < fControlVec.size(); i++)
	{
		tmp = fControlVec[i];
		GetWindowRect(tmp, &rect);
		
		SetRect(&rect, 3, 0, (rect.right - rect.left) + 3, rect.bottom - rect.top);
		//MapDialogRect(fhDlg, &rect);
		// Y is already in screen units
		rect.top = yOffset + ctrlOffset;
		rect.bottom = rect.bottom + rect.top;

		// Resize the window
		MoveWindow(tmp, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		//MoveWindow(tmp, rect.left, yOffset + ctrlOffset, (rect.right - rect.left), (rect.bottom - rect.top), FALSE);
		ctrlOffset += (rect.bottom - rect.top) + 2;

		ShowWindow(tmp, SW_SHOW);
	}
}

void plAutoUIParam::Hide()
{
	HWND tmp;

	for (int i = 0; i < fControlVec.size(); i++)
	{
		tmp = fControlVec[i];
		ShowWindow(tmp, SW_HIDE);
	}
}

int plAutoUIParam::GetHeight()
{
	return fHeight;
}

void plAutoUIParam::SetVisInfo(ParamID id, std::vector<std::string>* states)
{
	fVisID = id;
	fVisStates = *states;
}

bool plAutoUIParam::CheckVisibility(ParamID id, std::string state)
{
	if (fVisStates.size() == 0)
	{
		return true;
	}
	else
	{
		if (fVisID == id)
		{
			if (std::find(fVisStates.begin(), fVisStates.end(), state) != fVisStates.end())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

hsBool plAutoUIParam::GetBool(IParamBlock2 *pb)
{
	hsAssert(false, "Parameter is not a bool");
	return false;
}
float plAutoUIParam::GetFloat(IParamBlock2 *pb)
{
	hsAssert(false, "Parameter is not a float");
	return -1;
}
int plAutoUIParam::GetInt(IParamBlock2 *pb)
{
	hsAssert(false, "Parameter is not an int");
	return -1;
}
const char* plAutoUIParam::GetString(IParamBlock2 *pb)
{
	hsAssert(false, "Parameter is not a string");
	return nil;
}
int plAutoUIParam::GetCount(IParamBlock2 *pb)
{
	hsAssert(false, "Parameter is not a key list");
	return -1;
}
plKey plAutoUIParam::GetKey(IParamBlock2 *pb, int idx)
{
	hsAssert(false, "Parameter is not a key");
	return nil;
}
plComponentBase *plAutoUIParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(false, "Parameter is not a component");
	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plCheckBoxParam::plCheckBoxParam(ParamID id, const char *name) :
  plAutoUIParam(id, name), fhCheck(nil)
{
}

int plCheckBoxParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	// Create the checkbox
	fhCheck = ICreateControl(hDlg, "Button", fName, BS_AUTOCHECKBOX);
	yOffset += ISizeControl(hDlg, fhCheck, 90, 10, yOffset) + 2;

	// Set the check from the current value
	SendMessage(fhCheck, BM_SETCHECK, pb->GetInt(fID), 0);

	return yOffset;
}

bool plCheckBoxParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhCheck)
		{
			// Get the state
			hsBool checked = (SendMessage(fhCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
			// Set the state in the paramblock
			pb->SetValue(fID, 0, checked);
			return true;
		}
	}

	return false;
}

int plCheckBoxParam::GetParamType()
{
	return kTypeBool;
}
hsBool plCheckBoxParam::GetBool(IParamBlock2 *pb)
{
	return pb->GetInt(fID);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plSpinnerParam::plSpinnerParam(ParamID id, const char *name, bool isFloat) :
	plAutoUIParam(id, name), fIsFloat(isFloat), fhSpinner(nil)
{
}

int plSpinnerParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName);

	// Create the edit box
	HWND hEdit = ICreateControl(hDlg, "CustEdit");
	ISizeControl(hDlg, hEdit, 30, 10, yOffset);

	// Create the spinner (beside the edit box)
	fhSpinner = ICreateControl(hDlg, "SpinnerControl");
	yOffset += ISizeControl(hDlg, fhSpinner, 8, 10, yOffset, 33) + 2;

	// Initialize the spinner
	ISpinnerControl *spin = GetISpinner(fhSpinner);
	ParamDef &def = pb->GetParamDef(fID);
	if (fIsFloat)
	{
		spin->SetLimits(def.range_low.f, def.range_high.f, FALSE);
		spin->SetValue(pb->GetFloat(fID), FALSE);
		spin->SetScale(0.1f);
		spin->LinkToEdit(hEdit, EDITTYPE_FLOAT);
	}
	else
	{
		spin->SetLimits(def.range_low.i, def.range_high.i, FALSE);
		spin->SetValue(pb->GetInt(fID), FALSE);
		spin->SetScale(1);
		spin->LinkToEdit(hEdit, EDITTYPE_INT);
	}

	ReleaseISpinner(spin);

	return yOffset;
}

void plSpinnerParam::Show(int yOffset)
{
	yOffset += ISizeControl(fhDlg, fControlVec[0], 100, 8, yOffset) + 2;	
	ISizeControl(fhDlg, fControlVec[1], 30, 10, yOffset);
	ISizeControl(fhDlg, fControlVec[2], 8, 10, yOffset, 33);

	ShowWindow(fControlVec[0], SW_SHOW);
	ShowWindow(fControlVec[1], SW_SHOW);
	ShowWindow(fControlVec[2], SW_SHOW);
}

bool plSpinnerParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == CC_SPINNER_CHANGE)// && HIWORD(wParam) == FALSE)
	{
		ISpinnerControl *spin = (ISpinnerControl*)lParam;
		HWND hSpinner = spin->GetHwnd();
		if (hSpinner == fhSpinner)
		{
			// Set the state
			if (fIsFloat)
				pb->SetValue(fID, 0, spin->GetFVal());
			else
				pb->SetValue(fID, 0, spin->GetIVal());
			return true;
		}
	}

	return false;
}

int plSpinnerParam::GetParamType()
{
	return fIsFloat ? kTypeFloat : kTypeInt;
}
float plSpinnerParam::GetFloat(IParamBlock2 *pb)
{
	return (fIsFloat ? pb->GetFloat(fID) : -1);
}
int plSpinnerParam::GetInt(IParamBlock2 *pb)
{
	return (!fIsFloat ? pb->GetInt(fID) : -1);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plEditParam::plEditParam(ParamID id, const char *name, int lines) :
	plAutoUIParam(id, name), fhEdit(nil), fLines(lines)
{
}

int plEditParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName);

	//
	// Create the edit box
	//
	DWORD flags = ES_AUTOHSCROLL | ES_LEFT | WS_BORDER;
	// If this edit box has more than one line, add the multiline flags
	if (fLines > 1)
		flags |= ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN;

	fhEdit = ICreateControl(hDlg, "Edit", nil, flags, WS_EX_CLIENTEDGE);
	yOffset += ISizeControl(hDlg, fhEdit, 100, 5 + 8*fLines, yOffset) + 2;
	ISetControlFont(fhEdit);

	// Initialize the edit box
	SetWindowText(fhEdit, pb->GetStr(fID));

	return yOffset;
}

void plEditParam::Show(int yOffset)
{
	yOffset += ISizeControl(fhDlg, fControlVec[0], 100, 8, yOffset) + 2;
	ISizeControl(fhDlg, fControlVec[1], 100, 5 + 8*fLines, yOffset);

	ShowWindow(fControlVec[0], SW_SHOW);
	ShowWindow(fControlVec[1], SW_SHOW);
}

bool plEditParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND)
	{
		// Disable Max accelerators when the edit box gets focus
		if (HIWORD(wParam) == EN_SETFOCUS && (HWND)lParam == fhEdit)
		{
			plMaxAccelerators::Disable();
			return true;
		}
		else if (HIWORD(wParam) == EN_KILLFOCUS && (HWND)lParam == fhEdit)
		{
			// Get the text from the edit and store it in the paramblock
			int len = GetWindowTextLength(fhEdit)+1;
			if (len > 1)
			{
				char *buf = TRACKED_NEW char[len];
				GetWindowText(fhEdit, buf, len);
				pb->SetValue(fID, 0, buf);
				delete [] buf;
			}
			else
				pb->SetValue(fID, 0, "");

			plMaxAccelerators::Enable();

			return true;
		}
	}

	return false;
}

int plEditParam::GetParamType()
{
	return kTypeString;
}
const char* plEditParam::GetString(IParamBlock2 *pb)
{
	return pb->GetStr(fID);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#include "plPickNode.h"

plPickListParam::plPickListParam(ParamID id, const char *name, std::vector<Class_ID>* filter) :
	plAutoUIParam(id, name), fhList(nil), fhAdd(nil), fhRemove(nil)
{
	if (filter)
		fCIDs = *filter;
}

int plPickListParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the listbox
	fhList = ICreateControl(hDlg, "ListBox", nil, LBS_STANDARD | LBS_NOINTEGRALHEIGHT, WS_EX_CLIENTEDGE);
	yOffset += ISizeControl(hDlg, fhList, 100, 5+4*8, yOffset) + 2;
	ISetControlFont(fhList);

	// Create the Add button
	fhAdd = ICreateControl(hDlg, "Button");
	ISizeControl(hDlg, fhAdd, 48, 14, yOffset, 4);
	SetWindowText(fhAdd, "Add...");

	// Create the Remove button
	fhRemove = ICreateControl(hDlg, "Button");
	yOffset += ISizeControl(hDlg, fhRemove, 48, 14, yOffset, 54);
	SetWindowText(fhRemove, "Remove");

	return yOffset;
}

void plPickListParam::Show(int yOffset)
{
	yOffset += ISizeControl(fhDlg, fControlVec[0], 100, 8, yOffset) + 4;	
	yOffset += ISizeControl(fhDlg, fControlVec[1], 100, 5+4*8, yOffset) + 2;
	ISizeControl(fhDlg, fControlVec[2], 48, 14, yOffset, 4);
	ISizeControl(fhDlg, fControlVec[3], 48, 14, yOffset, 54);

	ShowWindow(fControlVec[0], SW_SHOW);
	ShowWindow(fControlVec[1], SW_SHOW);
	ShowWindow(fControlVec[2], SW_SHOW);
	ShowWindow(fControlVec[3], SW_SHOW);
}

bool plPickListParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_INITDIALOG)
	{
		IUpdateList(pb);
		return false;
	}

	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhAdd)
		{
			plPick::Node(pb, fID, &fCIDs, false, false);
			IUpdateList(pb);
			return true;
		}
		if ((HWND)lParam == fhRemove)
		{
			int sel = SendMessage(fhList, LB_GETCURSEL, 0, 0);
			if (sel != -1)
			{
				pb->Delete(fID, sel, 1);
				IUpdateList(pb);
			}
			return true;
		}
	}

	return false;
}

void plPickListParam::IUpdateList(IParamBlock2 *pb)
{
	SendMessage(fhList, LB_RESETCONTENT, 0, 0);

	int count = pb->Count(fID);
	for (int i = 0; i < count; i++)
	{
		INode *node = pb->GetINode(fID, 0, i);
		const char *name = node ? node->GetName() : "<deleted>";
		SendMessage(fhList, LB_ADDSTRING, 0, (LPARAM)name);
	}
}

int plPickListParam::GetParamType()
{
	return kTypeSceneObj;
}
int plPickListParam::GetCount(IParamBlock2 *pb)
{
	return pb->Count(fID);
}
plKey plPickListParam::GetKey(IParamBlock2 *pb, int idx)
{
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID, 0, idx);
	if (node)
		return node->GetKey();

	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class plPickButtonParam;

class PickNodeButtonFilter : public PickNodeCallback
{
public:
	virtual BOOL Filter(INode *node) { return TRUE; }
};
static PickNodeButtonFilter gPickFilter;

class PickNodeButtonMode : public PickModeCallback
{
public:
	plPickButtonParam *fParam;
	IParamBlock2 *fPB;

	BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags)
	{
		return (ip->PickNode(hWnd,m,&gPickFilter) != NULL);
	}
	BOOL Pick(IObjParam *ip, ViewExp *vpt);
	
	PickNodeCallback *GetFilter() { return &gPickFilter; }
	BOOL	RightClick(IObjParam *ip, ViewExp *vpt);
};

static PickNodeButtonMode gPickMode;

plPickButtonParam::plPickButtonParam(ParamID id, const char *name, std::vector<Class_ID>* filter, bool canConvertToType) :
	  plAutoUIParam(id, name), fButton(nil), fCanConvertToType(canConvertToType)
{
	if (filter)
		fCIDs = *filter;
}

int plPickButtonParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the picknode button
	HWND button = ICreateControl(hDlg, "CustButton");
	ISizeControl(hDlg, button, 84, 12, yOffset);

	fButton = GetICustButton(button);

	// Setup the button properties
	fButton->SetType(CBT_CHECK);
	fButton->SetButtonDownNotify(TRUE);
	fButton->SetCheckHighlight(TRUE);
	fButton->SetHighlightColor(GREEN_WASH);

	INode *node = (INode*)pb->GetReferenceTarget(fID);
	if (node)
		fButton->SetText(node->GetName());
	else
		fButton->SetText("(none)");

	// Create the Remove button
	fhRemove = ICreateControl(hDlg, "Button");
	yOffset += ISizeControl(hDlg, fhRemove, 17, 12, yOffset, 89);
	SetWindowText(fhRemove, "Clear");

	return yOffset;
}

void plPickButtonParam::Show(int yOffset)
{
	yOffset += ISizeControl(fhDlg, fControlVec[0], 100, 8, yOffset) + 4;	
	ISizeControl(fhDlg, fControlVec[1], 84, 12, yOffset);
	ISizeControl(fhDlg, fControlVec[2], 17, 12, yOffset, 89);


	ShowWindow(fControlVec[0], SW_SHOW);
	ShowWindow(fControlVec[1], SW_SHOW);
	ShowWindow(fControlVec[2], SW_SHOW);
}

bool plPickButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fCIDs.size() > 0)
			{
				if (fButton->IsChecked())
				{
					if (plPick::Node(pb, fID, &fCIDs, true, fCanConvertToType))
					{
						INode *node = (INode*)pb->GetReferenceTarget(fID);
						if (node)
							fButton->SetText(node->GetName());
					}
					fButton->SetCheck(FALSE);
				}
			}
			else
			{
				if (fButton->IsChecked())
				{
					gPickMode.fParam = this;
					gPickMode.fPB = pb;
					GetCOREInterface()->SetPickMode(&gPickMode);
				}
				else
				{
					GetCOREInterface()->ClearPickMode();
				}
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

void plPickButtonParam::SetPickNode(INode *node, IParamBlock2 *pb)
{
	if (node && pb)
	{
		pb->SetValue(fID, 0, (ReferenceTarget*)node);
		fButton->SetText(node->GetName());
	}

	fButton->SetCheck(FALSE);
}


BOOL PickNodeButtonMode::Pick(IObjParam *ip, ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	if (node && fParam && fPB)
		fParam->SetPickNode(node, fPB);
	fParam = nil;
	fPB = nil;

	return TRUE;
}

BOOL PickNodeButtonMode::RightClick(IObjParam *ip, ViewExp *vpt)
{
	if (fParam && fPB)
		fParam->SetPickNode(nil, nil);
	fParam = nil;
	fPB = nil;

	return TRUE;
}

int plPickButtonParam::GetParamType()
{
	return kTypeSceneObj;
}
int plPickButtonParam::GetCount(IParamBlock2 *pb)
{
	return (pb->GetReferenceTarget(fID) ? 1 : 0);
}
plKey plPickButtonParam::GetKey(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->GetKey();

	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickComponentButtonParam::plPickComponentButtonParam(ParamID id, const char *name, std::vector<Class_ID>* filter, bool canConvertToType) :
	  plPickButtonParam(id, name, filter, canConvertToType)
{
	hsAssert(filter, "Need to have a ClassID filter for pick component buttons");
}

int plPickComponentButtonParam::GetParamType()
{
	return kTypeComponent;
}

plComponentBase* plPickComponentButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickComponentListParam::plPickComponentListParam(ParamID id, const char *name, std::vector<Class_ID>* filter) :
	plPickListParam(id, name, filter)
{
}

bool plPickComponentListParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	return plPickListParam::IsMyMessage(msg, wParam, lParam, pb);
}

int plPickComponentListParam::GetParamType()
{
	return kTypeComponent;
}

plComponentBase *plPickComponentListParam::GetComponent(IParamBlock2 *pb, int idx)
{
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID, 0, idx);
	if (node)
		return node->ConvertToComponent();

	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickActivatorButtonParam::plPickActivatorButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickActivatorButtonParam::GetParamType()
{
	return kTypeActivator;
}
plComponentBase* plPickActivatorButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}

bool plPickActivatorButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::Activator(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}


	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickActivatorListParam::plPickActivatorListParam(ParamID id, const char *name) :
	plPickListParam(id, name, nil)
{
}

bool plPickActivatorListParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhAdd)
		{
			plPick::Activator(pb, fID, false);
			IUpdateList(pb);
			return true;
		}
	}

	return plPickListParam::IsMyMessage(msg, wParam, lParam, pb);
}

int plPickActivatorListParam::GetParamType()
{
	return kTypeActivator;
}

plComponentBase *plPickActivatorListParam::GetComponent(IParamBlock2 *pb, int idx)
{
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID, 0, idx);
	if (node)
		return node->ConvertToComponent();

	return nil;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#include "plPickMaterialMap.h"
#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#include "../plSurface/plLayerInterface.h"
#include "../plGImage/plDynamicTextMap.h"

plPickDynamicTextButtonParam::plPickDynamicTextButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickDynamicTextButtonParam::GetParamType()
{
	return kTypeDynamicText;
}

// temp hack to get the name of the texture map name
const char* plPickDynamicTextButtonParam::GetString(IParamBlock2 *pb)
{
	// get the plKeys based on the texture map that the DynamicText map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	return texmap->GetName();
}

int plPickDynamicTextButtonParam::GetCount(IParamBlock2 *pb)
{

	// get the plKeys based on the texture map that the DynamicText map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	// make sure that there was a texmap set
	if ( texmap )
	{
		plPlasmaMAXLayer *maxLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( texmap );
		if( maxLayer != nil )
		{
			// It's one of our Plasma layer types, which means most likely it got converted.
 
			// Note: if the following count is zero, it was never converted, which most likely means 
			// no visible geometry in the scene uses this layer. So why do you even care about it?
			return maxLayer->GetNumConversionTargets();
		}
	}
	return 0;
}

plKey plPickDynamicTextButtonParam::GetKey(IParamBlock2 *pb, int idx)
{

	// get the plKeys based on the texture map that the DynamicText map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	// make sure that there was a texmap set
	if ( texmap )
	{
		plPlasmaMAXLayer *maxLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( texmap );
		if( maxLayer != nil )
		{
			// make sure the index is valid
			if ( idx >= 0 && idx < maxLayer->GetNumConversionTargets() )
			{
				plLayerInterface *convertedLayer = maxLayer->GetConversionTarget(idx);
				if ( convertedLayer )
				{
					plBitmap* bmap = convertedLayer->GetTexture();
					// make sure there was a bitmap and that it is a DynamicTextMap
					if ( bmap )
						return bmap->GetKey();
				}
			}
		}
	}

	// otherwise we didn't find one, because of one of many reasons
	return nil;
}

int plPickDynamicTextButtonParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the picknode button
	HWND button = ICreateControl(hDlg, "CustButton");
	ISizeControl(hDlg, button, 84, 12, yOffset);

	fButton = GetICustButton(button);

	// Setup the button properties
	fButton->SetType(CBT_CHECK);
	fButton->SetButtonDownNotify(TRUE);
	fButton->SetCheckHighlight(TRUE);
	fButton->SetHighlightColor(GREEN_WASH);

	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	if (texmap)
		fButton->SetText(texmap->GetName());
	else
		fButton->SetText("(none)");

	// Create the Remove button
	fhRemove = ICreateControl(hDlg, "Button");
	yOffset += ISizeControl(hDlg, fhRemove, 17, 12, yOffset, 89);
	SetWindowText(fhRemove, "Clear");

	return yOffset;
}

bool plPickDynamicTextButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if ( plPickMaterialMap::PickTexmap(pb, fID) )
				{
					Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
					if (texmap)
						fButton->SetText(texmap->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


plPickSingleComponentButtonParam::plPickSingleComponentButtonParam(ParamID id, const char *name, int myType, Class_ID myClassToPick ) :
	plPickButtonParam(id, name, nil, false)
{
		fClassToPick = myClassToPick;
		fMyType = myType;
}

int plPickSingleComponentButtonParam::GetParamType()
{
	return fMyType;
}

plComponentBase* plPickSingleComponentButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickSingleComponentButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::GenericClass(pb, fID, true,fClassToPick))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


plPickExcludeRegionButtonParam::plPickExcludeRegionButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickExcludeRegionButtonParam::GetParamType()
{
	return kTypeExcludeRegion;
}

plComponentBase* plPickExcludeRegionButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickExcludeRegionButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::ExcludeRegion(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


plPickWaterComponentButtonParam::plPickWaterComponentButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickWaterComponentButtonParam::GetParamType()
{
	return kTypeWaterComponent;
}

plComponentBase* plPickWaterComponentButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickWaterComponentButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::WaterComponent(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickSwimCurrentInterfaceButtonParam::plPickSwimCurrentInterfaceButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickSwimCurrentInterfaceButtonParam::GetParamType()
{
	return kTypeSwimCurrentInterface;
}

plComponentBase* plPickSwimCurrentInterfaceButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickSwimCurrentInterfaceButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::Swim2DComponent(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickClusterComponentButtonParam::plPickClusterComponentButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickClusterComponentButtonParam::GetParamType()
{
	return kTypeClusterComponent;
}

plComponentBase* plPickClusterComponentButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}

bool plPickClusterComponentButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::ClusterComponent(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}
			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickAnimationButtonParam::plPickAnimationButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickAnimationButtonParam::GetParamType()
{
	return kTypeAnimation;
}

plComponentBase* plPickAnimationButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickAnimationButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::Animation(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


plPickBehaviorButtonParam::plPickBehaviorButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickBehaviorButtonParam::GetParamType()
{
	return kTypeBehavior;
}

plComponentBase* plPickBehaviorButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickBehaviorButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::Behavior(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickMaterialButtonParam::plPickMaterialButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickMaterialButtonParam::GetParamType()
{
	return kTypeMaterial;
}

// temp hack to get the name of the texture map name
const char* plPickMaterialButtonParam::GetString(IParamBlock2 *pb)
{
	// get the plKeys based on the texture map that the DynamicText map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	return texmap->GetName();
}

int plPickMaterialButtonParam::GetCount(IParamBlock2 *pb)
{

	// get the plKeys based on the texture map that the Texture map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	// make sure that there was a texmap set
	if ( texmap )
	{
		plPlasmaMAXLayer *maxLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( texmap );
		if( maxLayer != nil )
		{
			// It's one of our Plasma layer types, which means most likely it got converted.
 
			// Note: if the following count is zero, it was never converted, which most likely means 
			// no visible geometry in the scene uses this layer. So why do you even care about it?
			return maxLayer->GetNumConversionTargets();
		}
	}
	return 0;
}

plKey plPickMaterialButtonParam::GetKey(IParamBlock2 *pb, int idx)
{

	// get the plKeys based on the texture map that the Texture map is on
	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	// make sure that there was a texmap set
	if ( texmap )
	{
		plPlasmaMAXLayer *maxLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( texmap );
		if( maxLayer != nil )
		{
			// make sure the index is valid
			if ( idx >= 0 && idx < maxLayer->GetNumConversionTargets() )
			{
				plLayerInterface *convertedLayer = maxLayer->GetConversionTarget(idx);
				if ( convertedLayer )
				{
					plBitmap* bmap = convertedLayer->GetTexture();
					// make sure there was a bitmap
					if ( bmap )
						return bmap->GetKey();
				}
			}
		}
	}

	// otherwise we didn't find one, because of one of many reasons
	return nil;
}

int plPickMaterialButtonParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the picknode button
	HWND button = ICreateControl(hDlg, "CustButton");
	ISizeControl(hDlg, button, 84, 12, yOffset);

	fButton = GetICustButton(button);

	// Setup the button properties
	fButton->SetType(CBT_CHECK);
	fButton->SetButtonDownNotify(TRUE);
	fButton->SetCheckHighlight(TRUE);
	fButton->SetHighlightColor(GREEN_WASH);

	Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
	if (texmap)
		fButton->SetText(texmap->GetName());
	else
		fButton->SetText("(none)");

	// Create the Remove button
	fhRemove = ICreateControl(hDlg, "Button");
	yOffset += ISizeControl(hDlg, fhRemove, 17, 12, yOffset, 89);
	SetWindowText(fhRemove, "Clear");

	return yOffset;
}

bool plPickMaterialButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if ( plPickMaterialMap::PickTexmap(pb, fID) )
				{
					Texmap* texmap = (Texmap*)pb->GetReferenceTarget(fID);
					if (texmap)
						fButton->SetText(texmap->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickMaterialAnimationButtonParam::plPickMaterialAnimationButtonParam(ParamID id, const char *name) :
	plPickButtonParam(id, name, nil, false)
{
}

int plPickMaterialAnimationButtonParam::GetParamType()
{
	return kTypeMaterialAnimation;
}

const char* plPickMaterialAnimationButtonParam::GetString(IParamBlock2 *pb)
{
	Mtl* texmap = (Mtl*)pb->GetReferenceTarget(fID);
	return texmap->GetName();
}

int plPickMaterialAnimationButtonParam::GetCount(IParamBlock2 *pb)
{
	return fKeys.Count();
}

plKey plPickMaterialAnimationButtonParam::GetKey(IParamBlock2 *pb, int idx)
{	
	int kcount = fKeys.Count();

	if ( idx >= 0 && idx < kcount )
	{
		return fKeys[idx];
	}

	return nil;
}

// this is in plResponderMtl.cpp
extern int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char* segName, hsTArray<plKey>& keys);;

void plPickMaterialAnimationButtonParam::CreateKeyArray(IParamBlock2* pb)
{
	fKeys.Reset();

	Mtl* mtl = (Mtl*)pb->GetReferenceTarget(fID);

	int bob = GetMatAnimModKey(mtl, nil, nil, fKeys);
}

void plPickMaterialAnimationButtonParam::DestroyKeyArray()
{
	fKeys.Reset();
}

int plPickMaterialAnimationButtonParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the picknode button
	HWND button = ICreateControl(hDlg, "CustButton");
	ISizeControl(hDlg, button, 84, 12, yOffset);

	fButton = GetICustButton(button);

	// Setup the button properties
	fButton->SetType(CBT_CHECK);
	fButton->SetButtonDownNotify(TRUE);
	fButton->SetCheckHighlight(TRUE);
	fButton->SetHighlightColor(GREEN_WASH);

	Mtl* texmap = (Mtl*)pb->GetReferenceTarget(fID);
	if (texmap)
		fButton->SetText(texmap->GetName());
	else
		fButton->SetText("(none)");

	// Create the Remove button
	fhRemove = ICreateControl(hDlg, "Button");
	yOffset += ISizeControl(hDlg, fhRemove, 17, 12, yOffset, 89);
	SetWindowText(fhRemove, "Clear");

	return yOffset;
}

bool plPickMaterialAnimationButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if ( Mtl* mtl = plPickMaterialMap::PickMaterial(0) )
				{
					pb->SetValue(fID, 0, (ReferenceTarget*)mtl);
					fButton->SetText(mtl->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}

	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plDropDownListParam::plDropDownListParam(ParamID id, const char *name, std::vector<std::string>* options) :
	plAutoUIParam(id, name), fhList(nil)
{
	if (options)
		fOptions = *options;
}

int plDropDownListParam::CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset)
{
	yOffset += IAddStaticText(hDlg, yOffset, fName) + 2;

	// Create the combobox
	fhList = ICreateControl(hDlg, "ComboBox", nil, CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VSCROLL, WS_EX_CLIENTEDGE);
	ISizeControl(hDlg, fhList, 100, 100, yOffset);
	yOffset += 13 + 2;
	ISetControlFont(fhList);

	return yOffset + 5;
}

bool plDropDownListParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_INITDIALOG)
	{
		IUpdateList(pb);
		return false;
	}
	
	if ((HWND)lParam == fhList)
	{
		if (HIWORD(wParam) == CBN_SELENDOK)
		{
			int idx = ComboBox_GetCurSel(fhList);

			if (idx >=0 && idx < fOptions.size())
			{
				char* buf = TRACKED_NEW char[fOptions[idx].size() + 1];
				strcpy(buf, fOptions[idx].c_str());
				pb->SetValue(fID, 0, buf);
				delete [] buf;
			}

			return true;
		}
	}

	return false;
}

void plDropDownListParam::IUpdateList(IParamBlock2 *pb)
{
	std::string val = "";
	int selection = -1;
	
	if (pb)
	{
		char* bob = pb->GetStr(fID);
		if (bob)
			val = bob;
	}
	
	ComboBox_ResetContent(fhList);

	for (int i = 0; i < fOptions.size(); i++)
	{
		const char *name = fOptions[i].c_str();
		
		ComboBox_AddString(fhList, name);

		if (fOptions[i] == val)
		{
			selection = i;
		}
	}

	if (selection >= 0)
	{
		ComboBox_SetCurSel(fhList, selection);
	}
}

int plDropDownListParam::GetParamType()
{
	return kTypeDropDownList;
}

int plDropDownListParam::GetCount(IParamBlock2 *pb)
{
	return pb->Count(fID);
}

const char* plDropDownListParam::GetString(IParamBlock2 *pb)
{
	return pb->GetStr(fID);
}

void plDropDownListParam::Show(int yOffset)
{
	/*
	HWND tmp;

	int ctrlOffset = 0;
	RECT rect;

	for (int i = 0; i < fControlVec.size(); i++)
	{
		tmp = fControlVec[i];
		GetWindowRect(tmp, &rect);
		
		SetRect(&rect, 3, 0, (rect.right - rect.left) + 3, rect.bottom - rect.top);
		//MapDialogRect(fhDlg, &rect);
		// Y is already in screen units
		rect.top = yOffset + ctrlOffset;
		rect.bottom = rect.bottom + rect.top;

		// Resize the window
		MoveWindow(tmp, rect.left, rect.top, rect.right - rect.left, (tmp == fhList) ? 100 : rect.bottom - rect.top, TRUE);
		//MoveWindow(tmp, rect.left, yOffset + ctrlOffset, (rect.right - rect.left), (rect.bottom - rect.top), FALSE);
		ctrlOffset += (rect.bottom - rect.top) + 2;
		ShowWindow(tmp, SW_SHOW);
	}
	*/

	yOffset += ISizeControl(fhDlg, fControlVec[0], 100, 8, yOffset) + 4;
	ISizeControl(fhDlg, fhList, 100, 100, yOffset);

	ShowWindow(fControlVec[0], SW_SHOW);
	ShowWindow(fControlVec[1], SW_SHOW);
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plPickGrassComponentButtonParam::plPickGrassComponentButtonParam(ParamID id, const char *name) :
plPickButtonParam(id, name, nil, false)
{
}

int plPickGrassComponentButtonParam::GetParamType()
{
	return kTypeGrassComponent;
}

plComponentBase* plPickGrassComponentButtonParam::GetComponent(IParamBlock2 *pb, int idx)
{
	hsAssert(idx == 0, "Pick buttons only have one key");
	plMaxNode *node = (plMaxNode*)pb->GetReferenceTarget(fID);
	if (node)
		return node->ConvertToComponent();

	return nil;
}


bool plPickGrassComponentButtonParam::IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb)
{
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_BUTTONUP)
	{
		if ((HWND)lParam == fButton->GetHwnd())
		{
			if (fButton->IsChecked())
			{
				if (plPick::GrassComponent(pb, fID, true))
				{
					INode *node = (INode*)pb->GetReferenceTarget(fID);
					if (node)
						fButton->SetText(node->GetName());
				}
				fButton->SetCheck(FALSE);
			}

			return true;
		}
	}
	// check if the reset button is hit
	if (msg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND)lParam == fhRemove)
		{
			pb->SetValue(fID, 0, (ReferenceTarget*)nil);
			fButton->SetText("(none)");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
