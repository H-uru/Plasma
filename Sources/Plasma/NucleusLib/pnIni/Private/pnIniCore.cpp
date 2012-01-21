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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Private/pnIniCore.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static wchar_t * TrimWhitespace (wchar_t * name) {
    while (isspace((char) *name))
        ++name;

    for (wchar_t * term = name; *term; ++term) {
        if (isspace((char) *term)) {
            *term = 0;
            break;
        }
    }

    return name;
}


/****************************************************************************
*
*   IniValue
*
***/

//===========================================================================
IniValue::IniValue (IniKey * key, unsigned lineNum)
:   fKey(key)
,   fLineNum(lineNum)
{
    fIndex = key->fValues.Add(this);
}

//===========================================================================
IniValue::~IniValue () {
    wchar_t ** cur = fArgs.Ptr();
    wchar_t ** end = fArgs.Term();
    for (; cur < end; ++cur)
        free(*cur);
}

//===========================================================================
static void AddValueString (
    IniValue *  value,
    const wchar_t src[]
) {
    unsigned chars = StrLen(src) + 1;
    wchar_t * dst = (wchar_t*)malloc(sizeof(wchar_t) * chars);
    StrTokenize(&src, dst, chars, L" \t\r\n\"");
    value->fArgs.Add(StrDup(dst));

    free(dst);
}


/****************************************************************************
*
*   IniKey
*
***/

//===========================================================================
IniKey::IniKey (IniSection * section, const wchar_t name[])
:   fSection(section)
{
    StrCopy(fName, name, (unsigned) -1);
    fSection->fKeys.Add(this);
}

//===========================================================================
IniKey::~IniKey () {
    IniValue ** cur = fValues.Ptr();
    IniValue ** end = fValues.Term();
    for (; cur < end; ++cur)
        delete *cur;
}

//===========================================================================
inline unsigned IniKey::GetHash () const {
    return StrHashI(fName);
}

//===========================================================================
inline bool IniKey::operator== (const CHashKeyStrPtrI & rhs) const {
    return !StrCmpI(fName, rhs.GetString());
}

//===========================================================================
static IniValue * AddKeyValue (
    IniSection *    section,
    wchar_t *         string,
    unsigned        lineNum
) {
    string = TrimWhitespace(string);

    // Find or create the key
    IniKey * key = section->fKeys.Find(string);
    if (!key) {
        key = new(malloc(
            sizeof(*key) - sizeof(key->fName) + StrBytes(string)
        )) IniKey(section, string);
    }

    // Add a new value holder for the key
    return new IniValue(key, lineNum);
}


/****************************************************************************
*
*   IniSection
*
***/

//===========================================================================
IniSection::IniSection (const wchar_t name[]) {
    StrCopy(fName, name, (unsigned) -1);
}

//===========================================================================
IniSection::~IniSection () {
    fKeys.Clear();
}

//===========================================================================
inline unsigned IniSection::GetHash () const {
    return StrHashI(fName);
}

//===========================================================================
inline bool IniSection::operator== (const CHashKeyStrPtrI & rhs) const {
    return !StrCmpI(fName, rhs.GetString());
}

//===========================================================================
static IniSection * AddSection (
    Ini *       ini,
    wchar_t *     string
) {
    // Find or create the section
    IniSection * section = ini->fSections.Find(string);
    if (!section) {
        section = new(malloc(
            sizeof(*section) - sizeof(section->fName) + StrBytes(string)
        )) IniSection(string);
        ini->fSections.Add(section);
    }
    return section;
}


/****************************************************************************
*
*   Ini
*
***/

//===========================================================================
Ini::~Ini () {
    fSections.Clear();
}


/****************************************************************************
*
*   ParseBuffer
*
***/

