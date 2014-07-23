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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtCmd.cpp
*   
***/

#include "pnUtCmd.h"
#include "pnUtMisc.h"


/*****************************************************************************
*
*   Private
*
***/

#define  WHITESPACE     L" \"\t\r\n\x1A"
#define  FLAGS          L"-/"
#define  SEPARATORS     L"=:"
#define  TOGGLES        L"+-"
#define  ALL            WHITESPACE FLAGS SEPARATORS TOGGLES

#if HS_BUILD_FOR_WIN32
static const unsigned kMaxTokenLength = MAX_PATH;
#else
static const unsigned kMaxTokenLength = 1024;
#endif

struct CmdArgData {
    CmdArgDef def;
    union {
        bool          boolVal;
        float         floatVal;
        int           intVal;
        const wchar_t * strVal;
        unsigned      unsignedVal;
    } val;
    wchar_t *  buffer;
    unsigned nameChars;
    bool     isSpecified;

    ~CmdArgData () {
        if (buffer)
            free(buffer);
    }
};

struct CmdTokState {
    CCmdParser * parser;
    unsigned     pendingIndex;
    unsigned     unflaggedIndex;
};

class CICmdParser {
private:
    FARRAYOBJ(CmdArgData)   m_argArray;
    FARRAY(unsigned)        m_idLookupArray;
    unsigned                m_requiredCount;
    FARRAY(unsigned)        m_unflaggedArray;

    inline bool CheckFlag (unsigned flags, unsigned flag, unsigned mask) const;
    void Error (const CmdTokState * state, ECmdError errorCode, const wchar_t arg[], const wchar_t value[]) const;
    bool LookupFlagged (const wchar_t ** name, unsigned * lastIndex) const;
    bool ProcessValue (CmdTokState * state, unsigned index, const wchar_t str[]);
    void SetDefaultValue (CmdArgData & arg);
    bool TokenizeFlags (CmdTokState * state, const wchar_t str[]);

public:
    CICmdParser (const CmdArgDef def[], unsigned defCount);
    bool CheckAllRequiredArguments (CmdTokState * state);
    const CmdArgData * FindArgById (unsigned id) const;
    const CmdArgData * FindArgByName (const wchar_t name[]) const;
    bool Tokenize (CmdTokState * state, const wchar_t str[]);
    
};


/*****************************************************************************
*
*   CICmdParser implementation
*
***/

//===========================================================================
bool CICmdParser::CheckFlag (unsigned flags, unsigned flag, unsigned mask) const {
    return ((flags & mask) == flag);
}

//===========================================================================
bool CICmdParser::CheckAllRequiredArguments (CmdTokState * state) {
    bool result = (state->unflaggedIndex >= m_requiredCount);
    if (!result)
        Error(state, kCmdErrorTooFewArgs, nil, nil);
    return result;
}

//===========================================================================
CICmdParser::CICmdParser (const CmdArgDef def[], unsigned defCount) {
    unsigned loop;

    // Save the argument definitions
    unsigned maxId          = 0;
    unsigned unflaggedCount = 0;
    m_argArray.SetCount(defCount);
    for (loop = 0; loop < defCount; ++loop) {

        // Check whether this argument is flagged
        bool flagged = CheckFlag(def[loop].flags, kCmdArgFlagged, kCmdMaskArg);

        // Disallow names on unflagged arguments
        ASSERT(flagged || !def[loop].name);

        // Store the argument data
        CmdArgData & arg = m_argArray[loop];
        arg.def         = def[loop];
        arg.buffer      = nil;
        arg.nameChars   = def[loop].name ? StrLen(def[loop].name) : 0;
        arg.isSpecified = false;
        SetDefaultValue(arg);
        maxId = std::max(maxId, def[loop].id);

        // Track the number of unflagged arguments
        if (!flagged)
            ++unflaggedCount;

    }

    // Build the id lookup table
    unsigned idTableSize = std::min(maxId + 1, defCount * 2);
    m_idLookupArray.SetCount(idTableSize);
    m_idLookupArray.Zero();
    for (loop = 0; loop < defCount; ++loop)
        if (def[loop].id < idTableSize)
            m_idLookupArray[def[loop].id] = loop;

    // Build the unflagged array
    unsigned unflaggedIndex = 0;
    m_unflaggedArray.SetCount(unflaggedCount);
    for (loop = 0; loop < defCount; ++loop)
        if (CheckFlag(def[loop].flags, kCmdArgRequired, kCmdMaskArg))
            m_unflaggedArray[unflaggedIndex++] = loop;
    m_requiredCount = unflaggedIndex;
    for (loop = 0; loop < defCount; ++loop)
        if (!(CheckFlag(def[loop].flags, kCmdArgFlagged, kCmdMaskArg) ||
              CheckFlag(def[loop].flags, kCmdArgRequired, kCmdMaskArg)))
            m_unflaggedArray[unflaggedIndex++] = loop;

}

