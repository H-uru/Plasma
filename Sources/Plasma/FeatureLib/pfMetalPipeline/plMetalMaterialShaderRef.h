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
#ifndef _plMetalMaterialShaderRef_inc_
#define _plMetalMaterialShaderRef_inc_

#include "hsGMatState.h"
#include "plMetalDeviceRef.h"
#include "ShaderTypes.h"

#include <map>

class hsGMaterial;
class plMetalPipeline;
class plLayerInterface;

class plMetalMaterialShaderRef : public plMetalDeviceRef
{
protected:
    plMetalPipeline*                    fPipeline;
    hsGMaterial*                        fMaterial;
    //temporary holder for the fragment shader to use, we don't own this reference
    MTL::Function*                      fFragFunction;
private:
    std::vector<size_t>                 fPassIndices;
    //FIXME: This should be retained/released
    MTL::Device*                        fDevice;
    std::vector<MTL::Buffer *>          fPassArgumentBuffers;
    std::vector<MTL::Buffer *>          fPassColors;
    
public:
    void Link(plMetalMaterialShaderRef** back) { plMetalDeviceRef::Link((plMetalDeviceRef**)back); }
    plMetalMaterialShaderRef*    GetNext() { return (plMetalMaterialShaderRef*)fNext; }
    
    plMetalMaterialShaderRef(hsGMaterial* mat, plMetalPipeline *pipe);
    ~plMetalMaterialShaderRef();
    
    void Release();
    void CheckMateralRef();
    
    size_t GetNumPasses() const { return fNumPasses; }
    size_t GetPassIndex(size_t which) const { return fPassIndices[which]; }
    
    void EncodeArguments(MTL::RenderCommandEncoder *encoder, VertexUniforms *vertexUniforms, uint pass, std::vector<plLayerInterface*> *piggyBacks, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> preEncodeTransform, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> postEncodeTransform);
    void FastEncodeArguments(MTL::RenderCommandEncoder *encoder, VertexUniforms *vertexUniforms, uint pass);
    //probably not a good idea to call prepareTextures directly
    //mostly just a hack to keep plates working for now
    void prepareTextures(MTL::RenderCommandEncoder *encoder, uint pass);
    std::vector<size_t>                 fPassLengths;
    
    // Set the current Plasma state based on the input layer state and the material overrides.
    // fMatOverOn overrides to set a state bit whether it is set in the layer or not.
    // fMatOverOff overrides to clear a state bit whether it is set in the layer or not.s
    const hsGMatState ICompositeLayerState(const plLayerInterface* layer);
    
    static plLayerInterface* Passthrough(plLayerInterface* layer, uint32_t index) {
        return layer;
    }
private:
    void ILoopOverLayers();
    
    uint32_t fNumPasses;
    uint32_t IHandleMaterial(uint32_t layer, plMetalFragmentShaderArgumentBuffer *uniforms, std::vector<plLayerInterface*> *piggybacks, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> preEncodeTransform, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> postEncodeTransform);
    bool ICanEatLayer(plLayerInterface* lay);
    uint32_t ILayersAtOnce(uint32_t which);
    
    void IBuildLayerTexture(MTL::RenderCommandEncoder *encoder, uint32_t offsetFromRootLayer, plLayerInterface* layer, simd_float4 *colorMap);
    void PopulateFragmentShaderLayerFromLayer(plFragmentShaderLayer *fragmentLayer, plLayerInterface* layer);
    void EncodeTransform(plLayerInterface* layer, UVOutDescriptor *transform);
};

#endif
