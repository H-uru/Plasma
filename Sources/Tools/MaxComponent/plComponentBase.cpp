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
#include <string_theory/format>

#include "plMessageBox/hsMessageBox.h"

#include "plComponentBase.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNodeBase.h"
#include "MaxMain/MaxAPI.h"

#include "MaxMain/plPlasmaRefMsgs.h"
#include "plAutoUIComp.h"

plComponentBase::plComponentBase() : fClassDesc(), fCompPB(), fTargsPB()
{
}

plComponentBase::~plComponentBase()
{
    DeleteAllRefsFromMe();
}

CreateMouseCallBack* plComponentBase::GetCreateMouseCallBack()
{
    return nullptr;
}

void plComponentBase::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
    fClassDesc->BeginEditParams(ip, this, flags, prev);
}

void plComponentBase::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
    fClassDesc->EndEditParams(ip, this, flags, next);
}

int plComponentBase::NumParamBlocks()
{
    return 2;
}

IParamBlock2 *plComponentBase::GetParamBlock(int i)
{
    if (i == kRefComp)
        return fCompPB;
    else if (i == kRefTargs)
        return fTargsPB;

    return nullptr;
}

IParamBlock2 *plComponentBase::GetParamBlockByID(BlockID id)
{
    if (fCompPB && fCompPB->ID() == id)
        return fCompPB;
    else if (fTargsPB && fTargsPB->ID() == id)
        return fTargsPB;

    return nullptr;
}

// So our animatables will show up in the trackview
int plComponentBase::NumSubs()
{
    return 1;
}

Animatable *plComponentBase::SubAnim(int i)
{
    return fCompPB;
}

RefTargetHandle plComponentBase::Clone(RemapDir &remap)
{
    plComponentBase *obj = (plComponentBase*)fClassDesc->Create(false);
    // Do the base clone
    BaseClone(this, obj, remap);
    // Copy our references
    if (fCompPB)
        obj->ReplaceReference(kRefComp, fCompPB->Clone(remap));
    if (fTargsPB)
        obj->ReplaceReference(kRefTargs, fTargsPB->Clone(remap));

    return obj;
}

void plComponentBase::BuildMesh(TimeValue t)
{
}

void plComponentBase::FreeCaches()
{
}

void plComponentBase::GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
    box.MakeCube(Point3(0,0,0), 0);
}

void plComponentBase::GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
    box.MakeCube(Point3(0,0,0), 0);
}

int plComponentBase::Display(TimeValue t, INode *node, ViewExp *vpt, int flags)
{
    return 0;
}

int plComponentBase::HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
    return 0;
}

int plComponentBase::NumRefs()
{
    return 2;
}

RefTargetHandle plComponentBase::GetReference(int i)
{
    if (i == kRefComp)
        return fCompPB;
    else if (i == kRefTargs)
        return fTargsPB;

    return nullptr;
}

void plComponentBase::SetReference(int i, RefTargetHandle rtarg)
{
    if (i == kRefTargs)
        fTargsPB = (IParamBlock2*)rtarg;
    else if (i == kRefComp)
        fCompPB = (IParamBlock2*)rtarg;
}

RefResult plComponentBase::NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
                                            PartID& partID, RefMessage message
                                            MAX_REF_PROPAGATE)
{
    return REF_SUCCEED;
}

IOResult plComponentBase::Save(ISave* isave)
{
    return IO_OK;
}

IOResult plComponentBase::Load(ILoad* iload)
{
    return IO_OK;
}

void plComponentBase::AddTargetsToList(INodeTab& list)
{
    int i;
    for( i = 0; i < NumTargets(); i++ )
    {
        INode* targ = GetTarget(i);
        if( targ )
            list.Append(1, &targ);
    }
}

uint32_t plComponentBase::NumTargets()
{
    if (fTargsPB)
        return fTargsPB->Count(kTargs);

    return 0;
}

plMaxNodeBase *plComponentBase::GetTarget(uint32_t i)
{
    if (fTargsPB && i < NumTargets())
        return (plMaxNodeBase*)fTargsPB->GetINode(kTargs, 0, i);

    return nullptr;
}

void plComponentBase::AddTarget(plMaxNodeBase *target)
{
    if (!target)
        return;

    // Make sure we don't already ref this
    uint32_t count = fTargsPB->Count(kTargs);
    for (uint32_t i = 0; i < count; i++)
    {
        if (fTargsPB->GetINode(kTargs, 0, i) == target)
            return;
    }

    // Add it to our list
    fTargsPB->Append(kTargs, 1, (INode**)&target);
    
    NotifyDependents(FOREVER, (PartID)target, REFMSG_USER_TARGET_ADD);
}

