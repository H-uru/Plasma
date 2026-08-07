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

// Everything the material and shadow-apply vertex stages have in common: the
// layer specialization constants, the vertex attributes, and the two coordinate
// generators. Metal shares these as methods on VertexUniforms
// (FixedPipelineShaders.metal); GLSL has no such thing, so they live here and
// both stages include this file.
//
// Include this after `#version` and the scalar-layout extension.

//
// Specialization constants. Declared individually because that is what SPIR-V
// allows, then assembled into arrays below, exactly as the Metal shader does.
//
layout(constant_id = kSpecNumUVs)    const int numUVs = 0;
layout(constant_id = kSpecNumLayers) const int numLayers = 1;

layout(constant_id = kSpecLayerFlags + 0) const uint layerFlags0 = 0;
layout(constant_id = kSpecLayerFlags + 1) const uint layerFlags1 = 0;
layout(constant_id = kSpecLayerFlags + 2) const uint layerFlags2 = 0;
layout(constant_id = kSpecLayerFlags + 3) const uint layerFlags3 = 0;
layout(constant_id = kSpecLayerFlags + 4) const uint layerFlags4 = 0;
layout(constant_id = kSpecLayerFlags + 5) const uint layerFlags5 = 0;
layout(constant_id = kSpecLayerFlags + 6) const uint layerFlags6 = 0;
layout(constant_id = kSpecLayerFlags + 7) const uint layerFlags7 = 0;

// A global const array cannot be initialized from specialization constants, so
// the lookup is a function. The layer loop is bounded by a spec constant, so
// this folds away.
uint miscFlags(int index)
{
    switch (index) {
    case 0:  return layerFlags0;
    case 1:  return layerFlags1;
    case 2:  return layerFlags2;
    case 3:  return layerFlags3;
    case 4:  return layerFlags4;
    case 5:  return layerFlags5;
    case 6:  return layerFlags6;
    default: return layerFlags7;
    }
}

layout(constant_id = kSpecNumWeights) const int numWeights = 0;

layout(scalar, set = kDescSetUniforms, binding = kBindingVertexUniforms)
uniform VertexUniformsBlock { VertexUniforms uniforms; };

layout(location = kVtxAttrPosition)     in vec3 inPosition;
layout(location = kVtxAttrNormal)       in vec3 inNormal;
layout(location = kVtxAttrColor)        in vec4 inColor;
layout(location = kVtxAttrWeight)       in float inWeight1;
layout(location = kVtxAttrTexcoord)     in vec3 inTexCoord[kMaxLayers];


/**
 * Generates the sample coordinate for one layer.
 *
 * Ported from VertexUniforms::sampleLocation.
 */
vec3 sampleLocation(int index, vec4 cameraSpaceNormal, vec4 camPosition)
{
    const uint UVWSrc = uniforms.uvTransforms[index].UVWSrc;
    mat4 xfm = uniforms.uvTransforms[index].transform;
    const uint flags = miscFlags(index);

    if ((flags & (kMiscUseReflectionXform | kMiscUseRefractionXform)) != 0) {
        // Reflection coordinates start in camera space. Rotate them back into
        // Plasma's environment-map convention, ignoring camera translation.
        // This is the same Y/Z axis swap used by Metal and Direct3D.
        xfm = uniforms.cameraToWorldMatrix;
        xfm[0][3] = 0.0;
        xfm[1][3] = 0.0;
        xfm[2][3] = 0.0;

        for (int row = 0; row < 3; row++) {
            const float temp = xfm[1][row];
            xfm[1][row] = xfm[2][row];
            xfm[2][row] = temp;
        }

        if ((flags & kMiscUseRefractionXform) != 0) {
            xfm[0][2] = -xfm[0][2];
            xfm[1][2] = -xfm[1][2];
            xfm[2][2] = -xfm[2][2];
        }
    } else if ((flags & kMiscCam2Screen) != 0) {
        // Project camera-space positions into NDC, then scale and translate
        // them into texture space. The fragment stage performs the perspective
        // divide for kMiscPerspProjection.
        mat4 scale = mat4(1.0);
        scale[0][0] = 0.5;
        scale[1][1] = -0.5;

        mat4 translation = mat4(1.0);
        translation[0][3] = 0.5;
        translation[1][3] = -0.5;

        mat4 cameraToScreen = scale * translation;

        // Move clip W into sampleCoord.z, where sampleLayer() expects the
        // projective divisor, while retaining the depth component in W.
        float temp = cameraToScreen[2][2];
        cameraToScreen[2][2] = cameraToScreen[3][2];
        cameraToScreen[3][2] = temp;

        temp = cameraToScreen[2][3];
        cameraToScreen[2][3] = cameraToScreen[3][3];
        cameraToScreen[3][3] = temp;

        xfm = uniforms.projectionMatrix * cameraToScreen;
    } else if ((flags & kMiscProjection) != 0) {
        mat4 cam2World = uniforms.cameraToWorldMatrix;
        if ((UVWSrc & kUVWPosition) == 0) {
            cam2World[0][3] = 0.0;
            cam2World[1][3] = 0.0;
            cam2World[2][3] = 0.0;
        }
        xfm = cam2World * xfm;
    }

    vec4 sampleCoord;
    switch (UVWSrc) {
    case kUVWNormal:
        sampleCoord = cameraSpaceNormal * xfm;
        break;
    case kUVWPosition:
        sampleCoord = camPosition * xfm;
        break;
    case kUVWReflect:
        sampleCoord = reflect(normalize(camPosition), cameraSpaceNormal) * xfm;
        break;
    default:
        {
            const int uvIndex = int(UVWSrc & 0x0F);
            // DX uses 0,0 when the index is out of range; match that.
            sampleCoord = (uvIndex < numUVs) ? vec4(inTexCoord[uvIndex], 1.0) * xfm
                                             : vec4(0.0);
        }
        break;
    }
    return sampleCoord.xyz;
}

/** Ported from VertexUniforms::calcFog. Alpha carries the fog factor. */
vec4 calcFog(vec4 camPosition)
{
    float density;
    if (uniforms.fogExponential > 0) {
        density = exp(-pow(uniforms.fogValues.y * length(camPosition), uniforms.fogValues.x));
    } else if (uniforms.fogValues.y > 0.0) {
        const float start = uniforms.fogValues.x;
        const float end = uniforms.fogValues.y;
        density = (end - length(camPosition.xyz)) / (end - start);
    } else {
        density = 1.0;
    }
    return vec4(uniforms.fogColor, density);
}
