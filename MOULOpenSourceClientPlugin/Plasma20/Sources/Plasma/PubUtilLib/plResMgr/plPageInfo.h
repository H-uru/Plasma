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
//////////////////////////////////////////////////////////////////////////////
//
//	plPageInfo - Pack of info about an individual page
//

#ifndef _plPageInfo_h
#define _plPageInfo_h

#include "hsTypes.h"
#include "../pnKeyedObject/plUoid.h"
#include <vector>

class hsStream;
class plLocation;
class plPageInfo
{
public:
	struct ClassVersion { UInt16 Class; UInt16 Version; };
	typedef std::vector<ClassVersion> ClassVerVec;

protected:
	plLocation	fLocation;
	char*		fAge;
	char*		fPage;
	UInt16		fMajorVersion;
	ClassVerVec fClassVersions;
	UInt32		fChecksum;					
	UInt32		fDataStart, fIndexStart;

	void		IInit( void );
	void		ISetFrom( const plPageInfo &src );

public:

	plPageInfo();
	plPageInfo( const plLocation &loc );
	plPageInfo( const plPageInfo &src );
	virtual ~plPageInfo();

	const char* GetAge() const	{ return fAge; }
	const char* GetPage() const	{ return fPage; }

	plPageInfo &operator=( const plPageInfo &src );

	void				ClearClassVersions() { fClassVersions.clear(); }
	void				AddClassVersion(UInt16 classIdx, UInt16 version);
	const ClassVerVec&	GetClassVersions() const { return fClassVersions; }

	void	SetStrings( const char *age, const char *page );

	void				SetLocation(const plLocation& loc);
	const plLocation&	GetLocation() const;

	UInt16	GetMajorVersion() const { return fMajorVersion; }
	void	SetMajorVersion(UInt16 major) { fMajorVersion = major; }

	void	SetChecksum( UInt32 c ) { fChecksum = c; }
	UInt32	GetChecksum( void ) const { return fChecksum; }

	void	Read( hsStream *s );
	void	Write( hsStream *s );

	hsBool	IsValid( void ) const;

	UInt32	GetDataStart( void ) const { return fDataStart; }
	void	SetDataStart( UInt32 s ) { fDataStart = s; }

	UInt32	GetIndexStart( void ) const { return fIndexStart; }
	void	SetIndexStart( UInt32 s ) { fIndexStart = s; }
};
#endif // _plPageInfo_h
