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

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "plMaxAnimUtils.h"
#include "MaxExport/plErrorMsg.h"

float TimeValueToGameTime(TimeValue t)
{
    int FR = ::GetFrameRate();
    int TPF = ::GetTicksPerFrame();
    int TPS = TPF*FR;

    return float(t)/float(TPS);
}

bool GetSegMapAnimTime(const ST::string &animName, SegmentMap *segMap, SegmentSpec::SegType type, float& begin, float& end)
{
    if (segMap)
    {
        if (!animName.empty() && segMap->find(animName) != segMap->end())
        {
            SegmentSpec *spec = (*segMap)[animName];
            if (spec->fType == type)
            {
                if (spec->fStart != -1)
                    begin = spec->fStart;
                if (spec->fEnd != -1)
                    end = spec->fEnd;
                return true;
            }
        }
    }

    return false;
}

SegmentMap *GetSharedAnimSegmentMap(std::vector<Animatable*>& anims, plErrorMsg *pErrorMsg)
{
    if (anims.empty())
        return nullptr;

    SegmentMap *segMap = GetAnimSegmentMap(anims[0], pErrorMsg);
    if (!segMap)
        return nullptr;

    int i;
    for (i = 1; i < anims.size(); i++)
    {
        SegmentMap *curSegMap = GetAnimSegmentMap(anims[i], pErrorMsg);
        // This node doesn't have a segmap, so we can't have any anims shared among all the nodes.
        if (!curSegMap)
        {
            DeleteSegmentMap(segMap);
            return nullptr;
        }

        if (segMap->begin() == segMap->end())
        {
            DeleteSegmentMap(segMap);
            return nullptr;
        }

        SegmentMap::iterator it = segMap->begin();
        while (it != segMap->end())
        {
            if (curSegMap->find(it->second->fName) == curSegMap->end())
            {
                SegmentMap::iterator del = it;
                it++;
                segMap->erase(del->second->fName);
            }
            else
                it++;
        }

        DeleteSegmentMap(curSegMap);
    }

    return segMap;
}

SegmentSpec::SegmentSpec(float start, float end, const ST::string & name, SegType type) :
    fStart(start), fEnd(end), fName(name), fType(type), fInitial(-1)
{
}

SegmentSpec::~SegmentSpec()
{
}

// constants used for parsing the note tracks

enum NoteType
{
    kNoteStartAnim,
    kNoteEndAnim,
    kNoteStartLoop,
    kNoteEndLoop,
    kNoteMarker,
    kNoteStopPoint,
    kNoteInitial,
    kNoteUnknown,
    kNoteSuppress,
};

SegmentSpec::SegmentSpec()
{
    fStart = -1;
    fEnd = -1;
    fInitial = -1;
    fType = kAnim;
}

bool SegmentSpec::Contains(SegmentSpec *spec)
{
    if (!spec)
        return false;

    if (spec->fType == kMarker || spec->fType == kStopPoint)
        return (spec->fStart >= fStart && spec->fStart <= fEnd);

    if (fStart == -1 || fEnd == -1)
        return false;

    if (spec->fStart == -1)
        return (fStart < spec->fEnd);

    if (spec->fEnd == -1)
        return (fEnd > spec->fStart);

    if (fStart <= spec->fStart && fEnd >= spec->fEnd)
        return true;

    return false;
}

bool DoesHaveStopPoints(Animatable *anim)
{
    if (!anim || !anim->HasNoteTracks())
        return false;

    int numTracks = anim->NumNoteTracks();
    for (int i = 0; i < numTracks; i++)
    {
        DefNoteTrack *track = (DefNoteTrack *)anim->GetNoteTrack(i);
        int numKeys = track->keys.Count();

        for (int j = 0; j < numKeys; j++)
        {
            char buf[256];
            strcpy(buf, track->keys[j]->note);
            strlwr(buf);
            if (strstr(buf, "@stoppoint"))
                return true;
        }
    }

    return false;
}

