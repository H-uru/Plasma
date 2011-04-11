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
#include "HeadSpin.h"


#include "max.h"
#include "meshdlib.h" 
#include "dummy.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../MaxExport/plExportProgressBar.h"
#include "../MaxMain/plMaxNode.h"

#include "hsTypes.h"

#include "hsBitVector.h"
#include "../plMath/hsRadixSort.h"
#include "../plMath/plRandom.h"
#include "../pfAnimation/plBlower.h"

#include "plDicer.h"
#include "plDistribComponent.h"
#include "../MaxConvert/plDistributor.h"
#include "../MaxConvert/plDistTree.h"
#include "plMiscComponents.h"

#include "plClusterComponent.h"


#include "../MaxConvert/plClusterUtil.h"
#include "../plDrawable/plClusterGroup.h"
#include "../plDrawable/plSpanTemplate.h"

#include <vector>
using namespace std;

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Start with the component bookkeeping song and dance.
// Actual working code follows.
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void DummyCodeIncludeFuncCluster()
{

}

class plClusterComponentProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_COMMAND:
			if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_CLUSTER_DO_THE_DANCE) )
			{
				plClusterComponent* cc = (plClusterComponent*)map->GetParamBlock()->GetOwner();
				cc->Cluster(nil);

				return TRUE;
			}
			if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_CLUSTER_CLEAR) )
			{
				plClusterComponent* cc = (plClusterComponent*)map->GetParamBlock()->GetOwner();
				cc->Clear();

				return TRUE;
			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plClusterComponentProc gClusterCompProc;

//Max desc stuff necessary below.
CLASS_DESC(plClusterComponent, gClusterCompDesc, "Cluster",  "Cluster", COMP_TYPE_DISTRIBUTOR, CLUSTER_COMP_CID)

class plClusterCompAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if( id == plClusterComponent::kWindBones )
		{
			plClusterComponent *comp = (plClusterComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plClusterCompAccessor gClusterCompAccessor;

ParamBlockDesc2 gClusterBk
(	
	plComponent::kBlkComp, _T("Cluster"), 0, &gClusterCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_CLUSTER, IDS_COMP_CLUSTERS, 0, 0, &gClusterCompProc,

	plClusterComponent::kClusters,	_T("Clusters"),	TYPE_INODE_TAB, 0,		0, 0,
		end,

	plClusterComponent::kOptimization, _T("Optimization"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_CLUSTERSIZE, IDC_COMP_CLUSTERSIZE_SPIN, 1.0,
		end,	

	plClusterComponent::kFadeIns, _T("FadeIns"), TYPE_POINT3_TAB, 0,	0, 0,
		end,

	plClusterComponent::kFadeOuts, _T("FadeOuts"), TYPE_POINT3_TAB, 0,	0, 0,
		end,

	// OBSOLETE
	plClusterComponent::kWindBone, _T("WindBone"),	TYPE_INODE,		0, 0,
//		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_CLUSTER_WINDBONE,
//		p_prompt, IDS_COMP_CLUSTER_CHOSE_WINDBONE,
		end,

	plClusterComponent::kWindBones,	_T("WindBones"),	TYPE_INODE_TAB, 0,		0, 0,
		end,

	plClusterComponent::kAutoGen,	_T("AutoGen"),	TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_CLUST_AUTOEXPORT,
		end,

	plClusterComponent::kAutoInstance,	_T("AutoInstance"),	TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_CLUST_AUTOINSTANCE,
		end,

	end
);


hsBool plClusterComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fSetupDone = false;
	fExported = false;

	int numClust = fCompPB->Count(kClusters);
	int i;
	for( i = numClust-1; i >= 0; --i )
	{
		if( !fCompPB->GetINode(kClusters, TimeValue(0), i) )
			fCompPB->Delete(kClusters, i, 1);
	}

	return true;
}

static int CompTemplNodes(const void *elem1, const void *elem2) 
{
	plDistribInstance* distA = (plDistribInstance*)elem1;
	plDistribInstance* distB = (plDistribInstance*)elem2;
	
	plMaxNode* a = (plMaxNode*)distA->fNode;
	plMaxNode* b = (plMaxNode*)distB->fNode;
	
	if( a == b )
		return 0;
	
	if( a->GetRenderLevel(!a->GetNoDeferDraw()) < b->GetRenderLevel(!b->GetNoDeferDraw()) )
		return -1;
	if( a->GetRenderLevel(!a->GetNoDeferDraw()) > b->GetRenderLevel(!b->GetNoDeferDraw()) )
		return 1;
	
	if( a < b )
		return -1;
	
	return 1;
}

hsBool plClusterComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !fSetupDone )
	{
		int numClust = fCompPB->Count(kClusters);
		int i;
		for( i = 0; i < numClust; i++ )
		{
			plMaxNodeBase* clust = (plMaxNodeBase*)fCompPB->GetINode(kClusters, TimeValue(0), i);
			if( clust )
			{
				Box3 fade(fCompPB->GetPoint3(kFadeIns, TimeValue(0), i), fCompPB->GetPoint3(kFadeOuts, TimeValue(0), i));

				// Deal here is that, although we'd love to properly sort all the time, most of the time we don't
				// need to and/or can't afford it, at least with the closeup dense plants. This is a sort of hacky
				// way to guess whether we can afford a proper sort. The idea is that anything that is only visible
				// from farther than N feet away is probably some cheap imposter kind of representation that we can
				// afford to face sort (it's likely to be two sided convex objects too, which means it'll need the sort).
				// Can we do better? Not tonight.
				const float kMinDistantFadeIn = 70.f;
				const float kMaxDistantFadeOut = 10.f;
				BOOL faceSort = false;
				if( fade.Min()[2] < 0 )
				{
					if( fade.Min()[0] > kMinDistantFadeIn )
						faceSort = true;
				}
				else if( fade.Max()[2] > 0 )
				{
					if( fade.Max()[1] < kMaxDistantFadeOut )
						faceSort = true;
				}
				if( faceSort )
					faceSort = true;

				clust->SetFade(fade);
				clust->SetNoDeferDraw(true);
				clust->SetNoFaceSort(!faceSort);
				clust->SetNormalChan(plDistributor::kNormMapChan);

				if( i < fCompPB->Count(kWindBones) )
				{
					plMaxNodeBase* windBone = (plMaxNodeBase*)fCompPB->GetINode(kWindBones, TimeValue(0), i);
					// FISHHACK
					// BoneUpdate
					// Add clust as first bone, windBone as second.
					if( windBone && (windBone != clust) )
						clust->AddBone(windBone);
				}
			}
		}
		
		ISetupRenderDependencies();

		fClusterGroups.clear();
		
		if (fCompPB->GetInt(kAutoInstance))
		{
			hsBitVector doneBits;
			
			IBuildDistribTab();
			if( !fDistribTab.Count() )
			{
				fSetupDone = true;
				return true;
			}
			
			plDistribInstTab nodes;
			
			plExportProgressBar bar;
			if( IBuildNodeTab(nodes, pErrMsg, bar) )
			{
				nodes.Sort(CompTemplNodes);
				
				plClusterUtil util;
				
				int i = 0;
				while( i < nodes.Count() )
				{
					plMaxNode* repNode = (plMaxNode*)nodes[i].fNode;
					
					int nextNode;
					for( nextNode = i+1; (nextNode < nodes.Count()) && (nodes[i].fNode == nodes[nextNode].fNode); nextNode++ )
					{} // intentional, we just want the i value

					// As far as I can tell, we don't actually use the templates generated here, we just use the count
					// to know how many groups to create, and then generate the templates again in Convert().
					// Looks like a hack that never got cleaned up.
					plSpanTemplTab templs = util.MakeTemplates(repNode);
					int j;
					for( j = 0; j < templs.Count(); j++ )
					{
						fClusterGroups.push_back(util.CreateGroup(repNode, GetINode()->GetName()));
						delete templs[j];
					}

					i = nextNode;
				}
			}
			IClearNodeTab();
			IClearDistribTab();
		}

		fSetupDone = true;
	}
	return true;
}

hsBool plClusterComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg) 
{ 
	if( !fExported && (fCompPB->GetInt(kAutoInstance)) )
	{
		hsBitVector doneBits;

		IBuildDistribTab();
		if( !fDistribTab.Count() )
		{
			fExported = true;
			return true;
		}

		plDistribInstTab nodes;

		plExportProgressBar bar;
		if( IBuildNodeTab(nodes, pErrMsg, bar) )
		{
			nodes.Sort(CompTemplNodes);

			plClusterUtil util;

			plDeformVert defVert;
			plShadeVert shadeVert;

			int groupIdx = 0;
			int i = 0;
			while( i < nodes.Count() )
			{
				plL2WTab l2wTab;
				plMaxNode* repNode = (plMaxNode*)nodes[i].fNode;

				Matrix3 l2w = nodes[i].fObjectTM;
				l2wTab.Append(1, &l2w);

				int nextNode;
				for( nextNode = i+1; (nextNode < nodes.Count()) && (nodes[i].fNode == nodes[nextNode].fNode); nextNode++ )
				{
					l2wTab.Append(1, &nodes[nextNode].fObjectTM);
				}

				plSpanTemplTab templs = util.MakeTemplates(repNode);
				int j;
				for( j = 0; j < templs.Count(); j++ )
				{
					util.SetupGroup(fClusterGroups[groupIdx], repNode, templs[j]);
					groupIdx++;

					util.AddClusters(l2wTab, nil, nil);

				}
				i = nextNode;
			}
		}
		IClearNodeTab();
		IClearDistribTab();
		fExported = true;
	}
	return true; 
}

