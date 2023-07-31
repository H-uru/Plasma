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
//                                                                          //
//  pfConsoleEngine Functions                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleEngine.h"
#include "pfConsoleCmd.h"
#include "pfConsoleContext.h"
#include "pfConsoleParser.h"

#include <string_theory/format>
#include <string_theory/string_stream>
#include <utility>
#include <vector>

#include "plFile/plEncryptedStream.h"


const int32_t     pfConsoleEngine::fMaxNumParams = 16;


//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleEngine::pfConsoleEngine()
{
}

pfConsoleEngine::~pfConsoleEngine()
{
}

//// PrintCmdHelp ////////////////////////////////////////////////////////////

bool pfConsoleEngine::PrintCmdHelp(const ST::string& name, void (*PrintFn)(const ST::string&))
{
    pfConsoleCmd        *cmd;

    pfConsoleParser parser(name);
    auto [group, token] = parser.ParseGroupAndName();

    if (!token) {
        // Print help for this group
        if( group == pfConsoleCmdGroup::GetBaseGroup() )
            PrintFn(ST_LITERAL("Base commands and groups:"));
        else
            PrintFn(ST::format("Group {}:", group->GetName()));
        PrintFn(ST_LITERAL("  Subgroups:"));
        for (pfConsoleCmdGroup* subGrp = group->GetFirstSubGroup(); subGrp != nullptr; subGrp = subGrp->GetNext())
        {
            PrintFn(ST::format("    {}", subGrp->GetName()));
        }
        PrintFn(ST_LITERAL("  Commands:"));
        for (cmd = group->GetFirstCommand(); cmd != nullptr; cmd = cmd->GetNext())
        {
            PrintFn(ST::format("    {}: {}", cmd->GetName(), cmd->GetHelp().before_first('\n')));
        }

        return true;
    }

    /// OK, so what we found wasn't a group. Which means we need a command...
    cmd = group->FindCommandNoCase(*token);
    if (cmd == nullptr)
    {
        fErrorMsg = ST_LITERAL("Invalid syntax: command not found");
        return false;
    }

    /// That's it!
    PrintFn(ST::format("\nHelp for the command {}:", cmd->GetName()));
    PrintFn(ST::format("\\i{}", cmd->GetHelp()));
    PrintFn(ST::format("\\iUsage: {}", cmd->GetSignature()));

    return true;
}

//// GetCmdSignature /////////////////////////////////////////////////////////

ST::string pfConsoleEngine::GetCmdSignature(const ST::string& name)
{
    pfConsoleCmd        *cmd;
    
    pfConsoleParser parser(name);
    auto [group, token] = parser.ParseGroupAndName();

    if (!token) {
        fErrorMsg = ST_LITERAL("Invalid command syntax");
        return {};
    }

    /// OK, so what we found wasn't a group. Which means we need a command...
    cmd = group->FindCommandNoCase(*token);
    if (cmd == nullptr)
    {
        fErrorMsg = ST_LITERAL("Invalid syntax: command not found");
        return {};
    }

    /// That's it!
    return cmd->GetSignature();
}

//// Dummy Local Function ////////////////////////////////////////////////////

void DummyPrintFn(const ST::string& line)
{
}

//// ExecuteFile /////////////////////////////////////////////////////////////

bool pfConsoleEngine::ExecuteFile(const plFileName &fileName)
{
    int     line;

    std::unique_ptr<hsStream> stream = plEncryptedStream::OpenEncryptedFile(fileName);

    if( !stream )
    {
        fErrorMsg = ST_LITERAL("Cannot open given file");
//      return false;
        /// THIS IS BAD: because of the asserts we throw after this if we return false, a missing
        /// file will throw an assert. This is all well and good except for the age-specific .fni files,
        /// which aren't required to be there and rely on this functionality to test whether the file is
        /// present. Once age-specific .fni's are gone, reinstate the return false here. -mcn
        return true;
    }

    ST::string string;
    for (line = 1; stream->ReadLn(string); line++)
    {
        fLastErrorLine = string;

        if( !RunCommand( string, DummyPrintFn ) )
        {
            fErrorMsg = ST::format("Error in console file {}, command line {}: {}",
                     fileName.AsString(), line, fErrorMsg);
            return false;
        }
    }
    fLastErrorLine.clear();

    return true;
}

//// RunCommand //////////////////////////////////////////////////////////////
//  Updated 2.14.2001 mcn to support spaces, _ or . as group separators. This
//  requires tokenizing the entire line and searching the tokens one by one,
//  parsing them first as groups, then commands and then params.

