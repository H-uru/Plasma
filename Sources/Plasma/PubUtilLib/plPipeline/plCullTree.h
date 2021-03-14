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

#ifndef plCullTree_inc
#define plCullTree_inc

#include "hsBounds.h"
#include "hsGeometry3.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
#include "plCuller.h"
#include "plScene/plCullPoly.h"

#ifdef HS_DEBUGGING
#define DEBUG_POINTERS
#endif // HS_DEBUGGING

class plCullTree;
class plCullNode;

// for vis
struct hsPoint3;
struct hsVector3;
struct hsColorRGBA;

class plCullTree : public plCuller
{
protected:

    // Visualization stuff, to be nuked from production version.
    mutable bool                            fCapturePolys;
    mutable hsTArray<hsPoint3>              fVisVerts;
    mutable hsTArray<hsVector3>             fVisNorms;
    mutable hsTArray<hsColorRGBA>           fVisColors;
    mutable hsTArray<uint16_t>                fVisTris;
    mutable float                        fVisYon;

    mutable hsTArray<plCullPoly>    fScratchPolys;
    mutable hsTArray<int16_t>       fScratchClear;
    mutable hsTArray<int16_t>       fScratchSplit;
    mutable hsTArray<int16_t>       fScratchCulled;
    mutable hsBitVector             fScratchBitVec;
    mutable hsBitVector             fScratchTotVec;

    void        IVisPolyShape(const plCullPoly& poly, bool dark) const;
    void        IVisPolyEdge(const hsPoint3& p0, const hsPoint3& p1, bool dark) const;
    void        IVisPoly(const plCullPoly& poly, bool dark) const;


    hsPoint3                        fViewPos;

    int16_t                           fRoot;
    mutable hsTArray<plCullNode>    fNodeList; // Scratch list we make the tree from.
    plCullNode*                     IGetRoot() const { return IGetNode(fRoot); }
    plCullNode*                     IGetNode(int16_t i) const { return i >= 0 ? &fNodeList[i] : nullptr; }

    void                ITestNode(const plSpaceTree* space, int16_t who, hsTArray<int16_t>& outList) const; // Appends to outlist
    void                ITestList(const plSpaceTree* space, const hsTArray<int16_t>& inList, hsTArray<int16_t>& outList) const;

    int16_t               IAddPolyRecur(const plCullPoly& poly, int16_t iNode);
    int16_t               IMakeHoleSubTree(const plCullPoly& poly) const;
    int16_t               IMakePolySubTree(const plCullPoly& poly) const;
    int16_t               IMakePolyNode(const plCullPoly& poly, int i0, int i1) const;

    // Some scratch areas for the nodes use when building the tree etc.
    hsTArray<plCullPoly>&           ScratchPolys() const { return fScratchPolys; }
    hsTArray<int16_t>&              ScratchClear() const { return fScratchClear; }
    hsTArray<int16_t>&              ScratchSplit() const { return fScratchSplit; }
    hsTArray<int16_t>&              ScratchCulled() const { return fScratchCulled; }
    hsBitVector&                    ScratchBitVec() const { return fScratchBitVec; }
    hsBitVector&                    ScratchTotVec() const { return fScratchTotVec; }

    void                            ISetupScratch(uint16_t nNodes);

    friend class plCullNode;

public:
    plCullTree();
    ~plCullTree();

    void                    Reset(); // Called before starting to add polys for this frame.
    void                    InitFrustum(const hsMatrix44& world2NDC);
    void                    SetViewPos(const hsPoint3& pos);
    void                    AddPoly(const plCullPoly& poly);

    uint32_t                  GetNumNodes() const { return fNodeList.GetCount(); }

    void                    Harvest(const plSpaceTree* space, std::vector<int16_t>& outList) const override;
    virtual bool            BoundsVisible(const hsBounds3Ext& bnd) const;
    virtual bool            SphereVisible(const hsPoint3& center, float rad) const;