plClusterComponent::plClusterComponent()
{
	fClassDesc = &gClusterCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);

	fClusterBins = nil;
	fSizes[0] = fSizes[1] = fSizes[2] = 0;

	fAutoGen = FALSE;
}


void plClusterComponent::ICheckWindBone()
{
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Working end of the gun follows below this line.
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

BOOL plClusterComponent::AutoGen(plErrorMsg* pErrMsg)
{
	if( !fCompPB->Count(kClusters) && fCompPB->GetInt(kAutoGen) )
	{
		fAutoGen = true;
		return Cluster(pErrMsg);
	}
	return false;
}

void plClusterComponent::AutoClear(plErrorMsg* pErrMsg)
{
	if( fAutoGen )
	{
		Clear();
		fAutoGen = false;
	}
}

void plClusterComponent::IBuildDistribTab()
{
	plDistribCompTab& tab = fDistribTab;
	tab.ZeroCount();
	// Okay, laziest hackiest sort algorithm in the world follows.
	// But it's okay, the number of distributors is small.
	plDistribCompTab sortTab;
	Tab<float> valTab;
	// For each target
	int numTarg = NumTargets();
	int i;
	for( i = 0; i < numTarg; i++ )
	{
		plMaxNodeBase* targ = GetTarget(i);
		if( targ )
		{
			UInt32 count = targ->NumAttachedComponents();
			int j;
			for( j = 0; j < count; j++ )
			// For each DistribComponent
			{
				plComponentBase *comp = targ->GetAttachedComponent(j);
				if( comp && (comp->ClassID() == DISTRIBUTOR_COMP_CID) )
				{
					// Add DistribComponent replicants to nodeTab
					// making sure we don't add any nodes twice.
					plDistribComponent* distComp = (plDistribComponent*)comp;
					int k;
					for( k = 0; k < sortTab.Count(); k++ )
					{
						if( distComp == sortTab[k] )
							break;
					}
					if( k == sortTab.Count() )
					{
						sortTab.Append(1, &distComp);
						float val = distComp->GetIsoPriority();
						valTab.Append(1, &val);
					}
				}

			}
		}
	}
	BitArray gotten(sortTab.Count());
	while( tab.Count() < sortTab.Count() )
	{
		float maxVal = -1.f;
		int maxIdx = -1;
		for( i = 0; i < sortTab.Count(); i++ )
		{
			if( !gotten[i] )
			{
				if( valTab[i] > maxVal )
				{
					maxVal = valTab[i];
					maxIdx = i;
				}
			}
		}
		gotten.Set(maxIdx);
		tab.Append(1, &sortTab[maxIdx]);
	}
}

void plClusterComponent::IClearDistribTab()
{
	int i;
	for( i = 0; i < fDistribTab.Count(); i++ )
		fDistribTab[i]->Done();

	fDistribTab.ZeroCount();
}

BOOL plClusterComponent::IsFlexible() const
{
	int i;
	for( i = 0; i < fDistribTab.Count(); i++ )
	{
		if( fDistribTab[i]->IsFlexible() )
			return true;
	}
	return false;
}


BOOL plClusterComponent::IBuildNodeTab(plDistribInstTab& nodes, plErrorMsg* pErrMsg, plExportProgressBar& bar)
{
	plDistTree distTree;
	nodes.ZeroCount();
	int numDistrib = fDistribTab.Count();
	if( numDistrib )
	{
		int progCnt = 0;
		int i;
		for( i = 0; i < numDistrib; i++ )
			progCnt += fDistribTab[i]->NumTargets();
		if( !progCnt )
			progCnt = 1;

		bar.Start("Compiling", progCnt << 4);

		if( bar.Update(nil, 0) )
			return false;

		for( i = 0; i < numDistrib; i++ )
		{
			plDistribInstTab reps;
			if( !fDistribTab[i]->Distribute(reps, pErrMsg, bar, &distTree) )
				return false;
			if( reps.Count() )
				nodes.Append(reps.Count(), &reps[0]);
		}
	}
	return true;
}

void plClusterComponent::IClearNodeTab()
{
	int i;
	for( i = 0; i < fDistribTab.Count(); i++ )
		fDistribTab[i]->Done();
}

void plClusterComponent::Select()
{
	INodeTab nodeTab;
	int numClust = fCompPB->Count(kClusters);
	int i;
	for( i = 0; i < numClust; i++ )
	{
		INode* clust = fCompPB->GetINode(kClusters, TimeValue(0), i);
		if( clust )
		{
			nodeTab.Append(1, &clust);
		}
	}
	GetCOREInterface()->RemoveNamedSelSet(TSTR(GetINode()->GetName()));
	GetCOREInterface()->AddNewNamedSelSet(nodeTab, TSTR(GetINode()->GetName()));
}

void plClusterComponent::Clear()
{
	int numClust = fCompPB->Count(kClusters);
	if( !numClust )
		return;

	GetCOREInterface()->DisableSceneRedraw();

	plExportProgressBar bar;

	const int log2freq = 2;
	const int maskfreq = (1 << log2freq)-1;
	int totalSteps = numClust >> log2freq;
	if( !totalSteps )
		totalSteps = 1;
	bar.Start("Deleting", totalSteps);

	bar.Update(nil, 0);

	int i;
	for( i = 0; i < numClust; i++ )
	{
		plMaxNode* cluster = (plMaxNode*)fCompPB->GetINode(kClusters, TimeValue(0), i);
		if( cluster )
		{
			// HACK FISH - till we get a real fix for the slowdown caused by 
			// deleting things with a location component on them.
			int numComp = cluster->NumAttachedComponents();
			int j;
			for( j = numComp-1; j >= 0; --j )
			{
				plComponentBase* comp = cluster->GetAttachedComponent(j);
				if( comp )
				{
					comp->DeleteTarget(cluster);
				}
			}
			// END HACK FISH

			cluster->Delete(TimeValue(0), true);
		}

		if( !(i & maskfreq) )
			bar.Update(nil);
	}

	fCompPB->ZeroCount(kClusters);
	fCompPB->ZeroCount(kFadeIns);
	fCompPB->ZeroCount(kFadeOuts);
	fCompPB->ZeroCount(kWindBones);

	GetCOREInterface()->EnableSceneRedraw();

	GetCOREInterface()->ForceCompleteRedraw(FALSE);
}

BOOL plClusterComponent::Cluster(plErrorMsg* pErrMsg)
{
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime(), REDRAW_BEGIN);

	Clear();

	IGetLocation();

	ICheckWindBone();

//	fClusterSize = fCompPB->GetFloat(kClusterSize);
//	fClusterSize = 75.f;

	plExportProgressBar bar;

	IBuildDistribTab();

	INodeTab doneNodes;
	plBox3Tab fade;
	INodeTab bone;
	hsBitVector boneIsParent;
	hsBitVector doneBits;

	plDistribInstTab nodes;

	BOOL failed = true;
	if( IBuildNodeTab(nodes, pErrMsg, bar) )
	{
		failed = false;

		if( nodes.Count() )
		{
			bar.Start("Optimizing", nodes.Count());
			bar.Update(nil, 0);
		}

		int i;
		for( i = 0; i < nodes.Count(); i++ )
		{
			if( doneBits.IsBitSet(i) )
				continue;

			if( !nodes[i].fNode )
				continue;

			if( !ICanCluster(nodes[i]) )
				continue;

			plDistribInstTab shared;
			shared.Append(1, &nodes[i]);
			int j;
			for( j = 0; j < nodes.Count(); j++ )
			{
				if( !doneBits.IsBitSet(j) && ICanCluster(nodes[i], nodes[j]) )
				{
					shared.Append(1, &nodes[j]);
					doneBits.SetBit(j);
				}
			}
			INodeTab cluster;
			failed = !IClusterGroup(shared, cluster, bar);
			if( cluster.Count() )
			{
				int j;
				for( j = 0; j < cluster.Count(); j++ )
				{
					fade.Append(1, &shared[0].fFade);
					bone.Append(1, &shared[0].fBone);
					boneIsParent.SetBit(doneNodes.Count() + j, shared[0].fRigid);
					
					// Attach every component on the template node to the new node
					int k;
					plMaxNode *maxNode = (plMaxNode*)nodes[i].fNode;
					for (k = 0; k < maxNode->NumAttachedComponents(); k++)
					{
						plComponentBase *comp = maxNode->GetAttachedComponent(k);
						comp->AddTarget((plMaxNode*)cluster[j]);
					}
				}

				doneNodes.Append(cluster.Count(), &cluster[0]);

			}
			if( failed )
				break;
		}

	}

	IClearNodeTab();

	IClearDistribTab();

	IFinishDoneNodes(doneNodes, fade, bone, boneIsParent);

	if( failed )
		Clear();
	else
		Select();

	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime(), REDRAW_END);

	return failed;
}

