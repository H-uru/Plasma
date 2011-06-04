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
#include "hsConfig.h"
#include "hsWindows.h"

#include <D3d9.h>
#include <D3dx9core.h>

#include "hsTypes.h"

#include "plDXShader.h"

#include "../plSurface/plShader.h"

#include "plDXPipeline.h"

plDXShader::plDXShader(plShader* owner)
:	fOwner(owner),
	fErrorString(nil),
	fPipe(nil)
{
	owner->SetDeviceRef(this);
}

plDXShader::~plDXShader()
{
	fPipe = nil;

	ISetError(nil);
}

void plDXShader::SetOwner(plShader* owner)
{
	if( owner != fOwner )
	{
		Release();
		fOwner = owner;
		owner->SetDeviceRef(this);
	}
}

const char* plDXShader::ISetError(const char* errStr)
{
	delete [] fErrorString;
	if( errStr )
		fErrorString = hsStrcpy(errStr);
	else
		fErrorString = nil;

	return fErrorString;
}

HRESULT plDXShader::IOnError(HRESULT hr, const char* errStr)
{
	ISetError(errStr);

	fOwner->Invalidate();

	hsStatusMessage(errStr);

	return hr;
}

