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
#ifndef plLoginDialog_h_inc
#define plLoginDialog_h_inc

#include "plWndCtrls/plWndCtrls.h"
// #include "plEncryption/plChallengeResponse.h"
#include "pnNetCommon/plNetAddress.h"

class plMainDialog;
class plNetMsgAuthenticateChallenge;
class plNetMsgAccountAuthenticated;
class plNetMessage;

class plLoginDialog : public plDialog
{
    HWND fParentWnd;
public:
    DECLARE_WINDOWCLASS(plLoginDialog, plDialog);

    plLoginDialog( HWND parentWnd=nil );

    bool Login();

    void OnInitDialog();

    void HandleAuthChallenge(plNetMsgAuthenticateChallenge * msg);
    void HandleAccountAuthenticated(plNetMsgAccountAuthenticated * msg);
    void StartLogin();
    void CompleteLogin();
    void FailLogin(int reasonCode);
    void FailLogin(const char* str);
    void TimeoutLogin();
    void Logout();
    void SendAuthenticateHello();
    void SendAuthenticateResponse();

    bool IsNetworkPlayDisabled();
    bool IsNetworkPlayEnabled();
//  void UpdateCtrls();
    void SelectedLobbyChanged();
    void SelectedLobbyTextEdited();
    void OnLobbyListLostFocus();
    void SendLobbyLeave();
    bool RefreshLobbyList();
    bool GetCancelled() const { return fCancelled; }
    bool GetRememberPassword() const { return fRememberPassword.IsChecked();    }

    // callbacks
    virtual void GetLobbyList(plStringList& lobbies) = 0;   
    virtual bool AllowSinglePlayerLobby() { return true; }  // checked in non external-release build
    virtual void SetDataServerUserName(bool local, const char* s) {}
    virtual void SetDataServerPassword(bool local, const char* s) {}
    virtual void RemoveLobbyPeer() {}
    virtual void NotifyConnected() {}
    virtual void NotifyDisconnected() {}
    virtual void GetClientManifests() {}
    virtual void UpdateAllCtrls() {}
    virtual unsigned int GetPacketSize() = 0;
    virtual bool SendMsg(plNetMessage * msg, plNetAddress & addr) = 0;
    virtual void OnLoginClicked() {}

    std::string MakeSafeLobbyServerName(const std::string & value);

    int CallDefaultProc( unsigned int message, unsigned int wParam, LONG lParam );

    plEdit          fAccountName;
    plEdit          fPassword;
    plChallengeResponse fChallengeResponse;
    plComboBox      fLobbyList;
    plLabel         fLobbyText;
    plCheckBox      fRememberPassword;

#ifndef PLASMA_EXTERNAL_RELEASE
    plButton        fServerQueryBtn;
    void ServerQueryBtnClicked();
#endif

    plConfigValue   fAccountNameVal;
    plConfigValue   fPasswordVal;       // the pwd as a MD5 hash
    plConfigValue   fRememberPasswordVal;   // the checkbox state
    plConfigValue   fPasswordLen;       // the length of the original pwd
    plConfigValue   fLobbyVal;
    bool            fAutoLogin;

    plStatusBar     fStatusBar;

protected:
    int ICheckNetVersion(plNetMsgAuthenticateChallenge * msg);
    void ILogin();
    void IExit();
    void IOnRememberPwdChanged();

    bool    fCancelled;
    plButton fLoginBtn;
    plButton fCancelBtn;
};

#endif // plLoginDialog_h_inc
