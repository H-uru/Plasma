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

#include "plNoteTrackDlg.h"
#include "plNotetrackAnim.h"

#include "plInterp/plAnimEaseTypes.h"

plNoteTrackDlg::plNoteTrackDlg() : fhAnim(), fhLoop(), fPB(), fAnimID(-1), fLoopID(-1), fSegMap(), fOwner()
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

    fSegMap = GetAnimSegmentMap(fOwner, nullptr);
}

void plNoteTrackDlg::DeleteCache()
{
    if (fSegMap)
    {
        DeleteSegmentMap(fSegMap);
        fSegMap = nullptr;
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

    const MCHAR* savedAnim = fPB->GetStr(fAnimID);
    if (!savedAnim)
        savedAnim = _M("");

    // Add the names of the animations
    for (SegmentMap::iterator it = fSegMap->begin(); it != fSegMap->end(); it++)
    {
        SegmentSpec *spec = it->second;
        if (spec->fType == SegmentSpec::kAnim)
        {
            int idx = ComboBox_AddString(fhAnim, spec->fName.c_str());
            ComboBox_SetItemData(fhAnim, idx, kName);

            // If this is the saved animation name, select it
            if (!spec->fName.compare(savedAnim))
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
        SegmentSpec *animSpec = nullptr;
        ST::string animName = ST::string(fPB->GetStr(fAnimID));
        if (!animName.empty() && fSegMap->find(animName) != fSegMap->end())
            animSpec = (*fSegMap)[animName];

        // Get the saved loop name
        const MCHAR* loopName = fPB->GetStr(fLoopID);
        if (!loopName)
            loopName = _M("");

        for (SegmentMap::iterator i = fSegMap->begin(); i != fSegMap->end(); i++)
        {
            SegmentSpec *spec = i->second;

            if (spec->fType == SegmentSpec::kLoop)
            {
                // If the loop is contained by the animation, add it
                if (!animSpec || animSpec->Contains(spec))
                {
                    // Add the name
                    int idx = ComboBox_AddString(fhLoop, spec->fName.c_str());
                    ComboBox_SetItemData(fhLoop, idx, kName);

                    if (!spec->fName.compare(loopName))
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
        TCHAR buf[256];
        ComboBox_GetText(hCombo, buf, std::size(buf));
        return (*fSegMap)[ST::string(buf)]->fName.c_str();
    }

    return "";
}
