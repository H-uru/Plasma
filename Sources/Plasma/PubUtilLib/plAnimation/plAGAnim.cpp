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
#include "plAGAnim.h"

// local
#include "plMatrixChannel.h"

// global
#include "hsResMgr.h"
#include "hsStream.h"

// other
#include "plInterp/plAnimEaseTypes.h"
#include "plMessage/plAnimCmdMsg.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// STATICS
//
/////////////////////////////////////////////////////////////////////////////////////////

plAGAnim::plAnimMap plAGAnim::fAllAnims;

/////////////////////////////////////////////////////////////////////////////////////////
//
// plAGAnim
//
/////////////////////////////////////////////////////////////////////////////////////////

// dtor -------------
// -----
plAGAnim::~plAGAnim()
{
    if (!fName.empty())
    {
        RemoveAnim(fName);
    }

    //int numChannels = fChannels.size();
    int numApps = fApps.size();

    for (int i = 0; i < numApps; i++)
    {
        plAGApplicator *app = fApps[i];
        if (app)
        {
            plAGChannel *channel = app->GetChannel();
            if(channel)
                delete channel;
            
            delete app;
        }
    }
}

// GetChannelCount ------------------
// ----------------
int plAGAnim::GetChannelCount() const
{
    return fApps.size();
}

// GetChannel -------------------------------------
// -----------
plAGChannel * plAGAnim::GetChannel(int index) const
{
    plAGApplicator *app = fApps[index];
    return (app ? app->GetChannel() : nullptr);
}

// GetChannel --------------------------------------------
// -----------
plAGChannel * plAGAnim::GetChannel(const ST::string &name) const
{
    int appCount = fApps.size();

    for(int i = 0; i < appCount; i++)
    {
        plAGApplicator *app = fApps[i];
        plAGChannel *channel = app->GetChannel();
        ST::string channelName = app->GetChannelName();

        if(name.compare(channelName, ST::case_insensitive) == 0)
        {
            return channel;
        }
    }
    return nullptr;
}

// GetApplicatorCount ------------------
// -------------------
int plAGAnim::GetApplicatorCount() const
{
    return fApps.size();
}

// GetApplicator -----------------------------------
// --------------
plAGApplicator *plAGAnim::GetApplicator(int i) const
{
    return fApps[i];
}

// AddApplicator -------------------------------
// --------------
int plAGAnim::AddApplicator(plAGApplicator *app)
{
    hsAssert(app->GetChannel(), "Adding an applicator with no channel");
    fApps.push_back(app);

    // return the index of the channel
    return(fApps.size() - 1);
}

// RemoveApplicator ------------------------
// -----------------
bool plAGAnim::RemoveApplicator(int index)
{
    hsAssert(index < fApps.size(), "Out of range index for plAGAnim::RemoveApp()");

    if(index < fApps.size())
    {
        fApps.erase(fApps.begin() + index);
        return true;
    } else {
        return false;
    }
}

// ExtendToLength ----------------------------
// ---------------
void plAGAnim::ExtendToLength(float length)
{
    if (length > GetEnd())
        SetEnd(length);
}

// GetChannelName ------------------------------
// ---------------
ST::string plAGAnim::GetChannelName(int index)
{
    hsAssert(index < fApps.size(), "Out of range index for plAGAnim::GetChannelName()");

    if(index < fApps.size())
    {
        return fApps[index]->GetChannel()->GetName();
    } else {
        return ST::string();
    }
}

// Read --------------------------------------------
// -----
void plAGAnim::Read(hsStream *stream, hsResMgr *mgr)
{
    plSynchedObject::Read(stream, mgr);

    // read in the name of the animation itself
    fName = stream->ReadSafeString();

    fStart = stream->ReadLEFloat();
    fEnd = stream->ReadLEFloat();

    int numApps = stream->ReadLE32();

    fApps.reserve(numApps);             // pre-allocate for performance
    int i;
    for (i = 0; i < numApps; i++)
    {
        plAGApplicator * app = plAGApplicator::ConvertNoRef(mgr->ReadCreatable(stream));
        app->SetChannel(plAGChannel::ConvertNoRef(mgr->ReadCreatable(stream)));
        fApps.push_back(app);
    }
    plAGAnim::AddAnim(fName, this);
}

