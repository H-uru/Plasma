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
#ifndef _plResTreeView_h
#define _plResTreeView_h

class plResTreeView
{
	protected:

		static HWND	fInfoDlg;
		static bool	fFilter;

		static void	IFindNextObject( HWND tree );

	public:

		static void	FindObject( HWND tree );
		static void	FindNextObject( HWND tree );

		static void	FillTreeViewFromRegistry( HWND hWnd );
		static void	ClearTreeView( HWND hWnd );

		static BOOL CALLBACK	InfoDlgProc( HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam );

		static void	UpdateInfoDlg( HWND treeCtrl );

		static void	VerifyCurrentPage( HWND treeCtrl );

		static void	SelectionDblClicked( HWND treeCtrl );

		static void	FilterLoadables( bool filter, HWND treeCtrl );

		static void SaveSelectedObject(HWND treeCtrl);
};

#endif //_plResTreeView_h