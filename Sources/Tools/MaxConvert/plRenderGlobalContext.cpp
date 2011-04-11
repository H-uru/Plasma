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
#include "Max.h"

#include "../MaxMain/plMaxNode.h"
#include "plRenderGlobalContext.h"

plRenderGlobalContext::plRenderGlobalContext(Interface* ip, TimeValue t)
{
	fInterface = ip;

	renderer = ip->GetProductionRenderer();
	projType = PROJ_PERSPECTIVE;
	devWidth = 640;
	devHeight = 480;
	xscale = 1.6f;
	yscale = -1.6f;
	xc = 320.f;
	yc = 240.f;
	antialias = 1;
	camToWorld.IdentityMatrix();
	worldToCam.IdentityMatrix();
	nearRange = 0;
	farRange = 10000.f;
	devAspect = 1.f;          // PIXEL aspect ratio of device pixel H/W
	frameDur = 1.f;
	envMap = nil;
	globalLightLevel.White();
	atmos = nil;
	pToneOp = nil;	// The tone operator, may be NULL
	time = t;
	wireMode = false;		// wire frame render mode?
	wire_thick = 1.f;	// global wire thickness
	force2Side = false;	// is force two-sided rendering enabled
	inMtlEdit = false;		// rendering in mtl editor?
	fieldRender = false;	// are we rendering fields
	first_field = 1;	// is this the first field or the second?
	field_order = 1;	// which field is first: 0->even first,  1->odd first

	objMotBlur = true;	// is object motion blur enabled
	nBlurFrames = 10;	// number of object motion blur time slices

	SetRenderElementMgr(ip->GetRenderElementMgr(RS_Production));

}

plRenderGlobalContext::~plRenderGlobalContext()
{
	int i;
	for( i = 0; i < fInstList.GetCount(); i++ )
	{
		fInstList[i].Cleanup();
	}
}

void plRenderGlobalContext::Update(TimeValue t)
{
	time = t;

	int i;
	for( i = 0; i < fInstList.GetCount(); i++ )
		fInstList[i].Update(time);
}

void plRenderGlobalContext::MakeRenderInstances(plMaxNode* root, TimeValue t)
{
	time = t;
	int i;
	for( i = 0; i < root->NumberOfChildren(); i++ )
		IMakeRenderInstances((plMaxNode*)root->GetChildNode(i), t, false);

	for( i = 0; i < fInstList.GetCount() - 1; i++ )
		fInstList[i].SetNext(&fInstList[i+1]);
}

void plRenderGlobalContext::IMakeRenderInstances(plMaxNode* node, TimeValue t, hsBool isBarney)
{
	const char* dbgNodeName = node->GetName();
	if( !isBarney )
		isBarney = node->GetIsBarney();

	hsBool doMe = isBarney || (node->CanConvert() && node->GetDrawable());

	if( !doMe )
		return;

	int idx = fInstList.GetCount();

	plRenderInstance* inst = fInstList.Push();
	
	if( !inst->GetFromNode(node, t, idx) )
		fInstList.Pop();

	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
		IMakeRenderInstances((plMaxNode *)node->GetChildNode(i), t, isBarney);
}

void plRenderGlobalContext::IntersectRay(RenderInstance *inst, Ray& origRay, ISect &isct, ISectList &xplist, BOOL findExit)
{
	const float kFarAway = 1.e5f;
	Ray ray;
	if( findExit )
	{
		ray.p = origRay.p + origRay.dir * kFarAway;
		ray.dir = -ray.dir;
	}
	else
	{
		ray = origRay;
	}
	float at;
	Point3 norm;
	DWORD faceIdx;
	Point3 bary;
	int hit = inst->mesh->IntersectRay(ray, at, norm, faceIdx, bary);
	if( hit )
	{
		ISect thisHit;
		thisHit.t = findExit ? kFarAway - at : at;
		thisHit.exit = findExit;
		thisHit.backFace = findExit;
		thisHit.inst = inst;
		thisHit.fnum = faceIdx;
		thisHit.bc = bary;
		Point3 worldP = (ray.p + at * ray.dir);
		thisHit.p = inst->GetINode()->GetObjectTM(time) * worldP;
		thisHit.pc = worldToCam * thisHit.p;

		thisHit.mtlNum = inst->mesh->faces[faceIdx].getMatID();
		Mtl* mtl = inst->GetINode()->GetMtl();
		thisHit.matreq = mtl ? mtl->Requirements(thisHit.mtlNum) : 0;

		thisHit.next = nil;

		if( thisHit.matreq & (MTLREQ_TRANSP | MTLREQ_ADDITIVE_TRANSP) )
		{
			// advance the ray and try again. This one goes in the xplist.
			ISect* xHit = GetNewISect();
			*xHit = thisHit;
			xplist.Add(xHit);

			const float kAdvanceHack = 0.5f;
			Ray newRay;
			newRay.p = origRay.p + origRay.dir * (thisHit.t + kAdvanceHack);
			newRay.dir = origRay.dir;
			IntersectRay(inst, newRay, isct, xplist, findExit);
		}
		else
		{
			xplist.Prune(thisHit.t);
			isct = thisHit;
		}
	}
}

BOOL plRenderGlobalContext::IntersectWorld(Ray &ray, int skipID, ISect &hit, ISectList &xplist, int blurFrame)
{
	hit.t = -1.f;
	xplist.Init();
	int i;
	for( i = 0; i < fInstList.GetCount(); i++ )
	{
		if( skipID != i )
		{
			ISect thisHit;
			hit.t = -1.f;
			IntersectRay(&fInstList[i], ray, thisHit, xplist, false);
			if( thisHit.t >= 0 )
			{
				if( (hit.t < 0) || (thisHit.t < hit.t) )
				{
					// grab our new winner.
					hit = thisHit;
				}
			}
		}
	}
	return (hit.t >= 0) || !xplist.IsEmpty();
}
