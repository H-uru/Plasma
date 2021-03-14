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
#include "plKeysAndValues.h"
#include "hsStream.h"
#include <algorithm>


plKeysAndValues::plKeysAndValues()
{
}

plKeysAndValues::plKeysAndValues(const plKeysAndValues & src)
:   fKeys(src.fKeys)
{
}

plKeysAndValues & plKeysAndValues::operator =(const plKeysAndValues & src)
{
    fKeys = src.fKeys;
    return *this;
}


void plKeysAndValues::Clear()
{
    fKeys.clear();
}

void plKeysAndValues::RemoveKey(const ST::string & key)
{
    fKeys.erase(key);
}

bool plKeysAndValues::HasKey(const ST::string & key) const
{
    return (fKeys.find(key) != fKeys.end());
}

bool plKeysAndValues::KeyHasValue(const ST::string & key, const ST::string & value)
{
    Keys::const_iterator ki = fKeys.find(key);
    if (ki==fKeys.end())
        return false;
    return std::find_if(ki->second.begin(), ki->second.end(),
               [&value](const ST::string &v) { return v.compare_i(value) == 0; }
           ) != ki->second.end();
}

bool plKeysAndValues::KeyHasValue(const ST::string & key, int value)
{
    return KeyHasValue(key, ST::string::from_int(value));
}

bool plKeysAndValues::KeyHasValue(const ST::string & key, double value)
{
    return KeyHasValue(key, ST::string::from_double(value, 'f'));
}

bool plKeysAndValues::AddValue(const ST::string & key, const ST::string & value, KAddValueMode mode)
{
    switch (mode)
    {
    case kFailIfExists:
        if (HasKey(key))
            return false;
        break;
    case kReplaceIfExists:
        if (HasKey(key))
            RemoveKey(key);
        break;
    default:
        break;
    }
    fKeys[key].push_front(value);
    return true;
}

bool plKeysAndValues::AddValue(const ST::string & key, int value, KAddValueMode mode)
{
    return AddValue(key, ST::string::from_int(value), mode);
}

bool plKeysAndValues::AddValue(const ST::string & key, double value, KAddValueMode mode)
{
    return AddValue(key, ST::string::from_double(value, 'f'), mode);
}

bool plKeysAndValues::AddValues(const ST::string & key, const std::vector<ST::string> & values, KAddValueMode mode)
{
    for (int i=0; i<values.size(); i++)
        AddValue(key,values[i],mode);
    return true;
}

bool plKeysAndValues::SetValue(const ST::string & key, const ST::string & value)
{
    fKeys[key].clear();
    return AddValue(key,value);
}

bool plKeysAndValues::SetValue(const ST::string & key, int value)
{
    return SetValue(key, ST::string::from_int(value));
}

bool plKeysAndValues::SetValue(const ST::string & key, double value)
{
    return SetValue(key, ST::string::from_double(value, 'f'));
}

ST::string plKeysAndValues::GetValue(const ST::string & key, const ST::string & defval, bool * outFound) const
{
    Keys::const_iterator ki = fKeys.find(key);
    if (outFound)
        *outFound = (ki!=fKeys.end());
    if(ki != fKeys.end())
        return ki->second.front();
//  fKeys[key].push_front(defval);
    return defval;
}

uint32_t plKeysAndValues::GetValue(const ST::string & key, uint32_t defval, bool * outFound) const
{
    return GetValue(key, ST::string::from_uint(defval), outFound).to_uint();
}

int plKeysAndValues::GetValue(const ST::string & key, int defval, bool * outFound) const
{
    return GetValue(key, ST::string::from_int(defval), outFound).to_int();
}

double plKeysAndValues::GetValue(const ST::string & key, double defval, bool * outFound) const
{
    return GetValue(key, ST::string::from_double(defval, 'f'), outFound).to_double();
}

std::vector<ST::string> plKeysAndValues::GetAllValues(const ST::string & key)
{
    std::vector<ST::string> result;
    if (HasKey(key))
        for (Values::const_iterator vi=fKeys[key].begin(); vi!=fKeys[key].end(); ++vi)
            result.push_back(*vi);
    return result;
}

bool plKeysAndValues::GetKeyIterators(Keys::const_iterator & iter, Keys::const_iterator & end) const
{
    iter = fKeys.begin();
    end = fKeys.end();
    return true;
}

bool plKeysAndValues::GetValueIterators(const ST::string & key, Values::const_iterator & iter, Values::const_iterator & end) const
{
    Keys::const_iterator ki = fKeys.find(key);
    if(ki != fKeys.end())
    {
        iter = ki->second.begin();
        end  = ki->second.end();
        return true;
    }
    return false;
}

void plKeysAndValues::Read(hsStream * s)
{
    uint16_t nkeys = s->ReadLE16();
    for (int ki=0; ki<nkeys; ki++)
    {
        uint16_t strlen = s->ReadLE16();
        ST::char_buffer key;
        key.allocate(strlen);
        s->Read(strlen, key.data());
        uint16_t nvalues = s->ReadLE16();
        for (int vi=0; vi<nvalues; vi++)
        {
            strlen = s->ReadLE16();
            ST::char_buffer value;
            value.allocate(strlen);
            s->Read(strlen, value.data());
            // for now, only single value for key on stream is allowed.
            SetValue(key,value);
        }
    }
}

void plKeysAndValues::Write(hsStream * s)
{
    // write nkeys
    s->WriteLE16((uint16_t)fKeys.size());
    // iterate through keys
    Keys::const_iterator ki,ke;
    GetKeyIterators(ki,ke);
    for (;ki!=ke;++ki)
    {
        // write key string
        s->WriteLE16((uint16_t)ki->first.size());
        s->Write(ki->first.size(),ki->first.c_str());
        // write nvalues for this key
        s->WriteLE16((uint16_t)ki->second.size());
        // iterate through values for this key
        Values::const_iterator vi,ve;
        GetValueIterators(ki->first,vi,ve);
        for (;vi!=ve;++vi)
        {
            // write value string
            s->WriteLE16((uint16_t)vi->size());
            s->Write(vi->size(),vi->c_str());
        }
    }
}
