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
#include "max.h"
//#include "mtlhdr.h"
#include "Shaders.h"

//===========================================================================
// Useful variables...
//===========================================================================

static PhongShader phongShader;
static BlinnShader blinnShader;
static MetalShader metalShader;
static hsMaxShader plasmaShader;

static Shader *shaders[3] = {
        &phongShader,
        &metalShader,
        &blinnShader,
};

AColor black(0.0f,0.0f,0.0f,0.0f);

//===========================================================================
// Useful functions...
//===========================================================================

Shader *GetShader(int s) { return shaders[s]; };

#if 1
// Quadratic
static inline float Soften(float r) {
    return r*(2.0f-r);
}
#else
// Cubic
static inline float Soften(float r) {
    return r*r*(3.0f-2.0f*r);
}
#endif

float CompK(float f0) {
    return float(2.0*sqrt(f0)/sqrt(1.0-f0));
}

float fres_metal(float c, float k) {
    float b,rpl, rpp,c2;
    b = k*k + 1.0f;
    c2 = c*c;
    rpl = (b*c2-2*c+1)/(b*c2+2*c+1);
    rpp = (b-2*c+c2)/(b+2*c+c2);
    return (.5f*(rpl+rpp));
}

//===========================================================================
// Phong shader... don't know if this is ever going to be used
//===========================================================================

void PhongShader::Illum(ShadeContext &sc, SIllumParams &ip) {
    LightDesc *l;
    Color lightCol;
    BOOL is_shiny;
    Point3 R;
    if (is_shiny=(ip.sh_str>0.0f)) 
        R = sc.ReflectVector();
    
    for (int i=0; i<sc.nLights; i++) {
        l = sc.Light(i);
        register float NL, diffCoef;
        Point3 L;
        if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
            // diffuse
            if (NL<=0.0f) 
                continue;
            if (l->affectDiffuse)
                ip.diffIllum += diffCoef*lightCol;
            if (is_shiny&&l->affectSpecular) {
                // specular (Phong) 
                float c = DotProd(L,R);
                if (c>0.0f) {
                    if (ip.softThresh!=0.0&&diffCoef<ip.softThresh) {
                        float r = diffCoef/ip.softThresh;
                        c *= Soften(r);
                    }
                    c = (float)pow((double)c, (double)ip.ph_exp); // could use table lookup for speed
                    ip.specIllum += c*ip.sh_str*lightCol;
                }
            }
        }
    }
    ip.specIllum *= ip.spec; 
}

//===========================================================================
// hsMax shader... write this?
//===========================================================================

void hsMaxShader::Illum(ShadeContext &sc, SIllumParams &ip) {
    LightDesc *l;
    Color lightCol;
    BOOL is_shiny;
    Point3 R;
    if (is_shiny=(ip.sh_str>0.0f)) 
        R = sc.ReflectVector();
    
    for (int i=0; i<sc.nLights; i++) {
        l = sc.Light(i);
        register float NL, diffCoef;
        Point3 L;
        if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
            // diffuse
            if (NL<=0.0f) 
                continue;
            if (l->affectDiffuse)
                ip.diffIllum += diffCoef*lightCol;
            if (is_shiny&&l->affectSpecular) {
                // specular  
                float c = DotProd(L,R);
                if (c>0.0f) {
                    c = (float)pow((double)c, (double)ip.ph_exp); // could use table lookup for speed
                    ip.specIllum += c*ip.sh_str*lightCol;
                }
            }
        }
    }
    ip.specIllum *= ip.spec; 
}

//===========================================================================
// Blinn shader... don't know if this is ever going to be used
//===========================================================================

