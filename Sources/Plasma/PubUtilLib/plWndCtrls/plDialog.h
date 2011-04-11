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
#ifndef plDialog_h_inc
#define plDialog_h_inc

class plDialog : public plWindow
{
protected:
	DLGTEMPLATE* ILoadDlgTemplate()
	{
		HRSRC hRsrc = FindResourceEx(plWndCtrls::Instance(), RT_DIALOG, MAKEINTRESOURCE(fControlID), (WORD)(plWndCtrls::GetLanguage()));
		HGLOBAL hGlobal = LoadResource(plWndCtrls::Instance(), hRsrc);
		LPVOID lpsz = LockResource(hGlobal);
		return (DLGTEMPLATE*)lpsz;
	}

public:
	plDialog()
	{}
	plDialog( int inDialogId, plWindow * ownerWindow=nil )
	: plWindow( ownerWindow )
	{
		fControlID = inDialogId;
	}

	int CallDefaultProc( unsigned int message, unsigned int wParam, LONG lParam )
	{
		return 0;
	}
	virtual int DoModal()
	{
		CHECK(Handle()==nil);
		_Windows.push_back( this );
		_ModalCount++;
		int result = 0;
		if (plWndCtrls::GetLanguage() != 0)
			result = DialogBoxIndirectParam(plWndCtrls::Instance(),ILoadDlgTemplate(),fOwnerWindow?fOwnerWindow->Handle():nil,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);
		else
			result = DialogBoxParam(plWndCtrls::Instance(),MAKEINTRESOURCE(fControlID),fOwnerWindow?fOwnerWindow->Handle():nil,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);
		int err = GetLastError();
		_ModalCount--;
		return result;
	}
	void OpenWindow(bool visible=true)
	{
		CHECK(Handle()==nil);
		_Windows.push_back( this );

		HWND hWndCreated = nil;
		if (plWndCtrls::GetLanguage() != 0)
			hWndCreated = CreateDialogIndirectParam(plWndCtrls::Instance(),ILoadDlgTemplate(),nil,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);
		else
			hWndCreated = CreateDialogParam(plWndCtrls::Instance(),MAKEINTRESOURCE(fControlID),nil,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);

		int err = GetLastError();
		CHECK(hWndCreated);
		CHECK(hWndCreated==Handle());
		Show(visible);
	}
	void OpenChildWindow( int inControlId, bool visible )
	{
		CHECK(!Handle());
		_Windows.push_back( this );
		HWND hWndParent = inControlId ? GetDlgItem(fOwnerWindow->Handle(),inControlId) : fOwnerWindow ? fOwnerWindow->Handle() : nil;

		HWND hWndCreated = nil;
		if (plWndCtrls::GetLanguage() != 0)
			hWndCreated = CreateDialogIndirectParam(plWndCtrls::Instance(),ILoadDlgTemplate(),hWndParent,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);
		else
			hWndCreated = CreateDialogParam(plWndCtrls::Instance(),MAKEINTRESOURCE(fControlID),hWndParent,(int(APIENTRY*)(HWND,unsigned int,WPARAM,LPARAM))StaticWndProc,(LPARAM)this);

		int err = GetLastError();
		CHECK(hWndCreated);
		CHECK(hWndCreated==Handle());
		Show( visible );
	}
	virtual void OnInitDialog()
	{
		plWindow::OnInitDialog();
		for( int i=0; i<fControls.size(); i++ )
		{
			// Bind all child controls.
			plControl * control = fControls[i];
			CHECK(!control->Handle());
			control->Handle() = GetDlgItem( *this, control->fControlID );
			CHECK(control->Handle());
			_Windows.push_back(control);
			control->WindowDefWndProc = (WNDPROC)GetWindowLong( control->Handle(), GWL_WNDPROC );
			SetWindowLong( control->Handle(), GWL_WNDPROC, (LONG)plWindow::StaticWndProc );
		}
		for( int i=0; i<fControls.size(); i++ )
		{
			// Send create to all controls.
			fControls[i]->OnCreate();
		}
	}
	void EndDialog( int result )
	{
		::EndDialog( Handle(), result );
	}
	void EndDialogTrue()
	{
		EndDialog( 1 );
	}
	void EndDialogFalse()
	{
		EndDialog( 0 );
	}
};


#endif // plDialog_h_inc
