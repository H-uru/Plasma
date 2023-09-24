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
#ifndef plProfileManagerFull_h_inc
#define plProfileManagerFull_h_inc

#include <set>
#include <string_theory/string>

#include "plFileSystem.h"

#include "plProfileManager.h"

class plProfileManager;
class plGraphPlate;
class plStatusLog;
class hsStream;
class plProfileVar;

class plProfileManagerFull
{
public:
    typedef std::set<ST::string, ST::less_i> GroupSet;

protected:
    plProfileManager::VarVec& fVars;

    bool fLogStats; // If true, log the stats at the end of the frame
    ST::string fLogAgeName;
    ST::string fLogSpawnName;

    std::vector<plGraphPlate*> fGraphs;
    plGraphPlate* fDetailGraph;

    struct detailVar
    {
        plProfileVar* var;
        int32_t min;
        int32_t max;
    };

    std::vector<detailVar> fDetailVars; // the vars we want to show on the detail graph

    GroupSet fShowGroups;
    plProfileVar* fShowLaps;
    uint32_t fMinLap; // For Display

    void IPrintGroup(hsStream* s, const char* groupName, bool printTitle=false);
    void ILogStats();

    plProfileVar* IFindTimer(const ST::string& name);

    void ISetActive(const char* groupName, bool active);

    plProfileManagerFull();

public:
    static plProfileManagerFull& Instance();

    void EndFrame();    // Call end frame on our special timers
    void Update();

    void GetGroups(GroupSet& groups);
    void ShowGroup(const char* groupName);
    void ShowNextGroup();

    struct LapPair { const char* group; const char* varName; };
    typedef std::vector<LapPair> LapNames;
    void GetLaps(LapNames& lapNames);
    void ShowLaps(const char* groupName, const char* varName);
    void SetMinLap(int m) { fMinLap = m; };
    void PageDownLaps() { fMinLap += 40; }
    void PageUpLaps() { fMinLap = (fMinLap < 40) ? 0 : fMinLap - 40;}

    void CreateGraph(const char* varName, uint32_t min, uint32_t max);

    void ResetDefaultDetailVars();
    void ShowDetailGraph();
    void HideDetailGraph();
    void AddDetailVar(const char* varName, uint32_t min, uint32_t max);
    void RemoveDetailVar(const char* varName);
    void UpdateDetailLabels();

    void ResetMax();

    void LogStats(const ST::string& ageName, const ST::string& spawnName);
    plFileName GetProfilePath();

    // If you're going to call LogStats, make sure to call this first so all stats will be evaluated before logging
    void ActivateAllStats();

};

#endif // plProfileManagerFull_h_inc
