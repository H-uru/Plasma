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
#ifndef hsStlUtils_h_inc
#define hsStlUtils_h_inc


#include "HeadSpin.h"
#include <functional>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>


/*****************************************************************************
*
*   stl extensions
*
***/
namespace xtl
{

// Why oh why doesn't stl have copy_if?
// See Effective STL [Meyers 2001] Item 36.
template< typename InIt, typename OutIt, typename Pred >
OutIt copy_if( InIt srcBegin, InIt srcEnd, OutIt dstBegin, Pred pred )
{
    while ( srcBegin!=srcEnd )
    {
        if ( pred( *srcBegin ) )
            *dstBegin++ = *srcBegin;
        ++srcBegin;
    }
    return dstBegin;
}



// useful when clearing a vector/list/set of pointers that need to be deleted.
// use like:
//  std::vector<foo*>   vec;
//  std::for_each(vec.begin(),vec.end(),xtl::delete_ptr());
//  vec.clear();

struct delete_ptr
{
    template< class T > void operator()( T * p ) const { delete p;}
};

// useful when clearing a map of pointers that need to be deleted.
// use like:
//  typedef std::map<int,foo*> foomap;
//  foomap  m;
//  std::for_each(m.begin(),m.end(),xtl::delete_map_ptr_T<foomap>());
//  m.clear();

template< class A >
struct delete_map_ptr_T
{
    void operator()( typename A::value_type & pair ) const { delete pair.second;}
};

// case insensitive string comparer
//  useful in maps that use strings
struct stricmp_less : public std::binary_function<std::string, std::string, bool>
{
    bool operator()(const std::string & _X, const std::string & _Y) const
    {return ( stricmp(_X.c_str(),_Y.c_str()) < 0); }
};
struct wstricmp_less : public std::binary_function<std::wstring, std::wstring, bool>
{
    bool operator()(const std::wstring & _X, const std::wstring & _Y) const
    {return ( wcsicmp(_X.c_str(),_Y.c_str()) < 0); }
};

// struct stricmp_char_traits
// case insensitive char_traits. used in creating istring class below
#ifdef __SGI_STL_PORT
struct stricmp_char_traits : public __std_alias::char_traits< char >
#else
struct stricmp_char_traits : public std::char_traits< char >
#endif
{
    static int compare(const char * A, const char * B, size_t N)
    {
        for (size_t I=0; I<N; ++I, ++A,++B)
            if (tolower(*A)!=tolower(*B))
                return (lt(tolower(*A),tolower(*B))?-1:+1);
        return (0);
    }
    
    static const char * find(const char * S, size_t N, const char & C)
    {
        char c = tolower(C);
        for (; 0<N; --N, ++S)
            if (c==tolower(*S))
                return S;
        return NULL;
    }
};
#ifdef __SGI_STL_PORT
struct wstricmp_char_traits : public __std_alias::char_traits< wchar_t >
#else
struct wstricmp_char_traits : public std::char_traits< wchar_t >
#endif
{
    static int compare(const wchar_t * A, const wchar_t * B, size_t N)
    {
        for (size_t I=0; I<N; ++I, ++A,++B)
            if (tolower(*A)!=tolower(*B))
                return (lt(tolower(*A),tolower(*B))?-1:+1);
        return (0);
    }

    static const wchar_t * find(const wchar_t * S, size_t N, const wchar_t & C)
    {
        wchar_t c = tolower(C);
        for (; 0<N; --N, ++S)
            if (c==tolower(*S))
                return S;
        return NULL;
    }
};

// class istring
//  A string with case insensitive char_traits.
//  Calls to its find* methods are case insensitive.
typedef std::basic_string<char, stricmp_char_traits> istring;
typedef std::basic_string<wchar_t, wstricmp_char_traits> iwstring;

// std::string trim
std::string & trimleft(std::string & s, const char * charset=" \t\n\r");
std::wstring & trimleft(std::wstring & s, const wchar_t * charset=L" \t\n\r");
std::string & trimright(std::string & s, const char * charset=" \t\n\r");
std::wstring & trimright(std::wstring & s, const wchar_t * charset=L" \t\n\r");
std::string & trim(std::string & s, const char * charset=" \t\n\r");
std::wstring & trim(std::wstring & s, const wchar_t * charset=L" \t\n\r");
// xtl::istring trim
xtl::istring & trimleft(xtl::istring & s, const char * charset=" \t\n\r");
xtl::iwstring & trimleft(xtl::iwstring & s, const wchar_t * charset=L" \t\n\r");
xtl::istring & trimright(xtl::istring & s, const char * charset=" \t\n\r");
xtl::iwstring & trimright(xtl::iwstring & s, const wchar_t * charset=L" \t\n\r");
xtl::istring & trim(xtl::istring & s, const char * charset=" \t\n\r");
xtl::iwstring & trim(xtl::iwstring & s, const wchar_t * charset=L" \t\n\r");
// c-string trim
std::string trim(const char * s, const char * charset=" \t\n\r");
std::wstring trim(const wchar_t * s, const wchar_t * charset=L" \t\n\r");
// format
std::string format(const char * fmt, ...);
std::wstring format(const wchar_t * fmt, ...);
std::string formatv(const char * fmt, va_list args);
std::wstring formatv(const wchar_t * fmt, va_list args);
bool format(std::string & out, const char * fmt, ...);
bool format(std::wstring & out, const wchar_t * fmt, ...);
bool formatv(std::string & out, const char * fmt, va_list args);
bool formatv(std::wstring & out, const wchar_t * fmt, va_list args);


template <typename T> bool GetStringGroup(const std::string& s, T& group, char sep = ',');
template <typename T> bool GetStringGroup(const std::wstring& s, T& group, wchar_t sep = L',');
template <typename T> bool GetStringGroupAsString(const T& group, std::string& s, char sep = ',');
template <typename T> bool GetStringGroupAsString(const T& group, std::wstring& s, wchar_t sep = L',');


} // namespace xtd



#endif // hsStlUtils_h_inc
