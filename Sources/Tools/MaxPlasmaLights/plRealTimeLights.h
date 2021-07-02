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
//  plRealTimeLights - Header for the derived MAX RT light type plug-ins     //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PL_RTLIGHT_H
#define PL_RTLIGHT_H

#include "plRealTimeLightBase.h"
#include "resource.h"

class plMaxNode;



///////////////////////////////////////////////////////////////////////////////
//// Omni Light ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTOmniLight : public plRTLightBase
{
    public:

        Class_ID ClassID() override { return RTOMNI_LIGHT_CLASSID; }
        SClass_ID SuperClassID() override { return LIGHT_CLASS_ID; }

        int CanConvertToType(Class_ID obtype) override { return (obtype ==  RTOMNI_LIGHT_CLASSID ) ? 1 : 0; }
        plRTOmniLight();
            
        ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE) override;
        GenLight* NewLight(int type) override { return new plRTOmniLight(); }
        RefTargetHandle Clone(RemapDir &remap) override;

        void            InitNodeName(TSTR &s) override { s = _T("RTOmniLight"); }

        int             DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing) override;
        void            DrawCone(TimeValue t, GraphicsWindow *gw, float dist) override;
        virtual void    DrawArrows(TimeValue t, GraphicsWindow *gw, float dist);
        void            GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;

    protected:
        void    IBuildMeshes(BOOL isNew) override;
        bool    IHasAttenuation() override { return true; }
};

class plRTOmniLightDesc : public plMaxClassDesc<ClassDesc2>
{
    public:
    int             IsPublic() override             { return TRUE; }
    void*           Create(BOOL loading) override   { return new plRTOmniLight; }
    const TCHAR*    ClassName() override            { return GetString(IDS_DB_OMNI); }
    SClass_ID       SuperClassID() override         { return LIGHT_CLASS_ID; }
    Class_ID        ClassID() override              { return RTOMNI_LIGHT_CLASSID; }
    const TCHAR*    Category() override             { return _T("Plasma RunTime");}
    const TCHAR*    InternalName() override         { return _T("plRTOmni"); }  // returns fixed parsable name (scripter-visible name)
    HINSTANCE       HInstance() override            { return hInstance; }


    static plRTOmniLightDesc    fStaticDesc;

    static ClassDesc2   *GetDesc()        { return &fStaticDesc; }
};


///////////////////////////////////////////////////////////////////////////////
//// Spotlight ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTSpotLight : public plRTLightBase
{
    public:
        
        plRTSpotLight(BOOL loading);

        plRTSpotLight();

        Class_ID ClassID() override { return RTSPOT_LIGHT_CLASSID; }
        SClass_ID SuperClassID() override { return LIGHT_CLASS_ID; }

        int CanConvertToType(Class_ID obtype) override { return (obtype ==  RTSPOT_LIGHT_CLASSID ) ? 1 : 0; }
        ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE) override;
        GenLight* NewLight(int type) override { return new plRTSpotLight(); }
        RefTargetHandle Clone(RemapDir &remap) override;
        
        Texmap  *GetProjMap() override;

        BOOL    IsSpot() override { return TRUE; }
        int     GetProjector() override { return fLightPB->GetInt(kUseProjectorBool, 0); }

        void    InitNodeName(TSTR &s) override { s = _T("RTSpotLight"); }

        int     DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing) override;
        void    DrawCone(TimeValue t, GraphicsWindow *gw, float dist) override;
        void    GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;

    protected:
        void    IBuildMeshes(BOOL isNew) override;
        bool    IHasAttenuation() override { return true; }
};

class plRTSpotLightDesc : public plMaxClassDesc<ClassDesc2>
{
    public:
    int             IsPublic() override             { return TRUE; }
    void*           Create(BOOL loading = FALSE) override { return new plRTSpotLight; }
    const TCHAR*    ClassName() override            { return GetString(IDS_DB_FREE_SPOT); }
    SClass_ID       SuperClassID() override         { return LIGHT_CLASS_ID; }
    Class_ID        ClassID() override              { return RTSPOT_LIGHT_CLASSID; }
    const TCHAR*    Category() override             { return _T("Plasma RunTime"); }
    const TCHAR*    InternalName() override         { return _T("RTSpot"); }    // returns fixed parsable name (scripter-visible name)
    HINSTANCE       HInstance() override            { return hInstance; }

    static plRTSpotLightDesc    fStaticDesc;

    static ClassDesc2   *GetDesc()        { return &fStaticDesc; }

};


///////////////////////////////////////////////////////////////////////////////
//// Directional Light ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTDirLight : public plRTLightBase
{
    public:

        plRTDirLight();

        Class_ID ClassID() override { return RTDIR_LIGHT_CLASSID; }
        SClass_ID SuperClassID() override { return LIGHT_CLASS_ID; }

        ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE) override;
        GenLight* NewLight(int type) override { return new plRTDirLight(); }
        RefTargetHandle Clone(RemapDir &remap) override;

        int CanConvertToType(Class_ID obtype) override { return (obtype ==  RTDIR_LIGHT_CLASSID) ? 1 : 0; }

        void    DrawCone(TimeValue t, GraphicsWindow *gw, float dist) override;
        void    GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;

        BOOL IsDir() override { return TRUE; }

        void    InitNodeName(TSTR &s) override { s = _T("RTDirLight"); }

    protected:
        void    IBuildMeshes(BOOL isNew) override;

        void    IBuildZArrow( float x, float y, float zDist, float arrowSize, Point3 *pts );
};

class plRTDirLightDesc : public plMaxClassDesc<ClassDesc2>
{
    public:
    int             IsPublic() override             { return TRUE; }
    void*           Create(BOOL loading) override   { return new plRTDirLight; }
    const TCHAR*    ClassName() override            { return GetString(IDS_DB_DIRECTIONAL); }
    SClass_ID       SuperClassID() override         { return LIGHT_CLASS_ID; }
    Class_ID        ClassID() override              { return RTDIR_LIGHT_CLASSID; }
    const TCHAR*    Category() override             { return _T("Plasma RunTime");}
    const TCHAR*    InternalName() override         { return _T("RTDir"); } // returns fixed parsable name (scripter-visible name)
    HINSTANCE       HInstance() override            { return hInstance; }

    static plRTDirLightDesc     fStaticDesc;

    static ClassDesc2   *GetDesc()        { return &fStaticDesc; }

};


#endif