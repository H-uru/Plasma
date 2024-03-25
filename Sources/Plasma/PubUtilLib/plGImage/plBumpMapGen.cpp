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

#include "plBumpMapGen.h"

#include "plMipmap.h"

#include "hsFastMath.h"

#include "hsCodecManager.h"

plMipmap* plBumpMapGen::MakeCompatibleBlank(const plMipmap* src)
{
    return new plMipmap(src->GetWidth(), src->GetHeight(), plMipmap::kARGB32Config, 1, plMipmap::kUncompressed, plMipmap::UncompressedInfo::kRGB8888);
}

plMipmap* plBumpMapGen::TwosCompToBias(plMipmap* dst)
{
    uint8_t* pDst = (uint8_t*)dst->GetAddr32(0, 0);

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

plMipmap* plBumpMapGen::QikBumpMap(plMipmap* dst, const plMipmap* origSrc, uint32_t mask, uint32_t flags)
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
        newSrc->ResizeNicely((uint16_t)(dst->GetWidth()), (uint16_t)(dst->GetHeight()), plMipmap::kDefaultFilter);

        src = newSrc;
    }
    if( src->IsCompressed() )
    {
        plMipmap* newSrc = hsCodecManager::Instance().CreateUncompressedMipmap(const_cast<plMipmap*>(src), hsCodecManager::k32BitDepth);
        src = newSrc;
    }
    dst->SetCurrLevel(0);

    const int32_t divis = ((mask >> 0) & 0xff)
                    +((mask >> 8) & 0xff)
                    +((mask >> 16) & 0xff);

    const int width = src->GetWidth();
    const int height = src->GetHeight();

    const uint32_t alphaOr = flags & kScaleHgtByAlpha ? 0 : 0xff;

    uint32_t* pDst = dst->GetAddr32(0, 0);
    uint32_t* pBase = src->GetAddr32(0, 0);
    uint32_t* pSrc = pBase;
    int i;
    int j;
    for( j = 0; j < height; j++ )
    {
        uint32_t* pUp = j ? pSrc - width : pBase;
        uint32_t* pDn = j < height-1 ? pSrc + width : pBase;
        for( i = 0; i < width; i++ )
        {
            uint32_t* pLf = i ? pSrc - 1 : pSrc + width-1;
            uint32_t* pRt = i < width-1 ? pSrc + 1 : pSrc - width + 1;

            uint32_t up = (((*pUp & mask) >> 0) & 0xff)
                        + (((*pUp & mask) >> 8) & 0xff)
                        + (((*pUp & mask) >> 16) & 0xff);
            uint32_t dn = (((*pDn & mask) >> 0) & 0xff)
                        + (((*pDn & mask) >> 8) & 0xff)
                        + (((*pDn & mask) >> 16) & 0xff);

            uint32_t rt = (((*pRt & mask) >> 0) & 0xff)
                        + (((*pRt & mask) >> 8) & 0xff)
                        + (((*pRt & mask) >> 16) & 0xff);
            uint32_t lf = (((*pLf & mask) >> 0) & 0xff)
                        + (((*pLf & mask) >> 8) & 0xff)
                        + (((*pLf & mask) >> 16) & 0xff);

            uint32_t hgt = (((*pSrc & mask) >> 0) & 0xff)
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
            int32_t delUpDn = dn - up;
            delUpDn *= 255;
            delUpDn /= divis;

            int32_t delRtLf = lf - rt;
            delRtLf *= 255;
            delRtLf /= divis;

            hgt *= 255;
            hgt /= divis;

//          hgt = 0xff;

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

plMipmap* plBumpMapGen::QikNormalMap(plMipmap* dst, const plMipmap* src, uint32_t mask, uint32_t flags, float smooth)
{
    dst = QikBumpMap(dst, src, mask, flags & ~kBias);

    const int width = src->GetWidth();
    const int height = src->GetHeight();

    if( flags & kBubbleTest )
    {
        int8_t* pDst = (int8_t*)dst->GetAddr32(0, 0);

        int i;
        int j;
        for( j = 0; j < height; j++ )
        {
            for( i = 0; i < width; i++ )
            {
                float x = float(i) / float(width-1) * 2.f - 1.f;
                float y = float(j) / float(height-1) * 2.f - 1.f;

                float z = 1.f - x*x - y*y;
                if( z > 0 )
                    z = sqrt(z);
                else
                {
                    x = 0;
                    y = 0;
                    z = 1.f;
                }
                z *= smooth;
                float invLen = hsFastMath::InvSqrt(x*x + y*y + z*z) * 127.00f;


                pDst[2] = int8_t(x * invLen);
                pDst[1] = int8_t(y * invLen);
                pDst[0] = int8_t(z * invLen);

                pDst += 4;
            }
        }
    }
    else
    if( flags & kNormalize )
    {
        int8_t* pDst = (int8_t*)dst->GetAddr32(0, 0);

        int32_t nZ = int32_t(smooth * 127.00f);

        int i;
        int j;
        for( j = 0; j < height; j++ )
        {
            for( i = 0; i < width; i++ )
            {
                int32_t x = pDst[2];
                int32_t y = pDst[1];
        
                if( x )
                    x *= 1;
                if( y )
                    y *= 1;

                float invLen = hsFastMath::InvSqrt((float)(x*x + y*y + nZ*nZ)) * 127.0f;
                pDst[2] = int8_t(x * invLen);
                pDst[1] = int8_t(y * invLen);
                pDst[0] = int8_t(nZ * invLen);

                pDst += 4;
            }
        }
    }
    else if( smooth != 1.f )
    {
        int32_t divis = 127;
        int32_t nZ = 127;
        if( (smooth > 1.f) )
        {
            divis = (int32_t)(smooth * 127);
        }
        else
        {
            nZ = uint32_t(smooth * 127.5f); 
        }
        int8_t* pDst = (int8_t*)dst->GetAddr32(0, 0);

        int i;
        int j;
        for( j = 0; j < height; j++ )
        {
            for( i = 0; i < width; i++ )
            {
                int32_t v;
                *pDst = (int8_t)nZ;
                pDst++;

                v = *pDst * 127;
                v /= divis;
                *pDst = (int8_t)(v & 0xff);
                pDst++;
                
                v = *pDst * 127;
                v /= divis;
                *pDst = (int8_t)(v & 0xff);

                pDst += 2;
            }
        }
    }

    if( flags & kBias )
        TwosCompToBias(dst);

    return dst;
}
