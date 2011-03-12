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
// hsStringTokenizer.cpp

#include "hsStringTokenizer.h"
#include "hsUtils.h"

// String Tokenizer routines
hsStringTokenizer::hsStringTokenizer(const char *string, const char *seps) :
fQAsTok(true),
fInQuote(false),
fString(nil),
fSeps(nil),
fLastTerminator(nil)
{
	Reset(string,seps);
}

hsStringTokenizer::~hsStringTokenizer() 
{
	delete [] fString;
	delete [] fSeps;
}

hsBool hsStringTokenizer::HasMoreTokens() 
{
	return (*fTok != 0);
}

inline hsBool hsStringTokenizer::IsSep(char c) 
{
	if (!fQAsTok || !fInQuote) 
	{
		if ( fCheckAlphaNum || !isalnum(c) )
		{
			for (Int32 i=0; i<fNumSeps; i++) 
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
		return nil;

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
hsBool	hsStringTokenizer::Next( char *token, UInt32 maxTokLen )
{
	char *t = next();
	if( t == nil )
		return false;

	hsStrncpy( token, t, maxTokLen );
	return true;
}

// Restores the last character replaced to generate a terminator
void	hsStringTokenizer::RestoreLastTerminator( void )
{
	if( fLastTerminator != nil )
	{
		*fLastTerminator = fLastRep;
		fLastTerminator = nil;
	}
}

void hsStringTokenizer::Reset(const char *string, const char *seps) 
{
	if (fString)
		delete [] fString;
	fString = string ? hsStrcpy(string) : nil;

	if (fSeps)
		delete [] fSeps;
	fSeps = seps ? hsStrcpy(seps) : nil;
	fNumSeps = fSeps ? strlen(fSeps) : 0;
	fCheckAlphaNum = false;
	for (Int32 i=0; i<fNumSeps; i++)
	{
		if (isalnum(fSeps[i]))
		{
			fCheckAlphaNum=true;
			break;
		}
	}

	fTok = fString;

	fLastTerminator = nil;
	fLastRep = 0;

	// don't skip empty fields.
//	if (fTok && IsSep(*fTok))
//		next();
}

void hsStringTokenizer::ParseQuotes(hsBool qAsTok)
{
	fQAsTok = qAsTok;
}

///////////////////////////////////////////////////////////////////////////////

// String Tokenizer routines
hsWStringTokenizer::hsWStringTokenizer(const wchar *string, const wchar *seps) :
	fQAsTok(true),
	fInQuote(false),
	fString(nil),
	fSeps(nil),
	fLastTerminator(nil)
{
	Reset(string,seps);
}

hsWStringTokenizer::~hsWStringTokenizer() 
{
	delete [] fString;
	delete [] fSeps;
}

hsBool hsWStringTokenizer::HasMoreTokens() 
{
	return (*fTok != L'\0');
}

inline hsBool hsWStringTokenizer::IsSep(wchar c) 
{
	if (!fQAsTok || !fInQuote) 
	{
		if ( fCheckAlphaNum || !iswalnum(c) )
		{
			for (Int32 i=0; i<fNumSeps; i++) 
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

wchar *hsWStringTokenizer::next() 
{
	if (*fTok == L'\0') 
		return nil;

	wchar *cur = fTok;
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
hsBool	hsWStringTokenizer::Next( wchar *token, UInt32 maxTokLen )
{
	wchar *t = next();
	if( t == nil )
		return false;

	wcsncpy( token, t, maxTokLen - 1 );
	token[maxTokLen - 1] = L'\0';
	return true;
}

// Restores the last character replaced to generate a terminator
void	hsWStringTokenizer::RestoreLastTerminator( void )
{
	if( fLastTerminator != nil )
	{
		*fLastTerminator = fLastRep;
		fLastTerminator = nil;
	}
}

void hsWStringTokenizer::Reset(const wchar *string, const wchar *seps) 
{
	if (fString)
		delete [] fString;
	if (string)
	{
		int	count = wcslen(string);
		fString = TRACKED_NEW(wchar[count + 1]);
		wcscpy(fString, string);
		fString[count] = L'\0';
	}
	else
		fString = nil;

	if (fSeps)
		delete [] fSeps;
	if (seps)
	{
		int	count = wcslen(seps);
		fSeps = TRACKED_NEW(wchar[count + 1]);
		wcscpy(fSeps, seps);
		fSeps[count] = L'\0';
	}
	else
		fSeps = nil;

	fNumSeps = fSeps ? wcslen(fSeps) : 0;
	fCheckAlphaNum = false;
	for (Int32 i=0; i<fNumSeps; i++)
	{
		if (iswalnum(fSeps[i]))
		{
			fCheckAlphaNum=true;
			break;
		}
	}

	fTok = fString;

	fLastTerminator = nil;
	fLastRep = 0;

	// don't skip empty fields.
	//	if (fTok && IsSep(*fTok))
	//		next();
}

void hsWStringTokenizer::ParseQuotes(hsBool qAsTok)
{
	fQAsTok = qAsTok;
}
