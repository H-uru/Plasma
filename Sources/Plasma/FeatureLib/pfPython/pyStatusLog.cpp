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
// pyStatusLog   - a wrapper class to provide interface to the plStatusLog stuff
//
//  and interface to the ChatLog (ptChatStatusLog)
//////////////////////////////////////////////////////////////////////

#include <string_theory/string>

#include "pyColor.h"
#include "pyStatusLog.h"
#include "plStatusLog/plStatusLog.h"

pyStatusLog::pyStatusLog(plStatusLog* log/*=nullptr */)
: fLog( log )
, fICreatedLog( false )
{
}

pyStatusLog::~pyStatusLog()
{
    Close();
}


bool pyStatusLog::Open(const ST::string &logName, uint32_t numLines, uint32_t flags)
{
    // make sure its closed first
    Close();

    // create a status log guy for this
    fICreatedLog = true;
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog( (uint8_t)numLines, logName, flags );
    if (fLog)
    {
        fLog->SetForceLog(true);
        return true;
    }
    return false;
}

bool pyStatusLog::Write(const ST::string &text)
{
    if (fLog)
    {
        fLog->AddLine(text);
        return true;
    }

    return false;
}

bool pyStatusLog::WriteColor(const ST::string &text, pyColor& color)
{
    if (fLog)
    {
        uint32_t st_color = ((uint32_t)(color.getAlpha()*255)<<24) +
                                ((uint32_t)(color.getRed()*255)<<16) +
                                ((uint32_t)(color.getGreen()*255)<<8) + 
                                ((uint32_t)(color.getBlue()*255));
        fLog->AddLine(st_color, text);
        return true;
    }

    return false;
}

void pyStatusLog::Close()
{
    if (fLog && fICreatedLog)
    {
        delete fLog;
    }
    fLog = nullptr;
}
