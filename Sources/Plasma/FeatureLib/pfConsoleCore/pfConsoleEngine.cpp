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

#include <string_theory/string>
#include <utility>

#include "plFile/plEncryptedStream.h"


const int32_t     pfConsoleEngine::fMaxNumParams = 16;

static const char kTokenSeparators[] = " =\r\n\t,";
static const char kTokenGrpSeps[] = " =\r\n._\t,";

//WARNING: Potentially increments the pointer passed to it.
static const char *console_strtok( char *&line, bool haveCommand )
{
    char *begin = line;

    while (*begin && isspace(static_cast<unsigned char>(*begin)))
        ++begin;

    for (line = begin; *line; ++line) {
        if (!haveCommand) {
            for (const char *sep = kTokenGrpSeps; *sep; ++sep) {
                if (*line == *sep) {
                    *line = 0;
                    while (*++line && (*line == *sep))
                        /* skip duplicate delimiters */;
                    return begin;
                }
            }
        } else {
            if (*begin == '"' || *begin == '\'') {
                // Handle strings as a single token
                char *endptr = strchr(line + 1, *line);
                if (endptr == nullptr) {
                    // Bad string token sentry
                    return "\xFF";
                }
                *endptr = 0;
                line = endptr + 1;
                return begin + 1;
            }
            for (const char *sep = kTokenSeparators; *sep; ++sep) {
                if (*line == *sep) {
                    *line = 0;
                    while (*++line && (*line == *sep))
                        /* skip duplicate delimiters */;
                    return begin;
                }
            }
        }
    }

    if (begin == line)
        return nullptr;

    line = line + strlen(line);
    return begin;
}


//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleEngine::pfConsoleEngine()
{
}

pfConsoleEngine::~pfConsoleEngine()
{
}

//// PrintCmdHelp ////////////////////////////////////////////////////////////

bool    pfConsoleEngine::PrintCmdHelp( char *name, void (*PrintFn)( const char * ) )
{
    pfConsoleCmd        *cmd;
    pfConsoleCmdGroup   *group, *subGrp;
    const char          *ptr;
    static char         tempString[ 512 ];
    uint32_t              i;

    /// Scan for subgroups. This can be an empty loop
    group = pfConsoleCmdGroup::GetBaseGroup();
    ptr = console_strtok( name, false );
    while (ptr != nullptr)
    {
        // Take this token and check to see if it's a group
        if ((subGrp = group->FindSubGroupNoCase(ptr)) != nullptr)
            group = subGrp;
        else
            break;

        ptr = console_strtok( name, false );
    }

    if (ptr == nullptr)
    {
        if (group == nullptr)
        {
            ISetErrorMsg( "Invalid command syntax" );
            return false;
        }

        // Print help for this group
        if( group == pfConsoleCmdGroup::GetBaseGroup() )
            PrintFn("Base commands and groups:");
        else
            PrintFn(ST::format("Group {}:", group->GetName()).c_str());
        PrintFn("  Subgroups:");
        for (subGrp = group->GetFirstSubGroup(); subGrp != nullptr; subGrp = subGrp->GetNext())
        {
            PrintFn(ST::format("    {}", subGrp->GetName()).c_str());
        }
        PrintFn("  Commands:");
        for (cmd = group->GetFirstCommand(); cmd != nullptr; cmd = cmd->GetNext())
        {
            const char* p = cmd->GetHelp();
            for(i = 0; p[ i ] != 0 && p[ i ] != '\n'; i++) {
                tempString[ i ] = p[ i ];
            }
            tempString[ i ] = 0;

            PrintFn(ST::format("    {}: {}", cmd->GetName(), tempString).c_str());
        }

        return true;
    }

    /// OK, so what we found wasn't a group. Which means we need a command...
    cmd = group->FindCommandNoCase( ptr );
    if (cmd == nullptr)
    {
        ISetErrorMsg( "Invalid syntax: command not found" );
        return false;
    }

    /// That's it!
    PrintFn(ST::format("\nHelp for the command {}:", cmd->GetName()).c_str());
    PrintFn(ST::format("\\i{}", cmd->GetHelp()).c_str());
    PrintFn(ST::format("\\iUsage: {}", cmd->GetSignature()).c_str());

    return true;
}

//// GetCmdSignature /////////////////////////////////////////////////////////

