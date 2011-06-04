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
#include "hsTypes.h"
#include "hsStlUtils.h"
#include "plPythonPack.h"
#include "hsStream.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plSecureStream.h"
#include "../plFile/plStreamSource.h"
#include "hsStlSortUtils.h"
#include "marshal.h"

static const char* kPackFilePath = ".\\Python\\";

struct plPackOffsetInfo
{
	UInt32 fOffset;
	UInt32 fStreamIndex; // index of the stream object in the plPythonPack object that the file resides in
};

class plPythonPack
{
protected:
	std::vector<hsStream*> fPackStreams;
	bool fPackNotFound;		// No pack file, don't keep trying

	typedef std::map<std::string, plPackOffsetInfo> FileOffset;
	FileOffset fFileOffsets;

	plPythonPack();

public:
	~plPythonPack();

	static plPythonPack& Instance();

	bool Open();
	void Close();

	PyObject* OpenPacked(const char *fileName);
	hsBool IsPackedFile(const char* fileName);
};

PyObject* PythonPack::OpenPythonPacked(const char* fileName)
{
	return plPythonPack::Instance().OpenPacked(fileName);
}

hsBool PythonPack::IsItPythonPacked(const char* fileName)
{
	return plPythonPack::Instance().IsPackedFile(fileName);
}

plPythonPack::plPythonPack() : fPackNotFound(false)
{
}

plPythonPack::~plPythonPack()
{
	Close();
}

plPythonPack& plPythonPack::Instance()
{
	static plPythonPack theInstance;
	return theInstance;
}

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

bool plPythonPack::Open()
{
	if (fPackStreams.size() > 0)
		return true;
	
	// We already tried and it wasn't there
	if (fPackNotFound)
		return false;

	fPackNotFound = true;

	// Get the names of all the pak files
	std::vector<std::wstring> files = plStreamSource::GetInstance()->GetListOfNames(L"python", L".pak");

	std::vector<time_t> modTimes; // the modification time for each of the streams (to resolve duplicate file issues)

	// grab all the .pak files in the folder
	for (int curName = 0; curName < files.size(); curName++)
	{
		// obtain the stream
		hsStream *fPackStream = plStreamSource::GetInstance()->GetFile(files[curName]);
		if (fPackStream)
		{
			fPackStream->Rewind(); // make sure we're at the beginning of the file
			fPackNotFound = false;

			char* tempFilename = hsWStringToString(files[curName].c_str());
			struct stat buf;
			time_t curModTime = 0;
			if (stat(tempFilename,&buf)==0)
				curModTime = buf.st_mtime;
			modTimes.push_back(curModTime);
			delete [] tempFilename;

			// read the index data
			int numFiles = fPackStream->ReadSwap32();
			UInt32 streamIndex = (UInt32)(fPackStreams.size());
			for (int i = 0; i < numFiles; i++)
			{
				// and pack the index into our own data structure
				char* buf = fPackStream->ReadSafeString();
				std::string pythonName = buf; // reading a "string" from a hsStream directly into a stl string causes memory loss
				delete [] buf;
				UInt32 offset = fPackStream->ReadSwap32();

				plPackOffsetInfo offsetInfo;
				offsetInfo.fOffset = offset;
				offsetInfo.fStreamIndex = streamIndex;

				if (fFileOffsets.find(pythonName) != fFileOffsets.end())
				{
					UInt32 index = fFileOffsets[pythonName].fStreamIndex;
					if (modTimes[index] < curModTime) // is the existing file older then the new one?
						fFileOffsets[pythonName] = offsetInfo; // yup, so replace it with the new info
				}
				else
					fFileOffsets[pythonName] = offsetInfo; // no conflicts, add the info
			}
			fPackStreams.push_back(fPackStream);
		}
	}

	return !fPackNotFound;
}

void plPythonPack::Close()
{
	if (fPackStreams.size() == 0)
		return;
	
	int i;
	for (i=0; i<fPackStreams.size(); i++)
	{
		hsStream* fPackStream = fPackStreams[i];

		// do NOT close or delete the streams, the preloader will do that for us
		fPackStreams[i] = nil;
	}
	
	fPackStreams.clear();

	fFileOffsets.clear();
}

PyObject* plPythonPack::OpenPacked(const char* fileName)
{
	if (!Open())
		return nil;

	std::string pythonName = fileName;
	pythonName += ".py";

	FileOffset::iterator it = fFileOffsets.find(pythonName);
	if (it != fFileOffsets.end())
	{
		plPackOffsetInfo offsetInfo = (*it).second;
		hsStream* fPackStream = fPackStreams[offsetInfo.fStreamIndex];
		
		fPackStream->SetPosition(offsetInfo.fOffset);

		Int32 size = fPackStream->ReadSwap32();
		if (size > 0)
		{
			char *buf = TRACKED_NEW char[size];
			UInt32 readSize = fPackStream->Read(size, buf);
			hsAssert(readSize <= size, xtl::format("Python PackFile %s: Incorrect amount of data, read %d instead of %d",
				fileName, readSize, size).c_str());

			// let the python marshal make it back into a code object
			PyObject *pythonCode = PyMarshal_ReadObjectFromString(buf, size);

			delete [] buf;

			return pythonCode;
		}
	}

	return nil;
}

hsBool plPythonPack::IsPackedFile(const char* fileName)
{
	if (!Open())
		return nil;

	std::string pythonName = fileName;
	pythonName += ".py";

	FileOffset:: iterator it = fFileOffsets.find(pythonName);
	if (it != fFileOffsets.end())
		return true;

	return false;
}
