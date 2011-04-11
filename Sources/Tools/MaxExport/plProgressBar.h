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
// plProgressBar.h
#ifndef plProgressBar_inc
#define plProgressBar_inc

#include "hsTypes.h"
#include "hsScalar.h"

// The progress bar displays an amount that's *fraction* of the distance between min and max.
// i.e., if min is 0.2 and max is 0.7, the bar will run the gamut from min to max; at fraction
// 0.8, it will display 0.6 ( = 0.2 + 0.8 * (0.7 - 0.2)).

// The bar runs from 0 to 1 and all values should be in that range.
class plProgressBar
{
public:
	plProgressBar(hsScalar min, hsScalar max) : fMin(min), fMax(max) 
	{ 
		  hsAssert(min >= 0, "Min too small.");
		  hsAssert(min <= 1, "Min too big.");
		  hsAssert(max >= 0, "Max too small.");
		  hsAssert(max <= 1, "Max too big.");
		  hsAssert(min <= max, "Min and max out of order.");
	}

	virtual hsBool32 Update(hsScalar fraction) = 0;

	hsScalar GetTotalFraction(hsScalar f) const { return fMin + f * (fMax - fMin); }

private:
	hsScalar fMin;
	hsScalar fMax;
};

#endif // plProgressBar_inc