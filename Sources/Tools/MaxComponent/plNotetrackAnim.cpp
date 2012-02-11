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
#include "plNotetrackAnim.h"
#include "plComponentBase.h"
#include "MaxMain/plMaxNodeBase.h"

plNotetrackAnim::plNotetrackAnim() : fSegMap(nil)
{
}

plNotetrackAnim::plNotetrackAnim(Animatable *anim, plErrorMsg *pErrMsg)
{
    if (anim->SuperClassID() == HELPER_CLASS_ID &&
        ((Object*)anim)->CanConvertToType(COMPONENT_CLASSID))
    {
        plComponentBase *comp = (plComponentBase*)anim;
        std::vector<Animatable*> targs;

        for (int i = 0; i < comp->NumTargets(); i++)
        {
            plMaxNodeBase *node = comp->GetTarget(i);
            if (node)
                targs.push_back(node);
        }

        fSegMap = GetSharedAnimSegmentMap(targs, pErrMsg);
    }
    else
    {
        fSegMap = GetAnimSegmentMap(anim, pErrMsg);
    }

    if (fSegMap)
        fAnimIt = fSegMap->begin();
}

plNotetrackAnim::~plNotetrackAnim()
{
    DeleteSegmentMap(fSegMap);
}

plString plNotetrackAnim::GetNextAnimName()
{
    if (!fSegMap)
        return plString::Null;

    while (fAnimIt != fSegMap->end())
    {
        SegmentSpec *spec = fAnimIt->second;
        fAnimIt++;

        if (spec->fType == SegmentSpec::kAnim)
            return spec->fName;
    }

    fAnimIt = fSegMap->begin();
    return plString::Null;
}

plAnimInfo plNotetrackAnim::GetAnimInfo(const plString &animName)
{
    if (!fSegMap)
        return plAnimInfo();

    if (animName.IsEmpty() || fSegMap->find(animName) == fSegMap->end())
        return plAnimInfo(fSegMap, plString::Null);
    else 
        return plAnimInfo(fSegMap, animName);

    return plAnimInfo();
}

////////////////////////////////////////////////////////////////////////////////

plAnimInfo::plAnimInfo(SegmentMap *segMap, const plString &animName)
{
    fSegMap = segMap;
    fAnimSpec = !animName.IsNull() ? (*fSegMap)[animName] : nil;

    if (fSegMap)
    {
        fLoopIt = fSegMap->begin();
        fMarkerIt = fSegMap->begin();
        fStopPointIt = fSegMap->begin();
    }
}

plString plAnimInfo::GetAnimName()
{
    return fAnimSpec ? fAnimSpec->fName : plString::Null;
}

float plAnimInfo::GetAnimStart()
{
    return fAnimSpec ? fAnimSpec->fStart : -1;
}

float plAnimInfo::GetAnimEnd()
{
    return fAnimSpec ? fAnimSpec->fEnd : -1;
}

float plAnimInfo::GetAnimInitial()
{
    return fAnimSpec ? fAnimSpec->fInitial : -1;
}

plString plAnimInfo::GetNextLoopName()
{
    if (!fSegMap)
        return plString::Null;

    while (fLoopIt != fSegMap->end())
    {
        SegmentSpec *spec = fLoopIt->second;
        fLoopIt++;

        if (spec->fType == SegmentSpec::kLoop &&
            (!fAnimSpec || fAnimSpec->Contains(spec)))
            return spec->fName;
    }

    fLoopIt = fSegMap->begin();
    return plString::Null;
}

float plAnimInfo::GetLoopStart(const plString &loopName)
{
    if (!fSegMap || loopName.IsNull())
        return -1;

    if (fSegMap->find(loopName) != fSegMap->end())
    {
        SegmentSpec *spec = (*fSegMap)[loopName];
        if (spec->fStart == -1)
            return -1;//GetAnimStart();
        else
            return spec->fStart;
    }

    return -1;
}

float plAnimInfo::GetLoopEnd(const plString &loopName)
{
    if (!fSegMap || loopName.IsNull())
        return -1;

    if (fSegMap->find(loopName) != fSegMap->end())
    {
        SegmentSpec *spec = (*fSegMap)[loopName];
        if (spec->fEnd == -1)
            return -1;//GetAnimEnd();
        else
            return spec->fEnd;
    }

    return -1;
}

plString plAnimInfo::GetNextMarkerName()
{
    if (!fSegMap)
        return plString::Null;

    while (fMarkerIt != fSegMap->end())
    {
        SegmentSpec *spec = fMarkerIt->second;
        fMarkerIt++;

        if (spec->fType == SegmentSpec::kMarker &&
            (!fAnimSpec || fAnimSpec->Contains(spec)))
            return spec->fName;
    }

    fMarkerIt = fSegMap->begin();
    return plString::Null;
}

float plAnimInfo::GetMarkerTime(const plString &markerName)
{
    if (!fSegMap)
        return -1;

    if (fSegMap->find(markerName) != fSegMap->end())
    {
        SegmentSpec *spec = (*fSegMap)[markerName];
        return spec->fStart;
    }

    return -1;
}

float plAnimInfo::GetNextStopPoint()
{
    if (!fSegMap)
        return -1;

    while (fStopPointIt != fSegMap->end())
    {
        SegmentSpec *spec = fStopPointIt->second;
        fStopPointIt++;

        if (spec->fType == SegmentSpec::kStopPoint &&
            (!fAnimSpec || fAnimSpec->Contains(spec)))
            return spec->fStart;
    }

    fStopPointIt = fSegMap->begin();
    return -1;
}

bool plAnimInfo::IsSuppressed(const plString &animName)
{
    if (!fSegMap || animName.IsNull())
        return false;

    if (fSegMap->find(animName) != fSegMap->end())
    {
        SegmentSpec *spec = (*fSegMap)[animName];
        if (spec->fType == SegmentSpec::kSuppress)
            return true;
    }
    
    return false;
}
