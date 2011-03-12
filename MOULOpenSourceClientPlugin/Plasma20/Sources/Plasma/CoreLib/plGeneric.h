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
#ifndef __PLGENERIC_H__
#define __PLGENERIC_H__

#include "hsStream.h"
#include "hsStlUtils.h"

class plGeneric
{
public:
	enum GenericType
	{
		kNull,
		kBool,
		kInt,
		kFloat,
		kString
	};

private:
	GenericType		fType;
	bool			fBoolVal;
	int				fIntVal;
	double			fFloatVal;
	std::wstring	fStringVal;

	void IReset();

public:
	plGeneric();
	plGeneric(const bool& val);
	plGeneric(const int& val);
	plGeneric(const double& val);
	plGeneric(const char* val);
	plGeneric(const std::string& val);
	plGeneric(const wchar_t* val);
	plGeneric(const std::wstring& val);

	void SetToNull() {IReset();}
	plGeneric& operator=(const bool& val);
	plGeneric& operator=(const int& val);
	plGeneric& operator=(const double& val);
	plGeneric& operator=(const char* val);
	plGeneric& operator=(const std::string& val);
	plGeneric& operator=(const wchar_t* val);
	plGeneric& operator=(const std::wstring& val);

	// the cast functions will NOT cast from one type to another, use
	// GetType() to determine the type of parameter, then cast it to that type
	GenericType GetType() const {return fType;}
	operator const bool() const {return fBoolVal;}
	operator const int() const {return fIntVal;}
	operator const double() const {return fFloatVal;}
	operator const wchar_t*() const {return fStringVal.c_str();}
	operator const std::wstring() const {return fStringVal;}

	int Write(hsStream* stream);
	int Read(hsStream* stream);
};

#endif // __PLGENERIC_H__