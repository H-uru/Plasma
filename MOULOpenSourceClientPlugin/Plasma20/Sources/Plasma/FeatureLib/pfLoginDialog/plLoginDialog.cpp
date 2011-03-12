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
#include "plLoginDialog.h"
#include "resource.h"
#include "../plNetCommon/plNetCommonConstants.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plHttpServer/plHttpResponse.h"
#include "../plSDL/plSDL.h"
#include "../plFile/hsFiles.h"
#include "../plNetMessage/plNetCommonMessage.h"

// 'this' : used in base member initializer list
#pragma warning(disable:4355)

#define kAuthTimedOut WM_USER+2

plLoginDialog::plLoginDialog( HWND parentWnd )
:	plDialog(IDD_DIALOG_LOGIN)
,	fParentWnd( parentWnd )
,	fLoginBtn(this, IDC_LOGIN_LOGIN, plDelegate(this,(TDelegate)ILogin))
,	fCancelBtn(this, IDC_LOGIN_CANCEL, plDelegate(this,(TDelegate)IExit))
,	fAccountName(this,IDC_LOGIN_USERNAME)
,	fPassword(this,IDC_LOGIN_PASSWORD)
,	fLobbyList(this,IDC_LOGIN_LOBBYLIST)
,	fLobbyText(this, IDC_LOGIN_STATIC_SERVER)
,	fRememberPassword(this, IDC_REMEMBER_PASSWORD, plDelegate(this, (TDelegate)IOnRememberPwdChanged))
,	fCancelled(false)
,	fAutoLogin(false)
#ifndef PLASMA_EXTERNAL_RELEASE
,	fServerQueryBtn(this, IDC_SERVER_QUERY_BTN)
#endif
{
	fLobbyList.fSelectionEndOkDelegate = plDelegate(this,(TDelegate)SelectedLobbyChanged);
	fLobbyList.fEditUpdateDelegate = plDelegate(this,(TDelegate)SelectedLobbyTextEdited);
	fLobbyList.fKillFocusDelegate = plDelegate(this,(TDelegate)OnLobbyListLostFocus);
#ifndef PLASMA_EXTERNAL_RELEASE
	fServerQueryBtn.fClickDelegate = plDelegate(this,(TDelegate)ServerQueryBtnClicked);
#endif
}

std::string plLoginDialog::MakeSafeLobbyServerName(const std::string & value)
{
	//return plIDataServer::MakeSafeMachineName(value,"parablegame.cyanworlds.com");
	return "";
}


void plLoginDialog::SelectedLobbyChanged()
{
	fLobbyVal.SetValue(fLobbyList.GetValue().c_str());
	fLobbyList.SetEdited(false);
}

void plLoginDialog::SelectedLobbyTextEdited()
{
	fLobbyVal.SetValue(fLobbyList.GetValue().c_str());
	fLobbyList.SetEdited(true);
}

// Ugh
#ifdef PLASMA_EXTERNAL_RELEASE
#include <time.h>
#endif

bool plLoginDialog::RefreshLobbyList()
{
	fStatusBar.SetText(L"Refreshing lobby server list...");

	plStringList lobbies;
	std::vector<std::wstring> wLobbies;
	GetLobbyList(lobbies);

	// Strip off the shard name and just leave the address
	for (int i = 0; i < lobbies.size(); i++)
	{
		std::string& str = lobbies[i];

		std::string::size_type endofname = str.find('\t');
		if (endofname != std::string::npos)
			str.erase(str.begin() + endofname, str.end());

		wchar_t *wLobby = hsStringToWString(str.c_str());
		wLobbies.push_back(wLobby);
		delete [] wLobby;
	}

	fLobbyList.Empty();

#ifdef PLASMA_EXTERNAL_RELEASE
	// In release mode, put the user in a random lobby for rudimentary load balancing
	int numLobbies = lobbies.size();
	if (numLobbies > 0)
	{
		srand(time(NULL));
		int rnum = rand();
		int whichLobby = rnum % numLobbies;

		fLobbyList.AddString(wLobbies[whichLobby].c_str());
	}

	fLobbyList.SetCurrent(0);
#else
	if (AllowSinglePlayerLobby())
		fLobbyList.AddString(L"Single Player");
	fLobbyList.AddStrings(wLobbies);

	wchar_t *wLobby = hsStringToWString(fLobbyVal.GetValue().c_str());
	int index = fLobbyList.FindStringExact(wLobby);
	if (index==LB_ERR && fLobbyVal.GetValue().length()>0)
	{
		fLobbyList.AddString(wLobby);
		index = fLobbyList.FindStringExact(wLobby);
	}
	delete [] wLobby;

	fLobbyList.SetCurrent((index!=LB_ERR)?index:0);
#endif // PLASMA_EXTERNAL_RELEASE

	SelectedLobbyChanged();

	fStatusBar.SetText(L"");
	return true;
}

