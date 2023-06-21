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

#include "plGLMaterialShaderRef.h"
#include "plGLDevice.h"
#include "plGLPipeline.h"

#include <epoxy/gl.h>
#include <string_theory/format>
#include <string_theory/string_stream>

#include "HeadSpin.h"
#include "hsBitVector.h"

#include "hsGMatState.inl"
#include "plPipeDebugFlags.h"

#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plRenderTarget.h"
#include "plStatusLog/plStatusLog.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

// From plGLDevice.cpp
extern GLfloat* hsMatrix2GL(const hsMatrix44& src, GLfloat* dst);

const char* VERTEX_SHADER_STRING = R"(#version 330
precision lowp int;

struct lightSource {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 direction;
    vec3 spotProps; // (falloff, theta, phi)
    float constAtten;
    float linAtten;
    float quadAtten;
    float scale;
};


// Input Attributes
layout(location =  0) in vec3 aVtxPosition;
layout(location =  1) in vec3 aVtxNormal;
layout(location =  2) in vec4 aVtxColor;
layout(location =  3) in vec3 aVtxUVWSrc[8];

uniform mat4 uMatrixL2W;
uniform mat4 uMatrixW2L;
uniform mat4 uMatrixW2C;
uniform mat4 uMatrixProj;

uniform vec4 uGlobalAmb;
uniform vec4 uAmbientCol;
uniform float uAmbientSrc;
uniform vec4 uDiffuseCol;
uniform float uDiffuseSrc;
uniform vec4 uEmissiveCol;
uniform float uEmissiveSrc;
uniform vec4 uSpecularCol;
uniform float uSpecularSrc;

uniform float uInvertVtxAlpha; // Effectively boolean, 0.0 or 1.0

uniform lightSource uLampSources[8];

// Varying outputs
out vec4 vCamPosition;
out vec4 vCamNormal;
out vec4 vVtxColor;
out vec3 vVtxUVWSrc[8];

void main() {
    vec4 MAmbient = mix(aVtxColor.bgra, uAmbientCol, uAmbientSrc);
    vec4 MDiffuse = mix(aVtxColor.bgra, uDiffuseCol, uDiffuseSrc);
    vec4 MEmissive = mix(aVtxColor.bgra, uEmissiveCol, uEmissiveSrc);
    vec4 MSpecular = mix(aVtxColor.bgra, uSpecularCol, uSpecularSrc);

    vec4 LAmbient = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 LDiffuse = vec4(0.0, 0.0, 0.0, 0.0);

    vec3 Ndirection = normalize(mat3(uMatrixW2L) * aVtxNormal);

    for (int i = 0; i < 8; i++) {
        vVtxUVWSrc[i] = aVtxUVWSrc[i];

        float attenuation;
        vec3 direction;

        if (uLampSources[i].position.w == 0.0) {
            // Directional Light with no attenuation
            direction = -uLampSources[i].direction;
            attenuation = 1.0;
        } else {
            // Omni Light in all directions
            vec3 v2l = uLampSources[i].position.xyz - vec3(uMatrixL2W * vec4(aVtxPosition, 1.0));
            float distance = length(v2l);
            direction = normalize(v2l);

            attenuation = 1.0 / (uLampSources[i].constAtten + uLampSources[i].linAtten * distance + uLampSources[i].quadAtten * pow(distance, 2.0));

            if (uLampSources[i].spotProps.x > 0.0) {
                // Spot Light with cone falloff
                float a = dot(direction, normalize(-uLampSources[i].direction));
                float theta = uLampSources[i].spotProps.y;
                float phi = uLampSources[i].spotProps.z;
                float i = pow((a - phi) / (theta - phi), uLampSources[i].spotProps.x);

                attenuation *= clamp(i, 0.0, 1.0);
            }
        }

        LAmbient.rgb = LAmbient.rgb + attenuation * (uLampSources[i].ambient.rgb * uLampSources[i].scale);
        LDiffuse.rgb = LDiffuse.rgb + MDiffuse.rgb * (uLampSources[i].diffuse.rgb * uLampSources[i].scale) * max(0.0, dot(Ndirection, direction)) * attenuation;
    }

    vec4 ambient = clamp(MAmbient * (uGlobalAmb + LAmbient), 0.0, 1.0);
    vec4 diffuse = clamp(LDiffuse, 0.0, 1.0);
    vec4 material = clamp(ambient + diffuse + MEmissive, 0.0, 1.0);

    vVtxColor = vec4(material.rgb, abs(uInvertVtxAlpha - MDiffuse.a));

    vCamPosition = uMatrixW2C * (uMatrixL2W * vec4(aVtxPosition, 1.0));
    vCamNormal = uMatrixW2C * (uMatrixL2W * vec4(aVtxNormal, 0.0));

    gl_Position = uMatrixProj * vCamPosition;
})";


