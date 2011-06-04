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

#include "hsTypes.h"
#include "plLayerOr.h"

plLayerOr::plLayerOr()
{
	fState = TRACKED_NEW hsGMatState;
	fState->Reset();

	fOwnedChannels = kState;
}

plLayerOr::~plLayerOr()
{
}

void	plLayerOr::SetState( const hsGMatState& state )
{
	SetBlendFlags( state.fBlendFlags );
	SetClampFlags( state.fClampFlags );
	SetShadeFlags( state.fShadeFlags );
	SetZFlags( state.fZFlags );
	SetMiscFlags( state.fMiscFlags );
}

plLayerInterface	*plLayerOr::Attach( plLayerInterface* prev )
{
	fDirty = true;

	return plLayerInterface::Attach( prev );
}

UInt32 plLayerOr::Eval(double secs, UInt32 frame, UInt32 ignore)
{
	UInt32 ret = plLayerInterface::Eval(secs, frame, ignore);
	if( fUnderLay )
	{
		if( fDirty || (ret & kState) )
		{
			*fState = fUnderLay->GetState();
			if( fOringState.fBlendFlags & hsGMatState::kBlendMask )
				fState->fBlendFlags &= ~hsGMatState::kBlendMask;
			fState->fBlendFlags |= fOringState.fBlendFlags;

			fState->fClampFlags = fUnderLay->GetClampFlags() | fOringState.fClampFlags;
			fState->fShadeFlags = fUnderLay->GetShadeFlags() | fOringState.fShadeFlags;
			fState->fZFlags = fUnderLay->GetZFlags() | fOringState.fZFlags;
			fState->fMiscFlags = fUnderLay->GetMiscFlags() | fOringState.fMiscFlags;

			fDirty = false;
		}
	}
	return ret;
}
