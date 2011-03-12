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
#include "plProfileManagerFull.h"
#include "plProfileManager.h"

#include "../plPipeline/plDebugText.h"
#include "../plPipeline/plPlates.h"

#include "plCalculatedProfiles.h"

#include "hsStream.h"
#include "../pnUtils/pnUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plFile/plFileUtils.h"

plProfileManagerFull::plProfileManagerFull() :
	fVars(plProfileManager::Instance().fVars),
	fLogStats(false),
	fShowLaps(nil),
	fMinLap(0),
	fDetailGraph(nil)
{
}

plProfileManagerFull& plProfileManagerFull::Instance()
{
	static plProfileManagerFull theInstance;
	return theInstance;
}

void plProfileManagerFull::GetGroups(GroupSet& groups)
{
	groups.clear();
	for (int i = 0; i < fVars.size(); i++)
		groups.insert(fVars[i]->GetGroup());
}

void plProfileManagerFull::ShowGroup(const char* groupName)
{
	if (!groupName)
		groupName = "General";

	// If we're already showing this group, stop
	if (fShowGroups.find(groupName) != fShowGroups.end())
	{
		CreateStandardGraphs(groupName, false);
		fShowGroups.erase(groupName);
		ISetActive(groupName, false);
	}
	else
	{
		const char* shareGroupName = nil;
		for (int i = 0; i < fVars.size(); i++)
		{
			if (stricmp(fVars[i]->GetGroup(), groupName) == 0)
			{
				shareGroupName = fVars[i]->GetGroup();
			}
		}

		// We do have a group with this name, so insert one of the variable's
		// pointers to the name into our list (we can hang on to those pointers)
		if (shareGroupName)
		{
			ISetActive(shareGroupName, true);
			CreateStandardGraphs(shareGroupName, true);
			fShowGroups.insert(shareGroupName);
		}
	}
}

void plProfileManagerFull::ShowNextGroup()
{
	const char* curGroup = nil;
	if (fShowGroups.begin() != fShowGroups.end())
		curGroup = *(fShowGroups.begin());

	GroupSet groups;
	GetGroups(groups);

	const char* nextGroup = nil;
	if (curGroup)
	{
		CreateStandardGraphs(curGroup, false);

		GroupSet::iterator it = groups.find(curGroup);
		it++;
		if (it != groups.end())
		{
			nextGroup = *it;
		}
		ISetActive(curGroup,false);
	}
	else
	{
		nextGroup = *(groups.begin());
	}

	fShowGroups.clear();
	if (nextGroup)
	{
		ISetActive(nextGroup, true);
		CreateStandardGraphs(nextGroup, true);
		fShowGroups.insert(nextGroup);
	}
}

plProfileVar* plProfileManagerFull::IFindTimer(const char *name)
{
	for (int i = 0; i < fVars.size(); i++)
	{
		if (stricmp(fVars[i]->GetName(), name) == 0)
			return fVars[i];
	}

	return nil;
}

void plProfileManagerFull::GetLaps(LapNames& lapNames)
{
	for (int i = 0; i < fVars.size(); i++)
	{
		plProfileVar* var = fVars[i];
		if (var->GetLaps())
		{
			LapPair lapPair;
			lapPair.group = var->GetGroup();
			lapPair.varName = var->GetName();
			lapNames.push_back(lapPair);
		}
	}
}


enum
{
	kColName,
	kColValue,
	kColAvg,
	kColMax,
	kColIndex,
};

typedef std::vector<plProfileBase*> ProfileGroup;

static void PrintColumn(ProfileGroup& group, const char* groupName, int column, int x, int y, int& width, int& height, int off =0)
{
	plDebugText& txt = plDebugText::Instance();
	int yInc = txt.GetFontHeight() + 2;

	height = 0;
	width = 0;

	width = hsMaximum(width, txt.CalcStringWidth(groupName) + 1);
	txt.DrawString(x, y+height, groupName, 255, 255, 255, 255, plDebugText::kStyleBold);
	height += yInc;

	UInt32 samplesWidth = txt.CalcStringWidth("[000]");

	for (int i = 0; i < group.size(); i++)
	{
		char str[1024];

		switch (column)
		{
		case kColName:
			strcpy(str, group[i]->GetName());

			// Since we don't draw the samples text for stats that only have 1 sample, 
			// if the stat with the longest name is fluctuating between 1 and more than
			// 1 sample the width of the column will jump around.  So we calculate the
			// width based on the stat name plus the width of the widest sample we should
			// get
			width = hsMaximum(width, txt.CalcStringWidth(str) + samplesWidth + 1);

			// Now add on the samples text, if we have any
			if (group[i]->GetTimerSamples())
			{
				char cnt[20];
				sprintf(cnt, "[%d]", group[i]->GetTimerSamples());
				strcat(str, cnt);
			}
			break;
		case kColValue:
			group[i]->PrintValue(str);
			break;
		case kColAvg:
			group[i]->PrintAvg(str);
			break;
		case kColMax:
			group[i]->PrintMax(str);
			break;
		case kColIndex:
			sprintf(str,"[%3d]",i+off);
			break;
		}

		txt.DrawString(x, y+height, str);
		if (column != kColName)
			width = hsMaximum(width, txt.CalcStringWidth(str) + 1);
		height += yInc;
	}

	// So the columns don't jump around as much as values change, pad them out to a certain width
	width = hsMaximum(width, txt.CalcStringWidth("000.0 ms") + 1);
}

