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
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

// From plGLDevice.cpp
extern GLfloat* hsMatrix2GL(const hsMatrix44& src, GLfloat* dst);

plGLMaterialShaderRef::plGLMaterialShaderRef(hsGMaterial* mat, plPipeline* pipe)
    : plGLDeviceRef(), fMaterial(mat), fPipeline(pipe), fVertShaderRef(0), fFragShaderRef(0)
{
    ISetupShaderContexts();
    ILoopOverLayers();
    ICompile();
    ISetShaderVariableLocs();
    ICleanupShaderContexts();
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

        if (this->uLayerMat[i] != -1) {
            GLfloat matrix[16];
            glUniformMatrix4fv(this->uLayerMat[i], 1, GL_TRUE, hsMatrix2GL(layer->GetTransform(), matrix));
        }

        if (this->uTexture[i] != -1)
            glUniform1i(this->uTexture[i], numTextures);
        LOG_GL_ERROR_CHECK("Uniform Texture failed")

        numTextures++;
    }
}


void plGLMaterialShaderRef::ICompile()
{
    int32_t numTextures = 0;
    hsBitVector usedUVWs;

    for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        if (!layer)
            continue;

        // Load the image
        plMipmap* img = plMipmap::ConvertNoRef(layer->GetTexture());
        if (!img)
            continue;

        uint32_t uv = layer->GetUVWSrc() & plGBufferGroup::kUVCountMask;
        usedUVWs.SetBit(uv);

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, std::max(0, img->GetNumLevels() - 3));

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

    ST::string vtx = fVertexShader->Render();
    ST::string frg = fFragmentShader->Render();

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


void plGLMaterialShaderRef::ISetupShaderContexts()
{
    fVertexShader   = std::make_shared<plShaderContext>(kVertex, kShaderVersion);
    fFragmentShader = std::make_shared<plShaderContext>(kFragment, kShaderVersion);

    // Helper function to invert colour
    auto invColor = std::make_shared<plShaderFunction>("invColor", "vec3");
    auto argColor = std::make_shared<plArgumentNode>("color", "vec3", 0);
    invColor->PushOp(RETURN(SUB(CONSTANT("1.0"), argColor)));

    // Helper function to invert alpha
    auto invAlpha = std::make_shared<plShaderFunction>("invAlpha", "float");
    auto argAlpha = std::make_shared<plArgumentNode>("alpha", "float", 0);
    invAlpha->PushOp(RETURN(SUB(CONSTANT("1.0"), argAlpha)));

    fFragmentShader->PushFunction(invColor);
    fFragmentShader->PushFunction(invAlpha);
}


