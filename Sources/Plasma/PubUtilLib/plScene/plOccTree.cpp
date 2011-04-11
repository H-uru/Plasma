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


void plOccTree::AddPoly(plPolygon* poly)
{
	fBasePolys.Append(*poly);
}

void plOccTree::MakeOccTree()
{
	if( !fBasePolys.GetCount() )
		return;

	ISortBasePolys();

	int i;
	for( i = 0; i < fBasePolys.GetCount(); i++ )
		fRoot = IAddPolyRecur(fRoot, fBasePolys[i], false);

	fBasePolys.SetCount(0);
}

plOccNode* poOccTree::IMakeSubTree(plOccPoly* poly)
{
	plOccNode* nextNode = nil;
	plOccNode* lastNode = nil;

	int i;
	for( i = 0; i < poly->fVerts.GetCount(); i++ )
	{
		if( poly->fEdgeFlags[i] & plOccPoly::kEdgeClipped )
			continue;

		nextNode = fNodePool.Append();
		nextNode->fFlags = 0;
		nextNode->fOutChild = nil;

		int j = i+1 < poly->fVerts.GetCount() ? i+1 : 0;

		// Need to set the viewplane here. Calc once per base poly and use
		// that for fragments?
		nextNode->Init(poly->fVerts[i], poly->fVerts[j], fViewPos);

		if( nextNode->fInChild = lastChild )
			nextNode->fFlags = plOccNode::kHasInChild;
		else
		{
			nextNode->fInChild = fNodePool.Append();
			nextNode->fInChild->Init(poly, false);
		}

		lastNode = nextNode;
	}

	// If we have no nextNode, all our edges were clipped. In
	// that case, we'll just return an "out" leaf.
	if( !nextNode )
	{
		nextNode = fNodePool.Append();
		nextNode->fFlags = 0;
		nextNode->fInChild = nextNode->fOutChild = nil;
		nextNode->Init(poly, true);
	}

	return nextNode;
}

void plOccNode::Init(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& pv)
{
	hsVector3 v0, v1;
	v0.Set(&p0, &pv);
	v1.Set(&p1, &pv);

	fPlane.fNormal = v0 % v1;
	fPlane.fDist = fPlane.fNormal.InnerProduct(v0);
}

void plOccNode::Init(plOccPoly* poly)
{
	fPlane = poly->fPlane;
	// set the viewplane
	fFlags = kIsLeaf;
}

// Adding a poly to a node
//		if the node is nil
//			IMakeSubTree(poly) replaces the node
//		else
//		if the node is a leaf
//			pitch the poly
//			return node (no replacement)
//		else
//		if poly is inside the node
//			recur on node's inner child
//			return node (no replacement)
//		else
//		if poly is ouside the node
//			recur on node's outer child
//			return node (no replacement)
//		else (node splits poly)
//			recur on node's inner child
//			recur on node's outer child
//			return node (no replacement)
//		end
//
//		Special case - Degenarate poly's can come
//		from ITestPoly if an edge of the input poly
//		is on the plane. In that case, the function
//		will return kSplit, but either inPoly or outPoly
//		will have no vertices. That degenerate poly,
//		when added to a node, should just be pitched.
//	




// Returns new root, in case it changed.
// This assumes polys are being added front to back.
// This function will break the poly into fragments that fit in the
// current planes within the tree. Planes are added when a final fragment
// is added (in IMakeSubTree).
// We count on ITestPoly to properly mark edges which were created by
// clipping, as those won't generate leaf nodes.
plOccNode* plOccTree::IAddPolyRecur(plOccNode* node, plOccPoly* poly)
{
	if( !poly->fVerts.GetCount() )
		return node;

	if( !node )
	{
		return IMakeSubTree(poly);
	}

	plOccPoly* inPoly = nil;
	plOccPoly* outPoly = nil;


	UInt32 test = ITestPoly(node->fPlane, poly, inPoly, outPoly);

	switch( test )
	{
	case kAllIn:
		node->fInChild = IAddPolyRecur(node->fInChild, poly);
		break;
	case kAllOut:
		node->fOutChild = IAddPolyRecur(node->fOutChild, poly);
		break;
	case kSplit:
		node->fInChild = IAddPolyRecur(node->fInChild, inPoly);
		node->fOutChild = IAddPolyRecur(node->fOutChild, outPoly);
		break;
	};

	return node;
}

hsBool plOccTree::BoundsVisible(const hsBounds3Ext& bnd) const
{
	if( !fRoot )
		return true;

	return fRoot->IBoundsVisible(bnd);
}


hsBool plOccNode::IInChildBoundsVisible(const hsBounds3Ext& bnd) const
{
	return fInChild
			? fInChild->IBoundsVisible(bnd)
			: false;
}

hsBool plOccNode::IOutChildBoundsVisible(const hsBounds3Ext& bnd) const
{
	return fOutChild
			? fOutChild->IBoundsVisible(bnd)
			: true;
}

hsBool plOccNode::IBoundsVisible(const hsBounds3Ext& bnd) const
{
	hsPoint2 depth;
	bnd.TestPlane(fPlane.fNormal, depth);
	if( depth.fX > fPlane.fDist )
	{
		return IOutChildVisible(bnd);
	}
	else if( depth.fY < fPlane.fDist )
	{
		return IInChildVisible(bnd);
	}

	// here's where it gets wierd. we pass the bounds in
	// both directions. if either says it's visible, it's visible.
	// doesn't seem like it would work, but you never know.
	return IOutChildVisible(bnd) || IInChildVisible(bnd);
}