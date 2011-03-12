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
#ifndef PL_NOTETRACK_ANIM
#define PL_NOTETRACK_ANIM

#include "plMaxAnimUtils.h"

// WIP wrapper class to make plMaxAnimUtils easier to use.
class plAnimInfo
{
protected:
	SegmentMap *fSegMap;
	SegmentSpec *fAnimSpec;
	SegmentMap::iterator fLoopIt;
	SegmentMap::iterator fMarkerIt;
	SegmentMap::iterator fStopPointIt;

public:
	plAnimInfo() : fSegMap(NULL), fAnimSpec(NULL) {}
	plAnimInfo(SegmentMap *segMap, const char *animName);

	const char *GetAnimName();
	float GetAnimStart();
	float GetAnimEnd();
	float GetAnimInitial();

	const char *GetNextLoopName();
	float GetLoopStart(const char *loopName);
	float GetLoopEnd(const char *loopName);

	const char *GetNextMarkerName();
	float GetMarkerTime(const char *markerName);

	float GetNextStopPoint();	// Returns -1 on last stop point
	bool IsSuppressed(const char *animName);
};

class plNotetrackAnim
{
protected:
	SegmentMap *fSegMap;
	SegmentMap::iterator fAnimIt;

	plNotetrackAnim();

public:
	plNotetrackAnim(Animatable *anim, plErrorMsg *pErrMsg);
	~plNotetrackAnim();

	bool HasNotetracks() { return (fSegMap != NULL); }

	const char *GetNextAnimName();
	plAnimInfo GetAnimInfo(const char *animName);
};

#endif //PL_NOTETRACK_ANIM