static void PrintGroup(ProfileGroup& group, const char* groupName, int& x, int& y)
{
	int width, height;

	PrintColumn(group, groupName, kColName, x, y, width, height);
	x += width + 10;

	PrintColumn(group, "Avg", kColAvg, x, y, width, height);
	x += width + 10;

	PrintColumn(group, "Cur", kColValue, x, y, width, height);
	x += width + 10;

	PrintColumn(group, "Max", kColMax, x, y, width, height);
	x += width + 10;

	y += height;
}


static void PrintLapGroup(ProfileGroup& group, const char* groupName, int& x, int& y, int min)
{
	int width, height;

	if(min > 0)
	{
		PrintColumn(group, "Index", kColIndex, x, y, width, height, min);
		x += width + 10;
	}

	PrintColumn(group, "Avg", kColAvg, x, y, width, height);
	x += width + 10;
	
	PrintColumn(group, groupName, kColName, x, y, width, height);
	x += width + 10;

	PrintColumn(group, "Cur", kColValue, x, y, width, height);
	x += width + 10;

	y += height;
}

void plProfileManagerFull::EndFrame()
{
	CalculateProfiles();
}

void plProfileManagerFull::Update()
{
	if (fLogStats)
		ILogStats();

	//
	// Print the groups we're showing
	//
	int maxX = 0;

	int y = 10;
	GroupSet::iterator it;
	for (it = fShowGroups.begin(); it != fShowGroups.end(); it++)
	{
		const char* groupName = *it;

		std::vector<plProfileBase*> group;

		for (int i = 0; i < fVars.size(); i++)
			if (hsStrEQ(fVars[i]->GetGroup(), groupName))
				group.push_back(fVars[i]);

		int x = 10;
		PrintGroup(group, groupName, x, y);

		maxX = hsMaximum(maxX, x);
		y += 10;
	}

	//
	// Print the laps we're showing
	//
	if (fShowLaps && fShowLaps->GetLaps())
	{
		plProfileLaps* laps = fShowLaps->GetLaps();

		std::vector<plProfileBase*> group;
		int numLaps = laps->GetNumLaps();
		
		if(numLaps < fMinLap)
			fMinLap = 0;
		for (int i = 0; i < numLaps; i++)
		{
			if(i >= fMinLap && i < (fMinLap + 40))
				group.push_back(laps->GetLap(i));
		}
		y = 10;
		char buf[256];
		sprintf(buf, "%s - %s", fShowLaps->GetGroup(), fShowLaps->GetName());
		PrintLapGroup(group, buf, maxX, y, fMinLap);
	}

	//
	// Update the graphs
	//
	float size = 0.25;
	float xPos = 1 - size / 2;
	float yPos = -1 + size / 2;

	for (int i = 0; i < fGraphs.size(); i++)
	{
		plGraphPlate* graph = fGraphs[i];
		plProfileVar* var = IFindTimer(graph->GetTitle());

		if (var)
		{
			graph->SetPosition(xPos, yPos);
			graph->AddData(var->GetValue());
			graph->SetVisible(true);

			yPos += size;
		}
	}

	UpdateStandardGraphs(xPos, yPos);

	float detailSize = 0.9;
	float detailX = 1 - detailSize / 2;
	float detailY = 1 - detailSize / 2;
	if (fDetailGraph)
	{
		fDetailGraph->SetPosition(detailX,detailY);
		double value;
		double scale;
		int i;
		std::vector<Int32> values;
		for (i=0; i<fDetailVars.size(); i++)
		{
			value = (double)fDetailVars[i].var->GetValue();
			scale = 100.0/((double)(fDetailVars[i].max-fDetailVars[i].min));
			value = scale*value-fDetailVars[i].min;
			values.push_back((Int32)value);
		}
		fDetailGraph->AddData(values);
		fDetailGraph->SetVisible(true);
	}
}

