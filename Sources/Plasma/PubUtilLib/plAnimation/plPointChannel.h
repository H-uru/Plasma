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
#ifndef PLPOINTCHANNEL_H
#define PLPOINTCHANNEL_H

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include "plAGApplicator.h"
#include "plAGChannel.h"

#include "hsGeometry3.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plController;
class plControllerCacheInfo;


/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

// PLPOINTCHANNEL
/////////////////
// A source of animated hsPoint3 data
class plPointChannel : public plAGChannel
{
protected:
    hsPoint3 fResult;

public:
    plPointChannel();
    virtual ~plPointChannel();

    // AG PROTOCOL
    virtual const hsPoint3 & Value(double time);
    virtual void Value(hsPoint3 &point, double time);

    // combine it (allocates combine object)
    plAGChannel * MakeCombine(plAGChannel * channelB) override;

    // blend it (allocates blend object)
    plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority) override;

    // const eval at time zero
    plAGChannel * MakeZeroState() override;
    // make a timeScale instance
    plAGChannel * MakeTimeScale(plScalarChannel *timeSource) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plPointChannel );
    GETINTERFACE_ANY( plPointChannel, plAGChannel );
};

//////////////////
// PLPOINTCONSTANT
//////////////////
// A point source that just keeps handing out the same point
class plPointConstant : public plPointChannel
{
public:
    plPointConstant();
    plPointConstant(const hsPoint3 &point);
    virtual ~plPointConstant();

    void Set(const hsPoint3 &the_Point) { fResult = the_Point; }

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plPointConstant );
    GETINTERFACE_ANY( plPointConstant, plPointChannel );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

////////////////////
// PLPOINTTIMESCALE
////////////////////
// Adapts the time scale before passing it to the next channel in line.
// Use to instance animations while allowing each instance to run at different speeds.
class plPointTimeScale : public plPointChannel
{
protected:
    plScalarChannel *fTimeSource;
    plPointChannel *fChannelIn;

public:
    plPointTimeScale();
    plPointTimeScale(plPointChannel *channel, plScalarChannel *timeSource);
    virtual ~plPointTimeScale();

    bool IsStoppedAt(double time) override;
    const hsPoint3 & Value(double time) override;

    plAGChannel * Detach(plAGChannel * channel) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plPointTimeScale );
    GETINTERFACE_ANY( plPointTimeScale, plPointChannel );
};

// PLPOINTBLEND
///////////////
// Combines two point sources into one
class plPointBlend : public plPointChannel
{
public:
    plPointBlend();
    plPointBlend(plPointChannel *channelA, plPointChannel *channelB, plScalarChannel *channelBias);
    virtual ~plPointBlend();

    plAGChannel * Remove(plAGChannel *srceToRemove);

    const plPointChannel * GetPointChannelA() const { return fPointA; }
    void SetPointChannelA(plPointChannel *the_PointA) { fPointA = the_PointA; }

    const plPointChannel * GetPointChannelB() const { return fPointB; }
    void SetPointChannelB(plPointChannel *the_PointB) { fPointB = the_PointB; }

    const plScalarChannel * GetChannelBias() const { return fChannelBias; }
    void SetChannelBias(plScalarChannel * channel) { fChannelBias = channel; }

    //float GetBlend() const { return fBlend; }
    //void SetBlend(float the_blend) { fBlend = the_blend; }
    bool IsStoppedAt(double time) override;

    // AG PROTOCOL
    const hsPoint3 &Value(double time) override;

    // remove the specified channel from our graph
    plAGChannel * Detach(plAGChannel * channel) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plPointBlend );
    GETINTERFACE_ANY( plPointBlend, plPointChannel );

protected:
    plPointChannel *fPointA;
    plPointChannel *fPointB;
    plScalarChannel *fChannelBias;
};

// BLENDPOINTS
// Handy little function to share with others.
hsPoint3 BlendPoints(hsPoint3 &pointA, hsPoint3 &pointB, float blend);


///////////////////////////
// PLPOINTCONTROLLERCHANNEL
///////////////////////////

// converts a plController-style animation into a plPointChannel
class plPointControllerChannel : public plPointChannel
{
protected:
    plController *fController;
    
public:
    // xTORs
    plPointControllerChannel();
    plPointControllerChannel(plController *controller);
    virtual ~plPointControllerChannel();
    
    // AG PROTOCOL
    const hsPoint3 & Value(double time) override;
    virtual const hsPoint3 & Value(double time, plControllerCacheInfo *cache);
    
    plAGChannel * MakeCacheChannel(plAnimTimeConvert *atc) override;
    
    // PLASMA PROTOCOL
    // rtti
    CLASSNAME_REGISTER( plPointControllerChannel );
    GETINTERFACE_ANY( plPointControllerChannel, plPointChannel );
    
    // persistence
    void Write(hsStream *stream, hsResMgr *mgr) override;
    void Read(hsStream *s, hsResMgr *mgr) override;
};

////////////////////////////////
// PLPOINTCONTROLLERCACHECHANNEL
////////////////////////////////
// Same as plPointController, but with caching info
class plPointControllerCacheChannel : public plPointChannel
{
protected:
    plControllerCacheInfo *fCache;
    plPointControllerChannel *fControllerChannel;
    
public:
    plPointControllerCacheChannel();
    plPointControllerCacheChannel(plPointControllerChannel *channel, plControllerCacheInfo *cache);
    virtual ~plPointControllerCacheChannel();
    
    virtual const hsPoint3 & Value(double time, bool peek = false);

    plAGChannel * Detach(plAGChannel * channel) override;
    
    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plPointControllerCacheChannel );
    GETINTERFACE_ANY( plPointControllerCacheChannel, plPointChannel );
    
    // Created at runtime only, so no Read/Write
};

////////////////////////////
// 
// Channel Applicator classes

class plPointChannelApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plPointChannelApplicator );
    GETINTERFACE_ANY( plPointChannelApplicator, plAGApplicator );
};

class plLightDiffuseApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plLightDiffuseApplicator );
    GETINTERFACE_ANY( plLightDiffuseApplicator, plAGApplicator );
};

class plLightAmbientApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plLightAmbientApplicator );
    GETINTERFACE_ANY( plLightAmbientApplicator, plAGApplicator );
};

class plLightSpecularApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plLightSpecularApplicator );
    GETINTERFACE_ANY( plLightSpecularApplicator, plAGApplicator );
};




#endif // PLPOINTCHANNEL_H
