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

#include "plCalculatedProfiles.h"
#include "plProfile.h"
#include "plProfileManager.h"

#include "hsTimer.h"

#include "plPipeline/plPlates.h"

#ifdef HS_FIND_MEM_LEAKS
plProfile_CreateMemCounter("Allocated", "Memory", MemAllocated);
plProfile_CreateMemCounter("Peak Alloc", "Memory", MemPeakAlloc);
#endif

static plProfileVar gVarRFPS(ST_LITERAL("RFPS"), ST_LITERAL("General"), plProfileVar::kDisplayTime | plProfileVar::kDisplayFPS);

plProfile_Extern(DrawTriangles);
plProfile_Extern(MatChange);
plProfile_CreateCounter("Polys Per Material", "General", PolysPerMat);

#ifdef PL_PROFILE_ENABLED
#define plProfile_GetValue(varName) gProfileVar##varName.GetValue()
#else
#define plProfile_GetValue(varName) 0
#endif

void CalculateProfiles()
{
    // KLUDGE - do timing that overlaps the beginframe / endframe (where timing is normally reset)
    static uint64_t lastTicks = plProfileManager::GetTime();
    uint64_t curTicks = plProfileManager::GetTime();
    gVarRFPS.Set(curTicks - lastTicks);
    lastTicks = curTicks;

    // KLUDGE - calulate the polys per material
    if (plProfile_GetValue(MatChange) == 0)
        plProfile_Set(PolysPerMat, 0);
    else
        plProfile_Set(PolysPerMat, plProfile_GetValue(DrawTriangles) / plProfile_GetValue(MatChange));

    #ifdef HS_FIND_MEM_LEAKS
//  plProfile_Set(MemAllocated, MemGetAllocated());
//  plProfile_Set(MemPeakAlloc, MemGetPeakAllocated());
    #endif
}

static plGraphPlate* fFPSPlate = nullptr;

static bool ICreateStdPlate(plGraphPlate** graph)
{
    if (plPlateManager::InstanceValid())
    {
        plPlateManager::Instance().CreateGraphPlate(graph);
        (*graph)->SetSize(0.25, 0.25);
        (*graph)->SetDataRange(0, 100, 100);
        return true;
    }
    return false;
}

void CreateStandardGraphs(const ST::string& groupName, bool create)
{
    if (groupName == "General") {
        if (create)
        {
            if (ICreateStdPlate(&fFPSPlate))
            {
                fFPSPlate->SetTitle(ST_LITERAL("mSecs"));
                fFPSPlate->SetLabelText({ST_LITERAL("Tot"), ST_LITERAL("Draw"), ST_LITERAL("Upd")});
            }
        }
        else
        {
            plPlateManager::Instance().DestroyPlate(fFPSPlate);
            fFPSPlate = nullptr;
        }
    }
}

plProfile_CreateTimer("Draw", "General", DrawTime);
plProfile_CreateTimer("Update", "General", UpdateTime);

void UpdateStandardGraphs(float xPos, float yPos)
{
    #define PositionPlate(plate)        \
        plate->SetPosition(xPos, yPos); \
        yPos += 0.25;                   \
        plate->SetVisible(true);

    if (fFPSPlate)
    {
        fFPSPlate->AddData(
            (int32_t)gVarRFPS.GetValue(),
            (int32_t)plProfile_GetValue(DrawTime),
            (int32_t)plProfile_GetValue(UpdateTime));
        PositionPlate(fFPSPlate);
    }
}
