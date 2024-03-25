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
#include "plFile/plEncryptedStream.h"
#include "plProduct.h"
#include <string_theory/stdio>


void EncryptFiles(const plFileName& dir, const char* ext, bool encrypt);

void print_version() {
    puts(plProduct::ProductString().c_str());
    puts("");
}

void print_help() {
    puts("plFileEncrypt - Encrypts and Decrypts Uru Files.\n");
    print_version();
    puts("Usage: plFileEncrypt \t[(encrypt|-e)|(decrypt|-d|)|(--help|-h|-?|/h)|(-v)]");
    puts("\tencrypt|-e\t - Encrypts All .age, .fni, .ini, .csv, and .sdl files in the current folder.");
    puts("\tdecrypt|-d\t - Decrypts All .age, .fni, .ini, .csv, and .sdl files in the current folder.");
    puts("\t--help|-h|-?|/h\t - Prints Help. This Screen.");
    puts("\t-v|--version\t - Prints build version information");
}

int main(int argc, char *argv[])
{
    bool encrypt = true;
    const char* dir = ".";

#define ARGCMP(y) (strcmp(argv[1], y) == 0)
    if (argc > 1)
    {
        if (ARGCMP("encrypt") || ARGCMP("-e") )
        {
            if (argc > 2)
                dir = argv[2];
            encrypt = true;
        }
        else if (ARGCMP("decrypt") || ARGCMP("-d"))
        {
            if (argc > 2)
                dir = argv[2];
            encrypt = false;
        }
        else if(ARGCMP("--help") || ARGCMP("-h") || ARGCMP("-?")  || ARGCMP("/?"))
        {
            print_help();
            return 0;
        } 
        else if (ARGCMP("-v") || ARGCMP("--version"))
        {
            print_version();
            return 0;
        }
    }
#undef ARGCMP

    EncryptFiles(dir, "*.age", encrypt);
    EncryptFiles(dir, "*.fni", encrypt);
    EncryptFiles(dir, "*.ini", encrypt);
    EncryptFiles(dir, "*.sdl", encrypt);
    EncryptFiles(dir, "*.csv", encrypt);
    return 0;
}

void EncryptFiles(const plFileName& dir, const char* ext, bool encrypt)
{
    std::vector<plFileName> files = plFileSystem::ListDir(dir, ext);
    for (auto iter = files.begin(); iter != files.end(); ++iter)
    {
        if (encrypt)
        {
            ST::printf("encrypting: {}\n", iter->GetFileName());
            plEncryptedStream::FileEncrypt(*iter);
        }
        else
        {
            ST::printf("decrypting: {}\n", iter->GetFileName());
            plEncryptedStream::FileDecrypt(*iter);
        }
    }
}
