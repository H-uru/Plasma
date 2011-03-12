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

#ifndef plPageTreeMgr_inc
#define plPageTreeMgr_inc

#include "hsTemplates.h"

class plSceneNode;
class plSpaceTree;
class plPipeline;
class plCullPoly;
class plOccluder;
class plDrawable;
class plDrawVisList;
class plVolumeIsect;
class plVisMgr;

class plDrawSpanPair
{
public:
	plDrawSpanPair() {}
	plDrawSpanPair(UInt16 d, UInt16 s) : fDrawable(d), fSpan(s) {}
	UInt16		fDrawable;
	UInt16		fSpan;
};

class plDrawVisList
{
public:
	plDrawVisList() : fDrawable(nil) {}
	virtual ~plDrawVisList() {}

	plDrawable*			fDrawable;
	hsTArray<Int16>		fVisList;

	plDrawVisList& operator=(const plDrawVisList& v) { fDrawable = v.fDrawable; fVisList = v.fVisList; return *this; }
};

class plPageTreeMgr
{
protected:
	hsTArray<plSceneNode*>		fNodes;

	plSpaceTree*				fSpaceTree;
	plVisMgr*					fVisMgr;

	static hsBool				fDisableVisMgr;

	hsTArray<const plOccluder*>	fOccluders;
	hsTArray<const plCullPoly*>	fCullPolys;
	hsTArray<const plCullPoly*>	fSortedCullPolys;

	void						ITrashSpaceTree();
	hsBool						IBuildSpaceTree();
	hsBool						IRefreshTree(plPipeline* pipe);
	void						ISortCullPolys(plPipeline* pipe);
	hsBool						IGetOcclusion(plPipeline* pipe, hsTArray<Int16>& list);
	hsBool						IGetCullPolys(plPipeline* pipe);
	void						IResetOcclusion(plPipeline* pipe);
	void						IAddCullPolyList(const hsTArray<plCullPoly>& polyList);

	hsBool						ISortByLevel(plPipeline* pipe, hsTArray<plDrawVisList>& drawList, hsTArray<plDrawVisList>& sortedDrawList);
	int							IPrepForRenderSortingSpans(plPipeline* pipe, hsTArray<plDrawVisList>& drawVis, int& iDrawStart);
	hsBool						IRenderSortingSpans(plPipeline* pipe, hsTArray<plDrawVisList*>& drawList, hsTArray<plDrawSpanPair>& pairs);
	int							IRenderVisList(plPipeline* pipe, hsTArray<plDrawVisList>& visList);

public:
	plPageTreeMgr();
	virtual ~plPageTreeMgr();

	const hsTArray<plSceneNode*>& GetNodes() const { return fNodes; }

	void			AddNode(plSceneNode* node);
	void			RemoveNode(plSceneNode* node);
	virtual void	Reset(); // remove all nodes, nuke the space tree
	virtual hsBool	Empty() const { return !fNodes.GetCount(); }

	virtual int		Render(plPipeline* pipe);

	hsBool			Harvest(plVolumeIsect* isect, hsTArray<plDrawVisList>& levList);

	void			AddOccluderList(const hsTArray<plOccluder*> occList);

	plSpaceTree*	GetSpaceTree() { if( !fSpaceTree ) IBuildSpaceTree(); return fSpaceTree; }

	void			SetVisMgr(plVisMgr* visMgr) { fVisMgr = visMgr; }
	plVisMgr*		GetVisMgr() const { return fVisMgr; }

	static void		EnableVisMgr(hsBool on) { fDisableVisMgr = !on; }
	static hsBool	VisMgrEnabled() { return !fDisableVisMgr; }
};

#endif // plPageTreeMgr_inc