const char* FRAGMENT_SHADER_STRING = R"(#version 330
precision mediump float;
precision lowp int;

uniform mat4 uLayerMat0;

uniform mat4 uLayerMat[8];
uniform int uLayerUVWSrc[8];
uniform int uStartingLayer;
uniform int uLayersAtOnce;
uniform int uPassNumber;

uniform sampler2D uTexture0;

uniform float uAlphaThreshold;
uniform int uFogExponential;
uniform highp vec2 uFogValues;
uniform vec3 uFogColor;

// Varying inputs
in highp vec4 vCamPosition;
in highp vec4 vCamNormal;
in vec4 vVtxColor;
in highp vec3 vVtxUVWSrc[8];

// Rendered outputs
out vec4 fragColor;

void main() {
    float baseAlpha;
    vec4 coords0;
    vec4 image0;
    float currAlpha;
    vec3 currColor;

    baseAlpha = vVtxColor.a;
    coords0 = uLayerMat0 * vec4(vVtxUVWSrc[0], 1.0);
    image0 = texture(uTexture0, coords0.xy);
    currColor = image0.rgb;
    currAlpha = image0.a * baseAlpha;
    currColor = vVtxColor.rgb * currColor;

    if (currAlpha < uAlphaThreshold) { discard; };

    highp float fogFactor = 1.0;
    if (uFogExponential > 0) {
        fogFactor = exp(-pow(uFogValues.y * length(vCamPosition.xyz), uFogValues.x));
    } else {
        if (uFogValues.y > 0.0) {
            highp float start = uFogValues.x;
            highp float end = uFogValues.y;
            fogFactor = (end - length(vCamPosition.xyz)) / (end - start);
        }
    }

    currColor = mix(currColor, uFogColor, 1.0 - clamp(fogFactor, 0.0, 1.0));

    fragColor = vec4(currColor, currAlpha);
})";

plGLMaterialShaderRef::plGLMaterialShaderRef(hsGMaterial* mat, plGLPipeline* pipe)
    : plGLDeviceRef(), fMaterial(mat), fPipeline(pipe), fVertShaderRef(0), fFragShaderRef(0)
{
    ILoopOverLayers();
    ICompile();
    ISetShaderVariableLocs();
}

plGLMaterialShaderRef::~plGLMaterialShaderRef()
{
    Release();
}

void plGLMaterialShaderRef::Release()
{
    if (fVertShaderRef) {
        glDeleteShader(fVertShaderRef);
        fVertShaderRef = 0;
    }

    if (fFragShaderRef) {
        glDeleteShader(fFragShaderRef);
        fFragShaderRef = 0;
    }

    if (fRef) {
        glDeleteProgram(fRef);
        fRef = 0;
    }

    SetDirty(true);
}

