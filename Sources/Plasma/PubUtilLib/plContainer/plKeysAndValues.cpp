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
#include "plKeysAndValues.h"
#include "hsStream.h"
#include <algorithm>


plKeysAndValues::plKeysAndValues()
{
}

plKeysAndValues::plKeysAndValues(const plKeysAndValues & src)
:	fKeys(src.fKeys)
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

void plKeysAndValues::RemoveKey(const std::string & key)
{
	fKeys.erase(key.c_str());
}

bool plKeysAndValues::HasKey(const std::string & key) const
{
	return (fKeys.find(key.c_str()) != fKeys.end());
}

bool plKeysAndValues::KeyHasValue(const std::string & key, const std::string & value)
{
	Keys::const_iterator ki = fKeys.find(key.c_str());
	if (ki==fKeys.end())
		return false;
	return std::find(ki->second.begin(),ki->second.end(), value.c_str()) != ki->second.end();
}

bool plKeysAndValues::KeyHasValue(const std::string & key, int value)
{
	char buf[20];
	sprintf(buf, "%d", value);
	std::string v(buf);
	return KeyHasValue(key, v);    
}

bool plKeysAndValues::KeyHasValue(const std::string & key, double value)
{
	char buf[30];
	sprintf(buf, "%f", value);
	std::string v(buf);
	return KeyHasValue(key, v);    
}

bool plKeysAndValues::AddValue(const std::string & key, const std::string & value, KAddValueMode mode)
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
	}
	fKeys[key.c_str()].push_front(value.c_str());
	return true;
}

bool plKeysAndValues::AddValue(const std::string & key, int value, KAddValueMode mode)
{
	char buf[20];
	sprintf(buf, "%d", value);
	std::string v(buf);
	return AddValue(key,v,mode);
}

bool plKeysAndValues::AddValue(const std::string & key, double value, KAddValueMode mode)
{
	char buf[30];
	sprintf(buf, "%f", value);
	std::string v(buf);
	return AddValue(key,v,mode);    
}

bool plKeysAndValues::AddValues(const std::string & key, const std::vector<std::string> & values, KAddValueMode mode)
{
	for (int i=0; i<values.size(); i++)
		AddValue(key,values[i],mode);
	return true;
}

bool plKeysAndValues::SetValue(const std::string & key, const std::string & value)
{
	fKeys[key.c_str()].clear();
	return AddValue(key,value);
}

bool plKeysAndValues::SetValue(const std::string & key, int value)
{
	char buf[20];
	sprintf(buf, "%d", value);
	std::string v(buf);
	return SetValue(key, v);    
}

bool plKeysAndValues::SetValue(const std::string & key, double value)
{
	char buf[30];
	sprintf(buf, "%f", value);
	std::string v(buf);
	return SetValue(key, v);    
}

std::string plKeysAndValues::GetValue(const std::string & key, const std::string & defval, bool * outFound) const
{
	Keys::const_iterator ki = fKeys.find(key.c_str());
	if (outFound)
		*outFound = (ki!=fKeys.end());
	if(ki != fKeys.end())
		return ki->second.front().c_str();
//	fKeys[key.c_str()].push_front(defval.c_str());
	return defval;
}

UInt32 plKeysAndValues::GetValue(const std::string & key, UInt32 defval, bool * outFound) const
{
	char buf[20];
	sprintf(buf, "%ul", defval);
	std::string v(buf);
	return strtoul(GetValue(key,v,outFound).c_str(), nil, 0);
}

int plKeysAndValues::GetValue(const std::string & key, int defval, bool * outFound) const
{
	char buf[20];
	sprintf(buf, "%d", defval);
	std::string v(buf);
	return atol(GetValue(key,v,outFound).c_str());
}

double plKeysAndValues::GetValue(const std::string & key, double defval, bool * outFound) const
{
	char buf[30];
	sprintf(buf, "%f", defval);
	std::string v(buf);
	return atof(GetValue(key,v,outFound).c_str());
}

std::vector<std::string> plKeysAndValues::GetAllValues(const std::string & key)
{
	std::vector<std::string> result;
	xtl::istring xkey = key.c_str();
	if (HasKey(key))
		for (Values::const_iterator vi=fKeys[xkey].begin(); vi!=fKeys[xkey].end(); ++vi)
			result.push_back(vi->c_str());
	return result;
}

bool plKeysAndValues::GetKeyIterators(Keys::const_iterator & iter, Keys::const_iterator & end) const
{
	iter = fKeys.begin();
	end = fKeys.end();
	return true;
}

bool plKeysAndValues::GetValueIterators(const xtl::istring & key, Values::const_iterator & iter, Values::const_iterator & end) const
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
	UInt16 nkeys;
	s->ReadSwap(&nkeys);
	for (int ki=0; ki<nkeys; ki++)
	{
		UInt16 strlen;
		s->ReadSwap(&strlen);
		std::string key;
		key.assign(strlen+1,'\0');
		s->Read(strlen,(void*)key.data());
		key.resize(strlen);
		UInt16 nvalues;
		s->ReadSwap(&nvalues);
		for (int vi=0; vi<nvalues; vi++)
		{
			s->ReadSwap(&strlen);
			std::string value;
			value.assign(strlen+1,'\0');
			s->Read(strlen,(void*)value.data());
			value.resize(strlen);
			// for now, only single value for key on stream is allowed.
			SetValue(key,value);
		}
	}
}

void plKeysAndValues::Write(hsStream * s)
{
	// write nkeys
	s->WriteSwap((UInt16)fKeys.size());
	// iterate through keys
	Keys::const_iterator ki,ke;
	GetKeyIterators(ki,ke);
	for (ki;ki!=ke;++ki)
	{
		// write key string
		s->WriteSwap((UInt16)ki->first.size());
		s->Write(ki->first.size(),ki->first.c_str());
		// write nvalues for this key
		s->WriteSwap((UInt16)ki->second.size());
		// iterate through values for this key
		Values::const_iterator vi,ve;
		GetValueIterators(ki->first,vi,ve);
		for (vi;vi!=ve;++vi)
		{
			// write value string
			s->WriteSwap((UInt16)vi->size());
			s->Write(vi->size(),vi->c_str());
		}
	}
}
