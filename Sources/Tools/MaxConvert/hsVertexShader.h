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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef __HSVERTEXSHADER_H
#define __HSVERTEXSHADER_H

class hsConverterUtils;
class hsBitVector;

class plMaxLightContext;
class plGeometrySpan;
struct hsColorRGBA;
class plLightMapGen;

class hsVertexShader 
{
private:
    hsVertexShader();
public:
    virtual ~hsVertexShader();

	static hsVertexShader& Instance();

	void ShadeNode(INode* node, hsMatrix44& l2w, hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans);

	void Open();
	void Close();

private:

	/// Temporary vertex class
	class plTmpVertex3
	{
		public:
			hsPoint3	fLocalPos;
			hsVector3	fNormal;
	};

	hsBool ILightIncludesNode(LightObject* light, INode* node);

	void	INativeShadeVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, hsBool translucent);
	void	INativeShadowVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, hsBool translucent);

	hsBool IsTranslucent( hsGMaterial *material );

    void IShadeSpan( plGeometrySpan *span, INode* node );
	void IShadeVertices( plGeometrySpan *span, hsBitVector *dirtyVector, INode* node, hsBool translucent );

private:
    Interface           *fInterface;
	hsConverterUtils    &fConverterUtils;

	plLightMapGen*		fLightMapGen;

    hsMatrix44          fLocalToWorld, fNormalToWorld;		// fN2W is inv-transpose of fL2W

    int                 fShaded;    // just record-keeping

	hsColorRGBA			*fShadeColorTable;
	hsColorRGBA			*fIllumColorTable;
};

#endif