void plClusterComponent::IFinishDoneNodes(INodeTab& doneNodes, plBox3Tab& fade, INodeTab& bones, hsBitVector& boneIsParent)
{
	if( !doneNodes.Count() )
		return;

	NameMaker *nn = GetCOREInterface()->NewNameMaker();

	TSTR nodeName(GetINode()->GetName());
	int i;
	for( i = 0; i < doneNodes.Count(); i++ )
	{
		if( doneNodes[i] )
		{
			nn->MakeUniqueName(nodeName);
			doneNodes[i]->SetName(nodeName);

			ISetLocation((plMaxNode*)doneNodes[i]);
		}
		Point3* p3p;
		p3p = &fade[i].pmin;
		fCompPB->Append(kFadeIns, 1, &p3p);
		p3p = &fade[i].pmax;
		fCompPB->Append(kFadeOuts, 1, &p3p);

		INode* nilNode = nil;
		if( bones[i] )
		{
			if( boneIsParent.IsBitSet(i) )
			{
		
				bones[i]->AttachChild(doneNodes[i], true);
				fCompPB->Append(kWindBones, 1, &nilNode);
			}
			else
			{
				fCompPB->Append(kWindBones, 1, &bones[i]);
			}
		}
		else
		{
			fCompPB->Append(kWindBones, 1, &nilNode);
		}
	}

	// Add doneNodes to our PB, so we can keep track of who we've created.
	fCompPB->Append(kClusters, doneNodes.Count(), &doneNodes[0]);
}

