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
#include "plProfileManagerFull.h"
#include "plCalculatedProfiles.h"
#include "plProfileManager.h"

#include "hsStream.h"

#include <string_theory/format>

#include "plPipeline/plDebugText.h"
#include "plPipeline/plPlates.h"
#include "plUnifiedTime/plUnifiedTime.h"

plProfileManagerFull::plProfileManagerFull() :
    fVars(plProfileManager::Instance().fVars),
    fLogStats(),
    fShowLaps(),
    fMinLap(),
    fDetailGraph()
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
        const char* shareGroupName = nullptr;
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
    ST::string curGroup;
    if (fShowGroups.begin() != fShowGroups.end())
        curGroup = *(fShowGroups.begin());

    GroupSet groups;
    GetGroups(groups);

    ST::string nextGroup;
    if (!curGroup.empty())
    {
        CreateStandardGraphs(curGroup.c_str(), false);

        GroupSet::iterator it = groups.find(curGroup);
        it++;
        if (it != groups.end())
        {
            nextGroup = *it;
        }
        ISetActive(curGroup.c_str(), false);
    }
    else
    {
        nextGroup = *(groups.begin());
    }

    fShowGroups.clear();
    if (!nextGroup.empty())
    {
        ISetActive(nextGroup.c_str(), true);
        CreateStandardGraphs(nextGroup.c_str(), true);
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

    return nullptr;
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

static void PrintColumn(ProfileGroup& group, const ST::string& groupName, int column, int x, int y, int& width, int& height, int off =0)
{
    plDebugText& txt = plDebugText::Instance();
    int yInc = txt.GetFontHeight() + 2;

    height = 0;
    width = 0;

    width = std::max(width, static_cast<int>(txt.CalcStringWidth(groupName) + 1));
    txt.DrawString(x, y+height, groupName, 255, 255, 255, 255, plDebugText::kStyleBold);
    height += yInc;

    uint32_t samplesWidth = txt.CalcStringWidth(ST_LITERAL("[000]"));

    for (int i = 0; i < group.size(); i++)
    {
        char buf[1024];
        ST::string str;

        switch (column)
        {
        case kColName:
            str = group[i]->GetName();

            // Since we don't draw the samples text for stats that only have 1 sample, 
            // if the stat with the longest name is fluctuating between 1 and more than
            // 1 sample the width of the column will jump around.  So we calculate the
            // width based on the stat name plus the width of the widest sample we should
            // get
            width = std::max(width, static_cast<int>(txt.CalcStringWidth(str) + samplesWidth + 1));

            // Now add on the samples text, if we have any
            if (group[i]->GetTimerSamples())
            {
                str = ST::format("{}[{}]", str, group[i]->GetTimerSamples());
            }
            break;
        case kColValue:
            group[i]->PrintValue(buf);
            str = buf;
            break;
        case kColAvg:
            group[i]->PrintAvg(buf);
            str = buf;
            break;
        case kColMax:
            group[i]->PrintMax(buf);
            str = buf;
            break;
        case kColIndex:
            str = ST::format("[{3d}]", i + off);
            break;
        }

        txt.DrawString(x, y+height, str);
        if (column != kColName)
            width = std::max(width, static_cast<int>(txt.CalcStringWidth(str) + 1));
        height += yInc;
    }

    // So the columns don't jump around as much as values change, pad them out to a certain width
    width = std::max(width, static_cast<int>(txt.CalcStringWidth(ST_LITERAL("000.0 ms")) + 1));
}

static void PrintGroup(ProfileGroup& group, const ST::string& groupName, int& x, int& y)
{
    int width, height;

    PrintColumn(group, groupName, kColName, x, y, width, height);
    x += width + 10;

    PrintColumn(group, ST_LITERAL("Avg"), kColAvg, x, y, width, height);
    x += width + 10;

    PrintColumn(group, ST_LITERAL("Cur"), kColValue, x, y, width, height);
    x += width + 10;

    PrintColumn(group, ST_LITERAL("Max"), kColMax, x, y, width, height);
    x += width + 10;

    y += height;
}


static void PrintLapGroup(ProfileGroup& group, const ST::string& groupName, int& x, int& y, int min)
{
    int width, height;

    if(min > 0)
    {
        PrintColumn(group, ST_LITERAL("Index"), kColIndex, x, y, width, height, min);
        x += width + 10;
    }

    PrintColumn(group, ST_LITERAL("Avg"), kColAvg, x, y, width, height);
    x += width + 10;
    
    PrintColumn(group, groupName, kColName, x, y, width, height);
    x += width + 10;

    PrintColumn(group, ST_LITERAL("Cur"), kColValue, x, y, width, height);
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
        ST::string groupName = *it;

        std::vector<plProfileBase*> group;

        for (int i = 0; i < fVars.size(); i++)
            if (groupName.compare(fVars[i]->GetGroup()) == 0)
                group.push_back(fVars[i]);

        int x = 10;
        PrintGroup(group, groupName, x, y);

        maxX = std::max(maxX, x);
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
        PrintLapGroup(group, ST::format("{} - {}", fShowLaps->GetGroup(), fShowLaps->GetName()), maxX, y, fMinLap);
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
            graph->AddData((int32_t)var->GetValue());
            graph->SetVisible(true);

            yPos += size;
        }
    }

    UpdateStandardGraphs(xPos, yPos);

    float detailSize = 0.9f;
    float detailX = 1 - detailSize / 2;
    float detailY = 1 - detailSize / 2;
    if (fDetailGraph)
    {
        fDetailGraph->SetPosition(detailX,detailY);
        double value;
        double scale;
        int i;
        std::vector<int32_t> values;
        for (i=0; i<fDetailVars.size(); i++)
        {
            value = (double)fDetailVars[i].var->GetValue();
            scale = 100.0/((double)(fDetailVars[i].max-fDetailVars[i].min));
            value = scale*value-fDetailVars[i].min;
            values.push_back((int32_t)value);
        }
        fDetailGraph->AddData(std::move(values));
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
        if (strcmp(var->GetGroup(), groupName) == 0)
        {
            if (printTitle)
                sprintf(buf, "%s:%s", var->GetGroup(), var->GetName());
            else
                var->PrintAvg(buf, false);

            s->Write(strlen(buf), buf);
            s->WriteByte((uint8_t)',');
        }
    }
}

