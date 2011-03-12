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
#include "hsStream.h"
#include "plCCRMsg.h"

#include "../pnNetCommon/plNetApp.h"
#include "../plResMgr/plResManager.h"
#include "../plNetCommon/plNetCommon.h"

void plCCRPetitionMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgRead(stream, mgr);
	
	plMsgStdStringHelper::Peek(fNote, stream);
	plMsgStdStringHelper::Peek(fTitle, stream);
	stream->ReadSwap(&fPetitionType);
}

void plCCRPetitionMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgWrite(stream, mgr);

	plMsgStdStringHelper::Poke(fNote, stream);
	plMsgStdStringHelper::Poke(fTitle, stream);
	stream->WriteSwap(fPetitionType);
}

///////////////////////////////////////////////////////////

plCCRInvisibleMsg::plCCRInvisibleMsg() : fInvisLevel(0) 
{ 
	// send only to remote NetClientMgrs
	SetBCastFlag(kNetPropagate, true);
	SetBCastFlag(kLocalPropagate, false);
	AddReceiver(plNetApp::GetInstance()->GetKey());
}

void plCCRInvisibleMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgRead(stream, mgr);
	fAvKey=mgr->ReadKey(stream);
	fInvisLevel = stream->ReadByte();
}

void plCCRInvisibleMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgWrite(stream, mgr);
	mgr->WriteKey(stream, fAvKey);
	stream->WriteByte(fInvisLevel);
}

///////////////////////////////////////////////////////////

plCCRCommunicationMsg::plCCRCommunicationMsg() : fType(kUnInit), fCCRPlayerID(kInvalidPlayerID)
{ 
	SetBCastFlag(kBCastByType);
}

void plCCRCommunicationMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgRead(stream, mgr);
	plMsgStdStringHelper::Peek(fString, stream);	
	fType = (Type)stream->ReadSwap32();
	stream->ReadSwap(&fCCRPlayerID);
}

void plCCRCommunicationMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgWrite(stream, mgr);
	
	plMsgStdStringHelper::Poke(fString, stream);
	stream->WriteSwap32((int)fType);
	stream->WriteSwap(fCCRPlayerID);
}

///////////////////////////////////////////////////////////

plCCRBanLinkingMsg::plCCRBanLinkingMsg() : fBan(true) 
{ 
	AddReceiver(plNetApp::GetInstance()->GetKey());
}

void plCCRBanLinkingMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgRead(stream, mgr);
	fBan = stream->ReadBool();
}

void plCCRBanLinkingMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteBool(fBan);
}

///////////////////////////////////////////////////////////

plCCRSilencePlayerMsg::plCCRSilencePlayerMsg() : fSilence(true) 
{ 
	AddReceiver(plNetApp::GetInstance()->GetKey());
}

void plCCRSilencePlayerMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgRead(stream, mgr);
	fSilence = stream->ReadBool();
}

void plCCRSilencePlayerMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteBool(fSilence);
}