void plClusterComponent::ISetupRenderDependencies()
{
	hsRadixSort::Elem* listTrav;
	hsTArray<hsRadixSortElem> scratchList;

	int numClust = fCompPB->Count(kClusters);

	if( !numClust )
		return;

	scratchList.SetCount(numClust);

	int i;
	for( i = 0; i < numClust; i++ )
	{
		listTrav = &scratchList[i];
		listTrav->fBody = (void*)i;
		listTrav->fNext = listTrav+1;

		Point3 fadeMax = fCompPB->GetPoint3(kFadeOuts, TimeValue(0), i);
		listTrav->fKey.fFloat = fadeMax[2] > 0 ? -fadeMax[0] : -1.e33f; // Negate the distance to get decreasing sort.
	}
	listTrav->fNext = nil;

	hsRadixSort rad;
	hsRadixSort::Elem* sortedList = rad.Sort(scratchList.AcquireArray(), hsRadixSort::kFloat);

	hsRadixSort::Elem* prevStart = nil;
	hsRadixSort::Elem* prevEnd = nil;

	listTrav = sortedList;

	float currFade = listTrav->fKey.fFloat;
	listTrav = listTrav->fNext;

	while( listTrav )
	{
		if( listTrav->fKey.fFloat != currFade )
		{
			IAssignRenderDependencies(prevStart, prevEnd, sortedList, listTrav);

			currFade = listTrav->fKey.fFloat;
		}
		
		listTrav = listTrav->fNext;
	}
	IAssignRenderDependencies(prevStart, prevEnd, sortedList, listTrav);


	// Sort them by fade.Min()[2] < 0 ? fade.Min()[0] : 0 in decreasing order
	// make first sort value group render dependent on all targets
	// for each remaining sort value group
	//		make render dependent on members of previous sort value group.
}

