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
#ifndef plServerGuid_h_inc
#define plServerGuid_h_inc

#include "hsConfig.h"
#include "hsWide.h"
#include "../pnFactory/plCreatable.h"
#include <string>

////////////////////////////////////////////////////////////////////
// plServerGuid

class plServerGuid : public plCreatable
{
public:
	enum { kGuidBytes = 8 };
	struct Match
	{
		const plServerGuid * fGuid;
		Match( const plServerGuid * guid ):fGuid( guid ){}
		bool operator()( const plServerGuid * guid ) const { return guid->IsEqualTo( fGuid );}
	};

	union
	{
		UInt8	N[kGuidBytes];
		hsWide	fWide;
	};
	plServerGuid();
	plServerGuid( const plServerGuid & other );
	explicit plServerGuid( const char * s );
	explicit plServerGuid( const hsWide & v );


	plServerGuid& operator=( const plServerGuid & rhs );
	friend bool operator==( const plServerGuid & X, const plServerGuid & Y );
	friend bool operator!=( const plServerGuid & X, const plServerGuid & Y );
	friend bool operator<( const plServerGuid & X, const plServerGuid & Y) ;

	const char *	AsString( void ) const;	// returns static buffer.
	std::string		AsStdString( void ) const;
	bool			FromString( const char * s );

	hsWide			AsWide() const;
	void			FromWide( const hsWide & v );

	bool			IsSet( void ) const;
	bool			IsEqualTo( const plServerGuid * other ) const;
	operator std::string ( void ) const { return AsString();}

	void			Read( hsStream * s, hsResMgr* mgr=nil );
	void			Write( hsStream * s, hsResMgr* mgr=nil );
	void			CopyFrom( const plServerGuid & other );
	void			CopyFrom( const plServerGuid * other );
	void			Clear();

	static void SetGuidSeed( UInt32 seed );
	static bool GuidSeedIsSet( void ) { return fGuidSeed!=0;}
	static plServerGuid	GenerateGuid( void );

	CLASSNAME_REGISTER( plServerGuid );
	GETINTERFACE_ANY( plServerGuid, plCreatable );

private:
	static UInt32	fGuidSeed;	// only low 24 bits are used
};


#endif //  plServerGuid_h_inc
////////////////////////////////////////////////////////////////////
// End.
