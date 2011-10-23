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
#ifndef plDispatchLogBase_inc
#define plDispatchLogBase_inc

#include "hsTypes.h"

//
// For debugging messaging code.
//
class plMessage;
class plReceiver;

class plDispatchLogBase
{
public:
    enum Flags
    {
        kInclude            = 0x1,
        kLogLongReceives    = 0x2,
    };

protected:
    static UInt32 fFlags;
    static plDispatchLogBase* fInstance;

public:
    static plDispatchLogBase* GetInstance() { return fInstance; }

    virtual ~plDispatchLogBase() {}

    static void SetFlags(UInt32 f) { fFlags=f; }
    static UInt32 GetFlags() { return fFlags; }

    static bool IsLogging() { return fInstance != nil; }
    static bool IsLoggingLong() { return (fFlags & kLogLongReceives) != 0; }

    virtual void AddFilterType(UInt16 type)=0;
    virtual void AddFilterExactType(UInt16 type)=0;

    virtual void RemoveFilterType(UInt16 type)=0;
    virtual void RemoveFilterExactType(UInt16 type)=0;

    virtual void LogStatusBarChange(const char* name, const char* action)=0;
    virtual void LogLongReceive(const char* keyname, const char* className, UInt32 clonePlayerID, plMessage* msg, float ms)=0;

    virtual void DumpMsg(plMessage* msg, int numReceivers, int sendTime, Int32 indent)=0;
};

#endif  // plDispatchLogBase_inc
