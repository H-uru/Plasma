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
#include "plNetApp.h"
#include "pnMessage/plMessage.h"

plNetApp* plNetApp::fInstance = nullptr;
plNetObjectDebuggerBase* plNetObjectDebuggerBase::fInstance = nullptr;

//
// STATIC
//
plNetApp* plNetApp::GetInstance()
{
    return fInstance;
}

void plNetApp::SetInstance(plNetApp* app)
{
    fInstance = app;
}


bool plNetApp::StaticErrorMsg(const ST::string& msg)
{
    if (!GetInstance()) {
        return true;
    }

    return GetInstance()->ErrorMsg(msg);
}

bool plNetApp::StaticWarningMsg(const ST::string& msg)
{
    if (!GetInstance()) {
        return true;
    }

    return GetInstance()->WarningMsg(msg);
}

bool plNetApp::StaticAppMsg(const ST::string& msg)
{
    if (!GetInstance()) {
        return true;
    }

    return GetInstance()->AppMsg(msg);
}

bool plNetApp::StaticDebugMsg(const ST::string& msg)
{
    if (!GetInstance()) {
        return true;
    }

    return GetInstance()->DebugMsg(msg);
}


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

plNetClientApp::plNetClientApp() : fCCRLevel(0)
{

}

//
// STATIC FXN
// Inherit net cascade state.  Msg version
//
void plNetClientApp::InheritNetMsgFlags(const plMessage* parentMsg, plMessage* childMsg, bool startCascade)
{
    if (childMsg)
    {
        uint32_t childMsgFlags = childMsg->GetAllBCastFlags();
        InheritNetMsgFlags(parentMsg ? parentMsg->GetAllBCastFlags() : 0, &childMsgFlags, startCascade);
        childMsg->SetAllBCastFlags(childMsgFlags);
    }
}

//
// STATIC FXN
// Inherit net cascade state.  Flags version
// Set startCasCascade=true if called from outside the dispatcher, so that 
// the dispatcher won't mess with the flags when it goes to send out the msg.
//
void plNetClientApp::InheritNetMsgFlags(uint32_t parentMsgFlags, uint32_t* childMsgFlags, bool startCascade)
{
    if (!(*childMsgFlags & plMessage::kNetStartCascade))
    {
        if (parentMsgFlags & plMessage::kNetSent)
            *childMsgFlags |= plMessage::kNetSent;
        else
            *childMsgFlags &= ~plMessage::kNetSent;

        if (parentMsgFlags & plMessage::kNetNonLocal)
            *childMsgFlags |= plMessage::kNetNonLocal;      
        else
            *childMsgFlags &= ~plMessage::kNetNonLocal;     

        if (startCascade)
            *childMsgFlags |= plMessage::kNetStartCascade;
        else
            *childMsgFlags &= ~plMessage::kNetStartCascade;
    }
}

//
// STATIC
//
void plNetClientApp::UnInheritNetMsgFlags(plMessage* msg)
{
    msg->SetBCastFlag( plMessage::kCCRSendToAllPlayers, 0 );
    // if msg was propagated from another client...(and not originated on the server)
    if (msg && msg->HasBCastFlag(plMessage::kNetPropagate))
    {
        // This msg (and all it's responses) should not be resent, since we just recvd it.
        // Make sure it's marked for localPropagation now that it has arrived.
        // Also flag it as a remote (non-local) msg
        msg->SetBCastFlag(plMessage::kNetSent | 
            plMessage::kNetNonLocal | 
            plMessage::kLocalPropagate | 
            plMessage::kNetStartCascade);
        
        // clear the 'force' option, so it doesn't get sent out again
        msg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetNonDeterministic, 0);       
    }   
}
