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
#ifndef plLinkToAgeMsg_INC
#define plLinkToAgeMsg_INC

#include "pnMessage/plMessageWithCallbacks.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetCommon/plNetCommonHelpers.h"


////////////////////////////////////////////////////////////////////
// A msg which is sent to the networking system to cause the player to link
// to a new age.
//
class plKey;
class hsStream;
class hsResMgr;

class plLinkToAgeMsg : public plMessage
{
    plAgeLinkStruct     fAgeLink;
    char* fLinkInAnimName;

public:
    plLinkToAgeMsg();
    plLinkToAgeMsg( const plAgeLinkStruct * link );
    ~plLinkToAgeMsg();

    CLASSNAME_REGISTER( plLinkToAgeMsg );
    GETINTERFACE_ANY( plLinkToAgeMsg, plMessage );

    plAgeLinkStruct * GetAgeLink() { return &fAgeLink; }
    const plAgeLinkStruct * GetAgeLink() const { return &fAgeLink; }

    const char * GetLinkInAnimName() { return fLinkInAnimName; }
    void SetLinkInAnimName(const char* name) { delete [] fLinkInAnimName; fLinkInAnimName = hsStrcpy(name); }

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
    
    // WriteVersion writes the current version of this creatable and ReadVersion will read in
    // any previous version.
    virtual void ReadVersion(hsStream* s, hsResMgr* mgr);
    virtual void WriteVersion(hsStream* s, hsResMgr* mgr);
};

////////////////////////////////////////////////////////////////////
// For other things the linking mgr does besides linking players to ages.
// See plNetLinkingMgr::Cmds for a list of these.

class plLinkingMgrMsg : public plMessage
{
    uint8_t                   fLinkingMgrCmd;
    plCreatableListHelper   fArgs;

public:
    plLinkingMgrMsg();
    ~plLinkingMgrMsg();

    CLASSNAME_REGISTER( plLinkingMgrMsg );
    GETINTERFACE_ANY( plLinkingMgrMsg, plMessage );

    uint8_t   GetCmd() const { return fLinkingMgrCmd; }
    void    SetCmd( uint8_t v ) { fLinkingMgrCmd=v; }
    plCreatableListHelper * GetArgs() { return &fArgs; }

    void    Read( hsStream* stream, hsResMgr* mgr );
    void    Write( hsStream* stream, hsResMgr* mgr );

    void    ReadVersion( hsStream* s, hsResMgr* mgr );
    void    WriteVersion( hsStream* s, hsResMgr* mgr );
};


////////////////////////////////////////////////////////////////////

class plLinkEffectsTriggerMsg : public plMessage
{
protected:
    hsBool fLeavingAge;
    plKey fLinkKey;
    plKey fLinkInAnimKey;
    int fInvisLevel;
public:
    plLinkEffectsTriggerMsg() : fLeavingAge(true), fLinkKey(nil), fLinkInAnimKey(nil), fEffects(0), fInvisLevel(0) {  }
    ~plLinkEffectsTriggerMsg();

    CLASSNAME_REGISTER( plLinkEffectsTriggerMsg );
    GETINTERFACE_ANY( plLinkEffectsTriggerMsg, plMessage ); 

    void    SetInvisLevel(int invisLevel) { fInvisLevel=invisLevel; }
    int     GetInvisLevel() { return fInvisLevel; }

    void    SetLeavingAge(hsBool leaving) { fLeavingAge = leaving; }
    hsBool  IsLeavingAge() { return fLeavingAge; }

    void    SetLinkKey(plKey &key);
    const plKey GetLinkKey() const { return fLinkKey; }

    void    SetLinkInAnimKey(plKey &key);
    plKey   GetLinkInAnimKey() { return fLinkInAnimKey; }
    
    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);

    void ReadVersion(hsStream* s, hsResMgr* mgr);
    void WriteVersion(hsStream* s, hsResMgr* mgr);

    int32_t fEffects;
};

////////////////////////////////////////////////////////////////////

class plLinkEffectsTriggerPrepMsg : public plMessage
{
protected:
    plLinkEffectsTriggerMsg *fTrigger;

public:
    hsBool fLeavingAge;
    plKey fLinkKey; 

