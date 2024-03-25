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

#include "hsResMgr.h"
#include "hsStream.h"
#include "pnNetCommon/plNetApp.h"

#include "plCCRMsg.h"
#include "plNetCommon/plNetCommon.h"

plCCRPetitionMsg::plCCRPetitionMsg()
    : plCCRMessage(), fPetitionType(plNetCommon::PetitionTypes::kGeneralHelp)
{
    SetBCastFlag(kBCastByType);
}

void plCCRPetitionMsg::Read(hsStream* stream, hsResMgr* mgr) 
{
    plMessage::IMsgRead(stream, mgr);
    
    plMsgStdStringHelper::Peek(fNote, stream);
    plMsgStdStringHelper::Peek(fTitle, stream);
    stream->ReadByte(&fPetitionType);
}

void plCCRPetitionMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
    plMessage::IMsgWrite(stream, mgr);

    plMsgStdStringHelper::Poke(fNote, stream);
    plMsgStdStringHelper::Poke(fTitle, stream);
    stream->WriteByte(fPetitionType);
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
    fType = (Type)stream->ReadLE32();
    stream->ReadLE32(&fCCRPlayerID);
}

void plCCRCommunicationMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
    plMessage::IMsgWrite(stream, mgr);
    
    plMsgStdStringHelper::Poke(fString, stream);
    stream->WriteLE32((uint32_t)fType);
    stream->WriteLE32(fCCRPlayerID);
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