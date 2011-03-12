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

#include "hsTypes.h"
#include "hsUtils.h"

#include "pfLocalizedString.h"

#if HS_BUILD_FOR_MAC
#include <bxwchar.h>
#endif

//////////////////////////////////////////////////////////////////////
//// pfLocalizedString functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// Constructors ////////////////////////////////////////////////////

pfLocalizedString::pfLocalizedString(const wchar_t *plainText)
{
	fNumArguments = 0;
	IConvertFromPlainText(plainText);
}

pfLocalizedString::pfLocalizedString(const std::wstring & plainText)
{
	fNumArguments = 0;
	IConvertFromPlainText(plainText);
}

//// IConvertFromPlainText ///////////////////////////////////////////

void pfLocalizedString::IConvertFromPlainText(const std::wstring & plainText)
{
	textBlock curTextBlock;
	fText.clear();
	fPlainTextRep = plainText;
	fNumArguments = 0; // reset the argument count
	int curParameter = 0;

	for (std::wstring::size_type curIndex = 0; curIndex < plainText.size(); curIndex++)
	{
		wchar_t curChar = plainText[curIndex];
		bool isLastChar = (curIndex == (plainText.length() - 1));
		switch (curChar)
		{
		case L'\\':
			if (!isLastChar)
			{
				// we need to see the next character
				curIndex++;
				wchar_t nextChar = plainText[curIndex];
				if ((nextChar == L'%')||(nextChar == L'\\'))
				{
					// we recognize it as an escaped character, so add it to the text
					curTextBlock.fText += nextChar;
				}
				// otherwise we don't recognize it and it will be skipped
			}
			// if it's the last char, just drop it
			break;
		case L'%':
			if (!isLastChar)
			{
				// we need to grab the trailing s character
				std::wstring::size_type endArgPos = plainText.find(L"s", curIndex);
				if (endArgPos != std::wstring::npos) // make sure the s exists
				{
					if (endArgPos == (curIndex + 1)) // no number specifier
					{
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = true;
						curTextBlock.fParamIndex = curParameter;
						curParameter++;
						curTextBlock.fText = L"";
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = false;
						curTextBlock.fParamIndex = 0;
					}
					else // number specified
					{
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = true;
						curTextBlock.fText = L"";

						std::wstring number = plainText.substr(curIndex + 1, (endArgPos - (curIndex + 1)));
						curTextBlock.fParamIndex = _wtoi(number.c_str()) - 1; // args are 1-based, vectors are 0-based
						fText.push_back(curTextBlock);

						curTextBlock.fIsParam = false;
						curTextBlock.fParamIndex = 0;
					}
					fNumArguments++; // increment our argument count
					curIndex = endArgPos; // update our position
				}
				// if s didn't exist, we just skip this % sign
			}
			// if it was the last char, we just skip this % sign
			break;
		default:
			curTextBlock.fText += curChar;
			break;
		}
	}
	fText.push_back(curTextBlock);

	IUpdateXML();
}

//// IUpdatePlainText ////////////////////////////////////////////////

