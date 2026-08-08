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

#ifndef _plVulkanShaders_h_
#define _plVulkanShaders_h_

#include <cstddef>
#include <cstdint>

/** A SPIR-V module compiled into the library at build time. */
struct plVulkanShaderBlob
{
    const uint32_t* fWords;

    /** Size in bytes, which is what VkShaderModuleCreateInfo wants. */
    size_t fSizeInBytes;
};

extern const plVulkanShaderBlob kFixedPipelineVertexShader;
extern const plVulkanShaderBlob kFixedPipelineFragmentShader;

extern const plVulkanShaderBlob kPlateVertexShader;
extern const plVulkanShaderBlob kPlateFragmentShader;

extern const plVulkanShaderBlob kTextFontVertexShader;
extern const plVulkanShaderBlob kTextFontFragmentShader;

extern const plVulkanShaderBlob kShadowCasterVertexShader;
extern const plVulkanShaderBlob kShadowCasterFragmentShader;

extern const plVulkanShaderBlob kShadowApplyVertexShader;
extern const plVulkanShaderBlob kShadowApplyFragmentShader;

extern const plVulkanShaderBlob kAvatarVertexShader;
extern const plVulkanShaderBlob kAvatarFragmentShader;

extern const plVulkanShaderBlob kFullscreenVertexShader;
extern const plVulkanShaderBlob kBlurFragmentShader;
extern const plVulkanShaderBlob kGammaFragmentShader;

/*** The plShader programmable path ***/

extern const plVulkanShaderBlob kGrassVertexShader;
extern const plVulkanShaderBlob kGrassFragmentShader;

extern const plVulkanShaderBlob kCompCosinesVertexShader;
extern const plVulkanShaderBlob kCompCosinesFragmentShader;

extern const plVulkanShaderBlob kBiasNormalsVertexShader;
extern const plVulkanShaderBlob kBiasNormalsFragmentShader;

extern const plVulkanShaderBlob kWaveRipVertexShader;
extern const plVulkanShaderBlob kWaveRipFragmentShader;

extern const plVulkanShaderBlob kWaveDec1LayVertexShader;
extern const plVulkanShaderBlob kWaveDec1LayFragmentShader;

extern const plVulkanShaderBlob kWaveDecEnvVertexShader;
extern const plVulkanShaderBlob kWaveDecEnvFragmentShader;

extern const plVulkanShaderBlob kWaveFixedVertexShader;
extern const plVulkanShaderBlob kWaveFixedFragmentShader;


#endif // _plVulkanShaders_h_
