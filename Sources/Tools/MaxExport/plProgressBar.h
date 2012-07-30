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
// plProgressBar.h
#ifndef plProgressBar_inc
#define plProgressBar_inc

#include "HeadSpin.h"


// The progress bar displays an amount that's *fraction* of the distance between min and max.
// i.e., if min is 0.2 and max is 0.7, the bar will run the gamut from min to max; at fraction
// 0.8, it will display 0.6 ( = 0.2 + 0.8 * (0.7 - 0.2)).

// The bar runs from 0 to 1 and all values should be in that range.
class plProgressBar
{
public:
    plProgressBar(float min, float max) : fMin(min), fMax(max) 
    { 
          hsAssert(min >= 0, "Min too small.");
          hsAssert(min <= 1, "Min too big.");
          hsAssert(max >= 0, "Max too small.");
          hsAssert(max <= 1, "Max too big.");
          hsAssert(min <= max, "Min and max out of order.");
    }

    virtual bool32 Update(float fraction) = 0;

    float GetTotalFraction(float f) const { return fMin + f * (fMax - fMin); }

private:
    float fMin;
    float fMax;
};

#endif // plProgressBar_inc