void plProfileManagerFull::ActivateAllStats()
{
	for (int i = 0; i < fVars.size(); i++)
	{
		fVars[i]->SetActive(true);
		fVars[i]->Start();
	}
}

void plProfileManagerFull::IPrintGroup(hsStream* s, const char* groupName, bool printTitle)
{
	char buf[256];

	for (int i = 0; i < fVars.size(); i++)
	{
		plProfileVar* var = fVars[i];
		if (hsStrEQ(var->GetGroup(), groupName))
		{
			if (printTitle)
				sprintf(buf, "%s:%s", var->GetGroup(), var->GetName());
			else
				var->PrintAvg(buf, false);

			s->Write(strlen(buf), buf);
			s->WriteByte(',');
		}
	}
}

void plProfileManagerFull::LogStats(const char* ageName, const char* spawnName)
{
	fLogStats = true;
	wchar* temp = hsStringToWString(ageName);
	fLogAgeName = temp;
	delete [] temp;
	fLogSpawnName = spawnName;
}

const wchar* plProfileManagerFull::GetProfilePath()
{
	static wchar profilePath[MAX_PATH];
	static bool initialized = false;

	if (!initialized)
	{
		initialized = true;

		plUnifiedTime curTime = plUnifiedTime::GetCurrentTime(plUnifiedTime::kLocal);

		PathGetUserDirectory(profilePath, arrsize(profilePath));
		PathAddFilename(profilePath, profilePath, L"Profile", arrsize(profilePath));
		plFileUtils::CreateDir(profilePath);
	
		wchar buff[256];
		swprintf(buff, L"%02d-%02d-%04d_%02d-%02d//",
			curTime.GetMonth(),
			curTime.GetDay(),
			curTime.GetYear(),
			curTime.GetHour(),
			curTime.GetMinute());

		PathAddFilename(profilePath, profilePath, buff, arrsize(profilePath));
		plFileUtils::CreateDir(profilePath);
	}

	return profilePath;
}

void plProfileManagerFull::ILogStats()
{
	wchar statFilename[256];
	swprintf(statFilename, L"%s%s.csv", GetProfilePath(), fLogAgeName.c_str());

	bool exists = plFileUtils::FileExists(statFilename);

	hsUNIXStream s;
	if (s.Open(statFilename, L"ab"))
	{
		GroupSet groups;
		GetGroups(groups);

		GroupSet::iterator it;

		if (!exists)
		{
			const char* kSpawn = "Spawn";
			s.Write(strlen(kSpawn), kSpawn);
			s.WriteByte(',');

			for (it = groups.begin(); it != groups.end(); it++)
			{
				const char* groupName = *it;
				IPrintGroup(&s, groupName, true);
			}
			s.WriteByte('\r');
			s.WriteByte('\n');
		}

		s.Write(fLogSpawnName.length(), fLogSpawnName.c_str());
		s.WriteByte(',');

		for (it = groups.begin(); it != groups.end(); it++)
		{
			const char* groupName = *it;
			IPrintGroup(&s, groupName);
		}
		s.WriteByte('\r');
		s.WriteByte('\n');

		s.Close();
	}

	fLogStats = false;
	fLogAgeName = L"";
	fLogSpawnName = "";
}


void plProfileManagerFull::ShowLaps(const char* groupName, const char* varName)
{
	plProfileVar* var = nil;


	if(fShowLaps)
		fShowLaps->SetLapsActive(false);

	for (int i = 0; i < fVars.size(); i++)
	{
		int j = 0;
		while(fVars[i]->GetName()[j++] == ' ') {}
		if (stricmp(&(fVars[i]->GetName()[j-1]), varName) == 0 &&
			stricmp(fVars[i]->GetGroup(), groupName) == 0)
		{
			var = fVars[i];
			break;
		}
	}

	if (var)
	{
		if (var == fShowLaps)
		{
	
			fShowLaps = nil;
		}
		else 
		{
			fShowLaps = var;
		}
	}
	if(fShowLaps)
		fShowLaps->SetLapsActive(true);
}

