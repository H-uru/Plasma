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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plDXLightRef.h - Hardware Light DeviceRef Definition					 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	4.25.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXLightRef_h
#define _plDXLightRef_h

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "plDXDeviceRef.h"


//// Definition ///////////////////////////////////////////////////////////////

class plLightInfo;
class plDXLightSettings;
class plDXLightRef : public plDXDeviceRef
{
	public:
		plLightInfo			*fOwner;

		D3DLIGHT9	fD3DInfo;
		UInt32		fD3DIndex;
		hsScalar	fScale;

		plDXLightSettings	*fParentSettings;
		IDirect3DDevice9	*fD3DDevice;

		void			Link( plDXLightRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
		plDXLightRef	*GetNext( void ) { return (plDXLightRef *)fNext; }

		plDXLightRef()
		{
			fOwner = nil;
			fParentSettings = nil;
			fD3DDevice = nil;
			fD3DIndex = -1;
			fScale = 1.f;
		}

		virtual ~plDXLightRef();
		void	Release( void );

		void	UpdateD3DInfo( IDirect3DDevice9 *dev, plDXLightSettings *settings );
};


#endif // _plDXLightRef_h
