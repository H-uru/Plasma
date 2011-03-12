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
#ifndef plKeysAndValues_h_inc
#define plKeysAndValues_h_inc

#include "hsConfig.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "hsStream.h"

#pragma warning(disable:4284)

enum KAddValueMode
{
	kAlwaysAdd,			// Add another value if key already exists
	kReplaceIfExists,	// Replace any existing key with new value.
	kFailIfExists,		// Do not add if key already exists.
};


// class plKeysAndValues
//	A multimap class. Stores multiple values per key.
class plKeysAndValues : public hsStreamable
{
public:    
	typedef std::list<xtl::istring>
		Values;
	typedef std::map<xtl::istring, Values>
		Keys;

private:
	mutable Keys fKeys;

public:
	// ctor
	plKeysAndValues();
	plKeysAndValues(const plKeysAndValues & src);
	virtual ~plKeysAndValues(){}
	// assign
	plKeysAndValues & operator =(const plKeysAndValues & src);
	// clear
	void Clear();
	void RemoveKey(const std::string & key);
	// query
	bool HasKey(const std::string & key) const;
	bool KeyHasValue(const std::string & key, const std::string & value);
	bool KeyHasValue(const std::string & key, int value);
	bool KeyHasValue(const std::string & key, double value);
	// add
	bool AddValue(const std::string & key, const std::string & value, KAddValueMode mode=kAlwaysAdd);
	bool AddValue(const std::string & key, int value, KAddValueMode mode=kAlwaysAdd);
	bool AddValue(const std::string & key, double value, KAddValueMode mode=kAlwaysAdd);
	bool AddValues(const std::string & key, const std::vector<std::string> & values, KAddValueMode mode=kAlwaysAdd);
	// set (clear and add)
	bool SetValue(const std::string & key, const std::string & value);
	bool SetValue(const std::string & key, int value);
	bool SetValue(const std::string & key, double value);
	// get single value
	std::string GetValue(const std::string & key, const std::string & defval="", bool * outFound=nil) const;
	UInt32 GetValue(const std::string & key, UInt32 defval, bool * outFound=nil) const;
	int GetValue(const std::string & key, int defval, bool * outFound=nil) const;
	double GetValue(const std::string & key, double defval, bool * outFound=nil) const;
	std::vector<std::string> GetAllValues(const std::string & key);
	// key iterator
	bool GetKeyIterators(Keys::const_iterator & iter, Keys::const_iterator & end) const;
	// value iterator (use for getting all values for key)
	bool GetValueIterators(const xtl::istring & key, Values::const_iterator & iter, Values::const_iterator & end) const;
	// streamable
	void Read(hsStream * s);
	void Write(hsStream * s);
	// TODO:
	UInt32 GetStreamSize() { return 0;}
};


/////////////////////////////////////////////////////////////



#endif // plKeysAndValues_h_inc
