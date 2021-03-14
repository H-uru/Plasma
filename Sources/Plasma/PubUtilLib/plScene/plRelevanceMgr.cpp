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

#include "plRelevanceMgr.h"
#include "plRelevanceRegion.h"

#include "hsStream.h"
#include "hsStringTokenizer.h"

#include <tuple>

#include "pnMessage/plRefMsg.h"

#include "plIntersect/plRegionBase.h"

plRelevanceMgr* plRelevanceMgr::fInstance = nullptr;

plRelevanceMgr::plRelevanceMgr() : fEnabled(true)
{
}

void plRelevanceMgr::Init()
{
    fInstance = new plRelevanceMgr;
    fInstance->RegisterAs(kRelevanceMgr_KEY);
}

void plRelevanceMgr::DeInit()
{
    if (fInstance)
    {
        fInstance->UnRegisterAs(kRelevanceMgr_KEY);
        fInstance = nullptr;
    }
}

void plRelevanceMgr::IAddRegion(plRelevanceRegion *region)
{
    size_t dstIdx = fRegions.size();
    for (size_t i = 0; i < fRegions.size(); i++)
    {
        if (fRegions[i] == nullptr)
        {
            dstIdx = i;
            break;
        }
    }
    
    if (dstIdx == fRegions.size())
        fRegions.emplace_back(region);
    else
        fRegions[dstIdx] = region;

    region->SetMgrIndex(dstIdx + 1);
}

void plRelevanceMgr::IRemoveRegion(plRelevanceRegion *region)
{
    fRegions[region->fMgrIdx - 1] = nullptr;
}

void plRelevanceMgr::SetRegionVectors(const hsPoint3 &pos, hsBitVector &regionsImIn, hsBitVector &regionsICareAbout)
{
    regionsImIn.Clear();
    regionsICareAbout.Clear();
    regionsICareAbout.SetBit(0, true); // Always care about region zero, the special "No region" node

    bool inAnyRegion = false;

    for (size_t i = 0; i < fRegions.size(); i++)
    { 
        if (fRegions[i] && fRegions[i]->fRegion->IsInside(pos))
        {
            regionsImIn.SetBit(i + 1, true);
            regionsICareAbout |= fRegions[i]->fRegionsICareAbout;
            inAnyRegion = true;
        }
    }

    // If I'm not in any region, that means I'm in the special zero region and care about everything.
    if (!inAnyRegion)
    {
        regionsImIn.SetBit(0, true);
        regionsICareAbout.Set(fRegions.size());
    }
}
    
uint32_t plRelevanceMgr::GetNumRegions() const
{
    uint32_t count = (uint32_t)fRegions.size();
    while (count > 0 && fRegions[count - 1] == nullptr)
        count--;

    return count + 1; // Add 1 for the special zero-region
}

bool plRelevanceMgr::MsgReceive(plMessage* msg)
{
    plGenRefMsg *genMsg = plGenRefMsg::ConvertNoRef(msg);
    if (genMsg)
    {
        plRelevanceRegion *region = plRelevanceRegion::ConvertNoRef(genMsg->GetRef());
        if( genMsg->GetContext() & (plRefMsg::kOnCreate) )
        {
            IAddRegion(region);
        }
        else if( genMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            IRemoveRegion(region);
        }
        return true;
    }       
    
    return hsKeyedObject::MsgReceive(msg);
}

uint32_t plRelevanceMgr::GetIndex(const ST::string &regionName)
{
    for (size_t i = 0; i < fRegions.size(); i++)
    {
        if (fRegions[i] && regionName.compare_i(fRegions[i]->GetKeyName()) == 0)
            return uint32_t(i + 1);
    }

    return uint32_t(-1);
}

void plRelevanceMgr::MarkRegion(uint32_t localIdx, uint32_t remoteIdx, bool doICare)
{
    if (localIdx == (uint32_t)-1 || remoteIdx == (uint32_t)-1)
        return;

    if (localIdx - 1 >= fRegions.size() || remoteIdx - 1 >= fRegions.size() || fRegions[localIdx - 1] == nullptr)
        return;

    fRegions[localIdx - 1]->fRegionsICareAbout.SetBit(remoteIdx, doICare);
}

/*
*   This function expects a CSV file representing the matrix
*   
*               name1       name2       name3
*       name1   value       value       value
*       name2   value       value       value
*       name3   value       value       value
*
*   where the value determines how much the that row's region cares about the region in the current column.
*   (Currently, the possible values are:
*       0: Doesn't care
*       1 or greater: row cares about column
*/
void plRelevanceMgr::ParseCsvInput(hsStream *s)
{
    const int kBufSize = 512;
    char buff[kBufSize];
    std::vector<uint32_t> regions;
    hsStringTokenizer toke; 
    bool firstLine = true;
    
    while (!s->AtEnd())
    {
        if (!s->ReadLn(buff, kBufSize))
            break;
        
        if (firstLine)
        {
            firstLine = false;
            toke.Reset(buff, ",");
            
            while (toke.Next(buff, kBufSize))
            {
                ST::string name = ST::string::from_utf8(buff);
                if (name.empty())
                    continue; // ignore the initial blank one
                regions.emplace_back(GetIndex(name));
            }
        }
        else // parsing actual settings.
        {
            toke.Reset(buff, ",");
            if (!toke.Next(buff, kBufSize))
                continue;
            
            uint32_t rowIndex = GetIndex(buff);
            size_t column = 0;
            while (toke.Next(buff, kBufSize) && column < regions.size())
            {
                int value = atoi(buff);
                MarkRegion(rowIndex, regions[column], value != 0);

                column++;
            }
        }
    }
}

ST::string plRelevanceMgr::GetRegionNames(hsBitVector regions)
{
    ST::string retVal;
    if (regions.IsBitSet(0))
        retVal = ST_LITERAL("-Nowhere (0)-");

    for (size_t i = 0; i < fRegions.size(); ++i)
    {
        if (regions.IsBitSet(i + 1))
        {
            if (!retVal.empty())
                retVal += ", ";
            if (fRegions[i])
                retVal += fRegions[i]->GetKeyName();
        }
    }

    if (retVal.empty())
        retVal = ST_LITERAL("<NONE>");
    return retVal;
}
