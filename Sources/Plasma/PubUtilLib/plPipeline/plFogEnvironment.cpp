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
//	plFogEnvironment.cpp - Functions for the fog volume class				//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "plFogEnvironment.h"

#include "plTweak.h"

//// Constructors & Destructor ///////////////////////////////////////////////

plFogEnvironment::plFogEnvironment()
{
	fType = kNoFog;
}

plFogEnvironment::plFogEnvironment( hsScalar start, hsScalar end, hsScalar density, hsColorRGBA &color )
{
	Set( start, end, density, &color );
}

plFogEnvironment::plFogEnvironment( FogType type, hsScalar end, hsScalar density, hsColorRGBA &color )
{
	SetExp( type, end, density, &color );
}

plFogEnvironment::~plFogEnvironment()
{
}

//// Set /////////////////////////////////////////////////////////////////////

void	plFogEnvironment::Set( hsScalar start, hsScalar end, hsScalar density, const hsColorRGBA *color )
{
	if( density <= 0.f )
	{
		fType = kNoFog;
		fStart = 0.f;
		fEnd = 0.f;
		fDensity = 0.f;
	}
	else
	{
		fType = kLinearFog;
		fStart = start;
		fEnd = end;
		fDensity = density;
	}
	if( color != nil )
		fColor = *color;
}

void	plFogEnvironment::SetExp( FogType type, hsScalar end, hsScalar density, const hsColorRGBA *color )
{
	hsAssert( type == kExpFog || type == kExp2Fog, "Invalid fog type passed to plFogEnvironment" );
	if( density <= 0.f )
	{
		fType = kNoFog;
		fStart = 0.f;
		fEnd = 0.f;
		fDensity = 0.f;
	}
	else
	{
		fType = type;
		fStart = 0.0f;
		fEnd = end;
		fDensity = density;
	}
	if( color != nil )
		fColor = *color;
}

//// GetParameters ///////////////////////////////////////////////////////////
//	Gets the parameters. Sets start to 0 if the type is not linear (can be
//	nil).

void	plFogEnvironment::GetParameters( hsScalar *start, hsScalar *end, hsScalar *density, hsColorRGBA *color )
{
	hsAssert( fType != kLinearFog || start != nil, "Trying to get non-linear paramters on linear fog!" );
	hsAssert( end != nil && density != nil && color != nil, "Bad pointer to plFogEnvironment::GetParameters()" );

	if( fType == kLinearFog )
		*start = fStart;
	else if( start != nil )
		*start = 0.0f;

	*end = fEnd;
	*density = fDensity;
	*color = fColor;
}

//// GetPipelineParams ///////////////////////////////////////////////////////
//	Gets linear pipeline (DX8) specific parameters. Basically massages our
//	interface values into values that DX8 can use. In this case, we simply
//	scale our end value out by the density. The whole formula is:
//		pipelineEnd = ( end - start ) / density + start

void	plFogEnvironment::GetPipelineParams( hsScalar *start, hsScalar *end, hsColorRGBA *color )
{
//	hsAssert( fType == kLinearFog, "Getting linear pipeline params on non-linear fog!" );

	*color = fColor;
	switch(fType)
	{
	case kLinearFog:
		*start = fStart;
		*end = (fEnd - fStart) / fDensity + fStart;
		break;
	case kExpFog:
		{
			plConst(float) kKnee(0.0f);
			*start = fEnd * kKnee;
			*end = (fEnd - *start) / fDensity + *start;
		}
		break;
	default:
	case kExp2Fog:
		{
			plConst(float) kKnee(0.3f);
			*start = fEnd * kKnee;
			*end = (fEnd - *start) / fDensity + *start;
		}
		break;
	}
}

//// GetPipelineParams ///////////////////////////////////////////////////////
//	Gets exp/exp^2 pipeline (DX8) specific parameters. Basically massages our
//	interface values into values that DX8 can use. In this case, we're going
//	to modulate the density by the end value so that it actually ends at the
//	right spot. 

void	plFogEnvironment::GetPipelineParams( hsScalar *density, hsColorRGBA *color )
{
	const float ln256		= logf( 256.f );
	const float sqrtLn256	= sqrtf( ln256 );

	
	hsAssert( fType == kExpFog || fType == kExp2Fog, "Getting non-linear pipeline params on linear fog!" );

	*density = ( ( fType == kExpFog ) ? ln256: sqrtLn256 ) * fDensity / fEnd;
	*color = fColor;
}

//// Read ////////////////////////////////////////////////////////////////////

void	plFogEnvironment::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );

	fType = s->ReadByte();
	fStart = s->ReadSwapFloat();
	fEnd = s->ReadSwapFloat();
	fDensity = s->ReadSwapFloat();
	fColor.Read( s );
}

//// Write ///////////////////////////////////////////////////////////////////

void	plFogEnvironment::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write( s, mgr );

	s->WriteByte( fType );
	s->WriteSwapFloat( fStart );
	s->WriteSwapFloat( fEnd );
	s->WriteSwapFloat( fDensity );
	fColor.Write( s );
}

