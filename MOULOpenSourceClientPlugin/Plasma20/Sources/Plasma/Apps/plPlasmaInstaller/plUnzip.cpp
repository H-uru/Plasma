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
#include "HeadSpin.h"
#include "plUnzip.h"
#include "hsTypes.h"
#include "hsWindows.h"
#include "hsStream.h"

plUnzip::plUnzip() : fFile(nil)
{
}

bool plUnzip::Open(const char* filename)
{
	fFile = unzOpen(filename);
	return (fFile != nil);
}

bool plUnzip::Close()
{
	bool ret = false;

	if (fFile != nil)
	{
		ret = (UNZ_OK == unzClose(fFile));
		fFile = nil;
	}

	return ret;
}

void plUnzip::IGetFullPath(const char* destDir, const char* filename, char* outFilename)
{
	// Make sure the dest ends with a slash
	strcpy(outFilename, destDir);
	char lastChar = outFilename[strlen(outFilename)-1];
	if (lastChar != '\\' && lastChar != '/')
		strcat(outFilename, "\\");

	// Check if the output filename has any directories in it
	const char* forward = strrchr(filename, '/');
	const char* backward = strrchr(filename, '\\');

	if (!forward && !backward)
	{
		CreateDirectory(outFilename, NULL);
		strcat(outFilename, filename);
	}
	else
	{
		const char* fileOnly = (forward > backward) ? forward+1 : backward+1;
		strncat(outFilename, filename, fileOnly-filename);
		CreateDirectory(outFilename, NULL);

		strcat(outFilename, fileOnly);
	}
}

void plUnzip::IExtractCurrent(const char* destDir, char* fileName)
{
	char filename[MAX_PATH];
	if (unzGetCurrentFileInfo(fFile, nil, filename, sizeof(filename), nil, 0, nil, 0) == UNZ_OK)
	{
		strcpy(fileName, filename);

		if (unzOpenCurrentFile(fFile) == UNZ_OK)
		{
			char outFilename[MAX_PATH];
			IGetFullPath(destDir, filename, outFilename);

			// Make sure to take off the read-only flag if the file exists, and is RO
			DWORD attrs = GetFileAttributes(outFilename);
			if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY))
				SetFileAttributes(outFilename, attrs & ~FILE_ATTRIBUTE_READONLY);

			hsUNIXStream outFile;
			if (outFile.Open(outFilename, "wb"))
			{
				char buf[2048];
				int numRead;
				while ((numRead = unzReadCurrentFile(fFile, buf, sizeof(buf))) > 0)
				{
					outFile.Write(numRead, buf);
				}

				outFile.Close();

				unz_file_info_s info;
				unzGetCurrentFileInfo(fFile, &info, NULL, 0, NULL, 0, NULL, 0);

				SYSTEMTIME sysTime = {0};
				sysTime.wDay	= info.tmu_date.tm_mday;
				sysTime.wMonth	= info.tmu_date.tm_mon+1;
				sysTime.wYear	= info.tmu_date.tm_year;
				sysTime.wHour	= info.tmu_date.tm_hour;
				sysTime.wMinute	= info.tmu_date.tm_min;
				sysTime.wSecond = info.tmu_date.tm_sec;

				FILETIME localFileTime, utcFileTime;
				SystemTimeToFileTime(&sysTime, &localFileTime);

				LocalFileTimeToFileTime(&localFileTime, &utcFileTime);

				HANDLE hFile = CreateFile(outFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
				SetFileTime(hFile, NULL, NULL, &utcFileTime);
				CloseHandle(hFile);
			}

			unzCloseCurrentFile(fFile);
		}
	}
}

void plUnzip::ExtractAll(const char* destDir)
{
	if (unzGoToFirstFile(fFile) != UNZ_OK)
		return;

	do
	{
		IExtractCurrent(destDir);
	}
	while (unzGoToNextFile(fFile) == UNZ_OK);
}

bool plUnzip::ExtractNext(const char* destDir, char* fileName)
{
	IExtractCurrent(destDir, fileName);
	return (unzGoToNextFile(fFile) == UNZ_OK);
}
