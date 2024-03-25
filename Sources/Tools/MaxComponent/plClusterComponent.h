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

#ifndef plClusterComponent_inc
#define plClusterComponent_inc

const Class_ID CLUSTER_COMP_CID(0x508a10f4, 0x70112d59);

class hsRadixSortElem;
class plExportProgressBar;
class hsBitVector;
class plDistribInstTab;
class plMaxNodeTab;
class hsBitVector;
class plDistribComponent;
class plDistribInstance;
class plClusterGroup;

#include <vector>

class plDistribCompTab : public Tab<plDistribComponent*>
{
};

class plBox3Tab : public Tab<Box3>
{
};

class plClusterComponent : public plComponent
{
public:
    enum {
        kClusters       = 0,
        kOptimization,
        kFadeIns,
        kFadeOuts,
        kWindBone, // Obsolete, moved to DistribComponent
        kWindBones,
        kAutoGen,
        kAutoInstance
    };
protected:
    float               fClusterSize;

    // These are temp, only used during processing.
    plDistribInstTab**  fClusterBins;
    int                 fSizes[3];
    plDistribCompTab    fDistribTab;
    plComponentBase*    fLocationComp;

    std::vector<plClusterGroup*> fClusterGroups;

    // And more temps used only during Convert
    BOOL            fSetupDone;
    BOOL            fAutoGen;
    BOOL            fExported;

    void            ICheckWindBone();

    BOOL            IGetLocation();
    void            ISetLocation(plMaxNode* node);

    void            ISetupRenderDependencies();
    void            IAssignRenderDependencies(hsRadixSortElem*& prevStart, hsRadixSortElem*& prevEnd,
                                                   hsRadixSortElem*& currStart, hsRadixSortElem*& currEnd);

    BOOL            IBuildNodeTab(plDistribInstTab& nodes, plErrorMsg* pErrMsg, plExportProgressBar& bar);
    void            IClearNodeTab();
    void            IBuildDistribTab();
    void            IClearDistribTab();

    BOOL            IsFlexible() const;
    void            IRandomizeSkinWeights(Mesh* mesh, const Point3& flex) const;
    void            IFinishDoneNodes(INodeTab& doneNodes, plBox3Tab& fade, INodeTab& bone, hsBitVector& boneIsParent);
    BOOL            ICanCluster(plDistribInstance& node);
    BOOL            ICanCluster(plDistribInstance& node0, plDistribInstance& node1);
    BOOL            IClusterGroup(plDistribInstTab& nodes, INodeTab& clusters, plExportProgressBar& bar);
    INode*          IMakeOne(plDistribInstTab& nodes);
    Box3            IPartition(plDistribInstTab& nodes);

    void                    IClusterBins(plDistribInstTab& nodes, Box3& box);
    int                     IGetBinCount();
    void                    IDeleteClusterBins();
    plDistribInstTab*       IGetClusterBin(const Box3& box, const Point3& loc);

public:
    plClusterComponent();
    void DeleteThis() override { delete this; }

    void    Clear();
    BOOL    Cluster(plErrorMsg* pErrMsg);
    void    Select();

    BOOL    AutoGen(plErrorMsg* pErrMsg);
    void    AutoClear(plErrorMsg* pErrMsg);

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    int             GetNumGroups() { if (fSetupDone) return fClusterGroups.size(); return 0; }
    plClusterGroup *GetGroup(int index) { if (fSetupDone) return fClusterGroups[index]; return nullptr; }

};

#endif // plClusterComponent_inc
