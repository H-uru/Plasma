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
#include "hsUtils.h"
#include "plGeneric.h"

plGeneric::plGeneric(): fType(kNull), fBoolVal(false), fIntVal(0), fFloatVal(0.0) {}

plGeneric::plGeneric(const bool& val): fType(kBool), fBoolVal(val), fIntVal(0), fFloatVal(0.0) {}

plGeneric::plGeneric(const int& val): fType(kInt), fBoolVal(false), fIntVal(val), fFloatVal(0.0) {}

plGeneric::plGeneric(const double& val): fType(kFloat), fBoolVal(false), fIntVal(0), fFloatVal(val) {}

plGeneric::plGeneric(const char* val): fType(kString), fBoolVal(false), fIntVal(0), fFloatVal(0.0)
{
	wchar_t* temp = hsStringToWString(val);
	fStringVal = temp;
	delete [] temp;
}

plGeneric::plGeneric(const std::string& val): fType(kString), fBoolVal(false), fIntVal(0), fFloatVal(0.0)
{
	wchar_t* temp = hsStringToWString(val.c_str());
	fStringVal = temp;
	delete [] temp;
}

plGeneric::plGeneric(const wchar_t* val): fType(kString), fBoolVal(false), fIntVal(0), fFloatVal(0.0),
fStringVal(val) {}

plGeneric::plGeneric(const std::wstring& val): fType(kString), fBoolVal(false), fIntVal(0), fFloatVal(0.0),
fStringVal(val) {}

void plGeneric::IReset()
{
	fType = kNull;
	fBoolVal = false;
	fIntVal = 0;
	fFloatVal = 0.0;
	fStringVal = L"";
}

plGeneric& plGeneric::operator=(const bool& val)
{
	IReset();
	fType = kBool;
	fBoolVal = val;
	return *this;
}

plGeneric& plGeneric::operator=(const int& val)
{
	IReset();
	fType = kInt;
	fIntVal = val;
	return *this;
}

plGeneric& plGeneric::operator=(const double& val)
{
	IReset();
	fType = kFloat;
	fFloatVal = val;
	return *this;
}

plGeneric& plGeneric::operator=(const char* val)
{
	IReset();
	fType = kString;
	wchar_t* temp = hsStringToWString(val);
	fStringVal = temp;
	delete [] temp;
	return *this;
}

plGeneric& plGeneric::operator=(const std::string& val)
{
	IReset();
	fType = kString;
	wchar_t* temp = hsStringToWString(val.c_str());
	fStringVal = temp;
	delete [] temp;
	return *this;
}

plGeneric& plGeneric::operator=(const wchar_t* val)
{
	IReset();
	fType = kString;
	fStringVal = val;
	return *this;
}

plGeneric& plGeneric::operator=(const std::wstring& val)
{
	IReset();
	fType = kString;
	fStringVal = val;
	return *this;
}

int plGeneric::Write(hsStream* stream)
{
	stream->WriteByte((UInt8)fType);

	switch (fType)
	{
	case kNull:
		break; // nothing to write

	case kBool:
		stream->WriteBool(fBoolVal);
		break;

	case kInt:
		stream->WriteSwap(fIntVal);
		break;

	case kFloat:
		stream->WriteSwap(fFloatVal);
		break;

	case kString:
		stream->WriteSafeWString(fStringVal.c_str());
		break;
	}
	return stream->GetPosition();
}

int plGeneric::Read(hsStream* stream)
{
	IReset();
	fType = (GenericType)stream->ReadByte();
	switch (fType)
	{
	case kNull:
		break; // nothing to read

	case kBool:
		fBoolVal = (stream->ReadBool() != 0);
		break;

	case kInt:
		stream->ReadSwap(&fIntVal);
		break;

	case kFloat:
		stream->ReadSwap(&fFloatVal);
		break;

	case kString:
		{
			wchar_t* temp = stream->ReadSafeWString();
			if (temp)
			{
				fStringVal = temp;
				delete [] temp;
			}
		}
		break;
	}
	return stream->GetPosition();
}