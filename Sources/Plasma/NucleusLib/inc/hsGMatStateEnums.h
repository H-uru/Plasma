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

//////////////////////////////////////////////////////////////////////////////
//
// This is intended only to be included by hsGMatState.h and shader files!
// If you want these values in C++ code, include hsGMatState.h instead.
//
/////////////////////////////////////////////////////////////////////////////
//
// This file expects several macros to be declared before it is included.
//
// GMAT_STATE_ENUM_START(name) should declare an enum (if supported) with
// the given name
//
// GMAT_STATE_ENUM_END(name) should close the enum (if supported)
//
// GMAT_STATE_ENUM_VALUE(name, val) should declare an enum value or constant
//
//////////////////////////////////////////////////////////////////////////////

GMAT_STATE_ENUM_START(hsGMatBlendFlags)
    GMAT_STATE_ENUM_VALUE(kBlendTest,                     0x1) // dev

    // Rest of blends are mutually exclusive
    GMAT_STATE_ENUM_VALUE(kBlendAlpha,                    0x2) // dev
    GMAT_STATE_ENUM_VALUE(kBlendMult,                     0x4) // dev
    GMAT_STATE_ENUM_VALUE(kBlendAdd,                      0x8) // dev
    GMAT_STATE_ENUM_VALUE(kBlendAddColorTimesAlpha,      0x10) // dev
    GMAT_STATE_ENUM_VALUE(kBlendAntiAlias,               0x20)
    GMAT_STATE_ENUM_VALUE(kBlendDetail,                  0x40)
    GMAT_STATE_ENUM_VALUE(kBlendNoColor,                 0x80) // dev
    GMAT_STATE_ENUM_VALUE(kBlendMADD,                   0x100)
    GMAT_STATE_ENUM_VALUE(kBlendDot3,                   0x200)
    GMAT_STATE_ENUM_VALUE(kBlendAddSigned,              0x400)
    GMAT_STATE_ENUM_VALUE(kBlendAddSigned2X,            0x800)

    GMAT_STATE_ENUM_VALUE(kBlendMask, kBlendAlpha
                                    | kBlendMult
                                    | kBlendAdd
                                    | kBlendAddColorTimesAlpha
                                    | kBlendDetail
                                    | kBlendMADD
                                    | kBlendDot3
                                    | kBlendAddSigned
                                    | kBlendAddSigned2X)

    GMAT_STATE_ENUM_VALUE(kBlendInvertAlpha,           0x1000) // dev
    GMAT_STATE_ENUM_VALUE(kBlendInvertColor,           0x2000) // dev
    GMAT_STATE_ENUM_VALUE(kBlendAlphaMult,             0x4000)
    GMAT_STATE_ENUM_VALUE(kBlendAlphaAdd,              0x8000)
    GMAT_STATE_ENUM_VALUE(kBlendNoVtxAlpha,           0x10000)
    GMAT_STATE_ENUM_VALUE(kBlendNoTexColor,           0x20000)
    GMAT_STATE_ENUM_VALUE(kBlendNoTexAlpha,           0x40000)
    GMAT_STATE_ENUM_VALUE(kBlendInvertVtxAlpha,       0x80000) // Invert ONLY the vertex alpha source
    GMAT_STATE_ENUM_VALUE(kBlendAlphaAlways,         0x100000) // Alpha test always passes (even for alpha=0)
    GMAT_STATE_ENUM_VALUE(kBlendInvertFinalColor,    0x200000)
    GMAT_STATE_ENUM_VALUE(kBlendInvertFinalAlpha,    0x400000)
    GMAT_STATE_ENUM_VALUE(kBlendEnvBumpNext,         0x800000)
    GMAT_STATE_ENUM_VALUE(kBlendSubtract,           0x1000000)
    GMAT_STATE_ENUM_VALUE(kBlendRevSubtract,        0x2000000)
    GMAT_STATE_ENUM_VALUE(kBlendAlphaTestHigh,      0x4000000)
    GMAT_STATE_ENUM_VALUE(kBlendAlphaPremultiplied, 0x8000000)
GMAT_STATE_ENUM_END(hsGMatBlendFlags)


GMAT_STATE_ENUM_START(hsGMatClampFlags)
    GMAT_STATE_ENUM_VALUE(kClampTextureU,                 0x1)
    GMAT_STATE_ENUM_VALUE(kClampTextureV,                 0x2)

    GMAT_STATE_ENUM_VALUE(kClampTexture, kClampTextureU
                                       | kClampTextureV)
GMAT_STATE_ENUM_END(hsGMatClampFlags)


