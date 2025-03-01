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

#ifndef _pfConsoleParser_h
#define _pfConsoleParser_h

#include "HeadSpin.h"

#include <optional>
#include <string_theory/string>
#include <tuple>
#include <vector>

class pfConsoleCmd;
class pfConsoleCmdGroup;

class pfConsoleTokenizer
{
public:
    ST::string::const_iterator fPos;
    ST::string::const_iterator fEnd;
    ST::string fErrorMsg;

    pfConsoleTokenizer(ST::string::const_iterator begin, ST::string::const_iterator end) :
        fPos(begin), fEnd(end), fErrorMsg()
    {}
    pfConsoleTokenizer(const ST::string& line) : pfConsoleTokenizer(line.begin(), line.end()) {}

    // Parse the next command name or argument token from the input line.
    // On success, returns the parsed token.
    // (Note that a successfully parsed token may be an empty string, e. g. from an empty pair of quotes!)
    // If the next token couldn't be parsed (e. g. quote not closed),
    // returns an empty std::optional and sets fErrorMsg to a non-empty string.
    // If there are no further tokens in the line,
    // returns an empty std::optional and sets fErrorMsg to an empty string.
    std::optional<ST::string> NextNamePart();
    std::optional<ST::string> NextArgument();
};

class pfConsoleParser
{
public:
    pfConsoleTokenizer fTokenizer;

    pfConsoleParser(ST::string::const_iterator begin, ST::string::const_iterator end) : fTokenizer(begin, end) {}
    pfConsoleParser(const ST::string& line) : pfConsoleParser(line.begin(), line.end()) {}

    ST::string GetErrorMsg() const { return fTokenizer.fErrorMsg; }

    // Parse the command name part of the line as far as possible.
    // This consumes name part tokens and uses them to look up a command group
    // until a token is encountered that isn't a known group name.
    // Returns the found group and the first non-group token
    // (which may be an empty std::optional if the end of the line was reached).
    std::tuple<pfConsoleCmdGroup*, std::optional<ST::string>> ParseGroupAndName();

    // Parse the command name part of the line.
    // Returns the command corresponding to that name,
    // or nullptr if no matching command was found.
    pfConsoleCmd* ParseCommand();

    // Parse the entire command name part of the line
    // without taking the currently defined commands and groups into account.
    // This allows parsing command names that might not exist,
    // which can be useful for parsing configuration files that could contain unknown commands.
    // Note that this only works correctly for command names with . or _ separators, not spaces!
    // For example, "Group.Command" and "Group_Command" are parsed as expected,
    // but "Group Command" will be parsed as a command "Group" with argument "Command".
    // Returns a vector containing all tokens in the command name,
    // or an empty vector if the line is empty.
    std::vector<ST::string> ParseUnknownCommandName();

    // Parse the remainder of the line as command arguments.
    // On success, returns the parsed arguments with any surrounding quotes removed.
    // If any of the arguments couldn't be parsed,
    // returns an empty std::optional (call GetErrorMsg for an error message).
    std::optional<std::vector<ST::string>> ParseArguments();
};

#endif // _pfConsoleParser_h
