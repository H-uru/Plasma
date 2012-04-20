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
//////////////////////////////////////////////////////////////////////
//
// pfLocalizedString - a small class to handle localized strings and
//                     which can take parameters (like %s or %1s) and
//                     also can be easily translated to XML format
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "pfLocalizedString.h"

// MinGW sucks
#if defined(_WIN32) && !defined(_MSC_VER)
#   define swprintf _snwprintf
#endif

//////////////////////////////////////////////////////////////////////
//// pfLocalizedString functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// Constructors ////////////////////////////////////////////////////

pfLocalizedString::pfLocalizedString(const plString & plainText)
{
    fNumArguments = 0;
    IConvertFromPlainText(plainText);
}

//// IConvertFromPlainText ///////////////////////////////////////////

void pfLocalizedString::IConvertFromPlainText(const plString & plainText)
{
    textBlock curTextBlock;
    fText.clear();
    fPlainTextRep = plainText;
    fNumArguments = 0; // reset the argument count
    int curParameter = 0;

    plString::iterator iter = plainText.GetIterator();
    while (!iter.AtEnd())
    {
        wchar_t curChar = *iter;
        bool isLastChar = iter.AtEnd();
        switch (curChar)
        {
        case L'\\':
            if (!isLastChar)
            {
                // we need to see the next character
                iter++;
                wchar_t nextChar = *iter;
                if ((nextChar == L'%')||(nextChar == L'\\'))
                {
                    // we recognize it as an escaped character, so add it to the text
                    curTextBlock.fText += plString::FromWchar((const wchar_t *)(nextChar));
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
                        curTextBlock.fParamIndex = (uint8_t)wcstol(number.c_str(), NULL, 10) - 1; // args are 1-based, vectors are 0-based
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
    fPlainTextRep = "";

    for (std::vector<textBlock>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        textBlock curTextBlock = fText[curIndex];

        if (curTextBlock.fIsParam)
        {
            fPlainTextRep += plString::Format("%%%ds", curTextBlock.fParamIndex + 1);
        }
        else
        {
            // otherwise, we need to copy all the text over, making sure that % and \ are properly escaped
            for (plString::iterator iter = curTextBlock.fText.GetIterator(); !iter.AtEnd(); iter++)
            {
                if (((*iter) == L'\\') || ((*iter) == L'%'))
                    fPlainTextRep += "\\";
                fPlainTextRep += plString::FromWchar((const wchar_t *)(*iter));
            }
        }
    }
}

//// IConvertFromXML /////////////////////////////////////////////////

void pfLocalizedString::IConvertFromXML(const plString & xml)
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
                        curTextBlock.fParamIndex = (uint8_t)wcstol(number.c_str(), nil, 10) - 1; // args are 1-based, vectors are 0-based
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
    fXMLRep = "";
    for (std::vector<plString>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        textBlock curTextBlock = fText[curIndex];

        if (curTextBlock.fIsParam)
        {
            plString paramStr = plString::Format("%%%ds", curTextBlock.fParamIndex + 1);
            fXMLRep += paramStr;
        }
        else
        {
            // otherwise, we need to copy all the text over, making sure that %, &, <, and > are properly converted
            for (plString::iterator iter = curTextBlock.fText.GetIterator(); !iter.AtEnd(); iter++)
            {
                UniChar curChar = *iter;
                if (curChar == L'%')
                    fXMLRep += "\\%";
                else if (curChar == L'&')
                    fXMLRep += "&amp;";
                else if (curChar == L'<')
                    fXMLRep += "&lt;";
                else if (curChar == L'>')
                    fXMLRep += "&gt;";
                else
                    fXMLRep += plString::FromWchar((const wchar_t *)curChar);
            }
        }
    }
}

//// FromXML /////////////////////////////////////////////////////////

void pfLocalizedString::FromXML(const plString & xml)
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

pfLocalizedString &pfLocalizedString::operator=(const plString & plainText)
{
    IConvertFromPlainText(plainText);
    return *this;
}

plString pfLocalizedString::operator%(const std::vector<plString> & arguments)
{
    plString retVal = "";
    for (std::vector<plString>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
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
