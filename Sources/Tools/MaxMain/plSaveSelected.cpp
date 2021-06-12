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

#include "MaxComponent/plComponent.h"
#include "plMaxNode.h"

#include "MaxAPI.h"



bool IIsNodeInTab(INodeTab &tab, INode *node)
{
    for (int i = 0; i < tab.Count(); i++)
        if (tab[i] == node)
            return true;

    return false;
}

class ComponentRefs
{
public:
    plComponentBase *comp;
    INodeTab refs;
};

void plSaveSelected()
{
    Interface *ip = GetCOREInterface();

    // Get the Max filename to save to
    TCHAR buf[256]{ 0 };
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ip->GetMAXHWnd();
    ofn.lpstrFilter = _T("3ds max (*.max)\0*.max\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = std::size(buf);
//  ofn.lpstrInitialDir = ip->GetDir(APP_SCENE_DIR);
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = _T("max");

    if (GetSaveFileName(&ofn))
    {
        int count = ip->GetSelNodeCount();
        int i;
        
        // Put all the selected nodes in a list
        INodeTab selected;
        for (i = 0; i < count; i++)
        {
            plMaxNode *node = (plMaxNode*)ip->GetSelNode(i);
            selected.Append(1, (INode**)&node);
        }

        // Put all the components attached to those nodes in a list
        INodeTab components;
        for (i = 0; i < count; i++)
        {
            plMaxNode *node = (plMaxNode*)ip->GetSelNode(i);

            uint32_t compCount = node->NumAttachedComponents();
            for (int j = 0; j < compCount; j++)
            {
                INode *compNode = node->GetAttachedComponent(j)->GetINode();//Node(j);

                if (!IIsNodeInTab(components, compNode))
                    components.Append(1, &compNode);
            }
        }

        // Find the objects that the components are reffing that are not part of the selection.
        // Back them up, and delete them out of the components ref list so they won't be saved too.
        std::vector<ComponentRefs> out;
        for (i = 0; i < components.Count(); i++)
        {
            plComponentBase *comp = ((plMaxNode*)components[i])->ConvertToComponent();

            for (int j = comp->NumTargets() - 1; j >= 0; --j)
            {
                plMaxNodeBase *node = comp->GetTarget(j);

                if (!node)
                    continue;
                
                const MCHAR* t1 = components[i]->GetName();
                const MCHAR* t2 = node->GetName();

                if (!IIsNodeInTab(selected, node))
                {
                    uint32_t idx = -1;
                    for (uint32_t k = 0; k < out.size(); k++)
                    {
                        if (out[k].comp == comp)
                            idx = k;
                    }

                    if (idx == -1)
                    {
                        idx = out.size();
                        out.resize(idx+1);
                        out[idx].comp = comp;
                    }

                    out[idx].refs.Append(1, (INode**)&node);

                    comp->DeleteTarget(node);
                }
            }
        }

        // Save the selected objects and their components
        INodeTab allNodes;
        allNodes.Append(selected.Count(), &selected[0]);
        allNodes.Append(components.Count(), &components[0]);

        ip->FileSaveNodes(&allNodes, buf);

        // Restore the component refs to objects that weren't selected
        for (i = 0; i < out.size(); i++)
        {
            for (int j = 0; j < out[i].refs.Count(); j++)
                out[i].comp->AddTarget((plMaxNode*)out[i].refs[j]);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void IFindComponentsRecur(plMaxNode *node, std::vector<plComponentBase*> &components)
{
    if (node->IsComponent())
        components.push_back(node->ConvertToComponent());

    for (int i = 0; i < node->NumberOfChildren(); i++)
        IFindComponentsRecur((plMaxNode*)node->GetChildNode(i), components);
}

void IMergeComponents(plComponentBase *to, plComponentBase *from)
{
    for (int i = 0; i < from->NumTargets(); i++)
        to->AddTarget(from->GetTarget(i));

    // Delete the component from the scene
    theHold.Begin();
    GetCOREInterface()->DeleteNode(from->GetINode());
    theHold.Accept(_T("Delete Component"));
}

plMaxNode *IFindComponentRecur(plMaxNode *node, const MCHAR* name)
{
    if (_tcscmp(node->GetName(), name) == 0)
    {
        if (node->IsComponent())
            return node;
    }

    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        plMaxNode *ret = IFindComponentRecur((plMaxNode*)node->GetChildNode(i), name);
        if (ret)
            return ret;
    }

    return nullptr;
}

void plMerge()
{
    Interface *ip = GetCOREInterface();

    // Get the Max filename to merge
    TCHAR file[MAX_PATH]{ 0 };

    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ip->GetMAXHWnd();
    ofn.lpstrFilter = _T("3ds max (*.max)\0*.max\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = file;
    ofn.nMaxFile = std::size(file);
//  ofn.lpstrInitialDir = ip->GetDir(APP_SCENE_DIR);
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrTitle = _T("Merge");

    if (!GetOpenFileName(&ofn))
        return;

    // Don't actually merge yet, just get the names of every node in the file
    NameTab nodeNames;
    ip->MergeFromFile(file, TRUE, FALSE, FALSE, MERGE_LIST_NAMES, &nodeNames);

    // For each node name, search the current scene for a component with the same name.
    // If one is found, append "Merged" to it so its name won't cause a conflict during
    // the actual merge.
    std::vector<plMaxNode*> renamedNodes;
    int i;
    for (i = 0; i < nodeNames.Count(); i++)
    {
        plMaxNode *node = IFindComponentRecur((plMaxNode*)ip->GetRootNode(), nodeNames[i]);
        if (node)
        {
            MSTR buf;
            buf.printf(_M("%s Merged"), node->GetName());
            node->SetName(buf);

            renamedNodes.push_back(node);
        }
    }

    // Do the merge
    ip->MergeFromFile(file);

    // Rename the components back to their original names
    for (i = 0; i < renamedNodes.size(); i++)
    {
        TCHAR buf[256];
        _tcsncpy(buf, renamedNodes[i]->GetName(), std::size(buf));
        buf[_tcsnlen(buf, std::size(buf))-6] = '\0';
        renamedNodes[i]->SetName(buf);
    }

    // Put all the components in the scene in a list
    std::vector<plComponentBase*> components;
    IFindComponentsRecur((plMaxNode*)ip->GetRootNode(), components);

    nodeNames.ZeroCount();

    // For each component, search the scene for any other components with the same
    // name and type.  If there are any, merge their target lists and delete one.
    for (i = 0; i < renamedNodes.size(); i++)
    {
        if (!renamedNodes[i])
            continue;

        plComponentBase *oldComp = renamedNodes[i]->ConvertToComponent();
        auto oldCompName = oldComp->GetINode()->GetName();

        for (int j = 0; j < components.size(); j++)
        {
            plComponentBase *comp = components[j];

            if (oldComp == comp)
                components[j] = nullptr;
            else if (comp)
            {
                auto temp = comp->GetINode()->GetName();
                
                if (!_tcscmp(oldCompName, comp->GetINode()->GetName()) &&
                    comp->ClassID() == comp->ClassID())
                {
                    IMergeComponents(comp, oldComp);
                    nodeNames.AddName(oldCompName);
                    continue;
                }
            }
        }
    }

    // Send out merge notifications again, so that the component dialog will be updated
    BroadcastNotification(NOTIFY_FILE_PRE_MERGE);
    BroadcastNotification(NOTIFY_FILE_POST_MERGE);

#if 0
    if (nodeNames.Count() == 0)
        return;

    // Actually calculate the size of all the merged component names, because
    // a static buffer could be too small in large merges
    uint32_t size = 0;
    for (i = 0; i < nodeNames.Count(); i++)
        size += _tcslen(nodeNames[i]) + 1;

    // Put all the component names in a list and show it to the user
    TCHAR *buf = new TCHAR[size+25];
    _tcscpy(buf, _T("Components Merged:\n\n"));

    for (i = 0; i < nodeNames.Count(); i++)
    {
        _tcscat(buf, nodeNames[i]);
        _tcscat(buf, _T("\n"));
    }

    plMaxMessageBox(ip->GetMAXHWnd(), buf, _T("Components Merged"), MB_OK);
    
    delete [] buf;
#endif
}
