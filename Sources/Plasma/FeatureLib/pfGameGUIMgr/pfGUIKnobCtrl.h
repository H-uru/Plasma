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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfGUIKnobCtrl Header                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIKnobCtrl_h
#define _pfGUIKnobCtrl_h

#include "hsGeometry3.h"

#include <string_theory/string>

#include "pfGUIValueCtrl.h"

class plAGMasterMod;
class plKey;
class plMessage;

class pfGUIKnobCtrl : public pfGUIValueCtrl
{
    protected:

        std::vector<plKey> fAnimationKeys;
        ST::string      fAnimName;

        hsPoint3        fDragStart;
        float           fDragValue;
        bool            fDragging;

        hsPoint3        fAnimStartPos, fAnimEndPos; // Calculated at export time for kMapToScreenRange
        float           fDragRangeMin, fDragRangeMax;

                        // Computed once, once an anim is loaded that we can compute this with
        float           fAnimBegin, fAnimEnd;
        bool            fAnimTimesCalced;

        bool IEval(double secs, float del, uint32_t dirty) override; // called only by owner object's Eval()

        uint32_t      IGetDesiredCursor() const override;    // As specified in plInputInterface.h

        bool            ICalcAnimTimes();

    public:

        pfGUIKnobCtrl();

        CLASSNAME_REGISTER( pfGUIKnobCtrl );
        GETINTERFACE_ANY( pfGUIKnobCtrl, pfGUIValueCtrl );


        enum OurFlags
        {
            kReverseValues = kDerivedFlagsStart,
            kLeftRightOrientation,
            kMapToScreenRange,
            kTriggerOnlyOnMouseUp,
            kMapToAnimationRange
        };

        bool    MsgReceive(plMessage* pMsg) override;
        
        void Read(hsStream* s, hsResMgr* mgr) override;
        void Write(hsStream* s, hsResMgr* mgr) override;

        void    HandleMouseDown(hsPoint3 &mousePt, uint8_t modifiers) override;
        void    HandleMouseUp(hsPoint3 &mousePt, uint8_t modifiers) override;
        void    HandleMouseDrag(hsPoint3 &mousePt, uint8_t modifiers) override;

        void    UpdateBounds(hsMatrix44 *invXformMatrix = nullptr, bool force = false) override;

        void    SetCurrValue(float v) override;

        // Export only
        void    SetAnimationKeys(const std::vector<plKey> &keys, const ST::string &name);
        void    SetScreenRange( const hsPoint3 &startPos, const hsPoint3 &endPos ) { fAnimStartPos = startPos; fAnimEndPos = endPos; }
};

#endif // _pfGUIKnobCtrl_h
