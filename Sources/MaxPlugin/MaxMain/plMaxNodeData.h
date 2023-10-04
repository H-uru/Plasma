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

#ifndef plMaxNodeData_inc
#define plMaxNodeData_inc

#include <vector>

#include "hsBitVector.h"
#include "plLoadMask.h"
#include "plPhysicalProps.h"

class hsGMesh;
class plGeometrySpan;
class plLocation;
class plKey;
class plMaxBoneMap;
class plMaxNode;
class plMaxNodeBase;
class plSceneObject;
class plSharedMesh;

class plMaxNodeBaseTab : public Tab<plMaxNodeBase*>
{
};


//
//      Created hoping to trim some fat so to speak for all the bools that
//          we are tossing around.
//

class DataBF
{
public:
    enum DatBit
    {
        kCanConvert     = 0x0,
        kForceLocal,
        kDrawable,          // Absence of this prop forbids making this drawable
        kBlendToFB,
        kItinerant,
        kRunTimeLight,
        kMovable,
        kIsBarney,
        kForceMatCopy,
        kNoPreShade,
        kIsInstanced,
        kParticleRelated,
        kNoSpanReSort,
        kHasFade,
        kNoFaceSort,
        kNoSpanSort,
        kNoDeferDraw,
        kBLevelSet,
        kOLevelSet,
        kDup2Sided,
        kRadiateNorms,
        kGeoDice,
        kForcePreShade,
        kIsGUI,
        kSwappableGeomTarget,
        kNoShadow,
        kForceShadow,
        kFilterInherit,
        kReverseSort,
        kVS,
        kConcave,
        kCalcEdgeLens,
        kEnviron,
        kEnvironOnly,
        kPhysical,          // Absence of this prop forbids making this physical
        kSmoothAll,
        kForceSortable,
        kOverrideHighLevelSDL,
        kDisableNormal,
        kHasLoadMask,
        kWaterDecEnv,
        kWaterHeight,
        kAlphaTestHigh,
        kForceMatShade,
        kUnBounded,
        kForceVisLOS,
        kSortAsOpaque,
    };

    hsBitVector*    fBitVector;

    DataBF()                                            
    { 
        fBitVector = new hsBitVector;   
        fBitVector->SetBit(kDrawable); 
        fBitVector->SetBit(kPhysical); 
    }
    virtual ~DataBF()                                   { delete fBitVector; }

    DataBF& operator=(const DataBF& ot)
    {
        *fBitVector = *ot.fBitVector;
        return *this;
    }
    DataBF(const DataBF& ot)
    {
        fBitVector = new hsBitVector;
        *fBitVector = *ot.fBitVector;
    }

    void Init()                                         
    { 
        fBitVector = new hsBitVector; 
        fBitVector->SetBit(kDrawable); 
        fBitVector->SetBit(kPhysical); 
    }
    void DeInit()                                       { delete fBitVector; fBitVector = nullptr; }

    bool    CanBF(DatBit bitChoice)                 { return fBitVector->IsBitSet(bitChoice);   }
    void    SetBF(bool b, DatBit bitChoice)       { fBitVector->SetBit(bitChoice, b); }
};


class plMaxNodeData
{
public:
    plMaxNodeData() : 
        fpSO(),
        fDecalLevel(),
        fpMesh(),
        fSoundIdxCounter(),
        fAvatarSO(),
        fFade(Point3(0,0,0), Point3(0,0,0)),
        fNormalChan(),
        fWaterHeight(),
        fGDMaxFaces(), fGDMaxSize(), fGDMinFaces(),
        fSwapMesh(),
        fSwapTargetID((uint32_t)-1),
        fCachedAlphaHackLayerCounts(),
        fBoneMap(),
        fAnimCompression(1), // Should be plAnimCompressComp::kCompressionLow,
                             // but I don't want to include the entire header.
        fKeyReduceThreshold(0.0002f)
    { }
    ~plMaxNodeData() 
    { 
        fpKey = nullptr;
        fpRoomKey = nullptr;
        fpSO = 0; 
        fAvatarSO = nullptr;
        delete fCachedAlphaHackLayerCounts; 
        MaxDatBF.DeInit();
    }

