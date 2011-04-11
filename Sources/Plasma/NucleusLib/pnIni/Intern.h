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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Intern.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_INTERN_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Intern.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_INTERN_H


/*****************************************************************************
*
*   Ini
*
***/

struct IniValue {
    ARRAY(wchar *)  fArgs;
    IniKey *        fKey;
    unsigned        fIndex;
    unsigned        fLineNum;

    IniValue (IniKey * key, unsigned lineNum);
    ~IniValue ();
};

struct IniKey {
    HASHLINK(IniKey)  fLink;
    ARRAY(IniValue *) fValues;
    IniSection *      fSection;
    wchar             fName[1]; // variable length
    // no more fields

    IniKey (IniSection * section, const wchar name[]);
    ~IniKey ();

    unsigned GetHash () const;
    bool operator== (const CHashKeyStrPtrI & rhs) const;
};

struct IniSection {
    HASHTABLEDECL(IniKey, CHashKeyStrPtrI, fLink) fKeys;
    HASHLINK(IniSection)  fLink;
    wchar                 fName[1]; // variable length
    // no more fields

    IniSection (const wchar name[]);
    ~IniSection ();

    unsigned GetHash () const;
    bool operator== (const CHashKeyStrPtrI & rhs) const;
};

struct Ini {
    HASHTABLEDECL(IniSection, CHashKeyStrPtrI, fLink) fSections;

    ~Ini ();
};