void plLoginDialog::OnLobbyListLostFocus()
{
	std::string value = fLobbyList.GetValue();
	if (value.length()==0)
	{
		fLobbyList.SetCurrent(0);
		fLobbyList.SetValue(MakeSafeLobbyServerName(fLobbyList.GetValue()).c_str());
		SelectedLobbyChanged();
	}
}

#if 0
void plLoginDialog::UpdateCtrls()
{
	bool networkEnabled = IsNetworkPlayEnabled();
	bool loggedIn = GetLoggedIn();
	bool loggingIn = GetLoggingIn();
	bool loggedOut = GetLoggedOut();
	if (!networkEnabled && (loggedIn || loggingIn))
	{
		Logout();

		// these don't do anything.  need to set the vars in fMainDIalog?
		loggedIn = false;	
		loggingIn = false;
		loggedOut = true;
	}
}
#endif

bool plLoginDialog::IsNetworkPlayDisabled()
{
#ifdef PLASMA_EXTERNAL_RELEASE
	return false;
#else
	xtl::istring tmp = fLobbyVal.GetValue().c_str();
	return (tmp.compare("single player")==0);
#endif
}

bool plLoginDialog::IsNetworkPlayEnabled()
{
	return !IsNetworkPlayDisabled();
}

void plLoginDialog::OnInitDialog()
{
	plDialog::OnInitDialog();

	if ( fParentWnd )
		SetParent( Handle(), fParentWnd );

	fStatusBar.OpenWindow(this,true);

#ifdef PLASMA_EXTERNAL_RELEASE
	fLobbyList.Show(false);
	fLobbyText.Show(false);
#endif

#ifndef PLASMA_EXTERNAL_RELEASE
	fServerQueryBtn.Show(true);
#endif

	bool rememberPwd = (fRememberPasswordVal.GetValue()=="true");
	fAccountName.SetValue(fAccountNameVal.GetValue().c_str());
	if (rememberPwd)
	{
		int len = atoi(fPasswordLen.GetValue().c_str());
		std::string fakePwd(len, '*');
		fPassword.SetValue(fakePwd.c_str());
	}

	fRememberPassword.Check(rememberPwd);

	RefreshLobbyList();

	if ( fAutoLogin )
		fLoginBtn.Click();

//	SetForegroundWindow(*this);
}

bool plLoginDialog::Login()
{
	int ret = DoModal();
	if (ret<0)
	{
		hsAssert(false, xtl::format("plLoginDialog failed to initialize, err code %d, GetLastError %d", 
			ret, GetLastError()).c_str());
	}

	return (ret != 0);
}

void plLoginDialog::ILogin()
{
	OnLoginClicked();

	fAccountNameVal.SetValue(fAccountName.GetValue().c_str());
	
	std::string pwd = fPassword.GetValue();
	int pwdSize = pwd.size();

	std::string fakePwd = "*" + std::string(pwdSize-1, '*');
	if (pwd != fakePwd)	// user has entered a real pwd
	{
		fPasswordLen.SetValue(xtl::format("%d",pwd.size()).c_str());
		// MD5 HASH the pwd
		std::string hex;
		plChallengeResponse::HashPassword(pwd.c_str(), hex);
		fPasswordVal.SetValue(hex.c_str());
	}
	
	SetDataServerUserName(true, fAccountNameVal.GetValue().c_str());
	SetDataServerPassword(true, fPasswordVal.GetValue().c_str());
	SetDataServerUserName(false, fAccountNameVal.GetValue().c_str());
	SetDataServerPassword(false, fPasswordVal.GetValue().c_str());

	if (IsNetworkPlayEnabled())
		StartLogin();
	else
		CompleteLogin();
}

void plLoginDialog::IOnRememberPwdChanged()
{
	fRememberPasswordVal.SetValue(fRememberPassword.IsChecked() ? "true" : "false");
}

