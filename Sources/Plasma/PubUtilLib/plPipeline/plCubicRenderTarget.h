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
//  plCubicRenderTarget Class Header                                         //
//  Derived renderTarget class representing a collection of render targets   //
//  to be used for DYNAMIC cubic environment mapping.                        //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
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

        plRenderTarget  *fFaces[6];
        hsMatrix44      fWorldToCameras[6];
        hsMatrix44      fCameraToWorlds[6];
    
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
            fFaces[0] = fFaces[1] = fFaces[2] = fFaces[3] = fFaces[4] = fFaces[5] = nullptr;
        }

        plCubicRenderTarget( uint16_t flags, uint16_t width, uint16_t height, uint8_t bitDepth, uint8_t zDepth = -1, uint8_t sDepth = -1 ) 
            : plRenderTarget( flags, width, height, bitDepth, zDepth, sDepth )
        {
            int     i;


            for( i = 0; i < 6; i++ )
            {
                fFaces[i] = new plRenderTarget( flags, width, height, bitDepth, zDepth, sDepth );
                fFaces[i]->fParent = this;
                fWorldToCameras[i].Reset();
                fCameraToWorlds[i].Reset();
            }
        }

        virtual ~plCubicRenderTarget()
        {
            int         i;


            for( i = 0; i < 6; i++ ) 
                delete fFaces[i];
        }

        // Get the total size in bytes
        uint32_t  GetTotalSize() const override;

        virtual void                SetCameraMatrix(const hsPoint3& pos);
        virtual const hsMatrix44&   GetWorldToCamera(uint8_t face) const { return fWorldToCameras[face]; }
        virtual const hsMatrix44&   GetCameraToWorld(uint8_t face) const { return fCameraToWorlds[face]; }

        plRenderTarget  *GetFace(uint8_t face) const { return fFaces[face]; }

        uint32_t  Read(hsStream *s) override;
        uint32_t  Write(hsStream *s) override;

};


#endif // _plCubicRenderTarget_h
