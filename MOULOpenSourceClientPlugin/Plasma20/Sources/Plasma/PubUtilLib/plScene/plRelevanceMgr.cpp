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
#include "plRelevanceMgr.h"
#include "plRelevanceRegion.h"
#include "../plIntersect/plRegionBase.h"
#include "hsStream.h"
#include "hsStringTokenizer.h"

plRelevanceMgr* plRelevanceMgr::fInstance = nil;

plRelevanceMgr::plRelevanceMgr() : fEnabled(true)
{
}

void plRelevanceMgr::Init()
{
	fInstance = TRACKED_NEW plRelevanceMgr;
	fInstance->RegisterAs(kRelevanceMgr_KEY);
}

void plRelevanceMgr::DeInit()
{
	if (fInstance)
	{
		fInstance->UnRegisterAs(kRelevanceMgr_KEY);
		fInstance = nil;
	}
}

void plRelevanceMgr::IAddRegion(plRelevanceRegion *region)
{
	int i;
	int dstIdx = fRegions.GetCount();
	for (i = 0; i < fRegions.GetCount(); i++)
	{
		if (fRegions[i] == nil)
		{
			dstIdx = i;
			break;
		}
	}
	
	if (dstIdx == fRegions.GetCount())
		fRegions.Append(region);
	else
		fRegions[i] = region;

	region->SetMgrIndex(dstIdx + 1);
}

void plRelevanceMgr::IRemoveRegion(plRelevanceRegion *region)
{
	fRegions[region->fMgrIdx - 1] = nil;
}

void plRelevanceMgr::SetRegionVectors(const hsPoint3 &pos, hsBitVector &regionsImIn, hsBitVector &regionsICareAbout)
{
	regionsImIn.Clear();
	regionsICareAbout.Clear();
	regionsICareAbout.SetBit(0, true); // Always care about region zero, the special "No region" node

	hsBool inAnyRegion = false;

	int i;
	for (i = 0; i < fRegions.GetCount(); i++)
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
		regionsICareAbout.Set(fRegions.GetCount());
	}
}
	
UInt32 plRelevanceMgr::GetNumRegions() const
{
	int i;

	for (i = fRegions.GetCount(); i > 0 && fRegions[i - 1] == nil; i--);

	return i + 1; // Add 1 for the special zero-region
}
		

hsBool plRelevanceMgr::MsgReceive(plMessage* msg)
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

UInt32 plRelevanceMgr::GetIndex(char *regionName)
{
	int i;
	for (i = 0; i < fRegions.GetCount(); i++)
	{
		if (fRegions[i] && !stricmp(regionName, fRegions[i]->GetKeyName()))
			return i + 1;
	}

	return -1;
}

void plRelevanceMgr::MarkRegion(UInt32 localIdx, UInt32 remoteIdx, hsBool doICare)
{
	if (localIdx == (UInt32)-1 || remoteIdx == (UInt32)-1)
		return;

	if (localIdx - 1 >= fRegions.GetCount() || remoteIdx - 1 >= fRegions.GetCount() || fRegions[localIdx - 1] == nil)
		return;

	fRegions[localIdx - 1]->fRegionsICareAbout.SetBit(remoteIdx, doICare);
}

// tiny class for the function below
class plRegionInfo
{
public:
	char *fName;
	int fIndex;
	
	plRegionInfo() : fName(nil), fIndex(-1) {}
	~plRegionInfo() { delete [] fName; }
};

/*
*	This function expects a CSV file representing the matrix
*	
*				name1		name2		name3
*		name1	value		value		value
*		name2	value		value		value
*		name3	value		value		value
*
*	where the value determines how much the that row's region cares about the region in the current column.
*	(Currently, the possible values are:
*		0: Doesn't care
*		1 or greater: row cares about column
*/
void plRelevanceMgr::ParseCsvInput(hsStream *s)
{
	const int kBufSize = 512;
	char buff[kBufSize];
	hsTArray<plRegionInfo*> regions;	
	hsStringTokenizer toke; 
	hsBool firstLine = true;
	
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
				if (strcmp(buff, "") == 0)
					continue; // ignore the initial blank one

				plRegionInfo *info = TRACKED_NEW plRegionInfo;
				regions.Append(info);
				info->fName = hsStrcpy(buff);
				info->fIndex = GetIndex(buff);
			}
		}
		else // parsing actual settings.
		{
			toke.Reset(buff, ",");
			if (!toke.Next(buff, kBufSize))
				continue;
			
			int rowIndex = GetIndex(buff);
			int column = 0;
			while (toke.Next(buff, kBufSize) && column < regions.GetCount())
			{
				int value = atoi(buff);
				MarkRegion(rowIndex, regions[column]->fIndex, value != 0);

				column++;
			}
		}
	}

	int i;
	for (i = regions.GetCount() - 1; i >= 0; i--)
		delete regions[i];
}

std::string plRelevanceMgr::GetRegionNames(hsBitVector regions)
{
	std::string retVal = "";
	if (regions.IsBitSet(0))
		retVal = "-Nowhere (0)-";

	for (int i = 0; i < fRegions.GetCount(); ++i)
	{
		if (regions.IsBitSet(i + 1))
		{
			if (retVal.length() != 0)
				retVal += ", ";
			if (fRegions[i])
				retVal += fRegions[i]->GetKeyName();
		}
	}

	if (retVal.length() == 0)
		retVal = "<NONE>";
	return retVal;
}