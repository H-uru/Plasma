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

#include "hsTypes.h"
#include "plSpaceTreeMaker.h"
#include "../plMath/hsRadixSort.h"
#include "../plDrawable/plSpaceTree.h"

#include "hsUtils.h" // for testing, get hsRand()
#include "hsTimer.h"
#include "../plIntersect/plVolumeIsect.h"

//#define MF_DO_TIMES

enum mfTimeTypes
{
	kMakeTree = 0,
	kMakeFatTree,
	kSortList,
	kHarvest,
	kMakeSpaceTree,
	kMakeTreeAll,
	kHarvestSphere,
	kHarvestCone,
	kHarvestCapped,

	kNumTimes
};

#ifdef MF_DO_TIMES

double times[kNumTimes];
#define StartTimer(i) { times[(i)] -= hsTimer::GetSeconds(); }
#define StopTimer(i) { times[(i)] += hsTimer::GetSeconds(); }

#define InitTimers() { for( int i = 0; i < kNumTimes; i++ )times[i] = 0; }

#else // MF_DO_TIMES

#define StartTimer(i)
#define StopTimer(i)
#define InitTimers()

#endif // MF_DO_TIMES

// Create the tree

// more temp testing garbage
#if 0
plSpaceCullResult mySpaceCullFunction(const hsBounds3Ext& bnd)
{
	const hsPoint3& maxs = bnd.GetMaxs();
	const hsPoint3& mins = bnd.GetMins();

	if( maxs.fX < 0.25f )
		return kSpaceCulled;
	if( maxs.fY < 0.25f )
		return kSpaceCulled;
	if( maxs.fZ < 0.25f )
		return kSpaceCulled;

	if( mins.fX > 0.75f )
		return kSpaceCulled;
	if( mins.fY > 0.75f )
		return kSpaceCulled;
	if( mins.fZ > 0.75f )
		return kSpaceCulled;

	if( maxs.fX > 0.75f )
		return kSpaceSplit;
	if( maxs.fY > 0.75f )
		return kSpaceSplit;
	if( maxs.fZ > 0.75f )
		return kSpaceSplit;

	if( mins.fX < 0.25f )
		return kSpaceSplit;
	if( mins.fY < 0.25f )
		return kSpaceSplit;
	if( mins.fZ < 0.25f )
		return kSpaceSplit;

	return kSpaceClear;
}
#endif

void plSpaceTreeMaker::ISortList(hsTArray<plSpacePrepNode*>& nodes, const hsVector3& axis)
{
	StartTimer(kSortList);

	hsRadixSort::Elem*	list = fSortScratch;
	hsRadixSort::Elem* listTrav = list;
	Int32 n = nodes.GetCount();
	while( n-- )
	{
		listTrav->fKey.fFloat = axis.InnerProduct(nodes[n]->fWorldBounds.GetCenter());
		listTrav->fBody = (void*)nodes[n];
		listTrav->fNext = listTrav+1;
		listTrav++;
	}
	list[nodes.GetCount()-1].fNext = nil;
	UInt32 sortFlags = 0;
	hsRadixSort rad;
	hsRadixSort::Elem* sortedList = rad.Sort(list, sortFlags);
	listTrav = sortedList;

	int i;
	for( i = 0; i < nodes.GetCount(); i++ )
	{
		nodes[i] = (plSpacePrepNode*)(listTrav->fBody);
		listTrav = listTrav->fNext;
	}

	StopTimer(kSortList);

	return;
}

void plSpaceTreeMaker::ISplitList(hsTArray<plSpacePrepNode*>& nodes, const hsVector3& axis, hsTArray<plSpacePrepNode*>& lower, hsTArray<plSpacePrepNode*>& upper)
{

	ISortList(nodes, axis);

	int lowerCount = nodes.GetCount() / 2;
	int upperCount = nodes.GetCount() - lowerCount;
	lower.SetCount(lowerCount);
	upper.SetCount(upperCount);

	int i;
	for( i = 0; i < lowerCount; i++ )
		lower[i] = nodes[i];
	for( i = 0; i < upperCount; i++ )
		upper[i] = nodes[i + lowerCount];
}

