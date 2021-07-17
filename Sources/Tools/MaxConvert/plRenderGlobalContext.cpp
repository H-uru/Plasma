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

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "MaxMain/plMaxNode.h"
#include "plRenderGlobalContext.h"
#include "plRenderInstance.h"

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
    envMap = nullptr;
    globalLightLevel.White();
    atmos = nullptr;
    pToneOp = nullptr;  // The tone operator, may be NULL
    time = t;
    wireMode = false;       // wire frame render mode?
    wire_thick = 1.f;   // global wire thickness
    force2Side = false; // is force two-sided rendering enabled
    inMtlEdit = false;      // rendering in mtl editor?
    fieldRender = false;    // are we rendering fields
    first_field = 1;    // is this the first field or the second?
    field_order = 1;    // which field is first: 0->even first,  1->odd first

    objMotBlur = true;  // is object motion blur enabled
    nBlurFrames = 10;   // number of object motion blur time slices

    SetRenderElementMgr(ip->GetRenderElementMgr(RS_Production));

}

plRenderGlobalContext::~plRenderGlobalContext()
{
    for (plRenderInstance& inst : fInstList)
    {
        inst.Cleanup();
    }
}

void plRenderGlobalContext::Update(TimeValue t)
{
    time = t;

    for (plRenderInstance& inst : fInstList)
        inst.Update(time);
}

void plRenderGlobalContext::MakeRenderInstances(plMaxNode* root, TimeValue t)
{
    time = t;
    for (int i = 0; i < root->NumberOfChildren(); i++)
        IMakeRenderInstances((plMaxNode*)root->GetChildNode(i), t, false);

    for (hsSsize_t i = 0; i < (hsSsize_t)fInstList.size() - 1; i++)
        fInstList[i].SetNext(&fInstList[i+1]);
}

int plRenderGlobalContext::NumRenderInstances()
{
    return (int)fInstList.size();
}

RenderInstance* plRenderGlobalContext::GetRenderInstance(int i)
{
    if ((int)fInstList.size() > i)
        return &fInstList[i];
    else
        return nullptr;
}

void plRenderGlobalContext::IMakeRenderInstances(plMaxNode* node, TimeValue t, bool isBarney)
{
    auto dbgNodeName = node->GetName();
    if( !isBarney )
        isBarney = node->GetIsBarney();

    bool doMe = isBarney || (node->CanConvert() && node->GetDrawable());

    if( !doMe )
        return;

    int idx = (int)fInstList.size();

    plRenderInstance& inst = fInstList.emplace_back();
    
    if (!inst.GetFromNode(node, t, idx))
        fInstList.pop_back();

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

        thisHit.next = nullptr;

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
    for (size_t i = 0; i < fInstList.size(); i++)
    {
        if (skipID != (int)i)
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
