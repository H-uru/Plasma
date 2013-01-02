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
    size_t idx = s.find_last_not_of(charset);
    
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
    size_t idx = s.find_last_not_of(charset);

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