//===========================================================================
static void ParseBuffer (
    Ini *       ini,
    const wchar_t buffer[]
) {

    const wchar_t SECTION_OPEN_CHAR  = '[';
    const wchar_t SECTION_CLOSE_CHAR = ']';
    const wchar_t EQUIVALENCE_CHAR   = '=';
    const wchar_t VALUE_SEPARATOR    = ',';
    const wchar_t COMMENT_CHAR       = ';';
    const wchar_t QUOTE_CHAR         = '\"';
    const wchar_t NEWLINE            = '\n';

    enum {
        STATE_BEGIN,
        STATE_NEWLINE,
        STATE_SECTION,
        STATE_STRIP_TRAILING,
        STATE_KEY,
        STATE_VALUE,
    } state = STATE_BEGIN;

    IniSection * section    = nil;
    IniValue * value        = nil;
    const wchar_t * start     = nil;
    bool valInQuotes        = false;
    wchar_t dst[512];
    dst[0] = 0;

    for (unsigned lineNum = 1;; ++buffer) {

        // Get next character
        unsigned chr = *buffer;
        if (!chr)
            break;
        if (chr == '\r')
            continue;
        if (chr == '\n')
            ++lineNum;
        if (chr == '\t')
            chr = ' ';

        switch (state) {
            case STATE_BEGIN:
                ASSERT(chr == UNICODE_BOM);
                state = STATE_NEWLINE;
            break;

            case STATE_NEWLINE:
                if (chr == NEWLINE)
                    break;
                if (chr == ' ')
                    break;
                if (chr == SECTION_OPEN_CHAR) {
                    start = buffer + 1;
                    state = STATE_SECTION;
                }
                else if (chr == COMMENT_CHAR) {
                    state = STATE_STRIP_TRAILING;
                }
                else {
                    start = buffer;
                    state = STATE_KEY;
                }
            break;

            case STATE_SECTION:
                if (chr == NEWLINE) {
                    state = STATE_NEWLINE;
                    break;
                }
                
                if (chr == SECTION_CLOSE_CHAR) {
                    StrCopy(dst, start, min(buffer - start + 1, arrsize(dst)));
                    section = AddSection(ini, dst);
                    state = STATE_STRIP_TRAILING;
                }
            break;

            case STATE_STRIP_TRAILING:
                if (chr == NEWLINE)
                    state = STATE_NEWLINE;
            break;

            case STATE_KEY:
                if (chr == NEWLINE) {
                    state = STATE_NEWLINE;
                    break;
                }
                if (chr != EQUIVALENCE_CHAR)
                    break;

                if (!section) {
                    state = STATE_STRIP_TRAILING;
                    break;
                }

                StrCopy(dst, start, min(buffer - start + 1, arrsize(dst)));
                value = AddKeyValue(section, dst, lineNum);
                start = buffer + 1;
                state = STATE_VALUE;
                valInQuotes = false;
            break;

            case STATE_VALUE:
                if (chr == QUOTE_CHAR)
                    valInQuotes = !valInQuotes;
                if ((valInQuotes || chr != VALUE_SEPARATOR) && (chr != NEWLINE))
                    break;
                if (!value) {
                    state = chr == NEWLINE ? STATE_NEWLINE : STATE_STRIP_TRAILING;
                    break;
                }
                if (valInQuotes) {
                    state = chr == NEWLINE ? STATE_NEWLINE : STATE_STRIP_TRAILING;
                    break;
                }

                StrCopy(dst, start, min(buffer - start + 1, arrsize(dst)));
                AddValueString(value, dst);
                if (chr == VALUE_SEPARATOR)
                    start = buffer + 1;
                else
                    state = STATE_NEWLINE;
            break;
        }
    }

    // cleanup current value
    if (state == STATE_VALUE) {
        StrCopy(dst, start, min(buffer - start + 1, arrsize(dst)));
        AddValueString(value, dst);
    }
}


/****************************************************************************
*
*   ParseFile
*
***/

//===========================================================================
static void IniFileNotifyProc (
    AsyncFile         ,
    EAsyncNotifyFile  ,
    AsyncNotifyFile * ,
    void **
) {
}

