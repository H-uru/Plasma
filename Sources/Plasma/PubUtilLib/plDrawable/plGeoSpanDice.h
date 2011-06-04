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

#ifndef plGeoSpanDice_inc
#define plGeoSpanDice_inc

#include "hsGeometry3.h"
#include "hsTemplates.h"

class plGeometrySpan;

class plGeoSpanDice
{
protected:
	UInt32		fMinFaces;
	UInt32		fMaxFaces;
	hsPoint3	fMaxSize;

	hsBool				INeedSplitting(plGeometrySpan* src) const;
	plGeometrySpan*		IAllocSpace(plGeometrySpan* src, int numVerts, int numTris) const;
	plGeometrySpan*		IExtractTris(plGeometrySpan* src, hsTArray<UInt32>& tris) const;
	int					ISelectAxis(int exclAxis, plGeometrySpan* src) const;
	hsBool				IHalf(plGeometrySpan* src, hsTArray<plGeometrySpan*>& out, int exclAxis=0) const;

public:
	plGeoSpanDice();
	virtual ~plGeoSpanDice();

	hsBool				Dice(hsTArray<plGeometrySpan*>& spans) const;

	void SetMaxSize(const hsPoint3& size) { fMaxSize = size; }
	hsPoint3 GetMaxSize() const { return fMaxSize; }

	void SetMinFaces(int n) { fMinFaces = n; }
	int GetMinFaces() const { return fMinFaces; }

	void SetMaxFaces(int n) { fMaxFaces = n; }
	int GetMaxFaces() const { return fMaxFaces; }
};

#endif // plGeoSpanDice_inc
