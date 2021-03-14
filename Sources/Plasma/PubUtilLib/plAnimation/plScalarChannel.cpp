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

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// singular
#include "plScalarChannel.h"

// global
#include "hsResMgr.h"

// other
#include "plGLight/plLightInfo.h"
#include "plInterp/plController.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plSDL/plSDL.h"


/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarChannel
//
/////////////////////////////////////////////////////////////////////////////////////////

// value --------------------------------------------------------
// ------
const float & plScalarChannel::Value(double time, bool peek)
{
    return fResult;
}

// value --------------------------------------------------------------
// ------
void plScalarChannel::Value(float &scalar, double time, bool peek)
{
    scalar = Value(time, peek);
}

// MakeCombine -----------------------------------------------
// ------------
plAGChannel * plScalarChannel::MakeCombine(plAGChannel *other)
{
    return nullptr;
}

// MakeBlend ---------------------------------------------------
// ----------
plAGChannel * plScalarChannel::MakeBlend(plAGChannel * channelB,
                                         plScalarChannel * channelBias,
                                         int blendPriority)
{
    plScalarChannel * chanB = plScalarChannel::ConvertNoRef(channelB);
    plScalarChannel * chanBias = plScalarChannel::ConvertNoRef(channelBias);
    plAGChannel * result = this;

    if (chanB)
    {
        result = new plScalarBlend(this, chanB, chanBias);
    } else {
        hsStatusMessage("Blend operation failed.");
    }
    return result;
}

// MakeZeroState -----------------------------
// --------------
plAGChannel * plScalarChannel::MakeZeroState()
{
    return new plScalarConstant(Value(0));
}

