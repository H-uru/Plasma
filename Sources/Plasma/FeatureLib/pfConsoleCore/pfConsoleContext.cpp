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
//  pfConsoleContext                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleContext.h"

#include <string_theory/string>
#include <utility>

//// Static Root Context /////////////////////////////////////////////////////

pfConsoleContext    pfConsoleContext::fRootContext(ST_LITERAL("global"));

pfConsoleContext    &pfConsoleContext::GetRootContext()
{
    return fRootContext;
}

//// Constructor /////////////////////////////////////////////////////////////

pfConsoleContext::pfConsoleContext(ST::string name)
{
    fName = std::move(name);
    fAddWhenNotFound = true;
}

//// Clear ///////////////////////////////////////////////////////////////////

void    pfConsoleContext::Clear()
{
    fVarNames.clear();
    fVarValues.clear();
}

//// Getters /////////////////////////////////////////////////////////////////

size_t pfConsoleContext::GetNumVars() const
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");
    return fVarValues.size();
}

ST::string pfConsoleContext::GetVarName(size_t idx) const
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");

    if (idx >= fVarNames.size())
    {
        hsAssert( false, "GetVarName() index out of range for console context" );
        return {};
    }

    return fVarNames[ idx ];
}

const pfConsoleCmdParam &pfConsoleContext::GetVarValue(size_t idx) const
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");
    hsAssert(idx < fVarValues.size(), "GetVarValue() index out of range for console context");

    return fVarValues[ idx ];
}


//// FindVar /////////////////////////////////////////////////////////////////

hsSsize_t pfConsoleContext::FindVar(const ST::string& name) const
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");

    for (size_t idx = 0; idx < fVarNames.size(); idx++)
    {
        if (name.compare_i(fVarNames[idx]) == 0)
        {
            return hsSsize_t(idx);
        }
    }

    return -1;
}

//// RemoveVar ///////////////////////////////////////////////////////////////

void    pfConsoleContext::RemoveVar(size_t idx)
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");

    if (idx >= fVarValues.size())
    {
        hsAssert( false, "RemoveVar() index out of range for console context" );
        return;
    }

    fVarNames.erase(fVarNames.begin() + idx);
    fVarValues.erase(fVarValues.begin() + idx);
}

//// AddVar Variants /////////////////////////////////////////////////////////

void pfConsoleContext::IAddVar(ST::string name, const pfConsoleCmdParam& value)
{
    fVarNames.emplace_back(std::move(name));
    fVarValues.emplace_back(value);
}

void pfConsoleContext::AddVar(ST::string name, const pfConsoleCmdParam& value)
{
    hsSsize_t idx = FindVar(name);
    if( idx != -1 )
    {
        hsAssert( false, "AddVar() failed because variable already in console context" );
        return;
    }

    IAddVar(std::move(name), value);
}

void pfConsoleContext::AddVar(ST::string name, int value)
{
    pfConsoleCmdParam   param;
    param.SetInt( value );
    AddVar(std::move(name), param);
}

void pfConsoleContext::AddVar(ST::string name, float value)
{
    pfConsoleCmdParam   param;
    param.SetFloat( value );
    AddVar(std::move(name), param);
}

void pfConsoleContext::AddVar(ST::string name, ST::string value)
{
    pfConsoleCmdParam   param;
    param.SetString(std::move(value));
    AddVar(std::move(name), param);
}

void pfConsoleContext::AddVar(ST::string name, char value)
{
    pfConsoleCmdParam   param;
    param.SetChar( value );
    AddVar(std::move(name), param);
}

void pfConsoleContext::AddVar(ST::string name, bool value)
{
    pfConsoleCmdParam   param;
    param.SetBool( value );
    AddVar(std::move(name), param);
}

//// SetVar Variants /////////////////////////////////////////////////////////

bool    pfConsoleContext::SetVar(size_t idx, const pfConsoleCmdParam &value)
{
    hsAssert(fVarValues.size() == fVarNames.size(), "Mismatch in console var context arrays");
    if (idx >= fVarValues.size())
    {
        hsAssert( false, "SetVar() index out of range for console context" );
        return false;
    }

    fVarValues[ idx ] = value;

    return true;
}

bool pfConsoleContext::SetVar(const ST::string& name, const pfConsoleCmdParam& value)
{
    hsSsize_t idx = FindVar(name);
    if( idx == -1 )
    {
        if( fAddWhenNotFound )
        {
            // Don't panic, just add
            IAddVar( name, value );
            return true;
        }
        return false;
    }

    return SetVar((size_t)idx, value);
}

bool pfConsoleContext::SetVar(const ST::string& name, int value)
{
    pfConsoleCmdParam   param;
    param.SetInt( value );
    return SetVar( name, param );
}

bool pfConsoleContext::SetVar(const ST::string& name, float value)
{
    pfConsoleCmdParam   param;
    param.SetFloat( value );
    return SetVar( name, param );
}

bool pfConsoleContext::SetVar(const ST::string& name, ST::string value)
{
    pfConsoleCmdParam   param;
    param.SetString(std::move(value));
    return SetVar( name, param );
}

bool pfConsoleContext::SetVar(const ST::string& name, char value)
{
    pfConsoleCmdParam   param;
    param.SetChar( value );
    return SetVar( name, param );
}

bool pfConsoleContext::SetVar(const ST::string& name, bool value)
{
    pfConsoleCmdParam   param;
    param.SetBool( value );
    return SetVar( name, param );
}

