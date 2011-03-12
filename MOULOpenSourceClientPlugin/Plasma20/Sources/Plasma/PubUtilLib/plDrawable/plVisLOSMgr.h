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

#ifndef plVisLOSMgr_inc
#define plVisLOSMgr_inc

#include "hsGeometry3.h"
#include "hsTemplates.h"

class plSpaceTreeNode;
class plSpaceTree;
class plDrawableSpans;
class plDrawable;
class plSceneNode;
class plPageTreeMgr;
class plPipeline;
class hsBounds3Ext;

class plVisHit
{
public:
	hsPoint3		fPos;
};

class plSpaceHit
{
public:
	int			fIdx;
	hsScalar	fClosest;
};

class plVisLOSMgr
{
protected:
	plPageTreeMgr*	fPageMgr;
	plPipeline*		fPipe;

	hsScalar		fMaxDist;

	hsPoint3		fCurrFrom;
	hsPoint3		fCurrTarg;

	hsBool ISetup(const hsPoint3& pStart, const hsPoint3& pEnd);
	hsBool ICheckBound(const hsBounds3Ext& bnd, hsScalar& closest);
	hsBool ICheckSpaceTreeRecur(plSpaceTree* space, int which, hsTArray<plSpaceHit>& hits);
	hsBool ICheckSpaceTree(plSpaceTree* space, hsTArray<plSpaceHit>& hits);
	hsBool ICheckSceneNode(plSceneNode* node, plVisHit& hit);
	hsBool ICheckDrawable(plDrawable* d, plVisHit& hit);
	hsBool ICheckSpan(plDrawableSpans* dr, UInt32 spanIdx, plVisHit& hit);
	
public:
	hsBool Check(const hsPoint3& pStart, const hsPoint3& pEnd, plVisHit& hit);
	hsBool CursorCheck(plVisHit& hit);

	static plVisLOSMgr* Instance();

	static void Init(plPipeline* pipe, plPageTreeMgr* mgr) { Instance()->fPipe = pipe; Instance()->fPageMgr = mgr; }
	static void DeInit() { Instance()->fPipe = nil; Instance()->fPageMgr = nil; }
};

#endif // plVisLOSMgr_inc
