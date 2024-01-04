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

#ifndef plMessage_inc
#define plMessage_inc

#include <string>
#include <vector>

#include "pnFactory/plCreatable.h"
#include "pnKeyedObject/plKey.h"

class plKey;
class hsStream;

// Base class for messages only has enough info to route it
// and send it over the wire (Read/Write).
class plMessage : public plCreatable
{
public:
    typedef uint16_t  plStrLen;

    enum plBCastFlags {
        kBCastNone              = 0x0,
        kBCastByType            = 0x1,  // To everyone registered for this msg type or msgs this is derived from
        kBCastUNUSED_0          = 0x2,  // Obsolete option (never used). Was BCastBySender
        kPropagateToChildren    = 0x4,  // Propagate down through SceneObject heirarchy
        kBCastByExactType       = 0x8,  // To everyone registered for this exact msg type.
        kPropagateToModifiers   = 0x10, // Send the msg to an object and all its modifier
        kClearAfterBCast        = 0x20, // Clear registration for this type after sending this msg
        kNetPropagate           = 0x40, // Propagate this message over the network (remotely)
        kNetSent                = 0x80, // Internal use-This msg has been sent over the network 
        kNetUseRelevanceRegions = 0x100, // Used along with NetPropagate to filter the msg bcast using relevance regions
        kNetForce               = 0x200, // Used along with NetPropagate to force the msg to go out (ie. ignore cascading rules)
        kNetNonLocal            = 0x400, // Internal use-This msg came in over the network (remote msg)
        kLocalPropagate         = 0x800, // Propagate this message locally (ON BY DEFAULT)
        kNetNonDeterministic    = kNetForce, // This msg is a non-deterministic response to another msg 
        kMsgWatch               = 0x1000, // Debug only - will break in dispatch before sending this msg
        kNetStartCascade        = 0x2000, // Internal use-msg is non-local and initiates a cascade of net msgs. This bit is not inherited or computed, it's a property.
        kNetAllowInterAge       = 0x4000, // If rcvr is online but not in current age, they will receive msg courtesy of pls routing.
        kNetSendUnreliable      = 0x8000,  // Don't use reliable send when net propagating
        kCCRSendToAllPlayers    = 0x10000,  // CCRs can send a plMessage to all online players.
        kNetCreatedRemotely     = 0x20000,  // kNetSent and kNetNonLocal are inherited by child messages sent off while processing a net-propped
                                            // parent. This flag ONLY gets sent on the actual message that went across the wire.
    };

private:
    bool dispatchBreak;

    friend class plDispatch;
    friend class plDispatchLog;

private:
    plKey                   fSender;
    std::vector<plKey>      fReceivers;
    double                  fTimeStamp;

protected:
    uint32_t                  fBCastFlags;
    std::vector<uint32_t>*    fNetRcvrPlayerIDs;

    void IMsgRead(hsStream* stream, hsResMgr* mgr);     // default read implementation
    void IMsgWrite(hsStream* stream, hsResMgr* mgr);    // default write implementation

    void IMsgReadVersion(hsStream* stream, hsResMgr* mgr);
    void IMsgWriteVersion(hsStream* stream, hsResMgr* mgr);

public:
    plMessage();
    plMessage(const plKey &s,
                const plKey &r,
                const double* t);

    virtual ~plMessage();

    CLASSNAME_REGISTER(plMessage);
    GETINTERFACE_ANY(plMessage, plCreatable);

    // These must be implemented by all derived message classes (hence pure).
    // Derived classes should call the base-class default read/write implementation, 
    // so the derived Read() should call plMessage::IMsgRead().
    void Read(hsStream* stream, hsResMgr* mgr) override = 0;
    void Write(hsStream* stream, hsResMgr* mgr) override = 0;

    const plKey GetSender() const { return fSender; }
    plMessage&  SetSender(plKey s) { fSender = std::move(s); return *this; }

    size_t       GetNumReceivers() const;
    const plKey& GetReceiver(size_t i) const;
    plMessage&   RemoveReceiver(size_t i);

    plMessage& ClearReceivers();
    plMessage& AddReceiver(plKey r);
    plMessage& AddReceivers(const std::vector<plKey>& rList);

    bool Send(plKey r=nullptr, bool async=false); // Message will self-destruct after send.
    bool SendAndKeep(plKey r=nullptr, bool async=false); // Message won't self-destruct after send.

    double GetTimeStamp() const { return fTimeStamp; }
    plMessage& SetTimeStamp(double t) { fTimeStamp = t; return *this; }

    bool HasBCastFlag(uint32_t f) const { return 0 != (fBCastFlags & f); }
    plMessage& SetBCastFlag(uint32_t f, bool on=true) { if( on )fBCastFlags |= f; else fBCastFlags &= ~f; return *this; }

    void SetAllBCastFlags(uint32_t f) { fBCastFlags=f; }
    uint32_t GetAllBCastFlags() const { return fBCastFlags; }

    void AddNetReceiver( uint32_t plrID );
    void AddNetReceivers( const std::vector<uint32_t> & plrIDs );
    std::vector<uint32_t>* GetNetReceivers() const { return fNetRcvrPlayerIDs; }

    // just before dispatching this message, drop into debugger 
    void SetBreakBeforeDispatch (bool on) { dispatchBreak = on; }
    bool GetBreakBeforeDispatch () const { return dispatchBreak; }
};



/////////////////////////////////////////////////////////////////
// Helpers for reading/writing these types:
//      ST::string
//      std::string
//      c strings (char *)
//      c arrays (type [])


/////////////////////////////////////////////////////////////////
// reads/writes your std::string field

struct plMsgStdStringHelper
{
    static int Poke(const std::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int PokeBig(const std::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int Poke(const char * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions=0);
    static int PokeBig(const char * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions=0);
    static int Poke(const ST::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int PokeBig(const ST::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int Peek(std::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int PeekBig(std::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int Peek(ST::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
    static int PeekBig(ST::string & stringref, hsStream* stream, const uint32_t peekOptions=0);
};

/////////////////////////////////////////////////////////////////
// reads/writes your char * field  (deprecated)

struct plMsgCStringHelper
{
    static int Poke(const char * str, hsStream* stream, const uint32_t peekOptions=0);
    // deletes str and reallocates. you must delete [] str;
    static int Peek(char *& str, hsStream* stream, const uint32_t peekOptions=0);

    static int Poke(const ST::string & str, hsStream* stream, const uint32_t peekOptions=0);
    static int Peek(ST::string & str, hsStream* stream, const uint32_t peekOptions=0);
};

/////////////////////////////////////////////////////////////////
// reads/writes your type [] field
// don't use with uint8_t ordered types like int16_t,32.
// fine for int8_t, char, and IEEE formatted types like float, double.

struct plMsgCArrayHelper
{
    static int Poke(const void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions=0);
    static int Peek(void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions=0);
};





#endif // plMessage_inc