void plGLMaterialShaderRef::SetupTextureRefs()
{
    int32_t numTextures = 0;

    for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        if (!layer)
            continue;

        layer = fPipeline->IPushOverAllLayer(layer);

        // Load the image
        plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());

        if (!img) {
            layer = fPipeline->IPopOverAllLayer(layer);
            continue;
        }

        fPipeline->CheckTextureRef(layer);

        plGLTextureRef* texRef = static_cast<plGLTextureRef*>(img->GetDeviceRef());

        if (!texRef->fRef) {
            layer = fPipeline->IPopOverAllLayer(layer);
            continue;
        }

        LOG_GL_ERROR_CHECK("PRE-Active Texture failed")

        glActiveTexture(GL_TEXTURE0 + numTextures);
        LOG_GL_ERROR_CHECK("Active Texture failed")

        glBindTexture(texRef->fMapping, texRef->fRef);
        LOG_GL_ERROR_CHECK("Bind Texture failed");

        if (texRef->fMapping == GL_TEXTURE_2D) {
            // Ewww, but the same texture might be used by multiple layers with different clamping flags :(
            switch (layer->GetClampFlags()) {
            case hsGMatState::kClampTextureU:
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_T, GL_REPEAT);
                break;
            case hsGMatState::kClampTextureV:
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
            case hsGMatState::kClampTexture:
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                break;
            default:
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(texRef->fMapping, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
        }

        if (this->uLayerMat[i] != -1) {
            GLfloat matrix[16];
            glUniformMatrix4fv(this->uLayerMat[i], 1, GL_TRUE, hsMatrix2GL(layer->GetTransform(), matrix));
        }

        if (this->uTexture[i] != -1)
            glUniform1i(this->uTexture[i], numTextures);
        LOG_GL_ERROR_CHECK("Uniform Texture failed")

        layer = fPipeline->IPopOverAllLayer(layer);
        numTextures++;
    }
}


void plGLMaterialShaderRef::ICompile()
{
    LOG_GL_ERROR_CHECK("Begin Compile failed")

    int32_t numTextures = 0;
    hsBitVector usedUVWs;

    for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        if (!layer)
            continue;

        fPipeline->CheckTextureRef(layer);
        LOG_GL_ERROR_CHECK(ST::format("Check Texture Ref on layer \"{}\" failed", layer->GetKeyName()));
    }

    const char* vs_code = VERTEX_SHADER_STRING;
    const char* fs_code = FRAGMENT_SHADER_STRING;

    static GLuint vshader = 0;
    if (!vshader) {
        vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vs_code, nullptr);
        glCompileShader(vshader);
        LOG_GL_ERROR_CHECK("Vertex Shader compile failed");

#ifdef HS_DEBUGGING
        {
            GLint compiled = 0;
            glGetShaderiv(vshader, GL_COMPILE_STATUS, &compiled);
            if (compiled == 0) {
                GLint length = 0;
                glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &length);
                if (length) {
                    char* log = new char[length];
                    glGetShaderInfoLog(vshader, length, &length, log);
                    hsStatusMessage(log);
                    delete[] log;
                }
            }
        }
#endif
    }
    fVertShaderRef = vshader;

    fFragShaderRef = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fFragShaderRef, 1, &fs_code, nullptr);
    glCompileShader(fFragShaderRef);
    LOG_GL_ERROR_CHECK("Fragment Shader compile failed");

#ifdef HS_DEBUGGING
    {
        GLint compiled = 0;
        glGetShaderiv(fFragShaderRef, GL_COMPILE_STATUS, &compiled);
        if (compiled == 0) {
            GLint length = 0;
            glGetShaderiv(fFragShaderRef, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char* log = new char[length];
                glGetShaderInfoLog(fFragShaderRef, length, &length, log);
                hsStatusMessage(log);
                delete[] log;
            }
        }
    }
#endif

    fRef = glCreateProgram();
    LOG_GL_ERROR_CHECK("Create Program failed");

    if (plGLVersion() >= 43) {
        const char* name = ST::format("hsGMaterial::{}", fMaterial->GetKeyName()).c_str();
        glObjectLabel(GL_PROGRAM, fRef, strlen(name), name);
    }

    glAttachShader(fRef, fVertShaderRef);
    LOG_GL_ERROR_CHECK("Attach Vertex Shader failed")

    glAttachShader(fRef, fFragShaderRef);
    LOG_GL_ERROR_CHECK("Attach Fragment Shader failed")
}


