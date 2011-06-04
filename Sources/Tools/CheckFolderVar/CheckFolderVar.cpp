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
////////////////////////////////////////////////////////////////////
// This little app checks to see if the specified envrionment
// variable exists, creating it if necessary. If the variable
// doesn't exist, the app prompts the user with the Browse for
// Folder dialog box then sets the envrionment variable, using the
// selected folder as the value.
//
// Example:
// C:\>CheckFolderVar maxr4dir "Select the folder where max is installed then click Ok."
//


////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <string>

////////////////////////////////////////////////////////////////////

std::string gCurrentValue;

// get window handle of cancel button so we can disable it.
BOOL CALLBACK EnumChildWindowsCallbackProc(HWND hwnd,LPARAM lParam)
{
	char text[256];
	GetWindowText(hwnd,text,256);
	if (stricmp(text,"Cancel")==0)
	{
		*((HWND*)lParam) = hwnd;
		return FALSE;
	}
	return TRUE;
}

int CALLBACK BrosweForFolderCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		// disable cancel button
		HWND hCancelBtn = NULL;
		EnumChildWindows(hwnd,EnumChildWindowsCallbackProc,(LPARAM)&hCancelBtn);
		EnableWindow(hCancelBtn,FALSE);
		SendMessage(hwnd,BFFM_SETSELECTION,true,(LPARAM)gCurrentValue.data());
		break;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv)
{
	if (argc<2)
	{
		fprintf(stderr,"Usage: CheckFolderVar varname [-replace] [\"prompt msg\"]\n");
		return EXIT_FAILURE;
	}

	// read cmdline
	char ** args = argv;
	char * varname = NULL;
	bool replace = false;
	char prompt[1024] = "";
	for (int i=1; i<argc; i++)
	{
		if (!varname)
			varname = args[i];
		else if (stricmp(args[i],"-replace")==0)
			replace = true;
		else
			sprintf(prompt,"%s",args[i]);
	}
	if (prompt[0] == '\0')
		sprintf(prompt,"Set the '%s' environment variable by browsing to the desired folder and then click Ok:",varname);


	// open registry key
	HKEY hEnvKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,"Environment",0,KEY_WRITE|KEY_READ,&hEnvKey);

	// check if var already exists
	char value[MAX_PATH];
	DWORD bufsz = MAX_PATH;
	if(RegQueryValueEx(hEnvKey,varname,NULL,NULL,(LPBYTE)value,&bufsz) == ERROR_SUCCESS)
	{
		// exit if already exists and not replace
		if (!replace)
		{
			RegCloseKey(hEnvKey);
			return EXIT_SUCCESS;
		}
		gCurrentValue = value;
	}

TryAgain:
	// open browse for folder dialog
	BROWSEINFO  bInfo;
	LPITEMIDLIST itemList;
	LPMALLOC  shMalloc;
	memset( &bInfo, 0, sizeof( bInfo ) );
	bInfo.hwndOwner = NULL;
	bInfo.pidlRoot = NULL;
	bInfo.lpszTitle = prompt;
	bInfo.ulFlags = BIF_NEWDIALOGSTYLE;
	bInfo.pszDisplayName = NULL;
	bInfo.lpfn = BrosweForFolderCallbackProc;
	itemList = SHBrowseForFolder(&bInfo);
	if (!itemList)
		goto TryAgain;
	SHGetPathFromIDList(itemList,value);
	SHGetMalloc(&shMalloc);
	shMalloc->Free(itemList);
	shMalloc->Release();

	// set environment var
	RegSetValueEx(hEnvKey,varname,0,REG_SZ,(const BYTE*)value,strlen(value));

	// close registry key
	RegCloseKey(hEnvKey);

	// bubbye
	return EXIT_SUCCESS;
}
