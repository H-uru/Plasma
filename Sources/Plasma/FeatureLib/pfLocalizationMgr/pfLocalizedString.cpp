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


//////////////////////////////////////////////////////////////////////
//// pfLocalizedString functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// Constructors ////////////////////////////////////////////////////

pfLocalizedString::pfLocalizedString(const plString & plainText)
    : fNumArguments(0)
{
    IConvertFromPlainText(plainText);
}

//// IParameterize ///////////////////////////////////////////////////

void pfLocalizedString::IParameterize(const plString & inString)
{
    textBlock curTextBlock;
    fNumArguments = 0;      // Reset the argument count.
    fText.clear();          // Reset the text blocks.

    plString remainder = inString;
    plStringStream newText;
    int curParameter = 0;
    int nextToken = -1;

    while (!remainder.IsEmpty())
    {
        // Check if we have any params.
        nextToken = remainder.Find("%");
        if (nextToken != -1)
        {
            // Check it's not escaped.
            if ((nextToken == 0) || ((nextToken > 0) && (remainder.CharAt(nextToken-1) != '\\')))
            {
                // Check if it has an end (ignoring any terminators we need to cross a space to find).
                int endToken = remainder.Substr(nextToken).Find("s");
                if ((endToken != -1) && (remainder.Substr(nextToken, endToken).Find(" ") == -1))
                {
                    // Store existing block if it contains anything.
                    newText << remainder.Substr(0, nextToken);
                    curTextBlock.fText = newText.GetString().Replace("\\\\", "\\");
                    if (!curTextBlock.fText.IsEmpty())
                    {
                        fText.push_back(curTextBlock);
                        newText.Truncate();
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
                        curTextBlock.fParamIndex = remainder.Substr(nextToken + 1, endToken - 1).ToInt(10) - 1; // args start at 1
                        curTextBlock.fText = "";
                        fText.push_back(curTextBlock);
                    }
                    curTextBlock.fIsParam = false;
                    curTextBlock.fParamIndex = 0;
                    fNumArguments++;

                    // Continue, using the remaining string.
                    remainder = remainder.Substr(nextToken + endToken + 1);
                }
                else
                {
                    // We have an unescaped but unterminated %.
                    // For now, let's just pretend it was escaped;
                    // This way they'll show up visibly in-game and will be reported.
                    newText << "%";
                    remainder = remainder.Substr(nextToken + 1);
                }
            }
            else
            {
                // Copy the text up to the escape character, skip it, and continue.
                newText << remainder.Substr(0, nextToken - 1) << '%';
                remainder = remainder.Substr(nextToken + 1);
            }
        }
        else
        {
            // We're done.  Copy the remaining text and finish.
            newText << remainder;
            remainder = "";
            curTextBlock.fText = newText.GetString().Replace("\\\\", "\\");
            if (!curTextBlock.fText.IsEmpty())
            {
                fText.push_back(curTextBlock);
                newText.Truncate();
            }
        }
    }
}

//// IConvertFromPlainText ///////////////////////////////////////////

void pfLocalizedString::IConvertFromPlainText(const plString & plainText)
{
    fPlainTextRep = plainText;
    IParameterize(fPlainTextRep);

    IUpdateXML();
}

//// IUpdatePlainText ////////////////////////////////////////////////

void pfLocalizedString::IUpdatePlainText()
{
    plStringStream ss;

    for (std::vector<textBlock>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        textBlock curTextBlock = fText[curIndex];

        if (curTextBlock.fIsParam)
        {
            // Fill in parameter value.
            ss << "%%" << curTextBlock.fParamIndex + 1 << "s";
        }
        else
        {
            // Escape special characters.
            ss << curTextBlock.fText.Replace("\\","\\\\").Replace("%","\\%");
        }
    }
    fPlainTextRep = ss.GetString();
}

//// IConvertFromXML /////////////////////////////////////////////////

void pfLocalizedString::IConvertFromXML(const plString & xml)
{
    IParameterize(xml);

    IUpdatePlainText();
    IUpdateXML();
}

//// IUpdateXML //////////////////////////////////////////////////////

void pfLocalizedString::IUpdateXML()
{
    plStringStream ss;
    for (std::vector<plString>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
    {
        textBlock curTextBlock = fText[curIndex];

        if (curTextBlock.fIsParam)
        {
            // Fill in parameter value.
            ss << "%%" << curTextBlock.fParamIndex + 1 << "s";
        }
        else
        {
            // Encode XML entities.
            ss << curTextBlock.fText.Replace("%", "\\%").Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");
        }
    }
    fXMLRep = ss.GetString();
}

//// FromXML /////////////////////////////////////////////////////////

void pfLocalizedString::FromXML(const plString & xml)
{
    IConvertFromXML(xml);
}

//// Operators ///////////////////////////////////////////////////////

bool pfLocalizedString::operator<(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) < 0);
}

bool pfLocalizedString::operator>(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) > 0);
}

bool pfLocalizedString::operator==(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) == 0);
}

bool pfLocalizedString::operator<=(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) <= 0);
}

bool pfLocalizedString::operator>=(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) >= 0);
}

bool pfLocalizedString::operator!=(pfLocalizedString &obj)
{
    return (fPlainTextRep.Compare(obj.fPlainTextRep) != 0);
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
    plStringStream ss;
    for (std::vector<plString>::size_type curIndex = 0; curIndex < fText.size(); curIndex++)
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
    return ss.GetString();
}
