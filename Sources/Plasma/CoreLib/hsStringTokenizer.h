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
// hsStringTokenizer.h
#ifndef _hsStringTokenizer_Included_
#define _hsStringTokenizer_Included_

#include "HeadSpin.h"

template<typename CharT>
class hsBasicStringTokenizer
{
private:
    CharT* fSeps;
    CharT* fTok;
    CharT* fLastTerminator;
    CharT fLastRep;

    size_t fNumSeps;
    bool fQAsTok;
    bool fInQuote;
    bool fCheckAlphaNum;

public:
    hsBasicStringTokenizer(const CharT* string = nullptr, const CharT* seps = nullptr) :
        fSeps(), fLastTerminator(), fQAsTok(true), fInQuote(), fString()
    {
        Reset(string, seps);
    }

    ~hsBasicStringTokenizer()
    {
        delete[] fString;
        delete[] fSeps;
    }

    bool HasMoreTokens()
    {
        return *fTok != 0;
    }

    CharT* next()
    {
        if (*fTok == 0) {
            return nullptr;
        }

        CharT* cur = fTok;
        while (*fTok != 0 && !IsSep(*fTok)) {
            fTok++;
        }

        if (*fTok != 0) {
            fLastRep = *fTok;
            fLastTerminator = fTok;

            *fTok = 0;
            fTok++;
        }
        while (*fTok != 0 && IsSep(*fTok)) {
            fTok++;
        }

        return cur;
    }

    // Slightly more loop-friendly version of next
    bool Next(CharT* token, size_t maxTokLen)
    {
        CharT* t = next();
        if (t == nullptr) {
            return false;
        }

        for (size_t i = 0; i < maxTokLen; i++) {
            token[i] = t[i];
            if (t[i] == 0) {
                break;
            }
        }
        return true;
    }

    // Restores the last character replaced to generate a terminator
    void RestoreLastTerminator()
    {
        if (fLastTerminator != nullptr) {
            *fLastTerminator = fLastRep;
            fLastTerminator = nullptr;
        }
    }

    void Reset(const CharT* string, const CharT* seps)
    {
        delete[] fString;
        if (string) {
            size_t count = StrLen(string);
            fString = new CharT[count + 1];
            memcpy(fString, string, sizeof(CharT) * (count + 1));
        } else {
            fString = nullptr;
        }

        delete[] fSeps;
        if (seps) {
            fNumSeps = StrLen(seps);
            fSeps = new CharT[fNumSeps + 1];
            memcpy(fSeps, seps, sizeof(CharT) * (fNumSeps + 1));
        } else {
            fNumSeps = 0;
            fSeps = nullptr;
        }

        fCheckAlphaNum = false;
        for (size_t i = 0; i < fNumSeps; i++) {
            if (fSeps[i] >= 0 && fSeps[i] < 128 && isalnum(fSeps[i])) {
                fCheckAlphaNum = true;
                break;
            }
        }

        fTok = fString;

        fLastTerminator = nullptr;
        fLastRep = 0;

        // don't skip empty fields.
        //  if (fTok && IsSep(*fTok))
        //      next();
    }

    void ParseQuotes(bool qAsTok) { fQAsTok = qAsTok; }

    CharT* GetRestOfString() const { return fTok; }

    CharT* fString;

private:
    static size_t StrLen(const CharT* str)
    {
        size_t i = 0;
        while (str[i] != 0) {
            i++;
        }
        return i;
    }

    bool IsSep(CharT c)
    {
        if (!fQAsTok || !fInQuote) {
            if (fCheckAlphaNum || !(c >= 0 && c <= 127 && isalnum(c))) {
                for (size_t i = 0; i < fNumSeps; i++) {
                    if (fSeps[i] == c) {
                        return true;
                    }
                }
            }
        }
        if (fQAsTok && c == '\"') {
            fInQuote = !fInQuote;
            return true;
        }
        return false;
    }
};

using hsStringTokenizer = hsBasicStringTokenizer<char>;
using hsWStringTokenizer = hsBasicStringTokenizer<wchar_t>;

#endif // _hsStringTokenizer_Included_
