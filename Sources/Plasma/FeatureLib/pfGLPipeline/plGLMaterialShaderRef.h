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

#include <vector>

#include "hsGMatState.h"

class hsGMaterial;
class plPipeline;
class plLayerInterface;

class plGLMaterialShaderRef : public plGLDeviceRef
{
protected:
    hsGMaterial*                fMaterial;
    plPipeline*                 fPipeline;
    GLuint                      fVertShaderRef;
    GLuint                      fFragShaderRef;

    uint32_t                    fPasses;
    std::vector<hsGMatState>    fPassStates;

    int32_t                     fNumUVs;

public:
    void                    Link(plGLMaterialShaderRef** back) { plGLDeviceRef::Link((plGLDeviceRef**)back); }
    plGLMaterialShaderRef*  GetNext() { return (plGLMaterialShaderRef*)fNext; }

    plGLMaterialShaderRef(hsGMaterial* mat, plPipeline* pipe)
        : plGLDeviceRef(), fMaterial(mat), fPipeline(pipe), fVertShaderRef(), fFragShaderRef(), fPasses()
    {
        ILoopOverLayers();

        // TODO: Remove
        ICompile();
    }

    virtual ~plGLMaterialShaderRef();

    void Release();
    void SetupTextureRefs();

    int32_t GetNumUVs() const { return fNumUVs; }
    uint32_t GetNumPasses() const { return fPasses; }
    hsGMatState GetPassState(uint32_t which) const { return fPassStates[which]; }

protected:
    void ICompile();

    bool ILoopOverLayers();
    uint32_t IHandleMaterial(uint32_t layer, hsGMatState& state);
    uint32_t ILayersAtOnce(uint32_t which);
    bool ICanEatLayer(plLayerInterface* lay);
    //void IHandleFirstTextureStage(plLayerInterface* layer);
    //void IHandleFirstStageBlend(const hsGMatState& layerState);
};

#endif // _plGLMaterialShaderRef_inc_
