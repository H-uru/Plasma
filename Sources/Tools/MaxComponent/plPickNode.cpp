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

#include "HeadSpin.h"
#include <algorithm>

#include "plActivatorBaseComponent.h"
#include "plBehavioralComponents.h"
#include "plPhysicalComponents.h"

#include <iparamm2.h>

#include "plPickNode.h"

#include "plCameraComponents.h"
#include "plNavigableComponents.h"
#include "plPythonFileComponent.h"

class plPickNodeMax : public HitByNameDlgCallback
{
protected:
    bool fSingle;
    bool fCanConvertToType;
    IParamBlock2 *fPB;
    int fNodeListID;
    std::vector<Class_ID> fCIDs;    
    bool fRefKludge;
    plComponentBase *fComp;
    Mtl* fMtl;

    bool CanConvertToType(Object *obj);

public:
    plPickNodeMax(IParamBlock2 *pb, int nodeListID, std::vector<Class_ID>* cids, bool single, bool canConvertToType);

    GETDLGTEXT_RETURN_TYPE dialogTitle() override;
    GETDLGTEXT_RETURN_TYPE buttonText() override { return "OK"; }

    int filter(INode *node) override;

    void proc(INodeTab &nodeTab) override;
    
    BOOL showHiddenAndFrozen() override { return TRUE; }
    BOOL singleSelect() override { return fSingle; }

    void SetRefKludge(bool on) { fRefKludge = on; }

    void SetComponent(plComponentBase *comp) { fComp = comp; }
    void SetMtl(Mtl* mtl) { fMtl = mtl; }
};

bool plPickNodeMax::CanConvertToType(Object *obj)
{
    for (int i = 0; i < fCIDs.size(); i++)
    {
        if (obj->CanConvertToType(fCIDs[i]))
            return true;
    }

    return false;
}

plPickNodeMax::plPickNodeMax(IParamBlock2 *pb, int nodeListID, std::vector<Class_ID>* cids, bool single, bool canConvertToType) :
  fPB(pb), fNodeListID(nodeListID), fSingle(single), fCanConvertToType(canConvertToType), fRefKludge(), fComp(), fMtl()
{
    if (cids)
        fCIDs = *cids; 
}

GETDLGTEXT_RETURN_TYPE plPickNodeMax::dialogTitle()
{
    return fSingle ? "Select Node" : "Select Nodes";
}

int plPickNodeMax::filter(INode *node)
{
    if (node && node->GetObjectRef())
    {
        // Filtering by nodes a component is attached to
        if (fComp)
        {
            for (int i = 0; i < fComp->NumTargets(); i++)
            {
                if (fComp->GetTarget(i) == (plMaxNodeBase*)node)
                    return TRUE;
            }

            return FALSE;
        }

        // Filtering by nodes with a specific material on them
        if (fMtl)
        {
            if (node->GetMtl() == fMtl)
                return TRUE;
            return FALSE;
        }

        // Not filtering by ClassID and node is hidden or frozen
        if (fCIDs.size() == 0 && (node->IsHidden() || node->IsFrozen()))
            return FALSE;

        // We aren't filtering by ClassID or we are and we found a match
        if (fCIDs.size() == 0 ||
            (fCanConvertToType && CanConvertToType(node->GetObjectRef()) ||
            std::find(fCIDs.begin(), fCIDs.end(), node->GetObjectRef()->ClassID()) != fCIDs.end()))
        {
            if (fSingle)
            {
                if (fPB->GetReferenceTarget(fNodeListID) == (ReferenceTarget*)node)
                    return FALSE;
            }
            else
            {
                // Make sure we don't already ref this node
                for (int i = 0; i < fPB->Count(fNodeListID); i++)
                {
                    if (fPB->GetReferenceTarget(fNodeListID, 0, i) == (ReferenceTarget*)node)
                        return FALSE;
                }
            }

            // Don't allow a ref to ourselves (a cyclical reference)
            if (fPB->GetOwner() == node)
                return FALSE;

            // Approved and not in the list, add it
            return TRUE;
        }
    }

    return FALSE;
}