//===========================================================================
void CICmdParser::Error (const CmdTokState * state, ECmdError errorCode, const wchar_t arg[], const wchar_t value[]) const {

    // Compose the error text
    // (This text is only provided as a shortcut for trivial applications that
    // don't want to compose their own text. Normally, an application would
    // compose error text using its own localized strings.)
    unsigned chars  = 256 + (arg ? StrLen(arg) : 0) + (value ? StrLen(value) : 0);
    wchar_t *  buffer = (wchar_t *)malloc(chars * sizeof(wchar_t));
    switch (errorCode) {

        case kCmdErrorInvalidArg:
            StrPrintf(buffer, chars, L"Invalid argument: %s", arg);
        break;

        case kCmdErrorInvalidValue:
            StrPrintf(buffer, chars, L"Argument %s invalid value: %s", arg, value);
        break;

        case kCmdErrorTooFewArgs:
            StrPrintf(buffer, chars, L"Too few arguments");
        break;

        case kCmdErrorTooManyArgs:
            StrPrintf(buffer, chars, L"Too many arguments: %s", arg);
        break;

        DEFAULT_FATAL(errorCode);
    }

    // Call the error handler
    state->parser->OnError(buffer, errorCode, arg, value);

    // Free memory
    free(buffer);

}

//===========================================================================
const CmdArgData * CICmdParser::FindArgById (unsigned id) const {

    // Search for the argument with this id
    unsigned index;
    if (id < m_idLookupArray.Count())
        index = m_idLookupArray[id];
    else
        for (index = 0; index < m_argArray.Count(); ++index)
            if (m_argArray[index].def.id == id)
                break;

    // Verify that we found the correct argument
    if ( (index >= m_argArray.Count()) ||
         (m_argArray[index].def.id != id) )
        return nil;

    // Return the argument data
    return &m_argArray[index];

}

//===========================================================================
const CmdArgData * CICmdParser::FindArgByName (const wchar_t name[]) const {

    // Search for an argument with this name
    unsigned index = (unsigned)-1;
    if (!LookupFlagged(&name, &index))
        return nil;

    // Return the argument data
    return &m_argArray[index];

}

//===========================================================================
bool CICmdParser::LookupFlagged (const wchar_t ** name, unsigned * lastIndex) const {
    unsigned argCount  = m_argArray.Count();
    unsigned chars     = StrLen(*name);
    unsigned bestIndex = (unsigned)-1;
    unsigned bestChars = 0;

    // Check whether this argument is a suffix to any previously
    // provided prefix in this token
    for (unsigned prevChars = (*lastIndex != (unsigned)-1) ? m_argArray[*lastIndex].nameChars : 0;
         (prevChars != (unsigned)-1) && !bestChars;
         --prevChars) {
        const CmdArgData & prev = prevChars ? m_argArray[*lastIndex] : *(const CmdArgData *)nil;

        // Find this argument in the list
        for (unsigned index = 0; index < argCount; ++index) {
            const CmdArgData & arg = m_argArray[index];

            // Ignore non-flagged arguments
            if (!CheckFlag(arg.def.flags, kCmdArgFlagged, kCmdMaskArg))
                continue;

            // Ignore this argument if it wouldn't beat the previous best match
            if (arg.nameChars < bestChars + prevChars)
                continue;

            // Ignore this argument if it doesn't match the prefix
            bool caseSensitive = CheckFlag(arg.def.flags, kCmdCaseSensitive, kCmdCaseSensitive);
            if ( prevChars &&
                 ( (prevChars >= arg.nameChars) ||
                   ( caseSensitive && StrCmp(arg.def.name, prev.def.name, prevChars)) ||
                   (!caseSensitive && StrCmpI(arg.def.name, prev.def.name, prevChars)) ) )
                continue;

            // Ignore this argument if it doesn't match the suffix
            if ( ( caseSensitive && StrCmp(*name, arg.def.name + prevChars, arg.nameChars - prevChars)) ||
                 (!caseSensitive && StrCmpI(*name, arg.def.name + prevChars, arg.nameChars - prevChars)) )
                continue;

            // Track the best match
            bestIndex = index;
            bestChars = arg.nameChars - prevChars;
            if (bestChars == chars)
                break;

        }

    }

    // Return the result
    *name      += bestChars;
    *lastIndex  = bestIndex;
    return (bestChars != 0);
}