void plClusterComponent::IAssignRenderDependencies(hsRadixSortElem*& prevStart, hsRadixSortElem*& prevEnd,
												   hsRadixSortElem*& currStart, hsRadixSortElem*& currEnd)
{
	if( !prevStart )
	{
		hsRadixSort::Elem* q;
		for( q = currStart; q != currEnd; q = q->fNext )
		{
			int iNode = (int)q->fBody;
			plMaxNodeBase* clust = (plMaxNodeBase*)fCompPB->GetINode(kClusters, TimeValue(0), iNode);

			if( clust )
			{
				int i;
				for( i = 0; i < NumTargets(); i++ )
				{
					plMaxNodeBase* targ = GetTarget(i);
					if( targ )
						clust->AddRenderDependency(targ);
				}
			}
		}
	}
	else
	{
		hsRadixSort::Elem* q;
		for( q = currStart; q != currEnd; q = q->fNext )
		{
			int iNode = (int)q->fBody;
			plMaxNodeBase* clust = (plMaxNodeBase*)fCompPB->GetINode(kClusters, TimeValue(0), iNode);

			if( clust )
			{
#if 0
				hsRadixSort::Elem* p;
				for( p = prevStart; p != prevEnd; p = p->fNext )
				{
					iNode = (int)p->fBody;
					plMaxNodeBase* targ = (plMaxNodeBase*)fCompPB->GetINode(kClusters, TimeValue(0), iNode);
					clust->AddRenderDependency(targ);
				}
#else
				iNode = (int)prevStart->fBody;
				plMaxNodeBase* targ = (plMaxNodeBase*)fCompPB->GetINode(kClusters, TimeValue(0), iNode);
				clust->AddRenderDependency(targ);
#endif
			}
		}
	}
	prevStart = currStart;
	prevEnd = currEnd;
	currStart = currEnd;
}