void plPickNodeMax::proc(INodeTab &nodeTab)
{
    if (nodeTab.Count() > 0)
    {
        if (!fRefKludge)
        {
            if (fSingle)
                fPB->SetValue(fNodeListID, 0, nodeTab[0]);
            else
                fPB->Append(fNodeListID, nodeTab.Count(), &nodeTab[0]);
        }
        // Have to be a little wacky here since some of the params that use this are
        // ReferenceTargets while some are actually INodes (like they're supposed to be).
        else
        {
            if (fSingle)
                fPB->SetValue(fNodeListID, 0, (ReferenceTarget*)nodeTab[0]);
            else
                fPB->Append(fNodeListID, nodeTab.Count(), (ReferenceTarget**)&nodeTab[0]);
        }
    }
}

#include "plPickNodeBase.h"

bool plPick::Node(IParamBlock2 *pb, int paramID, std::vector<Class_ID>* cids, bool single, bool canConvertToType)
{
    plPickNodeMax pick(pb, paramID, cids, single, canConvertToType);
    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

bool plPick::NodeRefKludge(IParamBlock2 *pb, int paramID, std::vector<Class_ID>* cids, bool single, bool canConvertToType)
{
    plPickNodeMax pick(pb, paramID, cids, single, canConvertToType);
    pick.SetRefKludge(true);
    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

bool plPick::CompTargets(IParamBlock2 *pb, int paramID, plComponentBase *comp)
{
    plPickCompNode pick(pb, paramID, comp);
    return pick.DoPick();
}

bool plPick::MtlNodes(IParamBlock2* pb, int paramID, Mtl* mtl)
{
    plPickMtlNode pick(pb, paramID, mtl);
    return pick.DoPick();
}

#include "plMultistageBehComponent.h"

bool plPick::Activator(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(ACTIVATOR_BASE_CID);
    cid.push_back(PYTHON_FILE_CID);
    cid.push_back(BEHAVIORAL_SITTING_CID);
    cid.push_back(MULTISTAGE_BEH_CID);
    cid.push_back(SUBWORLD_REGION_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

bool plPick::DetectorEnable(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(ACTIVATOR_BASE_CID);
    cid.push_back(PYTHON_FILE_CID);
    cid.push_back(BEHAVIORAL_SITTING_CID);
    cid.push_back(NAV_LADDER_CID);
    cid.push_back(NAV_LADDER_CID);
    cid.push_back(CAM_REGION_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plGUIComponents.h"

bool plPick::GUIDialog(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(GUI_DIALOG_COMP_CLASS_ID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

bool plPick::GenericClass(IParamBlock2 *pb, int paramID, bool single, Class_ID classIDToPick )
{
    std::vector<Class_ID> cid;
    cid.push_back(classIDToPick);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plExcludeRegionComponent.h"

bool plPick::ExcludeRegion(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(XREGION_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plWaterComponent.h"

bool plPick::WaterComponent(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(WATER_COMP_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plPhysicalComponents.h"

bool plPick::Swim2DComponent(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(PHYS_SWIMSURFACE_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plClusterComponent.h"

bool plPick::ClusterComponent(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(CLUSTER_COMP_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plAnimComponent.h"

bool plPick::Animation(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(ANIM_COMP_CID);
    cid.push_back(ANIM_GROUP_COMP_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plOneShotComponent.h"
#include "plMultistageBehComponent.h"

bool plPick::Behavior(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(ONESHOTCLASS_ID);
    cid.push_back(MULTISTAGE_BEH_CID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}

#include "plGrassComponent.h"

bool plPick::GrassComponent(IParamBlock2 *pb, int paramID, bool single)
{
    std::vector<Class_ID> cid;
    cid.push_back(GRASS_COMPONENT_CLASS_ID);
    plPickNodeMax pick(pb, paramID, &cid, single, true);

    return (GetCOREInterface()->DoHitByNameDialog(&pick) != 0);
}