void GetSegment(const char *note, float time, SegmentMap *segMap, plErrorMsg *pErrMsg)
{
    char segName[256];
    char segSuffix[256];

    int matchedFields = sscanf(note, " %[^@] @ %s ", segName, segSuffix);

    ST::string name = segName;
    ST::string suffix = segSuffix;

    if (matchedFields == 2)
    {
        NoteType type = kNoteUnknown;

        if (!stricmp(segSuffix, "start") ||
            !stricmp(segSuffix, "begin"))
            type = kNoteStartAnim;
        else if (!stricmp(segSuffix, "end"))
            type = kNoteEndAnim;
        else if (!stricmp(segSuffix, "startloop") ||
                !stricmp(segSuffix, "loopstart") ||
                !stricmp(segSuffix, "beginloop") ||
                !stricmp(segSuffix, "loopbegin"))
            type = kNoteStartLoop;
        else if (!stricmp(segSuffix, "endloop") ||
                !stricmp(segSuffix, "loopend"))
            type = kNoteEndLoop;
        else if (!stricmp(segSuffix, "marker"))
            type = kNoteMarker;
        else if (!stricmp(segSuffix, "stoppoint"))
            type = kNoteStopPoint;
        else if (!stricmp(segSuffix, "initial"))
            type = kNoteInitial;
        else if (!stricmp(segSuffix, "suppress"))
            type = kNoteSuppress;

        if (type == kNoteUnknown)
        {
            if (pErrMsg)
            {
                pErrMsg->Set(true, "NoteTrack Anim Error", "Malformed segment note: %s", segName);
                pErrMsg->Show();
                pErrMsg->Set();
            }
        }
        else
        {
            SegmentMap::iterator existing = segMap->find(name);
            SegmentSpec *existingSpec = (existing != segMap->end()) ? (*existing).second : nullptr;
            const char *kErrorTitle = "NoteTrack Anim Error";

            if (existingSpec)
            {
                // an existing spec, but we're processing a start note?
                if (type == kNoteStartAnim && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Got out of order start note.  No Start given for %s", segName).Show();
                    pErrMsg->Set();
                }
                // existing spec, has an end, we're also processing an end?
                else if (type == kNoteEndAnim && existingSpec->fEnd != -1 && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Got two ends for the same segment %s", segName).Show();
                    pErrMsg->Set();
                }
                else if (type == kNoteStartLoop && existingSpec->fStart != -1 && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Got two loop starts for the same segment, %s", segName).Show();
                    pErrMsg->Set();
                }
                else if (type == kNoteEndLoop && existingSpec->fEnd != -1 && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Got two loop ends for the same segment, %s", segName).Show();
                    pErrMsg->Set();
                }
                else if (type == kNoteMarker && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Marker has the same name (%s) as another spec in its notetrack", segName).Show();
                    pErrMsg->Set();
                }
                else if (type == kNoteStopPoint && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Stop point has the same name (%s) as another spec in its notetrack", segName).Show();
                    pErrMsg->Set();
                }

                if (type == kNoteEndAnim || type == kNoteEndLoop)
                    existingSpec->fEnd = time;
                else if (type == kNoteStartLoop)
                    existingSpec->fStart = time;
                else if (type == kNoteInitial)
                    existingSpec->fInitial = time;
            }
            else
            {
                if (type == kNoteEndAnim && pErrMsg)
                {
                    pErrMsg->Set(true, kErrorTitle, "Got an end note without a corresponding start. Ignoring %s", segName).Show();
                    pErrMsg->Set();
                }
                else
                {
                    switch (type)
                    {
                    case kNoteStartAnim:
                        (*segMap)[name] = new SegmentSpec(time, -1, name, SegmentSpec::kAnim);
                        break;

                    case kNoteStartLoop:
                        (*segMap)[name] = new SegmentSpec(time, -1, name, SegmentSpec::kLoop);
                        break;

                    case kNoteEndLoop:
                        (*segMap)[name] = new SegmentSpec(-1, time, name, SegmentSpec::kLoop);
                        break;

                    case kNoteMarker:
                        (*segMap)[name] = new SegmentSpec(time, -1, name, SegmentSpec::kMarker);
                        break;

                    case kNoteStopPoint:
                        (*segMap)[name] = new SegmentSpec(time, -1, name, SegmentSpec::kStopPoint);
                        break;

                    case kNoteSuppress:
                        (*segMap)[name] = new SegmentSpec(-1, -1, name, SegmentSpec::kSuppress);
                        break;
                        
                    default:
                        break;
                    }
                }
            }
        }
    }
}

// Read through all the notes in all the note tracks on the given node
// Check the contents of each node for a name like "walk@start", i.e. <string>@[start | end]
// For each match, open a segment specification and p
SegmentMap * GetAnimSegmentMap(Animatable *anim, plErrorMsg *pErrMsg)
{
    if (!anim->HasNoteTracks())
        return nullptr;
    
    SegmentMap *segMap = new SegmentMap();

    int numTracks = anim->NumNoteTracks();

    for (int i = 0; i < numTracks; i++)
    {
        DefNoteTrack * track = (DefNoteTrack *)anim->GetNoteTrack(i);
        int numKeys = track->keys.Count();

        for (int j = 0; j < numKeys; j++)
        {
            auto note = track->keys[j]->note;
            float time = TimeValueToGameTime(track->keys[j]->time);
            GetSegment(note, time, segMap, pErrMsg);
        }
    }

    return segMap;
}

void DeleteSegmentMap(SegmentMap *segMap)
{
    // If we have a segment map, delete the memory associated with it
    if (segMap)
    {
        for (SegmentMap::iterator i = segMap->begin(); i != segMap->end(); i++)
            delete (*i).second;

        delete segMap;
    }
}
