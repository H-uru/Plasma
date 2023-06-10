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
#ifndef plStreamSource_h_inc
#define plStreamSource_h_inc

#include <map>
#include <memory>
#include <mutex>
#include "hsStream.h"

// A class for holding and accessing file streams. The preloader will insert
// files in here once they are loaded. In internal builds, if a requested file
// is not found, it will be retrieved from disk.
class plStreamSource
{
private:
    struct fileData
    {
        plFileName      fFilename; // includes path
        plFileName      fDir; // parent directory
        ST::string      fExt;
        std::unique_ptr<hsStream> fStream;
    };
    std::map<plFileName, fileData, plFileName::less_i> fFileData; // key is filename
    std::mutex fMutex;
    uint32_t fServerKey[4];

    void ICleanup(); // closes all file pointers and cleans up after itself

    plStreamSource();
public:
    ~plStreamSource() {ICleanup();}

    // Force a cleanup of all data (some apps need to get at those file again, and they can't while we have them open)
    void Cleanup() {ICleanup();}

    // File access functions
    hsStream* GetFile(const plFileName& filename); // internal builds will read from disk if it doesn't exist
    std::vector<plFileName> GetListOfNames(const plFileName& dir, const ST::string& ext); // internal builds merge from disk

    // For other classes to insert files (takes ownership of the stream if successful)
    bool InsertFile(const plFileName& filename, std::unique_ptr<hsStream>&& stream);

    /** Gets a pointer to our encryption key */
    uint32_t* GetEncryptionKey() { return fServerKey; }

    // Instance handling
    static plStreamSource* GetInstance();
};

#endif // plStreamSource_h_inc