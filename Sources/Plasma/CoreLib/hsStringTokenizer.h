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
// hsStringTokenizer.h
#ifndef _hsStringTokenizer_Included_
#define _hsStringTokenizer_Included_

#include "hsTypes.h"

class hsStringTokenizer 
{
private:
	char *fSeps;
	char *fTok;
	char *fLastTerminator;
	char fLastRep;

	Int32 fNumSeps;
	hsBool fQAsTok;
	hsBool fInQuote;
	hsBool fCheckAlphaNum;
public:
	hsStringTokenizer(const char *string=nil, const char *seps=nil);
	~hsStringTokenizer();
	char *next();
	hsBool	Next( char *token, UInt32 maxTokLen );
	hsBool HasMoreTokens();
	void Reset(const char *string, const char *seps);
	void ParseQuotes(hsBool qAsTok);

	char	*GetRestOfString( void ) const { return fTok; }

	char *fString;

	void	RestoreLastTerminator( void );

private:
	hsBool IsSep(char c);
};

class hsWStringTokenizer 
{
private:
	wchar *fSeps;
	wchar *fTok;
	wchar *fLastTerminator;
	wchar fLastRep;

	Int32 fNumSeps;
	hsBool fQAsTok;
	hsBool fInQuote;
	hsBool fCheckAlphaNum;
public:
	hsWStringTokenizer(const wchar *string=nil, const wchar *seps=nil);
	~hsWStringTokenizer();
	wchar *next();
	hsBool	Next( wchar *token, UInt32 maxTokLen );
	hsBool HasMoreTokens();
	void Reset(const wchar *string, const wchar *seps);
	void ParseQuotes(hsBool qAsTok);

	wchar	*GetRestOfString( void ) const { return fTok; }

	wchar *fString;

	void	RestoreLastTerminator( void );

private:
	hsBool IsSep(wchar c);
};

#endif // _hsStringTokenizer_Included_