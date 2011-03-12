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
//	plFogEnvironment.h - Header file for the fog environment class			//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plFogEnvironment_h
#define _plFogEnvironment_h

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsTypes.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"
#include "hsUtils.h"


//// plFogEnvironment Class Definition ////////////////////////////////////////////
//	Defines a fog environment. Bite me.

class plFogEnvironment : public hsKeyedObject
{
	protected:

		UInt8		fType;
		hsScalar	fStart;			// Used for linear fog only
		hsScalar	fEnd, fDensity;	// Always used!
		hsColorRGBA	fColor;

	public:

		CLASSNAME_REGISTER( plFogEnvironment );
		GETINTERFACE_ANY( plFogEnvironment, hsKeyedObject );

		enum FogType
		{
			kLinearFog		= 0,
			kExpFog,
			kExp2Fog,
			kNoFog
		};

		plFogEnvironment();
		plFogEnvironment( hsScalar start, hsScalar end, hsScalar density, hsColorRGBA &color );
		plFogEnvironment( FogType type, hsScalar end, hsScalar density, hsColorRGBA &color );
		~plFogEnvironment();

		// Sets the parameters for linear fog
		void	Set( hsScalar start, hsScalar end, hsScalar density, const hsColorRGBA *color = nil );

		// Sets the parameters for exp or exp^2 fog
		void	SetExp( FogType type, hsScalar end, hsScalar density, const hsColorRGBA *color = nil );

		// Sets the color
		void	SetColor( hsColorRGBA &color ) { fColor = color; }

		// Clear the environment to no fog
		void	Clear( void ) { fType = kNoFog; }

		// Gets the type
		UInt8	GetType( void ) { return fType; }

		// Gets the color
		hsColorRGBA	&GetColor( void ) { return fColor; }

		// Gets the parameters. Sets start to 0 if the type is not linear (can be nil)
		void	GetParameters( hsScalar *start, hsScalar *end, hsScalar *density, hsColorRGBA *color );

		// Gets linear pipeline (DX) specific parameters.
		void	GetPipelineParams( hsScalar *start, hsScalar *end, hsColorRGBA *color );

		// Gets exp or exp^2 pipeline (DX) specific parameters.
		void	GetPipelineParams( hsScalar *density, hsColorRGBA *color );

		virtual void Read(hsStream *s, hsResMgr *mgr);
		virtual void Write(hsStream *s, hsResMgr *mgr);
};

#endif //_plFogEnvironment_h

