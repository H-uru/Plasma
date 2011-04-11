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
#include "../plFile/hsFiles.h"
#include "../plFile/plFileUtils.h"
#include "../plFile/plSecureStream.h"
#include "../pnUtils/pnUtils.h"
#include "../pnProduct/pnProduct.h"
#include "hsUtils.h"

#include <time.h>
#include <string>

void print_version() {
	wchar productString[256];
	ProductString(productString, arrsize(productString));
	printf("%S\n\n", productString);
}

void print_help() {
	printf("plFileSecure - Secures Uru files and generates encryption.key files.\n\n");
	print_version();
	printf("Usage:\n");
	printf("\tplFileSecure (<directory> <ext>)|[/generate /default]\n");
	printf("\n");
	printf("<directory> <ext>    : The directory and extension of files to secure. Cannot\n");
	printf("                       be used with /generate. Uses the %s file in\n", plFileUtils::kKeyFilename);
	printf("                       the current directory (or default key if no file exists)\n");
	printf("/generate            : Generates a random key and writes it to a %s\n", plFileUtils::kKeyFilename);
	printf("                       file in the current directory. Cannot be used with\n");
	printf("                       <directory> <ext>\n");
	printf("/default             : If used with /generate, creates a %s file\n", plFileUtils::kKeyFilename);
	printf("                       with the default key. If used with <directory> <ext>, it\n");
	printf("                       secures with the default key instead of the\n");
	printf("                       %s file's key\n", plFileUtils::kKeyFilename);
	printf("\n");
}

void GenerateKey(bool useDefault)
{
	UInt32 key[4];
	if (useDefault)
	{
		unsigned memSize = min(arrsize(key), arrsize(plSecureStream::kDefaultKey));
		memSize *= sizeof(UInt32);
		memcpy(key, plSecureStream::kDefaultKey, memSize);
	}
	else
	{
		srand((unsigned)time(nil));
		double randNum = (double)rand() / (double)RAND_MAX; // converts to 0..1
		UInt32 keyNum = (UInt32)(randNum * (double)0xFFFFFFFF); // multiply it by the max unsigned 32-bit int
		key[0] = keyNum;

		randNum = (double)rand() / (double)RAND_MAX;
		keyNum = (UInt32)(randNum * (double)0xFFFFFFFF);
		key[1] = keyNum;

		randNum = (double)rand() / (double)RAND_MAX;
		keyNum = (UInt32)(randNum * (double)0xFFFFFFFF);
		key[2] = keyNum;

		randNum = (double)rand() / (double)RAND_MAX;
		keyNum = (UInt32)(randNum * (double)0xFFFFFFFF);
		key[3] = keyNum;
	}

	hsUNIXStream out;
	out.Open(plFileUtils::kKeyFilename, "wb");
	out.Write(sizeof(UInt32) * arrsize(key), (void*)key);
	out.Close();
}

void SecureFiles(std::string dir, std::string ext, UInt32* key)
{
	char filePath[256];

	hsFolderIterator folder(dir.c_str());
	while (folder.NextFileSuffix(ext.c_str()))
	{
		folder.GetPathAndName(filePath);
		printf("securing: %s\n", folder.GetFileName());
		plSecureStream::FileEncrypt(filePath, key);
	}
}

int main(int argc, char *argv[])
{
	bool generatingKey = false;
	bool useDefault = false;
	std::string directory;
	std::string ext;

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
				if (directory == "")
					directory = argv[i];
				else if (ext == "")
					ext = argv[i];
				else
				{
					print_help();
					return 0;
				}
			}
		}

		if (generatingKey && ((directory != "") || (ext != "")))
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

	if (ext[0] != '.')
		ext = "." + ext; // tack on the dot if necessary

	if (useDefault)
		SecureFiles(directory, ext, nil);
	else
	{
		UInt32 key[4];
		plFileUtils::GetSecureEncryptionKey(plFileUtils::kKeyFilename, key, arrsize(key));
		SecureFiles(directory, ext, key);
	}
	return 0;
}
