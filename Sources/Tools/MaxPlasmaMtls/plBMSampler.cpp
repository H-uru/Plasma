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

#include <cmath>

#include "Layers/plPlasmaMAXLayer.h"
#include "plBMSampler.h"

plBMSampler::plBMSampler(plPlasmaMAXLayer *layer, Bitmap *bm) : fBM(bm), fInitialized(false)
{
    // Get our parameters
    if( fBM && layer && layer->GetSamplerInfo( &fData ) )
    {
        u1 = fData.fClipU + fData.fClipW;
        v1 = fData.fClipV + fData.fClipH;
        bmw = fBM->Width();
        bmh = fBM->Height();
        fbmw = float(bmw-1);
        fbmh = float(bmh-1);
        clipx = int(fData.fClipU*fbmw);
        clipy = int(fData.fClipV*fbmh);
        fclipw = fData.fClipW*fbmw;
        fcliph = fData.fClipH*fbmh;
        cliph = int(fcliph);

        fInitialized = true;
    }
}

int plBMSampler::PlaceUV(ShadeContext &sc, float &u, float &v, int iu, int iv) 
{
    if (!fInitialized)
        return 1;

    if (u<fData.fClipU||v<fData.fClipV||u>u1||v>v1)
        return 0;

    u = (u-fData.fClipU)/fData.fClipW;
    v = (v-fData.fClipV)/fData.fClipH;

    return 1;
}

void plBMSampler::PlaceUVFilter(ShadeContext &sc, float &u, float &v, int iu, int iv) 
{
    if (!fInitialized)
        return;

    u = (u-fData.fClipU)/fData.fClipW;
    v = (v-fData.fClipV)/fData.fClipH;
}

AColor plBMSampler::Sample(ShadeContext& sc, float u,float v) 
{
    AColor none(0.0f, 0.0f, 0.0f, 0.0f);

    if (!fInitialized)
        return none;

    BMM_Color_64 c;
    int x,y;
    float fu,fv, intpart;
    fu = std::modf(u, &intpart);
    fv = 1.0f - std::modf(v, &intpart);
    if (fData.fEnableCrop)
    {
        if (fData.fCropPlacement)
        {
            if (!PlaceUV(sc,fu, fv, int(u), int(v)))
                return AColor(0,0,0,0);
            x = (int)(fu*fbmw+0.5f);
            y = (int)(fv*fbmh+0.5f);
        }
        else
        {
            x = clipx + static_cast<int>(fu * fclipw + 0.5f) % bmw;
            y = clipy + static_cast<int>(fv * fcliph + 0.5f) % bmh;
        }
    }
    else
    {
        x = (int)(fu*fbmw+0.5f);
        y = (int)(fv*fbmh+0.5f);
    }
    fBM->GetLinearPixels(x,y,1,&c);
    switch(fData.fAlphaSource)
    {
    case plBMSamplerData::kDiscard:
        c.a = 0xffff; 
        break;
    case plBMSamplerData::kFromRGB:
        c.a = (c.r+c.g+c.b)/3; 
        break;
        //  TBD
        // XPCOL needs to be handled in bitmap for filtering. 
        // Need to open a bitmap with this property.
        //  case ALPHA_XPCOL:  break; 
    }
    return c;
}


AColor plBMSampler::SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) 
{
    AColor none(0.0f, 0.0f, 0.0f, 0.0f);

    if (!fInitialized)
        return none;

    fBM->SetFilter(BMM_FILTER_PYRAMID);

    BMM_Color_64 c;
    float fu, fv, intpart;
    fu = std::modf(u, &intpart);
    fv = 1.0f - std::modf(v, &intpart);
    if (fData.fEnableCrop)
    {
        if (fData.fCropPlacement)
        {
            PlaceUVFilter(sc,fu, fv, int(u), int(v));
            du /= fData.fClipW;
            dv /= fData.fClipH;
            float du2 = 0.5f*du;
            float ua = fu-du2;
            float ub = fu+du2;
            if (ub<=0.0f||ua>=1.0f) return none;
            float dv2 = 0.5f*dv;
            float va = fv-dv2;
            float vb = fv+du2;
            if (vb<=0.0f||va>=1.0f) return none;
            BOOL clip = 0;
            if (ua<0.0f) { ua=0.0f; clip = 1; }
            if (ub>1.0f) { ub=1.0f; clip = 1; }
            if (va<0.0f) { va=0.0f; clip = 1; }
            if (vb>1.0f) { vb=1.0f; clip = 1; }
            fBM->GetFiltered(fu,fv,du,dv,&c);
            switch(fData.fAlphaSource)
            {
            case plBMSamplerData::kDiscard:
                c.a = 0xffff; 
                break;
            case plBMSamplerData::kFromRGB:   
                c.a = (c.r + c.g + c.b)/3; 
                break;
            }
            AColor ac(c);
            if (clip)
            {
                float f = ((ub-ua)/du) * ((vb-va)/dv);
                ac *= f;
            }
            return ac;
        }
        else
        {
            fu = (fData.fClipU + fData.fClipW*fu);
            fv = (fData.fClipV + fData.fClipH*fv);
            du *= fData.fClipW;
            dv *= fData.fClipH;
            fBM->GetFiltered(fu,fv,du,dv,&c);
        }
    }
    else 
        fBM->GetFiltered(fu,fv,du,dv,&c);

    switch (fData.fAlphaSource)
    {
    case plBMSamplerData::kDiscard:  
        c.a = 0xffff; 
        break;
    case plBMSamplerData::kFromRGB:   
        c.a = (c.r + c.g + c.b)/3; 
        break;
    }

    return c;
}
