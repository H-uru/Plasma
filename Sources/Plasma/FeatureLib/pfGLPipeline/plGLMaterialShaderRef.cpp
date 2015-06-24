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

#include <epoxy/gl.h>
#include <string_theory/format>
#include <string_theory/string_stream>

#include "HeadSpin.h"
#include "hsBitVector.h"

#include "plPipeline.h"
#include "plPipeDebugFlags.h"

#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plMipmap.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

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

        if (layer->GetUVWSrc() & (plLayerInterface::kUVWNormal | plLayerInterface::kUVWPosition | plLayerInterface::kUVWReflect)) {
            // Now we have a different problem...
            continue;
        }

        // Load the image
        plMipmap* img = plMipmap::ConvertNoRef(layer->GetTexture());
        if (!img)
            continue;

        GLenum e;
        plGLTextureRef* texRef = static_cast<plGLTextureRef*>(img->GetDeviceRef());

        if (!texRef->fRef)
            continue;

        LOG_GL_ERROR_CHECK("PRE-Active Texture failed")

        glActiveTexture(GL_TEXTURE0 + numTextures);
        LOG_GL_ERROR_CHECK("Active Texture failed")

        glBindTexture(GL_TEXTURE_2D, texRef->fRef);
        LOG_GL_ERROR_CHECK("Bind Texture failed")

        ST::string name = ST::format("uTexture{}", numTextures);

        GLint texture = glGetUniformLocation(fRef, name.c_str());
        glUniform1i(texture, numTextures);
        LOG_GL_ERROR_CHECK("Uniform Texture failed")

        numTextures++;
    }
}


void plGLMaterialShaderRef::ICompile()
{
    hsBitVector uvLayers;
    fNumUVs = 0;
    int32_t numTextures = 0;

    for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        if (!layer)
            continue;

        if (layer->GetUVWSrc() & (plLayerInterface::kUVWNormal | plLayerInterface::kUVWPosition | plLayerInterface::kUVWReflect)) {
            // Now we have a different problem...
            continue;
        }

        uint32_t uv = layer->GetUVWSrc() & plGBufferGroup::kUVCountMask;

        if (!uvLayers.IsBitSet(uv)) {
            fNumUVs++;
            uvLayers.SetBit(uv);
        }


        // Load the image
        plMipmap* img = plMipmap::ConvertNoRef(layer->GetTexture());
        if (!img)
            continue;

        numTextures++;

        plGLTextureRef* texRef = new plGLTextureRef();
        texRef->fOwner = img;
        img->SetDeviceRef(texRef);

        GLenum e;

        glGenTextures(1, &texRef->fRef);
        LOG_GL_ERROR_CHECK("Gen Texture failed")

        glBindTexture(GL_TEXTURE_2D, texRef->fRef);
        LOG_GL_ERROR_CHECK("Bind Texture failed")

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        if (img->IsCompressed()) {
            GLuint dxCompression = 0;
            uint8_t compType = img->fDirectXInfo.fCompressionType;

            if (compType == plBitmap::DirectXInfo::kDXT1)
                dxCompression = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            else if (compType == plBitmap::DirectXInfo::kDXT5)
                dxCompression = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

            for (uint8_t i = 0; i < img->GetNumLevels(); i++) {
                img->SetCurrLevel(i);

                if (img->GetCurrWidth() < 4 || img->GetCurrHeight() < 4)
                    continue;

                glCompressedTexImage2D(GL_TEXTURE_2D, i, dxCompression,
                                        img->GetCurrWidth(), img->GetCurrHeight(),
                                        0, img->GetCurrLevelSize(), img->GetCurrLevelPtr());

#ifdef HS_DEBUGGING
                if ((e = glGetError()) != GL_NO_ERROR) {
                    plStatusLog::AddLineSF("pipeline.log", "Texture Image failed: {}, at level {}", uint32_t(e), i);
                }
#endif
            }
        }
    }

    ST::string_stream vs_src;

    vs_src << "#version 100" << "\n";
    vs_src << "\n";
    vs_src << "uniform mat4 uMatrixL2W;" << "\n";
    vs_src << "uniform mat4 uMatrixW2C;" << "\n";
    vs_src << "uniform mat4 uMatrixProj;" << "\n";
    vs_src << "\n";
    vs_src << "attribute vec3 aVtxPosition;" << "\n";
    vs_src << "attribute vec3 aVtxNormal;" << "\n";
    vs_src << "attribute vec4 aVtxColor;" << "\n";
    for (int32_t i = 0; i < fNumUVs; i++) {
        vs_src << "attribute vec3 aVtxUVWSrc" << i << ";" << "\n";
    }
    vs_src << "\n";
    vs_src << "varying vec3 vVtxNormal;" << "\n";
    vs_src << "varying vec4 vVtxColor;" << "\n";
    for (int32_t i = 0; i < fNumUVs; i++) {
        vs_src << "varying vec3 vVtxUVWSrc" << i << ";" << "\n";
    }
    vs_src << "\n";
    vs_src << "void main() {" << "\n";
    vs_src << "    vVtxNormal = aVtxNormal;" << "\n";
    vs_src << "    vVtxColor = aVtxColor.zyxw;" << "\n";
    for (int32_t i = 0; i < fNumUVs; i++) {
        vs_src << "    vVtxUVWSrc" << i << " = aVtxUVWSrc" << i << ";" << "\n";
    }
    vs_src << "\n";
    vs_src << "    vec4 pos = uMatrixL2W * vec4(aVtxPosition, 1.0);" << "\n";
    vs_src << "         pos = uMatrixW2C * pos;" << "\n";
    vs_src << "         pos = uMatrixProj * pos;" << "\n";
    vs_src << "\n";
    vs_src << "    gl_Position = pos;" << "\n";
    vs_src << "}";



    ST::string_stream fs_src;
    fs_src << "#version 100" << "\n";
    fs_src << "\n";
    for (int32_t i = 0; i < numTextures; i++) {
        fs_src << "uniform sampler2D uTexture" << i << ";" << "\n";
    }
    fs_src << "\n";
    fs_src << "varying lowp vec3 vVtxNormal;" << "\n";
    fs_src << "varying mediump vec4 vVtxColor;" << "\n";
    for (int32_t i = 0; i < fNumUVs; i++) {
        fs_src << "varying mediump vec3 vVtxUVWSrc" << i << ";" << "\n";
    }
    fs_src << "\n";
    fs_src << "void main() {" << "\n";
    fs_src << "    mediump vec4 color = vVtxColor;" << "\n";
    /*if (numTextures > 0) {
        fs_src << "    color *= texture2D(uTexture0, vec2(vVtxUVWSrc0.x, vVtxUVWSrc0.y));" << "\n";
    }*/

    for (int32_t i = 0, tex = 0; i < fMaterial->GetNumLayers(); i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        if (!layer)
            continue;

        if (layer->GetUVWSrc() & (plLayerInterface::kUVWNormal | plLayerInterface::kUVWPosition | plLayerInterface::kUVWReflect)) {
            // Now we have a different problem...
            continue;
        }

        plMipmap* img = plMipmap::ConvertNoRef(layer->GetTexture());
        if (!img)
            continue;

        uint32_t uv = layer->GetUVWSrc() & plGBufferGroup::kUVCountMask;

        fs_src << "    color *= texture2D(uTexture" << tex << ", vec2(vVtxUVWSrc" << uv << ".x, vVtxUVWSrc" << uv << ".y));" << "\n";
        tex++;
    }

    fs_src << "    gl_FragColor = color;" << "\n";
    fs_src << "}";

    ST::string vtx = vs_src.to_string();
    ST::string frg = fs_src.to_string();

    const char* vs_code = vtx.c_str();
    const char* fs_code = frg.c_str();

    fVertShaderRef = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(fVertShaderRef, 1, &vs_code, nullptr);
    glCompileShader(fVertShaderRef);

