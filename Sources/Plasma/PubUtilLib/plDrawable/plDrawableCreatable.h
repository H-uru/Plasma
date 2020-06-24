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

#ifndef plDrawableCreatable_inc
#define plDrawableCreatable_inc

#include "pnFactory/plCreator.h"

#include "plActivePrintShape.h"
REGISTER_CREATABLE(plActivePrintShape);

#include "plClusterGroup.h"
REGISTER_CREATABLE(plClusterGroup);

#include "plCutter.h"
REGISTER_CREATABLE(plCutter);

#include "plDrawableSpans.h"
REGISTER_CREATABLE(plDrawableSpans);

#include "plDynaBulletMgr.h"
REGISTER_CREATABLE(plDynaBulletMgr);

#include "plDynaDecalMgr.h"
REGISTER_NONCREATABLE(plDynaDecalMgr);

#include "plDynaFootMgr.h"
REGISTER_CREATABLE(plDynaFootMgr);

#include "plDynaPuddleMgr.h"
REGISTER_CREATABLE(plDynaPuddleMgr);

#include "plDynaRippleMgr.h"
REGISTER_CREATABLE(plDynaRippleMgr);

#include "plDynaRippleVSMgr.h"
REGISTER_CREATABLE(plDynaRippleVSMgr);

#include "plDynaTorpedoMgr.h"
REGISTER_CREATABLE(plDynaTorpedoMgr);

#include "plDynaTorpedoVSMgr.h"
REGISTER_CREATABLE(plDynaTorpedoVSMgr);

#include "plDynaWakeMgr.h"
REGISTER_CREATABLE(plDynaWakeMgr);

#include "plInstanceDrawInterface.h"
REGISTER_CREATABLE(plInstanceDrawInterface);

#include "plMorphDelta.h"
REGISTER_CREATABLE(plMorphDelta);

#include "plMorphSequence.h"
REGISTER_CREATABLE(plMorphDataSet);
REGISTER_CREATABLE(plMorphSequence);

#include "plMorphSequenceSDLMod.h"
REGISTER_CREATABLE(plMorphSequenceSDLMod);

#include "plPrintShape.h"
REGISTER_CREATABLE(plPrintShape);

#include "plSharedMesh.h"
REGISTER_CREATABLE(plSharedMesh);
REGISTER_CREATABLE(plSharedMeshBCMsg);

#include "plSpaceTree.h"
REGISTER_CREATABLE(plSpaceTree);

#include "plWaveSetBase.h"
REGISTER_NONCREATABLE(plWaveSetBase);

#include "plWaveSet7.h"
REGISTER_CREATABLE(plWaveSet7);

#endif // plDrawableCreatable_inc
