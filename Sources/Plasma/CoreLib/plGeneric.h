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
#ifndef __PLGENERIC_H__
#define __PLGENERIC_H__

#include <string_theory/string>

class hsStream;

class plGeneric
{
public:
    enum GenericType
    {
        kNull,
        kBool,
        kInt,
        kFloat,
        kString
    };

private:
    GenericType     fType;
    bool            fBoolVal;
    int             fIntVal;
    double          fFloatVal;
    ST::string      fStringVal;

    void IReset();

public:
    plGeneric();
    plGeneric(const bool& val);
    plGeneric(const int& val);
    plGeneric(const double& val);
    plGeneric(const ST::string& val);

    void SetToNull() {IReset();}
    plGeneric& operator=(const bool& val);
    plGeneric& operator=(const int& val);
    plGeneric& operator=(const double& val);
    plGeneric& operator=(const ST::string& val);

    // the cast functions will NOT cast from one type to another, use
    // GetType() to determine the type of parameter, then cast it to that type
    GenericType GetType() const {return fType;}
    operator bool() const {return fBoolVal;}
    operator int() const {return fIntVal;}
    operator double() const {return fFloatVal;}
    operator ST::string() const {return fStringVal;}

    int Write(hsStream* stream);
    int Read(hsStream* stream);
};

#endif // __PLGENERIC_H__