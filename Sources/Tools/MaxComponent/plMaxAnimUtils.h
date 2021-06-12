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
#ifndef PLMAXANIMUTILS_H
#define PLMAXANIMUTILS_H

#include <map>
#include <vector>
#include <string_theory/string>

#include <tchar.h>

class plErrorMsg;
class Animatable;

// SEGMENTSPEC
// A simple helper class to hold information captured from the note track about animation segments
class SegmentSpec
{
public:
    enum SegType { kAnim, kLoop, kMarker, kStopPoint, kSuppress };

    float   fStart;     // beginning of the segment in game time
    float   fEnd;       // end of the segment in game time
    float   fInitial;   // initial position of the animation (-1 for the start)
    ST::string fName;   // name of the segment: controls lifespan of the name
    SegType fType;
    
    SegmentSpec();
    SegmentSpec(float start, float end, const ST::string & name, SegType);
    ~SegmentSpec();

    bool Contains(SegmentSpec *spec);
};


// a table mapping segment names to segment spec objects
typedef std::map<ST::string, SegmentSpec*, ST::less_i> SegmentMap;

// You can pass in nil for pErrMsg for silent operation
SegmentMap *GetAnimSegmentMap(Animatable *anim, plErrorMsg *pErrMsg);

void DeleteSegmentMap(SegmentMap *segMap);

SegmentMap *GetSharedAnimSegmentMap(std::vector<Animatable*>& anims, plErrorMsg *pErrorMsg);

bool GetSegMapAnimTime(const ST::string &animName, SegmentMap *segMap, SegmentSpec::SegType type, float& begin, float& end);

// For internal use
void GetSegment(const TCHAR* note, float time, SegmentMap *segMap, plErrorMsg *pErrMsg);

bool DoesHaveStopPoints(Animatable *anim);

#endif