hsBounds3Ext plSpaceTreeMaker::IFindDistToCenterAxis(hsTArray<plSpacePrepNode*>& nodes, hsScalar& length, hsVector3& axis)
{
	hsBounds3Ext bnd;
	bnd.MakeEmpty();

	hsAssert(nodes.GetCount() > 1, "Degenerate case");

	int i;
	for( i = 0; i < nodes.GetCount(); i++ )
	{
		bnd.Union(&nodes[i]->fWorldBounds);
	}
	length = 0;
	for( i = 0; i < nodes.GetCount(); i++ )
	{
		hsVector3 sep;
		sep.Set(&bnd.GetCenter(), &nodes[i]->fWorldBounds.GetCenter());
		hsScalar len = sep.MagnitudeSquared();
		if( len > length )
		{
			axis = sep;
			length = len;
		}
	}
	length = hsSquareRoot(length);
	if( length > 1.e-3f )
		axis /= length;
	else
		return IFindSplitAxis(nodes, length, axis);

	return bnd;
}

plSpacePrepNode* plSpaceTreeMaker::IMakeFatTreeRecur(hsTArray<plSpacePrepNode*>& nodes)
{
	if( !nodes.GetCount() )
		return nil;

	StartTimer(kMakeFatTree);

	plSpacePrepNode* subRoot = TRACKED_NEW plSpacePrepNode;
	fTreeSize++;

	if( nodes.GetCount() == 1 )
	{
		*subRoot = *nodes[0];

		subRoot->fChildren[0] = nil;
		subRoot->fChildren[1] = nil;

		StopTimer(kMakeFatTree);

		return subRoot;
	}

	// Find the overall bounds of the list.
	
	// Find the maximum length vector from nodes[i] center to list center.
	// If that length is zero, just use the maximum dimension of overall bounds.
	hsScalar length;
	hsVector3 axis;
	hsBounds3Ext bnd = IFindDistToCenterAxis(nodes, length, axis);

	hsTArray<plSpacePrepNode*> list0;
	hsTArray<plSpacePrepNode*> list1;
	ISplitList(nodes, axis, list0, list1);

	subRoot->fChildren[0] = IMakeTreeRecur(list0);
	subRoot->fChildren[1] = IMakeTreeRecur(list1);

	subRoot->fWorldBounds = bnd;

	StopTimer(kMakeFatTree);

	return subRoot;
}

hsBounds3Ext plSpaceTreeMaker::IFindSplitAxis(hsTArray<plSpacePrepNode*>& nodes, hsScalar& length, hsVector3& axis)
{
	hsBounds3Ext bnd;
	bnd.MakeEmpty();
	int i;
	for( i = 0; i < nodes.GetCount(); i++ )
	{
		bnd.Union(&nodes[i]->fWorldBounds);
	}
	hsScalar maxLen = bnd.GetMaxs()[0] - bnd.GetMins()[0];
	int maxAxis = 0;

	if( bnd.GetMaxs()[1] - bnd.GetMins()[1] > maxLen )
	{
		maxLen = bnd.GetMaxs()[1] - bnd.GetMins()[1];
		maxAxis = 1;
	}

	if( bnd.GetMaxs()[2] - bnd.GetMins()[2] > maxLen )
	{
		maxLen = bnd.GetMaxs()[2] - bnd.GetMins()[2];
		maxAxis = 2;
	}

	length = maxLen;
	switch( maxAxis )
	{
	case 0:
		axis.Set(1.f, 0, 0);
		break;
	case 1:
		axis.Set(0, 1.f, 0);
		break;
	case 2:
		axis.Set(0, 0, 1.f);
		break;
	}

	return bnd;
}

void plSpaceTreeMaker::IFindBigList(hsTArray<plSpacePrepNode*>& nodes, hsScalar length, const hsVector3& axis, hsTArray<plSpacePrepNode*>& giants, hsTArray<plSpacePrepNode*>& strimps)
{
	const hsScalar kCutoffFrac = 0.5f;

	giants.SetCount(0);
	strimps.SetCount(0);
	int i;
	for( i = 0; i < nodes.GetCount(); i++ )
	{
		hsPoint2 depth;
		nodes[i]->fWorldBounds.TestPlane(axis, depth);
		if( depth.fY - depth.fX > length * kCutoffFrac )
			giants.Append(nodes[i]);
		else
			strimps.Append(nodes[i]);
	}
}

plSpacePrepNode* plSpaceTreeMaker::INewSubRoot(const hsBounds3Ext& bnd)
{
	plSpacePrepNode* subRoot = TRACKED_NEW plSpacePrepNode;
	subRoot->fDataIndex = Int16(-1);
	fTreeSize++;

	subRoot->fWorldBounds = bnd;

	return subRoot;
}