BOOL plClusterComponent::ICanCluster(plDistribInstance& node)
{
	if( !node.fNode )
		return false;

	return true;
}

BOOL plClusterComponent::ICanCluster(plDistribInstance& node0, plDistribInstance& node1)
{
	if( !(node0.fNode && node1.fNode) )
		return false;

	if( !ICanCluster(node1) )
		return false;

	if( node0.fNode->GetMtl() != node1.fNode->GetMtl() )
		return false;

	if( (node0.fFade.Min() != node1.fFade.Min())
		||(node0.fFade.Max() != node1.fFade.Max()) )
		return false;

	if( node0.fBone != node1.fBone )
		return false;

	return true;
}

Box3 plClusterComponent::IPartition(plDistribInstTab& nodes)
{
	if( !nodes.Count() )
		return Box3();
	
	Box3 retVal;
	int i;
	for( i = 0; i < nodes.Count(); i++ )
	{
		retVal += nodes[i].fNodeTM.GetTrans();
	}
	Point3 mins = retVal.Min();
	Point3 maxs = retVal.Max();

//	mins += Point3(fClusterSize, fClusterSize, fClusterSize) * 0.5f;
//	maxs -= Point3(fClusterSize, fClusterSize, fClusterSize) * 1.5f;
//	maxs -= Point3(fClusterSize, fClusterSize, fClusterSize) * 1.0f;

	for( i = 0; i < 3; i++ )
	{
		if( mins[i] >= maxs[i] )
		{
			float mid = (mins[i] + maxs[i]) * 0.5f;
			mins[i] = mid - 1.f;
			maxs[i] = mid + 1.f;
		}
	}
	retVal = Box3(mins, maxs);

	return retVal;
}

