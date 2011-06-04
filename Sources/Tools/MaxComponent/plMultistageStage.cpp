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
#include "plMultistageStage.h"
#include "max.h"
#include "hsStream.h"
#include "resource.h"
#include "hsUtils.h"

#include "../plAvatar/plAnimStage.h"

// We don't want to be subject to any changes to ReadSafeString, so we just keep
// our own version now.  Unfortunately, some files were saved with a modified
// version of it, so we need to keep all the backwards compatability BS
char* MyReadSafeString(hsStream* s)
{
	char *name = nil;
	UInt16 numChars = s->ReadSwap16();

	bool oldFormat = !(numChars & 0xf000);
	if (oldFormat)
		s->ReadSwap16();

	numChars &= ~0xf000;
	hsAssert(numChars <= s->GetSizeLeft(), "Bad string");
	if (numChars > 0)
	{
		name = TRACKED_NEW char[numChars+1];
		s->Read(numChars, name);
		name[numChars] = '\0';		
	}

	return name;
}

void MyWriteSafeString(hsStream* s, const char* str)
{
	int len = hsStrlen(str);
	hsAssert(len<0xf000, "String too long");
	s->WriteSwap16(len | 0xf000);
	if (len > 0)
		s->Write(len, str);
}

plBaseStage::plBaseStage()
{
	fName = nil;
}

plBaseStage::~plBaseStage()
{
	delete [] fName;
}

BOOL plBaseStage::IStaticDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

	plBaseStage *stage = (plBaseStage*)GetWindowLong(hDlg, GWL_USERDATA);

	if (!stage)
		return FALSE;

	return stage->IDlgProc(hDlg, msg, wParam, lParam);
}

BOOL plBaseStage::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

HWND plBaseStage::ICreateDlg(int dialogID, char* title)
{
	return GetCOREInterface()->AddRollupPage(hInstance,
											MAKEINTRESOURCE(dialogID),
											IStaticDlgProc,
											title,
											(LPARAM)this);
}

void plBaseStage::IDestroyDlg(HWND hDlg)
{
	if (hDlg)
		GetCOREInterface()->DeleteRollupPage(hDlg);
}

const char* plBaseStage::GetName()
{
	if (!fName)
		fName = hsStrcpy("DefaultName");
	return fName;
}

void plBaseStage::SetName(const char* name)
{
	delete [] fName;
	fName = hsStrcpy(name);
}

void plBaseStage::Read(hsStream *stream)
{
	int version = stream->ReadSwap16();
	delete [] fName;
	fName = MyReadSafeString(stream);
}

void plBaseStage::Write(hsStream *stream)
{
	stream->WriteSwap16(1);
	MyWriteSafeString(stream, fName);
}

