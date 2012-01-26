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

#ifndef HS_BUILD_FOR_WIN32
#include <wchar.h>
#include <wctype.h>
#endif

class hsStringTokenizer 
{
private:
    char *fSeps;
    char *fTok;
    char *fLastTerminator;
    char fLastRep;

    int32_t fNumSeps;
    hsBool fQAsTok;
    hsBool fInQuote;
    hsBool fCheckAlphaNum;
public:
    hsStringTokenizer(const char *string=nil, const char *seps=nil);
    ~hsStringTokenizer();
    char *next();
    hsBool  Next( char *token, uint32_t maxTokLen );
    hsBool HasMoreTokens();
    void Reset(const char *string, const char *seps);
    void ParseQuotes(hsBool qAsTok);

    char    *GetRestOfString( void ) const { return fTok; }

    char *fString;

    void    RestoreLastTerminator( void );

private:
    hsBool IsSep(char c);
};

class hsWStringTokenizer 
{
private:
    wchar_t *fSeps;
    wchar_t *fTok;
    wchar_t *fLastTerminator;
    wchar_t fLastRep;

    int32_t fNumSeps;
    hsBool fQAsTok;
    hsBool fInQuote;
    hsBool fCheckAlphaNum;
public:
    hsWStringTokenizer(const wchar_t *string=nil, const wchar_t *seps=nil);
    ~hsWStringTokenizer();
    wchar_t *next();
    hsBool  Next( wchar_t *token, uint32_t maxTokLen );
    hsBool HasMoreTokens();
    void Reset(const wchar_t *string, const wchar_t *seps);
    void ParseQuotes(hsBool qAsTok);

    wchar_t   *GetRestOfString( void ) const { return fTok; }

    wchar_t *fString;

    void    RestoreLastTerminator( void );

private:
    hsBool IsSep(wchar_t c);
};

#endif // _hsStringTokenizer_Included_