void plProfileManagerFull::LogStats(const ST::string& ageName, const ST::string& spawnName)
{
    fLogStats = true;
    fLogAgeName = ageName;
    fLogSpawnName = spawnName;
}

plFileName plProfileManagerFull::GetProfilePath()
{
    static plFileName profilePath;

    if (!profilePath.IsValid())
    {
        plUnifiedTime curTime = plUnifiedTime::GetCurrent(plUnifiedTime::kLocal);

        profilePath = plFileName::Join(plFileSystem::GetUserDataPath(), "Profile",
            ST::format("{02}-{02}-{04}_{02}-{02}",
                       curTime.GetMonth(), curTime.GetDay(),
                       curTime.GetYear(), curTime.GetHour(),
                       curTime.GetMinute()));

        plFileSystem::CreateDir(profilePath, true);
    }

    return profilePath;
}

void plProfileManagerFull::ILogStats()
{
    plFileName statFilename = plFileName::Join(GetProfilePath(), fLogAgeName + ".csv");

    bool exists = plFileInfo(statFilename).Exists();

    hsUNIXStream s;
    if (s.Open(statFilename, "ab"))
    {
        GroupSet groups;
        GetGroups(groups);

        GroupSet::iterator it;

        if (!exists)
        {
            static const char kSpawn[] = "Spawn";
            s.Write(strlen(kSpawn), kSpawn);
            s.WriteByte((uint8_t)',');

            for (it = groups.begin(); it != groups.end(); it++)
            {
                ST::string groupName = *it;
                IPrintGroup(&s, groupName.c_str(), true);
            }
            s.WriteByte((uint8_t)'\r');
            s.WriteByte((uint8_t)'\n');
        }

        s.Write(fLogSpawnName.size(), fLogSpawnName.c_str());
        s.WriteByte((uint8_t)',');

        for (it = groups.begin(); it != groups.end(); it++)
        {
            ST::string groupName = *it;
            IPrintGroup(&s, groupName.c_str());
        }
        s.WriteByte((uint8_t)'\r');
        s.WriteByte((uint8_t)'\n');
    }

    fLogStats = false;
    fLogAgeName = "";
    fLogSpawnName = "";
}


void plProfileManagerFull::ShowLaps(const char* groupName, const char* varName)
{
    plProfileVar* var = nullptr;


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
    
            fShowLaps = nullptr;
        }
        else 
        {
            fShowLaps = var;
        }
    }
    if(fShowLaps)
        fShowLaps->SetLapsActive(true);
}

void plProfileManagerFull::CreateGraph(const char* varName, uint32_t min, uint32_t max)
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
        plGraphPlate* graph = nullptr;
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
    fDetailGraph->SetSize(0.9f, 0.9f);
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
        fDetailGraph = nullptr;
    }
}

void plProfileManagerFull::AddDetailVar(const char* varName, uint32_t min, uint32_t max)
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
        std::vector<uint32_t> colors;
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
