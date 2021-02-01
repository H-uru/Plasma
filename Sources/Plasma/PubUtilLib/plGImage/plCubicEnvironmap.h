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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plCubicEnvironmap Class Header                                           //
//  Derived bitmap class representing a collection of mipmaps to be used for //
//  cubic environment mapping.                                               //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
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
        uint32_t  GetTotalSize() const override;

        void    Read(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Read(s, mgr); this->Read(s); }
        void    Write(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Write(s, mgr); this->Write(s); }

        plMipmap    *GetFace( uint8_t face ) const { return fFaces[ face ]; }

        // Export-only: Copy the mipmap given into a face
        void        CopyToFace( plMipmap *mip, uint8_t face );

    protected:

        //// Protected Members ////

        plMipmap        *fFaces[ 6 ];
        bool            fInitialized;

        uint32_t  Read(hsStream *s) override;
        uint32_t  Write(hsStream *s) override;

};


#endif // plCubicEnvironmap_h
