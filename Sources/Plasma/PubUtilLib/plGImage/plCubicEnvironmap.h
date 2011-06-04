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
//	plCubicEnvironmap Class Header											 //
//	Derived bitmap class representing a collection of mipmaps to be used for //
//	cubic environment mapping.												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plCubicEnvironmap_h
#define _plCubicEnvironmap_h


#include "plBitmap.h"

class plMipmap;

//// Class Definition /////////////////////////////////////////////////////////

class plCubicEnvironmap : public plBitmap
{
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

		plCubicEnvironmap();
		virtual ~plCubicEnvironmap();

		CLASSNAME_REGISTER( plCubicEnvironmap );
		GETINTERFACE_ANY( plCubicEnvironmap, plBitmap );


		// Get the total size in bytes
		virtual UInt32	GetTotalSize( void ) const;

		virtual void	Read( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Read( s, mgr ); this->Read( s ); }
		virtual void	Write( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Write( s, mgr ); this->Write( s ); }

		plMipmap	*GetFace( UInt8 face ) const { return fFaces[ face ]; }

		// Export-only: Copy the mipmap given into a face
		void		CopyToFace( plMipmap *mip, UInt8 face );

	protected:

		//// Protected Members ////

		plMipmap		*fFaces[ 6 ];
		hsBool			fInitialized;

		virtual UInt32	Read( hsStream *s );
		virtual UInt32	Write( hsStream *s );

};


#endif // plCubicEnvironmap_h
