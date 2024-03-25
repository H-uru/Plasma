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

#ifndef plAnimPath_inc
#define plAnimPath_inc

#include <vector>

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "plTransform/hsAffineParts.h"
#include "pnFactory/plCreatable.h"

class plCompoundController;

class plAnimPath : public plCreatable
{
public:
    enum Flags
    {
        kNone           = 0x0,
        kFavorFwdSearch = 0x1,      // only move fwd on the curve when searching
        kFavorBwdSearch = 0x2,      // only move bwd on the curve when searching
        kCalcPosOnly    = 0x4,      // only compute pos when calling SetCurTime()
        kFarthest       = 0x8,
        kWrap           = 0x10,
        kIncrement      = 0x20,     // find the nearest / farthest point, but increment toward it
    };
protected:
    // The final product info
    hsMatrix44                  fXform;
    hsPoint3                    fPos;
    hsVector3                   fVel;
    hsVector3                   fAccel;
    float                    fTime; // presumably seconds

    // The paramters (and options) for this curve.
    uint32_t                      fAnimPathFlags;     // currently set at runtime only
    float                    fMinDistSq;
    float                    fLength; // presumably seconds

    // Controller stuff only works in local space.
    hsMatrix44                  fLocalToWorld;
    hsMatrix44                  fWorldToLocal;

    // Bounding sphere available for ignoring out of range
    hsPoint3                    fCenter;
    float                    fRadius;

    plCompoundController*       fController;

    hsAffineParts               fParts;

    // These are temps during a search. They're here to avoid recalc.
    mutable float                    fLastTime;
    mutable float                    fLastDistSq;
    mutable float                    fThisTime;
    mutable float                    fThisDistSq;
    mutable float                    fNextTime;
    mutable float                    fNextDistSq;
    mutable float                    fDelTime;
    mutable hsPoint3                    fPrevPos, fCurPos;

    void                        ICalcBounds();
    float                    ICalcTotalLength();
    float                    IShiftFore(hsPoint3 &pt) const;
    float                    IShiftBack(hsPoint3 &pt) const;
    float                    ISubDivFore(hsPoint3 &pt) const;
    float                    ISubDivBack(hsPoint3 &pt) const;
    void                        IInitInterval(float time, float delTime, hsPoint3 &pt) const;
    float                    ICheckInterval(hsPoint3 &pt) const;
    float                    IBestTime() const { return fLastDistSq < fThisDistSq 
                                                        ? (fLastDistSq < fNextDistSq 
                                                            ? fLastTime
                                                            : fNextTime)
                                                        : (fThisDistSq < fNextDistSq
                                                            ? fThisTime
                                                            : fNextTime); }

    // Visualization helper
    void IMakeSegment(std::vector<uint16_t>& idx, std::vector<hsPoint3>& pos,
                      hsPoint3& p1, hsPoint3& p2);
    
    // For computing arclen
    struct ArcLenDeltaInfo
    {
        float    fT;
        float    fArcLenDelta;   // arc len distance from prev sample point (array entry)
        ArcLenDeltaInfo(float t, float del) : fT(t),fArcLenDelta(del) {}
        ArcLenDeltaInfo() : fT(), fArcLenDelta() { }
    };
    std::vector<ArcLenDeltaInfo> fArcLenDeltas;
public:
    plAnimPath();
    virtual ~plAnimPath();

    CLASSNAME_REGISTER( plAnimPath );
    GETINTERFACE_ANY( plAnimPath, plCreatable );
    
    void Reset();

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
    const hsMatrix44& GetLocalToWorld() const { return fLocalToWorld; }
    const hsMatrix44& GetWorldToLocal() const { return fWorldToLocal; }

    // Visualization helper
    void MakeDrawList(std::vector<uint16_t>& idx, std::vector<hsPoint3>& pos);

    void SetAnimPathFlags(uint32_t f) { fAnimPathFlags=f; }
    uint32_t GetAnimPathFlags() const { return fAnimPathFlags; }

    void SetWrap(bool on) { if(on)fAnimPathFlags |= kWrap; else fAnimPathFlags &= ~kWrap; }
    bool GetWrap() const { return 0 != (fAnimPathFlags & kWrap); }

    void SetFarthest(bool on) { if(on)fAnimPathFlags |= kFarthest; else fAnimPathFlags &= ~kFarthest; }
    bool GetFarthest() const { return 0 != (fAnimPathFlags & kFarthest); }

    void SetCurTime(float t, uint32_t calcFlags=0);
    float GetCurTime() const { return fTime; }

    void SetController(plCompoundController* tmc);
    plCompoundController* GetController() const { return fController; }
    float GetLength() const { return fLength; } // seconds

    void SetMinDistance(float d) { fMinDistSq = d*d; }
    float GetMinDistance() const;

    hsMatrix44* GetMatrix44(hsMatrix44* xOut) const { *xOut = fXform; return xOut; }
    hsPoint3*   GetPosition(hsPoint3* pOut) const { *pOut = fPos; return pOut; }
    hsVector3*  GetVelocity(hsVector3* vOut) const { *vOut = fVel; return vOut; }
    hsVector3*  GetDirection(hsVector3* dOut) const { dOut->Set(fXform.fMap[0][2], fXform.fMap[1][2], fXform.fMap[2][2]); return dOut; }
    hsVector3*  GetUp(hsVector3* uOut) const { uOut->Set(fXform.fMap[0][1], fXform.fMap[1][1], fXform.fMap[2][1]); return uOut; }
    hsVector3*  GetAcceleration(hsVector3* aOut) const { *aOut = fAccel; return aOut; }
    
    bool OutOfRange(hsPoint3 &pt, float range) const;
    const hsAffineParts* Parts() const { return &fParts; }
    void InitParts(const hsAffineParts& p) { fParts = p; }

    float GetExtremePoint(hsPoint3 &worldPt) const; // Exhaustive search
    float GetExtremePoint(float lastTime, float delTime, hsPoint3 &worldPt) const; // Incremental search

    // for arclen usage
    void ComputeArcLenDeltas(int32_t numSamples=256);
    float GetLookAheadTime(float startTime, float arcLength, bool bwd, int32_t* startSrchIdx);

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;
};

#endif //plAnimPath_inc