void plGLMaterialShaderRef::ISetShaderVariableLocs()
{
    // Store the attribute locations for later
    aVtxPosition  = glGetAttribLocation(fRef, "aVtxPosition");
    aVtxNormal    = glGetAttribLocation(fRef, "aVtxNormal");
    aVtxColor     = glGetAttribLocation(fRef, "aVtxColor");

    aVtxUVWSrc.assign(16, -1);
    for (size_t i = 0; i < 16; i++) {
        ST::string name = ST::format("aVtxUVWSrc{}", i);
        aVtxUVWSrc[i] = glGetAttribLocation(fRef, name.c_str());
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


void plGLMaterialShaderRef::ICleanupShaderContexts()
{
    fVariables.clear();

    fVertexShader.reset();
    fFragmentShader.reset();
}


void plGLMaterialShaderRef::ILoopOverLayers()
{
    size_t j = 0;
    size_t pass = 1;
    for (j = 0; j < fMaterial->GetNumLayers(); )
    {
        size_t iCurrMat = j;
        hsGMatState currState;

        std::shared_ptr<plShaderFunction> fragPass = std::make_shared<plShaderFunction>(ST::format("pass{}", pass), "void");

        j = IHandleMaterial(iCurrMat, currState, fragPass);

        if (j == -1)
            break;

        pass++;
        fPassStates.push_back(currState);

        fFragmentShader->PushFunction(fragPass);

#if 0
        ISetFogParameters(fMaterial->GetLayer(iCurrMat));
#endif
    }

    // Build the fragment shader main function with the right passes
    std::shared_ptr<plShaderFunction> fragMain = std::make_shared<plShaderFunction>("main", "void");
    if (pass == 2) {
        fragMain->PushOp(CALL("pass1")); // Cheat for now
    } else {
        fragMain->PushOp(CALL("pass2")); // Cheat for now
    }
    fFragmentShader->PushFunction(fragMain);


    // Build the vertex shader main function to assign to the varying vars
    std::shared_ptr<plShaderFunction> vertMain = std::make_shared<plShaderFunction>("main", "void");
    for (const auto& pair : fVariables) {
        if (pair.second->klass == kVarying) {
            std::shared_ptr<plVaryingNode> vary = std::static_pointer_cast<plVaryingNode>(pair.second);

            ST::string name = ST::format("a{}", vary->name.substr(1));
            std::shared_ptr<plAttributeNode> attr = std::make_shared<plAttributeNode>(name, vary->type);

            if (name == "aVtxColor") {
                // Need to swizzle to color around
                vertMain->PushOp(ASSIGN(vary, PROP(attr, "zyxw")));
            } else {
                vertMain->PushOp(ASSIGN(vary, attr));
            }
        }
    }

    // Set the vertex transforms now
    std::shared_ptr<plTempVariableNode> pos = std::make_shared<plTempVariableNode>("pos", "vec4");
    std::shared_ptr<plAttributeNode> apos = IFindVariable<plAttributeNode>("aVtxPosition", "vec3");
    std::shared_ptr<plUniformNode> mL2W = IFindVariable<plUniformNode>("uMatrixL2W", "mat4");
    std::shared_ptr<plUniformNode> mW2C = IFindVariable<plUniformNode>("uMatrixW2C", "mat4");
    std::shared_ptr<plUniformNode> mProj = IFindVariable<plUniformNode>("uMatrixProj", "mat4");

    vertMain->PushOp(ASSIGN(pos, MUL(mL2W, CALL("vec4", apos, CONSTANT("1.0")))));
    vertMain->PushOp(ASSIGN(pos, MUL(mW2C, pos)));
    vertMain->PushOp(ASSIGN(pos, MUL(mProj, pos)));
    vertMain->PushOp(ASSIGN(OUTPUT("gl_Position"), pos));

    fVertexShader->PushFunction(vertMain);
}


uint32_t plGLMaterialShaderRef::IHandleMaterial(uint32_t layer, hsGMatState& state, std::shared_ptr<plShaderFunction> fn)
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

    ShaderBuilder sb;
    sb.fFunction = fn;
    sb.fIteration = 0;

    IBuildBaseAlpha(currLay, &sb);

    for (int32_t i = 0; i < currNumLayers; i++) {
        sb.fIteration = i;
        sb.fCurrColor.reset();
        sb.fCurrAlpha.reset();
        sb.fCurrCoord.reset();
        sb.fCurrImage.reset();

        plLayerInterface* layPtr = fMaterial->GetLayer(layer + i);
        if (!layPtr)
            return -1;

        IBuildLayerTransform(layer + i, layPtr, &sb);
        IBuildLayerTexture(layer + i, layPtr, &sb);
        IBuildLayerBlend(layPtr, &sb);

        sb.fPrevColor = sb.fCurrColor;
        sb.fPrevAlpha = sb.fCurrAlpha;
    }

    //IHandleTextureMode(layer);
    //IHandleShadeMode(layer);
    //IHandleZMode();
    //IHandleMiscMode()
    //IHandleTextureStage(0, layer);

    // Multiply in the vertex color at the end (but alpha is premultiplied!)
    std::shared_ptr<plVaryingNode> vtx_color = IFindVariable<plVaryingNode>("vVtxColor", "vec4");
    std::shared_ptr<plShaderNode> finalColor;

    if (sb.fCurrColor) {
        finalColor = MUL(PROP(vtx_color, "rgb"), sb.fCurrColor, true);
    } else {
        finalColor = PROP(vtx_color, "rgb");
    }

    sb.fFunction->PushOp(ASSIGN(OUTPUT("gl_FragColor"), CALL("vec4", finalColor, sb.fCurrAlpha)));

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


void plGLMaterialShaderRef::IBuildBaseAlpha(plLayerInterface* layer, ShaderBuilder* sb)
{
    // Local variable to store the starting alpha value
    std::shared_ptr<plTempVariableNode> base = std::make_shared<plTempVariableNode>("baseAlpha", "float");

    std::shared_ptr<plVaryingNode> vtxAlpha = IFindVariable<plVaryingNode>("vVtxColor", "vec4");


    if (layer->GetBlendFlags() & hsGMatState::kBlendInvertVtxAlpha) {
        // base = 1.0 - vVtxColor.a
        sb->fFunction->PushOp(ASSIGN(base, CALL("invAlpha", PROP(vtxAlpha, "a"))));
    } else {
        // base = vVtxColor.a
        sb->fFunction->PushOp(ASSIGN(base, PROP(vtxAlpha, "a")));
    }

    sb->fPrevAlpha = base;
}


void plGLMaterialShaderRef::IBuildLayerTransform(uint32_t idx, plLayerInterface* layer, ShaderBuilder* sb)
{
    std::shared_ptr<plVariableNode> matrix;

#if 0
    if (layer->GetMiscFlags() & (hsGMatState::kMiscUseReflectionXform | hsGMatState::kMiscUseRefractionXform)) {
    } else if (layer->GetMiscFlags() & hsGMatState::kMiscCam2Screen) {
    } else if (layer->GetMiscFlags() & hsGMatState::kMiscProjection) {
        ST::string matName = ST::format("uLayerMat{}", idx);
        std::shared_ptr<plUniformNode> layMat = IFindVariable<plUniformNode>(matName, "mat4");
    } else if (layer->GetMiscFlags() & hsGMatState::kMiscBumpChans) {
    } else
#endif
    {
        ST::string matName = ST::format("uLayerMat{}", idx);
        matrix = IFindVariable<plUniformNode>(matName, "mat4");
    }

    uint32_t uvwSrc = layer->GetUVWSrc() & plGBufferGroup::kUVCountMask;

    ST::string uvwName = ST::format("vVtxUVWSrc{}", uvwSrc);
    std::shared_ptr<plVaryingNode> layUVW = IFindVariable<plVaryingNode>(uvwName, "vec3");

    // Local variable to store the mesh uvw * layer matrix
    ST::string coordName = ST::format("coords{}", idx);
    std::shared_ptr<plTempVariableNode> coords = std::make_shared<plTempVariableNode>(coordName, "vec4");

    sb->fFunction->PushOp(ASSIGN(coords, MUL(matrix, CALL("vec4", layUVW, CONSTANT("1.0")))));

    sb->fCurrCoord = coords;
}


void plGLMaterialShaderRef::IBuildLayerTexture(uint32_t idx, plLayerInterface* layer, ShaderBuilder* sb)
{
    plBitmap* texture = layer->GetTexture();

    if (texture != nullptr && sb->fCurrCoord) {
        plMipmap* mip;
        plCubicEnvironmap* cube;

        // Local variable to store the mesh uvw * layer matrix
        ST::string imgName = ST::format("image{}", idx);
        std::shared_ptr<plTempVariableNode> img = std::make_shared<plTempVariableNode>(imgName, "vec4");

        sb->fCurrImage = img;

        if ((mip = plMipmap::ConvertNoRef(texture)) != nullptr) {
            ST::string samplerName = ST::format("uTexture{}", idx);
            std::shared_ptr<plUniformNode> sampler = IFindVariable<plUniformNode>(samplerName, "sampler2D");

            // image = texture2D(sampler, coords.xy)
            sb->fFunction->PushOp(ASSIGN(img, CALL("texture2D", sampler, PROP(sb->fCurrCoord, "xy"))));
        }

#if 0
        if ((cube = plCubicEnvironmap::ConvertNoRef(texture)) != nullptr) {
            ST::string samplerName = ST::format("uTexture{}", idx);
            std::shared_ptr<plUniformNode> sampler = IFindVariable<plUniformNode>(samplerName, "sampler3D");

            // image = texture3D(sampler, coords.xyz)
            sb->fFunction->PushOp(ASSIGN(img, CALL("texture3D", sampler, PROP(sb->fCurrCoord, "xyz"))));
        }
#endif
    }
}


void plGLMaterialShaderRef::IBuildLayerBlend(plLayerInterface* layer, ShaderBuilder* sb)
{
    if (!sb->fCurrImage) {
        hsStatusMessage("Got a layer with no image");
        sb->fCurrColor = sb->fPrevColor;
        sb->fCurrAlpha = sb->fPrevAlpha;
        return;
    }

    // Local variable to store the color value
    ST::string colName = ST::format("color{}", sb->fIteration);
    std::shared_ptr<plTempVariableNode> col = std::make_shared<plTempVariableNode>(colName, "vec3");

    // Local variable to store the alpha value
    ST::string alphaName = ST::format("alpha{}", sb->fIteration);
    std::shared_ptr<plTempVariableNode> alpha = std::make_shared<plTempVariableNode>(alphaName, "float");

    std::shared_ptr<plShaderNode> texCol;
    if (layer->GetBlendFlags() & hsGMatState::kBlendInvertColor) {
        // color = 1.0 - texture.rgb
        texCol = CALL("invColor", PROP(sb->fCurrImage, "rgb"));
    } else {
        // color = texture.rgb
        texCol = PROP(sb->fCurrImage, "rgb");
    }


    if (sb->fIteration == 0) {
        // Leave fCurrColor null if we are blending without texture color
        if (layer->GetBlendFlags() & hsGMatState::kBlendNoTexColor) {
            sb->fCurrColor = sb->fPrevColor;
        } else {
            sb->fFunction->PushOp(ASSIGN(col, texCol));
            sb->fCurrColor = col;
        }

        std::shared_ptr<plShaderNode> alphaVal;
        if (layer->GetBlendFlags() & hsGMatState::kBlendInvertAlpha) {
            // 1.0 - texture.a
            alphaVal = CALL("invAlpha", PROP(sb->fCurrImage, "a"));
        } else {
            // texture.a
            alphaVal = PROP(sb->fCurrImage, "a");
        }


        if (layer->GetBlendFlags() & hsGMatState::kBlendNoVtxAlpha) {
            // Only use texture alpha
            sb->fFunction->PushOp(ASSIGN(alpha, alphaVal));

            sb->fCurrAlpha = alpha;
        } else if (layer->GetBlendFlags() & hsGMatState::kBlendNoTexAlpha) {
            // Only use vertex alpha (prev alpha)
            sb->fCurrAlpha = sb->fPrevAlpha;
        } else {
            // Vertex alpha * base texture alpha
            sb->fFunction->PushOp(ASSIGN(alpha, MUL(alphaVal, sb->fPrevAlpha)));
            sb->fCurrAlpha = alpha;
        }
    } else {
        switch (layer->GetBlendFlags() & hsGMatState::kBlendMask)
        {

            case hsGMatState::kBlendAddColorTimesAlpha:
                hsAssert(false, "Blend mode unsupported on upper layers");
                break;

            case hsGMatState::kBlendAlpha:
            {
                if (layer->GetBlendFlags() & hsGMatState::kBlendNoTexColor) {
                    // color = prev
                    sb->fCurrColor = sb->fPrevColor;
                } else {
                    if (layer->GetBlendFlags() & hsGMatState::kBlendInvertAlpha) {
                        // color = texture.rgb + (texture.a * prev)
                        sb->fFunction->PushOp(ASSIGN(col, ADD(texCol, MUL(PROP(sb->fCurrImage, "a"), sb->fPrevColor, true))));
                        sb->fCurrColor = col;
                    } else {
                        // color = mix(prev, texture.rgb, texture.a)
                        sb->fFunction->PushOp(ASSIGN(col, CALL("mix", sb->fPrevColor, texCol, PROP(sb->fCurrImage, "a"))));
                        sb->fCurrColor = col;
                    }
                }


                std::shared_ptr<plShaderNode> alphaVal;
                if (layer->GetBlendFlags() & hsGMatState::kBlendInvertAlpha) {
                    // 1.0 - texture.a
                    alphaVal = CALL("invAlpha", PROP(sb->fCurrImage, "a"));
                } else {
                    // texture.a
                    alphaVal = PROP(sb->fCurrImage, "a");
                }

                if (layer->GetBlendFlags() & hsGMatState::kBlendAlphaAdd) {
                    // alpha = alphaVal + prev
                    sb->fFunction->PushOp(ASSIGN(alpha, ADD(alphaVal, sb->fPrevAlpha)));
                    sb->fCurrAlpha = alpha;
                } else if (layer->GetBlendFlags() & hsGMatState::kBlendAlphaMult) {
                    // alpha = alphaVal * prev
                    sb->fFunction->PushOp(ASSIGN(alpha, MUL(alphaVal, sb->fPrevAlpha)));
                    sb->fCurrAlpha = alpha;
                } else {
                    // alpha = prev
                    sb->fCurrAlpha = sb->fPrevAlpha;
                }
                break;
            }

            case hsGMatState::kBlendAdd:
            {
                // color = texture.rgb + prev
                sb->fFunction->PushOp(ASSIGN(col, ADD(texCol, sb->fPrevColor)));
                sb->fCurrColor = col;

                // alpha = prev
                sb->fCurrAlpha = sb->fPrevAlpha;
                break;
            }

            case hsGMatState::kBlendMult:
            {
                // color = color * prev
                sb->fFunction->PushOp(ASSIGN(col, MUL(texCol, sb->fPrevColor)));
                sb->fCurrColor = col;

                // alpha = prev
                sb->fCurrAlpha = sb->fPrevAlpha;
                break;
            }

            case hsGMatState::kBlendDot3:
            {
                hsStatusMessage("Blend DOT3");
                // color = (color.r * prev.r + color.g * prev.g + color.b * prev.b)
                // alpha = prev
                //break;
            }

            case hsGMatState::kBlendAddSigned:
            {
                hsStatusMessage("Blend AddSigned");
                // color = color + prev - 0.5
                // alpha = prev
                //break;
            }

            case hsGMatState::kBlendAddSigned2X:
            {
                hsStatusMessage("Blend AddSigned2X");
                // color = (color + prev - 0.5) << 1
                // alpha = prev
                //break;
            }

            case 0:
            {
                // color = texture.rgb
                sb->fFunction->PushOp(ASSIGN(col, texCol));
                sb->fCurrColor = col;

                // alpha = texture.a
                sb->fFunction->PushOp(ASSIGN(alpha, PROP(sb->fCurrImage, "a")));
                sb->fCurrAlpha = alpha;
                break;
            }
        }
    }

}


#if 0
void plGLMaterialShaderRef::IHandleTextureMode(plLayerInterface* layer)
{
    plBitmap* bitmap = layer->GetTexture();

    if (bitmap) {
        // EnvBumpNext is unused in production -- ignoring for now


        if (layer->GetBlendFlags() & hsGMatState::kBlendNoTexColor) {
            // color = null
        } else {
            if (lay->GetBlendFlags() & hsGMatState::kBlendInvertColor) {
                // color = 1.0 - texture.rgb
            } else {
                // color = texture.rgb
            }
        }

        if (layer->GetBlendFlags() & hsGMatState::kBlendInvertAlpha) {
            // alpha1 = 1.0 - texture.a
        } else {
            // alpha1 = texture.a
        }

        if (layer->GetBlendFlags() & hsGMatState::kBlendInvertVtxAlpha) {
            // alpha2 = 1.0 - vVtxColor.a
        } else {
            // alpha2 = vVtxColor.a
        }

        if (layer->GetBlendFlags() & hsGMatState::kBlendNoVtxAlpha) {
            // alpha = alpha1
        } else if (layer->GetBlendFlags() & hsGMatState::kBlendNoTexAlpha) {
            // alpha = alpha2
        } else {
            // alpha = alpha1 * alpha2
        }
    } else if (piggybacks) {
        // Plasma says it selects the vertex colours, but actually selects an
        // undefined arg2. We're going to assume that's wrong and select the
        // vertex colours.

        // color = vVtxColor.rgb
        // alpha = vVtxColor.a
    } else {
        // color = vVtxColor.rgb
        if (layer->GetBlendFlags() & hsGMatState::kBlendInvertVtxAlpha) {
            // alpha = 1.0 - vVtxColor.a
        } else {
            // alpha = vVtxColor.a
        }
    }
}

void plGLMaterialShaderRef::IHandleStageBlend(const hsGMatState& state)
{
    const uint32_t blendFlags = state.fBlendFlags;

    if (blendFlags & hsGMatState::kBlendInvertColor) {
        // color = 1.0 - texture.rgb
    } else {
        // color = texture.rgb
    }

    // Ignoring the unused kBlendEnvBumpNext

    switch (blendFlags & hsGMatState::kBlendMask)
    {
        case hsGMatState::kBlendAlpha:
        {
            if (blendFlags & hsGMatState::kBlendNoTexColor) {
                // if (prev) { color = prev }
            } else {
                if (blendFlags & hsGMatState::kBlendInvertAlpha) {
                    // color = color.rgb + (texture.a * prev.rgb)
                } else {
                    // color = mix(prev.rgb, color.rgb, texture.a)
                }
            }

            if (blendFlags & hsGMatState::kBlendAlphaAdd) {
                if (blendFlags & hsGMatState::kBlendInvertAlpha) {
                    // alpha = (1.0 - texture.a) + prev
                } else {
                    // alpha = texture.a + prev
                }
            } else if (blendFlags & hsGMatState::kBlendAlphaMult) {
                if (blendFlags & hsGMatState::kBlendInvertAlpha) {
                    // alpha = (1.0 - texture.a) * prev
                } else {
                    // alpha = texture.a * prev
                }
            } else {
                // alpha = prev
            }
            break;
        }

        case hsGMatState::kBlendAdd:
        {
            // color = color + prev
            // alpha = prev
            break;
        }

        case hsGMatState::kBlendMult:
        {
            // color = color * prev
            // alpha = prev
            break;
        }

        case hsGMatState::kBlendDot3:
        {
            // color = (color.r * prev.r + color.g * prev.g + color.b * prev.b)
            // alpha = prev
            break;
        }

        case hsGMatState::kBlendAddSigned:
        {
            // color = color + prev - 0.5
            // alpha = prev
            break;
        }

        case hsGMatState::kBlendAddSigned2X:
        {
            // color = (color + prev - 0.5) << 1
            // alpha = prev
            break;
        }

        case hsGMatState::kBlendAddColorTimesAlpha:
            hsAssert(false, "Blend mode unsupported on upper layers");
            break;

        case 0:
        {
            // color = color
            // alpha = prev
            break;
        }
    }
}
#endif
