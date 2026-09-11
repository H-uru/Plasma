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
#ifndef plClientGuid_h_inc
#define plClientGuid_h_inc

#include "HeadSpin.h"
#include "pnFactory/plCreatable.h"
#include "pnUUID/pnUUID.h"
#include <string_theory/string>

class plClientGuid : public plCreatable
{
    uint16_t  fFlags;
    plUUID    fAccountUUID;
    uint32_t  fPlayerID;
    uint8_t   fCCRLevel;
    bool      fProtectedLogin;
    uint8_t   fBuildType;     // see plNetCommon.h
    ST::string  fPlayerName;
    uint32_t  fSrcAddr;
    uint16_t  fSrcPort;
    bool      fReserved;
    ST::string  fClientKey;

public:
    enum Flags // 16 bits.
    {
        kAccountUUID    = 1<<0,
        kPlayerID       = 1<<1,
        kTempPlayerID   = 1<<2,
        kCCRLevel       = 1<<3,
        kProtectedLogin = 1<<4,
        kBuildType      = 1<<5,
        kPlayerName     = 1<<6,
        kSrcAddr        = 1<<7,
        kSrcPort        = 1<<8,
        kReserved       = 1<<9,
        kClientKey  = 1<<10,
    };
    plClientGuid();
    CLASSNAME_REGISTER( plClientGuid );
    GETINTERFACE_ANY( plClientGuid, plCreatable );

    ST::string AsString() const;
    ST::string AsLogString() const;
    void    Clear();
    void    CopyFrom(const plClientGuid * other);
    void    UpdateFrom(const plClientGuid * other);
    bool    IsEqualTo(const plClientGuid * other) const;
    bool    IsFlagSet( uint16_t flag ) const { return (fFlags&flag)!=0; }
    bool    IsFullyQualified() const { return HasAccountUUID()&&HasPlayerID();}

    void    Read(hsStream * s, hsResMgr* = nullptr) override;
    void    Write(hsStream * s, hsResMgr* = nullptr) override;

    bool    HasAccountUUID() const { return (fFlags&kAccountUUID&&!fAccountUUID.IsNull())?true:false;}
    bool    HasPlayerID() const { return (fFlags&kPlayerID&&fPlayerID>0)?true:false;}
    bool    HasPlayerName() const { return (fFlags&kPlayerName&&!fPlayerName.empty()); }
    bool    HasCCRLevel() const { return (fFlags&kCCRLevel)?true:false;}
    bool    HasProtectedLogin() const { return (fFlags&kProtectedLogin)?true:false;}
    bool    HasBuildType() const { return (fFlags&kBuildType)?true:false;}
    bool    HasSrcAddr() const { return (fFlags&kSrcAddr)!=0;}
    bool    HasSrcPort() const { return (fFlags&kSrcPort)!=0;}
    bool    HasReservedBit() const { return (fFlags&kReserved)!=0;}
    bool    HasClientKey() const { return (fFlags&kClientKey)!=0;}
    
    const plUUID * GetAccountUUID() const { return &fAccountUUID;}
    uint32_t  GetPlayerID() const { return fPlayerID;}
    ST::string GetPlayerName() const { return fPlayerName; }
    uint8_t   GetCCRLevel() const { return fCCRLevel; }
    bool    GetProtectedLogin() const { return ( fProtectedLogin!=0 ); }
    uint8_t   GetFlags() const { return (uint8_t)fFlags;}
    uint8_t   GetBuildType() const { return fBuildType;}
    uint32_t  GetSrcAddr() const { return fSrcAddr; }
    const char * GetSrcAddrStr() const;
    uint16_t  GetSrcPort() const { return fSrcPort; }
    bool    IsReserved() const { return fReserved!=0; }
    ST::string GetClientKey() const { return fClientKey; }

    void    SetAccountUUID(const plUUID * v);
    void    SetAccountUUID(const plUUID & v);
    void    SetPlayerID(uint32_t v);
    void    SetPlayerName(ST::string v);
    void    SetCCRLevel(uint8_t v);
    void    SetProtectedLogin(bool v);
    void    SetBuildType(uint8_t v);
    void    SetSrcAddr( uint32_t v );
    void    SetSrcAddrFromStr( const char * s );
    void    SetSrcPort( uint16_t v );
    void    SetReserved( bool v );
    void    SetClientKey(ST::string key);
    // When a client hasn't selected a player yet,
    // we need to uniquely identify them in the lobby server.
    // We do this by stuffing a temp value into the fPlayerID
    // while keeping the kPlayerID flag cleared.
    void    SetTempPlayerID(uint32_t id);

    friend bool operator==(const plClientGuid & X, const plClientGuid & Y);
    friend bool operator!=(const plClientGuid & X, const plClientGuid & Y);
    friend bool operator<(const plClientGuid & X, const plClientGuid & Y);
};




#endif // plClientGuid_h_inc
