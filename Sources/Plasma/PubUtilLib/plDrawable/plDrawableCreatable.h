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

#ifndef plDrawableCreatable_inc
#define plDrawableCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plDrawableSpans.h"

REGISTER_CREATABLE( plDrawableSpans );

#include "plSpaceTree.h"

REGISTER_CREATABLE( plSpaceTree );

#include "plSharedMesh.h"

REGISTER_CREATABLE( plSharedMesh );
REGISTER_CREATABLE( plSharedMeshBCMsg );

#include "plInstanceDrawInterface.h"

REGISTER_CREATABLE( plInstanceDrawInterface );

#include "plDynaDecalMgr.h"

REGISTER_NONCREATABLE( plDynaDecalMgr );

#include "plDynaFootMgr.h"

REGISTER_CREATABLE( plDynaFootMgr );

#include "plDynaRippleMgr.h"

REGISTER_CREATABLE( plDynaRippleMgr );

#include "plDynaBulletMgr.h"

REGISTER_CREATABLE( plDynaBulletMgr );

#include "plDynaPuddleMgr.h"

REGISTER_CREATABLE( plDynaPuddleMgr );

#include "plDynaTorpedoMgr.h"

REGISTER_CREATABLE( plDynaTorpedoMgr );

#include "plDynaTorpedoVSMgr.h"

REGISTER_CREATABLE( plDynaTorpedoVSMgr );

#include "plDynaWakeMgr.h"

REGISTER_CREATABLE( plDynaWakeMgr );

#include "plCutter.h"

REGISTER_CREATABLE( plCutter );

#include "plPrintShape.h"

REGISTER_CREATABLE( plPrintShape );

#include "plActivePrintShape.h"

REGISTER_CREATABLE( plActivePrintShape );

#include "plWaveSetBase.h"

REGISTER_NONCREATABLE( plWaveSetBase );

#include "plWaveSet7.h"

REGISTER_CREATABLE( plWaveSet7 );

#include "plMorphSequence.h"

REGISTER_CREATABLE( plMorphSequence );
REGISTER_CREATABLE( plMorphDataSet );

#include "plMorphSequenceSDLMod.h"

REGISTER_CREATABLE( plMorphSequenceSDLMod );

#include "plMorphDelta.h"

REGISTER_CREATABLE( plMorphDelta );

#include "plDynaRippleVSMgr.h"

REGISTER_CREATABLE( plDynaRippleVSMgr );

#include "plClusterGroup.h"

REGISTER_CREATABLE( plClusterGroup );

#endif // plDrawableCreatable_inc
