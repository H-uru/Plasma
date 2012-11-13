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
#ifndef plAGApplicator_h
#define plAGApplicator_h

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plAudioInterface;
class plCoordinateInterface;
class plDrawInterface;
class plSimulationInterface;
class plObjInterface;
class plAGModifier;

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
#include "pnFactory/plCreatable.h"
#include "plAvDefs.h"
#include "plString.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
class plAGChannel;
class plScalarChannel;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////
/** \class plAGApplicator
    Takes the end of a channel tree and applies it to a scene object.
    A transform applicator takes a matrix-typed-tree and applies it to
    the transform of the scene object.
    Other applicators might take floats and use them to animate alpha channels,
    etc.  */
class plAGApplicator : public plCreatable
{
public:
    // -- methods --
    /** Base constructor. */
    plAGApplicator();
    plAGApplicator(const plString &channelName);
    virtual ~plAGApplicator();

    /** Return our single input channel. Applicators only ever
        have a single input channel; you can always use blend
        or combine nodes to merge channels before routing
        them into the applicator. */
    plAGChannel *GetChannel() { return fChannel; }

    /** Set our input channel. Does not free the previous input channel. */
    void SetChannel(plAGChannel *channel) { fChannel = channel; }

    void SetChannelName(const plString &name);
    plString GetChannelName();

    /** Optionally suppress the action of this applicator.
        The applicator can still be forced to apply using the force
        paramater of the Apply function. */
    void Enable(bool on) { fEnabled = on; }

    /** Make a shallow copy of the applicator. Keep the same input channel
        but do not clone the input channel. */
    virtual plAGApplicator *CloneWithChannel(plAGChannel *channel);

    /** What animation type do we have? Used to determine whether two
        applicators are trying to animate the same thing or whether they
        can coexist peacefully. */
    virtual plAGPinType GetPinType() { return kAGPinUnknown; }
    
    // Join the incoming channel (if possible) to ours

    
    /** Determine whether the given applicator can be blended to ours. If so, this
        would be effected by pulling the input channel from the other applicator,
        blending it with our input channel via a new blend node, attaching that blend
        node as our new input, and throwing the other applicator away. */
    virtual bool CanBlend(plAGApplicator *app);
    /** Combine the two applicators if possible. \sa CanBlend */
    virtual plAGChannel * MergeChannel(plAGApplicator *app, plAGChannel *channel,
                                       plScalarChannel *blend, int blendPriority);
    
    /** \bug It makes no sense for an applicator to combine because combination always
             results in a different data type, which would require a different applicator. */
    virtual bool CanCombine(plAGApplicator *app) { return false; }

    /** Apply our channel's data to the scene object, via the modifier.
        This is the only function that actually changes perceivable scene state. */
    void Apply(const plAGModifier *mod, double time, bool force = false); // Apply our channel's data to the modifier
    
    // this is pretty much a HACK to support applicators that want to stick around when
    // their channel is gone so they can operate on the next channel that comes in
    // the RIGHT way to do this is to make applicators support the Detach() protocol just 
    // like channels...
    virtual bool AutoDelete() { return true; } // should we remove it when its input channel is gone?

    // PlOP
    CLASSNAME_REGISTER( plAGApplicator );
    GETINTERFACE_ANY( plAGApplicator, plCreatable );

    virtual void Write(hsStream *stream, hsResMgr *mgr);
    virtual void Read(hsStream *s, hsResMgr *mgr);

protected:
    // -- methods --
    virtual void IApply(const plAGModifier *mod, double time) = 0;

    // give derived classes access to the object interfaces
    plAudioInterface * IGetAI(const plAGModifier *modifier) const;
    plCoordinateInterface * IGetCI(const plAGModifier *modifier) const;
    plDrawInterface * IGetDI(const plAGModifier *modifier) const;
    plSimulationInterface * IGetSI(const plAGModifier *modifier) const;
    plObjInterface * IGetGI(const plAGModifier *modifier, uint16_t classIdx) const;

    // Allow plAGModifier to declare IGet?? as friends
    friend class plAGModifier;

    // -- members --
    plAGChannel *fChannel;
    bool fEnabled;
    plString fChannelName;
};

#endif
