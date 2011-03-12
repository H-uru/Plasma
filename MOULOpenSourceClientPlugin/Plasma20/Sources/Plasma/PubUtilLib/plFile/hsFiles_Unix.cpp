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
#include "hsFiles.h"

#if HS_BUILD_FOR_UNIX

#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glob.h>
#include "hsTemplates.h"
#include "plFileUtils.h"
#include "hsStlUtils.h"

struct hsFolderIterator_Data {
	glob_t fGlobBuf;
	bool fInited;
	int fCnt;
	hsFolderIterator_Data() : fInited(false), fCnt(0) {}
	// ~hsFolderIterator_Data() { fInited=false; globfree(&fData->fGlobBuf); }
};

hsFolderIterator::hsFolderIterator(const char path[], bool)
{
	fData = TRACKED_NEW hsFolderIterator_Data;

	this->SetPath(path);
}

hsFolderIterator::~hsFolderIterator()
{
	this->Reset();
	delete fData;
}

void hsFolderIterator::SetPath(const char path[])
{
	fPath[0] = 0;
	if (path)
	{
		::strcpy(fPath, path);
	}
	this->Reset();
}

void hsFolderIterator::Reset()
{
	if (fData->fInited)
	{
		globfree(&fData->fGlobBuf);
		fData->fCnt = 0;
		fData->fInited=false;
	}
}
hsBool hsFolderIterator::NextFile()
{
	if (fData->fInited == false)
	{	
		std::string path=fPath;
		if(!(strchr(fPath,'*')  || strchr(fPath,'?')  || strchr(fPath,'[')))
		{
			if (fPath[strlen(fPath)-1] != PATH_SEPARATOR)
				path = path + PATH_SEPARATOR_STR + "*";
			else
				path = path + "*";
		}

		if(glob(path.c_str(), 0, NULL, &fData->fGlobBuf) != 0 ) {
			return false;
		}
		fData->fInited=true;
		fData->fCnt = 0;
	}

	return fData->fCnt++ < fData->fGlobBuf.gl_pathc;
}

const char* hsFolderIterator::GetFileName() const
{
	if (!fData->fInited || fData->fCnt > fData->fGlobBuf.gl_pathc)
		throw "end of folder";

	const char* fn=fData->fGlobBuf.gl_pathv[fData->fCnt-1];
	return plFileUtils::GetFileName(fn);
}

hsBool	hsFolderIterator::IsDirectory( void ) const
{
	// rob, please forgive me, this is my best attempt...
	if(fData->fCnt > fData->fGlobBuf.gl_pathc )
		return false;

	struct stat info;
	const char* fn=fData->fGlobBuf.gl_pathv[fData->fCnt-1];
	if( stat( fn, &info ) )
	{
		printf("Error calling stat(): %s errno=%d\n", strerror(errno), errno);
		return false;
	}
	return ( info.st_mode & S_IFDIR ) ? true : false;
}

#endif

