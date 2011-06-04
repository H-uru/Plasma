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
//	plCubicRenderTarget Class Header										 //
//	Derived renderTarget class representing a collection of render targets	 //
//	to be used for DYNAMIC cubic environment mapping.						 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plCubicRenderTarget_h
#define _plCubicRenderTarget_h

#include "plRenderTarget.h"
#include "hsMatrix44.h"


//// Class Definition /////////////////////////////////////////////////////////

class plCubicRenderTarget : public plRenderTarget
{
	protected:

		//// Protected Members ////

		plRenderTarget	*fFaces[6];
		hsMatrix44		fWorldToCameras[6];
		hsMatrix44		fCameraToWorlds[6];
	
	public:

		//// Public Data ////

		enum Faces
		{
			kLeftFace = 0,
			kRightFace,
			kFrontFace,
			kBackFace,
			kTopFace,
			kBottomFace
		};

		//// Public Members ////

		CLASSNAME_REGISTER( plCubicRenderTarget );
		GETINTERFACE_ANY( plCubicRenderTarget, plRenderTarget );

		plCubicRenderTarget()
		{
			fFaces[0] = fFaces[1] = fFaces[2] = fFaces[3] = fFaces[4] = fFaces[5] = nil;
		}

		plCubicRenderTarget( UInt16 flags, UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth = -1, UInt8 sDepth = -1 ) 
			: plRenderTarget( flags, width, height, bitDepth, zDepth, sDepth )
		{
			int		i;


			for( i = 0; i < 6; i++ )
			{
				fFaces[i] = TRACKED_NEW plRenderTarget( flags, width, height, bitDepth, zDepth, sDepth );
				fFaces[i]->fParent = this;
				fWorldToCameras[i].Reset();
				fCameraToWorlds[i].Reset();
			}
		}

		virtual ~plCubicRenderTarget()
		{
			int			i;


			for( i = 0; i < 6; i++ ) 
				delete fFaces[i];
		}

		// Get the total size in bytes
		virtual UInt32	GetTotalSize( void ) const;

		virtual void				SetCameraMatrix(const hsPoint3& pos);
		virtual const hsMatrix44&	GetWorldToCamera(UInt8 face) const { return fWorldToCameras[face]; }
		virtual const hsMatrix44&	GetCameraToWorld(UInt8 face) const { return fCameraToWorlds[face]; }

		plRenderTarget	*GetFace(UInt8 face) const { return fFaces[face]; }

		virtual UInt32	Read(hsStream *s);
		virtual UInt32	Write(hsStream *s);

};


#endif // _plCubicRenderTarget_h