void plComponentBase::DeleteTarget(plMaxNodeBase *target)
{
    if (!target)
        return;

    uint32_t count = fTargsPB->Count(kTargs);
    for (uint32_t i = 0; i < count; i++)
    {
        if (fTargsPB->GetINode(kTargs, 0, i) == target)
        {
            fTargsPB->Delete(kTargs, i, 1);
            NotifyDependents(FOREVER, (PartID)target, REFMSG_USER_TARGET_DELETE);
            return;
        }
    }
}

void plComponentBase::DeleteAllTargets()
{
    while (fTargsPB->Count(kTargs) > 0)
    {
        INode *node = fTargsPB->GetINode(kTargs);
        if (node)
            NotifyDependents(FOREVER, (PartID)node, REFMSG_USER_TARGET_DELETE);

        fTargsPB->Delete(kTargs, 0, 1);
    }
}

bool plComponentBase::IsTarget(plMaxNodeBase *node)
{
    if (!node)
        return false;

    for (int i = 0; i < fTargsPB->Count(kTargs); i++)
        if (fTargsPB->GetINode(kTargs, 0, i) == node)
            return true;

    return false;
}

ST::string plComponentBase::IGetUniqueName(plMaxNodeBase* target)
{
    // Make sure we've actually got multiple *used* targets.  (Some could be nil)
    int numUsedTargs = 0;
    int thisTargIdx = -1;
    for (int i = 0; i < fTargsPB->Count(kTargs); i++)
    {
        plMaxNodeBase* thisTarg = (plMaxNodeBase*)fTargsPB->GetINode(kTargs, 0, i);

        if (thisTarg)
            numUsedTargs++;

        if (thisTarg == target)
            thisTargIdx = i;

        // If we've figured out whether or not we have multiple targets, and we
        // found which target this one is, we can break out early
        if (numUsedTargs > 1 && thisTargIdx != -1)
            break;
    }

    hsAssert(thisTargIdx != -1, "Bad target for IGetUniqueName");

    if (numUsedTargs > 1)
        return ST::format("{}_{}", GetINode()->GetName(), thisTargIdx);
    else
        return M2ST(GetINode()->GetName());
}

plMaxNodeBase *plComponentBase::GetINode()
{
    // Go through the reflist looking for RefMakers with a ref to this component.
    // There should only be one INode in this list.
    DependentIterator di(this);
    ReferenceMaker* rm = di.Next();
    while (rm != nullptr)
    {
        if (rm->SuperClassID() == BASENODE_CLASS_ID)
            return (plMaxNodeBase*)rm;

        rm = di.Next();
    }

    return nullptr;
}

bool plComponentBase::IsExternal()
{
    return CanConvertToType(EXT_COMPONENT_CLASSID);
}

bool plComponentBase::IsCurMsgLocal()
{
    // Was the last notification a message propagated up from a target?
    // Ignore it if so.
    if (fTargsPB->LastNotifyParamID() != -1)
        return false;

    // Was the last notification from a ref in the component PB?  If so, assume
    // it was a propagated message from the ref.  Real changes to that ref
    // parameter will be signalled with a user ref message.
    if (fCompPB->LastNotifyParamID() != -1)
    {
        ParamID id = fCompPB->LastNotifyParamID();
        if (is_ref(fCompPB->GetParamDef(id)))
            return false;
    }

    return true;
}

bool plComponentBase::IsObsolete()
{
    return ((plComponentClassDesc*)fClassDesc)->IsObsolete();
}

bool DoesPBReferenceNode(IParamBlock2* pb, INode* node);

