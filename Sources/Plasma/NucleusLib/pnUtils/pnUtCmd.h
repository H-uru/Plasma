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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCmd.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTCMD_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTCMD_H

#include "Pch.h"
#include "pnUtArray.h"
#include "pnUtStr.h"

/*****************************************************************************
*
*   Constants
*
***/

// Sets of mutually exclusive flags
const unsigned kCmdArgFlagged       = 0x0 << 0;     // default
const unsigned kCmdArgOptional      = 0x1 << 0;
const unsigned kCmdArgRequired      = 0x2 << 0;
const unsigned kCmdMaskArg          = 0xf << 0;

const unsigned kCmdTypeBool         = 0x0 << 4;     // default
const unsigned kCmdTypeInt          = 0x1 << 4;
const unsigned kCmdTypeUnsigned     = 0x2 << 4;
const unsigned kCmdTypeFloat        = 0x3 << 4;
const unsigned kCmdTypeString       = 0x4 << 4;
const unsigned kCmdMaskType         = 0xf << 4;

const unsigned kCmdBoolSet          = 0x0 << 8;     // default
const unsigned kCmdBoolUnset        = 0x1 << 8;
const unsigned kCmdMaskBool         = 0xf << 8;

// Other flags
const unsigned kCmdCaseSensitive    = 0x1 << 28;


// Error codes
enum ECmdError {
    kCmdErrorInvalidArg,
    kCmdErrorInvalidValue,
    kCmdErrorTooFewArgs,
    kCmdErrorTooManyArgs,
    kNumCmdErrors
};


/*****************************************************************************
*
*   Types
*
***/

struct CmdArgDef {
    unsigned        flags;
    const wchar_t *   name;   // must be compile-time constant
    unsigned        id;
};

class CCmdParser {
    class CICmdParser * fParser;

    static void DispatchError (const wchar_t str[], ECmdError errorCode, const wchar_t arg[], const wchar_t value[], void * param);
    static bool DispatchExtra (const wchar_t str[], void * param);

protected:
    CCmdParser ();
    void Initialize (const CmdArgDef def[], unsigned defCount);

public:
    CCmdParser (const CmdArgDef def[], unsigned defCount);
    virtual ~CCmdParser ();

    bool          GetBool (unsigned id) const;
    bool          GetBool (const wchar_t name[]) const;
    float         GetFloat (unsigned id) const;
    float         GetFloat (const wchar_t name[]) const;
    int           GetInt (unsigned id) const;
    int           GetInt (const wchar_t name[]) const;
    const wchar_t * GetString (unsigned id) const;
    const wchar_t * GetString (const wchar_t name[]) const;
    unsigned      GetUnsigned (unsigned id) const;
    unsigned      GetUnsigned (const wchar_t name[]) const;
    bool          IsSpecified (unsigned id) const;
    bool          IsSpecified (const wchar_t name[]) const;

    virtual void  OnError (const wchar_t str[], ECmdError errorCode, const wchar_t arg[], const wchar_t value[]);
    virtual bool  OnExtra (const wchar_t str[]);

    bool          Parse (const wchar_t cmdLine[] = nil);
};
#endif