void plGLMaterialShaderRef::ISetShaderVariableLocs()
{
    glLinkProgram(fRef);
    LOG_GL_ERROR_CHECK("Program Link failed");

    uPassNumber       = glGetUniformLocation(fRef, "uPassNumber");
    uAlphaThreshold   = glGetUniformLocation(fRef, "uAlphaThreshold");
    uInvertVtxAlpha   = glGetUniformLocation(fRef, "uInvertVtxAlpha");

    // Matrix inputs
    uMatrixProj     = glGetUniformLocation(fRef, "uMatrixProj");
    uMatrixC2W      = glGetUniformLocation(fRef, "uMatrixC2W");
    uMatrixW2C      = glGetUniformLocation(fRef, "uMatrixW2C");
    uMatrixL2W      = glGetUniformLocation(fRef, "uMatrixL2W");
    uMatrixW2L      = glGetUniformLocation(fRef, "uMatrixW2L");

    // Material inputs
    uGlobalAmbient  = glGetUniformLocation(fRef, "uGlobalAmb");
    uMatAmbientCol  = glGetUniformLocation(fRef, "uAmbientCol");
    uMatAmbientSrc  = glGetUniformLocation(fRef, "uAmbientSrc");
    uMatDiffuseCol  = glGetUniformLocation(fRef, "uDiffuseCol");
    uMatDiffuseSrc  = glGetUniformLocation(fRef, "uDiffuseSrc");
    uMatEmissiveCol = glGetUniformLocation(fRef, "uEmissiveCol");
    uMatEmissiveSrc = glGetUniformLocation(fRef, "uEmissiveSrc");
    uMatSpecularCol = glGetUniformLocation(fRef, "uSpecularCol");
    uMatSpecularSrc = glGetUniformLocation(fRef, "uSpecularSrc");

    // Fog inputs
    uFogExponential = glGetUniformLocation(fRef, "uFogExponential");
    uFogValues      = glGetUniformLocation(fRef, "uFogValues");
    uFogColor       = glGetUniformLocation(fRef, "uFogColor");

    // Lamp inputs
    for (size_t i = 0; i < 8; i++) {
        uLampSources[i].position     = glGetUniformLocation(fRef, ST::format("uLampSources[{}].position", i).c_str());
        uLampSources[i].ambient      = glGetUniformLocation(fRef, ST::format("uLampSources[{}].ambient", i).c_str());
        uLampSources[i].diffuse      = glGetUniformLocation(fRef, ST::format("uLampSources[{}].diffuse", i).c_str());
        uLampSources[i].specular     = glGetUniformLocation(fRef, ST::format("uLampSources[{}].specular", i).c_str());
        uLampSources[i].direction    = glGetUniformLocation(fRef, ST::format("uLampSources[{}].direction", i).c_str());
        uLampSources[i].spotProps    = glGetUniformLocation(fRef, ST::format("uLampSources[{}].spotProps", i).c_str());
        uLampSources[i].constAtten   = glGetUniformLocation(fRef, ST::format("uLampSources[{}].constAtten", i).c_str());
        uLampSources[i].linAtten     = glGetUniformLocation(fRef, ST::format("uLampSources[{}].linAtten", i).c_str());
        uLampSources[i].quadAtten    = glGetUniformLocation(fRef, ST::format("uLampSources[{}].quadAtten", i).c_str());
        uLampSources[i].scale        = glGetUniformLocation(fRef, ST::format("uLampSources[{}].scale", i).c_str());
    }

    size_t layerCount = fMaterial->GetNumLayers();

    uLayerMat.assign(layerCount, -1);
    for (size_t i = 0; i < layerCount; i++) {
        ST::string name = ST::format("uLayerMat{}", i);
        uLayerMat[i] = glGetUniformLocation(fRef, name.c_str());
    }

    uTexture.assign(layerCount, -1);
    for (size_t i = 0; i < layerCount; i++) {
        ST::string name = ST::format("uTexture{}", i);
        uTexture[i] = glGetUniformLocation(fRef, name.c_str());
    }
}


void plGLMaterialShaderRef::ILoopOverLayers()
{
    for (size_t j = 0; j < fMaterial->GetNumLayers(); ) {
        size_t iCurrMat = j;

        j = IHandleMaterial(iCurrMat);

        if (j == -1)
            break;

        fPassIndices.push_back(iCurrMat);
    }
}


const hsGMatState plGLMaterialShaderRef::ICompositeLayerState(plLayerInterface* layer)
{
    hsGMatState state;
    state.Composite(layer->GetState(), fPipeline->GetMaterialOverride(true), fPipeline->GetMaterialOverride(false));
    return state;
}


