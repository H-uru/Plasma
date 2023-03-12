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
#ifndef plNetServerSessionInfo_h_inc
#define plNetServerSessionInfo_h_inc

#include "HeadSpin.h"

#include <string_theory/string>

#include "plSpawnPointInfo.h"

#include "pnFactory/plCreatable.h"
#include "pnUUID/pnUUID.h"


class hsStream;
class hsResMgr;
class plVaultAgeInfoNode;
class plVaultAgeLinkNode;

///////////////////////////////////////////////////////////////////
// Holds info that describes an age

class plAgeInfoStruct : public plCreatable
{
    mutable uint8_t           fFlags;

    // Age dataset name "Neighborhood"
    ST::string      fAgeFilename;

    // Age string ID "Bevin"
    ST::string      fAgeInstanceName;

    // Age guid. Same as game server guid.
    plUUID  fAgeInstanceGuid;

    // User-defined age name: "My Teledahn"
    ST::string      fAgeUserDefinedName;

    // User-defined age description "This is Joe's Neighborhood"
    ST::string      fAgeDescription;

    // A modifier to user-defined name to make it unique in gui lists.
    // Assigned by vault server.
    int32_t           fAgeSequenceNumber;
    
    // The language of the client that created this age
    int32_t           fAgeLanguage;

    enum
    {
        kHasAgeFilename         = 1<<0,
        kHasAgeInstanceName     = 1<<1,
        kHasAgeInstanceGuid     = 1<<2,
        kHasAgeUserDefinedName  = 1<<3,
        kHasAgeSequenceNumber   = 1<<4,
        kHasAgeDescription      = 1<<5,
        kHasAgeLanguage         = 1<<6,
    };

    void    SetFlag( uint8_t bit, bool on=true ) const { (on)?fFlags|=bit:fFlags&=~bit;}
    void    ClearFlag( uint8_t bit ) { fFlags&=~bit;}
    bool    IsFlagSet( uint8_t bit ) const { return (fFlags&bit)!=0;}

public:
    plAgeInfoStruct()
    : fFlags( 0 )
    , fAgeSequenceNumber( 0 )
    , fAgeLanguage( -1 )
    {}

    CLASSNAME_REGISTER( plAgeInfoStruct );
    GETINTERFACE_ANY( plAgeInfoStruct, plCreatable );

    void    Clear();
    void    UpdateFlags() const;
    void    CopyFrom( const plAgeInfoStruct * other );
    void    CopyFrom( const plVaultAgeInfoNode * node );
    void    CopyFrom(const struct NetAgeInfo & info);
    bool    IsEqualTo( const plAgeInfoStruct * other ) const;

    ST::string  GetAgeFilename() const { return fAgeFilename; }
    ST::string  GetAgeInstanceName() const { return fAgeInstanceName; }
    const plUUID * GetAgeInstanceGuid() const { return &fAgeInstanceGuid; }
    ST::string  GetAgeUserDefinedName() const { return fAgeUserDefinedName; }
    ST::string  GetAgeDescription() const { return fAgeDescription; }
    uint32_t  GetAgeSequenceNumber() const { return fAgeSequenceNumber; }
    uint32_t  GetAgeLanguage() const { return fAgeLanguage; }

    void    SetAgeFilename( const ST::string & v );
    void    SetAgeInstanceName( const ST::string & v );
    void    SetAgeInstanceGuid( const plUUID * v );
    void    SetAgeUserDefinedName( const ST::string & v );
    void    SetAgeDescription( const ST::string & v );
    void    SetAgeSequenceNumber( uint32_t v );
    void    SetAgeLanguage( uint32_t v );