//===========================================================================
static bool ParseFile (
    Ini *       ini,
    const wchar_t fileName[]
) {
    // Open file
    uint64_t fileSize;
    uint64_t fileLastWriteTime;
    EFileError error;
    AsyncFile file = AsyncFileOpen(
        fileName,
        IniFileNotifyProc,
        &error,
        kAsyncFileReadAccess,
        kAsyncFileModeOpenExisting,
        kAsyncFileShareRead,
        nil,
        &fileSize,
        &fileLastWriteTime
    );
    if (!file)
        return false;

    bool result;
    if (fileSize > 256 * 1024) {
        result = false;
    }
    else if (!fileSize) {
        result = true;
    }
    else {
        // Read entire file into memory and NULL terminate wchar_t
        uint8_t * buffer = (uint8_t *) malloc((unsigned) fileSize + sizeof(wchar_t));
        AsyncFileRead(file, 0, buffer, (unsigned) fileSize, kAsyncFileRwSync, nil);
        * (wchar_t *) &buffer[fileSize] = 0;

        // Convert to unicode if necessary
        if (* (wchar_t *) buffer != UNICODE_BOM) {
            uint8_t * src  = buffer;
            // Allocate two extra spaces for UNICODE_BOM and terminator
            unsigned newBufferSize = ((unsigned) fileSize + 2) * sizeof(wchar_t);

            // Allocate new buffer
            wchar_t * dst = (wchar_t *) malloc(newBufferSize);
            
            // If it's UTF-8 file,convert to Unicode
            if (StrCmpI((char *)buffer, UTF8_BOM, StrLen(UTF8_BOM)) == 0) {
                // StrUtf8ToUnicode will convert UTF8_BOM to UNICODE_BOM
                StrUtf8ToUnicode(dst, (char *)src, newBufferSize);
            }
            else {
                // do simple conversion to Unicode
                dst[0] = UNICODE_BOM;
                for (unsigned index = 0;; ++index) {
                    if (0 == (dst[index + 1] = src[index]))
                        break;
                }
            }

            free(src);
            buffer = (uint8_t *) dst;
        }

        ParseBuffer(ini, (const wchar_t *) buffer);
        free(buffer);
        result = true;
    }

    AsyncFileClose(file, kAsyncFileDontTruncate);
    return result;
}


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
Ini * IniOpen (
    const wchar_t fileName[]
) {
    Ini * ini = new Ini;
    if (!ParseFile(ini, fileName)) {
        IniClose(ini);
        return nil;
    }
    return ini;
}

//===========================================================================
void IniClose (Ini * ini) {
    delete ini;
}

//===========================================================================
const IniSection * IniGetFirstSection (
    const Ini *         ini,
    wchar_t *             name,
    unsigned            chars
) {
    if (chars)
        *name = 0;
    if (!ini)
        return nil;

    const IniSection * section = ini->fSections.Head();
    if (section)
        StrCopy(name, section->fName, chars);
    return section;
}

//===========================================================================
const IniSection * IniGetNextSection (
    const IniSection *  section,
    wchar_t *             name,
    unsigned            chars
) {
    if (chars)
        *name = 0;
    if (!section)
        return nil;

    section = section->fLink.Next();
    if (section)
        StrCopy(name, section->fName, chars);
    return section;
}

//===========================================================================
const IniSection * IniGetSection (
    const Ini *         ini,
    const wchar_t         name[]
) {
    if (!ini)
        return nil;

    return ini->fSections.Find(
        CHashKeyStrPtrI(name)
    );
}

//===========================================================================
const IniKey * IniGetFirstKey (
    const IniSection *  section,
    wchar_t *             name,
    unsigned            chars
) {
    if (chars)
        *name = 0;
    if (!section)
        return nil;

    const IniKey * key = section->fKeys.Head();
    if (key)
        StrCopy(name, key->fName, chars);
    return key;
}

//============================================================================
const IniKey * IniGetFirstKey (
    const Ini *         ini,
    const wchar_t         sectionName[],
    wchar_t *             name,
    unsigned            chars
) {
    if (const IniSection * section = IniGetSection(ini, sectionName))
        return IniGetFirstKey(section, name, chars);

    return nil;
}

//===========================================================================
const IniKey * IniGetNextKey (
    const IniKey *      key,
    wchar_t *             name,
    unsigned            chars
) {
    if (chars)
        *name = 0;
    if (!key)
        return nil;

    key = key->fLink.Next();
    if (key)
        StrCopy(name, key->fName, chars);
    return key;
}

//===========================================================================
const IniKey * IniGetKey (
    const IniSection *  section,
    const wchar_t         name[]
) {
    if (!section)
        return nil;

    return section->fKeys.Find(
        CHashKeyStrPtrI(name)
    );
}

