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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Uuid.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#if 0

COMPILER_ASSERT(sizeof(Uuid) >= sizeof(GUID));

void Uuid::Clear()
{
	UuidCreateNil( (GUID *)this );
}

int Uuid::CompareTo( const Uuid * v ) const
{
	RPC_STATUS s;
	return UuidCompare( (GUID *)this, (GUID *)v, &s );
}

bool Uuid::IsEqualTo( const Uuid * v ) const
{
	return ( CompareTo( v )==0 );
}

void Uuid::CopyFrom( const Uuid * v )
{
	memcpy( (void*)fData, (const void*)v->fData, sizeof(fData) );
}

bool Uuid::IsNull() const
{
	RPC_STATUS s;
	return 1 == UuidIsNil( (GUID *)this, &s );
}

bool Uuid::FromString( const char * str )
{
	Clear();
	if ( !str )
		return false;
	return RPC_S_OK == UuidFromString( (unsigned char *)str, (GUID *)this );
}

bool Uuid::ToString( std::string & out ) const
{
	out = "";
	unsigned char * ubuf;
	RPC_STATUS s;
	s = UuidToString( (GUID *) this, &ubuf );
	bool success = ( s==RPC_S_OK );
	if ( success )
		out = (char*)ubuf;
	RpcStringFree( &ubuf );
	return success;
}

// static
Uuid Uuid::Generate()
{
	Uuid result;
	UuidCreate( (GUID *)&result );
	return result;
}

#endif


#ifdef HS_BUILD_FOR_WIN32

/*****************************************************************************
*
*   Exports
*
***/

COMPILER_ASSERT(sizeof(Uuid) >= sizeof(GUID));

//============================================================================
Uuid GuidGenerate () {
	Uuid result;
	UuidCreate( (GUID *)&result );
	return result;
}

//============================================================================
void GuidClear (Uuid * uuid) {
	UuidCreateNil((GUID *)uuid);
}

//============================================================================
bool GuidFromString (const wchar str[], Uuid * uuid) {
    ASSERT(uuid);
    COMPILER_ASSERT(sizeof(wchar) == sizeof(unsigned short));
    return RPC_S_OK == UuidFromStringW((unsigned short *) str, (GUID *) uuid);
}

//============================================================================
bool GuidFromString (const char str[], Uuid * uuid) {
    ASSERT(uuid);
    return RPC_S_OK == UuidFromStringA((unsigned char *) str, (GUID *) uuid);
}

//============================================================================
int GuidCompare (const Uuid & a, const Uuid & b) {
	RPC_STATUS s;
	return UuidCompare((GUID *)&a, (GUID *)&b, &s);
}

//============================================================================
bool GuidIsNil (const Uuid & uuid) {
	RPC_STATUS s;
	return 1 == UuidIsNil((GUID *)&uuid, &s );
}

//============================================================================
const wchar * GuidToString (const Uuid & uuid, wchar * dst, unsigned chars) {
	wchar * src;
	RPC_STATUS s;
	s = UuidToStringW( (GUID *) &uuid, &src );
	if (RPC_S_OK == s)
        StrCopy(dst, src, chars);
    else
        StrCopy(dst, L"", chars);
	RpcStringFreeW(&src);
    return dst;
}

//============================================================================
const char * GuidToString (const Uuid & uuid, char * dst, unsigned chars) {
	byte * src;
	RPC_STATUS s;
	s = UuidToStringA( (GUID *) &uuid, &src );
	if (RPC_S_OK == s)
        StrCopy(dst, (char *)src, chars);
    else
        StrCopy(dst, "", chars);
	RpcStringFreeA(&src);
    return dst;
}

#endif // HS_BUILD_FOR_WIN32
