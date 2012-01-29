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
//////////////////////////////////////////////////////////////////////////////
//
//  plResMgrSettings - Class that holds all the various settings for 
//                     plResManager
//
//// History /////////////////////////////////////////////////////////////////
//
//  6.22.2002 mcn   - Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plResMgrSettings_h
#define _plResMgrSettings_h

#include "HeadSpin.h"

class plResMgrSettings
{
protected:
    friend class plResManager;

    bool fFilterOlderPageVersions;
    bool fFilterNewerPageVersions;

    uint8_t fLoggingLevel;

    bool fPassiveKeyRead;
    bool fLoadPagesOnInit;

    plResMgrSettings()
    {
        fFilterOlderPageVersions = true;
        fFilterNewerPageVersions = true;
        fPassiveKeyRead = false;
        fLoadPagesOnInit = true;
        fLoggingLevel = 0;
    }

public:
    enum LogLevels
    {
        kNoLogging = 0,
        kBasicLogging = 1,
        kDetailedLogging = 2,
        kObjectLogging = 3,
        kObjectDetailLogging = 4
    };

    bool GetFilterOlderPageVersions() const { return fFilterOlderPageVersions; }
    void SetFilterOlderPageVersions(bool f) { fFilterOlderPageVersions = f; }

    bool GetFilterNewerPageVersions() const { return fFilterNewerPageVersions; }
    void SetFilterNewerPageVersions(bool f) { fFilterNewerPageVersions = f; }

    uint8_t   GetLoggingLevel() const { return fLoggingLevel; }
    void    SetLoggingLevel(uint8_t level) { fLoggingLevel = level; }

    bool GetPassiveKeyRead() const { return fPassiveKeyRead; }
    void SetPassiveKeyRead(bool p) { fPassiveKeyRead = p; }

    bool GetLoadPagesOnInit() const { return fLoadPagesOnInit; }
    void SetLoadPagesOnInit(bool load) { fLoadPagesOnInit = load; }

    static plResMgrSettings& Get();
};

#endif // _plResMgrSettings_h