plSpacePrepNode* plSpaceTreeMaker::IMakeTreeRecur(hsTArray<plSpacePrepNode*>& nodes)
{
	if( !nodes.GetCount() )
		return nil;

	if( nodes.GetCount() == 1 )
	{
		return IMakeFatTreeRecur(nodes);
	}

	StartTimer(kMakeTree);

	// Find the maximum bounds dimension
	hsScalar length;
	hsVector3 axis;
	hsBounds3Ext bnd = IFindSplitAxis(nodes, length, axis);

	// Find everyone with bounds over half that size in the same dimension as list0.
	hsTArray<plSpacePrepNode*> list0;
	hsTArray<plSpacePrepNode*> list1;
	IFindBigList(nodes, length, axis, list0, list1);

	plSpacePrepNode* subRoot = nil;

	// If list0 not empty, put them in first child, recur on remainder,
	if( list0.GetCount() && list1.GetCount() )
	{
		subRoot = INewSubRoot(bnd);
		subRoot->fChildren[0] = IMakeFatTreeRecur(list0); // too big
		subRoot->fChildren[1] = IMakeTreeRecur(list1); // remainder
	}
	else if( list0.GetCount() )
	{
		subRoot = IMakeFatTreeRecur(list0);
	}
	// Else sort along axis by bounds center, recur separately on lower and upper halves.
	else
	{
		ISplitList(nodes, axis, list0, list1);

		subRoot = INewSubRoot(bnd);
		subRoot->fChildren[0] = IMakeTreeRecur(list0);
		subRoot->fChildren[1] = IMakeTreeRecur(list1);

	}

	StopTimer(kMakeTree);

	return subRoot;
}

void plSpaceTreeMaker::IMakeTree()
{
	fSortScratch = TRACKED_NEW hsRadixSort::Elem[fLeaves.GetCount()];

	fPrepTree = IMakeTreeRecur(fLeaves);

	delete [] fSortScratch;
	fSortScratch = nil;
}

void plSpaceTreeMaker::Reset()
{
	fLeaves.Reset();
	fPrepTree = nil;
	fTreeSize = 0;
	fSortScratch = nil;
}

void plSpaceTreeMaker::IDeleteTreeRecur(plSpacePrepNode* node)
{
	if( node )
	{
		IDeleteTreeRecur(node->fChildren[0]);
		IDeleteTreeRecur(node->fChildren[1]);

		delete node;
	}
}

void plSpaceTreeMaker::Cleanup()
{
	IDeleteTreeRecur(fPrepTree);
	fPrepTree = nil;

	int i;
	for( i = 0; i < fLeaves.GetCount(); i++ )
		delete fLeaves[i];
	fLeaves.Reset();
	fDisabled.Reset();
}

Int32 plSpaceTreeMaker::AddLeaf(const hsBounds3Ext& worldBnd, hsBool disable)
{
	plSpacePrepNode* leaf = TRACKED_NEW plSpacePrepNode;
	fLeaves.Append(leaf);
	leaf->fDataIndex = fLeaves.GetCount()-1;
	leaf->fChildren[0] = nil;
	leaf->fChildren[1] = nil;
	
	leaf->fWorldBounds = worldBnd;
	if( leaf->fWorldBounds.GetType() != kBoundsNormal )
	{
		static const hsPoint3 zero(0.f, 0.f, 0.f);
		leaf->fWorldBounds.Reset(&zero);
	}

	fDisabled.SetBit(leaf->fDataIndex, disable);

	return leaf->fDataIndex;
}

//#define MF_DO_RAND
#define MF_DO_3D