void pfLocalizedString::IUpdatePlainText()
{
	fPlainTextRep = L"";
	for (std::vector<std::wstring>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
	{
		textBlock curTextBlock = fText[curIndex];

		if (curTextBlock.fIsParam)
		{
			std::wstring paramStr = L"%";
			wchar_t buff[256];
			_itow(curTextBlock.fParamIndex + 1, buff, 10);
			paramStr += buff;
			paramStr += L"s";
			fPlainTextRep += paramStr;
		}
		else
		{
			// otherwise, we need to copy all the text over, making sure that % and \ are properly escaped
			for (std::wstring::size_type curChar = 0; curChar < curTextBlock.fText.size(); curChar++)
			{
				if ((curTextBlock.fText[curChar] == L'\\')||(curTextBlock.fText[curChar] == L'%'))
					fPlainTextRep += L"\\";
				fPlainTextRep += curTextBlock.fText[curChar];
			}
		}
	}
}

//// IConvertFromXML /////////////////////////////////////////////////

void pfLocalizedString::IConvertFromXML(const std::wstring & xml)
{
	textBlock curTextBlock;
	fText.clear();
	fNumArguments = 0; // reset the argument counter
	int curParameter = 0;

	for (std::wstring::size_type curIndex = 0; curIndex < xml.length(); curIndex++)
	{
		wchar_t curChar = xml[curIndex];
		bool isLastChar = (curIndex == (xml.length() - 1));
		switch (curChar)
		{ // expat handles the &gt; &lt; and so on stuff for us
		case L'\\': // but we want to be able to escape the % sign and the \ character
			if (!isLastChar)
			{
				// we need to see the next character
				curIndex++;
				wchar_t nextChar = xml[curIndex];
				if ((nextChar == L'%')||(nextChar == L'\\'))
				{
					// we recognize it as an escaped character, so add it to the text
					curTextBlock.fText += nextChar;
				}
				// otherwise we don't recognize it and it will be skipped
			}
			// if it's the last char, just drop it
			break;
		case L'%':
			if (!isLastChar)
			{
				// we need to grab the trailing s character
				std::wstring::size_type endArgPos = xml.find(L"s", curIndex);
				if (endArgPos != std::wstring::npos) // make sure the s exists
				{
					if (endArgPos == (curIndex + 1)) // no number specifier
					{
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = true;
						curTextBlock.fParamIndex = curParameter;
						curParameter++;
						curTextBlock.fText = L"";
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = false;
						curTextBlock.fParamIndex = 0;
					}
					else // number specified
					{
						fText.push_back(curTextBlock);
						curTextBlock.fIsParam = true;
						curTextBlock.fText = L"";

						std::wstring number = xml.substr(curIndex + 1, (endArgPos - (curIndex + 1)));
						curTextBlock.fParamIndex = _wtoi(number.c_str()) - 1; // args are 1-based, vectors are 0-based
						fText.push_back(curTextBlock);

						curTextBlock.fIsParam = false;
						curTextBlock.fParamIndex = 0;
					}
					fNumArguments++; // increment the number of arguments
					curIndex = endArgPos; // update our position
				}
				// if s didn't exist, we just skip this % sign
			}
			// if it was the last char, we just skip this % sign
			break;
		default:
			curTextBlock.fText += curChar;
			break;
		}
	}
	fText.push_back(curTextBlock);

	IUpdatePlainText();
	IUpdateXML(); // we don't really get pure xml from the parser (since it auto translates all the &x; stuff)
}

//// IUpdateXML //////////////////////////////////////////////////////

void pfLocalizedString::IUpdateXML()
{
	fXMLRep = L"";
	for (std::vector<std::wstring>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
	{
		textBlock curTextBlock = fText[curIndex];

		if (curTextBlock.fIsParam)
		{
			std::wstring paramStr = L"%";
			wchar_t buff[256];
			_itow(curTextBlock.fParamIndex + 1, buff, 10);
			paramStr += buff;
			paramStr += L"s";
			fXMLRep += paramStr;
		}
		else
		{
			// otherwise, we need to copy all the text over, making sure that %, &, <, and > are properly converted
			for (std::wstring::size_type curChar = 0; curChar < curTextBlock.fText.size(); curChar++)
			{
				if (curTextBlock.fText[curChar] == L'%')
					fXMLRep += L"\\%";
				else if (curTextBlock.fText[curChar] == L'&')
					fXMLRep += L"&amp;";
				else if (curTextBlock.fText[curChar] == L'<')
					fXMLRep += L"&lt;";
				else if (curTextBlock.fText[curChar] == L'>')
					fXMLRep += L"&gt;";
				else
					fXMLRep += curTextBlock.fText[curChar];
			}
		}
	}
}

//// FromXML /////////////////////////////////////////////////////////

void pfLocalizedString::FromXML(const std::wstring & xml)
{
	IConvertFromXML(xml);
}

//// Operators ///////////////////////////////////////////////////////

bool pfLocalizedString::operator<(pfLocalizedString &obj)
{
	return (fPlainTextRep < obj.fPlainTextRep);
}

bool pfLocalizedString::operator>(pfLocalizedString &obj)
{
	return (fPlainTextRep > obj.fPlainTextRep);
}

bool pfLocalizedString::operator==(pfLocalizedString &obj)
{
	return (fPlainTextRep == obj.fPlainTextRep);
}

bool pfLocalizedString::operator<=(pfLocalizedString &obj)
{
	return (fPlainTextRep <= obj.fPlainTextRep);
}

bool pfLocalizedString::operator>=(pfLocalizedString &obj)
{
	return (fPlainTextRep >= obj.fPlainTextRep);
}

bool pfLocalizedString::operator!=(pfLocalizedString &obj)
{
	return (fPlainTextRep != obj.fPlainTextRep);
}

pfLocalizedString pfLocalizedString::operator+(pfLocalizedString &obj)
{
	fPlainTextRep += obj.fPlainTextRep;
	IConvertFromPlainText(fPlainTextRep);
	return *this;
}

pfLocalizedString &pfLocalizedString::operator+=(pfLocalizedString &obj)
{
	fPlainTextRep += obj.fPlainTextRep;
	IConvertFromPlainText(fPlainTextRep);
	return *this;
}

pfLocalizedString &pfLocalizedString::operator=(const std::wstring & plainText)
{
	IConvertFromPlainText(plainText);
	return *this;
}

pfLocalizedString &pfLocalizedString::operator=(const wchar_t *plainText)
{
	IConvertFromPlainText(plainText);
	return *this;
}

std::wstring pfLocalizedString::operator%(const std::vector<std::wstring> & arguments)
{
	std::wstring retVal = L"";
	for (std::vector<std::wstring>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
	{
		if (fText[curIndex].fIsParam)
		{
			int curParam = fText[curIndex].fParamIndex;
			if (curParam < arguments.size())
				retVal += arguments[curParam];
		}
		else
			retVal += fText[curIndex].fText;
	}
	return retVal;
}