bool pfConsoleEngine::RunCommand(const ST::string& line, void (*PrintFn)(const ST::string&))
{
    pfConsoleCmd        *cmd;
    int32_t               numParams, i;
    pfConsoleCmdParam   paramArray[ fMaxNumParams + 1 ];
    bool                valid = true;

    pfConsoleParser parser(line);
    auto [group, token] = parser.ParseGroupAndName();

    if (!token) {
        fErrorMsg = ST_LITERAL("Invalid command syntax");
        return false;
    }

    /// OK, so what we found wasn't a group. Which means we need a command next
    cmd = group->FindCommandNoCase(*token);
    if (cmd == nullptr)
    {
        fErrorMsg = ST_LITERAL("Invalid syntax: command not found");
        return false;
    }

    /// We have the group, we have the command from that group. So continue
    /// tokenizing (with the new separators now, mind you) and turn them into
    /// params

    for (numParams = 0; numParams < fMaxNumParams
                        && (token = parser.fTokenizer.NextArgument())
                        && valid; numParams++ )
    {
        // Special case for context variables--if we're specifying one, we want to just grab
        // the value of it and return that instead
        valid = false;
        if (token->starts_with("$")) {
            pfConsoleContext    &context = pfConsoleContext::GetRootContext();

            // Potential variable, see if we can find it
            hsSsize_t idx = context.FindVar(token->substr(1));
            if( idx == -1 )
            {
                fErrorMsg = ST_LITERAL("Invalid console variable name");
            }
            else
            {
                paramArray[ numParams ] = context.GetVarValue( idx );
                valid = true;
            }
        }

        if( !valid )
            valid = IConvertToParam(cmd->GetSigEntry(numParams), *token, &paramArray[numParams]);
    }

    ST::string errorMsg = parser.fTokenizer.fErrorMsg;
    if (!errorMsg.empty()) {
        fErrorMsg = ST::format("Invalid syntax: {}", errorMsg);
        return false;
    }

    for( i = numParams; i < fMaxNumParams + 1; i++ )
        paramArray[ i ].SetNone();

    if (!valid || (cmd->GetSigEntry(numParams) != pfConsoleCmd::kAny &&
                   cmd->GetSigEntry(numParams) != pfConsoleCmd::kNone))
    {
        // Print help string and return
        fErrorMsg.clear(); // Printed on next line
        PrintFn(ST_LITERAL("Invalid parameters to command"));
        PrintFn(ST::format("Usage: {}", cmd->GetSignature()));
        return false;
    }

    /// Execute it and return
    cmd->Execute( numParams, paramArray, PrintFn );
    return true;
}

//// IConvertToParam /////////////////////////////////////////////////////////
//  Converts a null-terminated string representing a parameter to a 
//  pfConsoleCmdParam argument.

bool pfConsoleEngine::IConvertToParam(uint8_t type, ST::string string, pfConsoleCmdParam *param)
{
    if( type == pfConsoleCmd::kNone )
        return false;

    if( type == pfConsoleCmd::kAny )
    {
        /// Want "any"
        param->SetAny(std::move(string));
    }
    else if( type == pfConsoleCmd::kString )
    {
        /// Want just a string
        param->SetString(std::move(string));
    }
    else if( type == pfConsoleCmd::kFloat )
    {
        ST::conversion_result res;
        float value = string.to_float(res);
        if (!res.ok() || !res.full_match()) {
            return false;
        }

        param->SetFloat(value);
    }
    else if( type == pfConsoleCmd::kInt )
    {
        ST::conversion_result res;
        int value = string.to_int(res, 10);
        if (!res.ok() || !res.full_match()) {
            return false;
        }

        param->SetInt(value);
    }
    else if( type == pfConsoleCmd::kBool )
    {
        if (string.compare_i("t") == 0) {
            param->SetBool(true);
        } else if (string.compare_i("f") == 0) {
            param->SetBool(false);
        } else {
            param->SetBool(string.to_bool());
        }
    }

    return true;
}

//// FindPartialCmd //////////////////////////////////////////////////////////
//  Given a string which is the beginning of a console command,
//  returns the best match of command (or group) for that string.

ST::string pfConsoleEngine::FindPartialCmd(const ST::string& line, bool findAgain, bool preserveParams)
{
    static pfConsoleCmd         *lastCmd = nullptr;
    static pfConsoleCmdGroup    *lastGroup = nullptr;

    /// Repeat search
    if( !findAgain )
    {
        lastCmd = nullptr;
        lastGroup = nullptr;
    }

    /// New search
    ST::string_stream newStr;

    pfConsoleParser parser(line);
    auto [group, token] = parser.ParseGroupAndName();

    // Add group name to replacement line.
    if (group != pfConsoleCmdGroup::GetBaseGroup()) {
        newStr << group->GetFullName() << '.';
    }

    if (token) {
        // Still got at least one token left. Try to match to either
        // a partial group or a partial command
        pfConsoleCmdGroup* subGrp = group->FindSubGroupNoCase(*token, pfConsoleCmdGroup::kFindPartial, lastGroup);
        if (subGrp != nullptr)
        {
            lastGroup = group = subGrp;
            newStr << group->GetName()  << '.';
        }
        else 
        {
            pfConsoleCmd* cmd = group->FindCommandNoCase(*token, pfConsoleCmdGroup::kFindPartial, lastCmd);
            if (cmd == nullptr)
                return {};

            newStr << cmd->GetName()  << ' ';
            lastCmd = cmd;
        }
    }

    if( preserveParams )
    {
        /// Preserve the rest of the string after the matched command
        newStr.append(parser.fTokenizer.fPos, parser.fTokenizer.fEnd - parser.fTokenizer.fPos);
    }

    return newStr.to_string();
}

//// FindNestedPartialCmd ////////////////////////////////////////////////////
//  Same as FindPartialCmd, only starts from the global group and searches
//  everything. The string passed should only be a partial command sans
//  groups. numToSkip specifies how many matches to skip before returning one
//  (so if numToSkip = 1, then this will return the second match found).

ST::string pfConsoleEngine::FindNestedPartialCmd(const ST::string& line, uint32_t numToSkip, bool preserveParams)
{
    /// Somewhat easier than FindPartialCmd...
    pfConsoleCmd* cmd = pfConsoleCmdGroup::GetBaseGroup()->FindNestedPartialCommand(line, &numToSkip);
    if (cmd == nullptr)
        return {};

    ST::string_stream name;
    name << cmd->GetFullName() << ' ';

    if( preserveParams )
    {
        /// Preserve the rest of the string after the matched command
    }

    return name.to_string();
}
