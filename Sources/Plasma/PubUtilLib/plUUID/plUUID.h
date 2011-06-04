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
#ifndef plUUID_h_inc
#define plUUID_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "../pnUtils/pnUtils.h"
#include "../pnFactory/plCreatable.h"

class hsStream;

class plUUID
{
    // must be first field in class
public:
	UInt8	fData[16];
	struct Match
	{
		const plUUID * fGuid;
		Match( const plUUID * guid ):fGuid( guid ){}
		bool operator()( const plUUID * guid ) const { return guid->IsEqualTo( fGuid );}
	};

	plUUID();
	plUUID( const char * s );
	plUUID( const plUUID & other );
    plUUID( const Uuid & uuid );
	void	Clear();
	bool	IsNull() const;
    bool    IsSet() const { return !IsNull(); }
	void	CopyFrom( const plUUID * v );
	void	CopyFrom( const plUUID & v );
	int		CompareTo( const plUUID * v ) const;
	bool	IsEqualTo( const plUUID * v ) const;
	bool	FromString( const char * str );
	bool	ToStdString( std::string & out ) const;
    inline std::string AsStdString() const { return AsString(); }
    const char * AsString() const; // returns static buffer
	void	Read( hsStream * s );
	void	Write( hsStream * s );
	operator std::string ( void ) const { return AsStdString();}
	bool	operator==( const plUUID & other ) const { return IsEqualTo( &other ); }
	bool	operator!=( const plUUID & other ) const { return !IsEqualTo( &other ); }
	int		operator <( const plUUID & other ) const { return CompareTo( &other ); }
    operator Uuid () const;

	static plUUID Generate();
};

class plCreatableUuid : public plUUID, public plCreatable {
public:
	CLASSNAME_REGISTER( plCreatableUuid );
	GETINTERFACE_ANY( plCreatableUuid, plCreatable );

	plCreatableUuid ();
	plCreatableUuid (const plCreatableUuid & other);
	plCreatableUuid (const plUUID & other);

	void	Read( hsStream * s, hsResMgr* ) { plUUID::Read(s); }
	void	Write( hsStream * s, hsResMgr* ) { plUUID::Write(s); }
};


#endif // plUUID_h_inc