void plLoginDialog::IExit()
{
	fAccountNameVal.SetValue(fAccountName.GetValue().c_str());
	fPasswordVal.SetValue(fPassword.GetValue().c_str());
	fLobbyVal.SetValue(fLobbyList.GetValue().c_str());
	SetDataServerUserName(true, fAccountNameVal.GetValue().c_str());
	SetDataServerPassword(true, fPasswordVal.GetValue().c_str());
	SetDataServerUserName(false, fAccountNameVal.GetValue().c_str());
	SetDataServerPassword(false, fPasswordVal.GetValue().c_str());
	SHORT state = GetKeyState(VK_SHIFT);
	if (state&0x8000)
		EndDialogTrue();
	else
		EndDialogFalse();
	fCancelled=true;
}

int plLoginDialog::ICheckNetVersion(plNetMsgAuthenticateChallenge * msg)
{
	if (msg)
	{
		if (msg->GetVersionMajor() != plNetMessage::kVerMajor ||
			msg->GetVersionMinor() != plNetMessage::kVerMinor)
		{
			std::string str = xtl::format("Login Failed, client/server version mismatch, client %d.%d, server %d.%d",
				plNetMessage::kVerMajor, plNetMessage::kVerMinor, 
				msg->GetVersionMajor(),
				msg->GetVersionMinor());
			FailLogin(str.c_str());
			return hsFail;
		}
		return hsOK;
	}
	return hsFail;	
}

void plLoginDialog::HandleAuthChallenge(plNetMsgAuthenticateChallenge * msg)
{
	int cnt = msg->PeekBuffer(msg->GetNetCoreMsg()->GetData(),msg->GetNetCoreMsg()->GetLen());

	// check protocol version first, in case msg contents are hosed
	if (ICheckNetVersion(msg) == hsFail)
		return;		// version err

	if (msg->IsContinuing())
	{
		// Respond to the Challenge
		std::string hex = plChallengeResponse::GetBufferAsHexStr(msg->GetChallenge().data(), msg->GetChallenge().size(), true);
		fChallengeResponse.SetChallenge(hex);
		fChallengeResponse.GenerateResponse(fAccountNameVal.GetValue().c_str(),fPasswordVal.GetValue().c_str());
		KillTimer(*this,kAuthTimedOut);
		SendAuthenticateResponse();
	}
	else
	{
		FailLogin(msg->GetHelloResult());
	}
}

void plLoginDialog::HandleAccountAuthenticated(plNetMsgAccountAuthenticated * msg)
{
	int cnt = msg->PeekBuffer(msg->GetNetCoreMsg()->GetData(),msg->GetNetCoreMsg()->GetLen());
	if (msg->IsAuthenticated())
	{
		CompleteLogin();
	}
	else
	{
		FailLogin(msg->GetAuthResult());
	}
}


void plLoginDialog::StartLogin()
{
	fLoginBtn.SetEnabled(false);
	std::string value = fLobbyList.GetValue();
	if (value.length()==0)
	{
		fLobbyList.SetCurrent(0);
		fLobbyList.SetValue(MakeSafeLobbyServerName(fLobbyList.GetValue()).c_str());
		SelectedLobbyChanged();
	}
	fStatusBar.SetText(L"Authenticating...");
//	fMainDialog->InitNetCore();
//	fMainDialog->fLoginState = kLoggingIn;
//	fAccountTab.UpdateCtrls();
//	fPlayerTab.SetPlayerVault(nil);
	SendAuthenticateHello();
}

void plLoginDialog::CompleteLogin()
{
	if ( Handle() )
		fLoginBtn.SetEnabled(true);

	KillTimer(*this,kAuthTimedOut);

	fStatusBar.SetText(L"");
	if (IsNetworkPlayEnabled())
		NotifyConnected();
	else
		NotifyDisconnected();

	EndDialogTrue();
	
	GetClientManifests();
	UpdateAllCtrls();
}

void plLoginDialog::FailLogin(const char* str)
{
	fLoginBtn.SetEnabled(true);
	KillTimer(*this, kAuthTimedOut);
	fStatusBar.SetText(L"");
	hsMessageBoxWithOwner((void*)*this,str,"Error",hsMessageBoxNormal);
	Logout();
}

void plLoginDialog::FailLogin(int reasonCode)
{
	std::string str = xtl::format("Failed to login to lobby server %s: %s", 
		fLobbyVal.GetValue().c_str(), plNetMsgAccountAuthenticated::GetAuthResultString(reasonCode));
	FailLogin(str.c_str());
}

