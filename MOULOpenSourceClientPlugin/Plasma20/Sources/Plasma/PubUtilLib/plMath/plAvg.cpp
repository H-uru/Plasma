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

#include "hsTypes.h"
#include "plAvg.h"
#include <math.h>

template class TimeBasedAvgRing<float>;
template class TimeBasedAvgRing<double>;

#define PercisionRoundUp(x) (ceil(x / kPercision) * kPercision)

template <class T>
const float TimeBasedAvgRing<T>::kPercision = 0.001;

template <class T>
void TimeBasedAvgRing<T>::AddItem(T value, double time)
{
	hsTempMutexLock lock( fLock );

	if ( fList.empty() )
	{
		// initialize with the first time and zero the first value
		fList.insert(fList.end(),Item<T>(0.0,time));
		fRingStart = fRingEnd = fList.begin();
		fAvg = (float)value;
	}
	else
	{
		// if we're within the percision amount subtract the RingEnd value from total
		// and update the RingEnd value by adding the current value to it
		if (time - (*fRingEnd).GetTime() <= kPercision)
		{
			fTotal -= PercisionRoundUp((*fRingEnd).GetValue());
			(*fRingEnd).SetValue((*fRingEnd).GetValue() + value);
		}
		else
		{
			// clean up the begining of the ring
			//// there can be some precision loss in the loop time calc
			//// check to see if the difference is within 1 milli
			while (time - (*fRingStart).GetTime() > fLen + kPercision
				&& fRingStart != fRingEnd)
			{
				// remove RingStart from the avg part of the average calc
				fTotal -= (*fRingStart).GetValue();

				TimeList::iterator prev = fRingStart++;

				// loop the ring if needed
				if (fRingStart == fList.end())
					fRingStart = fList.begin();


				// if the new ring start is in the range, interpolate
				//   and reuse prev
				if (time - (*fRingStart).GetTime() < fLen)
				{
					// remove RingStart from the avg part of the average calc
					fTotal -= PercisionRoundUp((*fRingStart).GetValue());

					// Set up the interp
					double remainder = fLen - (time - (*fRingStart).GetTime());
					double timedelta = (*fRingStart).GetTime() - (*prev).GetTime();
					(*prev).SetTime((*fRingStart).GetTime() - remainder);
					(*prev).SetValue(0);
					// rounding loss occurs here if T is not floting point
					double scale = remainder/timedelta;
					hsAssert(scale < 1.0 && scale > 0.0,"Interp Scale Out of Bounds");
					(*fRingStart).SetValue((float)((*fRingStart).GetValue() * scale));
					
					// add the new  value in
					fTotal += (*fRingStart).GetValue();
					
					// put prev back as ring start
					fRingStart = prev;
				}

			}

			// zero total & fAvg if we looped or neg
			if (fRingStart == fRingEnd || fTotal < 0.0)
			{
				fTotal = 0.0;
				fAvg = 0.0;
			}

			// put the new value in the ring by expanding the ring if needed
			//  or replacing an empty value
			fRingEnd++;
			if (fRingEnd == fList.end())
				fRingEnd = fList.begin();
			// Do we have free space?
			if (fRingEnd == fRingStart)
			{
				// no free space
				fList.insert(fRingEnd,Item<T>(value,time));
				fRingEnd--;
			}
			else
			{
				// yes free space @ fRingEnd
				(*fRingEnd) = Item<T>(value,time);
			}
		}

		//update the avg
		fTotal += (*fRingEnd).GetValue();
		double currentLen = (*fRingEnd).GetTime() - (*fRingStart).GetTime();
		if (currentLen < 1.0)
			fAvg = (float)fTotal;
		else
			fAvg = (float)(fTotal / currentLen);
	}

	// update the max avg
	fMaxAvg = hsMaximum( fMaxAvg, fAvg );

}

