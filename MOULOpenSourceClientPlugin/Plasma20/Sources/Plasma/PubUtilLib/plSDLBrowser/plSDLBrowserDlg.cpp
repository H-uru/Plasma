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
#include "hsTemplates.h"
#include "../plSDL/plSDL.h"

// 'this' : used in base member initializer list
#pragma warning(disable:4355)

plSDLBrowserDlg::plSDLBrowserDlg(int inDialogId) : 
plDialog(inDialogId) ,
fVarListBox(this,IDC_LIST_VAR),
fValueComboBox(this,IDC_COMBO_VALUE),
fOK(this,IDOK),
fCancel(this,IDCANCEL),
fSDRecSlider(this,IDC_SLIDER_SDRECS),
fCancelled(false),
fReadOnly(true),
fCurSDVar(nil),
fCurComboListBoxPos(-1),
fModified(false)
{ 
	fVarListBox.SelectionChangeDelegate = plDelegate(this,(TDelegate)OnVarListSelChanged);  
	fVarListBox.DoubleClickDelegate = plDelegate(this,(TDelegate)OnVarListDoubleClicked);  
	fValueComboBox.fSelectionChangeDelegate= plDelegate(this,(TDelegate)OnValueComboSelChanged);  
	fValueComboBox.fEditUpdateDelegate= plDelegate(this,(TDelegate)OnValueComboEditChanged);  
	fOK.fClickDelegate = plDelegate(this,(TDelegate)OnOKClicked);  
	fCancel.fClickDelegate = plDelegate(this,(TDelegate)OnCancelClicked);  
	fSDRecSlider.fThumbPositionDelegate= plDelegate(this,(TDelegate)OnSDRecSliderChanged);  
	fSDRecSlider.fThumbTrackDelegate= plDelegate(this,(TDelegate)OnSDRecSliderChanged);  
}

plSDLBrowserDlg::~plSDLBrowserDlg()
{
	
}

void plSDLBrowserDlg::SetDefaults()
{

}

plStateVariable* plSDLBrowserDlg::IGetListBoxVar(int cur)
{
	plStateDataRecord* sdRec=IGetCurrentStateDataRec();
	plStateVariable* var = (plStateVariable*)fVarListBox.GetItemData(cur);	// sometimes doesn't work?
	if (!var)
	{
		if (!IAtTopLevel())
			cur++;

		if (cur<sdRec->GetNumVars())
			var=sdRec->GetVar(cur);
		else
		{
			cur-=sdRec->GetNumVars();
			var=sdRec->GetSDVar(cur);
		}
	}
	return var;
}


int  plSDLBrowserDlg::Run()
{
	SetDefaults();
	int ret=DoModal();
	if (ret<0)
	{
		hsAssert(false, hsTempString(kFmtCtor, "SDL Browser dialog failed to initialize, err code %d, GetLastError %d", 
			ret, GetLastError()));
		return hsFail;
	}
	return ret;
}

plStateDataRecord*	plSDLBrowserDlg::IPopStateDataRec() 
{ 
	plStateDataRecord* sd=IGetCurrentStateDataRec(); 
	fStateDataRecStack.pop_back();	
	return sd; 
}

void	plSDLBrowserDlg::IAddListBoxVar(plStateVariable* var, int cnt)
{
	std::string s;
	s = s + (char*)hsTempString(kFmtCtor, "%s[",var->GetVarDescriptor()->GetName());
	if (var->GetVarDescriptor()->GetCount())
		s = s + (char*)hsTempString(kFmtCtor, "%d",var->GetVarDescriptor()->GetCount());
	s = s + (char*)hsTempString(kFmtCtor, "], %s",var->GetVarDescriptor()->GetTypeString());

	fVarListBox.AddString(s.c_str());
	fVarListBox.SetItemData(cnt, var);
	hsAssert(var==fVarListBox.GetItemData(cnt), "set item data failed");

	hsStatusMessageF("%s\n", s.c_str());
}

void	plSDLBrowserDlg::IPopulateVarListBox(plStateDataRecord* sd)
{
	fVarListBox.Empty();
	if (!sd)
		return;
	
	hsTempString title(kFmtCtor, "SDL Browser - %s, version %d",
		sd->GetDescriptor()->GetName(), sd->GetDescriptor()->GetVersion());
	SetText(title);

	int i, cnt=0;

	if (!IAtTopLevel())
	{
		fVarListBox.AddString("..");
		fVarListBox.SetItemData(cnt, IGetPreviousStateDataRec());
		cnt++;
	}

	for (i=0;i<sd->GetNumVars(); i++, cnt++)
	{
		plSimpleStateVariable* var=sd->GetVar(i);
		IAddListBoxVar(var, cnt);
	}

	for (i=0;i<sd->GetNumSDVars(); i++, cnt++)
	{
		plSDStateVariable* var=sd->GetSDVar(i);
		IAddListBoxVar(var, cnt);
	}
}

void	plSDLBrowserDlg::IPopulateValueComboBox(plSimpleStateVariable* var) 
{
	fValueComboBox.Empty();
	
	int i;
	for(i=0;i<var->GetCount(); i++)
	{
		fValueComboBox.AddString((char*)hsTempString(var->GetAsString(i)));
	}
}

plStateDataRecord*	plSDLBrowserDlg::IGetPreviousStateDataRec() const
{
	SDRecStack::const_iterator it=fStateDataRecStack.end();
	it--;	// last
	it--;	// 2nd to last
	return *it;
}
