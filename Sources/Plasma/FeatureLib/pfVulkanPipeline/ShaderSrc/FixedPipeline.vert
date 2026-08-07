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

// Ported from pfMetalPipeline/ShaderSrc/FixedPipelineShaders.metal.
//
// Plasma multiplies row vectors by matrices (`v * M`) throughout. GLSL gives
// `vec4 * mat4` the same meaning as MSL, so the math here is the original.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"
#include "FixedFunctionVertex.glsl"

layout(constant_id = kSpecPerPixelLighting) const bool perPixelLighting = false;

layout(scalar, set = kDescSetUniforms, binding = kBindingMaterial)
uniform MaterialBlock { plMaterialLightingDescriptor material; };

layout(scalar, set = kDescSetUniforms, binding = kBindingLights)
readonly buffer LightBlock { plShaderLightSource lights[]; };

/**
 * Ported from calcLitMaterialColor (FixedPipelineShaders.metal:187-247).
 *
 * The *Src fields are 0/1 mix selectors between the vertex color and the
 * material color, which is how Plasma's three kLiteMask lighting models are
 * expressed without branching.
 */
vec4 calcLitMaterialColor(vec4 materialColor, vec4 position, vec3 normal)
{
    vec3 LAmbient = vec3(0.0);
    vec3 LDiffuse = vec3(0.0);

    const vec3 MAmbient = mix(materialColor.rgb, material.ambientCol, float(material.ambientSrc));
    const vec4 MDiffuse = mix(materialColor, material.diffuseCol, float(material.diffuseSrc));
    const vec3 MEmissive = mix(materialColor.rgb, material.emissiveCol, float(material.emissiveSrc));

    for (uint i = 0u; i < material.lightCount; i++) {
        const uint index = material.activeLights[i].index;
        const float lightScale = material.activeLights[i].scale;
        if (lightScale == 0.0)
            continue;

        // w carries the attenuation for this fragment.
        vec4 direction;

        if (lights[index].position.w == 0.0) {
            // Directional: no attenuation, no range test.
            direction = vec4(-lights[index].direction.xyz, 1.0);
        } else {
            const vec3 v2l = lights[index].position.xyz - position.xyz;
            const float dist = length(v2l);

            if (dist > lights[index].range)
                continue;

            direction.xyz = normalize(v2l);
            direction.w = 1.0 / (lights[index].constAtten +
                                 lights[index].linAtten * dist +
                                 lights[index].quadAtten * dist * dist);

            if (lights[index].spotProps.x > 0.0) {
                const float theta = dot(direction.xyz, normalize(-lights[index].direction.xyz));
                const float gamma = lights[index].spotProps.y;  // inner cutoff
                const float phi = lights[index].spotProps.z;    // outer cutoff
                const float intensity = clamp((theta - phi) / (gamma - phi), 0.0, 1.0);

                direction.w *= pow(intensity, lights[index].spotProps.x);
            }
        }

        LAmbient += direction.w * (lights[index].ambient.rgb * lightScale);
        LDiffuse += MDiffuse.rgb * (lights[index].diffuse.rgb * lightScale) *
                    max(0.0, dot(normal, direction.xyz)) * direction.w;
    }

    const vec3 ambient = MAmbient * clamp(material.globalAmb.rgb + LAmbient, 0.0, 1.0);
    const vec3 diffuse = clamp(LDiffuse, 0.0, 1.0);

    return clamp(vec4(ambient + diffuse + MEmissive,
                      abs(float(material.invertAlpha) - MDiffuse.a)), 0.0, 1.0);
}


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outFogColor;
layout(location = 2) out vec3 outTexCoord[kMaxLayers];

// Only read when perPixelLighting; the fragment stage needs world space then.
layout(location = 10) out vec4 outWorldPos;
layout(location = 11) out vec3 outNormal;

void main()
{
    // Plasma stores vertex color as little-endian ARGB, so the attribute comes
    // in as BGRA and has to be swizzled.
    const vec4 vertexColor = inColor.bgra;

    vec4 position = vec4(inPosition, 1.0) * uniforms.localToWorldMatrix;

    // One weight is blended on the GPU between the span's transform and the
    // next palette matrix. More than one means ISoftwareVertexBlend already
    // folded everything in and localToWorldMatrix is identity.
    if (numWeights == 1) {
        vec4 position2 = vec4(inPosition, 1.0) * uniforms.blendMatrix1;
        position = (inWeight1 * position) + ((1.0 - inWeight1) * position2);
    }

    const vec3 worldNormal =
        normalize((vec4(inNormal, 0.0) * uniforms.localToWorldMatrix).xyz);

    if (perPixelLighting) {
        // Hand world space to the fragment stage and light it there.
        outWorldPos = position;
        outNormal = worldNormal;
        outColor = vertexColor;
    } else {
        outWorldPos = vec4(0.0);
        outNormal = vec3(0.0);
        outColor = calcLitMaterialColor(vertexColor, position, worldNormal);
    }

    const vec4 camPosition = position * uniforms.worldToCameraMatrix;
    outFogColor = calcFog(camPosition);

    const vec4 cameraSpaceNormal =
        normalize((vec4(inNormal, 0.0) * uniforms.localToWorldMatrix) * uniforms.worldToCameraMatrix);

    for (int layer = 0; layer < numLayers; layer++)
        outTexCoord[layer] = sampleLocation(layer, cameraSpaceNormal, camPosition);

    gl_Position = camPosition * uniforms.projectionMatrix;
}
