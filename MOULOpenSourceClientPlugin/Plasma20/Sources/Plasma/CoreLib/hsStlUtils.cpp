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
#include "hsStlUtils.h"

// stl extensions
namespace xtl {

//std::string
std::string & trimleft(std::string & s, const char * charset)
{
	s.erase(0, s.find_first_not_of(charset));
	return s;
}

std::wstring & trimleft(std::wstring & s, const wchar_t * charset)
{
	s.erase(0, s.find_first_not_of(charset));
	return s;
}

std::string & trimright(std::string & s, const char * charset)
{
	int idx = s.find_last_not_of(charset);
	
	if (std::string::npos == idx)
	{
		s.erase();
	}
	else
	{
		char c    = s.at(idx);
		s.erase(idx, std::string::npos);    
		s.append(1, c);
	}
	
	return s;
}

std::wstring & trimright(std::wstring & s, const wchar_t * charset)
{
	int idx = s.find_last_not_of(charset);

	if (std::wstring::npos == idx)
	{
		s.erase();
	}
	else
	{
		wchar_t c = s.at(idx);
		s.erase(idx, std::string::npos);
		s.append(1, c);
	}

	return s;
}

std::string & trim(std::string & s, const char * charset)
{
	trimleft(s,charset);
	trimright(s,charset);
	return s;
}

std::wstring & trim(std::wstring & s, const wchar_t * charset)
{
	trimleft(s,charset);
	trimright(s,charset);
	return s;
}

//xtl::istring
xtl::istring & trimleft(xtl::istring & s, const char * charset)
{
	s.erase(0, s.find_first_not_of(charset));
	return s;
}

xtl::iwstring & trimleft(xtl::iwstring & s, const wchar_t * charset)
{
	s.erase(0, s.find_first_not_of(charset));
	return s;
}

xtl::istring & trimright(xtl::istring & s, const char * charset)
{
	int idx = s.find_last_not_of(charset);
	
	if (xtl::istring::npos == idx)
	{
		s.erase();
	}
	else
	{
		char c    = s.at(idx);
		s.erase(idx, xtl::istring::npos);    
		s.append(1, c);
	}
	
	return s;
}

xtl::iwstring & trimright(xtl::iwstring & s, const wchar_t * charset)
{
	int idx = s.find_last_not_of(charset);

	if (xtl::iwstring::npos == idx)
	{
		s.erase();
	}
	else
	{
		wchar_t c = s.at(idx);
		s.erase(idx, xtl::iwstring::npos);
		s.append(1, c);
	}
	
	return s;
}

xtl::istring & trim(xtl::istring & s, const char * charset)
{
	trimleft(s,charset);
	trimright(s,charset);
	return s;
}

xtl::iwstring & trim(xtl::iwstring & s, const wchar_t * charset)
{
	trimleft(s,charset);
	trimright(s,charset);
	return s;
}

// c-string
std::string trim(const char * s, const char * charset)
{
	std::string result  = s;
	trimleft(result,charset);
	trimright(result,charset);
	return result;
}

std::wstring trim(const wchar_t * s, const wchar_t * charset)
{
	std::wstring result = s;
	trimleft(result,charset);
	trimright(result,charset);
	return result;
}


// format
std::string format(const char * fmt, ...)
{
	std::string result;
	va_list args;
	va_start(args,fmt);
	formatv(result,fmt,args);
	va_end(args);
	return result;
}

std::wstring format(const wchar_t * fmt, ...)
{
	std::wstring result;
	va_list args;
	va_start(args,fmt);
	formatv(result,fmt,args);
	va_end(args);
	return result;
}

std::string formatv(const char * fmt, va_list args)
{
	std::string result;
	formatv( result, fmt, args );
	return result;
}

std::wstring formatv(const wchar_t * fmt, va_list args)
{
	std::wstring result;
	formatv( result, fmt, args );
	return result;
}

bool format(std::string & out, const char * fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	bool r = formatv(out,fmt,args);
	va_end(args);
	return r;
}

bool format(std::wstring & out, const wchar_t * fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	bool r = formatv(out,fmt,args);
	va_end(args);
	return r;
}

bool formatv(std::string & out, const char * fmt, va_list args)
{
#define kBufSz 2048

	char buf[kBufSz];
	char * pbuf = buf;
	int len = 0;
	int attempts = 0;
	bool success = false;
	const int kMaxAttempts = 40;

	do
	{
		int maxlen = kBufSz*attempts+kBufSz-1;
		len = hsVsnprintf(pbuf,maxlen,fmt,args);
		attempts++;
		success = (len>=0 && len<maxlen);
		if (!success)
		{
			if (pbuf!=buf)
				delete [] pbuf;
			pbuf = TRACKED_NEW char[kBufSz+kBufSz*attempts];
		}
	}
	while (!success && attempts<kMaxAttempts);

	if (success)
	{
		pbuf[len] = '\0';
		out = pbuf;
	}

	if (success)
	{
		pbuf[len] = '\0';
		out = pbuf;
	}
	else
	{
		out = "";
		if ( attempts==kMaxAttempts )
		{
			hsDebugMessage( "xtl::formatv - Max reallocs occurred while formatting string. Result is likely truncated!", 0 );
		}
	}

	if (pbuf!=buf)
		delete [] pbuf;

	return success;
}

bool formatv(std::wstring & out, const wchar_t * fmt, va_list args)
{
#define kBufSz 2048
	
	wchar_t buf[kBufSz];
	wchar_t * pbuf = buf;
	int len = 0;
	int attempts = 0;
	bool success = false;
	const int kMaxAttempts = 40;
	
	do
	{
		int maxlen = kBufSz*attempts+kBufSz-1;
		len = hsVsnwprintf(pbuf,maxlen,fmt,args);
		attempts++;
		success = (len>=0 && len<maxlen);
		if (!success)
		{
			if (pbuf!=buf)
				delete [] pbuf;
			pbuf = TRACKED_NEW wchar_t[kBufSz+kBufSz*attempts];
		}
	}
	while (!success && attempts<kMaxAttempts);
	
	if (success)
	{
		pbuf[len] = L'\0';
		out = pbuf;
	}
	
	if (success)
	{
		pbuf[len] = L'\0';
		out = pbuf;
	}
	else
	{
		out = L"";
		if ( attempts==kMaxAttempts )
		{
			hsDebugMessage( "xtl::formatv - Max reallocs occurred while formatting wstring. Result is likely truncated!", 0 );
		}
	}
	
	if (pbuf!=buf)
		delete [] pbuf;
	
	return success;
}

typedef std::vector<std::string> StringVector;
typedef std::vector<std::wstring> WStringVector;
typedef std::list<std::string> StringList;
typedef std::list<std::wstring> WStringList;
typedef std::set<std::string> StringSet;
typedef std::set<std::wstring> WStringSet;

template bool GetStringGroup<StringList>(const std::string& s, StringList& group, char sep);
template bool GetStringGroup<WStringList>(const std::wstring& s, WStringList& group, wchar_t sep);
template bool GetStringGroup<StringVector>(const std::string& s, StringVector& group, char sep);
template bool GetStringGroup<WStringVector>(const std::wstring& s, WStringVector& group, wchar_t sep);
template bool GetStringGroup<StringSet>(const std::string& s, StringSet& group, char sep);
template bool GetStringGroup<WStringSet>(const std::wstring& s, WStringSet& group, wchar_t sep);

template <typename T> bool GetStringGroup(const std::string& s, T& group, char sep)
{
	bool ret = false;
	std::string::size_type oldpos = 0, newpos = 0;
	
	if (!(s.empty()))
	{
		do 
		{
			newpos = s.find(',',oldpos);
			group.insert(group.end(),s.substr(oldpos,newpos));
			if (newpos != s.npos)
				oldpos = newpos+1;
		} 
		while(newpos != s.npos);
		ret = true;
	}

	return ret;
}

template <typename T> bool GetStringGroup(const std::wstring& s, T& group, wchar_t sep)
{
	bool ret = false;
	std::wstring::size_type oldpos = 0, newpos = 0;

	if (!(s.empty()))
	{
		do
		{
			newpos = s.find(L',',oldpos);
			group.insert(group.end(),s.substr(oldpos,newpos));
			if (newpos != s.npos)
				oldpos = newpos+1;
		} while(newpos != s.npos);
		ret = true;
	}

	return ret;
}


template bool GetStringGroupAsString<StringList>(const StringList& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringList>(const WStringList& group, std::wstring& s, wchar_t sep);
template bool GetStringGroupAsString<StringVector>(const StringVector& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringVector>(const WStringVector& group, std::wstring& s, wchar_t sep);
template bool GetStringGroupAsString<StringSet>(const StringSet& group, std::string& s, char sep);
template bool GetStringGroupAsString<WStringSet>(const WStringSet& group, std::wstring& s, wchar_t sep);

template <typename T> bool GetStringGroupAsString(const T& group, std::string& s, char sep)
{
	typename T::const_iterator it = group.begin();
	bool fst = true;
	while (it != group.end())
	{
		if (!fst)
			s += ",";
		else
			fst = false;
		s+= (*it).c_str();
		it++;
	}

	return true;
}

template <typename T> bool GetStringGroupAsString(const T& group, std::wstring& s, wchar_t sep)
{
	typename T::const_iterator it = group.begin();
	bool fst = true;
	while (it != group.end())
	{
		if (!fst)
			s += L",";
		else
			fst = false;
		s+= (*it).c_str();
		it++;
	}
	
	return true;
}


} // namespace std

