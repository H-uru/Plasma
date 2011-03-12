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
#include "plLayerInterface.h"
#include "../plMessage/plLayRefMsg.h"
#include "plLayer.h"
#include "hsMatrix44.h"
#include "hsGMatState.h"
#include "hsResMgr.h"
#include "../pnNetCommon/plSDLTypes.h"

plLayerInterface::plLayerInterface() 
:	fUnderLay(nil),
	fOverLay(nil),
	fState(nil), 
	fTransform(nil), 
	fPreshadeColor(nil),
	fRuntimeColor(nil),
	fAmbientColor(nil),
	fOpacity(nil),
	fTexture(nil),
	fUVWSrc(nil),
	fLODBias(nil),
	fSpecularColor(nil),
	fSpecularPower(nil),
	fOwnedChannels(0),
	fPassThruChannels(0),
	fVertexShader(nil),
	fPixelShader(nil),
	fBumpEnvXfm(nil)
{

}

plLayerInterface::~plLayerInterface()
{
	if( fUnderLay )
		Detach(fUnderLay);

	delete fState;
	delete fPreshadeColor;
	delete fRuntimeColor;
	delete fAmbientColor;
	delete fSpecularColor;
	delete fOpacity;
	delete fTransform;

	delete fTexture;
	
	delete fUVWSrc;
	delete fLODBias;
	delete fSpecularPower;

	delete fVertexShader;
	delete fPixelShader;

	delete fBumpEnvXfm;
}

void plLayerInterface::ISetPassThru(UInt32 chans)
{
	fPassThruChannels |= chans;
	if( fOverLay )
		fOverLay->ISetPassThru(chans);

	// Since plLayerAnimation is the only derived class that uses its
	// fPassThruChannels info, it's the only one that actually saves
	// it to state.
	DirtySynchState(kSDLLayer, 0);
}

// The arbitration rules for different layers on the same stack
// wanting to control the same channel are currently:
// 1) Only one write-only value setter can be active at a time,
//		otherwise results are undefined.
// 2) A layer will only become active due to receiving a message.
// 3) A channel value for the stack is the value as set by the
//		last layer that was active. If no layers have ever been
//		active, the value is the static value of the bottom of the stack.
// 4) Since the stack is only Eval'd when visible, the third rule
//		must appear to be true when different layers become active
//		and inactive without ever having been Eval'd.
// 5) Taking advantage of rules 1) and 2), it follows that the last
//		layer to have become active on response to a message is also
//		the last layer to have been active.
// 6) So when a layer becomes active in it's MsgReceive(), it notifies
//		all channels above it that it now owns its channels, and they
//		should just pass through those channel values.
// Note that a layer may claim ownership of its channels but then lose
//		ownership (because another layer went active) before ever having
//		been Eval'd.
void plLayerInterface::ClaimChannels(UInt32 chans)
{
	if( fOverLay )
		fOverLay->ISetPassThru(chans);
	fPassThruChannels &= ~chans;
	DirtySynchState(kSDLLayer, 0);
}

UInt32 plLayerInterface::Eval(double secs, UInt32 frame, UInt32 ignore)
{
	if( fUnderLay )
		return fUnderLay->Eval(secs, frame, ignore);

	return UInt32(0);
}

// Export Only
void plLayerInterface::AttachViaNotify(plLayerInterface *prev)
{
	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kUnderLay);
	hsgResMgr::ResMgr()->AddViaNotify(prev->GetKey(), refMsg, plRefFlags::kActiveRef);
}

plLayerInterface* plLayerInterface::Attach(plLayerInterface* prev)
{
	if( !prev )
		return this;

	if( fUnderLay == prev )
		return this;

	if( fUnderLay )
	{
		fUnderLay->Attach(prev);
		prev = fUnderLay;
	}

	if( !OwnChannel(kState) )
		fState = prev->fState;

	if( !OwnChannel(kPreshadeColor) )
		fPreshadeColor = prev->fPreshadeColor;

	if( !OwnChannel( kRuntimeColor ) )
		fRuntimeColor = prev->fRuntimeColor;

	if( !OwnChannel(kAmbientColor) )
		fAmbientColor = prev->fAmbientColor;

	if( !OwnChannel( kSpecularColor ) )
		fSpecularColor = prev->fSpecularColor;

	if( !OwnChannel(kOpacity) )
		fOpacity = prev->fOpacity;

	if( !OwnChannel(kTransform) )
		fTransform = prev->fTransform;

	if( !OwnChannel(kTexture) )
		fTexture = prev->fTexture;

	if( !OwnChannel(kUVWSrc) )
		fUVWSrc = prev->fUVWSrc;

	if( !OwnChannel(kLODBias) )
		fLODBias = prev->fLODBias;

	if( !OwnChannel(kSpecularPower) )
		fSpecularPower = prev->fSpecularPower;

	if( !OwnChannel(kVertexShader) )
		fVertexShader = prev->fVertexShader;

	if( !OwnChannel(kPixelShader) )
		fPixelShader = prev->fPixelShader;

	if( !OwnChannel(kBumpEnvXfm) )
		fBumpEnvXfm = prev->fBumpEnvXfm;

	fUnderLay = prev;
	prev->fOverLay = this;

	return this;
}

