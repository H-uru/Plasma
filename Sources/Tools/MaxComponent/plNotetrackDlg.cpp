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
#include "HeadSpin.h"
#include "plNoteTrackDlg.h"
#include "plNotetrackAnim.h"

#include "max.h"
#include "iparamb2.h"
#include "../plInterp/plAnimEaseTypes.h"

plNoteTrackDlg::plNoteTrackDlg() : fhAnim(NULL), fhLoop(NULL), fPB(nil), fAnimID(-1), fLoopID(-1), fSegMap(nil), fOwner(nil)
{
}

plNoteTrackDlg::~plNoteTrackDlg()
{
	DeleteCache();
}

void plNoteTrackDlg::Init(HWND hAnim, HWND hLoop, int animID, int loopID, IParamBlock2 *pb, Animatable *owner)
{
	fhAnim = hAnim;
	fhLoop = hLoop;
	fPB = pb;
	fAnimID = animID;
	fLoopID = loopID;
	fOwner = owner;
}

void plNoteTrackDlg::ICacheNoteTrack()
{
	DeleteCache();

	fSegMap = GetAnimSegmentMap(fOwner, nil);
}

void plNoteTrackDlg::DeleteCache()
{
	if (fSegMap)
	{
		DeleteSegmentMap(fSegMap);
		fSegMap = nil;
	}
}

void plNoteTrackDlg::Load()
{
	ICacheNoteTrack();

	ILoadAnims();
	ILoadLoops();
}

void plNoteTrackDlg::AnimChanged()
{
	const char *animName = IGetSel(fhAnim);
	fPB->SetValue(fAnimID, 0, (TCHAR*)animName);

	ILoadLoops();
}

void plNoteTrackDlg::LoopChanged()
{
	const char *loopName = IGetSel(fhLoop);
	fPB->SetValue(fLoopID, 0, (TCHAR*)loopName);
}

void plNoteTrackDlg::ILoadAnims()
{
	if(fAnimID < 0 || !fhAnim)
		return;

	ComboBox_ResetContent(fhAnim);

	// Add the default option
	int def = ComboBox_AddString(fhAnim, ENTIRE_ANIMATION_NAME);
	ComboBox_SetItemData(fhAnim, def, kDefault);
	ComboBox_SetCurSel(fhAnim, def);

	if (!fSegMap)
		return;

	const char *savedAnim = fPB->GetStr(fAnimID);
	if (!savedAnim)
		savedAnim = "";

	// Add the names of the animations
	for (SegmentMap::iterator it = fSegMap->begin(); it != fSegMap->end(); it++)
	{
		SegmentSpec *spec = it->second;
		if (spec->fType == SegmentSpec::kAnim)
		{
			int idx = ComboBox_AddString(fhAnim, spec->fName);
			ComboBox_SetItemData(fhAnim, idx, kName);

			// If this is the saved animation name, select it
			if (!strcmp(spec->fName, savedAnim))
				ComboBox_SetCurSel(fhAnim, idx);
		}
	}
}

void plNoteTrackDlg::ILoadLoops()
{
	if(fLoopID < 0 || !fhLoop)
		return;

	ComboBox_ResetContent(fhLoop);

	// Add the default option
	int def = ComboBox_AddString(fhLoop, ENTIRE_ANIMATION_NAME);
	ComboBox_SetItemData(fhLoop, def, kDefault);
	ComboBox_SetCurSel(fhLoop, def);

	if (fSegMap)
	{
		// Get the animation segment (or leave it nil if we're using the entire animation)
		SegmentSpec *animSpec = nil;
		const char *animName = fPB->GetStr(fAnimID);
		if (animName && *animName != '\0' && fSegMap->find(animName) != fSegMap->end())
			animSpec = (*fSegMap)[animName];

		// Get the saved loop name
		const char *loopName = fPB->GetStr(fLoopID);
		if (!loopName)
			loopName = "";

		for (SegmentMap::iterator i = fSegMap->begin(); i != fSegMap->end(); i++)
		{
			SegmentSpec *spec = i->second;

			if (spec->fType == SegmentSpec::kLoop)
			{
				// If the loop is contained by the animation, add it
				if (!animSpec || animSpec->Contains(spec))
				{
					// Add the name
					int idx = ComboBox_AddString(fhLoop, spec->fName);
					ComboBox_SetItemData(fhLoop, idx, kName);

					if (!strcmp(loopName, spec->fName))
						ComboBox_SetCurSel(fhLoop, idx);
				}
			}
		}
	}
}

const char *plNoteTrackDlg::IGetSel(HWND hCombo)
{
	int sel = ComboBox_GetCurSel(hCombo);
	if (sel != CB_ERR && ComboBox_GetItemData(hCombo, sel) == kName)
	{
		char buf[256];
		ComboBox_GetText(hCombo, buf, sizeof(buf));
		return (*fSegMap)[buf]->fName;
	}

	return "";
}