void BlinnShader::Illum(ShadeContext &sc, SIllumParams &ip) {
    LightDesc *l;
    Color lightCol;
    
    // Blinn style phong
    BOOL is_shiny=(ip.sh_str>0.0f)?1:0; 
    double ph_exp = double(ip.ph_exp)*4.0; // This is to make the hilite compatible with normal phong
    for (int i=0; i<sc.nLights; i++) {
        l = sc.Light(i);
        register float NL, diffCoef;
        Point3 L;
        if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
            // diffuse
            if (NL<=0.0f) 
                continue;
            
            if (l->affectDiffuse)
                ip.diffIllum += diffCoef*lightCol;
            
            // specular (Phong) 
            if (is_shiny&&l->affectSpecular) {
                Point3 H = FNormalize(L-ip.V);
                float c = DotProd(ip.N,H);	 
                if (c>0.0f) {
                    if (ip.softThresh!=0.0&&diffCoef<ip.softThresh) {
                        c *= Soften(diffCoef/ip.softThresh);
                    }
                    c = (float)pow((double)c, ph_exp); // could use table lookup for speed
                    ip.specIllum += c*ip.sh_str*lightCol;
                }
            }
        }
    }
    ip.specIllum *= ip.spec; 
}

//===========================================================================
// Metal shader... don't know if this is ever going to be used
//===========================================================================

void MetalShader::Illum(ShadeContext &sc, SIllumParams &ip) {
    LightDesc *l;
    Color lightCol;
    BOOL gotKav = FALSE;
    float kav, fav0, m2inv,NV;
    
    //IPoint2 sp = sc.ScreenCoord();
    
    BOOL is_shiny;
    if (ip.sh_str>0.0f) {
        NV = -DotProd(ip.N,ip.V);  // N dot V: view vector is TOWARDS us.
        is_shiny = 1;
        float r = 1.0f-ip.shine;
        if (r==0.0f) r = .00001f;
        m2inv = 1.0f/(r*r);  
    }
    else 
        is_shiny = 0;
    
    
    for (int i=0; i<sc.nLights; i++) {
        l = sc.Light(i);
        register float NL, diffCoef;
        Point3 L;
        
        if (!l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) 
            continue;
        
        // diffuse
        if (NL>0.0f&&l->affectDiffuse)  // TBD is the NL test necessary?
            ip.diffIllum += diffCoef*lightCol;
        
        if (is_shiny&&l->affectSpecular) { // SPECULAR 
            Color fcol;
            float LH,NH,VH;
            float sec2;  // Was double?? TBD
            Point3 H;
            
            if (NV<0.0f) continue;
            
            H = FNormalize(L-ip.V);
            
            LH = DotProd(L,H);  // cos(phi)   
            NH = DotProd(ip.N,H);  // cos(alpha) 
            if (NH==0.0f) continue;
            VH = -DotProd(ip.V,H);
            
            // compute geometrical attenuation factor 
            float G = (NV<NL)? (2.0f*NV*NH/VH): (2.0f*NL*NH/VH);
            if (G>0.0f) {
                // Compute (approximate) indices of refraction
                //	this can be factored out for non-texture-mapped mtls
                if (!gotKav) {
                    fav0 = Intens(ip.diff);
                    if (fav0>=1.0f) fav0 = .9999f;
                    kav = CompK(fav0);	
                    gotKav = TRUE;
                }
                
                float fav = fres_metal(LH,kav);
                float t = (fav-fav0)/(1.0f-fav0);
                fcol = (1.0f-t)*ip.diff + Color(t,t,t);
                
                // Beckman distribution  (from Cook-Torrance paper)
                sec2 = 1.0f/(NH*NH);  // 1/sqr(cos) 
                float D = (.5f/PI)*sec2*sec2*m2inv*(float)exp((1.0f-sec2)*m2inv);  					
                if (G>1.0f) G = 1.0f;
                float Rs = ip.sh_str*D*G/(NV+.05f);	
                ip.specIllum += fcol*Rs*lightCol;
            }
        } 
    }
    ip.diffIllum *= 1.0f - ip.sh_str;
}


void MetalShader::SetShininess(float shininess, float shineStr) {
    float r = 1.0f-shininess;
    if (r==0.0f) r = .00001f;
    fm2inv = 1.0f/(r*r);  
    fshin_str = shineStr;
}

float MetalShader::EvalHilite(float x) {
    float c = (float)cos(x*PI);
    float sec2 = 1.0f/(c*c);	  /* 1/sqr(cos) */
    return fshin_str*(.5f/PI)*sec2*sec2*fm2inv*(float)exp((1.0f-sec2)*fm2inv);  					
}

