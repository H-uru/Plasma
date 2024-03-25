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
//  plRenderTarget.cpp - RenderTarget functions                              //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  7.19.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "plRenderTarget.h"

#include "plCubicRenderTarget.h"

#include "plgDispatch.h"
#include "hsGDeviceRef.h"
#include "hsStream.h"
#include "plPipeline.h"

#include "pnMessage/plPipeResMakeMsg.h"

///////////////////////////////////////////////////////////////////////////////
//// plRenderTarget Functions /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void plRenderTarget::SetKey(plKey k)
{
    hsKeyedObject::SetKey(k);
    if( k )
    {
        if( !fParent )
            plgDispatch::Dispatch()->RegisterForExactType( plPipeRTMakeMsg::Index(), GetKey() );
    }
}



bool plRenderTarget::MsgReceive(plMessage* msg)
{
    plPipeRTMakeMsg* make = plPipeRTMakeMsg::ConvertNoRef(msg);
    if( make )
    {
        if( !GetDeviceRef() || GetDeviceRef()->IsDirty() )
        {
            make->Pipeline()->MakeRenderTargetRef(this);
        }
        return true;
    }
    return plBitmap::MsgReceive(msg);
}

uint32_t  plRenderTarget::Read( hsStream *s )
{
    uint32_t  total = plBitmap::Read( s );

    fWidth = s->ReadLE16();
    fHeight = s->ReadLE16();

    fProportionalViewport = s->ReadBool();
    if( fProportionalViewport )
    {
        fViewport.fProportional.fLeft = s->ReadLEFloat();
        fViewport.fProportional.fTop = s->ReadLEFloat();
        fViewport.fProportional.fRight = s->ReadLEFloat();
        fViewport.fProportional.fBottom = s->ReadLEFloat();
    }
    else
    {
        fViewport.fAbsolute.fLeft = s->ReadLE16();
        fViewport.fAbsolute.fTop = s->ReadLE16();
        fViewport.fAbsolute.fRight = s->ReadLE16();
        fViewport.fAbsolute.fBottom = s->ReadLE16();
    }

    fZDepth = s->ReadByte();
    fStencilDepth = s->ReadByte();

    return total + 2 * 2 + 2 + 4 * ( fProportionalViewport ? sizeof( float ) : sizeof( uint16_t ) ) + sizeof( bool );
}

uint32_t  plRenderTarget::Write( hsStream *s )
{
    uint32_t  total = plBitmap::Write( s );

    s->WriteLE16( fWidth );
    s->WriteLE16( fHeight );

    s->WriteBool( fProportionalViewport );
    if( fProportionalViewport )
    {
        s->WriteLEFloat(fViewport.fProportional.fLeft);
        s->WriteLEFloat(fViewport.fProportional.fTop);
        s->WriteLEFloat(fViewport.fProportional.fRight);
        s->WriteLEFloat(fViewport.fProportional.fBottom);
    }
    else
    {
        s->WriteLE16( fViewport.fAbsolute.fLeft );
        s->WriteLE16( fViewport.fAbsolute.fTop );
        s->WriteLE16( fViewport.fAbsolute.fRight );
        s->WriteLE16( fViewport.fAbsolute.fBottom );
    }

    s->WriteByte( fZDepth );
    s->WriteByte( fStencilDepth );

    return total + 2 * 2 + 2 + 4 * ( fProportionalViewport ? sizeof( float ) : sizeof( uint16_t ) ) + sizeof( bool );
}

///////////////////////////////////////////////////////////////////////////////
//// plCubicRenderTarget Functions ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32_t  plCubicRenderTarget::Read( hsStream *s )
{
    int     i;
    uint32_t  total = plRenderTarget::Read( s );


    for( i = 0; i < 6; i++ )
    {
        if (fFaces[i] == nullptr)
            fFaces[ i ] = new plRenderTarget();

        fFaces[ i ]->fParent = this;
        total += fFaces[ i ]->Read( s );
    }

    return total;
}

uint32_t  plCubicRenderTarget::Write( hsStream *s )
{
    int     i;
    uint32_t  total = plRenderTarget::Write( s );
    

    for( i = 0; i < 6; i++ )
    {
        total += fFaces[ i ]->Write( s );
    }

    return total;
}

uint32_t  plCubicRenderTarget::GetTotalSize() const
{
    uint32_t      size = 0, i;
    
    for( i = 0; i < 6; i++ )
    {
        if (fFaces[i] != nullptr)
            size += fFaces[ i ]->GetTotalSize();
    }

    return size;
}

//// SetCameraMatrix //////////////////////////////////////////////////////////

void    plCubicRenderTarget::SetCameraMatrix(const hsPoint3& pos)
{
    hsMatrix44::MakeEnvMapMatrices(pos, fWorldToCameras, fCameraToWorlds);
}