// MakeTimeScale --------------------------------------------------------
// --------------
plAGChannel * plScalarChannel::MakeTimeScale(plScalarChannel *timeSource)
{
    return new plScalarTimeScale(this, timeSource);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarConstant
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------
// -----
plScalarConstant::plScalarConstant()
{
}

// ctor ------------------------------------------
// -----
plScalarConstant::plScalarConstant(float value)
{
    fResult = value;
}

// dtor -----------------------------
// -----
plScalarConstant::~plScalarConstant()
{
}

void plScalarConstant::Read(hsStream *stream, hsResMgr *mgr)
{
    plScalarChannel::Read(stream, mgr);
    fResult = stream->ReadLEFloat();
}

void plScalarConstant::Write(hsStream *stream, hsResMgr *mgr)
{
    plScalarChannel::Write(stream, mgr);
    stream->WriteLEFloat(fResult);
}



////////////////////
// PLSCALARTIMESCALE
////////////////////
// Insert into the graph when you need to change the speed or direction of time
// Also serves as a handy instancing node, since it just passes its data through.

// CTOR
plScalarTimeScale::plScalarTimeScale()
: fTimeSource(),
  fChannelIn()
{
}

// CTOR (channel, converter)
plScalarTimeScale::plScalarTimeScale(plScalarChannel *channel, plScalarChannel *timeSource)
: fChannelIn(channel),
  fTimeSource(timeSource)
{
}

// DTOR
plScalarTimeScale::~plScalarTimeScale()
{
}

bool plScalarTimeScale::IsStoppedAt(double time)
{
    return fTimeSource->IsStoppedAt(time);
}

// VALUE
const float & plScalarTimeScale::Value(double time, bool peek)
{
    fResult = fChannelIn->Value(fTimeSource->Value(time, peek));

    return fResult;
}

// DETACH
plAGChannel * plScalarTimeScale::Detach(plAGChannel * detach)
{
    plAGChannel *result = this;
    
    fChannelIn = plScalarChannel::ConvertNoRef(fChannelIn->Detach(detach));
    
    if(!fChannelIn || detach == this)
        result = nullptr;
    
    if(result != this)
        delete this;
    
    return result;
    
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarBlend
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------
// -----
plScalarBlend::plScalarBlend()
: fChannelA(),
  fChannelB(),
  fChannelBias()
{
}

// ctor ----------------------------------------------------------------------------
// -----
plScalarBlend::plScalarBlend(plScalarChannel * channelA, plScalarChannel * channelB,
                             plScalarChannel * channelBias)
: fChannelA(channelA),
  fChannelB(channelB),
  fChannelBias(channelBias)
{
}

// dtor -----------------------
// -----
plScalarBlend::~plScalarBlend()
{
    fChannelA = nullptr;
    fChannelB = nullptr;
    fChannelBias = nullptr;
}

// IsStoppedAt -------------------------------
// ------------
bool plScalarBlend::IsStoppedAt(double time)
{
    float blend = fChannelBias->Value(time);
    if (blend == 0)
        return fChannelA->IsStoppedAt(time);
    if (blend == 1)
        return fChannelB->IsStoppedAt(time);

    return (fChannelA->IsStoppedAt(time) && fChannelB->IsStoppedAt(time));
}

// Value ------------------------------------------------------
// ------
const float & plScalarBlend::Value(double time, bool peek)
{
    float curBlend = fChannelBias->Value(time, peek);
    if(curBlend == 0) {
        fChannelA->Value(fResult, time, peek);
    } else {
        if(curBlend == 1) {
            fChannelB->Value(fResult, time, peek);
        } else {
            const float &scalarA = fChannelA->Value(time, peek);
            const float &scalarB = fChannelB->Value(time, peek);
            fResult = scalarA + curBlend * (scalarB - scalarA);
        }
    }
    return fResult;
}


// Detach ----------------------------------------------
// -------
plAGChannel * plScalarBlend::Detach(plAGChannel *remove)
{
    plAGChannel *result = this;

    // it's possible that the incoming channel could reside down *all* of our
    // branches (it's a graph, not a tree,) so we always pass down all limbs
    fChannelBias = plScalarChannel::ConvertNoRef(fChannelBias->Detach(remove));
    fChannelA = plScalarChannel::ConvertNoRef(fChannelA->Detach(remove));
    fChannelB = plScalarChannel::ConvertNoRef(fChannelB->Detach(remove));

    if(!fChannelBias)
        result = fChannelA;
    else if(fChannelA && !fChannelB)
        result = fChannelA;
    else if(fChannelB && !fChannelA)
        result = fChannelB;
    else if(!fChannelA && !fChannelB)
        result = nullptr;

    if(result != this)
        delete this;

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARCONTROLLERCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------------------------
// -----
plScalarControllerChannel::plScalarControllerChannel()
: fController()
{
}

// ctor ----------------------------------------------------------------------------
// -----
plScalarControllerChannel::plScalarControllerChannel(plController *controller)
: fController(controller)
{
}

// dtor -----------------------------------------------
// -----
plScalarControllerChannel::~plScalarControllerChannel()
{
    if(fController) {
        delete fController;
        fController = nullptr;
    }
}

// Value ------------------------------------------------------------------
// ------
const float & plScalarControllerChannel::Value(double time, bool peek)
{
    return Value(time, peek, nullptr);
}

// Value ------------------------------------------------------------------
// ------
const float & plScalarControllerChannel::Value(double time, bool peek,
                                                  plControllerCacheInfo *cache)
{       
    fController->Interp((float)time, &fResult, cache);
    return fResult;
}

// MakeCacheChannel ------------------------------------------------------------
// -----------------
plAGChannel *plScalarControllerChannel::MakeCacheChannel(plAnimTimeConvert *atc)
{
    plControllerCacheInfo *cache = fController->CreateCache();
    cache->SetATC(atc);
    return new plScalarControllerCacheChannel(this, cache);
}

// Write -------------------------------------------------------------
// ------
void plScalarControllerChannel::Write(hsStream *stream, hsResMgr *mgr)
{
    plScalarChannel::Write(stream, mgr);

    hsAssert(fController, "Trying to write plScalarControllerChannel with nil controller. File will not be importable.");
    mgr->WriteCreatable(stream, fController);
}

// Read -------------------------------------------------------------
// -----
void plScalarControllerChannel::Read(hsStream *stream, hsResMgr *mgr)
{
    plScalarChannel::Read(stream, mgr);
    
    fController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARCONTROLLERCACHECHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------------------------------------
// -----
plScalarControllerCacheChannel::plScalarControllerCacheChannel()
: fControllerChannel(),
  fCache()
{
}

// ctor ---------------------------------------------------------------------------------------------
// -----
plScalarControllerCacheChannel::plScalarControllerCacheChannel(plScalarControllerChannel *controller,
                                                               plControllerCacheInfo *cache)
: fControllerChannel(controller),
  fCache(cache)
{
}

// dtor ---------------------------------------------------------
// -----
plScalarControllerCacheChannel::~plScalarControllerCacheChannel()
{
    delete fCache;
    fControllerChannel = nullptr;
}

// Value ---------------------------------------------------------------------
// ------
const float & plScalarControllerCacheChannel::Value(double time, bool peek)
{
    return fControllerChannel->Value(time, peek, fCache);
}

// Detach -----------------------------------------------------------------
// -------
plAGChannel * plScalarControllerCacheChannel::Detach(plAGChannel * channel)
{
    plAGChannel *result = this;

    if(channel == this)
    {
        result = nullptr;
    } else {
        fControllerChannel = plScalarControllerChannel::ConvertNoRef(fControllerChannel->Detach(channel));
        
        if(!fControllerChannel)
            result = nullptr;
    }
    if(result != this)
        delete this;

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLATCCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------
plATCChannel::plATCChannel()
: fConvert()
{
}

// ctor ----------------------------------------------
plATCChannel::plATCChannel(plAnimTimeConvert *convert)
: fConvert(convert)
{
}

// dtor ---------------------
plATCChannel::~plATCChannel()
{
}

// IsStoppedAt ------------------------------
// ------------
bool plATCChannel::IsStoppedAt(double time)
{
    return fConvert->IsStoppedAt(time);
}

// Value -----------------------------------------------------
// ------
const float & plATCChannel::Value(double time, bool peek)
{
    fResult = (peek ? fConvert->WorldToAnimTimeNoUpdate(time) : fConvert->WorldToAnimTime(time));
    return fResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARSDLCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------------
// -----
plScalarSDLChannel::plScalarSDLChannel()
: fLength(1), fVar()
{
    fResult = 0;
}

plScalarSDLChannel::plScalarSDLChannel(float length)
: fLength(length), fVar()
{
    fResult = 0;
}

// dtor ---------------------------------
plScalarSDLChannel::~plScalarSDLChannel()
{
}

// IsStoppedAt ------------------------------------
// ------------
bool plScalarSDLChannel::IsStoppedAt(double time) 
{ 
    return false; 
}

// Value -----------------------------------------------------------
// ------
const float & plScalarSDLChannel::Value(double time, bool peek)
{
    if (fVar)
        fVar->Get(&fResult);

    // the var will return a 0-1 value, scale to match our anim length.
    fResult *= fLength;
    return fResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// APPLICATORS
//
/////////////////////////////////////////////////////////////////////////////////////////

// IApply ------------------------------------------------------------------
// -------
void plScalarChannelApplicator::IApply(const plAGModifier *mod, double time)
{
}

// IApply --------------------------------------------------------------
// -------
void plSpotInnerApplicator::IApply(const plAGModifier *mod, double time)
{
    plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
    hsAssert(scalarChan, "Invalid channel given to plSpotInnerApplicator");

    plSpotLightInfo *sli = plSpotLightInfo::ConvertNoRef(IGetGI(mod, plSpotLightInfo::Index()));

    const float &scalar = scalarChan->Value(time);
    sli->SetSpotInner(hsDegreesToRadians(scalar)*0.5f);
}

// IApply --------------------------------------------------------------
// -------
void plSpotOuterApplicator::IApply(const plAGModifier *mod, double time)
{
    plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
    hsAssert(scalarChan, "Invalid channel given to plSpotInnerApplicator");

    plSpotLightInfo *sli = plSpotLightInfo::ConvertNoRef(IGetGI(mod, plSpotLightInfo::Index()));

    const float &scalar = scalarChan->Value(time);
    sli->SetSpotOuter(hsDegreesToRadians(scalar)*0.5f);
}


void plOmniApplicator::IApply(const plAGModifier *modifier, double time)
{
    plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
    hsAssert(scalarChan, "Invalid channel given to plLightOmniApplicator");

    plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

    oli->SetLinearAttenuation(scalarChan->Value(time));
}

void plOmniSqApplicator::IApply(const plAGModifier *modifier, double time)
{
    plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
    hsAssert(scalarChan, "Invalid channel given to plLightOmniApplicator");

    plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

    oli->SetQuadraticAttenuation(scalarChan->Value(time));
}

void plOmniCutoffApplicator::IApply(const plAGModifier *modifier, double time)
{
    plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
    hsAssert(scalarChan, "Invalid channel given to plOmniCutoffApplicator");

    plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

    oli->SetCutoffAttenuation( scalarChan->Value( time ) );
}