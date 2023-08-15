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
#ifndef hsGCompMatDefined
#define hsGCompMatDefined

#include <vector>

#include "hsColorRGBA.h"
#include "hsGMatState.h"

#include "pnNetCommon/plSynchedObject.h"

class hsScene;
class hsResMgr;
class hsGDeviceRef;
class hsG3DDevice;
class plLayerInterface;
class plLayer;

// inlines for Texture and Material after class declarations

class hsGMaterial : public plSynchedObject
{
public:
    // Things we have to know that some layer has
    enum hsGCompFlags {
        kCompShaded                         = 0x1,
        kCompEnvironMap                     = 0x2,
        kCompProjectOnto                    = 0x4,
        kCompSoftShadow                     = 0x8,
        kCompSpecular                       = 0x10,
        kCompTwoSided                       = 0x20,
        kCompDrawAsSplats                   = 0x40,
        kCompAdjusted                       = 0x80,
        kCompNoSoftShadow                   = 0x100,
        kCompDynamic                        = 0x200,
        kCompDecal                          = 0x400,
        kCompIsEmissive_OBSOLETE            = 0x800,
        kCompIsLightMapped                  = 0x1000,
        kCompNeedsBlendChannel              = 0x2000    // For materials that have extra layers to simulate vtx alpha
    };
    enum UpdateFlags
    {
        kUpdateAgain        = 0x01
    };

protected:
    uint32_t                  fLOD;
    std::vector<plLayerInterface*> fLayers;
    std::vector<plLayerInterface*> fPiggyBacks;

    uint32_t                  fCompFlags;
    uint32_t                  fLoadFlags;

    float                fLastUpdateTime;
    
#if PLASMA_PIPELINE_GL || PLASMA_PIPELINE_METAL
    hsGDeviceRef*                   fDeviceRef;
#endif

    void                IClearLayers();
    size_t              IMakeExtraLayer();

    void                    InsertLayer(plLayerInterface* lay, int32_t which = 0, bool piggyBack = false);
    void                    SetLayer(plLayerInterface* lay, int32_t which = 0, bool insert=false, bool piggyBack=false);
    void                    ReplaceLayer(plLayerInterface* oldLay, plLayerInterface* newLay, bool piggyBack = false);
    void                    RemoveLayer(plLayerInterface* oldLay, bool piggyBack = false);
public:
    hsGMaterial();
    ~hsGMaterial();

    virtual hsGMaterial*    Clone();
    virtual hsGMaterial*    CloneNoLayers(); // For things like blending copies, that manipulate layers directly.
                                             // copies no keyed objects.
    plLayer*                MakeBaseLayer();
    plLayerInterface*       GetLayer(size_t which);
    plLayerInterface*       GetPiggyBack(size_t which);
    uint32_t                AddLayerViaNotify(plLayerInterface* lay);
    size_t                  GetNumLayers() const        { return fLayers.size(); }
    void                    SetNumLayers(size_t cnt);
    size_t                  GetNumPiggyBacks() const    { return fPiggyBacks.size(); }

    void                    SetLOD(uint32_t l)            { fLOD = l; }
    uint32_t                  GetLOD() const              { return fLOD; }

    void                    SetCompositeFlags(uint32_t f) { fCompFlags = f; } // normally composite flags are calculated internally, not set.
    uint32_t                  GetCompositeFlags() const   { return fCompFlags; }
    uint32_t                  GetLoadFlags() const        { return fLoadFlags; }

    float                GetLastUpdateTime() const   { return fLastUpdateTime; }
    void                    SetLastUpdateTime(float f) { fLastUpdateTime = f; }
    bool                    IShouldUpdate(float secs, uint32_t flags) { return GetLastUpdateTime() != secs || (flags & kUpdateAgain); }

    bool                    IsDynamic() const           { return (fCompFlags & kCompDynamic); }
    bool                    IsDecal() const             { return (fCompFlags & kCompDecal); }
    bool                    NeedsBlendChannel()         { return (fCompFlags & kCompNeedsBlendChannel); }
    
#if PLASMA_PIPELINE_GL || PLASMA_PIPELINE_METAL
    void SetDeviceRef(hsGDeviceRef* ref);
    hsGDeviceRef* GetDeviceRef() const { return fDeviceRef; }
#endif

    virtual void        Read(hsStream* s);
    virtual void        Write(hsStream* s);
    void Read(hsStream* s, hsResMgr *group) override;
    void Write(hsStream* s, hsResMgr *group) override;

    virtual void Eval(double secs, uint32_t frame);
    virtual void Reset();
    virtual void Init();

    CLASSNAME_REGISTER( hsGMaterial );
    GETINTERFACE_ANY( hsGMaterial, hsKeyedObject );
    
    bool MsgReceive(plMessage* msg) override;
};

#endif // hsGCompMatDefined
