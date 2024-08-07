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

#include <vector>

#include "HeadSpin.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "MaxMain/plPlasmaRefMsgs.h"
#include "plPickNode.h"

#include "MaxExport/plExportProgressBar.h"

#include "pnSceneObject/plSceneObject.h"
#include "plDrawable/plMorphSequence.h"
#include "plDrawable/plSharedMesh.h"
#include "pnMessage/plRefMsg.h"

const Class_ID MORPHSEQ_COMP_CID(0x37100f0a, 0x2d1f6b87);
const Class_ID MORPHLAY_COMP_CID(0x138b1d44, 0x6c0a7417);

void DummyCodeIncludeFuncMorph()
{
}

class plMorphLayComp : public plComponent
{
protected:
public:
    enum 
    {
        kDeltas
    };
    
    plMorphLayComp();
    void DeleteThis() override { delete this; }
    
    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    bool SetupLayer(plMorphArray& morphArr, plMaxNode* baseNode, std::vector<plGeometrySpan*>* baseSpans, plErrorMsg* pErrMsg);
};

CLASS_DESC(plMorphLayComp, gMorphLayCompDesc, "Morph Layer",  "MorphLay", COMP_TYPE_AVATAR, MORPHLAY_COMP_CID)

ParamBlockDesc2 gMorphLayBk
(   
    plComponent::kBlkComp, _T("MorphLay"), 0, &gMorphLayCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_MORPHLAY, IDS_COMP_MORPHLAY, 0, 0, nullptr,

    plMorphLayComp::kDeltas,    _T("Deltas"),   TYPE_INODE_TAB, 0,      P_CAN_CONVERT, 0,
        p_ui,           TYPE_NODELISTBOX, IDC_LIST_TARGS, IDC_ADD_TARGS, 0, IDC_DEL_TARGS,
        p_classID,      triObjectClassID,
        p_end,

    p_end
);


plMorphLayComp::plMorphLayComp()
{
    fClassDesc = &gMorphLayCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plMorphLayComp::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    const int num = fCompPB->Count(kDeltas);
    int i;
    for( i = 0; i < num; i++ )
    {
        plMaxNode* deltaNode = (plMaxNode*)fCompPB->GetINode(kDeltas, TimeValue(0), i);
        if( deltaNode )
        {
            const MCHAR* deltaName = deltaNode->GetName();
            
            if( !deltaNode->GetSwappableGeom() )
                deltaNode->SetSwappableGeom(new plSharedMesh);
//          deltaNode->SetForceLocal(true);
        }
    }

    return true;
}

bool plMorphLayComp::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plMorphLayComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plMorphLayComp::SetupLayer(plMorphArray& morphArr, plMaxNode* baseNode, std::vector<plGeometrySpan*>* baseSpans, plErrorMsg* pErrMsg)
{
    const int num = fCompPB->Count(kDeltas);
    int i;
    // For each delta
    for( i = 0; i < num; i++ )
    {
        plMaxNode* deltaNode = (plMaxNode*)fCompPB->GetINode(kDeltas, TimeValue(0), i);
        if( !deltaNode )
            continue;
        const MCHAR* dbgNodeName = deltaNode->GetName();

        // Get the GeometrySpans. We ensured they would
        // be generated in it's SetupProperties, and they were created
        // in the MakeMesh pass.
        std::vector<plGeometrySpan*>* movedSpans = &deltaNode->GetSwappableGeom()->fSpans;

        // We want to move the deltas into the same space as the base mesh verts.
        // So the first question is, which space do we want to move them from?
        // Answer is, we want to take the deltas from where they are relative to their OTM (pivot point),
        // and move them into base node's local space. 
        // The idea is that we want to assume the pivot points of the two objects are aligned, even though
        // we don't require that they are. So we transform delta verts to their pivot space, pretend that is the
        // same space as baseNode's pivot space, and transform from that to baseNode's local space.
        // This is somewhat arbitrary, but it allows the delta meshes to be moved aside relative to the base
        // meshes, as well as aligning the Pivot points to align the deltas and base mesh.
        // So:
        //  From deltaLocal to deltaPivot space = deltaVertToPivot * deltaLocalToVert // vertToPivot = OTM
        //  From deltaPivot to basePivot = skip (pretend they are the same).
        //  From basePivot to baseLocal = baseVertToLocal * Inverse(baseVertToPivot) // because baseOTM is in baseVertToLocal
        // mf
        hsMatrix44 bvert2local = baseNode->GetVertToLocal44() * baseNode->Matrix3ToMatrix44(Inverse(baseNode->GetOTM()));
        hsMatrix44 dlocal2vert = deltaNode->GetOTM44() * deltaNode->GetLocalToVert44();
        hsMatrix44 d2b =  bvert2local * dlocal2vert;
        hsMatrix44 d2bTInv;
        d2b.GetInverse(&d2bTInv);
        d2bTInv.GetTranspose(&d2bTInv);
        // Error check
        if (movedSpans->size() != baseSpans->size())
        {
            pErrMsg->Set(true, deltaNode->GetName(), "Delta mesh mismatch with base").CheckAndAsk();
            pErrMsg->Set(false);
            continue;
        }

        plMorphDelta delta;
        delta.ComputeDeltas(*baseSpans, *movedSpans, d2b, d2bTInv);
        morphArr.AddDelta(delta);
    }

    return morphArr.GetNumDeltas() > 0;
}



