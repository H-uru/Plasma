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

#ifndef _plGLMaterialShaderRef_inc_
#define _plGLMaterialShaderRef_inc_

#include "plGLDeviceRef.h"
#include "plShaderNode.h"

#include <map>
#include <vector>

#include "hsGMatState.h"

class hsGMaterial;
class plPipeline;
class plLayerInterface;

class plGLMaterialShaderRef : public plGLDeviceRef
{
    typedef std::map<ST::string, std::shared_ptr<plGlobalVariableNode>> plShaderVarLookup;

    enum {
        kShaderVersion = 100
    };

    struct ShaderBuilder {
        std::shared_ptr<plShaderFunction>   fFunction;
        uint32_t                            fIteration;

        std::shared_ptr<plVariableNode>     fMatValues;

        std::shared_ptr<plVariableNode>     fPrevColor;
        std::shared_ptr<plVariableNode>     fPrevAlpha;

        std::shared_ptr<plVariableNode>     fCurrColor;
        std::shared_ptr<plVariableNode>     fCurrAlpha;
        std::shared_ptr<plVariableNode>     fCurrCoord;
        std::shared_ptr<plVariableNode>     fCurrImage;
    };

protected:
    hsGMaterial*                        fMaterial;
    plPipeline*                         fPipeline;
    GLuint                              fVertShaderRef;
    GLuint                              fFragShaderRef;

    std::vector<size_t>                 fPassIndices;

    std::shared_ptr<plShaderContext>    fVertexShader;
    std::shared_ptr<plShaderContext>    fFragmentShader;
    plShaderVarLookup                   fVariables;

public:
    // These are named to match the GLSL variable names and are public vars
    GLuint                      aVtxPosition;
    GLuint                      aVtxNormal;
    GLuint                      aVtxColor;
    std::vector<GLuint>         aVtxUVWSrc; // These are indexed by UV chan
    std::vector<GLuint>         uLayerMat;  // These are indexed by layer
    std::vector<GLuint>         uTexture;   // These are indexed by layer
    GLuint                      uGlobalAmbient;
    GLuint                      uMatAmbientCol;
    GLuint                      uMatAmbientSrc;
    GLuint                      uMatDiffuseCol;
    GLuint                      uMatDiffuseSrc;
    GLuint                      uMatEmissiveCol;
    GLuint                      uMatEmissiveSrc;
    GLuint                      uMatSpecularCol;
    GLuint                      uMatSpecularSrc;
    GLuint                      uPassNumber;

    void                    Link(plGLMaterialShaderRef** back) { plGLDeviceRef::Link((plGLDeviceRef**)back); }
    plGLMaterialShaderRef*  GetNext() { return (plGLMaterialShaderRef*)fNext; }

    plGLMaterialShaderRef(hsGMaterial* mat, plPipeline* pipe);
    virtual ~plGLMaterialShaderRef();

    void Release();
    void SetupTextureRefs();

    size_t GetNumPasses() const { return fPassIndices.size(); }
    size_t GetPassIndex(size_t which) const { return fPassIndices[which]; }
    hsGMaterial* GetMaterial() const { return fMaterial; }

protected:
    void ICompile();

    void ISetupShaderContexts();
    void ISetShaderVariableLocs();
    void ICleanupShaderContexts();

    template <typename T>
    std::shared_ptr<T> IFindVariable(const ST::string& name, const ST::string& type)
    {
        auto it = fVariables.find(name);
        if (it == fVariables.end()) {
            std::shared_ptr<T> var = std::make_shared<T>(name, type);
            fVariables[name] = var;
            return var;
        } else {
            return std::static_pointer_cast<T>(it->second);
        }
    }

    void ILoopOverLayers();
    uint32_t IHandleMaterial(uint32_t layer, std::shared_ptr<plShaderFunction> fn);
    uint32_t ILayersAtOnce(uint32_t which);
    bool ICanEatLayer(plLayerInterface* lay);
    void IBuildBaseAlpha(plLayerInterface* layer, ShaderBuilder* sb);
    void IBuildLayerTransform(uint32_t idx, plLayerInterface* layer, ShaderBuilder* sb);
    void IBuildLayerTexture(uint32_t idx, plLayerInterface* layer, ShaderBuilder* sb);
    void IBuildLayerBlend(plLayerInterface* layer, ShaderBuilder* sb);
    //void IHandleFirstTextureStage(plLayerInterface* layer);
    //void IHandleFirstStageBlend(const hsGMatState& layerState);
};

#endif // _plGLMaterialShaderRef_inc_
