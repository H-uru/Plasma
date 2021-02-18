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

#include "plLayerInterface.h"
#include "plLayer.h"

#include "HeadSpin.h"

#include "hsGMatState.h"
#include "hsMatrix44.h"
#include "hsResMgr.h"

#include "pnNetCommon/plSDLTypes.h"

#include "plMessage/plLayRefMsg.h"

plLayerInterface::plLayerInterface() 
:   fUnderLay(),
    fOverLay(),
    fState(),
    fTransform(),
    fPreshadeColor(),
    fRuntimeColor(),
    fAmbientColor(),
    fOpacity(),
    fTexture(),
    fUVWSrc(),
    fLODBias(),
    fSpecularColor(),
    fSpecularPower(),
    fOwnedChannels(),
    fPassThruChannels(),
    fVertexShader(),
    fPixelShader(),
    fBumpEnvXfm()
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

void plLayerInterface::ISetPassThru(uint32_t chans)
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
//      otherwise results are undefined.
// 2) A layer will only become active due to receiving a message.
// 3) A channel value for the stack is the value as set by the
//      last layer that was active. If no layers have ever been
//      active, the value is the static value of the bottom of the stack.
// 4) Since the stack is only Eval'd when visible, the third rule
//      must appear to be true when different layers become active
//      and inactive without ever having been Eval'd.
// 5) Taking advantage of rules 1) and 2), it follows that the last
//      layer to have become active on response to a message is also
//      the last layer to have been active.
// 6) So when a layer becomes active in its MsgReceive(), it notifies
//      all channels above it that it now owns its channels, and they
//      should just pass through those channel values.
// Note that a layer may claim ownership of its channels but then lose
//      ownership (because another layer went active) before ever having
//      been Eval'd.
void plLayerInterface::ClaimChannels(uint32_t chans)
{
    if( fOverLay )
        fOverLay->ISetPassThru(chans);
    fPassThruChannels &= ~chans;
    DirtySynchState(kSDLLayer, 0);
}

uint32_t plLayerInterface::Eval(double secs, uint32_t frame, uint32_t ignore)
{
    if( fUnderLay )
        return fUnderLay->Eval(secs, frame, ignore);

    return uint32_t(0);
}

// Export Only
void plLayerInterface::AttachViaNotify(plLayerInterface *prev)
{
    plLayRefMsg* refMsg = new plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kUnderLay);
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
            fState = nullptr;
        if( !OwnChannel(kPreshadeColor) )
            fPreshadeColor = nullptr;
        if( !OwnChannel( kRuntimeColor ) )
            fRuntimeColor = nullptr;
        if( !OwnChannel(kAmbientColor) )
            fAmbientColor = nullptr;
        if( !OwnChannel( kSpecularColor ) )
            fSpecularColor = nullptr;
        if( !OwnChannel(kOpacity) )
            fOpacity = nullptr;
        if( !OwnChannel(kTransform) )
            fTransform = nullptr;
        if( !OwnChannel(kTexture) )
            fTexture = nullptr;

        if( !OwnChannel(kUVWSrc) )
            fUVWSrc = nullptr;
        if( !OwnChannel(kLODBias) )
            fLODBias = nullptr;
        if( !OwnChannel(kSpecularPower) )
            fSpecularPower = nullptr;

        if( !OwnChannel(kVertexShader) )
            fVertexShader = nullptr;
        if( !OwnChannel(kPixelShader) )
            fPixelShader = nullptr;

        if( !OwnChannel(kBumpEnvXfm) )
            fBumpEnvXfm = nullptr;

        fUnderLay->fOverLay = nullptr;
        fUnderLay = nullptr;
    }
}

// Detach:
// If we are the one being detached, break our links to underlay
//      and then return nullptr, since everything has just been detached
//      from the stack.
// If our underlay is the one being detached, we need to unthread from it
//      and return ourselves.
// If it's not us, and not our underlay, just pass it to our underlay and let
//      it deal.
//
// Return value is new TOP of stack. li is now top of a separate stack.
plLayerInterface* plLayerInterface::Detach(plLayerInterface* li)
{
    if( li == this )
        return nullptr;

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
//      and then just return underlay, since it doesn't even know 
//      about our existence (so it doesn't need to know about the remove).
// If our underlay is the one being removed, we need to unthread it from
//      its underlay (if any), and then thread ourselves onto the underlay's 
//      former underlay.
// If it's not us, and not our underlay, just pass it to our underlay and let
//      it deal.
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

    plLayRefMsg* refMsg = new plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kUnderLay);    
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

bool plLayerInterface::MsgReceive(plMessage* msg)
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