void plClusterComponent::IClusterBins(plDistribInstTab& nodes, Box3& box)
{
	int i;
	for( i = 0; i < 3; i++ )
	{
		fSizes[i] = int((box.Max()[i] - box.Min()[i]) / fClusterSize);
		if( !fSizes[i] )
			fSizes[i] = 1;
	}

	int totSize = IGetBinCount();
	fClusterBins = TRACKED_NEW plDistribInstTab*[totSize];

	memset(fClusterBins, 0, sizeof(*fClusterBins) * totSize);

	for( i = 0; i < nodes.Count(); i++ )
	{
		Matrix3 l2w = nodes[i].fNodeTM;
		Point3 loc = l2w.GetTrans();

		plDistribInstTab* bin = IGetClusterBin(box, loc);
		bin->Append(1, &nodes[i]);
	}
}

int plClusterComponent::IGetBinCount()
{
	return fSizes[0] * fSizes[1] * fSizes[2];
}

void plClusterComponent::IDeleteClusterBins()
{
	int totSize = IGetBinCount();
	int i;
	for( i = 0; i < totSize; i++ )
		delete fClusterBins[i];
	delete [] fClusterBins;
	fClusterBins = nil;
}

plDistribInstTab* plClusterComponent::IGetClusterBin(const Box3& box, const Point3& loc)
{
	int coord[3];
	int j;
	for( j = 0; j < 3; j++ )
	{
		coord[j] = int((loc[j] - box.Min()[j]) / fClusterSize);
		if( coord[j] < 0 )
			coord[j] = 0;
		else if( coord[j] >= fSizes[j] )
			coord[j] = fSizes[j] - 1;
	}
	int idx = coord[0] * fSizes[1] * fSizes[2] + coord[1] * fSizes[2] + coord[2];
	if( !fClusterBins[idx] )
		fClusterBins[idx] = TRACKED_NEW plDistribInstTab;
	return fClusterBins[idx];
}

BOOL plClusterComponent::IClusterGroup(plDistribInstTab& nodes, INodeTab& clusters, plExportProgressBar& bar)
{
	BOOL retVal = true;

	hsBitVector doneNodes;

	const float kNoOptClusterSize = 100.f;
	const float kOptClusterSize = 100.f; // 30.f?
	const int kNoOptMaxFaces = 10000;
	const int kOptMaxFaces = 200;

	float optim = fCompPB->GetFloat(kOptimization) * 0.01f;

	float minClusterSize = kNoOptClusterSize + optim * (kOptClusterSize - kNoOptClusterSize);
	int maxFaces = kNoOptMaxFaces + int(optim * float(kOptMaxFaces - kNoOptMaxFaces));

	fClusterSize = minClusterSize;
	Box3 fade = nodes[0].fFade;
	if( fade.Min().z < 0 )
	{
		fClusterSize = fade.Min().x;
	}
	else if( fade.Max().z < 0 )
	{
		fClusterSize = fade.Max().x;
	}
	if( fClusterSize < minClusterSize )
		fClusterSize = minClusterSize;

	Box3 box = IPartition(nodes);

	IClusterBins(nodes, box);

	int totSize = IGetBinCount();
	int i;
	for( i = 0; i < totSize; i++ )
	{
		if( fClusterBins[i] )
		{
			INode* grp = IMakeOne(*fClusterBins[i]);
			if( grp )
			{
				INodeTab subGrp;
				plDicer dicer;
				dicer.SetMaxFaces(maxFaces);
				dicer.Dice(grp, subGrp);
				int j;
				for( j = 0; j < subGrp.Count(); j++ )
				{
					clusters.Append(1, &subGrp[j]);
				}
			}

			if( bar.Update(nil, fClusterBins[i]->Count()) )
			{
				retVal = false;
				break;
			}
		}
	}
	IDeleteClusterBins();

	return retVal;
}