    plLinkEffectsTriggerPrepMsg() : fLeavingAge(false), fLinkKey(nil), fTrigger(nil) {  }
    ~plLinkEffectsTriggerPrepMsg();
    
    CLASSNAME_REGISTER( plLinkEffectsTriggerPrepMsg );
    GETINTERFACE_ANY( plLinkEffectsTriggerPrepMsg, plMessage ); 
        
    void    SetTrigger(plLinkEffectsTriggerMsg *msg);
    plLinkEffectsTriggerMsg *GetTrigger() { return fTrigger; }
    
    // runtime-local only, no I/O
    void Read(hsStream* stream, hsResMgr* mgr) {}
    void Write(hsStream* stream, hsResMgr* mgr) {}
};

////////////////////////////////////////////////////////////////////


class plLinkEffectBCMsg : public plMessage
{
protected:
    uint32_t fLinkFlags;

public:
    enum // link flags
    {
        kLeavingAge     = 0x1,
        kSendCallback   = 0x2,
        kMute           = 0x4       // don't play any sound
    };

    plLinkEffectBCMsg();
    ~plLinkEffectBCMsg() { }

    CLASSNAME_REGISTER( plLinkEffectBCMsg );
    GETINTERFACE_ANY( plLinkEffectBCMsg, plMessage );   

    // These messages should never be sent over the net, so I'm
    // suspicious that R/W actually does something... but if
    // it isn't broken, I'm not going to touch it.
    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);

    void SetLinkFlag(uint32_t flag, hsBool on = true);
    hsBool HasLinkFlag(uint32_t flag);

    plKey fLinkKey;
};

////////////////////////////////////////////////////////////////////

class plLinkEffectPrepBCMsg : public plMessage
{
public:
    hsBool fLeavingAge;
    plKey fLinkKey;
    
    plLinkEffectPrepBCMsg();
    ~plLinkEffectPrepBCMsg() { }
    
    CLASSNAME_REGISTER( plLinkEffectPrepBCMsg );
    GETINTERFACE_ANY( plLinkEffectPrepBCMsg, plMessage );   
    
    // runtime-local only, no I/O
    void Read(hsStream* stream, hsResMgr* mgr) {}
    void Write(hsStream* stream, hsResMgr* mgr) {}
};

////////////////////////////////////////////////////////////////////

class plLinkCallbackMsg : public plEventCallbackMsg
{
public:
    plLinkCallbackMsg() { }
    ~plLinkCallbackMsg() { } 

    CLASSNAME_REGISTER( plLinkCallbackMsg );
    GETINTERFACE_ANY( plLinkCallbackMsg, plEventCallbackMsg );

    void Read(hsStream* stream, hsResMgr* mgr); 
    void Write(hsStream* stream, hsResMgr* mgr);

    plKey fLinkKey;
};

////////////////////////////////////////////////////////////////////

class plPseudoLinkEffectMsg : public plMessage
{
public:
    plKey fLinkObjKey;
    plKey fAvatarKey;

    plPseudoLinkEffectMsg();
    ~plPseudoLinkEffectMsg() {}

    CLASSNAME_REGISTER(plPseudoLinkEffectMsg);
    GETINTERFACE_ANY(plPseudoLinkEffectMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);

};

class plPseudoLinkAnimTriggerMsg : public plMessage
{
public:

    plPseudoLinkAnimTriggerMsg();
    plPseudoLinkAnimTriggerMsg(hsBool forward,  plKey avatarKey);
    ~plPseudoLinkAnimTriggerMsg() {}

    CLASSNAME_REGISTER(plPseudoLinkAnimTriggerMsg);
    GETINTERFACE_ANY(plPseudoLinkAnimTriggerMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);

    plKey fAvatarKey;
    hsBool fForward;

};

class plPseudoLinkAnimCallbackMsg : public plMessage
{
public:

    plPseudoLinkAnimCallbackMsg() {;}
    ~plPseudoLinkAnimCallbackMsg() {;}

    CLASSNAME_REGISTER(plPseudoLinkAnimCallbackMsg);
    GETINTERFACE_ANY(plPseudoLinkAnimCallbackMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);

    plKey fAvatarKey;
};
#endif // plLinkToAgeMsg_INC
