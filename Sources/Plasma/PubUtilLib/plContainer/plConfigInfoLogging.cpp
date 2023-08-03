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
#include "plConfigInfo.h"

#include <string_theory/format>

plConfigInfoLogging::plConfigInfoLogging()
{
}

plConfigInfoLogging::~plConfigInfoLogging()
{
}

bool plConfigInfoLogging::GetAllValues(std::vector<ST::string>& values, const ST::string & section, const ST::string & key, const ST::string & desc)
{
    ST::string descwdef = ST::format("\"Multiple Entries\"  # {}", desc);
    fLog.AddValue(section,key,descwdef,kReplaceIfExists);

    values = fConfigInfo.GetAllValues(section,key);
    return values.size() != 0;
}

#if USE_MULT_SECTIONS

bool plConfigInfoLogging::GetValueAny(ST::string& retval, const ST::string & key, const ST::string & desc, const ST::string & defval)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    bool found;
    retval = fConfigInfo.GetValueAny(key,defval,&found);
    return found;
}

bool plConfigInfoLogging::GetValueAny(int &retval, const ST::string & key, const ST::string & desc, int defval)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    bool found;
    retval = fConfigInfo.GetValueAny(key,defval,&found);
    return found;
}

bool plConfigInfoLogging::GetValueAny(bool &retval, const ST::string & key, const ST::string & desc, bool defval)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    bool found;
    retval = ( fConfigInfo.GetValueAny(key,(int)defval,&found)!=0 );
    return found;
}

bool plConfigInfoLogging::GetValueAny(float& retval, const ST::string & key, const ST::string & desc, float defval)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    bool found;
    retval = fConfigInfo.GetValueAny(key,defval,&found);
    return found;
}

bool plConfigInfoLogging::GetValueAny(double& retval, const ST::string & key, const ST::string & desc, double defval)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    bool found;
    retval = fConfigInfo.GetValueAny(key,defval,&found);
    return found;
}

bool plConfigInfoLogging::GetAllValuesAny(std::vector<ST::string>& values, const ST::string & key, const ST::string & desc)
{
    fLog.AddValue("ANY SECTION",key,desc,kReplaceIfExists);

    values = fConfigInfo.GetAllValuesAny(key);
    return values.size() != 0;
}

bool plConfigInfoLogging::GetValueIn(ST::string& retval, const ST::string & key,  const ST::string & desc, const ST::string & defval, const char * section1, ... /*, nullptr*/)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<ST::string> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);

    return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(ST::string& retval, const ST::string & key,  const ST::string & desc, const ST::string & defval, std::vector<ST::string> & sections )
{
    std::vector<ST::string>::iterator si = sections.begin();
    while (si != sections.end())
    {
        fLog.AddValue(*si,key,desc,kReplaceIfExists);
        si++;
    }

    bool found;
    retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
    return found;
}

bool plConfigInfoLogging::GetValueIn(int& retval, const ST::string & key,  const ST::string & desc, int defval, const char * section1, ... /*, nullptr*/)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<ST::string> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);

    return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(int& retval, const ST::string & key,  const ST::string & desc, int defval, std::vector<ST::string> & sections )
{
    std::vector<ST::string>::iterator si = sections.begin();
    while (si != sections.end())
    {
        fLog.AddValue(*si,key,desc,kReplaceIfExists);
        si++;
    }

    bool found;
    retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
    return found;
}

bool plConfigInfoLogging::GetValueIn(bool& retval, const ST::string & key,  const ST::string & desc, bool defval, const char * section1, ... /*, nullptr*/)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<ST::string> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);

    return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(bool& retval, const ST::string & key,  const ST::string & desc, bool defval, std::vector<ST::string> & sections )
{
    std::vector<ST::string>::iterator si = sections.begin();
    while (si != sections.end())
    {
        fLog.AddValue(*si,key,desc,kReplaceIfExists);
        si++;
    }

    bool found;
    retval = ( fConfigInfo.GetValueIn(key,(int)defval,&found,sections)!=0 );
    return found;
}

bool plConfigInfoLogging::GetValueIn(float& retval, const ST::string & key,  const ST::string & desc, double defval, const char * section1, ... /*, nullptr*/)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<ST::string> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);

    return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(float& retval, const ST::string & key,  const ST::string & desc, double defval, std::vector<ST::string> & sections )
{
    std::vector<ST::string>::iterator si = sections.begin();
    while (si != sections.end())
    {
        fLog.AddValue(*si,key,desc,kReplaceIfExists);
        si++;
    }

    bool found;
    retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
    return found;
}

bool plConfigInfoLogging::GetValueIn(double& retval, const ST::string & key,  const ST::string & desc, double defval, const char * section1, ... /*, nullptr*/)
{
    const char * section = section1;
    va_list va;
    va_start(va,section1);
    std::vector<ST::string> sections;
    while (section)
    {
        sections.push_back( section );
        section = va_arg(va,const char *);
    }
    va_end(va);

    return GetValueIn(retval,key,desc,defval,sections);
}

bool plConfigInfoLogging::GetValueIn(double& retval, const ST::string & key,  const ST::string & desc, double defval, std::vector<ST::string> & sections )
{
    std::vector<ST::string>::iterator si = sections.begin();
    while (si != sections.end())
    {
        fLog.AddValue(*si,key,desc,kReplaceIfExists);
        si++;
    }

    bool found;
    retval = fConfigInfo.GetValueIn(key,defval,&found,sections);
    return found;
}

#endif
