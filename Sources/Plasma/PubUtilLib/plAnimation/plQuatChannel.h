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
#ifndef PLQUATCHANNEL_INC
#define PLQUATCHANNEL_INC

#include "hsQuat.h"

#include "plAGChannel.h"
#include "plAGApplicator.h"

// PLQUATCHANNEL
/////////////
// A source of animated hsQuat data
class plQuatChannel : public plAGChannel
{
protected:
    hsQuat fResult;

public:
    plQuatChannel();
    virtual ~plQuatChannel();

    // AG PROTOCOL
    virtual const hsQuat & Value(double time);
    virtual void Value(hsQuat &quaternion, double time);

    // can this channel combine with the given channel?
    virtual bool CanCombine(plAGChannel * channelB);
    // combine it (allocates combine object)
    plAGChannel * MakeCombine(plAGChannel * channelB) override;

    // const eval at time zero
    plAGChannel * MakeZeroState() override;
    // make a timeScale instance
    plAGChannel * MakeTimeScale(plScalarChannel *timeSource) override;

    // blend it (allocates blend object)
    plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plQuatChannel );
    GETINTERFACE_ANY( plQuatChannel, plAGChannel );
};

// PLQUATCONSTANT
////////////
// A quat channel that just keeps handing out the same quaternion
class plQuatConstant : public plQuatChannel
{
public:
    plQuatConstant();
    plQuatConstant(const hsQuat &quaternion);
    virtual ~plQuatConstant();

    void Set(const hsQuat &the_Quat) { fResult = the_Quat; }

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plQuatConstant );
    GETINTERFACE_ANY( plQuatConstant, plQuatChannel );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

////////////////////
// PLQUATTIMESCALE
////////////////////
// Adapts the time scale before passing it to the next channel in line.
// Use to instance animations while allowing each instance to run at different speeds.
class plQuatTimeScale : public plQuatChannel
{
protected:
    plScalarChannel *fTimeSource;
    plQuatChannel *fChannelIn;

public:
    plQuatTimeScale();
    plQuatTimeScale(plQuatChannel *channel, plScalarChannel *timeSource);
    virtual ~plQuatTimeScale();

    bool IsStoppedAt(double time) override;
    const hsQuat & Value(double time) override;

    plAGChannel * Detach(plAGChannel * channel) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plQuatTimeScale );
    GETINTERFACE_ANY( plQuatTimeScale, plQuatChannel );
};

// PLQUATBLEND
//////////////
// Combines two quaternion sources into one
class plQuatBlend : public plQuatChannel
{
public:
    plQuatBlend();
    plQuatBlend(plQuatChannel *channelA, plQuatChannel *channelB, plScalarChannel *channelBias);
    virtual ~plQuatBlend();

    // GETTERS AND SETTERS FOR CHANNELS
    const plQuatChannel * GetQuatA() const { return fQuatA; }
    void SetQuatA(plQuatChannel *the_QuatA) { fQuatA = the_QuatA; }

    const plQuatChannel * GetQuatB() const { return fQuatB; }
    void SetQuatB(plQuatChannel *the_QuatB) { fQuatB = the_QuatB; }

    const plScalarChannel * GetChannelBias() const { return fChannelBias; }
    void SetChannelBias(plScalarChannel *channel) { fChannelBias = channel; }

    //float GetBlend() const { return fBlend; }
    //void SetBlend(float the_blend) { fBlend = the_blend; }
    bool IsStoppedAt(double time) override;

    // AG PROTOCOL
    const hsQuat &Value(double time) override;

    // remove the specified channel from our graph
    plAGChannel * Detach(plAGChannel * channel) override;

    // PLASMA PROTOCOL
    CLASSNAME_REGISTER( plQuatBlend );
    GETINTERFACE_ANY( plQuatBlend, plQuatChannel );

protected:
    plQuatChannel *fQuatA;
    plQuatChannel *fQuatB;
    plScalarChannel *fChannelBias;

};


////////////////////////////
// 
// Channel Applicator classes

class plQuatChannelApplicator : public plAGApplicator
{
protected:
    void IApply(const plAGModifier *mod, double time) override;

public:
    CLASSNAME_REGISTER( plQuatChannelApplicator );
    GETINTERFACE_ANY( plQuatChannelApplicator, plAGApplicator );
};


#endif // PLQUATCHANNEL_INC
