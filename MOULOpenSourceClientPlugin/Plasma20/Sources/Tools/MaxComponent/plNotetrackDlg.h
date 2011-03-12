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
#ifndef PLNOTETRACKDLG_INC
#define PLNOTETRACKDLG_INC

#include "hsTypes.h"
#include "hsWindows.h"	// For HWND
#include "plMaxAnimUtils.h"

class IParamBlock2;
class Animatable;

class plNoteTrackDlg
{
protected:
	// Derived class needs to set these
	HWND fhAnim;
	HWND fhLoop;
	IParamBlock2 *fPB;
	int fAnimID;
	int fLoopID;
	Animatable *fOwner;

	SegmentMap *fSegMap;

	enum
	{
		kDefault,
		kName,
	};

public:
	plNoteTrackDlg();
	virtual ~plNoteTrackDlg();

	void Init(HWND hAnim, HWND hLoop, int animID, int loopID, IParamBlock2 *pb, Animatable *owner);
	void Load();

	void AnimChanged();
	void LoopChanged();

	// This is done automatically in most cases, but if the dialog is being destroyed
	// but you're not deleting the plNoteTrackDlg, call this to free cached memory.
	void DeleteCache();

protected:
	virtual void ICacheNoteTrack();

	void ILoadAnims();
	void ILoadLoops();

	const char *IGetSel(HWND hCombo);
};


#endif // PLNOTETRACKDLG_INC