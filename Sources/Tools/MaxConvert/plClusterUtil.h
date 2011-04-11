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

#ifndef plClusterUtil_inc
#define plClusterUtil_inc

#include "hsGeometry3.h"
#include "hsColorRGBA.h"

#include "../plDrawable/plSpanInstance.h"

class plMaxNode;
class plClusterGroup;
class plSpanTemplateB;
class hsBounds3Ext;
class plCluster;
class hsGMaterial;

class plDeformVert
{
public:

	// Begin returns true if it's actually planning to do anything.
	// If it returns false, End will still get called, but GetDel probably won't.
	virtual hsBool Begin(INode* templNode, const hsBounds3Ext& wBnd) { return false; }
	virtual void End() {}

	hsVector3 GetDel(const hsPoint3& p) { return GetDel(Point3(p.fX, p.fY, p.fZ)); }
	virtual hsVector3 GetDel(const Point3& p) { return hsVector3(0,0,0); }
};

class plShadeVert
{
public:
	virtual hsBool Begin(INode* templNode, const hsBounds3Ext& wBnd) { return false; }
	virtual void End() {}

	Color GetShade(const hsPoint3& p, const hsVector3& n) { return GetShade(Point3(p.fX, p.fY, p.fZ), Point3(n.fX, n.fY, n.fZ)); }
	virtual Color GetShade(const Point3& p, const Point3& n) { return Color(0,0,0); }
};

class plL2WTab : public Tab<Matrix3> {};
class plL2WTabTab : public Tab<plL2WTab*> {};

class plPoint3Tab : public Tab<hsVector3> {};
class plPoint3TabTab : public Tab<plPoint3Tab*> {};

class plColorTab : public Tab<UInt32> {};
class plColorTabTab : public Tab<plColorTab*> {};

class plSpanTemplTab : public Tab<plSpanTemplateB*> {};

class plClusterUtil
{
protected:
	UInt32					fIdx;
	plClusterGroup*			fGroup;
	plMaxNode*				fTemplNode;
	plSpanTemplateB*		fTemplate;

	int						fMinFaces;
	int						fMaxFaces;
	hsScalar				fMinSize;

	int						fMinInsts;
	int						fMaxInsts;


	plSpanEncoding	ISelectEncoding(plPoint3TabTab& delPos, plColorTabTab& colors);
	void			IAllocPosAndColor(plSpanTemplateB* templ, const plL2WTab& insts,
									plPoint3TabTab& delPos, plColorTabTab& colors);
	void			IDelPosAndColor(plSpanTemplateB* templ,
									const plL2WTab& insts, plDeformVert* def, plShadeVert* shade,
									plPoint3TabTab& delPos, plColorTabTab& colors);
	void			IAddInstsToCluster(plCluster* cluster, plSpanTemplateB* templ, 
									   const plL2WTab& insts, 
									   plPoint3TabTab& delPos, 
									   plColorTabTab& colors);
	void			IFreePosAndColor(plPoint3TabTab& delPos, plColorTabTab& colors) const;

	void			IFreeClustersRecur(plL2WTabTab& dst) const;
	void			IFindClustersRecur(plSpanTemplateB* templ, plL2WTab& src, plL2WTabTab& dst);
	hsBool			ISplitCluster(plSpanTemplateB* templ, plL2WTab& src, plL2WTab& lo, plL2WTab& hi);
	int				ISelectAxis(const plL2WTab& src) const;
	Box3			IBound(const plL2WTab& src) const;
	Point3			ILength(const plL2WTab& src) const;

	void				ISortTemplate(plSpanTemplateB* templ) const;
	plSpanTemplateB*	IAddTemplate(plMaxNode* templNode, plGeometrySpan* geo);
	void				ITemplateFromGeo(plSpanTemplateB* templ, plGeometrySpan* geo);

	void				ISortTemplates(plSpanTemplTab& templs) const;
	void				IAddTemplates(plMaxNode* templNode, plSpanTemplTab& templs);

	void			ISetupGroupFromTemplate(plMaxNode* templ);

public:
	plClusterUtil();
	~plClusterUtil();

	plSpanTemplTab	MakeTemplates(INode* templNode);

	plClusterGroup* CreateGroup(plMaxNode* node, const char* name);
	plClusterGroup* SetupGroup(plClusterGroup* group, plMaxNode* node, plSpanTemplateB* templ);
	plClusterGroup* GetGroup() const { return fGroup; }

	void AddClusters(plL2WTab& insts, plDeformVert* def, plShadeVert* shade);

};

#endif // plClusterUtil_inc