void plLoginDialog::TimeoutLogin()
{
	fLoginBtn.SetEnabled(true);

	wchar_t *wStr = hsStringToWString(xtl::format("Timed out logging into lobby server %s.", fLobbyVal.GetValue().c_str()).c_str());
	fStatusBar.SetText(wStr);
	delete [] wStr;

	KillTimer(*this, kAuthTimedOut);
	Logout();
}

void plLoginDialog::Logout()
{
	KillTimer(*this, kAuthTimedOut);
	SendLobbyLeave();
	//fMainDialog->ShutdownNetCore();
	NotifyDisconnected();
}

void plLoginDialog::SendLobbyLeave()
{
	plNetMsgLeave msg;
	msg.SetReason( plPlayerUpdateConstants::kPlayerQuitting );
	SendMsg(&msg,plNetAddress(fLobbyVal.GetValue().c_str(),plNetLobbyServerConstants::GetPort()));
	RemoveLobbyPeer();
}

#define MSG_TIMEOUT 8000
#include "../pnNetCommon/plNetAddress.h"

void plLoginDialog::SendAuthenticateHello()
{
	SetTimer(*this,kAuthTimedOut,MSG_TIMEOUT,nil);
	plNetMsgAuthenticateHello msg;
	msg.SetAccountName(fAccountNameVal.GetValue().c_str());
	msg.SetMaxPacketSize(GetPacketSize());
	SendMsg(&msg,plNetAddress(fLobbyVal.GetValue().c_str(),plNetLobbyServerConstants::GetPort()));
}

void plLoginDialog::SendAuthenticateResponse()
{
	SetTimer(*this,kAuthTimedOut,MSG_TIMEOUT,nil);
	plNetMsgAuthenticateResponse msg;
	msg.SetResponse(fChallengeResponse.GetResponse());
	SendMsg(&msg,plNetAddress(fLobbyVal.GetValue().c_str(),plNetLobbyServerConstants::GetPort()));
}

int plLoginDialog::CallDefaultProc( unsigned int message, unsigned int wParam, LONG lParam )
{
	switch (message)
	{
	case WM_TIMER:
		switch (wParam)
		{
		case kAuthTimedOut:
			TimeoutLogin();
			break;
		}
	}

	return 0;
}




/////////////////////////////////////////////////////////////////////////////
#ifndef PLASMA_EXTERNAL_RELEASE
#define kServerInfoFilename	"server_info.html"

static void StringToLines(std::string str, plStringList & lines, bool includeBlankLines=true)
{
	xtl::trim(str);
	if (str.length()==0)
		return;
	str.append("\n");
	int pos;
	while ((pos=str.find("\n"))!=std::string::npos)
	{
		std::string line = xtl::trim(str.substr(0,pos).c_str());
		str.erase(0,pos+1);
		if (includeBlankLines || (!includeBlankLines && line.length()>0))
			lines.push_back(line);
	}
}

static void GetPathElements(std::string filename, plStringList & lst)
{
	int pos;
	while ((pos=filename.find_first_of("\\/"))!=std::string::npos)
	{
		std::string element = filename.substr(0,pos);
		filename.erase(0,pos+1);
		if (element.length())
			lst.push_back(element);
	}
}

struct SDLInfoParser
{
	std::string fFilename;
	std::string fDescriptorName;
	int			fVersion;
	void ParseString( const char * s )
	{
		std::string str = s;
		int p = str.find(",");
		fFilename = str.substr(0,p);
		str.erase(0,p+1);
		p = str.find(",");
		fDescriptorName = str.substr(0,p);
		str.erase(0,p+1);
		fVersion = atoi(str.c_str());

		p = fFilename.find_last_of("\\");
		if( p!=std::string::npos )
			fFilename.erase(0,p+1);
		p = fFilename.find_last_of("/");
		if( p!=std::string::npos )
			fFilename.erase(0,p+1);
	}
};


struct DescriptorReport
{
	int			fServerVersion;		// 0 means the descriptor is missing
	int			fClientVersion;
	DescriptorReport(): fServerVersion(0),fClientVersion(0){}
};

#define kStyleSheet	\
	"<style>"	\
	"BODY {"	\
	"	font-size : 10pt;"	\
	"	font-family : Verdana, Geneva, Arial, Helvetica, sans-serif;"	\
	"}"	\
	"TD {"	\
	"	font-size : 10pt;"	\
	"	font-family : Verdana, Geneva, Arial, Helvetica, sans-serif;"	\
	"}"	\
	"PRE {"	\
	"	margin-top : 0px;"	\
	"}"	\
	".SDLFile  { background-color : #FFFF99; }"	\
	".Title {"	\
	"	font-weight : bold;"	\
	"	text-align : center;"	\
	"	text-decoration : underline;"	\
	"}"	\
	".SectionHeader {"	\
	"	margin-bottom : 0px;"	\
	"}"	\
	"</style>"

