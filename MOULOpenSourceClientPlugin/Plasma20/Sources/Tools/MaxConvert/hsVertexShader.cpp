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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	hsVertexShader Class Functions											//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	5.9.2001 mcn - Updated to reflect the new (temporary) vertex color/		//
//				   lighting model.											//
//																			//
//////////////////////////////////////////////////////////////////////////////


#include "HeadSpin.h"

#include "Max.h"
#include "stdmat.h"
#include "istdplug.h"
#include "dummy.h"
#include "notetrck.h"

#include "../MaxMain/plMaxNode.h"
#include "hsBitVector.h"

#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "../plSurface/hsGMaterial.h"

#include "UserPropMgr.h"
#include "hsMaxLayerBase.h"
#include "hsVertexShader.h"

#include "hsConverterUtils.h"
#include "hsControlConverter.h"
#include "hsExceptionStack.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

#include "../plDrawable/plGeometrySpan.h"

#include "plMaxLightContext.h"
#include "plRenderGlobalContext.h"
#include "plLightMapGen.h"

#define MF_NATIVE_MAX_LIGHT
#define MF_NO_RAY_SHADOW

extern UserPropMgr gUserPropMgr;
extern TimeValue GetTime(Interface *gi);

//===========================================================================
// hsVertexShader
//===========================================================================

hsVertexShader::hsVertexShader() :
	fConverterUtils(hsConverterUtils::Instance()),
    fInterface(nil),
	fLightMapGen(nil),
    fShaded(0)
{
	hsGuardBegin("hsVertexShader::hsVertexShader");
    fLocalToWorld.Reset();
	hsGuardEnd;
}

hsVertexShader::~hsVertexShader()
{
	hsGuardBegin("hsVertexShader::~hsVertexShader");
	hsGuardEnd;
}

hsVertexShader& hsVertexShader::Instance()
{
	hsGuardBegin("hsVertexShader::Instance");
	static hsVertexShader instance;
	return instance;
	hsGuardEnd; 
}

void hsVertexShader::Open()
{
	hsGuardBegin("hsVertexShader::InitLights");

    fLocalToWorld.Reset();

    fInterface = ::GetCOREInterface();

	fLightMapGen = &plLightMapGen::Instance();

	hsGuardEnd;
}

void hsVertexShader::Close()
{
	hsGuardBegin("hsVertexShader::DeInitLights");

	fLightMapGen = nil;

	hsGuardEnd;
}

//// ShadeNode ///////////////////////////////////////////////////////////////
//	Same as the other ShadeNode, only this shades an array of plGeometrySpans.

void hsVertexShader::ShadeNode(INode* node, hsMatrix44& l2w, hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans)
{
	// If we're flagged for WaterColor, our vertex colors are already done.
	if( ((plMaxNodeBase*)node)->GetCalcEdgeLens() || node->UserPropExists("XXXWaterColor") )
		return;

	fLightMapGen->InitNode(node);

	fLocalToWorld = l2w;
	hsMatrix44 tempMatrix = w2l; // l2w's inverse
	tempMatrix.GetTranspose( &fNormalToWorld );	// Inverse-transpose of the fLocalToWorld matrix, 
	
	int			i;
	for( i = 0; i < spans.GetCount(); i++ )
		IShadeSpan( spans[ i ], node);
	
	fLightMapGen->DeInitNode();

	fShaded++;
}

//// IShadeSpan //////////////////////////////////////////////////////////////
//	Shades a single plGeometrySpan.
//	5.9.2001 mcn - Updated to support the new (temporary) vertex color/lighting
//				   method.

