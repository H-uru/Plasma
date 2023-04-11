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
// hsStringTokenizer.cpp

#include "hsStringTokenizer.h"

#include <wchar.h>
#include <wctype.h>

// String Tokenizer routines
hsStringTokenizer::hsStringTokenizer(const char *string, const char *seps) :
    fSeps(), fLastTerminator(), fQAsTok(true), fInQuote(), fString()
{
    Reset(string,seps);
}

hsStringTokenizer::~hsStringTokenizer() 
{
    delete[] fString;
    delete[] fSeps;
}

bool hsStringTokenizer::HasMoreTokens() 
{
    return (*fTok != 0);
}

inline bool hsStringTokenizer::IsSep(char c) 
{
    if (!fQAsTok || !fInQuote) 
    {
        if (fCheckAlphaNum || !isalnum(static_cast<unsigned char>(c)))
        {
            for (int32_t i=0; i<fNumSeps; i++) 
            {
                if (fSeps[i] == c) 
                    return true;
            }
        }
    }
    if (fQAsTok && c=='\"') 
    {
        fInQuote = !fInQuote;
        return true;
    }
    return false;
}

char *hsStringTokenizer::next() 
{
    if (*fTok == 0) 
        return nullptr;

    char *cur = fTok;
    while (*fTok != 0 && !IsSep(*fTok)) 
        fTok++;

    if (*fTok != 0) 
    {
        fLastRep = *fTok;
        fLastTerminator = fTok;

        *fTok = 0;
        fTok++;
    }
    while (*fTok != 0 && IsSep(*fTok)) 
        fTok++;

    return cur;
}

// Slightly more loop-friendly version of next
bool  hsStringTokenizer::Next( char *token, uint32_t maxTokLen )
{
    char *t = next();
    if (t == nullptr)
        return false;

    hsStrncpy( token, t, maxTokLen );
    return true;
}

// Restores the last character replaced to generate a terminator
void    hsStringTokenizer::RestoreLastTerminator()
{
    if (fLastTerminator != nullptr)
    {
        *fLastTerminator = fLastRep;
        fLastTerminator = nullptr;
    }
}

void hsStringTokenizer::Reset(const char *string, const char *seps) 
{
    if (fString)
        delete[] fString;
    fString = string ? hsStrcpy(string) : nullptr;

    if (fSeps)
        delete[] fSeps;
    fSeps = seps ? hsStrcpy(seps) : nullptr;
    fNumSeps = fSeps ? strlen(fSeps) : 0;
    fCheckAlphaNum = false;
    for (int32_t i=0; i<fNumSeps; i++)
    {
        if (isalnum(static_cast<unsigned char>(fSeps[i])))
        {
            fCheckAlphaNum=true;
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

void hsStringTokenizer::ParseQuotes(bool qAsTok)
{
    fQAsTok = qAsTok;
}

///////////////////////////////////////////////////////////////////////////////

// String Tokenizer routines
hsWStringTokenizer::hsWStringTokenizer(const wchar_t *string, const wchar_t *seps) :
    fSeps(), fLastTerminator(), fQAsTok(true), fInQuote(), fString()
{
    Reset(string,seps);
}

hsWStringTokenizer::~hsWStringTokenizer() 
{
    delete[] fString;
    delete[] fSeps;
}

bool hsWStringTokenizer::HasMoreTokens() 
{
    return (*fTok != L'\0');
}

inline bool hsWStringTokenizer::IsSep(wchar_t c) 
{
    if (!fQAsTok || !fInQuote) 
    {
        if ( fCheckAlphaNum || !iswalnum(c) )
        {
            for (int32_t i=0; i<fNumSeps; i++) 
            {
                if (fSeps[i] == c) 
                    return true;
            }
        }
    }
    if (fQAsTok && c==L'\"') 
    {
        fInQuote = !fInQuote;
        return true;
    }
    return false;
}

wchar_t *hsWStringTokenizer::next() 
{
    if (*fTok == L'\0') 
        return nullptr;

    wchar_t *cur = fTok;
    while (*fTok != L'\0' && !IsSep(*fTok)) 
        fTok++;

    if (*fTok != L'\0') 
    {
        fLastRep = *fTok;
        fLastTerminator = fTok;

        *fTok = L'\0';
        fTok++;
    }
    while (*fTok != L'\0' && IsSep(*fTok)) 
        fTok++;

    return cur;
}

// Slightly more loop-friendly version of next
bool  hsWStringTokenizer::Next( wchar_t *token, uint32_t maxTokLen )
{
    wchar_t *t = next();
    if (t == nullptr)
        return false;

    wcsncpy( token, t, maxTokLen - 1 );
    token[maxTokLen - 1] = L'\0';
    return true;
}

// Restores the last character replaced to generate a terminator
void    hsWStringTokenizer::RestoreLastTerminator()
{
    if (fLastTerminator != nullptr)
    {
        *fLastTerminator = fLastRep;
        fLastTerminator = nullptr;
    }
}

void hsWStringTokenizer::Reset(const wchar_t *string, const wchar_t *seps) 
{
    if (fString)
        delete[] fString;
    if (string)
    {
        int count = wcslen(string);
        fString = new wchar_t[count + 1];
        wcscpy(fString, string);
        fString[count] = L'\0';
    }
    else
        fString = nullptr;

    if (fSeps)
        delete[] fSeps;
    if (seps)
    {
        int count = wcslen(seps);
        fSeps = new wchar_t[count + 1];
        wcscpy(fSeps, seps);
        fSeps[count] = L'\0';
    }
    else
        fSeps = nullptr;

    fNumSeps = fSeps ? wcslen(fSeps) : 0;
    fCheckAlphaNum = false;
    for (int32_t i=0; i<fNumSeps; i++)
    {
        if (iswalnum(fSeps[i]))
        {
            fCheckAlphaNum=true;
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

void hsWStringTokenizer::ParseQuotes(bool qAsTok)
{
    fQAsTok = qAsTok;
}