    // Call init on MaxNodeData whose constructor was never called, e.g. if it was malloc'd.
    plMaxNodeData&  Init() 
    {
        memset( &fpKey, 0, sizeof( plKey ) ); 
        memset( &fpRoomKey, 0, sizeof( plKey ) ); 
        fRenderDependencies.Init(); 
        fBones.Init(); 
        fCachedAlphaHackLayerCounts = nullptr;
        MaxDatBF.Init();
        return *this; 
    }
    // Ditto
    void            DeInit()
    { 
        fpKey = nullptr;
        fpRoomKey = nullptr;
        fpSO = 0; fAvatarSO = nullptr;
        delete fCachedAlphaHackLayerCounts; 
        fCachedAlphaHackLayerCounts = nullptr;
        MaxDatBF.DeInit();
    }

    plKey           GetKey()                            { return fpKey;     }
    void            SetKey(plKey p )                    { fpKey = p;        }
    plSceneObject * GetSceneObject()                    { return fpSO;      }
    void            SetSceneObject(plSceneObject *p)    { fpSO = p;         }

    bool            CanConvert()                        { return MaxDatBF.CanBF(MaxDatBF.kCanConvert);  }
    void            SetCanConvert(bool b)             { MaxDatBF.SetBF(b, MaxDatBF.kCanConvert);      }

    bool            GetForceLocal()                     { return MaxDatBF.CanBF(MaxDatBF.kForceLocal);  }
    void            SetForceLocal(bool b)             { MaxDatBF.SetBF(b, MaxDatBF.kForceLocal);      }

    void *          GetMesh()                           { return fpMesh;    }   // void pointer for header simplicity
    void            SetMesh(hsGMesh *p)                 { fpMesh = p;       }       

    bool            GetDrawable()                       {return MaxDatBF.CanBF(MaxDatBF.kDrawable);     }
    void            SetDrawable(bool b)               { MaxDatBF.SetBF(b, MaxDatBF.kDrawable);        }

    bool            GetPhysical()                       {return MaxDatBF.CanBF(MaxDatBF.kPhysical);     }
    void            SetPhysical(bool b)               { MaxDatBF.SetBF(b, MaxDatBF.kPhysical);        }

    bool            GetRunTimeLight()                   {return MaxDatBF.CanBF(MaxDatBF.kRunTimeLight); }
    void            SetRunTimeLight(bool b)           { MaxDatBF.SetBF(b, MaxDatBF.kRunTimeLight);    }

