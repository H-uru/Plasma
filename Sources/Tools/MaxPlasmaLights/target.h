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
/**********************************************************************
 *<
    FILE: target.h

    DESCRIPTION:  Defines a Target Object Class

    CREATED BY: Dan Silva

    HISTORY: created 11 January 1995

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __TARGET__H__ 

#define __TARGET__H__

class TargetObject: public GeomObject {            
    friend class TargetObjectCreateCallBack;
    friend BOOL CALLBACK TargetParamDialogProc( HWND hDlg, UINT message, 
        WPARAM wParam, LPARAM lParam );
    
        // Mesh cache
        static HWND hSimpleCamParams;
        static IObjParam* iObjParams;       
        static Mesh mesh;       
        static int meshBuilt;

        void GetMat(TimeValue t, INode* inode, ViewExp* vpt, Matrix3& tm);
        void BuildMesh();

    //  inherited virtual methods for Reference-management
        RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
           PartID& partID, RefMessage message );

    public:
        TargetObject();

        //  inherited virtual methods:

        // From BaseObject
        int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
        void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
        int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
        CreateMouseCallBack* GetCreateMouseCallBack();
        void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
        void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
        TCHAR *GetObjectName() { return GetString(IDS_DB_TARGET); }

        // From Object
        ObjectState Eval(TimeValue time);
        void InitNodeName(TSTR& s) { s = GetString(IDS_DB_TARGET); }
        ObjectHandle ApplyTransform(Matrix3& matrix);
        int UsesWireColor() { return 0; }
        int IsRenderable() { return 0; }

        // From GeomObject
        int IntersectRay(TimeValue t, Ray& r, float& at);
        ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation      
        void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
        void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
        void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

        // From Animatable 
        void DeleteThis() {
             delete this; 
             }
        Class_ID ClassID() { return Class_ID(TARGET_CLASS_ID,0); }  
        void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_TARGET_CLASS)); }
        int IsKeyable(){ return 1;}
        LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
                WPARAM wParam,   LPARAM lParam ){return(0);}

        // From ref.h
        RefTargetHandle Clone(RemapDir& remap = DEFAULTREMAP);

        // IO
        IOResult Save(ISave *isave);
        IOResult Load(ILoad *iload);
    };


#endif
