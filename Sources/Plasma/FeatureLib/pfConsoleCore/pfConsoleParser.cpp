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

#include "pfConsoleParser.h"

#include <string_theory/string>

// Yes, this is invalid UTF-8. Yes, this is awful.
const ST::string pfConsoleTokenizer::kTokenizeError = ST_LITERAL("\xff");

static const char kTokenSeparators[] = " =\r\n\t,";
static const char kTokenGrpSeps[] = " =\r\n._\t,";

//WARNING: Potentially increments the pointer passed to it.
std::optional<ST::string> pfConsoleTokenizer::TokenizeCommandName(const char*& line)
{
    const char* begin = line;

    while (*begin && isspace(static_cast<unsigned char>(*begin)))
        ++begin;

    for (line = begin; *line; ++line) {
        for (const char *sep = kTokenGrpSeps; *sep; ++sep) {
            if (*line == *sep) {
                const char* end = line;
                while (*++line && (*line == *sep))
                    /* skip duplicate delimiters */;
                return ST::string::from_utf8(begin, end - begin);
            }
        }
    }

    if (begin == line) {
        return {};
    }

    return begin;
}

//WARNING: Potentially increments the pointer passed to it.
std::optional<ST::string> pfConsoleTokenizer::TokenizeArguments(const char*& line)
{
    const char* begin = line;

    while (*begin && isspace(static_cast<unsigned char>(*begin)))
        ++begin;

    for (line = begin; *line; ++line) {
        if (*begin == '"' || *begin == '\'') {
            // Handle strings as a single token
            ++begin;
            const char* end = strchr(begin, *line);
            if (end == nullptr) {
                return kTokenizeError;
            }
            line = end + 1;
            return ST::string::from_utf8(begin, end - begin);
        }
        for (const char *sep = kTokenSeparators; *sep; ++sep) {
            if (*line == *sep) {
                const char* end = line;
                while (*++line && (*line == *sep))
                    /* skip duplicate delimiters */;
                return ST::string::from_utf8(begin, end - begin);
            }
        }
    }

    if (begin == line) {
        return {};
    }

    return begin;
}