    bool            GetForceMatShade()                  {return MaxDatBF.CanBF(MaxDatBF.kForceMatShade);    }
    void            SetForceMatShade(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kForceMatShade);   }

    bool            GetForceVisLOS()                    {return MaxDatBF.CanBF(MaxDatBF.kForceVisLOS);  }
    void            SetForceVisLOS(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kForceVisLOS); }

    bool            GetEnviron()                        {return MaxDatBF.CanBF(MaxDatBF.kEnviron);  }
    void            SetEnviron(bool b)                { MaxDatBF.SetBF(b, MaxDatBF.kEnviron); }

    bool            GetEnvironOnly()                    {return MaxDatBF.CanBF(MaxDatBF.kEnvironOnly);  }
    void            SetEnvironOnly(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kEnvironOnly); }

    bool            GetWaterDecEnv()                    { return MaxDatBF.CanBF(MaxDatBF.kWaterDecEnv); }
    void            SetWaterDecEnv(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kWaterDecEnv);     }

    bool            GetItinerant()                      {return MaxDatBF.CanBF(MaxDatBF.kItinerant);    }
    void            SetItinerant(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kItinerant);       }

    bool            GetUnBounded()                      {return MaxDatBF.CanBF(MaxDatBF.kUnBounded);    }
    void            SetUnBounded(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kUnBounded);       }

    bool            GetDisableNormal()                  {return MaxDatBF.CanBF(MaxDatBF.kDisableNormal);    }
    void            SetDisableNormal(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kDisableNormal);       }

    bool            GetMovable()                        {return MaxDatBF.CanBF(MaxDatBF.kMovable);      }
    void            SetMovable(bool b)                { MaxDatBF.SetBF(b, MaxDatBF.kMovable);         }

    bool            GetNoPreShade()                     {return MaxDatBF.CanBF(MaxDatBF.kNoPreShade);   }
    void            SetNoPreShade(bool b)             { MaxDatBF.SetBF(b, MaxDatBF.kNoPreShade);      }

    bool            GetForcePreShade()                  {return MaxDatBF.CanBF(MaxDatBF.kForcePreShade);    }
    void            SetForcePreShade(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kForcePreShade);       }

    bool            GetNoShadow()                       {return MaxDatBF.CanBF(MaxDatBF.kNoShadow);     }
    void            SetNoShadow(bool b)               { MaxDatBF.SetBF(b, MaxDatBF.kNoShadow);        }

    bool            GetForceShadow()                    {return MaxDatBF.CanBF(MaxDatBF.kForceShadow);  }
    void            SetForceShadow(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kForceShadow);     }

    bool            GetAlphaTestHigh()                  {return MaxDatBF.CanBF(MaxDatBF.kAlphaTestHigh);    }
    void            SetAlphaTestHigh(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kAlphaTestHigh);       }

    bool            GetFilterInherit()                  {return MaxDatBF.CanBF(MaxDatBF.kFilterInherit);    }
    void            SetFilterInherit(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kFilterInherit);       }

    bool            GetIsBarney()                       {return MaxDatBF.CanBF(MaxDatBF.kIsBarney);     }
    void            SetIsBarney(bool b)               { MaxDatBF.SetBF(b, MaxDatBF.kIsBarney);        }

    bool            GetNoSpanSort()                     {return MaxDatBF.CanBF(MaxDatBF.kNoSpanSort);   }
    void            SetNoSpanSort(bool b)             { MaxDatBF.SetBF(b, MaxDatBF.kNoSpanSort);      }

    bool            GetNoSpanReSort()                   {return MaxDatBF.CanBF(MaxDatBF.kNoSpanReSort); }
    void            SetNoSpanReSort(bool b)           { MaxDatBF.SetBF(b, MaxDatBF.kNoSpanReSort);    }

    bool            GetNoFaceSort()                     {return MaxDatBF.CanBF(MaxDatBF.kNoFaceSort);   }
    void            SetNoFaceSort(bool b)             { MaxDatBF.SetBF(b, MaxDatBF.kNoFaceSort);      }

    bool            GetReverseSort()                    {return MaxDatBF.CanBF(MaxDatBF.kReverseSort);  }
    void            SetReverseSort(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kReverseSort);     }

    bool            GetSortAsOpaque()                   {return MaxDatBF.CanBF(MaxDatBF.kSortAsOpaque); }
    void            SetSortAsOpaque(bool b)           { MaxDatBF.SetBF(b, MaxDatBF.kSortAsOpaque);        }

    bool            GetVS()                             {return MaxDatBF.CanBF(MaxDatBF.kVS);   }
    void            SetVS(bool b)                     { MaxDatBF.SetBF(b, MaxDatBF.kVS);      }

    bool            GetHasWaterHeight()                 { return MaxDatBF.CanBF(MaxDatBF.kWaterHeight); }
    void            SetHasWaterHeight(bool b)         { MaxDatBF.SetBF(b, MaxDatBF.kWaterHeight); }
    float        GetWaterHeight()                    { return fWaterHeight; }
    void            SetWaterHeight(float f)          { SetHasWaterHeight(true); fWaterHeight = f; }

    bool            GetSmoothAll()                      {return MaxDatBF.CanBF(MaxDatBF.kSmoothAll);    }
    void            SetSmoothAll(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kSmoothAll);       }

    bool            GetForceSortable()                  {return MaxDatBF.CanBF(MaxDatBF.kForceSortable);    }
    void            SetForceSortable(bool b)          { MaxDatBF.SetBF(b, MaxDatBF.kForceSortable);       }

    bool            GetConcave()                        {return MaxDatBF.CanBF(MaxDatBF.kConcave);  }
    void            SetConcave(bool b)                { MaxDatBF.SetBF(b, MaxDatBF.kConcave);     }

    bool            GetCalcEdgeLens()                   {return MaxDatBF.CanBF(MaxDatBF.kCalcEdgeLens); }
    void            SetCalcEdgeLens(bool b)           { MaxDatBF.SetBF(b, MaxDatBF.kCalcEdgeLens);    }

    bool            GetNoDeferDraw()                    {return MaxDatBF.CanBF(MaxDatBF.kNoDeferDraw);  }
    void            SetNoDeferDraw(bool b)            { MaxDatBF.SetBF(b, MaxDatBF.kNoDeferDraw);     }

    bool            GetBlendToFB()                      {return MaxDatBF.CanBF(MaxDatBF.kBlendToFB);    }
    void            SetBlendToFB(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kBlendToFB);       }

    bool            GetForceMaterialCopy()              {return MaxDatBF.CanBF(MaxDatBF.kForceMatCopy); }
    void            SetForceMaterialCopy(bool b)      { MaxDatBF.SetBF(b, MaxDatBF.kForceMatCopy);    }

    bool            GetInstanced()                      {return MaxDatBF.CanBF(MaxDatBF.kIsInstanced);  }
    void            SetInstanced(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kIsInstanced);     }

    bool            GetParticleRelated()                {return MaxDatBF.CanBF(MaxDatBF.kParticleRelated);  }
    void            SetParticleRelated(bool b)        { MaxDatBF.SetBF(b, MaxDatBF.kParticleRelated);     }

    bool            GetDup2Sided()                      {return MaxDatBF.CanBF(MaxDatBF.kDup2Sided);    }
    void            SetDup2Sided(bool b)              { MaxDatBF.SetBF(b, MaxDatBF.kDup2Sided);       }

    bool            GetRadiateNorms()                   {return MaxDatBF.CanBF(MaxDatBF.kRadiateNorms); }
    void            SetRadiateNorms(bool b)           { MaxDatBF.SetBF(b, MaxDatBF.kRadiateNorms);    }

    bool            GetGeoDice()                        {return MaxDatBF.CanBF(MaxDatBF.kGeoDice);      }
    void            SetGeoDice(bool b)                { MaxDatBF.SetBF(b, MaxDatBF.kGeoDice);         }

    bool            GetIsGUI()                          {return MaxDatBF.CanBF(MaxDatBF.kIsGUI);        }
    void            SetIsGUI(bool b)                  { MaxDatBF.SetBF(b, MaxDatBF.kIsGUI);           }

    plSharedMesh*   GetSwappableGeom()                  { return fSwapMesh;}
    void            SetSwappableGeom(plSharedMesh *sm)  { fSwapMesh = sm;   }

    uint32_t          GetSwappableGeomTarget()            { return fSwapTargetID;}
    void            SetSwappableGeomTarget(uint32_t id)   { fSwapTargetID = id;}

    plMaxBoneMap*   GetBoneMap()                        { return fBoneMap;}
    void            SetBoneMap(plMaxBoneMap *bones)     { fBoneMap = bones;}

    int             GetGeoDiceMaxFaces()                { return fGDMaxFaces; }
    void            SetGeoDiceMaxFaces(int f)           { fGDMaxFaces = f; }

    float           GetGeoDiceMaxSize()                 { return fGDMaxSize; }
    void            SetGeoDiceMaxSize(float f)          { fGDMaxSize = f; }

    int             GetGeoDiceMinFaces()                { return fGDMinFaces; }
    void            SetGeoDiceMinFaces(int f)           { fGDMinFaces = f; }

    plKey           GetRoomKey()                        { return fpRoomKey; }
    void            SetRoomKey(plKey p)                 { fpRoomKey = p; }
    
    uint32_t          GetDecalLevel()                     { return fDecalLevel; }
    void            SetDecalLevel(uint32_t i)             { fDecalLevel = i; }

    uint32_t          GetSoundIdxCounter()                { return fSoundIdxCounter; }
    void            SetSoundIdxCounter( uint32_t i )      { fSoundIdxCounter = i; }

    plSceneObject * GetAvatarSO()                       { return fAvatarSO; }
    void            SetAvatarSO(plSceneObject *so)      { fAvatarSO = so; }
    
    int             NumRenderDependencies()             { return fRenderDependencies.Count(); }
    plMaxNodeBase*  GetRenderDependency(int i)          { return fRenderDependencies[i]; }
    void            AddRenderDependency(plMaxNodeBase* m)   { fRenderDependencies.Append(1, &m); }
    void            ClearRenderDependencies()           { fRenderDependencies.ZeroCount(); }

    int             NumBones()                          { return fBones.Count(); }
    plMaxNodeBase*  GetBone(int i)                      { return fBones[i]; }
    void            AddBone(plMaxNodeBase* m)           { fBones.Append(1, &m); }
    void            ClearBones()                        { fBones.ZeroCount(); }

    bool            HasFade()                           { return MaxDatBF.CanBF(MaxDatBF.kHasFade); }
    void            SetFade(const Box3& b)              { MaxDatBF.SetBF((b.Min()[2] != 0)||(b.Max()[2] != 0), MaxDatBF.kHasFade); fFade = b; }
    Box3            GetFade()                           { return fFade; }

    bool            HasLoadMask()                       { return MaxDatBF.CanBF(MaxDatBF.kHasLoadMask); }
    plLoadMask      GetLoadMask()                       { return HasLoadMask() ? fLoadMask : plLoadMask::kAlways; }
    void            AddLoadMask(const plLoadMask& m)    { if( !HasLoadMask() ) { fLoadMask = m; MaxDatBF.SetBF(true, MaxDatBF.kHasLoadMask); }else{ fLoadMask |= m; } }

    bool            HasNormalChan()                     { return fNormalChan > 0; }
    void            SetNormalChan(int n)                { fNormalChan = n; }
    int             GetNormalChan()                     { return fNormalChan; }

    bool            BlendLevelSet()                     { return MaxDatBF.CanBF(MaxDatBF.kBLevelSet); }
    void            SetBlendLevel(const plRenderLevel& l) { fBlendLevel = l; MaxDatBF.SetBF(true, MaxDatBF.kBLevelSet); }
    const plRenderLevel&    GetBlendLevel()             { return fBlendLevel; }
    bool            OpaqueLevelSet()                    { return MaxDatBF.CanBF(MaxDatBF.kOLevelSet); }
    void            SetOpaqueLevel(const plRenderLevel& l) { fOpaqueLevel = l; MaxDatBF.SetBF(true, MaxDatBF.kOLevelSet); }
    const plRenderLevel&    GetOpaqueLevel()            { return fOpaqueLevel; }

    plPhysicalProps* GetPhysicalProps()                 { return &fPhysicalProps; }

    std::vector<int>* GetAlphaHackLayersCache()         { return fCachedAlphaHackLayerCounts; }
    void            SetAlphaHackLayersCache(std::vector<int>* cache) { fCachedAlphaHackLayerCounts = cache; }
    bool            GetOverrideHighLevelSDL()           { return MaxDatBF.CanBF(MaxDatBF.kOverrideHighLevelSDL); }
    void            SetOverrideHighLevelSDL(bool b)   { MaxDatBF.SetBF(b, MaxDatBF.kOverrideHighLevelSDL); }
    uint8_t           GetAnimCompress()                   { return fAnimCompression; }
    void            SetAnimCompress(uint8_t v)            { fAnimCompression = v; }
    float        GetKeyReduceThreshold()             { return fKeyReduceThreshold; }
    void            SetKeyReduceThreshold(float v)   { fKeyReduceThreshold = v; }

protected:
    plKey           fpKey;
    plSceneObject * fpSO;
    // hacking this in here temporarily because everything's about to change in the mesh world...
    hsGMesh*        fpMesh;
    plKey           fpRoomKey;
    uint32_t          fDecalLevel;
    int32_t           fNormalChan;
    uint32_t          fSoundIdxCounter;
    plSceneObject * fAvatarSO;
    plMaxNodeBaseTab    fRenderDependencies;
    plMaxNodeBaseTab    fBones;
    Box3            fFade;
    int             fGDMaxFaces;
    float           fGDMaxSize;
    int             fGDMinFaces;
    plRenderLevel   fBlendLevel;
    plRenderLevel   fOpaqueLevel;
    plPhysicalProps fPhysicalProps;
    uint32_t          fSwapTargetID;
    std::vector<int>* fCachedAlphaHackLayerCounts;
    plSharedMesh    *fSwapMesh;
    plMaxBoneMap    *fBoneMap;
    plLoadMask      fLoadMask;
    float        fWaterHeight;
    uint8_t           fAnimCompression;
    float        fKeyReduceThreshold;
    DataBF  MaxDatBF;
};


#endif // plSimpleConvert_inc