//===========================================================================
bool CICmdParser::ProcessValue (CmdTokState * state, unsigned index, const wchar_t str[]) {
    CmdArgData & arg = m_argArray[index];
    arg.isSpecified = true;
    unsigned argType = arg.def.flags & kCmdMaskType;
    switch (argType) {

        case kCmdTypeBool:
            if (*str == '+')
                arg.val.boolVal = true;
            else if (*str == '-')
                arg.val.boolVal = false;
            else if (!*str)
                arg.val.boolVal = CheckFlag(arg.def.flags, kCmdBoolSet, kCmdMaskBool);
            else
                Error(state, kCmdErrorInvalidValue, arg.def.name, str);
        break;

        case kCmdTypeFloat:
            {
                const wchar_t * endPtr;
                arg.val.floatVal = StrToFloat(str, &endPtr);
                if (*endPtr)
                    Error(state, kCmdErrorInvalidValue, arg.def.name, str);
            }
        break;

        case kCmdTypeInt:
            {
                const wchar_t * endPtr;
                arg.val.intVal = StrToInt(str, &endPtr);
                if (*endPtr)
                    Error(state, kCmdErrorInvalidValue, arg.def.name, str);
            }
        break;

        case kCmdTypeString:
            if (arg.buffer)
                free(arg.buffer);
            arg.buffer = StrDup(str);
            arg.val.strVal = arg.buffer;
        break;

        case kCmdTypeUnsigned:
            {
                const wchar_t * endPtr;
                arg.val.unsignedVal = StrToUnsigned(str, &endPtr, 10);
                if (*endPtr)
                    Error(state, kCmdErrorInvalidValue, arg.def.name, str);
            }
        break;

        DEFAULT_FATAL(argType);

    }
    return true;
}

//===========================================================================
void CICmdParser::SetDefaultValue (CmdArgData & arg) {
    unsigned argType = arg.def.flags & kCmdMaskType;
    switch (argType) {

        case kCmdTypeBool:
            arg.val.boolVal = !CheckFlag(arg.def.flags, kCmdBoolSet, kCmdMaskBool);
        break;

        case kCmdTypeInt:
            arg.val.intVal = 0;
        break;

        case kCmdTypeUnsigned:
            arg.val.unsignedVal = 0;
        break;

        case kCmdTypeFloat:
            arg.val.floatVal = 0.0f;
        break;

        case kCmdTypeString:
            arg.val.strVal = L"";
        break;

        DEFAULT_FATAL(argType);

    }
}

//===========================================================================
bool CICmdParser::Tokenize (CmdTokState * state, const wchar_t str[]) {
    wchar_t buffer[kMaxTokenLength];
    bool  result = true;
    while (result && StrTokenize(&str, buffer, arrsize(buffer), WHITESPACE)) {

        // If the previous argument is awaiting a value, then use this token
        // as the value
        if (state->pendingIndex != (unsigned)-1) {
            result = ProcessValue(state, state->pendingIndex, buffer);
            state->pendingIndex = (unsigned)-1;
            continue;
        }

        // Identify and process flagged parameters
        if (StrChr(FLAGS, buffer[0]) && TokenizeFlags(state, buffer))
            continue;

        // Process unflagged parameters
        if (state->unflaggedIndex < m_unflaggedArray.Count()) {
            result = ProcessValue(state, m_unflaggedArray[state->unflaggedIndex++], buffer);
            continue;
        }

        // Process extra parameters
        if (state->parser->OnExtra(buffer))
            continue;

        // Process invalid parameters
        Error(state, kCmdErrorTooManyArgs, buffer, nil);
        result = false;
        break;

    }
    return result;
}

