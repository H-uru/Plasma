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
//  pfConsoleEngine Header                                                  //
//                                                                          //
//// Description /////////////////////////////////////////////////////////////
//                                                                          //
//  The engine is where parsing and execution of console commands takes     //
//  place. It takes place independently of the interface, so that the       //
//  execution can happen from an INI file, from a screen-based console or   //
//  even a GUI interface.                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleEngine_h
#define _pfConsoleEngine_h

#include "HeadSpin.h"

#include <string_theory/string>

class plFileName;

//// pfConsoleEngine Class Definition ////////////////////////////////////////

class pfConsoleCmdParam;
class pfConsoleCmdGroup;
class pfConsoleEngine
{
    private:

        static const int32_t      fMaxNumParams;

        bool IConvertToParam(uint8_t type, ST::string string, pfConsoleCmdParam *param);

        ST::string fErrorMsg;
        ST::string fLastErrorLine;

        // Recursive function to build a string of the groups a command is in
        void        IBuildCmdNameRecurse( pfConsoleCmdGroup *group, char *string );

    public:

        pfConsoleEngine();
        ~pfConsoleEngine();

        // Gets the signature for the command given (NO groups!)
        const char  *GetCmdSignature( char *name );

        // Prints out the help for a given command (or group)
        bool PrintCmdHelp(char *name, void (*PrintFn)(const ST::string&));

        // Breaks the given line into a command and parameters and runs the command
        bool RunCommand(const ST::string& line, void (*PrintFn)(const ST::string&));

        // Executes the given file as a sequence of console commands
        bool    ExecuteFile( const plFileName &fileName );

        // Get the last reported error
        ST::string GetErrorMsg() { return fErrorMsg; }

        // Get the line for which the last reported error was for
        ST::string GetLastErrorLine() { return fLastErrorLine; }

        // Does command completion on a partially-complete console line
        bool        FindPartialCmd( char *line, bool findAgain = false, bool perserveParams = false );

        // Does command completion without restrictions to any group, skipping the number of matches given
        bool        FindNestedPartialCmd( char *line, uint32_t numToSkip, bool perserveParams = false );
};


//// Use in your Main module to provide Console functionality ////////////////

#define PF_CONSOLE_LINK_FILE( name ) \
    void _console_##name##_file_dummy();

#define PF_CONSOLE_INITIALIZE( name ) \
    _console_##name##_file_dummy();

#define PF_CONSOLE_LINK_ALL() \
    PF_CONSOLE_LINK_FILE(Audio) \
    PF_CONSOLE_LINK_FILE(Avatar) \
    PF_CONSOLE_LINK_FILE(Core) \
    PF_CONSOLE_LINK_FILE(Game) \
    PF_CONSOLE_LINK_FILE(Main) \
    PF_CONSOLE_LINK_FILE(Net)

#define PF_CONSOLE_INIT_ALL() \
    PF_CONSOLE_INITIALIZE(Audio) \
    PF_CONSOLE_INITIALIZE(Avatar) \
    PF_CONSOLE_INITIALIZE(Core) \
    PF_CONSOLE_INITIALIZE(Game) \
    PF_CONSOLE_INITIALIZE(Main) \
    PF_CONSOLE_INITIALIZE(Net)


#endif //_pfConsoleEngine_h