const char  *pfConsoleEngine::GetCmdSignature( char *name )
{
    pfConsoleCmd        *cmd;
    pfConsoleCmdGroup   *group, *subGrp;
    const char          *ptr;

    
    /// Scan for subgroups. This can be an empty loop
    group = pfConsoleCmdGroup::GetBaseGroup();
    ptr = console_strtok( name, false );
    while (ptr != nullptr)
    {
        // Take this token and check to see if it's a group
        if ((subGrp = group->FindSubGroupNoCase(ptr)) != nullptr)
            group = subGrp;
        else
            break;

        ptr = console_strtok( name, false );
    }

    if (ptr == nullptr)
    {
        ISetErrorMsg( "Invalid command syntax" );
        return nullptr;
    }

    /// OK, so what we found wasn't a group. Which means we need a command...
    cmd = group->FindCommandNoCase( ptr );
    if (cmd == nullptr)
    {
        ISetErrorMsg( "Invalid syntax: command not found" );
        return nullptr;
    }

    /// That's it!
    return cmd->GetSignature();
}

//// Dummy Local Function ////////////////////////////////////////////////////

void    DummyPrintFn( const char *line )
{
}

//// ExecuteFile /////////////////////////////////////////////////////////////

bool pfConsoleEngine::ExecuteFile(const plFileName &fileName)
{
    char    string[ 512 ];
    int     line;

    hsStream* stream = plEncryptedStream::OpenEncryptedFile(fileName);

    if( !stream )
    {
        ISetErrorMsg( "Cannot open given file" );
//      return false;
        /// THIS IS BAD: because of the asserts we throw after this if we return false, a missing
        /// file will throw an assert. This is all well and good except for the age-specific .fni files,
        /// which aren't required to be there and rely on this functionality to test whether the file is
        /// present. Once age-specific .fni's are gone, reinstate the return false here. -mcn
        return true;
    }

    for (line = 1; stream->ReadLn(string, std::size(string)); line++)
    {
        strncpy(fLastErrorLine, string, std::size(fLastErrorLine));

        if( !RunCommand( string, DummyPrintFn ) )
        {
            snprintf(string, std::size(string), "Error in console file %s, command line %d: %s",
                     fileName.AsString().c_str(), line, fErrorMsg);
            ISetErrorMsg( string );
            stream->Close();
            delete stream;
            return false;
        }
    }
    stream->Close();
    delete stream;
    fLastErrorLine[ 0 ] = 0;

    return true;
}

//// RunCommand //////////////////////////////////////////////////////////////
//  Updated 2.14.2001 mcn to support spaces, _ or . as group separators. This
//  requires tokenizing the entire line and searching the tokens one by one,
//  parsing them first as groups, then commands and then params.

