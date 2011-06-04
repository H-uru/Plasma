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
#ifndef plClientGuid_h_inc
#define plClientGuid_h_inc

#include "hsConfig.h"
#include "hsStlUtils.h"
#include "../pnFactory/plCreatable.h"
#include "../plUUID/plUUID.h"

class plClientGuid : public plCreatable
{
	UInt16	fFlags;
	plUUID	fAccountUUID;
	UInt32	fPlayerID;
	UInt8	fCCRLevel;
	bool	fProtectedLogin;
	UInt8	fBuildType;		// see plNetCommon.h
	std::string fPlayerName;
	UInt32	fSrcAddr;
	UInt16	fSrcPort;
	bool	fReserved;
	std::string fClientKey;

public:
	enum Flags // 16 bits.
	{
		kAccountUUID	= 1<<0,
		kPlayerID		= 1<<1,
		kTempPlayerID	= 1<<2,
		kCCRLevel		= 1<<3,
		kProtectedLogin = 1<<4,
		kBuildType		= 1<<5,
		kPlayerName		= 1<<6,
		kSrcAddr		= 1<<7,
		kSrcPort		= 1<<8,
		kReserved		= 1<<9,
		kClientKey	= 1<<10,
	};
	plClientGuid();
	CLASSNAME_REGISTER( plClientGuid );
	GETINTERFACE_ANY( plClientGuid, plCreatable );

	std::string AsStdString() const;
	std::string AsLogString() const;
	void	Clear();
	void	CopyFrom(const plClientGuid * other);
	void	UpdateFrom(const plClientGuid * other);
	bool	IsEqualTo(const plClientGuid * other) const;
	bool	IsFlagSet( UInt16 flag ) const { return (fFlags&flag)!=0; }
	bool	IsFullyQualified() const { return HasAccountUUID()&&HasPlayerID();}

	void	Read(hsStream * s, hsResMgr* =nil);
	void	Write(hsStream * s, hsResMgr* =nil);

	bool	HasAccountUUID() const { return (fFlags&kAccountUUID&&!fAccountUUID.IsNull())?true:false;}
	bool	HasPlayerID() const { return (fFlags&kPlayerID&&fPlayerID>0)?true:false;}
	bool	HasPlayerName() const { return (fFlags&kPlayerName&&fPlayerName.size())?true:false; }
	bool	HasCCRLevel() const { return (fFlags&kCCRLevel)?true:false;}
	bool	HasProtectedLogin() const { return (fFlags&kProtectedLogin)?true:false;}
	bool	HasBuildType() const { return (fFlags&kBuildType)?true:false;}
	bool	HasSrcAddr() const { return (fFlags&kSrcAddr)!=0;}
	bool	HasSrcPort() const { return (fFlags&kSrcPort)!=0;}
	bool	HasReservedBit() const { return (fFlags&kReserved)!=0;}
	bool	HasClientKey() const { return (fFlags&kClientKey)!=0;}
	
	const plUUID * GetAccountUUID() const { return &fAccountUUID;}
	UInt32	GetPlayerID() const { return fPlayerID;}
	const char * GetPlayerName() const { return fPlayerName.c_str(); }
	UInt8	GetCCRLevel() const { return fCCRLevel; }
	bool	GetProtectedLogin() const { return ( fProtectedLogin!=0 ); }
	UInt8	GetFlags() const { return (UInt8)fFlags;}
	UInt8	GetBuildType() const { return fBuildType;}
	UInt32	GetSrcAddr() const { return fSrcAddr; }
	const char * GetSrcAddrStr() const;
	UInt16	GetSrcPort() const { return fSrcPort; }
	bool	IsReserved() const { return fReserved!=0; }
	const std::string& GetClientKey() const { return fClientKey; }

	void	SetAccountUUID(const plUUID * v);
	void	SetAccountUUID(const plUUID & v);
	void	SetPlayerID(UInt32 v);
	void	SetPlayerName( const char * v );
	void	SetCCRLevel(UInt8 v);
	void	SetProtectedLogin(bool v);
	void	SetBuildType(UInt8 v);
	void	SetSrcAddr( UInt32 v );
	void	SetSrcAddrFromStr( const char * s );
	void	SetSrcPort( UInt16 v );
	void	SetReserved( bool v );
	void	SetClientKey( const std::string& key );
	// When a client hasn't selected a player yet,
	// we need to uniquely identify them in the lobby server.
	// We do this by stuffing a temp value into the fPlayerID
	// while keeping the kPlayerID flag cleared.
	void	SetTempPlayerID(UInt32 id);

	friend bool operator==(const plClientGuid & X, const plClientGuid & Y);
	friend bool operator!=(const plClientGuid & X, const plClientGuid & Y);
	friend bool operator<(const plClientGuid & X, const plClientGuid & Y);
};




#endif // plClientGuid_h_inc
