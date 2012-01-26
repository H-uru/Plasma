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
#include "plAGApplicator.h"
#include "plAGModifier.h"
#include "hsResMgr.h"


// ctor --------
// -----
plAGApplicator::plAGApplicator()
: fChannel(nil),
  fChannelName(nil),
  fEnabled(true)
{
};

// ctor -------------------------------
// -----
plAGApplicator::plAGApplicator(const char *channelName)
: fChannel(nil),
  fEnabled(true)
{
    fChannelName = hsStrcpy(channelName);
};

plAGApplicator::~plAGApplicator()
{
    if(fChannelName)
        delete[] fChannelName;
}

void plAGApplicator::Apply(const plAGModifier *mod, double time, hsBool force)
{
    if (fEnabled || force)
        IApply(mod, time);
}

void plAGApplicator::SetChannelName(const char *name)
{
    if(name)
        fChannelName = hsStrcpy(name);
};


const char * plAGApplicator::GetChannelName()
{
    return fChannelName;
};

plAGChannel *plAGApplicator::MergeChannel(plAGApplicator *app, plAGChannel *channel, 
                                          plScalarChannel *blend, int blendPriority)    
{
    plAGChannel *result = nil;
    if(fChannel)
    {
        if (CanCombine(app))
            result = fChannel->MakeCombine(channel);
        else if (CanBlend(app))
            result = fChannel->MakeBlend(channel, blend, blendPriority);
    } else {
        result = channel;
    }

    if (result && result != fChannel)
        SetChannel(result);

    return result;
}

plAGApplicator *plAGApplicator::CloneWithChannel(plAGChannel *channel)
{
    plAGApplicator *app = plAGApplicator::ConvertNoRef(plFactory::Create(ClassIndex()));
    app->SetChannel(channel);
    app->Enable(fEnabled);
    app->SetChannelName(fChannelName);
    return app;
}

hsBool plAGApplicator::CanBlend(plAGApplicator *app)
{
    uint16_t ourClass = ClassIndex();
    uint16_t theirClass = app->ClassIndex();

    return(ourClass == theirClass);

//  return(this->HasBaseClass(theirClass)
//         || app->HasBaseClass(ourClass));
}


void plAGApplicator::Write(hsStream *stream, hsResMgr *mgr)
{
    plCreatable::Write(stream, mgr);

    stream->WriteBool(fEnabled);
    stream->WriteSafeString(fChannelName);
}

void plAGApplicator::Read(hsStream *stream, hsResMgr *mgr)
{
    plCreatable::Read(stream, mgr);

    fEnabled = stream->ReadBool();
    fChannel = nil; // Whatever is reading this applicator in should know what channel to assign it
    fChannelName = stream->ReadSafeString();
}

// IGETxI
// Gain access to of our modifier's target's interfaces.
// This is technically in violation of the principle that only modifiers can get non-const
// reference to their target's interfaces,
// BUT since the plAGApplicator architecture is wholly "owned" by the AGModifier, this
// seemed the most graceful way to do it without const_cast or modifying plModifier or plSceneObject

// IGETAI
plAudioInterface * plAGApplicator::IGetAI(const plAGModifier *modifier) const
{
    return modifier->LeakAI();
}

// IGETCI
plCoordinateInterface * plAGApplicator::IGetCI(const plAGModifier* modifier) const
{
    return modifier->LeakCI();
}

// IGETDI
plDrawInterface * plAGApplicator::IGetDI(const plAGModifier * modifier) const
{
    return modifier->LeakDI();
}

// IGETSI
plSimulationInterface * plAGApplicator::IGetSI(const plAGModifier * modifier) const
{
    return modifier->LeakSI();
}

plObjInterface * plAGApplicator::IGetGI(const plAGModifier * modifier, uint16_t classIdx) const
{
    return modifier->LeakGI(classIdx);
}