GMAT_STATE_ENUM_START(hsGMatShadeFlags)
    GMAT_STATE_ENUM_VALUE(kShadeSoftShadow,               0x1) // view, dev
    GMAT_STATE_ENUM_VALUE(kShadeNoProjectors,             0x2) // projector
    GMAT_STATE_ENUM_VALUE(kShadeEnvironMap,               0x4) // dev, load
    GMAT_STATE_ENUM_VALUE(kShadeVertexShade,             0x20) // dev
    GMAT_STATE_ENUM_VALUE(kShadeNoShade,                 0x40) // view,dev

    GMAT_STATE_ENUM_VALUE(kShadeBlack, kShadeNoShade)

    GMAT_STATE_ENUM_VALUE(kShadeSpecular,                0x80) // view, dev
    //GMAT_STATE_ENUM_VALUE(kShadeNoFog,                  0x100) // dev
    GMAT_STATE_ENUM_VALUE(kShadeWhite,                  0x200)
    GMAT_STATE_ENUM_VALUE(kShadeSpecularAlpha,          0x400)
    GMAT_STATE_ENUM_VALUE(kShadeSpecularColor,          0x800)
    GMAT_STATE_ENUM_VALUE(kShadeSpecularHighlight,     0x1000)
    GMAT_STATE_ENUM_VALUE(kShadeVertColShade,          0x2000)
    GMAT_STATE_ENUM_VALUE(kShadeInherit,               0x4000)
    GMAT_STATE_ENUM_VALUE(kShadeIgnoreVtxIllum,        0x8000)
    GMAT_STATE_ENUM_VALUE(kShadeEmissive,             0x10000) // Moved here 8.27 mcn. Only really sane to use with kMiscEndPassHere
    GMAT_STATE_ENUM_VALUE(kShadeReallyNoFog,          0x20000)
GMAT_STATE_ENUM_END(hsGMatShadeFlags)


GMAT_STATE_ENUM_START(hsGMatZFlags)
    GMAT_STATE_ENUM_VALUE(kZIncLayer,                     0x1) // dev
    GMAT_STATE_ENUM_VALUE(kZClearZ,                       0x4) // dev
    GMAT_STATE_ENUM_VALUE(kZNoZRead,                      0x8) // dev
    GMAT_STATE_ENUM_VALUE(kZNoZWrite,                    0x10)

    GMAT_STATE_ENUM_VALUE(kZMask, kZNoZWrite
                                | kZClearZ
                                | kZNoZRead)

    GMAT_STATE_ENUM_VALUE(kZLODBias,                     0x20)
GMAT_STATE_ENUM_END(hsGMatZFlags)


GMAT_STATE_ENUM_START(hsGMatMiscFlags)
    GMAT_STATE_ENUM_VALUE(kMiscWireFrame,                 0x1) // dev (running out of bits)
    GMAT_STATE_ENUM_VALUE(kMiscDrawMeshOutlines,          0x2) // dev, currently unimplemented
    GMAT_STATE_ENUM_VALUE(kMiscTwoSided,                  0x4) // view,dev
    GMAT_STATE_ENUM_VALUE(kMiscDrawAsSplats,              0x8) // dev? bwt
    GMAT_STATE_ENUM_VALUE(kMiscAdjustPlane,              0x10)
    GMAT_STATE_ENUM_VALUE(kMiscAdjustCylinder,           0x20)
    GMAT_STATE_ENUM_VALUE(kMiscAdjustSphere,             0x40)

    GMAT_STATE_ENUM_VALUE(kMiscAdjust, kMiscAdjustPlane
                                     | kMiscAdjustCylinder
                                     | kMiscAdjustSphere)

    GMAT_STATE_ENUM_VALUE(kMiscTroubledLoner,            0x80)
    GMAT_STATE_ENUM_VALUE(kMiscBindSkip,                0x100)
    GMAT_STATE_ENUM_VALUE(kMiscBindMask,                0x200)
    GMAT_STATE_ENUM_VALUE(kMiscBindNext,                0x400)
    GMAT_STATE_ENUM_VALUE(kMiscLightMap,                0x800)

    // Use the calculated reflection environment texture transform instead of
    // layer->GetTransform()
    GMAT_STATE_ENUM_VALUE(kMiscUseReflectionXform,     0x1000)
    GMAT_STATE_ENUM_VALUE(kMiscPerspProjection,        0x2000)
    GMAT_STATE_ENUM_VALUE(kMiscOrthoProjection,        0x4000)

    GMAT_STATE_ENUM_VALUE(kMiscProjection, kMiscPerspProjection
                                         | kMiscOrthoProjection)

    // Tells pipeline to start a new pass beginning with this layer
    // Kinda like troubledLoner, but only cuts off lower layers, not higher
    // ones (kMiscBindNext sometimes does this by implication)
    GMAT_STATE_ENUM_VALUE(kMiscRestartPassHere,        0x8000)

    GMAT_STATE_ENUM_VALUE(kMiscBumpLayer,             0x10000)
    GMAT_STATE_ENUM_VALUE(kMiscBumpDu,                0x20000)
    GMAT_STATE_ENUM_VALUE(kMiscBumpDv,                0x40000)
    GMAT_STATE_ENUM_VALUE(kMiscBumpDw,                0x80000)

    GMAT_STATE_ENUM_VALUE(kMiscBumpChans, kMiscBumpDu
                                        | kMiscBumpDv
                                        | kMiscBumpDw)

    GMAT_STATE_ENUM_VALUE(kMiscNoShadowAlpha,        0x100000)

    // Use a refraction-like hack.
    GMAT_STATE_ENUM_VALUE(kMiscUseRefractionXform,   0x200000)

    // Expects tex coords to be XYZ in camera space. Does a cam to screen (not
    // NDC) projection and swaps Z with W, so that the texture projection can
    // produce projected 2D screen coordinates.
    GMAT_STATE_ENUM_VALUE(kMiscCam2Screen,           0x400000)

    GMAT_STATE_ENUM_VALUE(kAllMiscFlags,           0xffffffff)
GMAT_STATE_ENUM_END(hsGMatMiscFlags)
