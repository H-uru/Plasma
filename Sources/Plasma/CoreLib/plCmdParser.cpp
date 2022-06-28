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

#include "plCmdParser.h"

#include "HeadSpin.h"

#include <algorithm>
#include <regex>
#include <vector>

#define  WHITESPACE     " \"\t\r\n\x1A"
#define  FLAGS          "-/"
#define  SEPARATORS     "=:"
#define  TOGGLES        "+-"
#define  ALL            WHITESPACE FLAGS SEPARATORS TOGGLES

#define hsCheckFlagBits(f,c,m) ((f & m)==c)


struct plCmdArgData
{
    plCmdArgDef def;

    union {
        bool        boolVal;
        float       floatVal;
        int32_t     intVal;
        const char* strVal;
        uint32_t    uintVal;
    } val;

    ST::string  buffer;
    size_t      nameChars;
    bool        isSpecified;
};

struct plCmdTokenState {
    size_t fPendingIndex;
    size_t fUnflaggedIndex;
};


class plCmdParserImpl
{
protected:
    ST::string                  fProgramName;
    std::vector<plCmdArgData>   fArgArray;
    std::vector<size_t>         fLookupArray;
    std::vector<size_t>         fUnflaggedArray;
    size_t                      fRequiredCount;
    CmdError                    fError;

    void SetDefaultValue(plCmdArgData& arg);
    bool ProcessValue(plCmdTokenState* state, size_t index, const ST::string& str);
    bool TokenizeFlags(plCmdTokenState* state, const ST::string& str);
    bool LookupFlagged(ST::string& name, size_t* lastIndex, bool force=false) const;

public:
    plCmdParserImpl(const plCmdArgDef* defs, size_t defCount);

    bool Tokenize(plCmdTokenState* state, std::vector<ST::string>& strs);
    const plCmdArgData* FindArgByName(const ST::string& name) const;
    const plCmdArgData* FindArgById(size_t id) const;
    bool CheckAllRequiredArguments(plCmdTokenState* state);

    const ST::string GetProgramName() const { return fProgramName; }

    CmdError GetError() const { return fError; }
};


plCmdParserImpl::plCmdParserImpl(const plCmdArgDef* defs, size_t defCount)
{
    size_t loop;
    fError = kCmdErrorSuccess;

    // Save the argument definitions
    size_t maxId          = 0;
    size_t unflaggedCount = 0;

    fArgArray.resize(defCount);

    for (loop = 0; loop < defCount; ++loop) {
        plCmdArgDef def = defs[loop];

        // Check whether this argument is flagged
        bool flagged = hsCheckFlagBits(def.flags,
                                       kCmdArgFlagged,
                                       kCmdArgMask);

        // Disallow names on unflagged arguments
        ASSERT(flagged || !def.name.empty());

        // Store the argument data
        plCmdArgData& arg = fArgArray[loop];
        arg.def           = def;
        arg.buffer        = ST::string();
        arg.nameChars     = def.name.size();
        arg.isSpecified   = false;

        SetDefaultValue(arg);
        maxId = std::max(maxId, def.id);

        // Track the number of unflagged arguments
        if (!flagged) {
            ++unflaggedCount;
        }
    }


    // Build the id lookup table
    size_t idTableSize = std::min(maxId + 1, defCount * 2);
    fLookupArray.resize(idTableSize);

    for (loop = 0; loop < defCount; ++loop) {
        if (defs[loop].id < idTableSize) {
            fLookupArray[defs[loop].id] = loop;
        }
    }


    // Build the unflagged array
    size_t unflaggedIndex = 0;
    fUnflaggedArray.resize(unflaggedCount);

    for (loop = 0; loop < defCount; ++loop) {
        bool req = hsCheckFlagBits(defs[loop].flags,
                                   kCmdArgRequired,
                                   kCmdArgMask);

        if (req) {
            fUnflaggedArray[unflaggedIndex++] = loop;
        }
    }

    fRequiredCount = unflaggedIndex;

    for (loop = 0; loop < defCount; ++loop) {
        bool flagged = hsCheckFlagBits(defs[loop].flags,
                                       kCmdArgFlagged,
                                       kCmdArgMask);

        bool req     = hsCheckFlagBits(defs[loop].flags,
                                       kCmdArgRequired,
                                       kCmdArgMask);

        if (!flagged && !req) {
            fUnflaggedArray[unflaggedIndex++] = loop;
        }
    }
}