#ifdef MF_DO_RAND
#define MF_SETPOINT(pt,a,b,c) pt.Set(hsRand()/32767.f, hsRand()/32767.f, hsRand()/32767.f)
#else // MF_DO_RAND
#define MF_SETPOINT(pt,a,b,c) pt.Set(a,b,c)
#endif // MF_DO_RAND
void plSpaceTreeMaker::TestTree()
{
	Reset();

	const int kTestSize = 10;
	int i;
	for( i = 0; i < kTestSize; i++ )
	{
		int j;
		for( j = 0; j < kTestSize; j++ )
		{
			int k;
#ifdef MF_DO_3D
			for( k = 0; k < kTestSize; k++ )
#else // MF_DO_3D
			k = 0;
#endif // MF_DO_3D
			{
				hsBounds3Ext bnd;
				hsPoint3 pt;
				MF_SETPOINT(pt, float(i-1)/kTestSize, float(j-1)/kTestSize, float(k-1)/kTestSize);
				bnd.Reset(&pt);
				MF_SETPOINT(pt, float(i)/kTestSize, float(j)/kTestSize, float(k)/kTestSize);
				bnd.Union(&pt);

				AddLeaf(bnd);
			}
		}
	}

	hsBitVector list;

	plSpaceTree* tree = MakeTree();

#if 0 // HACK TESTING MOVE TO VOLUMECULL

	hsMatrix44 liX;
	hsMatrix44 invLiX;
	liX.MakeTranslateMat(&hsVector3(0.5f, 0.5f, 0));
	liX.GetInverse(&invLiX);

	plSphereIsect sphere;
	sphere.SetRadius(0.2);
	sphere.SetTransform(liX, invLiX);
	
	tree->SetViewPos(*hsPoint3().Set(0,0,0));

	plConeIsect cone;
	cone.SetAngle(hsScalarPI*0.25f);
	cone.SetTransform(liX, invLiX);

	StartTimer(kHarvestCone);

	list.Clear();
	tree->HarvestLeaves(&cone, list);

	StopTimer(kHarvestCone);

	plConeIsect capped;
	capped.SetAngle(hsScalarPI*0.25f);
	capped.SetLength(0.5f);
	capped.SetTransform(liX, invLiX);

	StartTimer(kHarvestCapped);

	list.Clear();
	tree->HarvestLeaves(&capped, list);

	StopTimer(kHarvestCapped);

	StartTimer(kHarvestSphere);

	list.Clear();
	tree->HarvestLeaves(&sphere, list);

	StopTimer(kHarvestSphere);

#endif // HACK TESTING MOVE TO VOLUMECULL

	delete tree;
}

plSpaceTree* plSpaceTreeMaker::MakeTree()
{
	// DEBUG FISH
	InitTimers();
	// DEBUG FISH

	StartTimer(kMakeTreeAll);

	if( !fLeaves.GetCount() )
		return IMakeEmptyTree();

	if( fLeaves.GetCount() < 2 )
		return IMakeDegenerateTree();

	IMakeTree();

	plSpaceTree* retVal = IMakeSpaceTree();

	Cleanup();

	StopTimer(kMakeTreeAll);

	return retVal;
}

plSpaceTree* plSpaceTreeMaker::IMakeEmptyTree()
{
	plSpaceTree* tree = TRACKED_NEW plSpaceTree;

	tree->fTree.SetCount(1);
	tree->fTree[0].fWorldBounds.Reset(&hsPoint3(0,0,0));
	tree->fTree[0].fFlags = plSpaceTreeNode::kEmpty;
	tree->fRoot = 0;
	tree->fNumLeaves = 0;

	Cleanup();

	return tree;
}

plSpaceTree* plSpaceTreeMaker::IMakeDegenerateTree()
{
	plSpaceTree* tree = TRACKED_NEW plSpaceTree;
	
	tree->fTree.Push();

	tree->fRoot = 0;
	tree->fTree[0].fWorldBounds = fLeaves[0]->fWorldBounds;
	tree->fTree[0].fFlags = plSpaceTreeNode::kIsLeaf;
	tree->fTree[0].fLeafIndex = 0;
	tree->fTree[0].fParent = plSpaceTree::kRootParent;
	tree->fNumLeaves = 1;

	if( fDisabled.IsBitSet(0) )
		tree->SetLeafFlag(0, plSpaceTreeNode::kDisabled, true);

	Cleanup();

	return tree;
}

int plSpaceTreeMaker::ITreeDepth(plSpacePrepNode* subRoot)
{
	if( !subRoot )
		return 0;

	int dep0 = ITreeDepth(subRoot->fChildren[0]);
	int dep1 = ITreeDepth(subRoot->fChildren[1]);

	int dep = hsMaximum(dep0, dep1);

	return dep+1;
}

plSpaceTree* plSpaceTreeMaker::IMakeSpaceTree()
{
	StartTimer(kMakeSpaceTree);

	plSpaceTree* tree = TRACKED_NEW plSpaceTree;

	plSpacePrepNode* head = fPrepTree;

	tree->fTree.SetCount(fLeaves.GetCount());
	
	IGatherLeavesRecur(head, tree);
	
	int level = ITreeDepth(head);
	while( level > 0 )
		IMakeSpaceTreeRecur(head, tree, --level, 0);

	tree->fRoot = tree->fTree.GetCount()-1;
	tree->fTree[tree->fRoot].fParent = plSpaceTree::kRootParent;
	tree->fNumLeaves = fLeaves.GetCount();

	int i;
	for( i = 0; i < fLeaves.GetCount(); i++ )
	{
		if( fDisabled.IsBitSet(i) )
			tree->SetLeafFlag(i, plSpaceTreeNode::kDisabled, true);
	}

	StopTimer(kMakeSpaceTree);

	return tree;
}