// Write --------------------------------------------
// ------
void plAGAnim::Write(hsStream *stream, hsResMgr *mgr)
{
    plSynchedObject::Write(stream, mgr);

    stream->WriteSafeString(fName);

    stream->WriteLEFloat(fStart);
    stream->WriteLEFloat(fEnd);

    int numApps = fApps.size();

    stream->WriteLE32(numApps);

    int i;
    for (i = 0; i < numApps; i++)
    {
        plAGApplicator *app = fApps[i];
        hsAssert(app, "Missing applicator during write.");
        plAGChannel *channel = nullptr;
        if (app)
            channel = app->GetChannel();
    
        hsAssert(channel, "Missing channel during write.");
        mgr->WriteCreatable(stream, app);
        mgr->WriteCreatable(stream, channel);
    }
}

void plAGAnim::ClearAnimationRegistry()
{
    fAllAnims.clear();
}

// AddAnim ----------------------------------------------
// --------
void plAGAnim::AddAnim(const ST::string & name, plAGAnim *anim)
{
    // Only register the animation if it's got a "real" name. Unnamed animations
    // all get the same standard name.
    if(name.compare(ENTIRE_ANIMATION_NAME) != 0)
    {
        hsAssert(anim, "registering nil anim");
        fAllAnims[name] = anim;
    }
}

// FindAnim -----------------------------------
// ---------
plAGAnim * plAGAnim::FindAnim(const ST::string &name)
{
    plAnimMap::iterator i = fAllAnims.find(name);

    if(i != fAllAnims.end())
    {
        return (*i).second;
    } else {
        return nullptr;
    }
}

// RemoveAnim -------------------------------
// -----------
bool plAGAnim::RemoveAnim(const ST::string &name)
{
    plAnimMap::iterator i = fAllAnims.find(name);

    if(i != fAllAnims.end())
    {
        fAllAnims.erase(i);
        return true;
    } else {
        return false;
    }
}

// DumpAnimationRegistry -------------
// ----------------------
void plAGAnim::DumpAnimationRegistry()
{
    plAnimMap::iterator i = fAllAnims.begin();
    int j = 0;

    do {
        plAGAnim *anim = (*i).second;
        ST::string name = anim->GetName();
        hsStatusMessageF("GLOBAL ANIMS [%d]: <%s>", j++, name.c_str());
    } while(++i != fAllAnims.end());
}

