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
#ifndef plSDLBrowserDLG_inc
#define plSDLBrowserDLG_inc

#include "resource.h"
#include "../plWndCtrls/plWndCtrls.h"
#include <vector>

class plSimpleStateVariable;
class plStateDataRecord;
class plStateVariable;
class plSDStateVariable;
class plSDLBrowserDlg : public plDialog
{
private:
    bool            fCancelled;
    bool            fModified;
    bool            fReadOnly;
    plListBox       fVarListBox;
    plComboBox      fValueComboBox;
    plButton        fOK, fCancel;
    plTrackBar      fSDRecSlider;
    plSDStateVariable* fCurSDVar;
    int             fCurComboListBoxPos;
    
    typedef std::vector<plStateDataRecord*> SDRecStack;
    SDRecStack      fStateDataRecStack;

    plStateDataRecord*  IGetCurrentStateDataRec() const { return fStateDataRecStack.back(); }
    plStateDataRecord*  IGetPreviousStateDataRec() const;
    bool    IAtTopLevel() const { return fStateDataRecStack.size()==1;  }
    void    IPushStateDataRec(plStateDataRecord* sd) { fStateDataRecStack.push_back(sd);    }
    plStateDataRecord*  IPopStateDataRec();
    void    IPopulateVarListBox(plStateDataRecord* sd) ;
    void    IAddListBoxVar(plStateVariable* var, int cnt);
    void    IPopulateValueComboBox(plSimpleStateVariable* var) ;
    plStateVariable* IGetListBoxVar(int cur);
public:

    DECLARE_WINDOWCLASS(plSDLBrowserDialog, plDialog);
    plSDLBrowserDlg(int inDialogId=IDD_DIALOG_SDLBROWSER);
    ~plSDLBrowserDlg();
    
    int Run();

    // callbacks
    void OnInitDialog();
    void OnVarListSelChanged();
    void OnVarListDoubleClicked();
    void OnValueComboSelChanged();
    void OnValueComboEditChanged();
    void OnOKClicked();
    void OnCancelClicked();
    void OnSDRecSliderChanged();

    // setters
    void SetReadOnly(bool r) { fReadOnly=r; }
    void SetStateDataRec(plStateDataRecord* sd) { fStateDataRecStack.clear(); fStateDataRecStack.push_back(sd); }
    void SetDefaults();
    
    // getters
    bool GetCancelled() const { return fCancelled; }
    bool GetModified() const { return fModified; }
};

#endif  // plSDLBrowserDLG_inc