class plMorphSeqComp : public plComponent
{
protected:
    plMorphLayComp*             IGetLayerComp(int i);

public:
    enum 
    {
        kLayers,
        kBaseNode
    };
    
    plMorphSeqComp();
    void DeleteThis() override { delete this; }
    
    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;
};


CLASS_DESC(plMorphSeqComp, gMorphSeqCompDesc, "Morph Sequence",  "MorphSeq", COMP_TYPE_AVATAR, MORPHSEQ_COMP_CID)


class plMorphSeqProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_TARGS)
            {
                std::vector<Class_ID> cids;
                cids.push_back(MORPHLAY_COMP_CID);
                IParamBlock2 *pb = map->GetParamBlock();
                plPick::Node(pb, plMorphSeqComp::kLayers, &cids, false, false);

                map->Invalidate(plMorphSeqComp::kLayers);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plMorphSeqProc gMorphSeqProc;

/////////////////////////////////////////////////////////////////////////////////////


ParamBlockDesc2 gMorphSeqBk
(   
    plComponent::kBlkComp, _T("MorphSeq"), 0, &gMorphSeqCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_MORPHSEQ, IDS_COMP_MORPHSEQ, 0, 0, &gMorphSeqProc,

    plMorphSeqComp::kLayers,    _T("Layers"),   TYPE_INODE_TAB, 0,      0, 0,
        p_ui,           TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
        p_end,

    plMorphSeqComp::kBaseNode, _T("BaseNode"),  TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CHOOSE_OBJECT,
        p_sclassID, GEOMOBJECT_CLASS_ID,
        p_prompt, IDS_COMP_CHOSE_OBJECT,
        p_end,

    p_end
);

bool plMorphSeqComp::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    plMaxNode* baseNode = (plMaxNode*)fCompPB->GetINode(kBaseNode);
    if( !baseNode )
    {
        pErrMsg->Set(true, node->GetName(), "Missing base (neutral) geometry node").CheckAndAsk();
        pErrMsg->Set(false);
        return true;
    }
    if( !baseNode->GetSwappableGeom() )
        baseNode->SetSwappableGeom(new plSharedMesh);
//  baseNode->SetForceLocal(true);

    return true;
}

bool plMorphSeqComp::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    return true;
}

bool plMorphSeqComp::Convert(plMaxNode* node, plErrorMsg* pErrMsg) 
{ 
    const MCHAR* dbgNodeName = node->GetName();

    // Make our plMorphSequence modifier
    plMorphSequence* morphSeq = const_cast<plMorphSequence *>(plMorphSequence::ConvertNoRef(node->GetSceneObject()->GetModifierByType(plMorphSequence::Index())));
    if (!morphSeq)
    {
        morphSeq = new plMorphSequence;
        node->AddModifier(morphSeq, IGetUniqueName(node));
    }
    
    // Get our base geometry.
    plMaxNode* baseNode = (plMaxNode*)fCompPB->GetINode(kBaseNode);
    plSharedMesh* mesh = baseNode->GetSwappableGeom();
    std::vector<plGeometrySpan*>* baseSpans = &mesh->fSpans;
    //morphSeq->AddSharedMesh(mesh);

    // Error check we have some base geometry.

    plMorphDataSet *set = new plMorphDataSet;
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), set, node->GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(set->GetKey(), new plGenRefMsg(mesh->GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);

    const int num = fCompPB->Count(kLayers);
    int i;
    // For each layer that we have
    for( i = 0; i < num; i++ )
    {
        plMorphLayComp* layComp = IGetLayerComp(i);
        if( !layComp )
            continue;

        // Layer is a sequence of geometry deltas. A layer will
        // become a plMorphArray.
        plMorphArray morphArr;

        if( layComp->SetupLayer(morphArr, baseNode, baseSpans, pErrMsg) )
        {
            //morphSeq->AddLayer(morphArr);
            set->fMorphs.emplace_back(morphArr);
        }
    }

    // Error check - should make sure we wound up with something valid,
    // as opposed to a million empty deltas or something.

    return true; 
}

bool plMorphSeqComp::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

plMorphLayComp* plMorphSeqComp::IGetLayerComp(int i)
{
    plMaxNode* node = (plMaxNode*)fCompPB->GetINode(kLayers, TimeValue(0), i);
    if( !node )
        return nullptr;

    plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
    if( !comp )
        return nullptr;

    if( comp->ClassID() == MORPHLAY_COMP_CID )
    {
        return (plMorphLayComp*)comp;
    }

    return nullptr;
}

plMorphSeqComp::plMorphSeqComp()
{
    fClassDesc = &gMorphSeqCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}