// SharesPinsWith -----------------------------------------
// ---------------
bool plAGAnim::SharesPinsWith(const plAGAnim *anim) const
{
    int i, j;
    for (i = 0; i < fApps.size(); i++)
    {
        for (j = 0; j < anim->fApps.size(); j++)
        {
            if (!fApps[i]->GetChannelName().compare(anim->fApps[j]->GetChannelName()) &&
                fApps[i]->CanBlend(anim->fApps[j]))
            {
                return true;
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plATCAnim
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------
// -----
plATCAnim::plATCAnim()
    : plAGAnim(),
      fInitial(-1), fAutoStart(true),
      fLoopStart(), fLoopEnd(), fLoop(),
      fEaseInType(plAnimEaseTypes::kNoEase),
      fEaseOutType(plAnimEaseTypes::kNoEase),
      fEaseInLength(), fEaseOutLength(),
      fEaseInMin(), fEaseInMax(),
      fEaseOutMin(), fEaseOutMax()
{ }

// ctor --------------------------------------------------------
// -----
plATCAnim::plATCAnim(const ST::string &name, double start, double end)
    : plAGAnim(name, start, end),
      fInitial(-1), fAutoStart(true),
      fLoopStart((float)start), fLoopEnd((float)end), fLoop(),
      fEaseInType(plAnimEaseTypes::kNoEase),
      fEaseOutType(plAnimEaseTypes::kNoEase),
      fEaseInLength(), fEaseOutLength(),
      fEaseInMin(), fEaseInMax(),
      fEaseOutMin(), fEaseOutMax()
{ }

// dtor ---------------
// -----
plATCAnim::~plATCAnim()
{
    fMarkers.clear();
    fLoops.clear();
    fStopPoints.clear();
}

// Read ---------------------------------------------
// -----
void plATCAnim::Read(hsStream *stream, hsResMgr *mgr)
{
    plAGAnim::Read(stream, mgr);

    fInitial = stream->ReadLEFloat();
    fAutoStart = stream->ReadBool();
    fLoopStart = stream->ReadLEFloat();
    fLoopEnd = stream->ReadLEFloat();
    fLoop = stream->ReadBool();

    fEaseInType = stream->ReadByte();
    fEaseInMin = stream->ReadLEFloat();
    fEaseInMax = stream->ReadLEFloat();
    fEaseInLength = stream->ReadLEFloat();
    fEaseOutType = stream->ReadByte();
    fEaseOutMin = stream->ReadLEFloat();
    fEaseOutMax = stream->ReadLEFloat();
    fEaseOutLength = stream->ReadLEFloat();

    int i;
    int numMarkers = stream->ReadLE32();
    for (i = 0; i < numMarkers; i++)
    {
        ST::string name = stream->ReadSafeString();
        float time = stream->ReadLEFloat();
        fMarkers[name] = time;
    }

    int numLoops = stream->ReadLE32();
    for (i = 0; i < numLoops; i++)
    {
        ST::string name = stream->ReadSafeString();
        float begin = stream->ReadLEFloat();
        float end = stream->ReadLEFloat();
        fLoops[name] = std::pair<float,float>(begin,end);
    }

    uint32_t numStops = stream->ReadLE32();
    fStopPoints.resize(numStops);
    stream->ReadLEFloat(numStops, fStopPoints.data());
}

// Write ---------------------------------------------
// ------
void plATCAnim::Write(hsStream *stream, hsResMgr *mgr)
{
    plAGAnim::Write(stream, mgr);

    stream->WriteLEFloat(fInitial);
    stream->WriteBool(fAutoStart);
    stream->WriteLEFloat(fLoopStart);
    stream->WriteLEFloat(fLoopEnd);
    stream->WriteBool(fLoop);

    stream->WriteByte(fEaseInType);
    stream->WriteLEFloat(fEaseInMin);
    stream->WriteLEFloat(fEaseInMax);
    stream->WriteLEFloat(fEaseInLength);
    stream->WriteByte(fEaseOutType);
    stream->WriteLEFloat(fEaseOutMin);
    stream->WriteLEFloat(fEaseOutMax);
    stream->WriteLEFloat(fEaseOutLength);

    stream->WriteLE32((uint32_t)fMarkers.size());
    for (const auto& fMarker : fMarkers)
    {
        stream->WriteSafeString(fMarker.first);
        stream->WriteLEFloat(fMarker.second);
    }

    stream->WriteLE32((uint32_t)fLoops.size());
    for (const auto& loop : fLoops)
    {
        stream->WriteSafeString(fLoop.first);
        std::pair<float,float>& loop = fLoop.second;
        stream->WriteLEFloat(loop.first);
        stream->WriteLEFloat(loop.second);
    }

    stream->WriteLE32((uint32_t)fStopPoints.size());
    stream->WriteLEFloat(fStopPoints.size(), fStopPoints.data());
}

// CheckLoop --------------
// ----------
void plATCAnim::CheckLoop()
{
    if (fLoopStart == fLoopEnd)
    {
        fLoopStart = fStart;
        fLoopEnd = fEnd;
    }
}

// AddLoop ------------------------------------------------------
// --------
void plATCAnim::AddLoop(const ST::string &name, float start, float end)
{
    fLoops[name] = std::pair<float,float>(start, end);
}

// GetLoop --------------------------------------------------------------
// --------
bool plATCAnim::GetLoop(const ST::string &name, float &start, float &end) const
{
    LoopMap::const_iterator it = fLoops.find(name);
    if (it != fLoops.end())
    {
        const std::pair<float,float>& loop = (*it).second;
        start = loop.first;
        end = loop.second;
        return true;
    }

    return false;
}

// GetLoop --------------------------------------------------------
// --------
bool plATCAnim::GetLoop(uint32_t num, float &start, float &end) const
{
    if (num >= fLoops.size())
        return false;

    LoopMap::const_iterator it = fLoops.begin();

    while (num > 0)
    {
        it++;
        num--;
    }
    const std::pair<float,float>& loop = (*it).second;
    start = loop.first;
    end = loop.second;
    return true;
}

// GetNumLoops ----------------------
// ------------
uint32_t plATCAnim::GetNumLoops() const
{
    return fLoops.size();
}

// AddMarker ------------------------------------------
// ----------
void plATCAnim::AddMarker(const ST::string &name, float time)
{
    fMarkers[name] = time;
}

// GetMarker -------------------------------------
// ----------
float plATCAnim::GetMarker(const ST::string &name) const
{
    if (fMarkers.find(name) != fMarkers.end())
        return (*fMarkers.find(name)).second;
    return -1;
}

// CopyMarkerNames -------------------------------------
// ----------------
void plATCAnim::CopyMarkerNames(std::vector<ST::string> &out)
{
    MarkerMap::iterator it = fMarkers.begin();

    out.reserve(fMarkers.size());
    for (; it != fMarkers.end(); it++)
    {
        out.push_back((*it).first);
    }
}

// AddStopPoint ---------------------------
// -------------
void plATCAnim::AddStopPoint(float time)
{
    fStopPoints.push_back(time);
}

// NumStopPoints ----------------
// --------------
uint32_t plATCAnim::NumStopPoints()
{
    return fStopPoints.size();
}

// GetStopPoint --------------------------
// -------------
float plATCAnim::GetStopPoint(uint32_t i)
{
    hsAssert(i < fStopPoints.size(), "Invalid index for GetStopPoint");
    return fStopPoints[i];
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plEmoteAnim
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ------------------
// -----
plEmoteAnim::plEmoteAnim()
    : fBodyUsage(kBodyFull), fFadeIn(), fFadeOut()
{
}

// ctor ------------------------------------------------------------------------------
// -----
plEmoteAnim::plEmoteAnim(const ST::string &animName, double begin, double end, float fadeIn,
                         float fadeOut, BodyUsage bodyUsage)
: plATCAnim(animName, begin, end),
  fFadeIn(fadeIn),
  fFadeOut(fadeOut),
  fBodyUsage(bodyUsage)
{
}

// Read -----------------------------------------------
// -----
void plEmoteAnim::Read(hsStream *stream, hsResMgr *mgr)
{
    plATCAnim::Read(stream, mgr);

    // plAGAnim::RegisterEmote(fName, this);
    fFadeIn = stream->ReadLEFloat();
    fFadeOut = stream->ReadLEFloat();
    fBodyUsage = static_cast<BodyUsage>(stream->ReadByte());

}

// Write -----------------------------------------------
// ------
void plEmoteAnim::Write(hsStream *stream, hsResMgr *mgr)
{
    plATCAnim::Write(stream, mgr);
    stream->WriteLEFloat(fFadeIn);
    stream->WriteLEFloat(fFadeOut);
    stream->WriteByte(static_cast<uint8_t>(fBodyUsage));
}

// GetBodyUsage ----------------------------------------
// -------------
plEmoteAnim::BodyUsage plEmoteAnim::GetBodyUsage() const
{
    return fBodyUsage;
}

// GetFadeIn -----------------------
// ----------
float plEmoteAnim::GetFadeIn() const
{
    return fFadeIn;
}

// GetFadeOut -----------------------
// -----------
float plEmoteAnim::GetFadeOut() const
{
    return fFadeOut;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plAgeGlobalAnim
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------
// -----
plAgeGlobalAnim::plAgeGlobalAnim()
: plAGAnim()
{
}

// ctor --------------------------------------------------------------------
// -----
plAgeGlobalAnim::plAgeGlobalAnim(const ST::string &name, double start, double end)
: plAGAnim(name, start, end)
{
}


// Read ---------------------------------------------------
// -----
void plAgeGlobalAnim::Read(hsStream *stream, hsResMgr *mgr)
{
    plAGAnim::Read(stream, mgr);

    fGlobalVarName = stream->ReadSafeString();
}

// Write ---------------------------------------------------
// ------
void plAgeGlobalAnim::Write(hsStream *stream, hsResMgr *mgr)
{
    plAGAnim::Write(stream, mgr);

    stream->WriteSafeString(fGlobalVarName);
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// UTILITIES
//
/////////////////////////////////////////////////////////////////////////////////////////

// GetStartToEndTransform -----------------------------------------------
bool GetStartToEndTransform(const plAGAnim *anim, hsMatrix44 *startToEnd,
                            hsMatrix44 *endToStart, const ST::string &channelName)
{
    double start = 0.0f;    // assumed
    double end = anim->GetEnd();

    GetRelativeTransform(anim, start, end, startToEnd, endToStart, channelName);
    return true;
}

// GetRelativeTransform ---------------------------------------------------
bool GetRelativeTransform(const plAGAnim *anim, double timeA, double timeB,
                          hsMatrix44 *a2b, hsMatrix44 *b2a, const ST::string &channelName)
{
    bool result = false;
    plAGChannel *maybeChannel = anim->GetChannel(channelName);
    hsAssert(maybeChannel, "Couldn't find channel with given name.");
    if(maybeChannel)
    {
        plMatrixChannel *channel = plMatrixChannel::ConvertNoRef(maybeChannel);
        hsAssert(channel, "Found channel, but it's not a matrix channel.");
        
        if(channel)
        {
            hsMatrix44 matA;
            hsMatrix44 matB;

            channel->Value(matA, timeA);
            channel->Value(matB, timeB);

            if(a2b)                 // requested a transform from point A to point B
            {
                hsMatrix44 invA;
                matA.GetInverse(&invA);
                *a2b = invA * matB;
            }
            if(b2a)                 // requested a transform from point B to point A
            {
                hsMatrix44 invB;
                matB.GetInverse(&invB);
                *b2a = invB * matA;
                
                if(a2b)
                {
                    hsMatrix44 invB2;
                    a2b->GetInverse(&invB2);
                }
            }
        }
    }
    return result;
}
