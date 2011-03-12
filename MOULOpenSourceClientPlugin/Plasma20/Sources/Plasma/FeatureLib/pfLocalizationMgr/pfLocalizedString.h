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
//////////////////////////////////////////////////////////////////////
//
// pfLocalizedString - a small class to handle localized strings and
//                     which can take parameters (like %s or %1s) and
//                     also can be easily translated to XML format
//
//////////////////////////////////////////////////////////////////////

#ifndef _pfLocalizedString_h
#define _pfLocalizedString_h

#include "hsTypes.h"
#include "hsStlUtils.h"


//// pfLocalizedString Class Definition //////////////////////////////
//	a small class to handle localized strings and which can take
//  parameters (like %s or %1s) and also can be easily translated to
//  XML format

class pfLocalizedString
{
protected:
	// stores all basic text and param information in a nice, easy-to-use block of data
	struct textBlock
	{
		bool		fIsParam; // if true, then this is a parameter, not a string
		std::wstring	fText;
		UInt8		fParamIndex;

		textBlock() : fIsParam(false), fParamIndex(0) {}
	};

	std::vector<textBlock>	fText; // the individual text elements that make up this string
	std::wstring			fXMLRep; // the XML representation of this string
	std::wstring			fPlainTextRep; // the plain text representation of this string
	UInt16				fNumArguments; // number of arguments this string has

	void IConvertFromPlainText(const std::wstring & plainText);
	void IUpdatePlainText(); // from the internal representation
	void IConvertFromXML(const std::wstring & xml);
	void IUpdateXML(); // from the internal representation
public:
	pfLocalizedString() : fNumArguments(0) {}
	pfLocalizedString(const wchar_t *plainText);
	pfLocalizedString(const std::wstring & plainText);
	virtual ~pfLocalizedString() {}

	// To translate to and from xml format (where <, > and other signs can't be used)
	void FromXML(const std::wstring & xml);
	std::wstring ToXML() {return fXMLRep;}

	UInt16 GetArgumentCount() {return fNumArguments;}

	// Various operators, they all work pretty much the same as the standard string or wstring operators
	// but note that the all work on the plain text representation (not the XML representation)
	bool operator<(pfLocalizedString &obj);
	bool operator>(pfLocalizedString &obj);
	bool operator==(pfLocalizedString &obj);
	bool operator<=(pfLocalizedString &obj);
	bool operator>=(pfLocalizedString &obj);
	bool operator!=(pfLocalizedString &obj);

	operator const wchar_t *() {return fPlainTextRep.c_str();}
	operator std::wstring() {return fPlainTextRep;}

	pfLocalizedString operator+(pfLocalizedString &obj);
	pfLocalizedString &operator+=(pfLocalizedString &obj);

	pfLocalizedString &operator=(const std::wstring & plainText);
	pfLocalizedString &operator=(const wchar_t *plainText);

	// Specialized operator for replacing text with arguments
	std::wstring operator%(const std::vector<std::wstring> & arguments);
};

#endif