void plLoginDialog::ServerQueryBtnClicked()
{
	hsUNIXStream file;
	file.Open( kServerInfoFilename, "wt" );
	file.WriteString("<html>"kStyleSheet"<body>\n");

	try
	{
		typedef std::map< std::string, DescriptorReport > DescriptorReports;
		typedef std::map< std::string, DescriptorReports > FileReports;

		FileReports	fileReports;


		/*plURL	url;
		plHttpRequest request;
		plHttpResponse response;

		// read server build date etc.
		url.SetHost( fLobbyList.GetValue().c_str() );
		url.SetPort( 7676 );
		url.SetFile( "VersionInfo" );
		request.SetUrl( url );
		request.SetType( plHttpRequest::kGet );
		if ( !request.MakeRequest( response ) )
			throw 0;
		file.WriteString("<h3 class=SectionHeader>Server Info</h3>\n" );
		file.WriteString( "<pre>\n" );
		file.WriteString( response.c_str() );
		file.WriteString( "</pre>\n" );

		// get server's SDL info
		url.SetFile( "SDLInfo" );
		request.SetUrl( url );
		if ( !request.MakeRequest( response ) )
			throw 0;
		plStringList lines;
		StringToLines( response, lines, false );
		SDLInfoParser parser;
		{for ( plStringList::iterator ii=lines.begin(); ii!=lines.end(); ++ii )
		{
			parser.ParseString( (*ii).c_str() );
			fileReports[ parser.fFilename ][ parser.fDescriptorName ].fServerVersion = parser.fVersion;
		}}

		// get client's SDL info
		plSDLMgr::GetInstance()->DeInit();
		plSDLMgr::GetInstance()->SetSDLDir( "SDL" );
		plSDLMgr::GetInstance()->Init();
		const plSDL::DescriptorList * cds = plSDLMgr::GetInstance()->GetDescriptors();
		{for ( plSDL::DescriptorList::const_iterator ii=cds->begin(); ii!=cds->end(); ++ii )
		{
			plStateDescriptor * descriptor = *ii;
			std::string filename = descriptor->GetFilename();
			int p = filename.find_last_of(PATH_SEPARATOR_STR);
			if( p!=std::string::npos )
				filename.erase(0,p+1);
			fileReports[ filename ][ descriptor->GetName() ].fClientVersion = descriptor->GetVersion();
		}}

		// write SDL comparison report
		file.WriteString("<h3 class=SectionHeader>SDL File Comparison</h3>\n" );
		file.WriteString("Version=0 means descriptor doesn't exist.<br><br>\n" );
		file.WriteString( "<table><tr class=Title><td>File</td><td>Server Version</td><td>Client Version</td><td>Status</td></tr>\n" );


		{ for ( FileReports::iterator ii=fileReports.begin(); ii!=fileReports.end(); ++ii )
		{
			std::string	sdlFilename = ii->first;
			DescriptorReports & descrReports = ii->second;
			file.WriteFmt( "<tr><td colspan=5 class=SDLFile>%s</td></tr>\n", sdlFilename.c_str() );
			{ for ( DescriptorReports::iterator jj=descrReports.begin(); jj!=descrReports.end(); ++jj )
			{
#define kSDLBad	"<font color=red><b>Bad</b></font>"
#define kSDLOk	"<font color=green><b>Ok</b></font>"
				std::string descrName = jj->first;
				DescriptorReport & descrReport = jj->second;
				file.WriteFmt( "<tr><td>&nbsp;&nbsp;&nbsp;%s</td><td align=right>%d</td><td align=right>%d</td><td align=center>%s</td></tr>\n",
					descrName.c_str(), descrReport.fServerVersion, descrReport.fClientVersion,
					( descrReport.fServerVersion==descrReport.fClientVersion ) ? kSDLOk:kSDLBad );
			}}
		}}

		file.WriteString("</table>\n");*/
	}
	catch (...)
	{
		file.WriteString("<p>An error occurred while querying the server.\n");
	}

	file.WriteString("</body></html>\n");
	file.Close();

	ShellExecute( nil, nil, kServerInfoFilename, nil, nil, SW_SHOWNORMAL );
}
#endif
/////////////////////////////////////////////////////////////////////////////


