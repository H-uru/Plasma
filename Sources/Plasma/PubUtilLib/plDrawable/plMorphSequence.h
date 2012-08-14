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

#ifndef plMorphSequence_inc
#define plMorphSequence_inc

#include "pnModifier/plSingleModifier.h"
#include "plMorphArray.h"

class plDrawable;
class plDrawInterface;
class plSharedMesh;
class plMorphSequenceSDLMod;

class plMorphArrayWeights
{
public:
    hsTArray<float> fDeltaWeights;
};

class plSharedMeshInfo
{
public:
    enum
    {
        kInfoDirtyMesh = 0x1
    };

    plSharedMesh*       fMesh;
    hsTArray<int32_t>     fCurrIdx;
    plDrawable*         fCurrDraw;
    hsTArray<plMorphArrayWeights> fArrayWeights;
    uint8_t               fFlags;

    plSharedMeshInfo() : fMesh(nil), fCurrDraw(nil), fFlags(0) {}
};

// Keyed storage class for morph arrays/deltas
// supply your own weights.
class plMorphDataSet : public hsKeyedObject
{
public:
    hsTArray<plMorphArray>  fMorphs;

    CLASSNAME_REGISTER( plMorphDataSet );
    GETINTERFACE_ANY( plMorphDataSet, hsKeyedObject );
    
    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);     
};

// A place to hold incoming state while we're still waiting for the
// mesh and morph data to load.
class plMorphState
{
public:
    plKey fSharedMeshKey;
    hsTArray<plMorphArrayWeights> fArrayWeights;
};

class plMorphSequence : public plSingleModifier
{
    friend class plMorphSequenceSDLMod;
    
protected:
    enum
    {
        kDirty              = 0x1,
        kHaveSnap           = 0x2,
        kHaveShared         = 0x4,
        kDirtyIndices       = 0x8
    };
    uint32_t                      fMorphFlags;

    hsTArray<plMorphArray>      fMorphs;

    //int32_t                     fActiveMesh; // Doesn't appear to be used.
    hsTArray<plSharedMeshInfo>  fSharedMeshes;
    hsTArray<plMorphState>      fPendingStates;
    plMorphSequenceSDLMod*      fMorphSDLMod;
    int8_t                        fGlobalLayerRef;

    const plDrawInterface*      IGetDrawInterface() const;

    virtual bool IEval(double secs, float del, uint32_t dirty) { return false; }

    void ISetHaveSnap(bool on) { if(on)fMorphFlags |= kHaveSnap; else fMorphFlags &= ~kHaveSnap; }
    void ISetDirty(bool on);

    bool        IResetShared(int iShare);
    void        IApplyShared(int iShare);
    bool        IFindIndices(int iShare);
    void        IReleaseIndices(int iShare);

    void        IRenormalize(hsTArray<plAccessSpan>& dst) const;

    void        IResetShared();
    void        IReleaseIndices(); // Puts everyone inactive
    void        IFindIndices(); // Refresh Indicies
    void        IApplyShared(); // Apply whatever morphs are active

    int32_t       IFindPendingStateIndex(plKey meshKey) const; // Do we have pending state for this mesh?
    int32_t       IFindSharedMeshIndex(plKey meshKey) const; // What's this mesh's index in our array?
    bool        IIsUsingDrawable(plDrawable *draw); // Are we actively looking at spans in this drawable?

    // Internal functions for maintaining that all meshes share the same global weight(s) (fGlobalLayerRef)
    void        ISetAllSharedToGlobal();
    void        ISetSingleSharedToGlobal(int idx);
        

public:
    plMorphSequence();
    virtual ~plMorphSequence();

    CLASSNAME_REGISTER( plMorphSequence );
    GETINTERFACE_ANY( plMorphSequence, plSingleModifier );

    virtual bool MsgReceive(plMessage* msg);

    virtual void AddTarget(plSceneObject* so);
    virtual void RemoveTarget(plSceneObject* so);
    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr); 

    void Init();
    void Activate();
    void DeInit();
    void DeActivate();

    void Apply() const;
    void Reset(const plDrawInterface* di=nil) const;

    int GetNumLayers(plKey meshKey = nil) const; 
    void AddLayer(const plMorphArray& ma) { fMorphs.Append(ma); }

    int GetNumDeltas(int iLay, plKey meshKey = nil) const;
    float GetWeight(int iLay, int iDel, plKey meshKey = nil) const;
    void SetWeight(int iLay, int iDel, float w, plKey meshKey = nil);

    bool GetHaveSnap() const { return 0 != (fMorphFlags & kHaveSnap); }
    bool GetDirty() const { return 0 != (fMorphFlags & kDirty); }
    bool GetUseSharedMesh() const { return 0 != (fMorphFlags & kHaveShared); }

    void SetUseSharedMesh(bool on) { if(on)fMorphFlags |= kHaveShared; else fMorphFlags &= ~kHaveShared; }
    void AddSharedMesh(plSharedMesh* mesh);
    void RemoveSharedMesh(plSharedMesh* mesh);
    static void FindMorphMods(const plSceneObject *so, hsTArray<const plMorphSequence*> &mods);
    plMorphSequenceSDLMod *GetSDLMod() const { return fMorphSDLMod; }
};

#endif // plMorphSequence_inc
