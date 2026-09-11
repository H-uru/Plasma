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

#include "plNetObjectDebugger.h"

#include <string_theory/string_stream>

#include "hsResMgr.h"

#include "pnKeyedObject/hsKeyedObject.h"

#include "plNetClientComm/plNetClientComm.h"
#include "plResMgr/plKeyFinder.h"
#include "plStatusLog/plStatusLog.h"

//
// return true if string matches objName according to flags
//
bool plNetObjectDebugger::DebugObject::StringMatches(const ST::string& str) const
{
    if (str.empty())
        return false;

    if (fFlags & kExactStringMatch)
        return str.compare(fObjName, ST::case_insensitive) == 0;

    if (fFlags & kEndStringMatch)
        return str.ends_with(fObjName, ST::case_insensitive);

    if (fFlags & kStartStringMatch)
        return str.starts_with(fObjName, ST::case_insensitive);

    if (fFlags & kSubStringMatch)
        return str.contains(fObjName, ST::case_insensitive);

    hsAssert(false, "missing flags");
    return false;
}

//
// if both objName and pageName are provided, and this object has page info,
//      return true if object matches both string and location.
// else just return true if object matches string
//
bool plNetObjectDebugger::DebugObject::ObjectMatches(const ST::string& objName, const ST::string& pageName)
{
    if (objName.empty())
        return false;

    if (pageName.empty() || (fFlags & kPageMatch)==0)
    {
        // only have enough info to match by objName
        return StringMatches(objName);
    }

    plLocation loc;
    loc = plKeyFinder::Instance().FindLocation(NetCommGetAge()->ageDatasetName, pageName);
    return (StringMatches(objName) && loc==fLoc);
}

//
// try to match by plLocation
//
bool plNetObjectDebugger::DebugObject::ObjectMatches(const hsKeyedObject* obj)
{
    if (!obj || !obj->GetKey())
        return false;

    if ((fFlags & kPageMatch)==0)
    {
        // match based on object name only
        return StringMatches(obj->GetKeyName());
    }

    return (obj->GetKey()->GetUoid().GetLocation()==fLoc);
}

/////////////////////////////////////////////////////////////////
// plNetObjectDebugger
/////////////////////////////////////////////////////////////////
plNetObjectDebugger::plNetObjectDebugger()
    : fStatusLog(plStatusLogMgr::GetInstance().CreateStatusLog(
          40, "NetObject.log",
          plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kTimestamp
      )),
      fDebugging()
{
}

plNetObjectDebugger::~plNetObjectDebugger()
{
    ClearAllDebugObjects();
    delete fStatusLog;
}

//
// STATIC
//
plNetObjectDebugger* plNetObjectDebugger::GetInstance()
{
    static plNetObjectDebugger gNetObjectDebugger;

    if (plNetObjectDebuggerBase::GetInstance() == nullptr)
        plNetObjectDebuggerBase::SetInstance(&gNetObjectDebugger);

    return &gNetObjectDebugger;
}

bool plNetObjectDebugger::AddDebugObject(ST::string objName, const ST::string& pageName)
{
    if (objName.empty())
        return false;

    //
    // set string matching flags
    //
    uint32_t flags=0;
    if (objName.front() == '*')
    {
        if (objName.back() == '*')
        {
            flags = kSubStringMatch;    // *foo*
            objName = objName.substr(1, objName.size() - 2);
        }
        else
        {
            flags = kEndStringMatch;    // *foo
            objName = objName.substr(1, objName.size() - 1);
        }
    }

    if (!flags && objName.back() == '*')
    {
        flags = kStartStringMatch;      // foo*
        objName = objName.substr(0, objName.size() - 1);
    }

    if (!flags)
    {
        flags = kExactStringMatch;
    }

    //
    // set plLocation
    //
    plLocation loc;
    if (!pageName.empty())
    {
        loc = plKeyFinder::Instance().FindLocation(NetCommGetAge()->ageDatasetName, pageName);
        flags |= kPageMatch;
    }

    fDebugObjects.push_back(new DebugObject(std::move(objName), loc, flags));

    return true;
}

bool plNetObjectDebugger::RemoveDebugObject(const ST::string& objName, const ST::string& pageName)
{
    bool didIt=false;
    if (pageName.empty())
    {
        DebugObjectList::iterator it =fDebugObjects.begin();
        for( ; it != fDebugObjects.end(); )
        {
            if ( (*it) && (*it)->ObjectMatches(objName, pageName))
            {
                delete *it;
                it = fDebugObjects.erase(it);
                didIt=true;
            }
            else
                it++;
        }
    }

    return didIt;
}

void plNetObjectDebugger::ClearAllDebugObjects()
{
    DebugObjectList::iterator it =fDebugObjects.begin();
    for( ; it != fDebugObjects.end(); it++)
    {
        delete *it;
    }
    fDebugObjects.clear();
}

//
// write to status log if there's a string match
//
void plNetObjectDebugger::LogMsgIfMatch(const ST::string& msg) const
{
    if (GetNumDebugObjects() == 0 || msg.empty())
        return;

    // extract object name from msg, expects '...object:foo,...'
    const auto objTag = ST_LITERAL("object");
    ST_ssize_t pos = msg.find(objTag, ST::case_insensitive);
    if (pos > 0)
    {
        const char* c = &msg[pos] + objTag.size();

        // move past spaces
        while ( *c || *c==' ' )
            c++;

        // copy objName token
        ST::string_stream buildName;
        while (*c && *c != ',' && *c != ' ')
            buildName.append_char(*c++);
        ST::string objName = buildName.to_string(true, ST::substitute_invalid);

        DebugObjectList::const_iterator it = fDebugObjects.begin();
        for(; it != fDebugObjects.end(); it++)
        {
            if ((*it) && (*it)->StringMatches(objName))
            {
                LogMsg(msg);
                break;
            }
        }
    }
}

void plNetObjectDebugger::LogMsg(const ST::string& msg) const
{
    fStatusLog->AddLine(msg);
}

bool plNetObjectDebugger::IsDebugObject(const hsKeyedObject* obj) const
{
    DebugObjectList::const_iterator it =fDebugObjects.begin();
    for( ; it != fDebugObjects.end(); it++)
        if ((*it) && (*it)->ObjectMatches(obj))
        {
            return true;
        }

    return false;
}
