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
#include "plChecksum.h"
#include "hsStream.h"


plChecksum::plChecksum(unsigned int bufsize, const char* buffer)
{
	unsigned int wndsz = GetWindowSize(),i = 0;
	fSum = 0;

	const char* bufferAbsEnd = buffer + bufsize;
	const char* bufferEnvenEnd = buffer + bufsize - (bufsize % wndsz);

	while (buffer < bufferEnvenEnd)
	{
		fSum += hsSWAP32(*((SumStorage*)buffer));
		buffer += wndsz;
	}

	SumStorage last = 0;
	while (buffer < bufferAbsEnd)
	{
		((char*)&last)[i % wndsz] = *buffer;
		buffer++;
	}
	fSum+= hsSWAP32(last);
}

plMD5Checksum::plMD5Checksum( UInt32 size, UInt8 *buffer )
{
	fValid = false;
	Start();
	AddTo( size, buffer );
	Finish();
}

plMD5Checksum::plMD5Checksum()
{
	Clear();
}

plMD5Checksum::plMD5Checksum( const plMD5Checksum &rhs )
{
	memcpy( fChecksum, rhs.fChecksum, sizeof( fChecksum ) );
	fValid = rhs.fValid;
}

plMD5Checksum::plMD5Checksum( const char *fileName )
{
	CalcFromFile( fileName );
}

void plMD5Checksum::Clear()
{
	memset( fChecksum, 0, sizeof( fChecksum ) );
	fValid = false;
}

void	plMD5Checksum::CalcFromFile( const char *fileName)
{
	FILE *fp;
	fValid = false;
	
	if( fp = fopen(fileName, "rb" ) )
	{
		unsigned loadLen = 1024 * 1024;
		Start();

		UInt8 *buf = TRACKED_NEW UInt8[loadLen];
	
		while(int read = fread(buf, sizeof(UInt8), loadLen, fp))
			AddTo( read, buf );
		delete[] buf;

		Finish();
		fclose(fp);
	}
}

void	plMD5Checksum::Start( void )
{
	MD5_Init( &fContext );
	fValid = false;
}

void	plMD5Checksum::AddTo( UInt32 size, const UInt8 *buffer )
{
	MD5_Update( &fContext, buffer, size );
}

void	plMD5Checksum::Finish( void )
{
	MD5_Final( fChecksum, &fContext );
	fValid = true;
}

const char	*plMD5Checksum::GetAsHexString( void ) const
{
	const int	kHexStringSize = ( 2 * MD5_DIGEST_LENGTH ) + 1;
	static char	tempString[ kHexStringSize ];

	int		i;
	char	*ptr;


	hsAssert( fValid, "Trying to get string version of invalid checksum" );

	for( i = 0, ptr = tempString; i < sizeof( fChecksum ); i++, ptr += 2 )
		sprintf( ptr, "%02x", fChecksum[ i ] );

	*ptr = 0;

	return tempString;
}

UInt8	plMD5Checksum::IHexCharToInt( char c ) const
{
	switch( c )
	{
		// yes, it's ugly, but it'll be fast :)
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;

		case 'a': return 10;
		case 'b': return 11;
		case 'c': return 12;
		case 'd': return 13;
		case 'e': return 14;
		case 'f': return 15;

		case 'A': return 10;
		case 'B': return 11;
		case 'C': return 12;
		case 'D': return 13;
		case 'E': return 14;
		case 'F': return 15;
	}

	return 0xff;
}

void		plMD5Checksum::SetFromHexString( const char *string )
{
	const char	*ptr;
	int			i;


	hsAssert( strlen( string ) == 2 * MD5_DIGEST_LENGTH, "Invalid string in MD5Checksum Set()" );

	for( i = 0, ptr = string; i < sizeof( fChecksum ); i++, ptr += 2 )
		fChecksum[ i ] = ( IHexCharToInt( ptr[ 0 ] ) << 4 ) | IHexCharToInt( ptr[ 1 ] );

	fValid = true;
}

void plMD5Checksum::SetValue(UInt8* checksum)
{
	fValid = true;
	memcpy(fChecksum, checksum, sizeof(fChecksum));
}

bool	plMD5Checksum::operator==( const plMD5Checksum &rhs ) const
{
	return (fValid && rhs.fValid && memcmp(fChecksum, rhs.fChecksum, sizeof(fChecksum)) == 0);
}
