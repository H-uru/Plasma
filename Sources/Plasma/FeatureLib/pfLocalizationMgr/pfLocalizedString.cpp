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

#include "pfLocalizedString.h"

#include "HeadSpin.h"

#include <algorithm>
#include <regex>
#include <string_theory/string_stream>


//////////////////////////////////////////////////////////////////////
//// pfLocalizedString functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// Constructors ////////////////////////////////////////////////////

pfLocalizedString::pfLocalizedString(const ST::string & plainText)
    : fNumArguments(0)
{
    IConvertFromPlainText(plainText);
}

//// IParameterize ///////////////////////////////////////////////////

void pfLocalizedString::IParameterize(const ST::string & inString)
{
    textBlock curTextBlock;
    fNumArguments = 0;      // Reset the argument count.
    fText.clear();          // Reset the text blocks.

    ST::string remainder = inString;
    ST::string_stream newText;
    int curParameter = 0;
    ST_ssize_t nextToken = -1;

    while (!remainder.empty())
    {
        // Check if we have any params.
        nextToken = remainder.find('%');
        if (nextToken != -1)
        {
            // Check it's not escaped.
            if ((nextToken == 0) || ((nextToken > 0) && (remainder[nextToken-1] != '\\')))
            {
                // Check if it has an end (ignoring any terminators we need to cross a space to find).
                ST_ssize_t endToken = remainder.substr(nextToken).find('s');
                if ((endToken != -1) && !remainder.substr(nextToken, endToken).contains(' '))
                {
                    // Store existing block if it contains anything.
                    newText << remainder.substr(0, nextToken);
                    curTextBlock.fText = newText.to_string().replace("\\\\", "\\");
                    if (!curTextBlock.fText.empty())
                    {
                        fText.push_back(curTextBlock);
                        newText.truncate();
                    }

                    if (endToken == nextToken + 1)
                    {
                        // Store non-indexed param block.
                        curTextBlock.fIsParam = true;
                        curTextBlock.fParamIndex = curParameter++;
                        curTextBlock.fText = "";
                        fText.push_back(curTextBlock);
                    }
                    else
                    {
                        // Store indexed param block.
                        curTextBlock.fIsParam = true;
                        curTextBlock.fParamIndex = remainder.substr(nextToken + 1, endToken - 1).to_int(10) - 1; // args start at 1
                        curTextBlock.fText = "";
                        fText.push_back(curTextBlock);
                    }
                    curTextBlock.fIsParam = false;
                    curTextBlock.fParamIndex = 0;
                    fNumArguments++;

                    // Continue, using the remaining string.
                    remainder = remainder.substr(nextToken + endToken + 1);
                }
                else
                {
                    // We have an unescaped but unterminated %.
                    // For now, let's just pretend it was escaped;
                    // This way they'll show up visibly in-game and will be reported.
                    newText << "%";
                    remainder = remainder.substr(nextToken + 1);
                }
            }
            else
            {
                // Copy the text up to the escape character, skip it, and continue.
                newText << remainder.substr(0, nextToken - 1) << '%';
                remainder = remainder.substr(nextToken + 1);
            }
        }
        else
        {
            // We're done.  Copy the remaining text and finish.
            newText << remainder;
            remainder = "";
            curTextBlock.fText = newText.to_string().replace("\\\\", "\\");
            if (!curTextBlock.fText.empty())
            {
                fText.push_back(curTextBlock);
                newText.truncate();
            }
        }
    }
}

//// IConvertFromPlainText ///////////////////////////////////////////

void pfLocalizedString::IConvertFromPlainText(const ST::string & plainText)
{
    fPlainTextRep = plainText;
    IParameterize(fPlainTextRep);

    IUpdateXML();
}

//// IUpdatePlainText ////////////////////////////////////////////////

void pfLocalizedString::IUpdatePlainText()
{
    ST::string_stream ss;

    for (std::vector<textBlock>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        textBlock curTextBlock = fText[curIndex];

        if (curTextBlock.fIsParam)
        {
            // Fill in parameter value.
            ss << "%" << curTextBlock.fParamIndex + 1 << "s";
        }
        else
        {
            // Escape special characters.
            ss << curTextBlock.fText.replace("\\","\\\\").replace("%","\\%");
        }
    }
    fPlainTextRep = ss.to_string();
}

//// IConvertFromXML /////////////////////////////////////////////////

void pfLocalizedString::IConvertFromXML(const ST::string & xml)
{
    IParameterize(xml);

    IUpdatePlainText();
    IUpdateXML();
}

//// IUpdateXML //////////////////////////////////////////////////////

void pfLocalizedString::IUpdateXML()
{
    // If we find anything that looks like an esHTML tag (eg <p>, <pb>, etc.) then we want
    // to use CDATA. Since the text is small an contiguous, we will assume that it occurs
    // in the same block. If not, and nothing else trips us... Whatever.
    bool wantCData = std::any_of(
        fText.cbegin(), fText.cend(),
        [](const textBlock& block) {
            static const std::regex regex("<.+>");
            return std::regex_search(block.fText.cbegin(), block.fText.cend(), regex);
        }
    );

    ST::string_stream ss;
    if (wantCData)
        ss << "<![CDATA[";

    for (const auto& curTextBlock : fText) {
        if (curTextBlock.fIsParam) {
            // Fill in parameter value.
            ss << "%" << curTextBlock.fParamIndex + 1 << "s";
        } else if (!wantCData) {
            // Encode XML entities.
            ss << curTextBlock.fText.replace("%", "\\%").replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
        } else {
            ss << curTextBlock.fText;
        }
    }

    if (wantCData)
        ss << "]]>";
    fXMLRep = ss.to_string();
}

//// FromXML /////////////////////////////////////////////////////////

void pfLocalizedString::FromXML(const ST::string & xml)
{
    IConvertFromXML(xml);
}

//// Operators ///////////////////////////////////////////////////////

bool pfLocalizedString::operator<(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) < 0);
}

bool pfLocalizedString::operator>(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) > 0);
}

bool pfLocalizedString::operator==(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) == 0);
}

bool pfLocalizedString::operator<=(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) <= 0);
}

bool pfLocalizedString::operator>=(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) >= 0);
}

bool pfLocalizedString::operator!=(pfLocalizedString &obj)
{
    return (fPlainTextRep.compare(obj.fPlainTextRep) != 0);
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

pfLocalizedString &pfLocalizedString::operator=(const ST::string & plainText)
{
    IConvertFromPlainText(plainText);
    return *this;
}

ST::string pfLocalizedString::operator%(const std::vector<ST::string> & arguments)
{
    ST::string_stream ss;
    for (std::vector<ST::string>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        if (fText[curIndex].fIsParam)
        {
            int curParam = fText[curIndex].fParamIndex;
            if (curParam < arguments.size())
                ss << arguments[curParam];
        }
        else
            ss << fText[curIndex].fText;
    }
    return ss.to_string();
}
