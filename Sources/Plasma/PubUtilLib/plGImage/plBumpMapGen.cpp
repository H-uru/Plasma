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

#include "plBumpMapGen.h"

#include "plMipmap.h"

#include "hsFastMath.h"

#include "hsCodecManager.h"

plMipmap* plBumpMapGen::MakeCompatibleBlank(const plMipmap* src)
{
	return TRACKED_NEW plMipmap(src->GetWidth(), src->GetHeight(), plMipmap::kARGB32Config, 1, plMipmap::kUncompressed, plMipmap::UncompressedInfo::kRGB8888);
}

plMipmap* plBumpMapGen::TwosCompToBias(plMipmap* dst)
{
	UInt8* pDst = (UInt8*)dst->GetAddr32(0, 0);

	const int width = dst->GetWidth();
	const int height = dst->GetHeight();
	int i;
	int j;
	for( j = 0; j < height; j++ )
	{
		for( i = 0; i < width; i++ )
		{
			*pDst++ += 128;
			*pDst++ += 128;
			*pDst++ += 128;
			pDst++;
		}
	}

	return dst;
}

plMipmap* plBumpMapGen::QikBumpMap(plMipmap* dst, const plMipmap* origSrc, UInt32 mask, UInt32 flags)
{
	const plMipmap* src = origSrc;
	if( !dst )
	{
		dst = MakeCompatibleBlank(src);
	}
	else if( (src->GetWidth() != dst->GetWidth()) || (src->GetHeight() != dst->GetHeight()) )
	{
		plMipmap* newSrc = src->Clone();
		// Note here that ResizeNicely currently does a point sample if scaling up (and about
		// as expensive a point sample as possible without using transcendental functions).
		// This might be correctable by calling plMipmap::Filter after the upscale. Or I 
		// could just assert that dst dimensions match src dimensions.
		newSrc->ResizeNicely((UInt16)(dst->GetWidth()), (UInt16)(dst->GetHeight()), plMipmap::kDefaultFilter);

		src = newSrc;
	}
	if( src->IsCompressed() )
	{
		plMipmap* newSrc = hsCodecManager::Instance().CreateUncompressedMipmap(const_cast<plMipmap*>(src), hsCodecManager::k32BitDepth);
		src = newSrc;
	}
	dst->SetCurrLevel(0);

	const Int32 divis = ((mask >> 0) & 0xff)
					+((mask >> 8) & 0xff)
					+((mask >> 16) & 0xff);

	const int width = src->GetWidth();
	const int height = src->GetHeight();
	const int stride = src->GetRowBytes(); // Should be width * 4;

	const UInt32 alphaOr = flags & kScaleHgtByAlpha ? 0 : 0xff;

	UInt32* pDst = dst->GetAddr32(0, 0);
	UInt32* pBase = src->GetAddr32(0, 0);
	UInt32* pSrc = pBase;
	int i;
	int j;
	for( j = 0; j < height; j++ )
	{
		UInt32* pUp = j ? pSrc - width : pBase;
		UInt32* pDn = j < height-1 ? pSrc + width : pBase;
		for( i = 0; i < width; i++ )
		{
			UInt32* pLf = i ? pSrc - 1 : pSrc + width-1;
			UInt32* pRt = i < width-1 ? pSrc + 1 : pSrc - width + 1;

			UInt32 up = (((*pUp & mask) >> 0) & 0xff)
						+ (((*pUp & mask) >> 8) & 0xff)
						+ (((*pUp & mask) >> 16) & 0xff);
			UInt32 dn = (((*pDn & mask) >> 0) & 0xff)
						+ (((*pDn & mask) >> 8) & 0xff)
						+ (((*pDn & mask) >> 16) & 0xff);

			UInt32 rt = (((*pRt & mask) >> 0) & 0xff)
						+ (((*pRt & mask) >> 8) & 0xff)
						+ (((*pRt & mask) >> 16) & 0xff);
			UInt32 lf = (((*pLf & mask) >> 0) & 0xff)
						+ (((*pLf & mask) >> 8) & 0xff)
						+ (((*pLf & mask) >> 16) & 0xff);

			UInt32 hgt = (((*pSrc & mask) >> 0) & 0xff)
						+ (((*pSrc & mask) >> 8) & 0xff)
						+ (((*pSrc & mask) >> 16) & 0xff);
			
			if( hgt )
				hgt *= 1;

			// Multiply by alpha, divide by 255 (so *= float(alpha/255))
			// If we aren't scaling by alpha, we just force alpha to be 255.
			hgt *= ((*pSrc >> 24) & 0xff) | alphaOr; // scale by alpha
			hgt /= 255;

			// divis has an implicit 255. For example, all three channels
			// are on, so divis = 0xff+0xff+0xff = 3*255.
			// So we muliply by 255 and divide by divis, so in this example,
			// that means divide by 3.
			Int32 delUpDn = dn - up;
			delUpDn *= 255;
			delUpDn /= divis;

			Int32 delRtLf = lf - rt;
			delRtLf *= 255;
			delRtLf /= divis;

			hgt *= 255;
			hgt /= divis;

//			hgt = 0xff;

			*pDst = ((delRtLf & 0xff) << 16)
					|((delUpDn & 0xff) << 8)
					|((0xff) << 0)
					|((hgt & 0xff) << 24);

			if( delRtLf )
				hgt *= 1;
			if( delUpDn )
				hgt *= 1;


			pUp++;
			pDn++;
			pSrc++;
			pDst++;
		}
	}

	if( flags & kBias )
		TwosCompToBias(dst);

	if( origSrc != src )
		delete src;

	return dst;
}