INode* plClusterComponent::IMakeOne(plDistribInstTab& nodes)
{
	if( !nodes.Count() )
		return nil;

	TriObject* triObj = CreateNewTriObject();
	Mesh* outMesh = &triObj->mesh;

	*outMesh = *nodes[0].fMesh;

	INode *outNode = GetCOREInterface()->CreateObjectNode(triObj);

	Matrix3 l2w = nodes[0].fObjectTM;
	Matrix3 w2l = Inverse(l2w);

	MeshDelta meshDelta(*outMesh);

	int i;
	for( i = 1; i < nodes.Count(); i++ )
	{
		Mesh nextMesh(*nodes[i].fMesh);

		Matrix3 relativeTransform = nodes[i].fObjectTM * w2l;

		// If we've stashed normals on this mesh, they are in the mesh's
		// native local space. The transform of the positions is handled
		// automatically by meshDelta.AttachMesh (hence passing in the matrix),
		// but the meshDelta hasn't a clue that the normal map channel isn't
		// just more UVs. No problem, I'll handle it myself.
		if( nextMesh.mapVerts(plDistributor::kNormMapChan) )
		{
			Point3* norms = nextMesh.mapVerts(plDistributor::kNormMapChan);
			int k;
			for( k = 0; k < nextMesh.getNumMapVerts(plDistributor::kNormMapChan); k++ )
			{
				norms[k] = relativeTransform.VectorTransform(norms[k]);
			}
		}

		IRandomizeSkinWeights(&nextMesh, nodes[i].fFlex);

		meshDelta.AttachMesh(*outMesh, nextMesh, relativeTransform, 0);

		meshDelta.Apply(*outMesh);

	}

	outNode->SetNodeTM(TimeValue(0), l2w);
	outNode->CopyProperties(nodes[0].fNode);
	outNode->SetMtl(nodes[0].fNode->GetMtl());
	outNode->SetObjOffsetPos(Point3(0,0,0));
	Quat identQuat;
	identQuat.Identity();
	outNode->SetObjOffsetRot(identQuat);
	outNode->SetObjOffsetScale(ScaleValue(Point3(1.f, 1.f, 1.f)));

	outNode->Hide(false);

	return outNode;
}

BOOL plClusterComponent::IGetLocation()
{
	fLocationComp = nil;
	int numTarg = NumTargets();
	int i;
	for( i = 0; i < numTarg; i++ )
	{
		plMaxNodeBase* targ = GetTarget(i);
		if( targ )
		{
			UInt32 numComp = targ->NumAttachedComponents(false);
			int j;
			for( j = 0; j < numComp; j++ )
			{
				plComponentBase* comp = targ->GetAttachedComponent(j, false);
				if( comp && (comp->ClassID() == ROOM_CID || comp->ClassID() == PAGEINFO_CID) )
				{
					if( fLocationComp && (fLocationComp != comp) )
					{
						fLocationComp = nil;
						return false;
					}
					fLocationComp = comp;
				}
			}
		}
	}
	return fLocationComp != nil;
}

void plClusterComponent::ISetLocation(plMaxNode* node)
{
	if( fLocationComp )
		fLocationComp->AddTarget(node);
}

void plClusterComponent::IRandomizeSkinWeights(Mesh* mesh, const Point3& flex) const
{
	const int iWgtMap = plDistributor::kWgtMapChan;

	UVVert *wgtMap = mesh->mapVerts(iWgtMap);	
	int numWgtVerts = mesh->getNumMapVerts(iWgtMap);

	static plRandom random;

	float interMeshRandomNess = flex[1];
	float intraMeshRandomNess = flex[2];

	float r = interMeshRandomNess > 0 ? random.RandRangeF(1.f - interMeshRandomNess, 1.f) : 1.f;
	int i;
	for( i = 0; i < numWgtVerts; i++ )
	{
		UVVert uvw = wgtMap[i];
		float s = r;
		if( intraMeshRandomNess > 0 )
			s *= random.RandRangeF(1.f - intraMeshRandomNess, 1.f);
		uvw *= s;
		mesh->setMapVert(iWgtMap, i, uvw);
	}
}