void hsVertexShader::IShadeSpan( plGeometrySpan *span, INode* node )
{
	hsColorRGBA			preDiffuse, rtDiffuse, matAmbient;
	hsBitVector			dirtyVector;
	int					i;
	hsBool				translucent, shadeIt, addingIt;
	plLayerInterface	*layer = nil;


	hsGuardBegin("hsVertexShader::ShadeSpan");
	
	const char* dbgNodeName = node->GetName(); 

	if( span->fNumVerts == 0 )
		return;

	fShadeColorTable = TRACKED_NEW hsColorRGBA[ span->fNumVerts ];	
	fIllumColorTable = TRACKED_NEW hsColorRGBA[ span->fNumVerts ];	
	
	translucent = IsTranslucent( span->fMaterial );

	/// Get material layer #0
	addingIt = false;
	shadeIt = !( span->fProps & plGeometrySpan::kPropNoPreShade );

	if( span->fMaterial->GetNumLayers() != 0 )
	{
		layer = span->fMaterial->GetLayer( 0 );
		if( layer->GetShadeFlags() & hsGMatState::kShadeNoShade )
			shadeIt = false;
		if( layer->GetBlendFlags() & hsGMatState::kBlendAdd )
			addingIt = true;
	}
	float opacity = 1.f;
	for( i = 0; i < span->fMaterial->GetNumLayers(); i++ )
	{
		plLayerInterface* lay = span->fMaterial->GetLayer(i);
		if( (lay->GetBlendFlags() & hsGMatState::kBlendAlpha)
			&&
			(
				!i	
				||
				(lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere)
			)
		  )
		{
			opacity = span->fMaterial->GetLayer(i)->GetOpacity();
		}
	}

	/// Generate color table
	if( shadeIt )
		IShadeVertices( span, &dirtyVector, node, translucent );
	else
	{
		for( i = 0; i < span->fNumVerts; i++ )	
		{
			/// This is good for the old way, but not sure about the new way. Test once new way is in again -mcn
//			fShadeColorTable[ i ].Set( 1, 1, 1, 1 );
//			fIllumColorTable[ i ].Set( 0, 0, 0, 1 );
			hsPoint3	position;
			hsVector3	normal;
			hsColorRGBA	color, illum;

			span->ExtractVertex( i, &position, &normal, &color, &illum );
			span->ExtractInitColor( i, &color, &illum );
			fShadeColorTable[ i ].Set( color.r, color.g, color.b, color.a );
			fIllumColorTable[ i ].Set( illum.r, illum.g, illum.b, 1 );
		}
	}

	/// Get mat colors to modulate by
	if( layer == nil )
	{
		preDiffuse.Set( 1, 1, 1, 1 );
		rtDiffuse.Set( 1, 1, 1, 1 );
		matAmbient.Set( 0, 0, 0, 0 );
	}
	else
	{
		if( layer->GetShadeFlags() & hsGMatState::kShadeWhite )
		{
			preDiffuse.Set( 1, 1, 1, 1 );
			rtDiffuse.Set( 1, 1, 1, 1 );
			matAmbient.Set( 0, 0, 0, 0 );
		}
		else
		{
			preDiffuse = layer->GetPreshadeColor();		// This is for vertex-based lighting, which basically ignores preshading
			rtDiffuse = layer->GetRuntimeColor();		// This is for vertex-based lighting, which basically ignores preshading
			matAmbient = layer->GetAmbientColor();
			matAmbient.a = 0;
		}
		preDiffuse.a = opacity;
		rtDiffuse.a = opacity;
	}
#if 0

	/// Multiply by the material color, and scale by opacity if we're additive blending
	/// Apply colors now, multiplying by the material color as we go
	for( i = 0; i < span->fNumVerts; i++ )
	{
		fShadeColorTable[ i ] *= matDiffuse;
		fShadeColorTable[ i ] += matAmbient;
		fIllumColorTable[ i ] *= matDiffuse;
		fIllumColorTable[ i ] += matAmbient;
	}

	if( addingIt )
	{
		for( i = 0; i < span->fNumVerts; i++ )
		{
			float opacity = fShadeColorTable[ i ].a;
			fShadeColorTable[ i ] *= opacity;
			fIllumColorTable[ i ] *= opacity;
		}
	}
#else
	/// Combine shade and illum together into the diffuse color
	if( ( span->fProps & plGeometrySpan::kLiteMask ) != plGeometrySpan::kLiteMaterial )
	{
		/// The two vertex lighting formulas take in a vetex color pre-processed, i.e. in 
		/// the form of: vtxColor = ( maxVtxColor * materialDiffuse + maxIllumColor )
		span->fProps |= plGeometrySpan::kDiffuseFoldedIn;
		if( !shadeIt )
		{
			for( i = 0; i < span->fNumVerts; i++ )
			{
				fIllumColorTable[ i ].a = 0;
				fShadeColorTable[ i ] = (fShadeColorTable[ i ] * rtDiffuse) + fIllumColorTable[ i ];
				fIllumColorTable[ i ].Set( 0, 0, 0, 0 );
			}
		}
		else
		{
			for( i = 0; i < span->fNumVerts; i++ )
			{
				fIllumColorTable[ i ].a = 1.f;
				// Following needs to be changed to allow user input vertex colors to modulate
				// the runtime light values.
//				fShadeColorTable[ i ] = fIllumColorTable[ i ] * rtDiffuse;
				fShadeColorTable[ i ] = fShadeColorTable[ i ] * fIllumColorTable[ i ] * rtDiffuse;
				fIllumColorTable[ i ].Set( 0, 0, 0, 0 );
			}
		}
	}
	else
	{
		if( !shadeIt )
		{
			// Not shaded, so runtime lit, so we want BLACK vertex colors
			for( i = 0; i < span->fNumVerts; i++ )
			{
				fShadeColorTable[ i ].Set( 0, 0, 0, 0 );
				fIllumColorTable[ i ].Set( 0, 0, 0, 0 );
			}
		}
		else
		{
			for( i = 0; i < span->fNumVerts; i++ )
			{
				fShadeColorTable[ i ] *= fIllumColorTable[ i ];
				fIllumColorTable[ i ].Set( 0, 0, 0, 0 );
			}
		}
	}
#endif

	/// Loop and stuff
	for( i = 0; i < span->fNumVerts; i++ )
		span->StuffVertex( i, fShadeColorTable + i, fIllumColorTable + i );

	delete [] fShadeColorTable;       
	delete [] fIllumColorTable;       

	hsGuardEnd;
}