//===========================================================================
const IniValue * IniGetFirstValue (
    const IniKey *      key,
    unsigned *          lineNum
) {
    if (lineNum)
        *lineNum = 0;

    const IniValue * value = nil;
    for (;;) {
        if (!key)
            break;

        value = key->fValues[0];
        if (lineNum)
            *lineNum = value->fLineNum;
        break;
    }

    return value;
}

//===========================================================================
const IniValue * IniGetFirstValue (
    const IniSection *  section,
    const wchar_t         keyName[],
    unsigned *          lineNum
) {
    const IniValue * value = nil;
    if (lineNum)
        *lineNum = 0;

    for (;;) {
        if (!section)
            break;

        const IniKey * key = section->fKeys.Find(
            CHashKeyStrPtrI(keyName)
        );
        value = IniGetFirstValue(key, lineNum);
        break;
    }

    return value;
}

//===========================================================================
const IniValue * IniGetFirstValue (
    const Ini *     ini,
    const wchar_t     sectionName[],
    const wchar_t     keyName[],
    unsigned *      lineNum
) {
    const IniValue * value = nil;
    if (lineNum)
        *lineNum = 0;

    for (;;) {
        if (!ini)
            break;

        const IniSection * section = ini->fSections.Find(
            CHashKeyStrPtrI(sectionName)
        );

        value = IniGetFirstValue(section, keyName, lineNum);
        break;
    }

    return value;
}

//===========================================================================
const IniValue * IniGetNextValue (
    const IniValue *    value,
    unsigned *          lineNum
) {
    if (lineNum)
        *lineNum = 0;

    const IniKey * key = value->fKey;
    if (value->fIndex + 1 < key->fValues.Count()) {
        value = key->fValues[value->fIndex + 1];
        if (lineNum)
            *lineNum = value->fLineNum;
    }
    else {
        value = nil;
    }

    return value;
}

//===========================================================================
bool IniGetUnsigned (
    const IniValue *    value,
    unsigned *          result,
    unsigned            index,
    unsigned            defaultValue
) {
    ASSERT(result);

    for (;;) {
        if (!value)
            break;
            
        wchar_t str[32];
        if (!IniGetString(value, str, arrsize(str), index, nil))
            break;

        if (!str[0])
            break;

        *result = StrToUnsigned(str, nil, 0);
        return true;
    }

    *result = defaultValue;
    return false;
}

//===========================================================================
bool IniGetString (
    const IniValue *    value,
    wchar_t *             result,
    unsigned            resultChars,
    unsigned            index,
    const wchar_t         defaultValue[]
) {
    ASSERT(result);

    bool found = value && index < value->fArgs.Count();

    if (found)
        StrCopy(result, value->fArgs[index], resultChars);
    else if (defaultValue)
        StrCopy(result, defaultValue, resultChars);

    return found;
}

//===========================================================================
bool IniGetUuid (
    const IniValue *    value,
    Uuid *              uuid,
    unsigned            index,
    const Uuid &        defaultValue
) {
    wchar_t str[128];
    if (IniGetString(value, str, arrsize(str), index, nil))
        return GuidFromString(str, uuid);
    else
        *uuid = defaultValue;

    return false;
}

//===========================================================================
unsigned IniGetBoundedValue (
    const IniValue *    value,
    const wchar_t         section[],
    const wchar_t         key[],
    unsigned            index,
    unsigned            minVal,
    unsigned            maxVal,
    unsigned            defVal
) {
    if (!value)
        return defVal;

    unsigned result;
    IniGetUnsigned(value, &result, index, defVal);
    if ((result < minVal) || (result > maxVal)) {
        result = defVal;
    }

    return result;
}

//===========================================================================
unsigned IniGetBoundedValue (
    const Ini * ini,
    const wchar_t section[],
    const wchar_t key[],
    unsigned    index,
    unsigned    minVal,
    unsigned    maxVal,
    unsigned    defVal
) {
    unsigned lineNum;
    const IniValue * value = IniGetFirstValue(
        ini,
        section,
        key,
        &lineNum
    );

    return IniGetBoundedValue(
        value,
        section,
        key,
        index,
        minVal,
        maxVal,
        defVal
    );
}