plMipmap* plBumpMapGen::QikNormalMap(plMipmap* dst, const plMipmap* src, UInt32 mask, UInt32 flags, hsScalar smooth)
{
	dst = QikBumpMap(dst, src, mask, flags & ~kBias);

	const int width = src->GetWidth();
	const int height = src->GetHeight();

	if( flags & kBubbleTest )
	{
		Int8* pDst = (Int8*)dst->GetAddr32(0, 0);

		Int32 nZ = Int32(smooth * 255.99f);

		int i;
		int j;
		for( j = 0; j < height; j++ )
		{
			for( i = 0; i < width; i++ )
			{
				hsScalar x = hsScalar(i) / hsScalar(width-1) * 2.f - 1.f;
				hsScalar y = hsScalar(j) / hsScalar(height-1) * 2.f - 1.f;

				hsScalar z = 1.f - x*x - y*y;
				if( z > 0 )
					z = hsSquareRoot(z);
				else
				{
					x = 0;
					y = 0;
					z = 1.f;
				}
				z *= smooth;
				hsScalar invLen = hsFastMath::InvSqrt(x*x + y*y + z*z) * 127.00f;


				pDst[2] = Int8(x * invLen);
				pDst[1] = Int8(y * invLen);
				pDst[0] = Int8(z * invLen);

				pDst += 4;
			}
		}
	}
	else
	if( flags & kNormalize )
	{
		Int8* pDst = (Int8*)dst->GetAddr32(0, 0);

		Int32 nZ = Int32(smooth * 127.00f);

		int i;
		int j;
		for( j = 0; j < height; j++ )
		{
			for( i = 0; i < width; i++ )
			{
				Int32 x = pDst[2];
				Int32 y = pDst[1];
		
				if( x )
					x *= 1;
				if( y )
					y *= 1;

				hsScalar invLen = hsFastMath::InvSqrt((hsScalar)(x*x + y*y + nZ*nZ)) * 127.0f;
				pDst[2] = Int8(x * invLen);
				pDst[1] = Int8(y * invLen);
				pDst[0] = Int8(nZ * invLen);

				pDst += 4;
			}
		}
	}
	else if( smooth != 1.f )
	{
		Int32 divis = 127;
		Int32 nZ = 127;
		if( (smooth > 1.f) )
		{
			divis = (Int32)(smooth * 127);
		}
		else
		{
			nZ = UInt32(smooth * 127.5f); 
		}
		Int8* pDst = (Int8*)dst->GetAddr32(0, 0);

		int i;
		int j;
		for( j = 0; j < height; j++ )
		{
			for( i = 0; i < width; i++ )
			{
				Int32 v;
				*pDst = (Int8)nZ;
				pDst++;

				v = *pDst * 127;
				v /= divis;
				*pDst = (Int8)(v & 0xff);
				pDst++;
				
				v = *pDst * 127;
				v /= divis;
				*pDst = (Int8)(v & 0xff);

				pDst += 2;
			}
		}
	}

	if( flags & kBias )
		TwosCompToBias(dst);

	return dst;
}
