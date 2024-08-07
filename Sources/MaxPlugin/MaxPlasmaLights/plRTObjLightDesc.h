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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plRTObjLightDesc.h - Header for the various ObjLightDesc classes         //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTObjLightDesc_h
#define _plRTObjLightDesc_h

class plRTLightBase;
class plMaxNode;

//// AttenRanges Class ////////////////////////////////////////////////////////

class AttenRanges 
{
    public:
        float aStart, aEnd; // Attenuation start and end and hot spot scaling for volume shading        
        //float aNearStart, aNearEnd;   // Near Attenuation start and end and hot spot scaling for volume shading       
        float decayRadius;
};

//// BaseObjLight Class ///////////////////////////////////////////////////////

class BaseObjLight : public ObjLightDesc 
{
    public:     
        Color intensCol;    // intens*color 
        Color shadColor;
        float contrast,kA,kB,diffSoft;
        int decayType;
        BOOL diffSoften;
        float decayRadius;
        plRTLightBase* gl;
        BaseObjLight(INode *n);
        void DeleteThis() override { delete this; }
        int Update(TimeValue t, const RendContext& rc, RenderGlobalContext * rgc, BOOL shadows, BOOL shadowGeomChanged) override;
        void UpdateGlobalLightLevel(Color globLightLevel) override { intensCol = ls.intens*ls.color*globLightLevel; }
        virtual Color AttenuateIllum(ShadeContext& sc,Point3 p,Color &colStep,Point3 &dp,int filt, float ldp, float &distAtten, AttenRanges &ranges) {return Color(0,0,0);}     
        virtual BOOL UseAtten()=0;
        virtual BOOL IsFacingLight(Point3 &dir) {return FALSE;}
        virtual int LightType()=0;

        inline float ContrastFunc(float nl) {
            if (diffSoft!=0.0f) {
                float p = nl*nl*(2.0f-nl);  // based on Hermite interpolant 
                nl = diffSoft*p + (1.0f-diffSoft)*nl;
                }
            return (contrast==0.0f)? nl: 
                nl/(kA*nl+kB);  //  the "Bias" function described in Graphics Gems IV, pp. 401ff
            }

};

//// OmniLight Class //////////////////////////////////////////////////////////

class OmniLight : public BaseObjLight 
{       
    Matrix3 tmCamToLight[6]; 
    BOOL shadow, doShadows, shadowRay;
    Texmap *projMap;
    BOOL needMultiple;
    BOOL genCanDoOmni;
    float zfac, xscale, yscale, fov, sz2,size,sizeClip,sampSize,sampSize2;
    public:     
        OmniLight(INode *inode, BOOL forceShadowBuf );
        ~OmniLight();
        int Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged) override;
        int UpdateViewDepParams(const Matrix3& worldToCam) override;
        BOOL UseAtten() override { return TRUE; }
        int LightType() override { return plRTLightBase::RT_OMNI; }
};

//// SpotLight Class //////////////////////////////////////////////////////////

class SpotLight: public BaseObjLight 
{   
    Point3 lightDir;  // light direction in render space
    BOOL projector; //, shadowRay, overshoot;
    float hot_cos, fall_cos, fall_tan, fall_sin;
    float hotpct, ihotpct;  
    float zfac, xscale,yscale, fov, sz2, curve;
    float out_range,in_range, range_span;
    Point2 rectv0, rectv1;
    Texmap* projMap;
    public:
        SpotLight(INode *inode, BOOL forceShadowBuf );
        ~SpotLight() {} //  FreeShadGens();     }
        int Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged) override;
        int UpdateViewDepParams(const Matrix3& worldToCam) override;
        BOOL UseAtten() override { return ls.useAtten; }
        BOOL IsFacingLight(Point3 &dir) override;
        int LightType() override { return FSPOT_LIGHT; }
};

//// DirLight Class ///////////////////////////////////////////////////////////

class DirLight : public BaseObjLight 
{
    Point3 lightDir;  // light direction in render space    
    //BOOL projector;//,overshoot;
    float hotsz, fallsz, fallsq;    
    float xscale, yscale, sz2, curve;
    float out_range,in_range, range_span;
    float hotpct,ihotpct;
    float aspect;
    float sw2, sh2;
    Texmap* projMap;
    public:
        DirLight(INode *inode, BOOL forceShadowBuf );
        ~DirLight() {   /*  FreeShadGens();*/}
        int Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged) override;
        int UpdateViewDepParams(const Matrix3& worldToCam) override;
        BOOL UseAtten() override { return FALSE; }
        int LightType() override { return plRTLightBase::RT_FREE_DIR; }
};


#endif  // _plRTObjLightDesc_h
