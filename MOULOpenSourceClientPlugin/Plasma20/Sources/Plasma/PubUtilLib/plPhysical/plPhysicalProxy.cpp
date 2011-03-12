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
#include "plPhysicalProxy.h"
#include "plPhysical.h"
#include "../plPhysX/plPXPhysicalControllerCore.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../pnMessage/plProxyDrawMsg.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"

plPhysicalProxy::plPhysicalProxy()
:	plProxyGen(hsColorRGBA().Set(0,0,0,1.f), hsColorRGBA().Set(1.f,0.8f,0.2f,1.f), 0.5f),
	fOwner(nil)
{
}

plPhysicalProxy::plPhysicalProxy(const hsColorRGBA& amb, const hsColorRGBA& dif, hsScalar opac)
:	plProxyGen(amb, dif, opac),
	fOwner(nil),
	fController(nil)
{
}

plPhysicalProxy::~plPhysicalProxy()
{
}

bool plPhysicalProxy::Init(plPhysical* liInfo)
{
	plProxyGen::Init(liInfo);

	fOwner = liInfo;
	fProxyMsgType = plProxyDrawMsg::kPhysical;

	return fOwner != nil;
}

bool plPhysicalProxy::Init(plPXPhysicalControllerCore* controller)
{
	if (controller)
		if (controller->GetOwner())
			plProxyGen::Init(controller->GetOwner()->GetObjectPtr());

	fController = controller;
	fProxyMsgType = plProxyDrawMsg::kPhysical;

	return fController != nil;
}

plKey plPhysicalProxy::IGetNode() const 
{
	if (fOwner)
		return fOwner->GetSceneNode();
	if (fController)
		return fController->GetOwner();
	return nil;
}

plDrawableSpans* plPhysicalProxy::ICreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	if (fOwner)
	{
		return fOwner->CreateProxy(mat, idx, addTo);
	}
	if (fController)
	{
		return fController->CreateProxy(mat,idx,addTo);
	}
	return nil;
}