// The following goofy cache-friendly tree set up slows down the tree build by 10%, but speeds up the runtime by 9%. 
// Sounds fair.
#if 0 // Leaves first
Int16 plSpaceTreeMaker::IMakeSpaceTreeRecur(plSpacePrepNode* sub, plSpaceTree* tree, const int targetLevel, int currLevel)
{
	if( currLevel == targetLevel )
	{
		Int16 nodeIdx = tree->fTree.GetCount();
		tree->fTree.Push();

		tree->fTree[nodeIdx].fWorldBounds = sub->fWorldBounds;

		sub->fIndex = nodeIdx;

		if( !sub->fChildren[0] )
		{
			hsAssert(!sub->fChildren[1], "Unsupported unbalance of tree");

			tree->fTree[nodeIdx].fFlags = plSpaceTreeNode::kIsLeaf;
			tree->fTree[nodeIdx].fLeafIndex = sub->fDataIndex;

			return nodeIdx;
		}
		hsAssert(sub->fChildren[1], "Unsupported unbalance of tree");

		tree->fTree[nodeIdx].fFlags = plSpaceTreeNode::kNone;


		return nodeIdx;
	}

	Int16 nodeIdx = sub->fIndex;

	if( !sub->fChildren[0] )
	{
		hsAssert(!sub->fChildren[1] , "Unsupported unbalance of tree");
		return nodeIdx;
	}
	hsAssert(sub->fChildren[1] , "Unsupported unbalance of tree");

	tree->fTree[nodeIdx].fChildren[0] = IMakeSpaceTreeRecur(sub->fChildren[0], tree, targetLevel, currLevel+1);
	tree->fTree[tree->fTree[nodeIdx].fChildren[0]].fParent = nodeIdx;

	tree->fTree[nodeIdx].fChildren[1] = IMakeSpaceTreeRecur(sub->fChildren[1], tree, targetLevel, currLevel+1);
	tree->fTree[tree->fTree[nodeIdx].fChildren[1]].fParent = nodeIdx;

	return nodeIdx;
}

#else // Leaves first
void plSpaceTreeMaker::IGatherLeavesRecur(plSpacePrepNode* sub, plSpaceTree* tree)
{
	// if it's a leaf, stuff it in the right slot, else recur
	if( !sub->fChildren[0] )
	{
		hsAssert(!sub->fChildren[1], "Unsupported unbalance of tree");
		
		plSpaceTreeNode& leaf = tree->fTree[sub->fDataIndex];
		Int16 nodeIdx = sub->fDataIndex;
		leaf.fWorldBounds = sub->fWorldBounds;
		sub->fIndex = nodeIdx;
		leaf.fFlags = plSpaceTreeNode::kIsLeaf;
		leaf.fLeafIndex = nodeIdx;
		
		return;
	}
	hsAssert(sub->fChildren[1], "Unsupported unbalance of tree");

	IGatherLeavesRecur(sub->fChildren[0], tree);
	IGatherLeavesRecur(sub->fChildren[1], tree);
}

void plSpaceTreeMaker::IMakeSpaceTreeRecur(plSpacePrepNode* sub, plSpaceTree* tree, const int targetLevel, int currLevel)
{
	// If it's a leaf, we've already done it.
	if( !sub->fChildren[0] )
	{
		hsAssert(!sub->fChildren[1], "Unsupported unbalance of tree");
		return;
	}

	hsAssert(sub->fChildren[0] && sub->fChildren[1], "Shouldn't get this deep, already got the leaves");

	if( currLevel == targetLevel )
	{
		Int16 nodeIdx = tree->fTree.GetCount();
		tree->fTree.Push();

		tree->fTree[nodeIdx].fWorldBounds = sub->fWorldBounds;

		sub->fIndex = nodeIdx;

		tree->fTree[nodeIdx].fFlags = plSpaceTreeNode::kNone;

		tree->fTree[nodeIdx].fChildren[0] = sub->fChildren[0]->fIndex;
		tree->fTree[sub->fChildren[0]->fIndex].fParent = nodeIdx;
		
		tree->fTree[nodeIdx].fChildren[1] = sub->fChildren[1]->fIndex;
		tree->fTree[sub->fChildren[1]->fIndex].fParent = nodeIdx;

		return;
	}

	IMakeSpaceTreeRecur(sub->fChildren[0], tree, targetLevel, currLevel+1);
	IMakeSpaceTreeRecur(sub->fChildren[1], tree, targetLevel, currLevel+1);
}

#endif // Leaves first