uint32_t plGLMaterialShaderRef::IHandleMaterial(uint32_t layer)
{
    if (!fMaterial || layer >= fMaterial->GetNumLayers() || !fMaterial->GetLayer(layer))
        return -1;

    if (false /*ISkipBumpMap(fMaterial, layer)*/)
        return -1;

    // Ignoring the bit about ATI Radeon and UVW limits

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoDecals) && (fMaterial->GetCompositeFlags() & hsGMaterial::kCompDecal))
        return -1;

    // Ignoring the bit about self-rendering cube maps

    plLayerInterface* currLay = fPipeline->IPushOverBaseLayer(fMaterial->GetLayer(layer));

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW) && (currLay->GetMiscFlags() & hsGMatState::kMiscBumpDu))
        currLay = fMaterial->GetLayer(++layer);

    currLay = fPipeline->IPushOverAllLayer(currLay);

    hsGMatState state = ICompositeLayerState(currLay);

    // Stuff about ZInc

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagDisableSpecular))
        state.fShadeFlags &= ~hsGMatState::kShadeSpecular;

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoAlphaBlending))
        state.fBlendFlags &= ~hsGMatState::kBlendMask;

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (state.fMiscFlags & hsGMatState::kMiscBumpChans) ) {
        switch (state.fMiscFlags & hsGMatState::kMiscBumpChans)
        {
            case hsGMatState::kMiscBumpDu:
                break;
            case hsGMatState::kMiscBumpDv:
                if (!(fMaterial->GetLayer(layer-2)->GetBlendFlags() & hsGMatState::kBlendAdd))
                {
                    state.fBlendFlags &= ~hsGMatState::kBlendMask;
                    state.fBlendFlags |= hsGMatState::kBlendMADD;
                }
                break;
            case hsGMatState::kMiscBumpDw:
                if (!(fMaterial->GetLayer(layer-1)->GetBlendFlags() & hsGMatState::kBlendAdd))
                {
                    state.fBlendFlags &= ~hsGMatState::kBlendMask;
                    state.fBlendFlags |= hsGMatState::kBlendMADD;
                }
                break;
            default:
                break;
        }
    }

    uint32_t currNumLayers = ILayersAtOnce(layer);

    if (state.fMiscFlags & (hsGMatState::kMiscBumpDu | hsGMatState::kMiscBumpDw)) {
        //ISetBumpMatrices(currLay);
    }

    currLay = fPipeline->IPopOverAllLayer(currLay);
    currLay = fPipeline->IPopOverBaseLayer(currLay);

    return layer + currNumLayers;
}


uint32_t plGLMaterialShaderRef::ILayersAtOnce(uint32_t which)
{
    uint32_t currNumLayers = 1;

    plLayerInterface* lay = fMaterial->GetLayer(which);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoMultitexture))
        return currNumLayers;

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (lay->GetMiscFlags() & hsGMatState::kMiscBumpChans)) {
        currNumLayers = 2;
        return currNumLayers;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) || (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner))
        return currNumLayers;

    int i;
    int maxLayers = 8;
    if (which + maxLayers > fMaterial->GetNumLayers())
        maxLayers = fMaterial->GetNumLayers() - which;

    for (i = currNumLayers; i < maxLayers; i++) {
        plLayerInterface* lay = fMaterial->GetLayer(which + i);

        // Ignoring max UVW limit

        if ((lay->GetMiscFlags() & hsGMatState::kMiscBindNext) && (i+1 >= maxLayers))
            break;

        if (lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere)
            break;

        if (!(fMaterial->GetLayer(which + i - 1)->GetMiscFlags() & hsGMatState::kMiscBindNext) && !ICanEatLayer(lay))
            break;

        currNumLayers++;
    }

    return currNumLayers;
}


bool plGLMaterialShaderRef::ICanEatLayer(plLayerInterface* lay)
{
    if (!lay->GetTexture())
        return false;

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) ||
        (lay->GetBlendFlags() & hsGMatState::kBlendAddColorTimesAlpha) ||
        (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner))
        return false;

    if ((lay->GetBlendFlags() & hsGMatState::kBlendAlpha) && (lay->GetAmbientColor().a < 1.f))
        return false;

    if (!(lay->GetZFlags() & hsGMatState::kZNoZWrite))
        return false;

    return true;
}