void plCmdParserImpl::SetDefaultValue(plCmdArgData& arg)
{
    uint32_t argType = arg.def.flags & kCmdTypeMask;

    switch (argType) {
    case kCmdTypeBool:
        arg.val.boolVal = !hsCheckFlagBits(arg.def.flags,
                                           kCmdBoolSet,
                                           kCmdBoolMask);
        break;

    case kCmdTypeInt:
        arg.val.intVal = 0;
        break;

    case kCmdTypeUint:
        arg.val.uintVal = 0;
        break;

    case kCmdTypeFloat:
        arg.val.floatVal = 0.0f;
        break;

    case kCmdTypeString:
        arg.val.strVal = "";
        break;

    DEFAULT_FATAL(argType);
    }
}


bool plCmdParserImpl::Tokenize(plCmdTokenState* state, std::vector<ST::string>& strs)
{
    bool result = true;

    for (auto it = strs.begin(); result && it != strs.end(); ++it) {
        if (fProgramName.empty()) {
            fProgramName = *it;
            continue;
        }

        // If the previous argument is awaiting a value, then use this token
        // as the value
        if (state->fPendingIndex != size_t(-1)) {
            result = ProcessValue(state, state->fPendingIndex, *it);
            state->fPendingIndex = size_t(-1);
            continue;
        }

        // Identify and process flagged parameters
        static const std::regex re_flags("[" FLAGS "].+");
        if (std::regex_match(it->c_str(), re_flags) && TokenizeFlags(state, *it)) {
            continue;
        }

        // Process unflagged parameters
        if (state->fUnflaggedIndex < fUnflaggedArray.size()) {
            result = ProcessValue(state, fUnflaggedArray[state->fUnflaggedIndex++], *it);
            continue;
        }

        // Process invalid parameters
        if (!fError) {
            fError = kCmdErrorTooManyArgs;
        }
        result = false;
        break;
    }

    return result;
}


bool plCmdParserImpl::ProcessValue(plCmdTokenState* state, size_t index, const ST::string& str)
{
    plCmdArgData& arg = fArgArray[index];
    arg.isSpecified = true;
    uint32_t argType = arg.def.flags & kCmdTypeMask;

    switch (argType) {
    case kCmdTypeBool:
        if (str.compare_i("true") == 0)
            arg.val.boolVal = true;
        else if (str.compare_i("false") == 0)
            arg.val.boolVal = false;
        else if (str.empty())
            arg.val.boolVal = hsCheckFlagBits(arg.def.flags,
                                              kCmdBoolSet,
                                              kCmdBoolMask);
        else
            fError = kCmdErrorInvalidValue;
        break;

    case kCmdTypeFloat:
        arg.val.floatVal = str.to_float();
        break;

    case kCmdTypeInt:
        arg.val.intVal = str.to_int();
        break;

    case kCmdTypeString:
        arg.buffer = str;
        arg.val.strVal = arg.buffer.c_str();
        break;

    case kCmdTypeUint:
        arg.val.uintVal = str.to_uint(10);
        break;

    DEFAULT_FATAL(argType);

    }
    return true;
}

