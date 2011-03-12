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

// other
#include "../plMessage/plAnimCmdMsg.h"

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

// ctor ------------
// -----
plAGAnim::plAGAnim()
: plSynchedObject()
{
	fName = nil;
}

// ctor ------------------------------------------------------
// -----
plAGAnim::plAGAnim(const char *name, double start, double end)
: fStart((hsScalar)start),
  fEnd((hsScalar)end)
{
	if (name == nil)
		name = "";

	fName = TRACKED_NEW char[strlen(name) + 1];
	strcpy(fName, name);
}

// dtor -------------
// -----
plAGAnim::~plAGAnim()
{
	if (fName)
	{
		RemoveAnim(fName);
		delete[] fName;
		fName = nil;
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
	return (app ? app->GetChannel() : nil);
}

// GetChannel --------------------------------------------
// -----------
plAGChannel * plAGAnim::GetChannel(const char *name) const
{
	int appCount = fApps.size();

	for(int i = 0; i < appCount; i++)
	{
		plAGApplicator *app = fApps[i];
		plAGChannel *channel = app->GetChannel();
		const char *channelName = app->GetChannelName();

		if(stricmp(name, channelName) == 0)
		{
			return channel;
		}
	}
	return nil;
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
hsBool plAGAnim::RemoveApplicator(int index)
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
void plAGAnim::ExtendToLength(hsScalar length)
{
	if (length > GetEnd())
		SetEnd(length);
}

// GetChannelName ------------------------------
// ---------------
const char * plAGAnim::GetChannelName(int index)
{
	hsAssert(index < fApps.size(), "Out of range index for plAGAnim::GetChannelName()");

	if(index < fApps.size())
	{
		return fApps[index]->GetChannel()->GetName();
	} else {
		return nil;
	}
}

// Read --------------------------------------------
// -----
void plAGAnim::Read(hsStream *stream, hsResMgr *mgr)
{
	plSynchedObject::Read(stream, mgr);

	// read in the name of the animation itself
	fName = stream->ReadSafeString();

	fStart = stream->ReadSwapScalar();
	fEnd = stream->ReadSwapScalar();

	int numApps = stream->ReadSwap32();

	fApps.reserve(numApps);				// pre-allocate for performance
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

	stream->WriteSwapScalar(fStart);
	stream->WriteSwapScalar(fEnd);

	int numApps = fApps.size();

	stream->WriteSwap32(numApps);

	int i;
	for (i = 0; i < numApps; i++)
	{
		plAGApplicator *app = fApps[i];
		hsAssert(app, "Missing applicator during write.");
		plAGChannel *channel = nil;
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
void plAGAnim::AddAnim(const char * name, plAGAnim *anim)
{
	// Only register the animation if it's got a "real" name. Unnamed animations
	// all get the same standard name.
	if(strcmp(name, ENTIRE_ANIMATION_NAME) != 0)
	{
		hsAssert(anim, "registering nil anim");
		fAllAnims[name] = anim;
	}
}

// FindAnim -----------------------------------
// ---------
plAGAnim * plAGAnim::FindAnim(const char *name)
{
	plAnimMap::iterator i = fAllAnims.find(name);

	if(i != fAllAnims.end())
	{
		return (*i).second;
	} else {
		return nil;
	}
}

// RemoveAnim -------------------------------
// -----------
hsBool plAGAnim::RemoveAnim(const char *name)
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
		const char *name = anim->GetName();
		hsStatusMessageF("GLOBAL ANIMS [%d]: <%s>", j++, name);
	} while(++i != fAllAnims.end());
}

// SharesPinsWith -----------------------------------------
// ---------------
hsBool plAGAnim::SharesPinsWith(const plAGAnim *anim) const
{
	int i, j;
	for (i = 0; i < fApps.size(); i++)
	{
		for (j = 0; j < anim->fApps.size(); j++)
		{
			if (!strcmp(fApps[i]->GetChannelName(), anim->fApps[j]->GetChannelName()) &&
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
: plAGAnim()
{
}

// ctor --------------------------------------------------------
// -----
plATCAnim::plATCAnim(const char *name, double start, double end)
: plAGAnim(name, start, end),
  fInitial(-1),
  fAutoStart(true),
  fLoopStart((hsScalar)start),
  fLoopEnd((hsScalar)end),
  fLoop(false),
  fEaseInType(plAnimEaseTypes::kNoEase),
  fEaseOutType(plAnimEaseTypes::kNoEase),
  fEaseInLength(0),
  fEaseOutLength(0),
  fEaseInMin(0.f),
  fEaseInMax(0.f),
  fEaseOutMin(0.f),
  fEaseOutMax(0.f)
{
}

// dtor ---------------
// -----
plATCAnim::~plATCAnim()
{
	for (MarkerMap::iterator it = fMarkers.begin(); it != fMarkers.end(); it++)
		delete [] (char*)it->first;
	fMarkers.clear();
	for( LoopMap::iterator it2 = fLoops.begin(); it2 != fLoops.end(); it2++ )
		delete [] (char *)it2->first;
	fLoops.clear();
	fStopPoints.clear();
}

// Read ---------------------------------------------
// -----
void plATCAnim::Read(hsStream *stream, hsResMgr *mgr)
{
	plAGAnim::Read(stream, mgr);

	fInitial = stream->ReadSwapScalar();
	fAutoStart = stream->Readbool();
	fLoopStart = stream->ReadSwapScalar();
	fLoopEnd = stream->ReadSwapScalar();
	fLoop = stream->Readbool();

	fEaseInType = stream->ReadByte();
	fEaseInMin = stream->ReadSwapScalar();
	fEaseInMax = stream->ReadSwapScalar();
	fEaseInLength = stream->ReadSwapScalar();
	fEaseOutType = stream->ReadByte();
	fEaseOutMin = stream->ReadSwapScalar();
	fEaseOutMax = stream->ReadSwapScalar();
	fEaseOutLength = stream->ReadSwapScalar();

	int i;
	int numMarkers = stream->ReadSwap32();
	for (i = 0; i < numMarkers; i++)
	{
		char *name = stream->ReadSafeString();
		float time = stream->ReadSwapFloat();
		fMarkers[name] = time;
	}

	int numLoops = stream->ReadSwap32();
	for (i = 0; i < numLoops; i++)
	{
		char *name = stream->ReadSafeString();
		float begin = stream->ReadSwapScalar();
		float end = stream->ReadSwapScalar();
		fLoops[name] = std::pair<float,float>(begin,end);
	}

	int numStops = stream->ReadSwap32();
	for (i = 0; i < numStops; i++)
		fStopPoints.push_back(stream->ReadSwapScalar());
}

// Write ---------------------------------------------
// ------
void plATCAnim::Write(hsStream *stream, hsResMgr *mgr)
{
	plAGAnim::Write(stream, mgr);

	stream->WriteSwapScalar(fInitial);
	stream->Writebool(fAutoStart);
	stream->WriteSwapScalar(fLoopStart);
	stream->WriteSwapScalar(fLoopEnd);
	stream->Writebool(fLoop);

	stream->WriteByte(fEaseInType);
	stream->WriteSwapScalar(fEaseInMin);
	stream->WriteSwapScalar(fEaseInMax);
	stream->WriteSwapScalar(fEaseInLength);
	stream->WriteByte(fEaseOutType);
	stream->WriteSwapScalar(fEaseOutMin);
	stream->WriteSwapScalar(fEaseOutMax);
	stream->WriteSwapScalar(fEaseOutLength);

	stream->WriteSwap32(fMarkers.size());
	for (MarkerMap::iterator it = fMarkers.begin(); it != fMarkers.end(); it++)
	{
		stream->WriteSafeString(it->first);
		stream->WriteSwapFloat(it->second);
	}

	stream->WriteSwap32(fLoops.size());
	for (LoopMap::iterator loopIt = fLoops.begin(); loopIt != fLoops.end(); loopIt++)
	{
		stream->WriteSafeString(loopIt->first);
		std::pair<float,float>& loop = loopIt->second;
		stream->WriteSwapFloat(loop.first);
		stream->WriteSwapFloat(loop.second);
	}

	int i;
	stream->WriteSwap32(fStopPoints.size());
	for (i = 0; i < fStopPoints.size(); i++)
		stream->WriteSwapScalar(fStopPoints[i]);
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
void plATCAnim::AddLoop(const char *name, float start, float end)
{
	char *nameCpy = hsStrcpy(name);
	fLoops[nameCpy] = std::pair<float,float>(start, end);
}

// GetLoop --------------------------------------------------------------
// --------
bool plATCAnim::GetLoop(const char *name, float &start, float &end) const
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
bool plATCAnim::GetLoop(UInt32 num, float &start, float &end) const
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
UInt32 plATCAnim::GetNumLoops() const
{
	return fLoops.size();
}

// AddMarker ------------------------------------------
// ----------
void plATCAnim::AddMarker(const char *name, float time)
{
	char *nameCpy = hsStrcpy(name);
	fMarkers[nameCpy] = time;
}

// GetMarker -------------------------------------
// ----------
float plATCAnim::GetMarker(const char *name) const
{
	if (fMarkers.find(name) != fMarkers.end())
		return (*fMarkers.find(name)).second;
	return -1;
}

// CopyMarkerNames -------------------------------------
// ----------------
void plATCAnim::CopyMarkerNames(std::vector<char*> &out)
{
	MarkerMap::iterator it = fMarkers.begin();

	for (; it != fMarkers.end(); it++)
	{
		out.push_back(hsStrcpy((*it).first));
	}
}

// AddStopPoint ---------------------------
// -------------
void plATCAnim::AddStopPoint(hsScalar time)
{
	fStopPoints.push_back(time);
}

// NumStopPoints ----------------
// --------------
UInt32 plATCAnim::NumStopPoints()
{
	return fStopPoints.size();
}

// GetStopPoint --------------------------
// -------------
hsScalar plATCAnim::GetStopPoint(UInt32 i)
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
: fBodyUsage(kBodyFull)
{
}

// ctor ------------------------------------------------------------------------------
// -----
plEmoteAnim::plEmoteAnim(const char *animName, double begin, double end, float fadeIn,
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
	fFadeIn = stream->ReadSwapScalar();
	fFadeOut = stream->ReadSwapScalar();
	fBodyUsage = static_cast<BodyUsage>(stream->ReadByte());

}

// Write -----------------------------------------------
// ------
void plEmoteAnim::Write(hsStream *stream, hsResMgr *mgr)
{
	plATCAnim::Write(stream, mgr);
	stream->WriteSwapScalar(fFadeIn);
	stream->WriteSwapScalar(fFadeOut);
	stream->WriteByte(static_cast<UInt8>(fBodyUsage));
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
	fGlobalVarName = nil;
}

// ctor --------------------------------------------------------------------
// -----
plAgeGlobalAnim::plAgeGlobalAnim(const char *name, double start, double end)
: plAGAnim(name, start, end),
  fGlobalVarName(nil)
{
}

// dtor ---------------------------
// -----
plAgeGlobalAnim::~plAgeGlobalAnim()
{
	delete [] fGlobalVarName;
}

void plAgeGlobalAnim::SetGlobalVarName(char *name) 
{ 
	delete [] fGlobalVarName; 
	fGlobalVarName = hsStrcpy(name); 
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
							hsMatrix44 *endToStart, const char *channelName)
{
	double start = 0.0f;	// assumed
	double end = anim->GetEnd();

	GetRelativeTransform(anim, start, end, startToEnd, endToStart, channelName);
	return true;
}

// GetRelativeTransform ---------------------------------------------------
bool GetRelativeTransform(const plAGAnim *anim, double timeA, double timeB,
						  hsMatrix44 *a2b, hsMatrix44 *b2a, const char *channelName)
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

			if(a2b)					// requested a transform from point A to point B
			{
				hsMatrix44 invA;
				matA.GetInverse(&invA);
				*a2b = invA * matB;
			}
			if(b2a)					// requested a transform from point B to point A
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