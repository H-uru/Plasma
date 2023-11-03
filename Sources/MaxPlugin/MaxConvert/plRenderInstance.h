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

#ifndef plRenderInstance_inc
#define plRenderInstance_inc

#include <vector>

#include <render.h>

class plRenderInstance : public RenderInstance
{
protected:
    plRenderInstance*           fNext;
    Interval                    fValid;

    INode*                      fNode;
    Object*                     fObject;

    BOOL                        fDeleteMesh;

    std::vector<LightDesc*>     fLights;

public:
    plRenderInstance();
    virtual ~plRenderInstance();

    BOOL GetFromNode(INode* node, TimeValue& t, int idx);
    BOOL Update(TimeValue& t);
    void SetNext(plRenderInstance* n) { fNext = n; }
    void Cleanup();

    RenderInstance *Next() override { return fNext; }    // next in list

    Interval MeshValidity() override { return fValid; }

    int NumLights() override { return (int)fLights.size(); }
    LightDesc *Light(int n) override { return fLights[n]; }
    virtual void AddLight(LightDesc* l) { fLights.emplace_back(l); }
    virtual void ClearLights() { fLights.clear(); }

    BOOL CastsShadowsFrom(const ObjLightDesc& lt) override; // is lt shadowed by this instance?

    INode *GetINode() override { return fNode; }                          // get INode for instance
    Object *GetEvalObject() override { return fObject; }                      // evaluated object for instance

    Point3 GetFaceNormal(int faceNum) override;         // geometric normal in camera coords
    Point3 GetFaceVertNormal(int faceNum, int vertNum) override;  // camera coords
    void GetFaceVertNormals(int faceNum, Point3 n[3]) override   // camera coords
    {
        n[0] = GetFaceVertNormal(faceNum, 0);
        n[1] = GetFaceVertNormal(faceNum, 1);
        n[2] = GetFaceVertNormal(faceNum, 2);
    }   
    Point3 GetCamVert(int vertnum) override;              // coord for vertex in camera coords
    void GetObjVerts(int fnum, Point3 obp[3]) override; // vertices of face in object coords
    void GetCamVerts(int fnum, Point3 cp[3]) override; // vertices of face in camera(view) coords

    // Material-by-face access
    // Objects can provide a material as a function of face number via the IChkMtlAPI interface (chkmtlapi.h).
    // This method will return RenderInstance::mtl if flag INST_MTL_BYFACE is not set. If INST_MTL_BYFACE is
    // set it will return the proper by-face mtl. // DS 4/3/00
    Mtl *GetMtl(int faceNum) override;
    ULONG MtlRequirements(int mtlNum, int faceNum) override;      // node's mtl requirements. DS 3/31/00: added faceNum to support mtl-per-face objects
};

#endif // plRenderInstance_inc
