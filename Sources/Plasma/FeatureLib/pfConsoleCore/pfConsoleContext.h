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
//  pfConsoleContext Header                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleContext_h
#define _pfConsoleContext_h

#include "HeadSpin.h"
#include "pfConsoleCmd.h"

#include <string_theory/string>

//// Class Definition ////////////////////////////////////////////////////////

class pfConsoleContext
{
    protected:

        bool    fAddWhenNotFound;       // Controls whether we add variables on Set() calls if they're not found

        ST::string fName;

        std::vector<ST::string> fVarNames;
        std::vector<pfConsoleCmdParam> fVarValues;

        void IAddVar(ST::string name, const pfConsoleCmdParam &value);

        static pfConsoleContext fRootContext;

    public:

        pfConsoleContext(ST::string name);

        void    Clear();

        size_t              GetNumVars() const;
        ST::string GetVarName(size_t idx) const;
        const pfConsoleCmdParam   &GetVarValue(size_t idx) const;

        hsSsize_t FindVar(const ST::string& name) const;
        void    RemoveVar(size_t idx);

        void AddVar(ST::string name, const pfConsoleCmdParam& value);
        void AddVar(ST::string name, int value);
        void AddVar(ST::string name, float value);
        void AddVar(ST::string name, ST::string value);
        void AddVar(ST::string name, char value);
        void AddVar(ST::string name, bool value);

        bool    SetVar(size_t idx, const pfConsoleCmdParam &value);

        bool SetVar(const ST::string& name, const pfConsoleCmdParam& value);
        bool SetVar(const ST::string& name, int value);
        bool SetVar(const ST::string& name, float value);
        bool SetVar(const ST::string& name, ST::string value);
        bool SetVar(const ST::string& name, char value);
        bool SetVar(const ST::string& name, bool value);

        // Decide whether Sets() on nonexistant variables will fail or add a new variable
        void    SetAddWhenNotFound( bool f ) { fAddWhenNotFound = f; }
        bool    GetAddWhenNotFound() const { return fAddWhenNotFound; }

        static pfConsoleContext &GetRootContext();
};


#endif //_pfConsoleContext_h

