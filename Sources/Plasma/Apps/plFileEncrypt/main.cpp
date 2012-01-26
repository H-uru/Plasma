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
#include "plFile/hsFiles.h"
#include "plFile/plEncryptedStream.h"
#include "pnProduct/pnProduct.h"


void EncryptFiles(const char* dir, const char* ext, bool encrypt);

void print_version(){
    wchar_t productString[256];
    ProductString(productString, arrsize(productString));
    printf("%S\n\n", productString);
}

void print_help() {
    printf("plFileEncrypt - Encrypts and Decrypts Uru Files.\n\n");
    print_version();
    printf("Usage: plFileEncrypt \t[(encrypt|-e)|(decrypt|-d|)|(--help|-h|-?|/h)|(-v)]\n");
    printf("\tencrypt|-e\t - Encrypts All .age, .fni, .ini, .csv, and .sdl files in the current folder.\n");
    printf("\tdecrypt|-d\t - Decrypts All .age, .fni, .ini, .csv, and .sdl files in the current folder.\n");
    printf("\t--help|-h|-?|/h\t - Prints Help. This Screen.\n");
    printf("\t-v|--version\t - Prints build version information\n");
}


int main(int argc, char *argv[])
{
    bool encrypt = true;
    const char* dir = ".";

    if (argc > 1)
    {
        if (hsStrEQ(argv[1], "encrypt") || hsStrEQ(argv[1], "-e") )
        {
            if (argc > 2)
                dir = argv[2];
            encrypt = true;
        }
        else if (hsStrEQ(argv[1], "decrypt") || hsStrEQ(argv[1], "-d"))
        {
            if (argc > 2)
                dir = argv[2];
            encrypt = false;
        }
        else if(hsStrEQ(argv[1], "--help") || hsStrEQ(argv[1], "-h") || hsStrEQ(argv[1], "-?")  || hsStrEQ(argv[1], "/?"))
        {
            print_help();
            return 0;
        } 
        else if (hsStrEQ(argv[1], "-v") || hsStrEQ(argv[1], "--version"))
        {
            print_version();            
            return 0;
        }
    }

    EncryptFiles(dir, ".age", encrypt);
    EncryptFiles(dir, ".fni", encrypt);
    EncryptFiles(dir, ".ini", encrypt);
    EncryptFiles(dir, ".sdl", encrypt);
    EncryptFiles(dir, ".csv", encrypt);
    return 0;
}

void EncryptFiles(const char* dir, const char* ext, bool encrypt)
{
    char filePath[256];

    hsFolderIterator folder(dir);
    while (folder.NextFileSuffix(ext))
    {
        folder.GetPathAndName(filePath);
        if (encrypt)
        {
            printf("encrypting: %s\n", folder.GetFileName());
            plEncryptedStream::FileEncrypt(filePath);
        }
        else
        { 
            printf("decrypting: %s\n", folder.GetFileName());
            plEncryptedStream::FileDecrypt(filePath);
        }
    }
}