//===========================================================================
bool CICmdParser::TokenizeFlags (CmdTokState * state, const wchar_t str[]) {

    // Process each separately flagged token within the string
    wchar_t buffer[kMaxTokenLength];
    bool  result = true;
    while (result && StrTokenize(&str, buffer, arrsize(buffer), ALL)) {
        if (!buffer[0])
            continue;

        // Process each flag within the token
        unsigned      lastIndex = (unsigned)-1;
        const wchar_t * bufferPtr = buffer;
        while (result) {

            // Lookup the argument name
            result = LookupFlagged(&bufferPtr, &lastIndex);
            if (!result) {
                Error(state, kCmdErrorInvalidArg, bufferPtr, nil);
                result = false;
                break;
            }

            // If this argument is boolean, allow it to share a common prefix
            // with the next argument.  In this case there is no place for
            // the user to provide a value, so use the default value.
            if (*bufferPtr && 
                CheckFlag(m_argArray[lastIndex].def.flags, kCmdTypeBool, kCmdMaskType)) {
                result = ProcessValue(state, lastIndex, L"");
                continue;
            }

            break;
        }
        if (!result)
            break;

        // Check for an argument value provided using a separator
        if (*str && StrChr(SEPARATORS, *str)) {
            result = ProcessValue(state, lastIndex, str + 1);
            break;
        }

        // Process values for boolean arguments
        if (CheckFlag(m_argArray[lastIndex].def.flags, kCmdTypeBool, kCmdMaskType)) {

            // Check for a value provided with a toggle
            if (*str && StrChr(TOGGLES, *str)) {
                wchar_t tempStr[] = {*str, 0};
                result = ProcessValue(state, lastIndex, tempStr);
                ++str;
                continue;
            }

            // Check for a default value
            else {
                result = ProcessValue(state, lastIndex, L"");
                continue;
            }

        }

        // Process values for non-boolean arguments
        else {

            // Check for an argument value immediately following the name
            if (*bufferPtr) {
                result = ProcessValue(state, lastIndex, bufferPtr);
                break;
            }

            // Check for an argument value in the next token
            else {
                state->pendingIndex = lastIndex;
                break;
            }

        }

    }
    return result;
}


/*****************************************************************************
*
*   CCmdParser implementation
*
***/

//===========================================================================
CCmdParser::CCmdParser () {
    fParser = nil;
}

//===========================================================================
CCmdParser::CCmdParser (const CmdArgDef def[], unsigned defCount) {
    Initialize(def, defCount);
}

//===========================================================================
CCmdParser::~CCmdParser () {
    delete fParser;
}

//===========================================================================
bool CCmdParser::GetBool (unsigned id) const {
    return fParser->FindArgById(id)->val.boolVal;
}

//===========================================================================
bool CCmdParser::GetBool (const wchar_t name[]) const {
    return fParser->FindArgByName(name)->val.boolVal;
}

//===========================================================================
float CCmdParser::GetFloat (unsigned id) const {
    return fParser->FindArgById(id)->val.floatVal;
}

//===========================================================================
float CCmdParser::GetFloat (const wchar_t name[]) const {
    return fParser->FindArgByName(name)->val.floatVal;
}

//===========================================================================
int CCmdParser::GetInt (unsigned id) const {
    return fParser->FindArgById(id)->val.intVal;
}

//===========================================================================
int CCmdParser::GetInt (const wchar_t name[]) const {
    return fParser->FindArgByName(name)->val.intVal;
}

//===========================================================================
const wchar_t * CCmdParser::GetString (unsigned id) const {
    return fParser->FindArgById(id)->val.strVal;
}

//===========================================================================
const wchar_t * CCmdParser::GetString (const wchar_t name[]) const {
    return fParser->FindArgByName(name)->val.strVal;
}

//===========================================================================
unsigned CCmdParser::GetUnsigned (unsigned id) const {
    return fParser->FindArgById(id)->val.unsignedVal;
}

//===========================================================================
unsigned CCmdParser::GetUnsigned (const wchar_t name[]) const {
    return fParser->FindArgByName(name)->val.unsignedVal;
}

//===========================================================================
void CCmdParser::Initialize (const CmdArgDef def[], unsigned defCount) {
    fParser = new CICmdParser(def, defCount);
}

//===========================================================================
bool CCmdParser::IsSpecified (unsigned id) const {
    if (const CmdArgData * data = fParser->FindArgById(id))
        return data->isSpecified;
    return false;
}

//===========================================================================
bool CCmdParser::IsSpecified (const wchar_t name[]) const {
    if (const CmdArgData * data = fParser->FindArgByName(name))
        return data->isSpecified;
    return false;
}

//===========================================================================
void CCmdParser::OnError (const wchar_t str[], ECmdError errorCode, const wchar_t arg[], const wchar_t value[]) {
}

//===========================================================================
bool CCmdParser::OnExtra (const wchar_t str[]) {
    return false;
}

//===========================================================================
bool CCmdParser::Parse (const wchar_t cmdLine[]) {
    // If no command line was passed, use the application's command line,
    // skipping past the program name
    if (!cmdLine) {
        cmdLine = AppGetCommandLine();
        StrTokenize(&cmdLine, nil, 0, WHITESPACE);
        while (*cmdLine == L' ')
            ++cmdLine;
    }

    // Process the command line
    CmdTokState state = {
        this,
        (unsigned)-1,  // pending index
        0              // unflagged index
    };
    bool result;
    result = fParser->Tokenize(&state, cmdLine);
    if (result)
        result = fParser->CheckAllRequiredArguments(&state);

    return result;
}