bool plCmdParserImpl::TokenizeFlags(plCmdTokenState* state, const ST::string& str)
{
    bool result = true;
    std::vector<ST::string> tokens = str.tokenize(WHITESPACE SEPARATORS);

    for (auto it = tokens.begin(); result && it != tokens.end(); ++it) {
        size_t lastIndex = size_t(-1);
        ST::string buffer = *it;

        if (buffer.empty()) {
            continue;
        }
        
        buffer = buffer.trim_left(FLAGS);

        while (result) {
            // Lookup the argument name
            result = LookupFlagged(buffer, &lastIndex);
            if (!result) {
                fError = kCmdErrorInvalidArg;
                result = false;
            }

            break;
        }

        if (!result) {
            break;
        }

        // Check for an argument value provided using a separator
        static const std::regex re_separators(".+[" SEPARATORS "].+");
        if (std::regex_match(str.c_str(), re_separators) && !(*(++it)).empty()) {
            result = ProcessValue(state, lastIndex, *it);
            break;
        }

        bool isBool = hsCheckFlagBits(fArgArray[lastIndex].def.flags,
                                      kCmdTypeBool,
                                      kCmdTypeMask);

        // Process values for boolean arguments
        if (isBool) {
            result = ProcessValue(state, lastIndex, ST::string());
            continue;
        }

        // Process values for non-boolean arguments
        else {
            // Check for an argument value immediately following the name
            if (!buffer.empty()) {
                result = ProcessValue(state, lastIndex, buffer);
                break;
            }

            // Check for an argument value in the next token
            else {
                state->fPendingIndex = lastIndex;
                break;
            }
        }
    }

    return result;
}

bool plCmdParserImpl::LookupFlagged(ST::string& name, size_t* lastIndex, bool force) const
{
    size_t argCount  = fArgArray.size();
    size_t chars     = name.size();
    size_t bestIndex = size_t(-1);
    size_t bestChars = 0;


    size_t prevChars = 0;
    if (*lastIndex != size_t(-1)) {
        prevChars = fArgArray[*lastIndex].def.name.size();
    }

    for (; prevChars != size_t(-1) && !bestChars; --prevChars) {
        // Find this argument in the list
        for (size_t index = 0; index < argCount; ++index) {
            const plCmdArgData& arg = fArgArray[index];

            // Ignore non-flagged arguments
            bool flagged = hsCheckFlagBits(arg.def.flags,
                                           kCmdArgFlagged,
                                           kCmdArgMask);
            if (!flagged && !force)
                continue;

            // Ignore this arg if it wouldn't beat the previous best match
            if (arg.def.name.size() < bestChars + prevChars)
                continue;

            // Ignore this argument if it doesn't match the prefix
            bool caseSensitive = hsCheckBits(arg.def.flags,
                                             kCmdCaseSensitive);

            if (prevChars) {
                const plCmdArgData& prev = fArgArray[*lastIndex];

                if (prevChars >= arg.def.name.size())
                    continue;

                if (caseSensitive &&
                    arg.def.name.compare_n(prev.def.name, prevChars))
                    continue;

                if (!caseSensitive &&
                    arg.def.name.compare_ni(prev.def.name, prevChars))
                    continue;
            }

            // Ignore this argument if it doesn't match the suffix
            ST::string suffix = arg.def.name.substr(prevChars);
            if (caseSensitive && suffix.compare_n(name, std::min(name.size(), suffix.size())))
                continue;

            if (!caseSensitive && suffix.compare_ni(name, std::min(name.size(), suffix.size())))
                continue;

            // Track the best match
            bestIndex = index;
            bestChars = arg.def.name.size() - prevChars;
            if (bestChars == chars)
                break;
        }
    }

    // Return the result
    name        = name.substr(bestChars);
    *lastIndex  = bestIndex;
    return bestChars != 0;
}

const plCmdArgData* plCmdParserImpl::FindArgByName(const ST::string& name) const
{
    // Search for an argument with this name
    size_t index = size_t(-1);
    ST::string arg = name;
    if (!LookupFlagged(arg, &index, true)) {
        return nullptr;
    }

    // Return the argument data
    return &fArgArray[index];
}

