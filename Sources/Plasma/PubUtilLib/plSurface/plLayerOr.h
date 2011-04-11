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

#ifndef _plLayerOr_h
#define _plLayerOr_h

#include "plLayerInterface.h"
#include "hsGMatState.h"

class plLayerOr : public plLayerInterface
{
	protected:
		hsGMatState			fOringState;
		hsBool				fDirty;

	public:
		plLayerOr();
		virtual ~plLayerOr();

		CLASSNAME_REGISTER( plLayerOr );
		GETINTERFACE_ANY( plLayerOr, plLayerInterface );

		void	SetBlendFlags( UInt32 f )	{ fOringState.fBlendFlags = f; }
		void	SetClampFlags( UInt32 f )	{ fOringState.fClampFlags = f; }
		void	SetShadeFlags( UInt32 f )	{ fOringState.fShadeFlags = f; }
		void	SetZFlags( UInt32 f )		{ fOringState.fZFlags = f; }
		void	SetMiscFlags( UInt32 f )	{ fOringState.fMiscFlags = f; }
		void	SetState( const hsGMatState& state );

		virtual plLayerInterface*	Attach(plLayerInterface* prev);

		virtual UInt32			Eval(double secs, UInt32 frame, UInt32 ignore);
};

#endif _plLayerOr_h