void plProfileManagerFull::CreateGraph(const char* varName, UInt32 min, UInt32 max)
{
	// If the graph is already created, destroy it
	for (int i = 0; i < fGraphs.size(); i++)
	{
		if (strcmp(fGraphs[i]->GetTitle(), varName) == 0)
		{
			plPlateManager::Instance().DestroyPlate(fGraphs[i]);
			fGraphs.erase(fGraphs.begin()+i);
			return;
		}
	}

	plProfileVar* var = IFindTimer(varName);
	if (var)
	{
		plGraphPlate* graph = nil;
		plPlateManager::Instance().CreateGraphPlate(&graph);
		graph->SetSize(0.25, 0.25);
		graph->SetDataRange(min, max, 100);
		graph->SetTitle(var->GetName());

		fGraphs.push_back(graph);
	}
}

void plProfileManagerFull::ResetDefaultDetailVars()
{
	fDetailVars.clear();
	AddDetailVar("ApplyAnimation",0,50);
	AddDetailVar("AnimatingPhysicals",0,50);
	AddDetailVar("StoppedAnimPhysicals",0,50);
	AddDetailVar("DrawableTime",0,50);
	AddDetailVar("Polys",0,150000);
	AddDetailVar("Step",0,50);
	AddDetailVar("LineOfSight",0,50);
	AddDetailVar("  PhysicsUpdates",0,50);
	AddDetailVar("Stream Shove Time",0,50);
	AddDetailVar("RenderSetup",0,50);
}

void plProfileManagerFull::ShowDetailGraph()
{
	// if graph is already created, kill it
	if (fDetailGraph)
		HideDetailGraph();
	if (fDetailVars.size() == 0)
		ResetDefaultDetailVars();
	
	plPlateManager::Instance().CreateGraphPlate(&fDetailGraph);
	fDetailGraph->SetSize(0.9,0.9);
	fDetailGraph->SetDataRange(0,500,500);
	fDetailGraph->SetDataLabels(0,100); // should be relatively simple to cast everything to a 0-100 range
	fDetailGraph->SetTitle("Detail");
	UpdateDetailLabels();
}

void plProfileManagerFull::HideDetailGraph()
{
	if (fDetailGraph)
	{
		plPlateManager::Instance().DestroyPlate(fDetailGraph);
		fDetailGraph = nil;
	}
}

void plProfileManagerFull::AddDetailVar(const char* varName, UInt32 min, UInt32 max)
{
	int i=0;
	for (i=0; i<fDetailVars.size(); i++)
	{
		if (stricmp(fDetailVars[i].var->GetName(), varName) == 0)
			return; // don't add it again
	}

	plProfileVar* var = IFindTimer(varName);
	if (!var)
		return;
	var->SetActive(true);

	if (fDetailVars.size() == 10)
		fDetailVars.erase(fDetailVars.begin()); // we don't want any more then 10 at this point, so drop the oldest one
	detailVar temp;
	temp.var = var;
	temp.min = min;
	temp.max = max;
	fDetailVars.push_back(temp);
	UpdateDetailLabels();
}

void plProfileManagerFull::RemoveDetailVar(const char* varName)
{
	int i=0;
	for (i=0; i<fDetailVars.size(); i++)
	{
		if (stricmp(fDetailVars[i].var->GetName(), varName) == 0)
		{
			fDetailVars.erase(fDetailVars.begin()+i);
		}
	}
	UpdateDetailLabels();
}

void plProfileManagerFull::UpdateDetailLabels()
{
	if (fDetailGraph)
	{
		int i;
		std::vector<std::string> labels;
		for (i=0; i<fDetailVars.size(); i++)
			labels.push_back(fDetailVars[i].var->GetName());

		fDetailGraph->SetLabelText(labels);

		// update the colors as well, just in case
		std::vector<UInt32> colors;
		colors.push_back(0xff00ff00); // green
		colors.push_back(0xff0000ff); // blue
		colors.push_back(0xffffff00); // yellow
		colors.push_back(0xffff00ff); // pink
		colors.push_back(0xffffffff); // white
		colors.push_back(0xff00ffff); // cyan
		colors.push_back(0xffff8000); // orange
		colors.push_back(0xff8000ff); // purple
		colors.push_back(0xffff0080); // fuscha
		colors.push_back(0xff808080); // grey

		fDetailGraph->SetDataColors(colors);
	}
}

void plProfileManagerFull::ResetMax()
{
	for (int i = 0; i < fVars.size(); i++)
		fVars[i]->ResetMax();
}

void plProfileManagerFull::ISetActive(const char* groupName, bool active)
{
	for (int i = 0; i < fVars.size(); i++)
	{
		if (stricmp(fVars[i]->GetGroup(), groupName) == 0)
		{
			fVars[i]->SetActive(active);
		}
	}
}
