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
#ifndef plCCRMsg_h
#define plCCRMsg_h

#include <string_theory/string>
#include <utility>

#include "pnMessage/plMessage.h"
#include "plNetCommon/plNetCommon.h"

//
// Abstract Baseclass for CCR messages.
// All CCR messages must derive from this or the server won't be able to tell that's what they are.
//
class plCCRMessage : public plMessage
{
public:
    CLASSNAME_REGISTER( plCCRMessage);
    GETINTERFACE_ANY( plCCRMessage, plMessage );    
};


//
// Player wants to petition a CCR
//
class plCCRPetitionMsg : public plCCRMessage
{
private:
    uint8_t fPetitionType;
    ST::string fNote;
    ST::string fTitle;
public:
    plCCRPetitionMsg();
    ~plCCRPetitionMsg() {}

    CLASSNAME_REGISTER( plCCRPetitionMsg);
    GETINTERFACE_ANY( plCCRPetitionMsg, plCCRMessage ); 

    // petition text
    void SetNote(ST::string note) { fNote = std::move(note); }
    ST::string GetNote() const { return fNote; }

    // title
    void SetTitle(ST::string title) { fTitle = std::move(title); }
    ST::string GetTitle() const { return fTitle; }

    // petition type
    void SetType(const uint8_t t) { fPetitionType=t;  }
    uint8_t GetType() const { return fPetitionType;   }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

//
// A CCR has gone invisible
//
class plCCRInvisibleMsg : public plCCRMessage
{
public:
    plKey   fAvKey;
    uint8_t   fInvisLevel;     // 0 means visible

    plCCRInvisibleMsg();
    ~plCCRInvisibleMsg() {}

    CLASSNAME_REGISTER( plCCRInvisibleMsg);
    GETINTERFACE_ANY( plCCRInvisibleMsg, plCCRMessage );    

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

//
// For CCR-player communication
//
class plCCRCommunicationMsg : public plCCRMessage
{
public:
    enum Type
    {
        kUnInit = 0,
        kBeginCommunication,
        kChat,
        kEndCommunication,
        kReturnChatMsg
    };
    ST::string fString;
    Type fType;
    uint32_t fCCRPlayerID;

    plCCRCommunicationMsg();
    ~plCCRCommunicationMsg() {}

    CLASSNAME_REGISTER( plCCRCommunicationMsg);
    GETINTERFACE_ANY( plCCRCommunicationMsg, plCCRMessage );    

    // getters and setters
    void SetMessageText(ST::string message) { fString = std::move(message); }
    ST::string GetMessageText() const { return fString; }

    void SetType(Type t) { fType=t; }
    Type GetType() const { return fType; }
    
    void SetCCRPlayerID(uint32_t t) { fCCRPlayerID=t; }
    uint32_t GetCCRPlayerID() const { return fCCRPlayerID; }
    
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

//
// A CCR has banned a player from leaving the age
//
class plCCRBanLinkingMsg : public plCCRMessage
{
public:
    bool fBan;    // ban or un ban

    plCCRBanLinkingMsg() ;
    ~plCCRBanLinkingMsg() {}

    CLASSNAME_REGISTER( plCCRBanLinkingMsg);
    GETINTERFACE_ANY( plCCRBanLinkingMsg, plCCRMessage );   

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

//
// A CCR has silenced a player
//
class plCCRSilencePlayerMsg : public plCCRMessage
{
public:
    bool fSilence;    // ban or un ban

    plCCRSilencePlayerMsg() ;
    ~plCCRSilencePlayerMsg() {}

    CLASSNAME_REGISTER( plCCRSilencePlayerMsg);
    GETINTERFACE_ANY( plCCRSilencePlayerMsg, plCCRMessage );    

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

#endif  // plCCRMsg_h

