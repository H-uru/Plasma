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
#ifndef __SHADERS_H
#define __SHADERS_H

class Shader;

//===========================================================================
// Interface tools...
//===========================================================================

struct SIllumParams {
    unsigned long flags;
    float sh_str, ph_exp, shine, softThresh;
    Color amb, diff, spec;
    Point3 N, V;
    Color diffIllum, specIllum;
};

const int SHADER_PHONG  = 0;
const int SHADER_METAL  = 1;
const int SHADER_BLINN  = 2;
const int SHADER_PLASMA = 3;

// Return shader of given type, using indices above...
Shader *GetShader(int s);

extern AColor black;

//===========================================================================
// Abstract shader...
//===========================================================================

class Shader {
public:
    virtual void Illum(ShadeContext &sc, SIllumParams &ip)=0;
    virtual void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol)=0;
    virtual void SetShininess(float shininess, float shineStr)=0;
    virtual float EvalHilite(float x)=0;
};

//===========================================================================
// Phong shader... 
//===========================================================================

class PhongShader : public Shader {
    float fs;
    float shin_str;
public:
    void Illum(ShadeContext &sc, SIllumParams &ip);
    void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.spec; };
    void SetShininess(float shininess, float shineStr) {
        fs = (float)pow(2.0,shininess*10.0);
        shin_str = shineStr;
    }
    float EvalHilite(float x) {
        return shin_str*(float)pow((double)cos(x*PI),(double)fs);  
    }
};

//===========================================================================
// Blinn shader... 
//===========================================================================

class BlinnShader : public Shader {
    float fs;
    float shin_str;
public:
    void Illum(ShadeContext &sc, SIllumParams &ip);
    void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.spec; };
    void SetShininess(float shininess, float shineStr) {
        fs = (float)pow(2.0,shininess*10.0);
        shin_str = shineStr;
    }
    float EvalHilite(float x) {
        return shin_str*(float)pow((double)cos(x*PI),(double)fs); 
    }
};

//===========================================================================
// Metal shader... 
//===========================================================================

class MetalShader : public Shader {
    float fm2inv, fshin_str;
public:
    void Illum(ShadeContext &sc, SIllumParams &ip);
    void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.diff; };
    void SetShininess(float shininess, float shineStr);
    float EvalHilite(float x);
};

//===========================================================================
// hsMax layer shader... 
//===========================================================================

class hsMaxShader : public Shader {
    float fs;
    float shin_str;
public:
    void Illum(ShadeContext &sc, SIllumParams &ip);
    void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.spec; };
    void SetShininess(float shininess, float shineStr) {
        fs = (float)pow(2.0,shininess*10.0);
        shin_str = shineStr;
    }
    float EvalHilite(float x) {
        return shin_str*(float)pow((double)cos(x*PI),(double)fs);  
    }
};





#endif

