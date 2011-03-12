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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plDniCoordinateInfo.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


#ifdef CLIENT
///////////////////////////////////////////////////////////////////

const UInt8 plDniCoordinateInfo::StreamVersion = 1;


plDniCoordinateInfo::plDniCoordinateInfo()
: fHSpans(0)
, fVSpans(0)
, fTorans(0)
{
}

void plDniCoordinateInfo::CopyFrom( const plDniCoordinateInfo * other )
{
	hsRAMStream stream;
	plCreatable * otherNonConst = const_cast<plDniCoordinateInfo*>( other );	// because plCreatable Write isn't const, but should be.
	otherNonConst->Write( &stream, nil );
	stream.Rewind();
	Read( &stream, nil );
}

void plDniCoordinateInfo::Read( hsStream* s, hsResMgr* mgr )
{
	UInt8 streamVer;
	s->ReadSwap( &streamVer );
	if ( streamVer==StreamVersion )
	{
		s->ReadSwap( &fHSpans );
		s->ReadSwap( &fVSpans );
		s->ReadSwap( &fTorans );
	}
}

void plDniCoordinateInfo::Write( hsStream* s, hsResMgr* mgr )
{
	s->WriteSwap( StreamVersion );
	s->WriteSwap( fHSpans );
	s->WriteSwap( fVSpans );
	s->WriteSwap( fTorans );
}

std::string	plDniCoordinateInfo::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%sDniCoords[%d,%d,%d]", space.c_str(), fHSpans, fVSpans, fTorans );
	return result;
}
#endif // def CLIENT