bool    pfConsoleEngine::RunCommand( char *line, void (*PrintFn)( const char * ) )
{
    pfConsoleCmd        *cmd;
    pfConsoleCmdGroup   *group, *subGrp;
    int32_t               numParams, i, numQuotedParams = 0;
    pfConsoleCmdParam   paramArray[ fMaxNumParams + 1 ];
    const char          *ptr;
    bool                valid = true;


    hsAssert(line != nullptr, "Bad parameter to RunCommand()");

    /// Loop #1: Scan for subgroups. This can be an empty loop
    group = pfConsoleCmdGroup::GetBaseGroup();
    ptr = console_strtok( line, false );
    while (ptr != nullptr)
    {
        // Take this token and check to see if it's a group
        if ((subGrp = group->FindSubGroupNoCase(ptr)) != nullptr)
            group = subGrp;
        else
            break;

        ptr = console_strtok( line, false );
    }

    if (ptr == nullptr)
    {
        ISetErrorMsg( "Invalid command syntax" );
        return false;
    }

    /// OK, so what we found wasn't a group. Which means we need a command next
    cmd = group->FindCommandNoCase( ptr );
    if (cmd == nullptr)
    {
        ISetErrorMsg( "Invalid syntax: command not found" );
        return false;
    }

    /// We have the group, we have the command from that group. So continue
    /// tokenizing (with the new separators now, mind you) and turn them into
    /// params

    for( numParams = numQuotedParams = 0; numParams < fMaxNumParams 
                        && (ptr = console_strtok(line, true)) != nullptr
                        && valid; numParams++ )
    {
        if( ptr[ 0 ] == '\xFF' )
        {
            ISetErrorMsg( "Invalid syntax: unterminated quoted parameter" );
            return false;
        }

        // Special case for context variables--if we're specifying one, we want to just grab
        // the value of it and return that instead
        valid = false;
        if( ptr[ 0 ] == '$' )
        {
            pfConsoleContext    &context = pfConsoleContext::GetRootContext();

            // Potential variable, see if we can find it
            hsSsize_t idx = context.FindVar( ptr + 1 );
            if( idx == -1 )
            {
                ISetErrorMsg( "Invalid console variable name" );
            }
            else
            {
                paramArray[ numParams ] = context.GetVarValue( idx );
                valid = true;
            }
        }

        if( !valid )
            valid = IConvertToParam(cmd->GetSigEntry(numParams), ptr, &paramArray[numParams]);
    }
    for( i = numParams; i < fMaxNumParams + 1; i++ )
        paramArray[ i ].SetNone();

    if (!valid || (cmd->GetSigEntry(numParams) != pfConsoleCmd::kAny &&
                   cmd->GetSigEntry(numParams) != pfConsoleCmd::kNone))
    {
        // Print help string and return
        ISetErrorMsg( "" ); // Printed on next line
        PrintFn("Invalid parameters to command");
        PrintFn(ST::format("Usage: {}", cmd->GetSignature()).c_str());
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
//  Given a string which is the beginning of a console command, modifies the
//  string to represent the best match of command (or group) for that string.
//  WARNING: modifies the string passed to it.

bool    pfConsoleEngine::FindPartialCmd( char *line, bool findAgain, bool preserveParams )
{
    pfConsoleCmd        *cmd = nullptr;
    pfConsoleCmdGroup   *group, *subGrp;

    static const char           *ptr = nullptr;
    static char                 *insertLoc = nullptr;
    static pfConsoleCmd         *lastCmd = nullptr;
    static pfConsoleCmdGroup    *lastGroup = nullptr;
    static char                 newStr[ 256 ];
    static char                 *originalLine = line;


    /// Repeat search
    if( !findAgain )
    {
        lastCmd = nullptr;
        lastGroup = nullptr;
    }

    /// New search
    if( strlen( line ) > sizeof( newStr ) )
        return false;

    newStr[ 0 ] = 0;
    insertLoc = newStr;

    /// Loop #1: Scan for subgroups. This can be an empty loop
    group = pfConsoleCmdGroup::GetBaseGroup();
    ptr = console_strtok( line, false );
    while (ptr != nullptr)
    {
        // Take this token and check to see if it's a group
        if( ( subGrp = group->FindSubGroupNoCase( ptr, 0/*pfConsoleCmdGroup::kFindPartial*/
            , /*lastGroup*/nullptr)) != nullptr)
        {
            group = subGrp;
            strcat( newStr, group->GetName() );
            insertLoc += strlen( group->GetName() );
        }
        else
            break;

        ptr = console_strtok( line, false );
        strcat( newStr, "." );
        insertLoc++;
    }

    if (ptr != nullptr)
    {
        // Still got at least one token left. Try to match to either
        // a partial group or a partial command
        if ((subGrp = group->FindSubGroupNoCase(ptr, pfConsoleCmdGroup::kFindPartial, lastGroup)) != nullptr)
        {
            lastGroup = group = subGrp;
            strcat( newStr, group->GetName() );
            strcat( newStr, "." );
        }
        else 
        {
            cmd = group->FindCommandNoCase( ptr, pfConsoleCmdGroup::kFindPartial, lastCmd );
            if (cmd == nullptr)
                return false;

            strcat( newStr, cmd->GetName() );
            strcat( newStr, " " );
            lastCmd = cmd;
        }
    }

    if( preserveParams )
    {
        /// Preserve the rest of the string after the matched command
        if (line != nullptr)
            strcat( newStr, line );
    }

    // Copy back!
    strcpy( originalLine, newStr );

    return true;
}

//// FindNestedPartialCmd ////////////////////////////////////////////////////
//  Same as FindPartialCmd, only starts from the global group and searches
//  everything. The string passed should only be a partial command sans
//  groups. numToSkip specifies how many matches to skip before returning one
//  (so if numToSkip = 1, then this will return the second match found).

bool    pfConsoleEngine::FindNestedPartialCmd( char *line, uint32_t numToSkip, bool preserveParams )
{
    pfConsoleCmd        *cmd;

    
    /// Somewhat easier than FindPartialCmd...
    cmd = pfConsoleCmdGroup::GetBaseGroup()->FindNestedPartialCommand( line, &numToSkip );
    if (cmd == nullptr)
        return false;

    /// Recurse back up and get the group hierarchy
    line[ 0 ] = 0;
    IBuildCmdNameRecurse( cmd->GetParent(), line );
    strcat( line, cmd->GetName() );
    strcat( line, " " );

    if( preserveParams )
    {
        /// Preserve the rest of the string after the matched command
    }

    return true;
}

//// IBuildCmdNameRecurse ////////////////////////////////////////////////////

void    pfConsoleEngine::IBuildCmdNameRecurse( pfConsoleCmdGroup *group, char *string )
{
    if (group == nullptr || group == pfConsoleCmdGroup::GetBaseGroup())
        return;


    IBuildCmdNameRecurse( group->GetParent(), string );

    strcat( string, group->GetName() );
    strcat( string, "." );
}
