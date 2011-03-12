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

#ifndef plPipelineCreatable_inc
#define plPipelineCreatable_inc

#include "../pnFactory/plCreator.h"

#include <d3d9.h>

#include "plDXPipeline.h"

REGISTER_NONCREATABLE( plDXPipeline );

#include "hsFogControl.h"

REGISTER_NONCREATABLE( hsFogControl );

#include "plFogEnvironment.h"

REGISTER_CREATABLE( plFogEnvironment );

#include "plRenderTarget.h"

REGISTER_CREATABLE( plRenderTarget );

#include "plCubicRenderTarget.h"

REGISTER_CREATABLE( plCubicRenderTarget );

#include "plCubicRenderTargetModifier.h"

REGISTER_CREATABLE( plCubicRenderTargetModifier );

#include "plTransitionMgr.h"

REGISTER_CREATABLE( plTransitionMgr );

#include "plDynamicEnvMap.h"
REGISTER_CREATABLE( plDynamicEnvMap );
REGISTER_CREATABLE( plDynamicCamMap );

#endif // plPipelineCreatable_inc
