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
#include "plLayerShadowBase.h"

///////////////////////////////////////////////////////////////////////////
plLayerLightBase::plLayerLightBase()
:	fDirty(true)
{
	fOwnedChannels = kState
				| kAmbientColor
				| kPreshadeColor;

	fState = TRACKED_NEW hsGMatState;
	fState->Reset();
	
	fAmbientColor = TRACKED_NEW hsColorRGBA;
	fAmbientColor->Set(0,0,0,1.f);

	fPreshadeColor = TRACKED_NEW hsColorRGBA;
	fPreshadeColor->Set(0,0,0,1.f);
}

plLayerLightBase::~plLayerLightBase()
{
}

plLayerInterface* plLayerLightBase::Attach(plLayerInterface* prev)
{
	fDirty = true;
	return plLayerInterface::Attach(prev);
}

UInt32 plLayerLightBase::Eval(double secs, UInt32 frame, UInt32 ignore)
{
	UInt32 ret = plLayerInterface::Eval(secs, frame, ignore);
	if( fUnderLay )
	{
		if( fDirty || (ret & kState) )
		{
			*fState = fUnderLay->GetState();

			UInt32 blend = fState->fBlendFlags;

			fState->fBlendFlags &= ~hsGMatState::kBlendMask;

			switch( blend )
			{
			case hsGMatState::kBlendAlpha:
				fState->fBlendFlags |= hsGMatState::kBlendAddColorTimesAlpha;
				break;
			default:
				fState->fBlendFlags |= hsGMatState::kBlendAdd;
				break;
			}

			fState->fZFlags |= hsGMatState::kZNoZWrite;

			fState->fShadeFlags |= hsGMatState::kShadeIgnoreVtxIllum;

			ret |= kState | kAmbientColor;

			fDirty = false;
		}
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////
plLayerShadowBase::plLayerShadowBase()
:	fDirty(true)
{
	fOwnedChannels = kState
				| kAmbientColor
				| kPreshadeColor;

	fState = TRACKED_NEW hsGMatState;
	fState->Reset();
	
	fAmbientColor = TRACKED_NEW hsColorRGBA;
	fAmbientColor->Set(0,0,0,1.f);

	fPreshadeColor = TRACKED_NEW hsColorRGBA;
	fPreshadeColor->Set(0,0,0,1.f);
}

plLayerShadowBase::~plLayerShadowBase()
{
}

plLayerInterface* plLayerShadowBase::Attach(plLayerInterface* prev)
{
	fDirty = true;
	return plLayerInterface::Attach(prev);
}

UInt32 plLayerShadowBase::Eval(double secs, UInt32 frame, UInt32 ignore)
{
	UInt32 ret = plLayerInterface::Eval(secs, frame, ignore);
	if( fUnderLay )
	{
		if( fDirty || (ret & kState) )
		{
			*fState = fUnderLay->GetState();
			fState->fBlendFlags &= ~hsGMatState::kBlendMask;
			//WHITE
			fState->fBlendFlags |= hsGMatState::kBlendAlpha;

			fState->fZFlags |= hsGMatState::kZNoZWrite;

			fState->fShadeFlags |= hsGMatState::kShadeIgnoreVtxIllum;

			ret |= kState | kAmbientColor | kPreshadeColor;

			fDirty = false;
		}
	}

	return ret;
}