static bool CheckRef(ReferenceTarget* targ, INode* node)
{
    if (!targ)
        return false;
    if (targ->SuperClassID() == BASENODE_CLASS_ID)
    {
        if ((INode*)targ == node)
            return true;
    }
    else if (targ->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
    {
        return DoesPBReferenceNode((IParamBlock2*)targ, node);
    }
    return false;
}

static bool DoesPBReferenceNode(IParamBlock2* pb, INode* node)
{
    for (int i = 0; i < pb->NumParams(); i++)
    {
        ParamID id = pb->IndextoID(i);
        ParamType2 type = pb->GetParameterType(id);
        if (type == TYPE_INODE)
        {
            if (pb->GetINode(id) == node)
                return true;
        }
        if (type == TYPE_INODE_TAB)
        {
            for (int iNode = 0; iNode < pb->Count(id); iNode++)
            {
                if (pb->GetINode(id, 0, iNode) == node)
                    return true;
            }
        }
        if (type == TYPE_REFTARG)
        {
            ReferenceTarget* targ = pb->GetReferenceTarget(id);
            bool ret = CheckRef(targ, node);
            if (ret)
                return true;
        }
        if (type == TYPE_REFTARG_TAB)
        {
            for (int iRef = 0; iRef < pb->Count(id); iRef++)
            {
                ReferenceTarget* targ = pb->GetReferenceTarget(id, 0, iRef);
                bool ret = CheckRef(targ, node);
                if (ret)
                    return true;
            }
        }
    }

    return false;
}

bool plComponentBase::DoReferenceNode(INode* node)
{
    if (!fCompPB)
        return false;

    return DoesPBReferenceNode(fCompPB, node);
}

void plComponentBase::CreateRollups()
{
    if (!fCompPB)
        return;

    ParamBlockDesc2 *pd = fCompPB->GetDesc();
    plComponentClassDesc *cd = (plComponentClassDesc*)pd->cd;

    // This is a plAutoUIComp, we need to treat it differently
    if (cd->IsAutoUI())
    {
        plAutoUIClassDesc *cd = (plAutoUIClassDesc*)pd->cd;
        cd->CreateAutoRollup(fCompPB);
    }
    // We're using a normal param block with auto UI
    else if (pd->flags & P_AUTO_UI)
    {
        if (pd->flags & P_MULTIMAP)
        {
            int nMaps = pd->map_specs.Count();
            for (int i = 0; i < nMaps; i++)
            {
                ParamBlockDesc2::map_spec spec = pd->map_specs[i];

                // Create the rollout
                IParamMap2 *map = CreateCPParamMap2(spec.map_id,
                                                    fCompPB,
                                                    GetCOREInterface(),
                                                    hInstance,
                                                    MAKEINTRESOURCE(spec.dlg_template),
                                                    GetString(spec.title),
                                                    spec.rollup_flags,
                                                    spec.dlgProc,
                                                    nullptr,
                                                    ROLLUP_CAT_STANDARD);

                // Save the rollout in the paramblock
                fCompPB->SetMap(map, spec.map_id);
            }
        }
        else
        {
            // Create the rollout
            IParamMap2 *map = CreateCPParamMap2(0,
                                                fCompPB,
                                                GetCOREInterface(),
                                                hInstance,
                                                MAKEINTRESOURCE(pd->dlg_template),
                                                GetString(pd->title),
                                                pd->flags,
                                                pd->dlgProc,
                                                nullptr,
                                                ROLLUP_CAT_STANDARD);

            // Save the rollout in the paramblock
            fCompPB->SetMap(map);
        }
    }

    fCompPB->ReleaseDesc();
}

void plComponentBase::DestroyRollups()
{
    if (!fCompPB)
        return;

    ParamBlockDesc2 *pd = fCompPB->GetDesc();
    plComponentClassDesc *cd = (plComponentClassDesc*)pd->cd;

    // This is a plAutoUIComp, we need to treat it differently
    if (cd->IsAutoUI())
    {
        plAutoUIClassDesc *autoCD = (plAutoUIClassDesc*)pd->cd;
        autoCD->DestroyAutoRollup();
    }
    // We're using a normal param block with auto UI
    else if (pd->flags & P_AUTO_UI)
    {
        if (pd->flags & P_MULTIMAP)
        {
            int nMaps = pd->map_specs.Count();
            for (int i = 0; i < nMaps; i++)
            {
                MapID id = pd->map_specs[i].map_id;
                // Destroy any parammap saved in the rollup
                IParamMap2 *map = fCompPB->GetMap(id);
                fCompPB->SetMap(nullptr, id);
                if (map)
                    DestroyCPParamMap2(map);
            }
        }
        else
        {
            // Destroy any parammap saved in the rollup
            IParamMap2 *map = fCompPB->GetMap();
            fCompPB->SetMap(nullptr);
            if (map)
                DestroyCPParamMap2(map);
        }
    }

    fCompPB->ReleaseDesc();
}

////////////////////////////////////////////////////////////////////////////////

static bool INodeHasComponent(plMaxNodeBase *node, plMaxNodeBase *compNode)
{
    uint32_t count = node->NumAttachedComponents();
    for (uint32_t i = 0; i < count; i++)
    {
        if (node->GetAttachedComponent(i)->GetINode() == compNode)
            return true;
    }

    return false;
}

int plSharedComponents(INodeTab& nodes, INodeTab& components)
{
    components.ZeroCount();

    if (nodes.Count() == 0)
        return 0;

    plMaxNodeBase *firstNode = (plMaxNodeBase*)nodes[0];
    int num = firstNode->NumAttachedComponents();
    
    // Resize the list to it's max size to be more efficient
    components.SetCount(num);

    int i;

    // Put all the components on the first node into a list
    for (i = 0; i < num; i++)
        components[i] = firstNode->GetAttachedComponent(i)->GetINode();

    // Delete any components that aren't on all the other nodes
    for (i = 1; i < nodes.Count(); i++)
    {
        plMaxNodeBase *node = (plMaxNodeBase*)nodes[i];
        uint32_t count = node->NumAttachedComponents();

        for (int j = components.Count()-1; j >= 0; j--)
        {
            if (!INodeHasComponent(node, (plMaxNodeBase*)components[j]))
                components.Delete(j, 1);
        }
    }

    return components.Count();
}


///////////////////////////////////////////////////////////////////////////////////////////

static void UpdateComponentVisibility(plMaxNodeBase *node)
{
    plComponentBase *comp = node->ConvertToComponent();
    if (comp)
    {
        node->Freeze(TRUE);
        node->Hide(!comp->AllowUnhide());
    }

    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        plMaxNodeBase *childNode = (plMaxNodeBase*)node->GetChildNode(i);
        UpdateComponentVisibility(childNode);
    }
}