#ifdef HS_DEBUGGING
    {
        GLint compiled = 0;
        glGetShaderiv(fVertShaderRef, GL_COMPILE_STATUS, &compiled);
        if (compiled == 0) {
            GLint length = 0;
            glGetShaderiv(fVertShaderRef, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char* log = new char[length];
                glGetShaderInfoLog(fVertShaderRef, length, &length, log);
                hsStatusMessage(log);
            }
        }
    }
#endif

    fFragShaderRef = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fFragShaderRef, 1, &fs_code, nullptr);
    glCompileShader(fFragShaderRef);

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
            }
        }
    }
#endif

    fRef = glCreateProgram();
    LOG_GL_ERROR_CHECK("Create Program failed")

    glAttachShader(fRef, fVertShaderRef);
    LOG_GL_ERROR_CHECK("Attach Vertex Shader failed")

    glAttachShader(fRef, fFragShaderRef);
    LOG_GL_ERROR_CHECK("Attach Fragment Shader failed")

    glLinkProgram(fRef);
    LOG_GL_ERROR_CHECK("Link Program failed")
}


bool plGLMaterialShaderRef::ILoopOverLayers()
{
    size_t j = 0;
    for (j = 0; j < fMaterial->GetNumLayers(); )
    {
        size_t iCurrMat = j;
        hsGMatState currState;

        j = IHandleMaterial(iCurrMat, currState);

        if (j == -1)
            break;

        fPasses++;
        fPassStates.push_back(currState);

#if 0
        ISetFogParameters(fMaterial->GetLayer(iCurrMat));
#endif
    }

    return false;
}


uint32_t plGLMaterialShaderRef::IHandleMaterial(uint32_t layer, hsGMatState& state)
{
    if (!fMaterial || layer >= fMaterial->GetNumLayers() || !fMaterial->GetLayer(layer))
        return -1;

    if (false /*ISkipBumpMap(fMaterial, layer)*/)
        return -1;

    // Ignoring the bit about ATI Radeon and UVW limits

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoDecals) && (fMaterial->GetCompositeFlags() & hsGMaterial::kCompDecal))
        return -1;

    // Ignoring the bit about self-rendering cube maps

    plLayerInterface* currLay = /*IPushOverBaseLayer*/ fMaterial->GetLayer(layer);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW) && (currLay->GetMiscFlags() & hsGMatState::kMiscBumpDu))
        currLay = fMaterial->GetLayer(++layer);

    //currLay = IPushOverAllLayer(currLay);

    state = currLay->GetState();

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagDisableSpecular))
        state.fShadeFlags &= ~hsGMatState::kShadeSpecular;

    if (state.fZFlags & hsGMatState::kZIncLayer) {
        // Set the Z-bias
        //ISetLayer(1);
    } else {
        // Clear any Z-bias
        //IBottomLayer();
    }

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

#if 0
    ICalcLighting(currLay);
#endif

    if (state.fMiscFlags & (hsGMatState::kMiscBumpDu | hsGMatState::kMiscBumpDw)) {
        //ISetBumpMatrices(currLay);
    }

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
