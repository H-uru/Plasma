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
//  plRTProjDirLight.h - Header for the derived MAX RT projected directional //
//                       light                                               //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTProjDirLight_h
#define _plRTProjDirLight_h

class Class_ID;
class IParamBlock2;
class ReferenceMaker;


///////////////////////////////////////////////////////////////////////////////
//// plRTProjPBAccessor ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTProjPBAccessor : public PBAccessor
{
    public:
        void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
        void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) override;

        static plRTProjPBAccessor   *Instance() { return &fAccessor; }

    protected:

        static plRTProjPBAccessor   fAccessor;
};

///////////////////////////////////////////////////////////////////////////////
//// Projected Directional Light //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTProjDirLight : public plRTLightBase
{
    public:

        friend class plRTProjPBAccessor;

        enum Blocks
        {
            kBlkProj = kBlkDerivedStart
        };

        enum References
        {
            kRefMainRollout = kRefDerivedStart,
            kRefProjRollout,

            kNumRefs
        };

        enum ProjRollout
        {
            kWidth,
            kHeight,
            kRange,
            kShowCone,
            kProjMap,
            kTexmap,

            // Someone goofed and used this index from the base class in our param block.
            // luckily it worked because there was no overlap, but we can't change it to
            // clean it up without breaking everything that uses it. So hopefully this
            // will at least clarify what's going on and prevent someone from stepping on
            // the index later.
            kProjTypeRadio = plRTLightBase::kProjTypeRadio,
        };

        plRTProjDirLight();

        /// Class ID stuff
        Class_ID    ClassID() override { return RTPDIR_LIGHT_CLASSID; }
        SClass_ID   SuperClassID() override { return LIGHT_CLASS_ID; }

        ObjLightDesc    *CreateLightDesc(INode *n, BOOL forceShadowBuf = FALSE) override;
        GenLight        *NewLight(int type) override { return new plRTProjDirLight(); }
        RefTargetHandle Clone(RemapDir &remap) override;

        int     CanConvertToType(Class_ID obtype) override { return ( obtype ==  RTPDIR_LIGHT_CLASSID ) ? 1 : 0; }

        void    GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;
        int     DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing) override;
        void    DrawCone(TimeValue t, GraphicsWindow *gw, float dist) override;

        BOOL            IsDir() override { return TRUE; }
        RefTargetHandle GetReference(int i) override;
        void            SetReference(int ref, RefTargetHandle rtarg) override;
        int             NumRefs() override { return kNumRefs; }

        int             NumSubs() override { return 2; }
        MSTR            SubAnimName(int i MAX_NAME_LOCALIZED2) override;
        Animatable      *SubAnim(int i) override;

        int             NumParamBlocks() override;
        IParamBlock2    *GetParamBlock(int i) override;
        IParamBlock2    *GetParamBlock2() override { return fLightPB; }
        IParamBlock2    *GetParamBlockByID(BlockID id) override;

        Texmap          *GetProjMap() override;
        
        void            InitNodeName(MSTR &s) override { s = _M( "RTProjDirLight" ); }

        // To get using-light-as-camera-viewport to work
        int             GetSpotShape() override { return RECT_LIGHT; }
        float           GetAspect(TimeValue t, Interval &valid = Interval(0,0)) override;
        float           GetFallsize(TimeValue t, Interval &valid = Interval(0,0)) override;
        int             Type() override { return DIR_LIGHT; }
        float           GetTDist(TimeValue t, Interval &valid = Interval(0,0)) override;
        void            SetFallsize(TimeValue time, float f) override;

        RefResult       EvalLightState(TimeValue t, Interval& valid, LightState *ls) override;

    protected:

        IParamBlock2    *fProjPB;

        void            IBuildMeshes(BOOL isNew) override;
        
        void            IBuildRectangle( float width, float height, float z, Point3 *pts );
};

#endif  // _plRTProjDirLight_h
