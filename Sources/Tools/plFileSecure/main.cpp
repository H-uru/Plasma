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
#include "plFile/plSecureStream.h"
#include "plProduct.h"


#include <ctime>
#include <string>
#include <algorithm>
#include <string_theory/stdio>

void print_version() {
    puts(plProduct::ProductString().c_str());
    puts("");
}

void print_help() {
    puts  ("plFileSecure - Secures Uru files and generates encryption.key files.\n");
    print_version();
    puts  ("Usage:");
    puts  ("\tplFileSecure (<directory> <ext>)|[/generate /default]");
    puts  ("");
    puts  ("<directory> <ext>    : The directory and extension of files to secure. Cannot");
    printf("                       be used with /generate. Uses the %s file in\n", plSecureStream::kKeyFilename);
    puts  ("                       the current directory (or default key if no file exists)");
    printf("/generate            : Generates a random key and writes it to a %s\n", plSecureStream::kKeyFilename);
    puts  ("                       file in the current directory. Cannot be used with");
    puts  ("                       <directory> <ext>");
    printf("/default             : If used with /generate, creates a %s file\n", plSecureStream::kKeyFilename);
    puts  ("                       with the default key. If used with <directory> <ext>, it");
    puts  ("                       secures with the default key instead of the");
    printf("                       %s file's key\n", plSecureStream::kKeyFilename);
    puts  ("");
}

void GenerateKey(bool useDefault)
{
    uint32_t key[4];
    if (useDefault)
    {
        unsigned memSize = std::min(std::size(key), std::size(plSecureStream::kDefaultKey));
        memSize *= sizeof(uint32_t);
        memcpy(key, plSecureStream::kDefaultKey, memSize);
    }
    else
    {
        srand((unsigned)time(nullptr));
        double randNum = (double)rand() / (double)RAND_MAX; // converts to 0..1
        uint32_t keyNum = (uint32_t)(randNum * (double)0xFFFFFFFF); // multiply it by the max unsigned 32-bit int
        key[0] = keyNum;

        randNum = (double)rand() / (double)RAND_MAX;
        keyNum = (uint32_t)(randNum * (double)0xFFFFFFFF);
        key[1] = keyNum;

        randNum = (double)rand() / (double)RAND_MAX;
        keyNum = (uint32_t)(randNum * (double)0xFFFFFFFF);
        key[2] = keyNum;

        randNum = (double)rand() / (double)RAND_MAX;
        keyNum = (uint32_t)(randNum * (double)0xFFFFFFFF);
        key[3] = keyNum;
    }

    hsUNIXStream out;
    out.Open(plSecureStream::kKeyFilename, "wb");
    out.Write(sizeof(uint32_t) * std::size(key), (void*)key);
}

void SecureFiles(const plFileName& dir, const ST::string& ext, uint32_t* key)
{
    std::vector<plFileName> files = plFileSystem::ListDir(dir, ext.c_str());
    for (auto iter = files.begin(); iter != files.end(); ++iter)
    {
        ST::printf("securing: {}\n", iter->GetFileName());
        plSecureStream::FileEncrypt(*iter, key);
    }
}

int main(int argc, char *argv[])
{
    bool generatingKey = false;
    bool useDefault = false;
    plFileName directory;
    ST::string ext;

    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];
            if ((arg[0] == '-')||(arg[0] == '/'))
            {
                // this arg is a flag of some kind
                arg = arg.substr(1, arg.length()); // trim the dash or slash
                if ((stricmp(arg.c_str(), "g") == 0) || (stricmp(arg.c_str(), "generate") == 0))
                {
                    if (!generatingKey)
                        generatingKey = true;
                    else
                    {
                        print_help();
                        return 0;
                    }
                }
                else if ((stricmp(arg.c_str(), "d") == 0) || (stricmp(arg.c_str(), "default") == 0))
                {
                    if (!useDefault)
                        useDefault = true;
                    else
                    {
                        print_help();
                        return 0;
                    }
                }
                else
                {
                    print_help();
                    return 0;
                }
            }
            else
            {
                // else it is a directory or extension
                if (!directory.IsValid())
                    directory = argv[i];
                else if (ext.empty())
                    ext = argv[i];
                else
                {
                    print_help();
                    return 0;
                }
            }
        }

        if (generatingKey && ((directory.IsValid()) || (!ext.empty())))
        {
            print_help();
            return 0;
        }
    }
    else
    {
        print_help();
        return 0;
    }

    if (generatingKey)
    {
        GenerateKey(useDefault);
        return 0;
    }

    // Make sure ext is a real pattern, or we won't find anything
    if (ext.front() == '.')
        ext = "*" + ext;
    else if (ext.front() != '*')
        ext = "*." + ext;

    if (useDefault)
        SecureFiles(directory, ext, nullptr);
    else
    {
        uint32_t key[4];
        plSecureStream::GetSecureEncryptionKey(plSecureStream::kKeyFilename, key, std::size(key));
        SecureFiles(directory, ext, key);
    }
    return 0;
}