static void UnlinkComponents(plMaxNodeBase *node)
{
    plComponentBase *comp = node->ConvertToComponent();
    if (comp)
        comp->DeleteAllTargets();
    
    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        plMaxNodeBase *child = (plMaxNodeBase*)node->GetChildNode(i);
        UnlinkComponents(child);
    }
}

static void FindObsoleteComponents(plMaxNodeBase *node, std::vector<plComponentBase*>& obsoleteComps)
{
    plComponentBase *comp = node->ConvertToComponent();
    if (comp && comp->IsObsolete())
        obsoleteComps.push_back(comp);

    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        plMaxNodeBase *childNode = (plMaxNodeBase*)node->GetChildNode(i);
        FindObsoleteComponents(childNode, obsoleteComps);
    }
}

static bool gUpdatingComponents = false;

#include <unordered_set>


static void ComponentNotify(void *param, NotifyInfo *info)
{
    if (info->intcode == NOTIFY_NODE_UNHIDE)
    {
        if (!gUpdatingComponents)
        {
            plMaxNodeBase *node = (plMaxNodeBase*)info->callParam;
            plComponentBase *comp = node ? node->ConvertToComponent() : nullptr;
            if (comp)
            {
                node->Hide(!comp->AllowUnhide());
                node->Freeze(TRUE);
            }
        }
    }
    else if (info->intcode == NOTIFY_FILE_POST_OPEN)
    {
        plComponentShow::Update();
        GetCOREInterface()->ForceCompleteRedraw();
                                            // Also checking bitmap silent mode, since the export script uses that
        if (!hsMessageBox_SuppressPrompts && !TheManager->SilentMode())
        {
            // Get all the obsolete components in the scene
            std::vector<plComponentBase*> obsoleteComps;
            FindObsoleteComponents((plMaxNodeBase*)GetCOREInterface()->GetRootNode(), obsoleteComps);

            // For now, just get the categories (could get the component names)
            std::unordered_set<ST::string> names;
            for (auto i : obsoleteComps)
                names.insert(M2ST(i->GetObjectName(true)));

            if (!obsoleteComps.empty())
            {
                ST::string_stream ss;
                ss << "Components of the following obsolete types\nwere found in this scene.  Please delete them.\n\n";
                for (const auto& name : names)
                    ss << name << '\n';
                plMaxMessageBox(nullptr, ST2T(ss.to_string()), _T("Obsolete Components"), MB_OK);
            }
        }
    }
    // WEIRD HACK - If lots of components are linked to objects when a scene is closed
    // the closing goes really slow.  I assume it is just repeatedly calling some
    // unoptimized section of code (it would probably happen for any large number of
    // references).  So, at this point if the scene was changed the user would have already
    // been prompted to save it, so we can modify the scene by deleting all the component
    // references. -Colin
    else if (info->intcode == NOTIFY_SYSTEM_SHUTDOWN ||
            info->intcode == NOTIFY_FILE_PRE_OPEN)
    {
        UnlinkComponents((plMaxNodeBase*)GetCOREInterface()->GetRootNode());
    }
}

void plComponentShow::Init()
{
    RegisterNotification(ComponentNotify, nullptr, NOTIFY_FILE_POST_OPEN);
    RegisterNotification(ComponentNotify, nullptr, NOTIFY_NODE_UNHIDE);
    RegisterNotification(ComponentNotify, nullptr, NOTIFY_SYSTEM_SHUTDOWN);
    RegisterNotification(ComponentNotify, nullptr, NOTIFY_FILE_PRE_OPEN);
}

void plComponentShow::Update()
{
    gUpdatingComponents = true;
    UpdateComponentVisibility((plMaxNodeBase*)GetCOREInterface()->GetRootNode());
    gUpdatingComponents = false;
}
