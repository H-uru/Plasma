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
#ifndef _plKickableLog_h_inc
#define _plKickableLog_h_inc

#include "plStatusLog/plStatusLog.h"

// Define this to get extremely spammy logging about kickable forces and such.
//#define USE_KICKABLE_LOG

// Calls to plKickableLog don't get optimized out correctly if the function
// body is empty due to preprocessing. So, use these macros to output to the
// kickable log to ensure proper optimization.
#ifdef USE_KICKABLE_LOG
#   define plKickableLog_Blue(...) plKickableLog::Blue(__VA_ARGS__)
#   define plKickableLog_Green(...) plKickableLog::Green(__VA_ARGS__)
#   define plKickableLog_Red(...) plKickableLog::Red(__VA_ARGS__)
#   define plKickableLog_White(...) plKickableLog::White(__VA_ARGS__)
#   define plKickableLog_Yellow(...) plKickableLog::Yellow(__VA_ARGS__)
#else
#   define plKickableLog_Blue(...) ((void)0)
#   define plKickableLog_Green(...) ((void)0)
#   define plKickableLog_Red(...) ((void)0)
#   define plKickableLog_White(...) ((void)0)
#   define plKickableLog_Yellow(...) ((void)0)
#endif

class plKickableLog
{
    static plStatusLog* sLog;

public:
    static void Blue(const char* line)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLine(plStatusLog::kBlue, line);
#endif
    }

    template<typename... _Args>
    static void Blue(const char* fmt, _Args&&... args)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLineF(plStatusLog::kBlue, fmt, std::forward<_Args>(args)...);
#endif
    }

    static void Green(const char* line)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLine(plStatusLog::kGreen, line);
#endif
    }

    template<typename... _Args>
    static void Green(const char* fmt, _Args&&... args)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLineF(plStatusLog::kGreen, fmt, std::forward<_Args>(args)...);
#endif
    }

    static void Red(const char* line)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLine(plStatusLog::kRed, line);
#endif
    }

    template<typename... _Args>
    static void Red(const char* fmt, _Args&&... args)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLineF(plStatusLog::kRed, fmt, std::forward<_Args>(args)...);
#endif
    }

    static void White(const char* line)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLine(plStatusLog::kWhite, line);
#endif
    }

    template<typename... _Args>
    static void White(const char* fmt, _Args&&... args)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLineF(plStatusLog::kWhite, fmt, std::forward<_Args>(args)...);
#endif
    }

    static void Yellow(const char* line)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLine(plStatusLog::kYellow, line);
#endif
    }

    template<typename... _Args>
    static void Yellow(const char* fmt, _Args&&... args)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (sLog)
            sLog->AddLineF(plStatusLog::kYellow, fmt, std::forward<_Args>(args)...);
#endif
    }
};

#endif
