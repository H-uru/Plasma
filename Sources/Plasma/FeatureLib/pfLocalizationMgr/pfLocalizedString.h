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

#ifndef _pfLocalizedString_h
#define _pfLocalizedString_h

#include "HeadSpin.h"
#include <string_theory/string>
#include <vector>

//// pfLocalizedString Class Definition //////////////////////////////
//  a small class to handle localized strings and which can take
//  parameters (like %s or %1s) and also can be easily translated to
//  XML format

class pfLocalizedString
{
protected:
    // stores all basic text and param information in a nice, easy-to-use block of data
    struct textBlock
    {
        bool        fIsParam; // if true, then this is a parameter, not a string
        ST::string  fText;
        uint8_t     fParamIndex;

        textBlock() : fIsParam(false), fParamIndex(0) {}
    };

    std::vector<textBlock>    fText;   // the individual text elements that make up this string
    ST::string fXMLRep;         // the XML representation of this string
    ST::string fPlainTextRep;   // the plain text representation of this string
    uint16_t fNumArguments;     // number of arguments this string has

    void IParameterize(const ST::string & inString);
    void IConvertFromPlainText(const ST::string & plainText);
    void IUpdatePlainText(); // from the internal representation
    void IConvertFromXML(const ST::string & xml);
    void IUpdateXML(); // from the internal representation
public:
    pfLocalizedString() : fNumArguments(0) {}
    pfLocalizedString(const ST::string & plainText);
    virtual ~pfLocalizedString() {}

    // To translate to and from xml format (where <, > and other signs can't be used)
    void FromXML(const ST::string & xml);
    ST::string ToXML() const { return fXMLRep; }

    uint16_t GetArgumentCount() const { return fNumArguments; }

    // Various operators, they all work pretty much the same as the standard string or wstring operators
    // but note that the all work on the plain text representation (not the XML representation)
    bool operator<(pfLocalizedString &obj) const;
    bool operator>(pfLocalizedString &obj) const;
    bool operator==(pfLocalizedString &obj) const;
    bool operator<=(pfLocalizedString &obj) const;
    bool operator>=(pfLocalizedString &obj) const;
    bool operator!=(pfLocalizedString &obj) const;

    operator ST::string() const { return fPlainTextRep; }
    operator bool() const { return !fPlainTextRep.empty(); }

    pfLocalizedString operator+(pfLocalizedString &obj);
    pfLocalizedString &operator+=(pfLocalizedString &obj);
    pfLocalizedString &operator=(const ST::string & plainText);

    // Specialized operator for replacing text with arguments
    ST::string operator%(const std::vector<ST::string> & arguments) const;
};

#endif
