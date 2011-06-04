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
//	plRenderTarget.cpp - RenderTarget functions					   			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	7.19.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plRenderTarget.h"
#include "plCubicRenderTarget.h"
#include "hsStream.h"
#include "hsGDeviceRef.h"

#include "plPipeline.h"
#include "plgDispatch.h"
#include "../pnMessage/plPipeResMakeMsg.h"

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



hsBool plRenderTarget::MsgReceive(plMessage* msg)
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

UInt32	plRenderTarget::Read( hsStream *s )
{
	UInt32	total = plBitmap::Read( s );

	fWidth = s->ReadSwap16();
	fHeight = s->ReadSwap16();

	fProportionalViewport = s->ReadBool();
	if( fProportionalViewport )
	{
		fViewport.fProportional.fLeft = s->ReadSwapScalar();
		fViewport.fProportional.fTop = s->ReadSwapScalar();
		fViewport.fProportional.fRight = s->ReadSwapScalar();
		fViewport.fProportional.fBottom = s->ReadSwapScalar();
	}
	else
	{
		fViewport.fAbsolute.fLeft = s->ReadSwap16();
		fViewport.fAbsolute.fTop = s->ReadSwap16();
		fViewport.fAbsolute.fRight = s->ReadSwap16();
		fViewport.fAbsolute.fBottom = s->ReadSwap16();
	}

	fZDepth = s->ReadByte();
	fStencilDepth = s->ReadByte();

	return total + 2 * 2 + 2 + 4 * ( fProportionalViewport ? sizeof( hsScalar ) : sizeof( UInt16 ) ) + sizeof( hsBool );
}

UInt32	plRenderTarget::Write( hsStream *s )
{
	UInt32	total = plBitmap::Write( s );

	s->WriteSwap16( fWidth );
	s->WriteSwap16( fHeight );

	s->WriteBool( fProportionalViewport );
	if( fProportionalViewport )
	{
		s->WriteSwapScalar( fViewport.fProportional.fLeft );
		s->WriteSwapScalar( fViewport.fProportional.fTop );
		s->WriteSwapScalar( fViewport.fProportional.fRight );
		s->WriteSwapScalar( fViewport.fProportional.fBottom );
	}
	else
	{
		s->WriteSwap16( fViewport.fAbsolute.fLeft );
		s->WriteSwap16( fViewport.fAbsolute.fTop );
		s->WriteSwap16( fViewport.fAbsolute.fRight );
		s->WriteSwap16( fViewport.fAbsolute.fBottom );
	}

	s->WriteByte( fZDepth );
	s->WriteByte( fStencilDepth );

	return total + 2 * 2 + 2 + 4 * ( fProportionalViewport ? sizeof( hsScalar ) : sizeof( UInt16 ) ) + sizeof( hsBool );
}

///////////////////////////////////////////////////////////////////////////////
//// plCubicRenderTarget Functions ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UInt32	plCubicRenderTarget::Read( hsStream *s )
{
	int		i;
	UInt32	total = plRenderTarget::Read( s );


	for( i = 0; i < 6; i++ )
	{
		if( fFaces[ i ] == nil )
			fFaces[ i ] = TRACKED_NEW plRenderTarget();

		fFaces[ i ]->fParent = this;
		total += fFaces[ i ]->Read( s );
	}

	return total;
}

UInt32	plCubicRenderTarget::Write( hsStream *s )
{
	int		i;
	UInt32	total = plRenderTarget::Write( s );
	

	for( i = 0; i < 6; i++ )
	{
		total += fFaces[ i ]->Write( s );
	}

	return total;
}

UInt32	plCubicRenderTarget::GetTotalSize( void ) const
{
	UInt32		size = 0, i;
	
	for( i = 0; i < 6; i++ )
	{
		if( fFaces[ i ] != nil )
			size += fFaces[ i ]->GetTotalSize();
	}

	return size;
}

//// SetCameraMatrix //////////////////////////////////////////////////////////

void	plCubicRenderTarget::SetCameraMatrix(const hsPoint3& pos)
{
	hsMatrix44::MakeEnvMapMatrices(pos, fWorldToCameras, fCameraToWorlds);
}
