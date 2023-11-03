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
#ifndef __HSVERTEXSHADER_H
#define __HSVERTEXSHADER_H

#include <vector>

#include "hsGeometry3.h"
#include "hsMatrix44.h"

class hsBitVector;
struct hsColorRGBA;
class hsConverterUtils;
class plGeometrySpan;
class plLightMapGen;
class hsGMaterial;
class plMaxLightContext;

class INode;
class Interface;

class hsVertexShader 
{
private:
    hsVertexShader();
public:
    virtual ~hsVertexShader() noexcept(false);

    static hsVertexShader& Instance();

    void ShadeNode(INode* node, hsMatrix44& l2w, hsMatrix44& w2l, std::vector<plGeometrySpan *> &spans);

    void Open();
    void Close();

private:

    /// Temporary vertex class
    class plTmpVertex3
    {
        public:
            hsPoint3    fLocalPos;
            hsVector3   fNormal;
    };

    bool ILightIncludesNode(LightObject* light, INode* node);

    void    INativeShadeVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, bool translucent);
    void    INativeShadowVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, bool translucent);

    bool IsTranslucent( hsGMaterial *material );

    void IShadeSpan( plGeometrySpan *span, INode* node );
    void IShadeVertices( plGeometrySpan *span, hsBitVector *dirtyVector, INode* node, bool translucent );

private:
    Interface           *fInterface;
    hsConverterUtils    &fConverterUtils;

    plLightMapGen*      fLightMapGen;

    hsMatrix44          fLocalToWorld, fNormalToWorld;      // fN2W is inv-transpose of fL2W

    int                 fShaded;    // just record-keeping

    hsColorRGBA         *fShadeColorTable;
    hsColorRGBA         *fIllumColorTable;
};

#endif