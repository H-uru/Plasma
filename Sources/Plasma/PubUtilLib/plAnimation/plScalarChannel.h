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
#ifndef PLSCALARCHANNEL_INC
#define PLSCALARCHANNEL_INC

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
// base
#include "plAGChannel.h"
#include "plAGApplicator.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////
class plController;
class plAnimTimeConvert;
class plSimpleStateVariable;
class plControllerCacheInfo;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

//////////////////
// PLSCALARCHANNEL
//////////////////
// an animation channel that outputs a scalar value
class plScalarChannel : public plAGChannel
{
protected:
    float fResult;

public:
    plScalarChannel() : plAGChannel(), fResult() { }

    // AG PROTOCOL
    virtual const float & Value(double time, bool peek = false);
    virtual void Value(float &result, double time, bool peek = false);

    // combine it (allocates combine object)
    plAGChannel * MakeCombine(plAGChannel * channelB) override;

    // blend it (allocates blend object)
    plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority) override;

    // const eval at time zero
    plAGChannel * MakeZeroState() override;
    
    // make a timeScale instance
    plAGChannel * MakeTimeScale(plScalarChannel *timeSource) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarChannel );
    GETINTERFACE_ANY( plScalarChannel, plAGChannel );
};

///////////////////
// PLSCALARCONSTANT
///////////////////
// A scalar source that just keeps handing out the same value
class plScalarConstant : public plScalarChannel
{
public:
    plScalarConstant();
    plScalarConstant(float value);
    virtual ~plScalarConstant();

    void Set(float value) { fResult = value; }
    float Get() { return fResult; }

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarConstant );
    GETINTERFACE_ANY( plScalarConstant, plScalarChannel );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};


////////////////////
// PLSCALARTIMESCALE
////////////////////
// Adapts the time scale before passing it to the next channel in line.
// Use to instance animations while allowing each instance to run at different speeds.
class plScalarTimeScale : public plScalarChannel
{
protected:
    plScalarChannel *fTimeSource;
    plScalarChannel *fChannelIn;

public:
    plScalarTimeScale();
    plScalarTimeScale(plScalarChannel *channel, plScalarChannel *timeSource);
    virtual ~plScalarTimeScale();

    bool IsStoppedAt(double time) override;
    const float & Value(double time, bool peek = false) override;
    plAGChannel * Detach(plAGChannel * channel) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarTimeScale );
    GETINTERFACE_ANY( plScalarTimeScale, plScalarChannel );
};

////////////////
// PLSCALARBLEND
////////////////
// blends two scalars into one with weighting
class plScalarBlend : public plScalarChannel
{
protected:
    plScalarChannel * fChannelA;
    plScalarChannel * fChannelB;
    plScalarChannel * fChannelBias;
    
public:
    // xTORs
    plScalarBlend();
    plScalarBlend(plScalarChannel * channelA, plScalarChannel * channelB, plScalarChannel * channelBias);
    virtual ~plScalarBlend();

    // SPECIFICS
    const plScalarChannel * GetChannelA() const { return fChannelA; }
    void SetChannelA(plScalarChannel * channel) { fChannelA = channel; }

    const plScalarChannel * GetChannelB() const { return fChannelB; }
    void SetChannelB(plScalarChannel * channel) { fChannelB = channel; }

    const plScalarChannel * GetChannelBias() const { return fChannelBias; }
    void SetChannelBias(plScalarChannel * channel) { fChannelBias = channel; }

    bool IsStoppedAt(double time) override;

    // AG PROTOCOL
    const float & Value(double time, bool peek = false) override;
    
    // remove the specified channel from our graph
    plAGChannel * Detach(plAGChannel * channel) override;
    
    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarBlend );
    GETINTERFACE_ANY( plScalarBlend, plScalarChannel );
};

