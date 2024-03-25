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
//////////////////////////////////////////////////////////////////////
//
// pyStream   - a wrapper class to provide interface to the File stream stuff
//
//////////////////////////////////////////////////////////////////////


#include "pyStream.h"

#include <string_theory/string>

#include "plFileSystem.h"

#include "plFile/plEncryptedStream.h"

pyStream::pyStream()
: fStream()
{
}

bool pyStream::Open(const plFileName& fileName, const ST::string& flags)
{
    // make sure its closed first
    Close();

    if (fileName.IsValid())
    {
        if (!flags.empty())
        {
            bool readflag = false;
            bool writeflag = false;
            bool encryptflag = false;
            for (char flag : flags)
            {
                if (flag == 'r' || flag == 'R')
                    readflag = true;
                if (flag == 'w' || flag == 'W')
                    writeflag = true;
                if (flag == 'e' || flag == 'E')
                    encryptflag = true;
            }
            // if there is a write flag, it takes priorty over read
            if (writeflag)
            {
                // force encryption?
                if (encryptflag)
                {
                    auto stream = std::make_unique<plEncryptedStream>();
                    stream->Open(fileName, "wb");
                    fStream = std::move(stream);
                }
                else
                    fStream = plEncryptedStream::OpenEncryptedFileWrite(fileName);
            }
            else
                fStream = plEncryptedStream::OpenEncryptedFile(fileName);
            return true;
        }
    }
    return false;
}

std::vector<ST::string> pyStream::ReadLines()
{
    
    // read all the lines in the file and put in a python list object
    // create the list
    std::vector<ST::string> pyPL;

    if (fStream)
    {
        char buf[4096];
        
        while (!fStream->AtEnd())
        {
            if (fStream->ReadLn(buf, sizeof(buf), 0, 0))
                pyPL.push_back(buf);
        }
    }

    return pyPL;
}

bool pyStream::WriteLines(const std::vector<ST::string> & lines)
{
    if (fStream)
    {
        int i;
        for ( i=0 ; i<lines.size() ; i++ )
        {
            fStream->WriteString(lines[i]);
        }
        return true;
    }

    return false;
}


void pyStream::Close()
{
    fStream.reset();
}
