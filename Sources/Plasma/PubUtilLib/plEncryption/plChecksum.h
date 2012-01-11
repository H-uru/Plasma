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
#ifndef PL_CHECKSUM_H
#define PL_CHECKSUM_H

#include "hsTypes.h"
#include <openssl/md5.h>

class plChecksum
{
public:
    typedef UInt32 SumStorage;
private:
    SumStorage fSum;
public:
    plChecksum(unsigned int bufsize, const char* buffer);
    static int GetChecksumSize() { return sizeof(SumStorage); }
    static int GetWindowSize() { return sizeof(SumStorage); }
    SumStorage GetChecksum() { return fSum; }
};

class hsStream;

class plMD5Checksum 
{
    protected:

        hsBool  fValid;
        MD5_CTX fContext;
        UInt8   fChecksum[ MD5_DIGEST_LENGTH ];

        UInt8   IHexCharToInt( char c ) const;

    public:

        plMD5Checksum( UInt32 size, UInt8 *buffer );
        plMD5Checksum();
        plMD5Checksum( const plMD5Checksum &rhs );
        plMD5Checksum( const char *fileName );
        plMD5Checksum( hsStream* stream );

        hsBool  IsValid( void ) const { return fValid; }
        void    Clear();

        void    CalcFromFile( const char *fileName );
        void    CalcFromStream( hsStream* stream );

        void    Start( void );
        void    AddTo( UInt32 size, const UInt8 *buffer );
        void    Finish( void );

        const UInt8 *GetValue( void ) const { return fChecksum; }
        UInt32      GetSize( void ) const { return sizeof( fChecksum ); }

        // Backdoor for cached checksums (ie, if you loaded it off disk)
        void SetValue(UInt8* checksum);

        // Note: GetAsHexString() returns a pointer to a static string; do not rely on the contents of this string between calls!
        const char  *GetAsHexString( void ) const;
        void        SetFromHexString( const char *string );

        bool    operator==( const plMD5Checksum &rhs ) const;
        bool    operator!=( const plMD5Checksum &rhs ) const { return !operator==( rhs ); }
};
#endif // PL_CHECKSUM_H

