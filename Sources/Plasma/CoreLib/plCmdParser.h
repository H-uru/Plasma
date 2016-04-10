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

#ifndef _plCmdParser_h_
#define _plCmdParser_h_

#include <string_theory/string>

enum CmdArg
{
    kCmdArgFlagged              = 0x0,      // default
    kCmdArgOptional             = 0x1,
    kCmdArgRequired             = 0x2,
    kCmdArgMask                 = 0xF
};

enum CmdType
{
    kCmdTypeBool                = 0x0 << 4, // default
    kCmdTypeInt                 = 0x1 << 4,
    kCmdTypeUint                = 0x2 << 4,
    kCmdTypeFloat               = 0x3 << 4,
    kCmdTypeString              = 0x4 << 4,
    kCmdTypeMask                = 0xF << 4
};

enum CmdBool
{
    kCmdBoolSet                 = 0x0 << 8, // default
    kCmdBoolUnset               = 0x1 << 8,
    kCmdBoolMask                = 0xF << 8
};

enum CmdCase
{
    kCmdCaseSensitive           = 0x1 << 28
};

// Error codes
enum CmdError
{
    kCmdErrorSuccess,
    kCmdErrorInvalidArg,
    kCmdErrorInvalidValue,
    kCmdErrorTooFewArgs,
    kCmdErrorTooManyArgs,
    kNumCmdErrors
};


struct plCmdArgDef
{
    uint32_t        flags;
    ST::string      name;   // must be compile-time constant
    size_t          id;
};


class plCmdParser
{
protected:
    class plCmdParserImpl*  fParser;

    plCmdParser() : fParser(nullptr) { }
    void Initialize(const plCmdArgDef* defs, size_t defCount);

public:
    plCmdParser(const plCmdArgDef* defs, size_t defCount);
    virtual ~plCmdParser();

    bool            Parse(const ST::string& cmdLine);
    bool            Parse(std::vector<ST::string>& argv);

    ST::string      GetProgramName() const;

    bool            GetBool(size_t id) const;
    bool            GetBool(const ST::string& name) const;

    float           GetFloat(size_t id) const;
    float           GetFloat(const ST::string& name) const;

    int32_t         GetInt(size_t id) const;
    int32_t         GetInt(const ST::string& name) const;

    ST::string      GetString(size_t id) const;
    ST::string      GetString(const ST::string& name) const;

    uint32_t        GetUint(size_t id) const;
    uint32_t        GetUint(const ST::string& name) const;

    bool            IsSpecified(size_t id) const;
    bool            IsSpecified(const ST::string& name) const;

    CmdError        GetError() const;
};

#endif //_plCmdParser_h_
