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
#include "hsStream.h"
#include "plCluster.h"

#include "plSpanTemplate.h"
#include "plSpanInstance.h"

#include "hsFastMath.h"

plCluster::plCluster()
:	fGroup(nil)
{
}

plCluster::~plCluster()
{
	int i;
	for( i = 0; i < fInsts.GetCount(); i++ )
		delete fInsts[i];
}

void plCluster::Read(hsStream* s, plClusterGroup* grp)
{
	fGroup = grp;

	fEncoding.Read(s);

	hsAssert(fGroup->GetTemplate(), "Template should have been loaded by now");
	const int numVerts = fGroup->GetTemplate()->NumVerts();
	const int numInst = s->ReadSwap32();
	fInsts.SetCount(numInst);
	int i;
	for( i = 0; i < numInst; i++ )
	{
		fInsts[i] = TRACKED_NEW plSpanInstance;
		fInsts[i]->Read(s, fEncoding, numVerts);
	}

}

void plCluster::Write(hsStream* s) const
{
	fEncoding.Write(s);

	const int numVerts = fGroup->GetTemplate()->NumVerts();
	s->WriteSwap32(fInsts.GetCount());
	int i;
	for( i = 0; i < fInsts.GetCount(); i++ )
	{
		fInsts[i]->Write(s, fEncoding, numVerts);
	}
}

inline void inlTESTPOINT(const hsPoint3& destP, 
						 hsScalar& minX, hsScalar& minY, hsScalar& minZ, 
						 hsScalar& maxX, hsScalar& maxY, hsScalar& maxZ)
{
	if( destP.fX < minX )
		minX = destP.fX;
	else if( destP.fX > maxX )
		maxX = destP.fX;

	if( destP.fY < minY )
		minY = destP.fY;
	else if( destP.fY > maxY )
		maxY = destP.fY;

	if( destP.fZ < minZ )
		minZ = destP.fZ;
	else if( destP.fZ > maxZ )
		maxZ = destP.fZ;
}

void plCluster::UnPack(UInt8* vDst, UInt16* iDst, int idxOffset, hsBounds3Ext& wBnd) const
{
	hsScalar minX = 1.e33f;
	hsScalar minY = 1.e33f;
	hsScalar minZ = 1.e33f;

	hsScalar maxX = -1.e33f;
	hsScalar maxY = -1.e33f;
	hsScalar maxZ = -1.e33f;

	hsAssert(fGroup->GetTemplate(), "Can't unpack without a template");
	const plSpanTemplate& templ = *fGroup->GetTemplate();
	int i;
	for( i = 0; i < fInsts.GetCount(); i++ )
	{
		// First, just copy our template, offsetting by prescribed amount.
		const UInt16* iSrc = templ.IndexData();
		int n = templ.NumIndices();
		while( n-- )
		{
			*iDst = *iSrc + idxOffset;
			iDst++;
			iSrc++;
		}
		idxOffset += templ.NumVerts();

		memcpy(vDst, templ.VertData(), templ.VertSize());

		// Now we need to fix it up. That means,
		// a) Possibly adding a delta to the position.
		// b) Transforming the position and normal.
		// c) Possibly overwriting some (or all) of the color.

		// If we have individual position and/or color info, apply
		// it, along with the transform.
		if( GetInst(i).HasPosDelta() || GetInst(i).HasColor() )
		{
			const int posOff = templ.PositionOffset();
			const int normOff = templ.NormalOffset();
			const int colOff = templ.ColorOffset();
			const int stride = templ.Stride();

			const hsMatrix44 l2w = GetInst(i).LocalToWorld();
			hsMatrix44 w2l;
			GetInst(i).WorldToLocal().GetTranspose(&w2l);
			
			plSpanInstanceIter iter(fInsts[i], fEncoding, templ.NumVerts());
			const int numVerts = templ.NumVerts();
			int iVert;
			for( iVert = 0, iter.Begin(); iVert < numVerts; iVert++, iter.Advance() )
			{
				hsPoint3* pos = (hsPoint3*)(vDst + posOff);
				*pos = iter.Position(*pos);
				*pos = l2w * *pos;
				inlTESTPOINT(*pos, minX, minY, minZ, maxX, maxY, maxZ);

				hsVector3* norm = (hsVector3*)(vDst + normOff);
				*norm = w2l * *norm;
				hsFastMath::NormalizeAppr(*norm);

				UInt32* color = (UInt32*)(vDst + colOff);
				*color = iter.Color(*color);

				vDst += stride;
			}
		}
		else
		{
			// Just transform the position and normal.
			const int posOff = templ.PositionOffset();
			const int normOff = templ.NormalOffset();
			const int stride = templ.Stride();
			
			const hsMatrix44 l2w = GetInst(i).LocalToWorld();
			hsMatrix44 w2l;
			GetInst(i).WorldToLocal().GetTranspose(&w2l);
			
			const int numVerts = templ.NumVerts();
			int iVert;
			for( iVert = 0; iVert < numVerts; iVert++ )
			{
				hsPoint3* pos = (hsPoint3*)(vDst + posOff);
				*pos = l2w * *pos;
				inlTESTPOINT(*pos, minX, minY, minZ, maxX, maxY, maxZ);

				hsVector3* norm = (hsVector3*)(vDst + normOff);
				*norm = w2l * *norm;

				vDst += stride;
			}
		}
	}
	wBnd.Reset(&hsPoint3(minX, minY, minZ));
	wBnd.Union(&hsPoint3(maxX, maxY, maxZ));
}


