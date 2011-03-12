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

#ifndef hsNoiseFunc_inc
#define hsNoiseFunc_inc

#include "hsRefCnt.h"
#include "hsScalar.h"

struct hsPoint3;

class hsNoiseFunc : public hsRefCnt // should inherit from keyed object
{
public:
	hsNoiseFunc();
	virtual ~hsNoiseFunc();
	
	virtual void Seed(UInt32 s);
	virtual hsScalar Noise(hsScalar lo=0, hsScalar hi=hsScalar1, hsScalar t=0) = 0; // t = [0..1] - returns random num [lo..hi] scaled by fTable[t]

	virtual hsScalar NoisePoint(const hsPoint3& p, hsScalar lo=0, hsScalar hi=hsScalar1, hsScalar t=0) = 0; // t = [0..1] - returns random num [lo..hi] scaled by fTable[t]
};

class hsTableNoise : public hsNoiseFunc // should inherit from keyed object
{
protected:
	hsScalar*		fTable;
	UInt32			fTableLen;
	
	
public:
	hsTableNoise();
	virtual ~hsTableNoise();
	
	void SetTable(int len, hsScalar* arr); // copies. arr should be hsScalars in range [0..1]
	hsScalar* GetTable(int& len) { len = fTableLen; return fTable; } // should be debug only, access through noise func
	
	virtual hsScalar Noise(hsScalar lo=0, hsScalar hi=hsScalar1, hsScalar t=0); // t = [0..1] - returns random num [lo..hi] scaled by fTable[t]

	virtual hsScalar NoisePoint(const hsPoint3& p, hsScalar lo=0, hsScalar hi=hsScalar1, hsScalar t=0); // t = [0..1] - returns random num [lo..hi] scaled by fTable[t]
};

#endif // hsNoiseFunc_inc