const plCmdArgData* plCmdParserImpl::FindArgById(size_t id) const
{
    // Search for the argument with this id
    size_t index;
    if (id < fLookupArray.size()) {
        index = fLookupArray[id];
    } else {
        for (index = 0; index < fArgArray.size(); ++index) {
            if (fArgArray[index].def.id == id) {
                break;
            }
        }
    }

    // Verify that we found the correct argument
    if ((index >= fArgArray.size()) || (fArgArray[index].def.id != id)) {
        return nullptr;
    }

    // Return the argument data
    return &fArgArray[index];
}

bool plCmdParserImpl::CheckAllRequiredArguments(plCmdTokenState* state)
{
    bool result = (state->fUnflaggedIndex >= fRequiredCount);

    if (!result) {
        fError = kCmdErrorTooFewArgs;
    }

    return result;
}



plCmdParser::plCmdParser(const plCmdArgDef* defs, size_t defCount)
{
    Initialize(defs, defCount);
}

plCmdParser::~plCmdParser()
{
    delete fParser;
}

void plCmdParser::Initialize(const plCmdArgDef* defs, size_t defCount)
{
    fParser = new plCmdParserImpl(defs, defCount);
}

bool plCmdParser::Parse(const ST::string& cmdLine)
{
    // Process the command line
    plCmdTokenState state = {
        size_t(-1), // pending index
        0           // unflagged index
    };

    std::vector<ST::string> tokens = cmdLine.tokenize(WHITESPACE);

    bool result = fParser->Tokenize(&state, tokens);

    if (result) {
        result = fParser->CheckAllRequiredArguments(&state);
    }

    return result;
}

bool plCmdParser::Parse(std::vector<ST::string>& argv)
{
    // Process the command line
    plCmdTokenState state = {
        size_t(-1), // pending index
        0           // unflagged index
    };

    bool result = fParser->Tokenize(&state, argv);

    if (result) {
        result = fParser->CheckAllRequiredArguments(&state);
    }

    return result;
}


ST::string plCmdParser::GetProgramName() const
{
    return fParser->GetProgramName();
}


bool plCmdParser::GetBool(size_t id) const
{
    return fParser->FindArgById(id)->val.boolVal;
}

bool plCmdParser::GetBool(const ST::string& name) const
{
    return fParser->FindArgByName(name)->val.boolVal;
}

float plCmdParser::GetFloat(size_t id) const
{
    return fParser->FindArgById(id)->val.floatVal;
}

float plCmdParser::GetFloat(const ST::string& name) const
{
    return fParser->FindArgByName(name)->val.floatVal;
}

int32_t plCmdParser::GetInt(size_t id) const
{
    return fParser->FindArgById(id)->val.intVal;
}

int32_t plCmdParser::GetInt(const ST::string& name) const
{
    return fParser->FindArgByName(name)->val.intVal;
}

ST::string plCmdParser::GetString(size_t id) const
{
    return fParser->FindArgById(id)->val.strVal;
}

ST::string plCmdParser::GetString(const ST::string& name) const
{
    return fParser->FindArgByName(name)->val.strVal;
}

uint32_t plCmdParser::GetUint(size_t id) const
{
    return fParser->FindArgById(id)->val.uintVal;
}

uint32_t plCmdParser::GetUint(const ST::string& name) const
{
    return fParser->FindArgByName(name)->val.uintVal;
}

bool plCmdParser::IsSpecified(size_t id) const
{
    if (const plCmdArgData* data = fParser->FindArgById(id)) {
        return data->isSpecified;
    }

    return false;
}

bool plCmdParser::IsSpecified(const ST::string& name) const
{
    if (const plCmdArgData* data = fParser->FindArgByName(name)) {
        return data->isSpecified;
    }

    return false;
}

CmdError plCmdParser::GetError() const
{
    return fParser->GetError();
}
