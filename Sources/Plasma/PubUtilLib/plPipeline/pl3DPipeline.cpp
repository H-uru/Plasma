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

#include "pl3DPipeline.h"

plProfile_CreateTimer("RenderScene",            "PipeT", RenderScene);
plProfile_CreateTimer("VisEval",                "PipeT", VisEval);
plProfile_CreateTimer("VisSelect",              "PipeT", VisSelect);
plProfile_CreateTimer("PlateMgr",               "PipeT", PlateMgr);
plProfile_CreateTimer("DebugText",              "PipeT", DebugText);
plProfile_CreateTimer("Reset",                  "PipeT", Reset);
plProfile_CreateTimer("PrepShadows",            "PipeT", PrepShadows);
plProfile_CreateTimer("PrepDrawable",           "PipeT", PrepDrawable);
plProfile_CreateTimer("  Skin",                 "PipeT", Skin);
plProfile_CreateTimer("  AvSort",               "PipeT", AvatarSort);
plProfile_CreateTimer("     ClearLights",       "PipeT", ClearLights);

plProfile_CreateTimer("RenderSpan",             "PipeT", RenderSpan);
plProfile_CreateTimer("  MergeCheck",           "PipeT", MergeCheck);
plProfile_CreateTimer("  MergeSpan",            "PipeT", MergeSpan);
plProfile_CreateTimer("  SpanTransforms",       "PipeT", SpanTransforms);
plProfile_CreateTimer("  SpanFog",              "PipeT", SpanFog);
plProfile_CreateTimer("  SelectLights",         "PipeT", SelectLights);
plProfile_CreateTimer("  SelectProj",           "PipeT", SelectProj);
plProfile_CreateTimer("  CheckDyn",             "PipeT", CheckDyn);
plProfile_CreateTimer("  CheckStat",            "PipeT", CheckStat);
plProfile_CreateTimer("  RenderBuff",           "PipeT", RenderBuff);
plProfile_CreateTimer("  RenderPrim",           "PipeT", RenderPrim);

plProfile_CreateTimer("FindSceneLights",        "PipeT", FindSceneLights);
plProfile_CreateTimer("  Find Lights",          "PipeT", FindLights);
plProfile_CreateTimer("    Find Perms",         "PipeT", FindPerm);
plProfile_CreateTimer("    FindSpan",           "PipeT", FindSpan);
plProfile_CreateTimer("    FindActiveLights",   "PipeT", FindActiveLights);
plProfile_CreateTimer("    ApplyActiveLights",  "PipeT", ApplyActiveLights);
plProfile_CreateTimer("      ApplyMoving",      "PipeT", ApplyMoving);
plProfile_CreateTimer("      ApplyToSpec",      "PipeT", ApplyToSpec);
plProfile_CreateTimer("      ApplyToMoving",    "PipeT", ApplyToMoving);

plProfile_CreateCounter("LightOn",              "PipeC", LightOn);
plProfile_CreateCounter("LightVis",             "PipeC", LightVis);
plProfile_CreateCounter("LightChar",            "PipeC", LightChar);
plProfile_CreateCounter("LightActive",          "PipeC", LightActive);
plProfile_CreateCounter("Lights Found",         "PipeC", FindLightsFound);
plProfile_CreateCounter("Perms Found",          "PipeC", FindLightsPerm);
plProfile_CreateCounter("NumSkin",              "PipeC", NumSkin);
plProfile_CreateCounter("AvRTPoolUsed",         "PipeC", AvRTPoolUsed);
plProfile_CreateCounter("AvRTPoolCount",        "PipeC", AvRTPoolCount);
plProfile_CreateCounter("AvRTPoolRes",          "PipeC", AvRTPoolRes);
plProfile_CreateCounter("AvRTShrinkTime",       "PipeC", AvRTShrinkTime);
plProfile_CreateCounter("AvatarFaces",          "PipeC", AvatarFaces);
plProfile_CreateCounter("Merge",                "PipeC", SpanMerge);
plProfile_CreateCounter("LiState",              "PipeC", MatLightState);
plProfile_CreateCounter("EmptyList",            "PipeC", EmptyList);

plProfile_CreateCounterNoReset("Reload",        "PipeC", PipeReload);

plProfile_CreateCounter("Polys",                "General",  DrawTriangles);
plProfile_CreateCounter("Material Change",      "Draw",     MatChange);
plProfile_CreateCounter("Feed Triangles",       "Draw",     DrawFeedTriangles);
plProfile_CreateCounter("Draw Prim Static",     "Draw",     DrawPrimStatic);
plProfile_CreateCounter("Layer Change",         "Draw",     LayChange);

plProfile_CreateMemCounter("Total Texture Size", "Draw", TotalTexSize);
plProfile_CreateMemCounter("Vertices", "Memory", MemVertex);
plProfile_CreateMemCounter("Indices", "Memory", MemIndex);
plProfile_CreateMemCounter("Textures", "Memory", MemTexture);

PipelineParams plPipeline::fDefaultPipeParams;
PipelineParams plPipeline::fInitialPipeParams;

int mfCurrentTest = 100;

plDisplayHelper* plDisplayHelper::fCurrentDisplayHelper = nullptr;
