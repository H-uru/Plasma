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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Private/pnIniCore.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_PRIVATE_PNINICORE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Private/pnIniCore.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_PRIVATE_PNINICORE_H


/*****************************************************************************
*
*   Ini file parsing functions
*
***/

struct Ini;
struct IniKey;
struct IniValue;
struct IniSection;


// File
Ini * IniOpen (
    const wchar_t         filename[]
);
void IniClose (
    Ini *               ini
);

// Section
const IniSection * IniGetFirstSection (
    const Ini *         ini,
    wchar_t *             name,
    unsigned            chars
);
const IniSection * IniGetNextSection (
    const IniSection *  section,
    wchar_t *             name,
    unsigned            chars
);
const IniSection * IniGetSection (
    const Ini *         ini,
    const wchar_t         name[]
);

// Key
const IniKey * IniGetFirstKey (
    const IniSection *  section,
    wchar_t *             name,
    unsigned            chars
);
const IniKey * IniGetFirstKey (
    const Ini *         ini,
    const wchar_t         sectionName[],
    wchar_t *             name,
    unsigned            chars
);
const IniKey * IniGetNextKey (
    const IniKey *      key,
    wchar_t *             name,
    unsigned            chars
);
const IniKey * IniGetKey (
    const IniSection *  ini,
    const wchar_t         name[]
);

// Value
const IniValue * IniGetFirstValue (
    const IniKey *      key,
    unsigned *          iter
);
const IniValue * IniGetFirstValue (
    const IniSection *  section,
    const wchar_t         keyName[],
    unsigned *          iter
);
const IniValue * IniGetFirstValue (
    const Ini *         ini,
    const wchar_t         sectionName[],
    const wchar_t         keyName[],
    unsigned *          iter
);
const IniValue * IniGetNextValue (
    const IniValue *    value,
    unsigned *          iter
);

// Data
bool IniGetUnsigned (
    const IniValue *    value,
    unsigned *          result,
    unsigned            index = 0,
    unsigned            defaultValue = 0
);
bool IniGetString (
    const IniValue *    value,
    wchar_t *             result,
    unsigned            resultChars,
    unsigned            index = 0,
    const wchar_t         defaultValue[] = nil
);
bool IniGetUuid (
    const IniValue *    value,
    Uuid *              result,
    unsigned            index = 0,
    const Uuid &        defaultValue = kNilGuid
);

// Bounded values
unsigned IniGetBoundedValue (
    const Ini *         ini,
    const wchar_t         sectionName[],
    const wchar_t         keyName[],
    unsigned            index,
    unsigned            minVal,
    unsigned            maxVal,
    unsigned            defVal
);
unsigned IniGetBoundedValue (
    const IniValue *    value,
    unsigned            index,
    unsigned            minVal,
    unsigned            maxVal,
    unsigned            defVal
);
