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

#ifndef plDistTree_inc
#define plDistTree_inc

#include "hsTemplates.h"

class plDistNode
{
public:
	enum {
		kIsLeaf = 0x1
	};
	UInt32		fFlags;

	Int32		fChildren[8];
	Box3		fBox;
	Box3		fFade;

	union
	{
		void*		fPData;
		UInt32		fIData;
	};

	const Box3&	GetBox() const { return fBox; }

	BOOL		IsLeaf() const { return 0 != (fFlags & kIsLeaf); }
	void		SetIsLeaf(BOOL on) { if(on)fFlags |= kIsLeaf; else fFlags &= ~kIsLeaf; }
};

class plDistTree
{
protected:
	
	Int32							fRoot;

	hsLargeArray<plDistNode>		fNodes;

	Int32	IAddNodeRecur(Int32 iNode, const Box3& box, const Box3& fade, UInt32 iData);

	Int32	IMergeNodes(Int32 iNode, const Box3& box, const Box3& fade, UInt32 iData);
	Int32	INextNode(const Box3& box, const Box3& fade, UInt32 iData);

	Int32	IGetChild(const Box3& parent, const Box3& child) const;

	inline BOOL	IBoxesClear(const Box3& box0, const Box3& box1) const;
	inline BOOL IFadesClear(const Box3& fade0, const Box3& fade1) const;

	BOOL	IBox0ContainsBox1(const Box3& box0, const Box3& box1, const Box3& fade0, const Box3& fade1) const;

	BOOL	IBoxClearRecur(Int32 iNode, const Box3& box, const Box3& fade) const;
	BOOL	IPointClearRecur(Int32 iNode, const Point3& pt, const Box3& fade) const;

	void	IHarvestBoxRecur(Int32 iNode, const Box3& box, Tab<Int32>& out) const;

public:
	plDistTree();
	virtual ~plDistTree();

	void Reset();

	void AddBoxPData(const Box3& box, const Box3& fade, void* pData=nil) { AddBoxIData(box, fade, UInt32(pData)); }
	void AddBoxIData(const Box3& box, const Box3& fade, UInt32 iData=0);
	void AddBox(const Box3& box, const Box3& fade=NonFade()) { AddBoxIData(box, fade, 0); }

	BOOL BoxClear(const Box3& box, const Box3& fade) const;
	BOOL PointClear(const Point3& pt, const Box3& fade) const;

	BOOL IsEmpty() const { return fRoot < 0; }

	static Box3	NonFade() { return Box3(Point3(0,0,0), Point3(0,0,0)); }

	void HarvestBox(const Box3& box, Tab<Int32>& out) const;

	const plDistNode& GetBox(Int32 i) const { return fNodes[i]; }
};

#endif // plDistTree_inc
