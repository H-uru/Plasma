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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef __PLCOMPONENT_PROC_BASE_H__
#define __PLCOMPONENT_PROC_BASE_H__

#include <string>
#include <vector>

#include "MaxMain/MaxAPI.h"

//
//  Const Char* [] Version
//
class plBaseComponentProc : public ParamMap2UserDlgProc
{
protected:
    void ILoadComboBox(HWND hComboBox, const TCHAR* names[])
    {
        SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);
        for (int i = 0; names[i]; i++)
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)names[i]);
        SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
    }
};

class plLCBoxComponentProc : public plBaseComponentProc
{
protected:
    void ILoadListBox(HWND hListBox, IParamBlock2 *pb, int param, const TCHAR* names[])
    {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < pb->Count(param); i++)
        {
            int idx = pb->GetInt(param, 0, i);
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)names[idx]);
        }
    }
};

//
//  VChar Version (Vector<string>)
//


typedef std::vector<std::string> VStringArray;

class plVSBaseComponentProc : public ParamMap2UserDlgProc
{
protected:
    void ILoadComboBox(HWND hComboBox, VStringArray &names)
    {
        SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);

        int nNames = 0;

        if(names.empty())
            return;
        else
            nNames = names.size();

        if(nNames)
            for (int i = 0; i < nNames ; i++)
            {
                const char * name = names[i].c_str();
                SendMessage(hComboBox, CB_INSERTSTRING, i, (LPARAM)name);
                
            }
    
        SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
        

    }
    
    void ILoadListBox(HWND hListBox, VStringArray &names, int* GetItemVals, int NumVals)
    {

        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        for (int i = 0; i < names.size(); i++)
        {
            const char* c_name = names[i].c_str();
            LRESULT idxptr = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)c_name);
            
            SendMessage(hListBox, LB_SETITEMDATA, (WPARAM) idxptr, (LPARAM) GetItemVals[i]);
            
        }
    }




};





#endif