////////////////////////////
// PLSCALARCONTROLLERCHANNEL
////////////////////////////
// converts a plController-style animation into a plScalarChannel
class plScalarControllerChannel : public plScalarChannel
{
protected:
    plController *fController;
    
public:
    // xTORs
    plScalarControllerChannel();
    plScalarControllerChannel(plController *controller);
    virtual ~plScalarControllerChannel();
    
    // AG PROTOCOL
    const float & Value(double time, bool peek = false) override;
    virtual const float & Value(double time, bool peek, plControllerCacheInfo *cache);
    
    plAGChannel *MakeCacheChannel(plAnimTimeConvert *atc) override;
        
    // PLASMA PROTOCOL
    // rtti
    CLASSNAME_REGISTER( plScalarControllerChannel );
    GETINTERFACE_ANY( plScalarControllerChannel, plScalarChannel );
    
    // persistence
    void Write(hsStream *stream, hsResMgr *mgr) override;
    void Read(hsStream *s, hsResMgr *mgr) override;
};

/////////////////////////////////
// PLSCALARCONTROLLERCACHECHANNEL
/////////////////////////////////
// Same as plScalarController, but with caching info
class plScalarControllerCacheChannel : public plScalarChannel
{
protected:
    plControllerCacheInfo *fCache;
    plScalarControllerChannel *fControllerChannel;
    
public:
    plScalarControllerCacheChannel();
    plScalarControllerCacheChannel(plScalarControllerChannel *channel, plControllerCacheInfo *cache);
    virtual ~plScalarControllerCacheChannel();
    
    const float & Value(double time, bool peek = false) override;
    
    plAGChannel * Detach(plAGChannel * channel) override;
    
    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarControllerCacheChannel );
    GETINTERFACE_ANY( plScalarControllerCacheChannel, plScalarChannel );
    
    // Created at runtime only, so no Read/Write
};

////////////////////
// PLATCChannel
////////////////////
// Channel interface for a plAnimTimeConvert object
class plATCChannel : public plScalarChannel
{
protected:
    plAnimTimeConvert *fConvert;

public:
    plATCChannel();
    plATCChannel(plAnimTimeConvert *convert);
    virtual ~plATCChannel();

    bool IsStoppedAt(double time) override;
    const float & Value(double time, bool peek = false) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plATCChannel );
    GETINTERFACE_ANY( plATCChannel, plScalarChannel );
};

////////////////////
// PLSCALARSDLCHANNEL
////////////////////
// Returns the value of an SDL scalar variable
class plScalarSDLChannel : public plScalarChannel
{
protected:
    plSimpleStateVariable *fVar;
    float fLength;

public:
    plScalarSDLChannel();
    plScalarSDLChannel(float length);
    virtual ~plScalarSDLChannel();

    bool IsStoppedAt(double time) override;
    const float & Value(double time, bool peek = false) override;

    void SetVar(plSimpleStateVariable *var) { fVar = var; }

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plScalarSDLChannel );
    GETINTERFACE_ANY( plScalarSDLChannel, plScalarChannel );
};


////////////////////////////
// 
// Channel Applicator classes

class plScalarChannelApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plScalarChannelApplicator );
    GETINTERFACE_ANY( plScalarChannelApplicator, plAGApplicator );
};

class plSpotInnerApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plSpotInnerApplicator );
    GETINTERFACE_ANY( plSpotInnerApplicator, plAGApplicator );
};

class plSpotOuterApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plSpotOuterApplicator );
    GETINTERFACE_ANY( plSpotOuterApplicator, plAGApplicator );
};

class plOmniApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plOmniApplicator );
    GETINTERFACE_ANY( plOmniApplicator, plAGApplicator );
};

class plOmniSqApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plOmniSqApplicator );
    GETINTERFACE_ANY( plOmniSqApplicator, plAGApplicator );
};

class plOmniCutoffApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plOmniCutoffApplicator );
    GETINTERFACE_ANY( plOmniCutoffApplicator, plAGApplicator );
};


#endif
