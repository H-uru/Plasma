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
// local
#include "plAGModifier.h"
#include "plMatrixChannel.h"


// global
#include "hsResMgr.h"
#include "hsTimer.h"

// other
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSimulationInterface.h"

/////////////////
//
// PLAGMODIFIER
//
/////////////////
// Applies animation graph output to a single scene object.

// CTOR
plAGModifier::plAGModifier()
: plSingleModifier()
{
    fAutoApply = true;
    fEnabled = true;
}

// CTOR(name)
plAGModifier::plAGModifier(const ST::string &name, bool autoApply)
: plSingleModifier(), fAutoApply(autoApply)
{
    fChannelName = name;
    fEnabled = true;
}

// DTOR
plAGModifier::~plAGModifier()
{
    int i;
    for (i = 0; i < fApps.size(); i++)
    {
        delete fApps[i];
    }
}

// GETCHANNELNAME
ST::string plAGModifier::GetChannelName() const
{
    return fChannelName;
}

// ENABLE
void plAGModifier::Enable(bool val)
{
    fEnabled = val;
}

// SETCHANNELNAME
void plAGModifier::SetChannelName(const ST::string & name)
{
    fChannelName = name;
}

// IAPPLYCHANNELS (time)
// One AGModifier, although applied to a single scene object, can have
// multiple channels on it affecting, say, position, color, aroma, etc.
//
// There are cases where we want to call this and won't know the delta,
// we don't seem to ever need it for this function, so I'm taking it out.
// If you run into a case where you think it's necessary, see me. -Bob
void plAGModifier::Apply(double time) const
{
    if (!fEnabled)
        return;
    
    for (int i = 0; i < fApps.size(); i++)
    {
        plAGApplicator *app = fApps[i];
        
        app->Apply(this, time);
    }
}

// IEVAL
// Apply our channels to our scene object
bool plAGModifier::IEval(double time, float delta, uint32_t dirty)
{
    if(fAutoApply) {
    //  Apply(time, delta);
    }
    return true;
}

// GETAPPLICATOR
plAGApplicator * plAGModifier::GetApplicator(plAGPinType pinType) const
{
    int numApps = fApps.size();

    for (int i = 0; i < numApps; i++)
    {
        plAGApplicator *app = fApps[i];
        plAGPinType otherType = app->GetPinType();
        if(otherType == pinType)
            return app;
    }
    return nullptr;
}

// SETAPPLICATOR
void plAGModifier::SetApplicator(plAGApplicator *newApp)
{
    int numApps = fApps.size();
    plAGPinType newPinType = newApp->GetPinType();

    // *** NOTE: this code is completely untested. Since I happened to be here
    // I sketched out how it *should* work and implemented the base protocol.
    // In reality, most of these code paths are not accessed now...
    // -- mm
    for(int i = 0; i < numApps; i++)
    {
        plAGApplicator *existingApp = fApps[i];
        plAGPinType extPinType = existingApp->GetPinType();

        if(extPinType == newPinType)
        {
            hsStatusMessage("Two applicators accessing same pin type...congratulations for being the first to test this.");
            // these applicators both try to set the same thing; try to merge them

            plAGChannel *newChannel = newApp->GetChannel();

            hsAssert(!newChannel, "Trying to merge in new applicator which already has channel. Incomplete.");

            // *** right now I just want to support the case of putting in a new applicator - not merging animations

            plAGChannel *extChannel = existingApp->GetChannel();
            newApp->SetChannel(extChannel);
            existingApp->SetChannel(nullptr);
            fApps[i] = newApp;

            delete existingApp;
            return;

            // NOTE: we should make these arbitrate, but I'm not going to right now because
            // there's not currently an (easy) way to merge two applicators without allowing a blend.
//          if(existingApp->CanCombine(newApp))
//          {
//              // the existing applicator promises to provide the functionality we need...merge into it.
//              existingApp->MergeChannel(newApp);
//          } else {
//              // couldn't merge into the existing channel; can we merge it into us instead?
//              if(newApp->CanCombine(extApp))
//              {
//                  // okay, WE can provide the functionality of the existing applicator.
//                  fApps[i] = newApp;                      // take over its spot in the applicators
//                  newApp->MergeChannel(existingApp);      // and merge it into us
//              }
//          }
        }
    }
    // didn't find any conflicts; just add our app on the end
    fApps.push_back(newApp);
}

// MERGECHANNEL
// Intended as a replacement for attach/blend channel. You want to add a channel to this node,
// we do that for you. Don't ask us how, you shouldn't have to know.
plAGChannel * plAGModifier::MergeChannel(plAGApplicator *app,
                                         plAGChannel *channel,
                                         plScalarChannel *blend,
                                         plAGAnimInstance *anim,
                                         int priority)
{
    int numApps = fApps.size();
    plAGChannel * result = nullptr;

    for (int i = 0; i < numApps; i++)
    {
        plAGApplicator *existingApp = fApps[i];
        result = existingApp->MergeChannel(app, channel, blend, priority);
        if (result)
            return result;
    }

    if (!result)
    {
        // didn't blend or combine with an existing channel; add a new channel
        plAGApplicator *newApp = app->CloneWithChannel(channel);
        fApps.push_back(newApp);
    }
    return result;
}

// DETACHCHANNEL
bool plAGModifier::DetachChannel(plAGChannel * channel)
{
    plAppTable::iterator i = fApps.begin();

    while( i != fApps.end() )
    {
        plAGApplicator *app = *i;
        plAGChannel *existingChannel = app->GetChannel();
        if(existingChannel)
        {
            plAGChannel *replacementChannel = existingChannel->Detach(channel);

            if (existingChannel != replacementChannel)
            {
                app->SetChannel(replacementChannel);
                if( ! replacementChannel && app->AutoDelete())
                {
                    // Don't need to adjust the iterator since we're about to exit the loop
                    fApps.erase(i);
                    delete app;
                }
                return true;
            }
        }
        ++i;
    }
    return false;
}

// READ
void plAGModifier::Read(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Read(stream, mgr);

    // read in the name of the modifier
    fChannelName = stream->ReadSafeString();
}

// WRITE
void plAGModifier::Write(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Write(stream, mgr);

    // write out the name of the modifier
    stream->WriteSafeString(fChannelName);
}

/////////////////////////////////////////
/////////////////////////////////////////
//
// MOVE
//
/////////////////////////////////////////
/////////////////////////////////////////

const plModifier * FindModifierByClass(const plSceneObject *obj, int classID)
{
    if(obj)
    {
        size_t modCount = obj->GetNumModifiers();

        for (size_t i = 0; i < modCount; i++)
        {
            const plModifier *mod = obj->GetModifier(i);

            if(mod)     // modifier might not be loaded yet
            {
                if(mod->ClassIndex() == classID)
                {
                    return mod;
                }
            }
        }
    }
    return nullptr;
}

