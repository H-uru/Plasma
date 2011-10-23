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
/**********************************************************************
 *<
    FILE: prim.h

    DESCRIPTION:

    CREATED BY: Dan Silva

    HISTORY:

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __PRIM__H
#define __PRIM__H

#include "Max.h"
#include "resource.h"

#ifdef DESIGN_VER //for conversion to amodeler solids
#include "igeomimp.h"
#include "plugapi.h"
#endif

TCHAR *GetString(int id);

extern ClassDesc* GetBoxobjDesc();
extern ClassDesc* GetSphereDesc();
extern ClassDesc* GetCylinderDesc();
extern ClassDesc* GetSimpleCamDesc();
extern ClassDesc* GetOmniLightDesc();
extern ClassDesc* GetDirLightDesc();
extern ClassDesc *GetTDirLightDesc();
extern ClassDesc* GetFSpotLightDesc();
extern ClassDesc* GetTSpotLightDesc();
extern ClassDesc* GetLookatCamDesc();
extern ClassDesc* GetSplineDesc();
#ifdef DESIGN_VER
extern ClassDesc* GetOrthoSplineDesc();
#endif
extern ClassDesc* GetNGonDesc();
extern ClassDesc* GetDonutDesc();
extern ClassDesc* GetTargetObjDesc();
extern ClassDesc* GetBonesDesc();
extern ClassDesc* GetRingMasterDesc();
extern ClassDesc* GetSlaveControlDesc();
extern ClassDesc* GetQuadPatchDesc();
extern ClassDesc* GetTriPatchDesc();
extern ClassDesc* GetTorusDesc();
extern ClassDesc* GetMorphObjDesc();
extern ClassDesc* GetCubicMorphContDesc();
extern ClassDesc* GetRectangleDesc();
extern ClassDesc* GetBoolObjDesc();
extern ClassDesc* GetTapeHelpDesc();
extern ClassDesc* GetProtHelpDesc();
extern ClassDesc* GetTubeDesc();
extern ClassDesc* GetConeDesc();
extern ClassDesc* GetHedraDesc();
extern ClassDesc* GetCircleDesc();
extern ClassDesc* GetEllipseDesc();
extern ClassDesc* GetArcDesc();
extern ClassDesc* GetStarDesc();
extern ClassDesc* GetHelixDesc();
extern ClassDesc* GetRainDesc();
extern ClassDesc* GetSnowDesc();
extern ClassDesc* GetTextDesc();
extern ClassDesc* GetTeapotDesc();
extern ClassDesc* GetBaryMorphContDesc();
#ifdef DESIGN_VER
extern ClassDesc* GetOrthoSplineDesc();
extern ClassDesc* GetParallelCamDesc();
#endif
extern ClassDesc* GetGridobjDesc();
extern ClassDesc* GetNewBonesDesc();

extern HINSTANCE hInstance;


#endif
