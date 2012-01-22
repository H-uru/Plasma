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
/*****************************************************************************
*
*   plLogDecrypt - Used by mantis to decrypt log files
*
***/

#include <cstdio>
#include <cstring>
#include "HeadSpin.h"

#include "plStatusLog/plEncryptLogLine.h"

void IProcessFile(const char *path)
{
    char out_path[512];
    strcpy(out_path, path);
    strcat(out_path, ".decrypt");

    FILE * fpIn = fopen(path, "rb");
    FILE * fpOut = fopen(out_path, "w");

    if( fpIn != nil && fpOut != nil)
    {
        uint8_t line[ 2048 ];
        while( !feof( fpIn ) )
        {
            // Read next string
            long pos = ftell(fpIn);
            if( pos == -1L )
                break;
            uint8_t hint = (uint8_t)pos;
            uint16_t sizeHint = (uint16_t)pos;
            uint16_t size;

            if( stricmp( path + strlen( path ) - 4, ".log" ) == 0 )
            {
                int i;
                for( i = 0; i < 511; i++ )
                {
                    int c = fgetc( fpIn );
                    if( c == EOF || c == hint )
                        break;
                    line[ i ] = (uint8_t)c;
                }
                line[ i ] = 0;
                size = i;
            }
            else
            {
                // uint16_t line length is encoded first
                int c = fgetc( fpIn );
                if( c == EOF )
                    break;
                size = ( c & 0xff ) | ( fgetc( fpIn ) << 8 );

                size = size ^ sizeHint;
                
                if( size > sizeof( line ) )
                {
                    hsAssert( size <= sizeof( line ) - 1, "Invalid line size" );
                    break;
                }

                fread( line, 1, size, fpIn );
                line[ size ] = 0;
            }

            plStatusEncrypt::Decrypt( line, size, hint );
            fprintf(fpOut, "%s\n", (const char *)line);
        }
    }

    if (fpIn)
        fclose(fpIn);
    if (fpOut)
        fclose(fpOut);
}

int main(int argc, const char * argv[])
{
    if (argc == 2)
    {
        IProcessFile(argv[1]);
    }

    return 0;
}
