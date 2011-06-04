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

#include "plGrabCubeMap.h"

#include "plPipeline.h"
#include "plDrawable.h"

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"
#include "hsBounds.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"

#include "../plGImage/plMipmap.h"

#include "../plJPEG/plJPEG.h"

#include "../plMessage/plRenderRequestMsg.h"

plGrabCubeRenderRequest::plGrabCubeRenderRequest()
:	fQuality(75)
{
}

void plGrabCubeRenderRequest::Render(plPipeline* pipe, plPageTreeMgr* pageMgr)
{
	if( !fFileName[0] )
		return;

	plRenderRequest::Render(pipe, pageMgr);

	pipe->EndRender();

	plMipmap		mipmap;
	if( pipe->CaptureScreen(&mipmap) )
	{
		plJPEG::Instance().SetWriteQuality(fQuality);

		plJPEG::Instance().WriteToFile(fFileName, &mipmap);
	}

	pipe->BeginRender();
}


void plGrabCubeMap::GrabCube(plPipeline* pipe, plSceneObject* obj, const char* pref, const hsColorRGBA& clearColor, UInt8 q)
{
	hsPoint3 center;
	if( obj && !(obj->GetLocalToWorld().fFlags & hsMatrix44::kIsIdent) )
	{
		center = obj->GetLocalToWorld().GetTranslate();
	}
	else if( obj && obj->GetDrawInterface() )
	{
		center = obj->GetDrawInterface()->GetWorldBounds().GetCenter();
	}
	else
	{
		center = pipe->GetCameraToWorld().GetTranslate();
	}
	ISetupRenderRequests(pipe, center, pref, clearColor, q);
}

void plGrabCubeMap::GrabCube(plPipeline* pipe, const hsPoint3& center, const char* pref, const hsColorRGBA& clearColor, UInt8 q)
{
	ISetupRenderRequests(pipe, center, pref, clearColor, q);
}

void plGrabCubeMap::ISetupRenderRequests(plPipeline* pipe, const hsPoint3& center, const char* pref, const hsColorRGBA& clearColor, UInt8 q) const
{
	hsMatrix44 worldToCameras[6];
	hsMatrix44 cameraToWorlds[6];
	hsMatrix44::MakeEnvMapMatrices(center, worldToCameras, cameraToWorlds);

	UInt32 renderState 
		= plPipeline::kRenderNormal
		| plPipeline::kRenderClearColor
		| plPipeline::kRenderClearDepth;

	float hither;
	float yon;
	pipe->GetDepth(hither, yon);

	const char* suff[6] = {
		"LF",
		"RT",
		"BK",
		"FR",
		"UP",
		"DN" };

	int i;
	for( i = 0; i < 6; i++ )
	{
		plGrabCubeRenderRequest* req = TRACKED_NEW plGrabCubeRenderRequest;
		req->SetRenderState(renderState);

		req->SetDrawableMask(plDrawable::kNormal);
		req->SetSubDrawableMask(plDrawable::kSubAllTypes);

		req->SetHither(hither);
		req->SetYon(yon);

		req->SetFovX(90.f);
		req->SetFovY(90.f);

		req->SetClearColor(clearColor);
		req->SetClearDepth(1.f);

		req->SetClearDrawable(nil);
		req->SetRenderTarget(nil);

		req->SetCameraTransform(worldToCameras[i], cameraToWorlds[i]);

		req->fQuality = q;
		sprintf(req->fFileName, "%s_%s.jpg", pref, suff[i]);

		plRenderRequestMsg* reqMsg = TRACKED_NEW plRenderRequestMsg(nil, req);
		reqMsg->Send();
		hsRefCnt_SafeUnRef(req);
	}
}