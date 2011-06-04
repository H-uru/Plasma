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
#include "plNetApp.h"
#include "hsStlUtils.h"
#include "../pnMessage/plMessage.h"

plNetApp* plNetApp::fInstance = nil;
plNetObjectDebuggerBase* plNetObjectDebuggerBase::fInstance=nil;

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

bool plNetApp::StaticErrorMsg(const char* fmt, ...)
{
	if ( !GetInstance() )
		return true;
	va_list args;
	va_start(args, fmt);
	return GetInstance()->ErrorMsgV(fmt, args);
}

bool plNetApp::StaticDebugMsg(const char* fmt, ...)
{
	if ( !GetInstance() )
		return true;
	va_list args;
	va_start(args, fmt);
	return GetInstance()->DebugMsgV(fmt, args);
}

bool plNetApp::StaticWarningMsg(const char* fmt, ...)
{
	if ( !GetInstance() )
		return true;
	va_list args;
	va_start(args, fmt);
	return GetInstance()->WarningMsgV(fmt, args);
}

bool plNetApp::StaticAppMsg(const char* fmt, ...)
{
	if ( !GetInstance() )
		return true;
	va_list args;
	va_start(args, fmt);
	return GetInstance()->AppMsgV(fmt, args);
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
		UInt32 childMsgFlags = childMsg->GetAllBCastFlags();
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
void plNetClientApp::InheritNetMsgFlags(UInt32 parentMsgFlags, UInt32* childMsgFlags, bool startCascade)
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