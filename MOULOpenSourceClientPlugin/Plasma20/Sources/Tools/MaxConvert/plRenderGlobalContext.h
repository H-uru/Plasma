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

#ifndef plRenderGlobalContext_inc
#define plRenderGlobalContext_inc

#include "hsTemplates.h"
#include "plRenderInstance.h"

class plRenderGlobalContext : public RenderGlobalContext
{
protected:
	Interface*							fInterface;

	hsTArray<plRenderInstance>			fInstList;

	void								IMakeRenderInstances(plMaxNode* node, TimeValue t, hsBool isBarney);

public:
	plRenderGlobalContext(Interface* ip, TimeValue t);
	~plRenderGlobalContext();

	void Update(TimeValue t);

	void MakeRenderInstances(plMaxNode* root, TimeValue t);

	virtual int NumRenderInstances() { return fInstList.GetCount(); }
	virtual RenderInstance* GetRenderInstance( int i ) { return i < fInstList.GetCount() ? &fInstList[i] : nil; }

	virtual void IntersectRay(RenderInstance *inst, Ray& ray, ISect &isct, ISectList &xpList, BOOL findExit);
	virtual BOOL IntersectWorld(Ray &ray, int skipID, ISect &hit, ISectList &xplist, int blurFrame = NO_MOTBLUR);
};

#endif // plRenderGlobalContext_inc