void plLayerInterface::IUnthread()
{
	if( fUnderLay )
	{
		if( !OwnChannel(kState) )
			fState = nil;
		if( !OwnChannel(kPreshadeColor) )
			fPreshadeColor = nil;
		if( !OwnChannel( kRuntimeColor ) )
			fRuntimeColor = nil;
		if( !OwnChannel(kAmbientColor) )
			fAmbientColor = nil;
		if( !OwnChannel( kSpecularColor ) )
			fSpecularColor = nil;
		if( !OwnChannel(kOpacity) )
			fOpacity = nil;
		if( !OwnChannel(kTransform) )
			fTransform = nil;
		if( !OwnChannel(kTexture) )
			fTexture = nil;

		if( !OwnChannel(kUVWSrc) )
			fUVWSrc = nil;
		if( !OwnChannel(kLODBias) )
			fLODBias = nil;
		if( !OwnChannel(kSpecularPower) )
			fSpecularPower = nil;

		if( !OwnChannel(kVertexShader) )
			fVertexShader = nil;
		if( !OwnChannel(kPixelShader) )
			fPixelShader = nil;

		if( !OwnChannel(kBumpEnvXfm) )
			fBumpEnvXfm = nil;

		fUnderLay->fOverLay = nil;
		fUnderLay = nil;
	}
}

// Detach:
// If we are the one being detached, break our links to underlay
//		and then return nil, since everything has just been detached
//		from the stack.
// If our underlay is the one being detached, we need to unthread from it
//		and return ourselves.
// If it's not us, and not our underlay, just pass it to our underlay and let
//		it deal.
//
// Return value is new TOP of stack. li is now top of a separate stack.
plLayerInterface* plLayerInterface::Detach(plLayerInterface* li)
{
	if( li == this )
		return nil;

	if( li == fUnderLay )
	{
		IUnthread();
		return this;
	}

	fUnderLay->Detach(li);

	return this;
}

// Remove:
// If we are the one being removed, break our links to underlay
//		and then just return underlay, since it doesn't even know 
//		about our existence (so it doesn't need to know about the remove).
// If our underlay is the one being removed, we need to unthread it from
//		its underlay (if any), and then thread ourselves onto the underlay's 
//		former underlay.
// If it's not us, and not our underlay, just pass it to our underlay and let
//		it deal.
//
// Return value is new TOP of stack.
plLayerInterface* plLayerInterface::Remove(plLayerInterface* li)
{
	plLayerInterface* under = fUnderLay;

	if( li == this )
	{
		
		IUnthread();

		return under;
	}

	// This is an error, because it means we're being asked
	// to detach from something we aren't attached to.
	if( !under )
	{
		hsAssert(false, "Detaching from unknown layerinterface");
		return this;
	}

	IUnthread();

	plLayerInterface* newUnderLay = under->Remove(li);

	Attach(newUnderLay);

	return this;
}

plLayerInterface *plLayerInterface::GetAttached()
{
	return fUnderLay;
}

void plLayerInterface::Read(hsStream* s, hsResMgr* mgr)
{
	plSynchedObject::Read(s, mgr);

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kUnderLay);	
	plKey key = mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);
	if( key && !fUnderLay )
		Attach(plLayer::DefaultLayer());

	// Temporary setting default netgroup by our key.
	SetNetGroup(SelectNetGroup(GetKey()));
}

void plLayerInterface::Write(hsStream* s, hsResMgr* mgr)
{
	plSynchedObject::Write(s, mgr);

    mgr->WriteKey(s, fUnderLay);
}

hsBool plLayerInterface::MsgReceive(plMessage* msg)
{
	plLayRefMsg* refMsg = plLayRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		switch( refMsg->fType )
		{
		case plLayRefMsg::kUnderLay:
			{
				plLayerInterface* underLay = plLayerInterface::ConvertNoRef(refMsg->GetRef());
				if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				{
					if( fUnderLay )
						Detach(fUnderLay);

					Attach(underLay);
				}
				else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				{
					Detach(fUnderLay);
				}
				return true;
			}
		}
	}
	return plSynchedObject::MsgReceive(msg);
}