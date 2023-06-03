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

#include <Python.h>
#include <marshal.h>
#include <ctime>

#include "HeadSpin.h"
#include "hsStream.h"

#include "plPythonPack.h"

#include "plFile/plStreamSource.h"

struct plPackOffsetInfo
{
    uint32_t fOffset;
    uint32_t fStreamIndex; // index of the stream object in the plPythonPack object that the file resides in
};

class plPythonPack
{
protected:
    std::vector<hsStream*> fPackStreams;
    bool fPackNotFound;     // No pack file, don't keep trying

    typedef std::map<ST::string, plPackOffsetInfo> FileOffset;
    FileOffset fFileOffsets;

    plPythonPack();

public:
    ~plPythonPack();

    static plPythonPack& Instance();

    bool Open();
    void Close();

    PyObject* OpenPacked(const ST::string& sfileName);
    bool IsPackedFile(const ST::string& fileName);
};

PyObject* PythonPack::OpenPythonPacked(const ST::string& fileName)
{
    return plPythonPack::Instance().OpenPacked(fileName);
}

bool PythonPack::IsItPythonPacked(const ST::string& fileName)
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

bool plPythonPack::Open()
{
    if (fPackStreams.size() > 0)
        return true;
    
    // We already tried and it wasn't there
    if (fPackNotFound)
        return false;

    fPackNotFound = true;

    // Get the names of all the pak files
    std::vector<plFileName> files = plStreamSource::GetInstance()->GetListOfNames("python", "pak");

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

            time_t curModTime = 0;
            plFileInfo info(files[curName]);
            if (info.Exists())
                curModTime = info.ModifyTime();
            modTimes.push_back(curModTime);

            // read the index data
            int numFiles = fPackStream->ReadLE32();
            uint32_t streamIndex = (uint32_t)(fPackStreams.size());
            for (int i = 0; i < numFiles; i++)
            {
                // and pack the index into our own data structure
                ST::string pythonName = fPackStream->ReadSafeString();
                uint32_t offset = fPackStream->ReadLE32();

                plPackOffsetInfo offsetInfo;
                offsetInfo.fOffset = offset;
                offsetInfo.fStreamIndex = streamIndex;

                if (fFileOffsets.find(pythonName) != fFileOffsets.end())
                {
                    uint32_t index = fFileOffsets[pythonName].fStreamIndex;
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
        // do NOT close or delete the streams, the preloader will do that for us
        fPackStreams[i] = nullptr;
    }

    fPackStreams.clear();
    fFileOffsets.clear();
}

PyObject* plPythonPack::OpenPacked(const ST::string& fileName)
{
    if (!Open())
        return nullptr;

    ST::string pythonName = fileName + ".py";

    FileOffset::iterator it = fFileOffsets.find(pythonName);
    if (it != fFileOffsets.end())
    {
        plPackOffsetInfo offsetInfo = (*it).second;
        hsStream* fPackStream = fPackStreams[offsetInfo.fStreamIndex];
        
        fPackStream->SetPosition(offsetInfo.fOffset);

        int32_t size = fPackStream->ReadLE32();
        if (size > 0)
        {
            char *buf = new char[size];
            uint32_t readSize = fPackStream->Read(size, buf);
            hsAssert(readSize <= size, ST::format("Python PackFile {}: Incorrect amount of data, read {} instead of {}",
                     fileName, readSize, size).c_str());

            // let the python marshal make it back into a code object
            PyObject *pythonCode = PyMarshal_ReadObjectFromString(buf, size);

            delete [] buf;

            return pythonCode;
        }
    }

    return nullptr;
}

bool plPythonPack::IsPackedFile(const ST::string& fileName)
{
    if (!Open())
        return false;

    ST::string pythonName = fileName + ".py";

    FileOffset:: iterator it = fFileOffsets.find(pythonName);
    if (it != fFileOffsets.end())
        return true;

    return false;
}