//// IShadeVertices //////////////////////////////////////////////////////////
//	Shades an array of vertices from a plGeometrySpan.
//	5.9.2001 mcn - Updated for the new lighting model. Now on runtime, we
//				   want the following properties on each vertex:
//					diffuseColor = vertexColor * matDiffuse + matAmbient (including alpha)
//					specularColor = ( illumniation + pre-shading ) * matDiffuse + matAmbient
//				   We do the mat modulation outside of this function, so we 
//				   just gotta make sure the two arrays get the right values.

void hsVertexShader::IShadeVertices( plGeometrySpan *span, hsBitVector *dirtyVector, INode* node, hsBool translucent )
{
	hsGuardBegin( "hsVertexShader::IShadeVertices" );

	plMaxNode* maxNode = (plMaxNode*)node;
	if( maxNode->CanConvert() && (nil != maxNode->GetLightMapComponent()) )
		return;

    int		index;

	hsPoint3		position;
	hsVector3		normal;
	hsColorRGBA		color, illum;
	plTmpVertex3	*vertices;


	/// Allocate temp vertex array
	vertices = TRACKED_NEW plTmpVertex3[ span->fNumVerts ];
	for( index = 0; index < span->fNumVerts; index++ )
	{
		span->ExtractVertex( index, &position, &normal, &color, &illum );
		span->ExtractInitColor( index, &color, &illum );

		/// fShadeColorTable is the shaded portion. fIllumColorTable is the illuminated portion;
		/// for more and less confusing details, see above.
		fShadeColorTable[ index ].Set( color.r, color.g, color.b, color.a );
		fIllumColorTable[ index ].Set( illum.r, illum.g, illum.b, 1 );

		position = fLocalToWorld * position;
		normal = fNormalToWorld * normal;

		vertices[ index ].fLocalPos = position;
		vertices[ index ].fNormal = normal;
		vertices[ index ].fNormal.Normalize();
	}

	const char* dbgNodeName = node->GetName();

	TimeValue t = fInterface->GetTime();
	Box3 bbox;
	node->EvalWorldState(t).obj->GetDeformBBox(t, bbox, &node->GetObjectTM(t));
	plMaxLightContext ctx(bbox, t);

	for( index = 0; index < span->fNumVerts; index++ )
		INativeShadeVtx(fIllumColorTable[index], ctx, vertices[ index ], translucent);

	// Delete temp arrays
	delete [] vertices;

	hsGuardEnd;
}

void hsVertexShader::INativeShadeVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, hsBool translucent)
{
	ctx.SetPoint(vtx.fLocalPos, vtx.fNormal);

	Color color = fLightMapGen->ShadePoint(ctx);
	shade.r += color.r;
	shade.g += color.g;
	shade.b += color.b;

	// To handle two-sided translucency here, we should compute the shade on each side and sum them.
	if( translucent )
	{
		ctx.SetPoint(vtx.fLocalPos, -vtx.fNormal);
		color = fLightMapGen->ShadePoint(ctx);
		shade.r += color.r;
		shade.g += color.g;
		shade.b += color.b;
	}
}

void hsVertexShader::INativeShadowVtx(hsColorRGBA& shade, plMaxLightContext& ctx, const plTmpVertex3& vtx, hsBool translucent)
{
	ctx.SetPoint(vtx.fLocalPos, vtx.fNormal);

	Color color = fLightMapGen->ShadowPoint(ctx);
	shade.r += color.r;
	shade.g += color.g;
	shade.b += color.b;
}

hsBool hsVertexShader::IsTranslucent( hsGMaterial *material )
{
	hsGuardBegin("hsVertexShader::IsTranslucent");

    if( material )
    {
        plLayerInterface* layer = material->GetLayer(0);
        if( layer && ( layer->GetShadeFlags() & hsGMatState::kShadeSoftShadow ) )
        {
            return true;
        }
    }

    return false;
	hsGuardEnd; 
}