    bool    HasAgeFilename() const { return IsFlagSet( kHasAgeFilename ); }
    bool    HasAgeInstanceName() const { return IsFlagSet( kHasAgeInstanceName ); }
    bool    HasAgeInstanceGuid() const { return IsFlagSet( kHasAgeInstanceGuid ) && fAgeInstanceGuid.IsSet(); }
    bool    HasAgeUserDefinedName() const { return IsFlagSet( kHasAgeUserDefinedName ); }
    bool    HasAgeDescription() const { return IsFlagSet( kHasAgeDescription ); }
    bool    HasAgeSequenceNumber() const { return IsFlagSet( kHasAgeSequenceNumber ); }
    bool    HasAgeLanguage() const { return IsFlagSet( kHasAgeLanguage ); }

    void    Read(hsStream * s, hsResMgr*) override;
    void    Write(hsStream * s, hsResMgr*) override;

    ST::string AsString() const;
};

////////////////////////////////////////////////////////////////////

class plAgeLinkStruct : public plCreatable
{
    uint16_t              fFlags;

    // Where we want to link.
    plAgeInfoStruct     fAgeInfo;

    // The linking rule to use. See plNetCommon::LinkingRules
    int8_t                fLinkingRules;

    // Where to spawn avatar when we load the age specified in fAgeInfo
    plSpawnPointInfo    fSpawnPoint;

    // Override PLS/MCP load balancing rules for CCRs.
    uint8_t               fAmCCR;

    // If this is a child age link, who is the parent
    //    ...Age dataset name  like "Neighborhood"
    ST::string         fParentAgeFilename;

    enum
    {
        kHasAgeInfo         = 1<<0,
        kHasLinkingRules    = 1<<1,
        kHasSpawnPt_DEAD    = 1<<2,
        kHasSpawnPt_DEAD2   = 1<<3,
        kHasAmCCR           = 1<<4,
        kHasSpawnPt         = 1<<5,
        kHasParentAgeFilename   = 1<<6,
    };

    void    SetFlag( uint16_t bit, bool on=true ) { (on)?fFlags|=bit:fFlags&=~bit;}
    void    ClearFlag( uint16_t bit ) { fFlags&=~bit;}
    bool    IsFlagSet( uint16_t bit ) const { return (fFlags&bit)!=0;}

public:
    plAgeLinkStruct();
    CLASSNAME_REGISTER( plAgeLinkStruct );
    GETINTERFACE_ANY( plAgeLinkStruct, plCreatable );

    plAgeInfoStruct * GetAgeInfo() { return &fAgeInfo; }
    const plAgeInfoStruct * GetAgeInfo() const { return &fAgeInfo; }

    ST::string GetParentAgeFilename() const { return fParentAgeFilename; }
    void    SetParentAgeFilename(ST::string v);

    void    CopyFrom( const plAgeLinkStruct * other );
    void    CopyFrom( const plVaultAgeLinkNode * node );
    bool    IsEqualTo( const plAgeLinkStruct * other ) const;
    void    Clear();

    bool    HasAgeInfo() const { return IsFlagSet( kHasAgeInfo ); }
    bool    HasLinkingRules() const { return IsFlagSet( kHasLinkingRules ); }
    bool    HasSpawnPt() const { return IsFlagSet( kHasSpawnPt ); }
    bool    HasAmCCR() const { return IsFlagSet( kHasAmCCR ); }
    bool    HasParentAgeFilename() const { return IsFlagSet( kHasParentAgeFilename ); }

    void    SetLinkingRules( int v ) { SetFlag( kHasLinkingRules ); fLinkingRules=v; }
    int     GetLinkingRules() const { return fLinkingRules; }
    void    SetSpawnPoint( const plSpawnPointInfo & point ) { SetFlag( kHasSpawnPt ); fSpawnPoint=point; }
    plSpawnPointInfo & SpawnPoint() { return fSpawnPoint; }
    const plSpawnPointInfo & SpawnPoint() const { return fSpawnPoint; }
    void    SetAmCCR( bool v ) { SetFlag( kHasAmCCR ); fAmCCR=v?1:0; }
    bool    GetAmCCR() const { return fAmCCR!=0; }

    void    Read(hsStream * s, hsResMgr*) override;
    void    Write(hsStream * s, hsResMgr*) override;

    ST::string AsString() const;
};

#endif // plNetServerSessionInfo_h_inc
