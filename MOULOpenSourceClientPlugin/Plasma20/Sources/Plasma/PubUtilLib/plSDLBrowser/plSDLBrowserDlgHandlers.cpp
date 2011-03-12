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
#include "plSDLBrowserDlg.h"
#include "../plSDL/plSDL.h"

void plSDLBrowserDlg::OnInitDialog()
{
	plDialog::OnInitDialog();
	const int kWinPos=250;

	plRect r=GetWindowRect();
	r.Min.X+=kWinPos;
	r.Min.Y+=kWinPos;
	r.Max.X+=kWinPos;
	r.Max.Y+=kWinPos;
	MoveWindow(r, true);

	fSDRecSlider.Hide();
	
	if (IGetCurrentStateDataRec())
		IPopulateVarListBox(IGetCurrentStateDataRec());
}

void plSDLBrowserDlg::OnOKClicked()			
{	
	
	EndDialogTrue();	
}

void plSDLBrowserDlg::OnCancelClicked()
{
	EndDialogFalse();
	fCancelled=true;
}	

void plSDLBrowserDlg::OnVarListSelChanged()
{
	int cur=fVarListBox.GetCurrent();
	std::string curString=fVarListBox.GetString(cur);	
	if (curString=="..")
		return;

	plStateVariable* var=IGetListBoxVar(cur);
	hsAssert(var, "nil var?");
	if (var->GetAsSimpleStateVar())
	{
		plSimpleStateVariable* sVar=var->GetAsSimpleStateVar();
		IPopulateValueComboBox(sVar);
		fValueComboBox.SetCurrent(0);
		fCurComboListBoxPos=0;
	}
}

void plSDLBrowserDlg::OnVarListDoubleClicked()
{
	fCurSDVar=nil;
	fSDRecSlider.Hide();

	int cur=fVarListBox.GetCurrent();
	std::string curString=fVarListBox.GetString(cur);	
	if (curString=="..")
	{
		IPopStateDataRec();
		IPopulateVarListBox(IGetCurrentStateDataRec());
		return;
	}

	plStateVariable* var=IGetListBoxVar(cur);
	hsAssert(var, "nil var?");
	if (var->GetAsSDStateVar())
	{
		// user doubleclicked an SDVar
		plSDStateVariable* sdVar=var->GetAsSDStateVar();
		if (sdVar->GetCount()==0)
		{
			sdVar->Resize(1);
		}
		else
		if (sdVar->GetCount()>1)
		{
			fSDRecSlider.Show();
			int max=var->GetCount();
			fSDRecSlider.SetRange(0, max-1);
		}
		IPushStateDataRec(sdVar->GetStateDataRecord(0));
		IPopulateVarListBox(IGetCurrentStateDataRec());
		fCurSDVar=sdVar;
		fValueComboBox.Empty();
	}
}

void plSDLBrowserDlg::OnValueComboSelChanged()
{
	int cur=fValueComboBox.GetCurrent();
	hsStatusMessageF("Changing cur combo box sel to %d\n", cur);
	if (cur>=0)
		fCurComboListBoxPos=cur;
}

void plSDLBrowserDlg::OnValueComboEditChanged()
{
	if (fReadOnly)
		return;

	// get var from list box
	int listBoxPos=fVarListBox.GetCurrent();
	plStateVariable* var=IGetListBoxVar(listBoxPos);
	hsAssert(var, "nil var?");
	hsAssert(var->GetAsSimpleStateVar(), "wrong type of var");
	
	// change value of var
	int comboxBoxPos=fValueComboBox.GetCurrent();
	if (comboxBoxPos<0)
		comboxBoxPos=fCurComboListBoxPos;
	if (comboxBoxPos>=0)
	{
		std::string editString=fValueComboBox.GetText();
		if (var->GetAsSimpleStateVar()->SetFromString(editString.c_str(), comboxBoxPos))
		{
			hsStatusMessageF("changing item %d to %s", comboxBoxPos, editString.c_str());
			fValueComboBox.InsertString(comboxBoxPos, editString.c_str());
			fValueComboBox.DeleteString(comboxBoxPos+1);
			var->GetAsSimpleStateVar()->SetDirty(true);
			var->GetAsSimpleStateVar()->SetUsed(true);
		}	
#if 0
		editString.reverse();
		fValueComboBox.SetText(editString.c_str());
#endif
	}
	fModified=true;
}

void plSDLBrowserDlg::OnSDRecSliderChanged()
{
	int pos=fSDRecSlider.GetPos();
	IPopStateDataRec();
	IPushStateDataRec(fCurSDVar->GetStateDataRecord(pos));
	IPopulateVarListBox(IGetCurrentStateDataRec());
	fValueComboBox.Empty();

	hsStatusMessageF("Slider pos=%d\n", pos);
}