void plBaseStage::IBaseClone(plBaseStage* clone)
{
	clone->fName = hsStrcpy(fName);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

HWND plStandardStage::fDlg = NULL;

plStandardStage::plStandardStage()
{
	fAnimName = nil;
	fNumLoops = 0;
	fLoopForever = false;
	fForward = 0;
	fBackward = 0;
	fStageAdvance = 0;
	fStageRegress = 0;
	fNotify = 0;
	fUseGlobalCoord = false;
	fDoAdvanceTo = false;
	fAdvanceTo = 0;
	fDoRegressTo = false;
	fRegressTo = 0;
}

plStandardStage::~plStandardStage()
{
	delete [] fAnimName;
}

void plStandardStage::Read(hsStream *stream)
{
	plBaseStage::Read(stream);

	UInt16 version = stream->ReadSwap16();

	delete [] fAnimName;
	fAnimName = MyReadSafeString(stream);
	fNumLoops = stream->ReadSwap32();
	fLoopForever = stream->Readbool();
	fForward = stream->ReadByte();
	fBackward = stream->ReadByte();
	fStageAdvance = stream->ReadByte();
	fStageRegress = stream->ReadByte();
	fNotify = stream->ReadByte();
	fUseGlobalCoord = stream->Readbool();
	if(version > 1)
	{
		// these guys were added in version 2
		fDoAdvanceTo = stream->Readbool();
		fAdvanceTo = stream->ReadSwap32();
		fDoRegressTo = stream->Readbool();
		fRegressTo = stream->ReadSwap32();
	}
}

void plStandardStage::Write(hsStream *stream)
{
	plBaseStage::Write(stream);

	stream->WriteSwap16(2);

	MyWriteSafeString(stream, fAnimName);
	stream->WriteSwap32(fNumLoops);
	stream->Writebool(fLoopForever);
	stream->WriteByte(fForward);
	stream->WriteByte(fBackward);
	stream->WriteByte(fStageAdvance);
	stream->WriteByte(fStageRegress);
	stream->WriteByte(fNotify);
	stream->Writebool(fUseGlobalCoord);

	// these next 4 were added in version 2
	stream->Writebool(fDoAdvanceTo);
	stream->WriteSwap32(fAdvanceTo);
	stream->Writebool(fDoRegressTo);
	stream->WriteSwap32(fRegressTo);
}

void plStandardStage::CreateDlg()
{
	hsAssert(!fDlg, "Dialog wasn't destroyed");
	fDlg = ICreateDlg(IDD_COMP_MULTIBEH_NORMAL, "Standard Stage");

	IInitDlg();
}

void plStandardStage::DestroyDlg()
{
	// This shitty Max edit box doesn't notify of changes if it has focus during
	// shutdown, so we just get it no matter what.
	IGetAnimName();

	IDestroyDlg(fDlg);
	fDlg = nil;
}

#define SetBit(f,b,on) on ? hsSetBits(f,b) : hsClearBits(f,b)

BOOL plStandardStage::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		{
			int code = HIWORD(wParam);
			int id = LOWORD(wParam);

			// Combo changed
			if (code == CBN_SELCHANGE)
			{
				int sel = ComboBox_GetCurSel((HWND)lParam);
				int type = ComboBox_GetItemData((HWND)lParam, sel);

				if (id == IDC_FORWARD_COMBO)
					fForward = type;
				else if (id == IDC_BACKWARD)
					fBackward = type;
				else if (id == IDC_ADVANCE)
					fStageAdvance = type;
				else if (id == IDC_REGRESS)
					fStageRegress = type;

				SetSaveRequiredFlag();

				return TRUE;
			}
			// Button clicked or checkbox checked
			else if (code == BN_CLICKED)
			{
				bool isChecked = (Button_GetCheck((HWND)lParam) == BST_CHECKED);
				if (id == IDC_LOOP_FOREVER)
				{
					fLoopForever = isChecked;

					ISpinnerControl *spin = GetISpinner(GetDlgItem(fDlg, IDC_NUM_LOOPS_SPIN));
					spin->Enable(!fLoopForever);
				}
				else if (id == IDC_GLOBAL_COORD)
				{
					fUseGlobalCoord = isChecked;
				}
				else if (id == IDC_CHECK_ENTER)
					SetBit(fNotify, plAnimStage::kNotifyEnter, isChecked);
				else if (id == IDC_CHECK_LOOP)
					SetBit(fNotify, plAnimStage::kNotifyLoop, isChecked);
				else if (id == IDC_CHECK_ADVANCE)
					SetBit(fNotify, plAnimStage::kNotifyAdvance, isChecked);
				else if (id == IDC_CHECK_REGRESS)
					SetBit(fNotify, plAnimStage::kNotifyRegress, isChecked);
				else if (id == IDC_DO_ADVANCE_TO)
				{
					fDoAdvanceTo = isChecked;
					ISpinnerControl *spin = GetISpinner(GetDlgItem(fDlg, IDC_ADVANCE_STAGE_SPIN));
					spin->Enable(fDoAdvanceTo);
				} else if (id == IDC_DO_REGRESS_TO)
				{
					fDoRegressTo = isChecked;
					ISpinnerControl *spin = GetISpinner(GetDlgItem(fDlg, IDC_REGRESS_STAGE_SPIN));
					spin->Enable(fDoRegressTo);
				}

				SetSaveRequiredFlag();

				return TRUE;
			}
		}
		break;

	// Num loops spinner changed
	case CC_SPINNER_CHANGE:
		{
			ISpinnerControl* spin = (ISpinnerControl*)lParam;
			if (LOWORD(wParam) == IDC_NUM_LOOPS_SPIN)
			{
				fNumLoops = spin->GetIVal();
			} else if (LOWORD(wParam) == IDC_ADVANCE_STAGE_SPIN) {
				fAdvanceTo = spin->GetIVal();
			} else if (LOWORD(wParam) == IDC_REGRESS_STAGE_SPIN) {
				fRegressTo = spin->GetIVal();
			}

			SetSaveRequiredFlag();
			return TRUE;
		}
		break;

	// Anim name changed
	case WM_CUSTEDIT_ENTER:
		if (LOWORD(wParam) == IDC_ANIM_NAME)
		{
			IGetAnimName();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void plStandardStage::IGetAnimName()
{
	ICustEdit* edit = GetICustEdit(GetDlgItem(fDlg, IDC_ANIM_NAME));
	char buf[256];
	edit->GetText(buf, sizeof(buf));

	if (!hsStrEQ(buf, fAnimName))
	{
		delete [] fAnimName;
		fAnimName = hsStrcpy(buf);

		SetSaveRequiredFlag();
	}
}

struct NameType
{
	const char* name;
	int type;
};

static NameType gForward[] =
{
	{ "None",		plAnimStage::kForwardNone },
	{ "Keyboard",	plAnimStage::kForwardKey },
	{ "Automatic",	plAnimStage::kForwardAuto }
};

static NameType gBackward[] =
{
	{ "None",		plAnimStage::kBackNone },
	{ "Keyboard",	plAnimStage::kBackKey },
	{ "Automatic",	plAnimStage::kBackAuto }
};

static NameType gAdvance[] =
{
	{ "None",		plAnimStage::kAdvanceNone },
	{ "Auto At End",plAnimStage::kAdvanceAuto }
};

static NameType gRegress[] =
{
	{ "None",		plAnimStage::kRegressNone },
	{ "Auto At End",plAnimStage::kRegressAuto }
};

static void LoadCombo(HWND hCombo, NameType* nameInt, int size, int curVal)
{
	int num = size / sizeof(NameType);

	for (int i = 0; i < num; i++)
	{
		int idx = ComboBox_AddString(hCombo, nameInt[i].name);
		ComboBox_SetItemData(hCombo, idx, nameInt[i].type);

		if (nameInt[i].type == curVal)
			ComboBox_SetCurSel(hCombo, idx);
	}
}

void plStandardStage::IInitDlg()
{
	ICustEdit* edit = GetICustEdit(GetDlgItem(fDlg, IDC_ANIM_NAME));
	edit->SetText(fAnimName);

	HWND hForward = GetDlgItem(fDlg, IDC_FORWARD_COMBO);
	LoadCombo(hForward, gForward, sizeof(gForward), fForward);

	HWND hBackward = GetDlgItem(fDlg, IDC_BACKWARD);
	LoadCombo(hBackward, gBackward, sizeof(gBackward), fBackward);

	HWND hAdvance = GetDlgItem(fDlg, IDC_ADVANCE);
	LoadCombo(hAdvance, gAdvance, sizeof(gAdvance), fStageAdvance);

	HWND hRegress = GetDlgItem(fDlg, IDC_REGRESS);
	LoadCombo(hRegress, gRegress, sizeof(gRegress), fStageRegress);

	CheckDlgButton(fDlg, IDC_CHECK_ENTER, (fNotify & plAnimStage::kNotifyEnter) ? BST_CHECKED : BST_UNCHECKED); 
	CheckDlgButton(fDlg, IDC_CHECK_LOOP, (fNotify & plAnimStage::kNotifyLoop) ? BST_CHECKED : BST_UNCHECKED); 
	CheckDlgButton(fDlg, IDC_CHECK_ADVANCE, (fNotify & plAnimStage::kNotifyAdvance) ? BST_CHECKED : BST_UNCHECKED); 
	CheckDlgButton(fDlg, IDC_CHECK_REGRESS, (fNotify & plAnimStage::kNotifyRegress) ? BST_CHECKED : BST_UNCHECKED); 

	ISpinnerControl *spin = SetupIntSpinner(fDlg, IDC_NUM_LOOPS_SPIN, IDC_NUM_LOOPS_EDIT, 0, 10000, fNumLoops);

	CheckDlgButton(fDlg, IDC_LOOP_FOREVER, fLoopForever ? BST_CHECKED : BST_UNCHECKED); 
	if (fLoopForever)
		spin->Disable();

	spin = SetupIntSpinner(fDlg, IDC_ADVANCE_STAGE_SPIN, IDC_ADVANCE_STAGE_EDIT, -1, 100, fAdvanceTo);
	CheckDlgButton(fDlg, IDC_DO_ADVANCE_TO, fDoAdvanceTo ? BST_CHECKED : BST_UNCHECKED);
	if (! fDoAdvanceTo)
		spin->Disable();

	spin = SetupIntSpinner(fDlg, IDC_REGRESS_STAGE_SPIN, IDC_REGRESS_STAGE_EDIT, -1, 100, fRegressTo);
	CheckDlgButton(fDlg, IDC_DO_REGRESS_TO, fDoRegressTo ? BST_CHECKED : BST_UNCHECKED);
	if (! fDoRegressTo)
		spin->Disable();

	CheckDlgButton(fDlg, IDC_GLOBAL_COORD, fUseGlobalCoord ? BST_CHECKED : BST_UNCHECKED); 
}

plAnimStage* plStandardStage::CreateStage()
{
	int loopCount = fLoopForever ? -1 : fNumLoops;
	plAnimStage* stage = TRACKED_NEW plAnimStage(fAnimName,
										fNotify,
										(plAnimStage::ForwardType)fForward,
										(plAnimStage::BackType)fBackward,
										(plAnimStage::AdvanceType)fStageAdvance,
										(plAnimStage::RegressType)fStageRegress,
										loopCount,
										fDoAdvanceTo,
										fAdvanceTo,
										fDoRegressTo,
										fRegressTo);

	return stage;
}

plBaseStage* plStandardStage::Clone()
{
	plStandardStage* clone = TRACKED_NEW plStandardStage;
	clone->fAnimName = hsStrcpy(fAnimName);
	clone->fNumLoops = fNumLoops;
	clone->fLoopForever = fLoopForever;
	clone->fForward = fForward;
	clone->fBackward = fBackward;
	clone->fStageAdvance = fStageAdvance;
	clone->fStageRegress = fStageRegress;
	clone->fNotify = fNotify;
	clone->fUseGlobalCoord = fUseGlobalCoord;

	IBaseClone(clone);

	return clone;
}