    // Visualization stuff. Only to be called by the pipeline (or some other vis manager).
    void                    SetVisualizationYon(float y) const { fVisYon = y; }
    void                    BeginCapturePolys() const { fCapturePolys = true; }
    void                    EndCapturePolys() const { fCapturePolys = false; }
    hsTArray<hsPoint3>&     GetCaptureVerts() const { return fVisVerts; }
    hsTArray<hsVector3>&    GetCaptureNorms() const { return fVisNorms; }
    hsTArray<hsColorRGBA>&  GetCaptureColors() const { return fVisColors; }
    hsTArray<uint16_t>&       GetCaptureTris() const { return fVisTris; }
    void                    ReleaseCapture() const;
};

class plCullNode
{
public:
enum plCullStatus
{
    kClear,
    kCulled,
    kSplit,
    kPureSplit
};
protected:
    hsVector3           fNorm;
    float            fDist;

    bool                fIsFace;

    int16_t               fInnerChild;
    int16_t               fOuterChild;

    const plCullTree*           fTree;

    plCullNode*                 IGetNode(int16_t i) const;

#ifdef DEBUG_POINTERS
    mutable plCullNode*         fInnerPtr;
    mutable plCullNode*         fOuterPtr;

    void                        ISetPointersRecur() const;
#else // DEBUG_POINTERS
    void                        ISetPointersRecur() const {}
#endif // DEBUG_POINTERS

    // Bounds only version
    plCullNode::plCullStatus    ITestBoundsRecur(const hsBounds3Ext& bnd) const;
    plCullNode::plCullStatus    ITestSphereRecur(const hsPoint3& center, float rad) const;

    // Using the nodes
    plCullNode::plCullStatus    ITestNode(const plSpaceTree* space, int16_t who, hsTArray<int16_t>& clear, hsTArray<int16_t>& split, hsTArray<int16_t>& culled) const;
    void                        ITestNode(const plSpaceTree* space, int16_t who, hsBitVector& totList, hsBitVector& outList) const;
    void                        IHarvest(const plSpaceTree* space, std::vector<int16_t>& outList) const;

    // Constructing the tree
    float                    IInterpVert(const hsPoint3& p0, const hsPoint3& p1, hsPoint3& out) const;
    plCullNode::plCullStatus    ISplitPoly(const plCullPoly& poly, plCullPoly*& innerPoly, plCullPoly*& outerPoly) const;
    void                        IMarkClipped(const plCullPoly& poly, const hsBitVector& onVerts) const;
    void                        ITakeHalfPoly(const plCullPoly& scrPoly, 
                                   const hsTArray<int>& vtxIdx, 
                                   const hsBitVector& onVerts, 
                                   plCullPoly& outPoly) const;
    void                        IBreakPoly(const plCullPoly& poly, const hsTArray<float>& depths,
                                    hsBitVector& inVerts,
                                    hsBitVector& outVerts,
                                    hsBitVector& onVerts,
                                    plCullPoly& srcPoly) const;

    hsTArray<plCullPoly>&           ScratchPolys() const { return fTree->ScratchPolys(); }
    hsTArray<int16_t>&              ScratchClear() const { return fTree->ScratchClear(); }
    hsTArray<int16_t>&              ScratchSplit() const { return fTree->ScratchSplit(); }
    hsTArray<int16_t>&              ScratchCulled() const { return fTree->ScratchCulled(); }
    hsBitVector&                    ScratchBitVec() const { return fTree->ScratchBitVec(); }
    hsBitVector&                    ScratchTotVec() const { return fTree->ScratchTotVec(); }

    friend class plCullTree;
public:

    void    Init(const plCullTree* t, const hsVector3& n, float d) { fIsFace = false; fTree = t; fInnerChild = fOuterChild = -1; SetPlane(n, d); }
    void    Init(const plCullTree* t, const plCullPoly& poly) { Init(t, poly.fNorm, poly.fDist); }

    void    SetPlane(const hsVector3& n, float d) { fNorm = n; fDist = d; }
    
    const hsVector3& GetNormal() const { return fNorm; }
    float GetDist() const { return fDist; }

    plCullStatus    TestBounds(const hsBounds3Ext& bnd) const;
    plCullStatus    TestSphere(const hsPoint3& center, float rad) const;
};

inline plCullNode* plCullNode::IGetNode(int16_t i) const
{ 
    return fTree->IGetNode(i); 
}

#endif // plCullTree_inc
