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
#include "plController.h"
#include "hsInterp.h"
#include "hsResMgr.h"

#include "plTransform/hsEuler.h"
#include "plAnimTimeConvert.h"

/////////////////////////////////////////////
// Controller interp caching
/////////////////////////////////////////////

static const char *kInvalidInterpString = "Invalid call to plController::Interp()";

plControllerCacheInfo::plControllerCacheInfo() : fNumSubControllers(0), fSubControllers(nil), fKeyIndex(0), fAtc(nil) {}

plControllerCacheInfo::~plControllerCacheInfo()
{
    int i;
    for (i = 0; i < fNumSubControllers; i++)
        delete fSubControllers[i];

    delete [] fSubControllers;
}

void plControllerCacheInfo::SetATC(plAnimTimeConvert *atc)
{
    fAtc = atc;
    int i;
    for (i = 0; i < fNumSubControllers; i++)
        if (fSubControllers[i])
            fSubControllers[i]->SetATC(atc);
}

//////////////////////////////////////////////////////////////////////////////////////////

plLeafController::~plLeafController()
{
    delete[] fKeys;
}

void plLeafController::Interp(float time, float* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kScalarKeyFrame || fType == hsKeyFrame::kBezScalarKeyFrame, kInvalidInterpString);
    
    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    if (fType == hsKeyFrame::kScalarKeyFrame)
    {
        hsScalarKey *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsScalarKey), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::LinInterp(k1->fValue, k2->fValue, t, result);
    }
    else
    {
        hsBezScalarKey *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsBezScalarKey), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::BezInterp(k1, k2, t, result);
    }
}

void plLeafController::Interp(float time, hsScalarTriple* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kPoint3KeyFrame || fType == hsKeyFrame::kBezPoint3KeyFrame, kInvalidInterpString);

    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    if (fType == hsKeyFrame::kPoint3KeyFrame)
    {
        hsPoint3Key *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsPoint3Key), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::LinInterp(&k1->fValue, &k2->fValue, t, result);
    }
    else
    {
        hsBezPoint3Key *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsBezPoint3Key), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::BezInterp(k1, k2, t, result);
    }
}

void plLeafController::Interp(float time, hsScaleValue* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kScaleKeyFrame || fType == hsKeyFrame::kBezScaleKeyFrame, kInvalidInterpString);

    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    if (fType == hsKeyFrame::kScaleKeyFrame)
    {
        hsScaleKey *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsScaleKey), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::LinInterp(&k1->fValue, &k2->fValue, t, result);
    }
    else
    {
        hsBezScaleKey *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsBezScaleKey), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::BezInterp(k1, k2, t, result);
    }
}

void plLeafController::Interp(float time, hsQuat* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kQuatKeyFrame || 
             fType == hsKeyFrame::kCompressedQuatKeyFrame32 ||
             fType == hsKeyFrame::kCompressedQuatKeyFrame64, kInvalidInterpString);

    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    if (fType == hsKeyFrame::kQuatKeyFrame)
    {
        hsQuatKey *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsQuatKey), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
        hsInterp::LinInterp(&k1->fValue, &k2->fValue, t, result);
    }
    else if (fType == hsKeyFrame::kCompressedQuatKeyFrame32)
    {
        hsCompressedQuatKey32 *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsCompressedQuatKey32), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);

        hsQuat q1, q2;
        k1->GetQuat(q1);
        k2->GetQuat(q2);
        hsInterp::LinInterp(&q1, &q2, t, result);
    }
    else // (fType == hsKeyFrame::kCompressedQuatKeyFrame64)
    {
        hsCompressedQuatKey64 *k1, *k2;
        float t;
        uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
        hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsCompressedQuatKey64), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);

        hsQuat q1, q2;
        k1->GetQuat(q1);
        k2->GetQuat(q2);
        hsInterp::LinInterp(&q1, &q2, t, result);
    }
}

void plLeafController::Interp(float time, hsMatrix33* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kMatrix33KeyFrame, kInvalidInterpString);

    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    hsMatrix33Key *k1, *k2;
    float t;
    uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
    hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsMatrix33Key), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
    hsInterp::LinInterp(&k1->fValue, &k2->fValue, t, result);
}

void plLeafController::Interp(float time, hsMatrix44* result, plControllerCacheInfo *cache) const
{
    hsAssert(fType == hsKeyFrame::kMatrix44KeyFrame, kInvalidInterpString);

    hsBool tryForward = (cache? cache->fAtc->IsForewards() : true);
    hsMatrix44Key *k1, *k2;
    float t;
    uint32_t *idxStore = (cache ? &cache->fKeyIndex : &fLastKeyIdx);
    hsInterp::GetBoundaryKeyFrames(time, fNumKeys, fKeys, sizeof(hsMatrix44Key), (hsKeyFrame**)&k1, (hsKeyFrame**)&k2, idxStore, &t, tryForward);
    hsInterp::LinInterp(&k1->fValue, &k2->fValue, t, result);
}

void plLeafController::Interp(float time, hsColorRGBA* result, plControllerCacheInfo *cache) const
{
    hsPoint3 value;
    Interp(time, &value, cache);
    result->r = value.fX;
    result->g = value.fY;
    result->b = value.fZ;
}

plControllerCacheInfo *plLeafController::CreateCache() const
{
    plControllerCacheInfo *cache = new plControllerCacheInfo;
    cache->fNumSubControllers = 0;
    return cache;
}

float plLeafController::GetLength() const
{
    uint32_t stride = GetStride();
    if (stride == 0 || fNumKeys == 0)
        return 0;

    uint8_t *ptr = (uint8_t *)fKeys;
    return ((hsKeyFrame *)(ptr + (fNumKeys - 1) * stride))->fFrame / MAX_FRAMES_PER_SEC; 
}

uint32_t plLeafController::GetStride() const
{
    switch (fType)
    {
    case hsKeyFrame::kPoint3KeyFrame:
        return sizeof(hsPoint3Key);
    case hsKeyFrame::kBezPoint3KeyFrame:
        return sizeof(hsBezPoint3Key);
    case hsKeyFrame::kScalarKeyFrame:
        return sizeof(hsScalarKey);
    case hsKeyFrame::kBezScalarKeyFrame:
        return sizeof(hsBezScalarKey);
    case hsKeyFrame::kScaleKeyFrame:
        return sizeof(hsScaleKey);
    case hsKeyFrame::kBezScaleKeyFrame:
        return sizeof(hsBezScaleKey);
    case hsKeyFrame::kQuatKeyFrame:
        return sizeof(hsQuatKey);
    case hsKeyFrame::kCompressedQuatKeyFrame32:
        return sizeof(hsCompressedQuatKey32);
    case hsKeyFrame::kCompressedQuatKeyFrame64:
        return sizeof(hsCompressedQuatKey64);
    case hsKeyFrame::k3dsMaxKeyFrame:
        return sizeof(hsG3DSMaxKeyFrame);
    case hsKeyFrame::kMatrix33KeyFrame:
        return sizeof(hsMatrix33Key);
    case hsKeyFrame::kMatrix44KeyFrame:
        return sizeof(hsMatrix44Key);
    case hsKeyFrame::kUnknownKeyFrame:
    default:
        return 0;
    }
}

hsPoint3Key *plLeafController::GetPoint3Key(uint32_t i) const
{
    if (fType != hsKeyFrame::kPoint3KeyFrame)
        return nil;

    return (hsPoint3Key *)((uint8_t *)fKeys + i * sizeof(hsPoint3Key));
}

hsBezPoint3Key *plLeafController::GetBezPoint3Key(uint32_t i) const
{
    if (fType != hsKeyFrame::kBezPoint3KeyFrame)
        return nil;

    return (hsBezPoint3Key *)((uint8_t *)fKeys + i * sizeof(hsBezPoint3Key));
}

hsScalarKey *plLeafController::GetScalarKey(uint32_t i) const
{
    if (fType != hsKeyFrame::kScalarKeyFrame)
        return nil;

    return (hsScalarKey *)((uint8_t *)fKeys + i * sizeof(hsScalarKey));
}

hsBezScalarKey *plLeafController::GetBezScalarKey(uint32_t i) const
{
    if (fType != hsKeyFrame::kBezScalarKeyFrame)
        return nil;

    return (hsBezScalarKey *)((uint8_t *)fKeys + i * sizeof(hsBezScalarKey));
}

hsScaleKey *plLeafController::GetScaleKey(uint32_t i) const
{
    if (fType != hsKeyFrame::kScaleKeyFrame)
        return nil;

    return (hsScaleKey *)((uint8_t *)fKeys + i * sizeof(hsScaleKey));
}

hsBezScaleKey *plLeafController::GetBezScaleKey(uint32_t i) const
{
    if (fType != hsKeyFrame::kBezScaleKeyFrame)
        return nil;

    return (hsBezScaleKey *)((uint8_t *)fKeys + i * sizeof(hsBezScaleKey));
}

hsQuatKey *plLeafController::GetQuatKey(uint32_t i) const
{
    if (fType != hsKeyFrame::kQuatKeyFrame)
        return nil;

    return (hsQuatKey *)((uint8_t *)fKeys + i * sizeof(hsQuatKey));
}

hsCompressedQuatKey32 *plLeafController::GetCompressedQuatKey32(uint32_t i) const
{
    if (fType != hsKeyFrame::kCompressedQuatKeyFrame32)
        return nil;

    return (hsCompressedQuatKey32 *)((uint8_t *)fKeys + i * sizeof(hsCompressedQuatKey32));
}

hsCompressedQuatKey64 *plLeafController::GetCompressedQuatKey64(uint32_t i) const
{
    if (fType != hsKeyFrame::kCompressedQuatKeyFrame64)
        return nil;

    return (hsCompressedQuatKey64 *)((uint8_t *)fKeys + i * sizeof(hsCompressedQuatKey64));
}

hsG3DSMaxKeyFrame *plLeafController::Get3DSMaxKey(uint32_t i) const
{
    if (fType != hsKeyFrame::k3dsMaxKeyFrame)
        return nil;

    return (hsG3DSMaxKeyFrame *)((uint8_t *)fKeys + i * sizeof(hsG3DSMaxKeyFrame));
}

hsMatrix33Key *plLeafController::GetMatrix33Key(uint32_t i) const
{
    if (fType != hsKeyFrame::kMatrix33KeyFrame)
        return nil;

    return (hsMatrix33Key *)((uint8_t *)fKeys + i * sizeof(hsMatrix33Key));
}

hsMatrix44Key *plLeafController::GetMatrix44Key(uint32_t i) const
{
    if (fType != hsKeyFrame::kMatrix44KeyFrame)
        return nil;

    return (hsMatrix44Key *)((uint8_t *)fKeys + i * sizeof(hsMatrix44Key));
}

void plLeafController::GetKeyTimes(hsTArray<float> &keyTimes) const
{
    int cIdx;
    int kIdx;
    uint32_t stride = GetStride();
    uint8_t *keyPtr = (uint8_t *)fKeys;
    for (cIdx = 0, kIdx = 0; cIdx < fNumKeys, kIdx < keyTimes.GetCount();)
    {
        float kTime = keyTimes[kIdx];
        float cTime = ((hsKeyFrame*)(keyPtr + cIdx * stride))->fFrame / MAX_FRAMES_PER_SEC;
        if (cTime < kTime)
        {
            keyTimes.InsertAtIndex(kIdx, cTime);
            cIdx++;
            kIdx++;
        }
        else if (cTime > kTime)
        {
            kIdx++;
        }
        else
        {
            kIdx++;
            cIdx++;
        }
    }

    // All remaining times in the controller are later than the original keyTimes set
    for (; cIdx < fNumKeys; cIdx++)
    {
        float cTime = ((hsKeyFrame*)(keyPtr + cIdx * stride))->fFrame / MAX_FRAMES_PER_SEC;
        keyTimes.Append(cTime);
    }
}

void plLeafController::AllocKeys(uint32_t numKeys, uint8_t type)
{
    delete fKeys;
    fNumKeys = numKeys;
    fType = type;

    switch (fType)
    {
    case hsKeyFrame::kPoint3KeyFrame:
        fKeys = new hsPoint3Key[fNumKeys];
        break;

    case hsKeyFrame::kBezPoint3KeyFrame:
        fKeys = new hsBezPoint3Key[fNumKeys];
        break;

    case hsKeyFrame::kScalarKeyFrame:
        fKeys = new hsScalarKey[fNumKeys];
        break;

    case hsKeyFrame::kBezScalarKeyFrame:
        fKeys = new hsBezScalarKey[fNumKeys];
        break;

    case hsKeyFrame::kScaleKeyFrame:
        fKeys = new hsScaleKey[fNumKeys];
        break;

    case hsKeyFrame::kBezScaleKeyFrame:
        fKeys = new hsBezScaleKey[fNumKeys];
        break;

    case hsKeyFrame::kQuatKeyFrame:
        fKeys = new hsQuatKey[fNumKeys];
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame32:
        fKeys = new hsCompressedQuatKey32[fNumKeys];
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame64:
        fKeys = new hsCompressedQuatKey64[fNumKeys];
        break;

    case hsKeyFrame::k3dsMaxKeyFrame:
        fKeys = new hsG3DSMaxKeyFrame[fNumKeys];
        break;

    case hsKeyFrame::kMatrix33KeyFrame:
        fKeys = new hsMatrix33Key[fNumKeys];
        break;

    case hsKeyFrame::kMatrix44KeyFrame:
        fKeys = new hsMatrix44Key[fNumKeys];
        break;

    case hsKeyFrame::kUnknownKeyFrame:
    default:
        hsAssert(false, "Trying to allocate unknown keyframe type");
        break;
    }
}

void plLeafController::QuickScalarController(int numKeys, float* times, float* values, uint32_t valueStrides)
{
    AllocKeys(numKeys, hsKeyFrame::kScalarKeyFrame);
    int i;
    for( i = 0; i < numKeys; i++ )
    {
        ((hsScalarKey*)fKeys)[i].fFrame = (uint16_t)(*times++ * MAX_FRAMES_PER_SEC);
        ((hsScalarKey*)fKeys)[i].fValue = *values;
        values = (float *)((uint8_t *)values + valueStrides);
    }
}

// If all the keys are the same, this controller is pretty useless.
// This situation actually comes up a lot because of the biped killer
// trying to convert character studio animations.
hsBool plLeafController::AllKeysMatch() const
{
    if (fNumKeys <= 1)
        return true;

    int idx;
    for (idx = 1; idx < fNumKeys; idx++)
    {
        switch (fType)
        {
        case hsKeyFrame::kPoint3KeyFrame:
            {
                hsPoint3Key *k1 = GetPoint3Key(idx - 1);
                hsPoint3Key *k2 = GetPoint3Key(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kBezPoint3KeyFrame:
            {
                hsBezPoint3Key *k1 = GetBezPoint3Key(idx - 1);
                hsBezPoint3Key *k2 = GetBezPoint3Key(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kScalarKeyFrame:
            {
                hsScalarKey *k1 = GetScalarKey(idx - 1);
                hsScalarKey *k2 = GetScalarKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kBezScalarKeyFrame:
            {
                hsBezScalarKey *k1 = GetBezScalarKey(idx - 1);
                hsBezScalarKey *k2 = GetBezScalarKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kScaleKeyFrame:
            {
                hsScaleKey *k1 = GetScaleKey(idx - 1);
                hsScaleKey *k2 = GetScaleKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kBezScaleKeyFrame:
            {
                hsBezScaleKey *k1 = GetBezScaleKey(idx - 1);
                hsBezScaleKey *k2 = GetBezScaleKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kQuatKeyFrame:
            {
                hsQuatKey *k1 = GetQuatKey(idx - 1);
                hsQuatKey *k2 = GetQuatKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kCompressedQuatKeyFrame32:
            {
                hsCompressedQuatKey32 *k1 = GetCompressedQuatKey32(idx - 1);
                hsCompressedQuatKey32 *k2 = GetCompressedQuatKey32(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kCompressedQuatKeyFrame64:
            {
                hsCompressedQuatKey64 *k1 = GetCompressedQuatKey64(idx - 1);
                hsCompressedQuatKey64 *k2 = GetCompressedQuatKey64(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::k3dsMaxKeyFrame:
            {
                hsG3DSMaxKeyFrame *k1 = Get3DSMaxKey(idx - 1);
                hsG3DSMaxKeyFrame *k2 = Get3DSMaxKey(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kMatrix33KeyFrame:
            {
                hsMatrix33Key *k1 = GetMatrix33Key(idx - 1);
                hsMatrix33Key *k2 = GetMatrix33Key(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kMatrix44KeyFrame:
            {
                hsMatrix44Key *k1 = GetMatrix44Key(idx - 1);
                hsMatrix44Key *k2 = GetMatrix44Key(idx);
                if (!k1->CompareValue(k2))
                    return false;
                break;
            }
        case hsKeyFrame::kUnknownKeyFrame:
        default:
            hsAssert(false, "Trying to compare unknown keyframe type");
            return false;
        }
    }
    return true;
}

hsBool plLeafController::PurgeRedundantSubcontrollers()
{
    return AllKeysMatch();
}

void plLeafController::Read(hsStream* s, hsResMgr *mgr)
{
    uint8_t type = s->ReadByte();
    uint32_t numKeys = s->ReadLE32();
    AllocKeys(numKeys, type);

    int i;
    switch (fType)
    {
    case hsKeyFrame::kPoint3KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsPoint3Key *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kBezPoint3KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezPoint3Key *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kScalarKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsScalarKey *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kBezScalarKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezScalarKey *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kScaleKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsScaleKey *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kBezScaleKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezScaleKey *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kQuatKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsQuatKey *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame32:
        for (i = 0; i < fNumKeys; i++)
            ((hsCompressedQuatKey32 *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame64:
        for (i = 0; i < fNumKeys; i++)
            ((hsCompressedQuatKey64 *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::k3dsMaxKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsG3DSMaxKeyFrame *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kMatrix33KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsMatrix33Key *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kMatrix44KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsMatrix44Key *)fKeys)[i].Read(s);
        break;

    case hsKeyFrame::kUnknownKeyFrame:
    default:
        hsAssert(false, "Reading in controller with unknown key data");
        break;
    }
}

void plLeafController::Write(hsStream* s, hsResMgr *mgr)
{
    s->WriteByte(fType);
    s->WriteLE32(fNumKeys);

    int i;
    switch (fType)
    {
    case hsKeyFrame::kPoint3KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsPoint3Key *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kBezPoint3KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezPoint3Key *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kScalarKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsScalarKey *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kBezScalarKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezScalarKey *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kScaleKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsScaleKey *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kBezScaleKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsBezScaleKey *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kQuatKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsQuatKey *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame32:
        for (i = 0; i < fNumKeys; i++)
            ((hsCompressedQuatKey32 *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kCompressedQuatKeyFrame64:
        for (i = 0; i < fNumKeys; i++)
            ((hsCompressedQuatKey64 *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::k3dsMaxKeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsG3DSMaxKeyFrame *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kMatrix33KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsMatrix33Key *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kMatrix44KeyFrame:
        for (i = 0; i < fNumKeys; i++)
            ((hsMatrix44Key *)fKeys)[i].Write(s);
        break;

    case hsKeyFrame::kUnknownKeyFrame:
    default:
        hsAssert(false, "Writing controller with unknown key data");
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////

plCompoundController::plCompoundController() : fXController(nil), fYController(nil), fZController(nil) {}

plCompoundController::~plCompoundController()
{
    delete fXController;
    delete fYController;
    delete fZController;
}

void plCompoundController::Interp(float time, hsScalarTriple* result, plControllerCacheInfo *cache) const
{
    if (fXController)
        fXController->Interp(time, &result->fX, (cache ? cache->fSubControllers[0] : nil));
    if (fYController)
        fYController->Interp(time, &result->fY, (cache ? cache->fSubControllers[1] : nil));
    if (fZController)
        fZController->Interp(time, &result->fZ, (cache ? cache->fSubControllers[2] : nil));
}

void plCompoundController::Interp(float time, hsQuat* result, plControllerCacheInfo *cache) const
{
    hsEuler eul(0,0,0,EulOrdXYZs);

    fXController->Interp(time, &eul.fX, (cache ? cache->fSubControllers[0] : nil));
    fYController->Interp(time, &eul.fY, (cache ? cache->fSubControllers[1] : nil));
    fZController->Interp(time, &eul.fZ, (cache ? cache->fSubControllers[2] : nil));

    eul.GetQuat(result);
}

void plCompoundController::Interp(float time, hsAffineParts* parts, plControllerCacheInfo *cache) const
{
    if (fXController)
        fXController->Interp(time, &parts->fT, (cache ? cache->fSubControllers[0] : nil));

    if (fYController)
        fYController->Interp(time, &parts->fQ, (cache ? cache->fSubControllers[1] : nil));

    hsScaleValue sv;
    if (fZController)
    {
        fZController->Interp(time, &sv, (cache ? cache->fSubControllers[2] : nil));
        parts->fU = sv.fQ;
        parts->fK = sv.fS;
    }
}

void plCompoundController::Interp(float time, hsColorRGBA* result, plControllerCacheInfo *cache) const
{
    fXController->Interp(time, &result->r, (cache ? cache->fSubControllers[0] : nil));
    fYController->Interp(time, &result->g, (cache ? cache->fSubControllers[1] : nil));
    fZController->Interp(time, &result->b, (cache ? cache->fSubControllers[2] : nil));
}

float plCompoundController::GetLength() const
{
    float len=0;
    int i;
    for(i=0; i<3; i++)
    {
        if (GetController(i))
            len = hsMaximum(len, GetController(i)->GetLength());
    }
    return len;
}

void plCompoundController::GetKeyTimes(hsTArray<float> &keyTimes) const
{
    if (fXController)
        fXController->GetKeyTimes(keyTimes);
    if (fYController)
        fYController->GetKeyTimes(keyTimes);
    if (fZController)
        fZController->GetKeyTimes(keyTimes);
}

hsBool plCompoundController::AllKeysMatch() const
{
    return (!fXController || fXController->AllKeysMatch()) &&
           (!fYController || fYController->AllKeysMatch()) &&
           (!fZController || fZController->AllKeysMatch());
}

// Careful here... We might detect that one of our subcontrollers
// has animation keys that all have the same value. That doesn't
// mean they're all zero though. An avatar animation might have
// elbow bend a constant 90 degrees through the entire anim, but
// if we delete the controller and assume zero, we'll have problems.
// Transform controller channels get around this by sampling the source
// first and using that to fill in the missing subcontrollers.
//
// Note: that one of our subcontrollers could itself be a compound
// controller. An example would be a controller for XYZ Euler angles
// that's a sub of the pos/rot/scale transform controller.
// It's possible that some of these sub-sub controllers could be
// removed, but then we'd have to store the default values somewhere.
// At the moment, this doesn't seem likely to save us enough space
// to be worth the effort. (This is why this function doesn't
// recursively call purge on its subcontrollers.)
hsBool plCompoundController::PurgeRedundantSubcontrollers()
{
    if (fXController && fXController->AllKeysMatch())
    {
        delete fXController;
        fXController = nil;
    }

    if (fYController && fYController->AllKeysMatch())
    {
        delete fYController;
        fYController = nil;
    }

    if (fZController && fZController->AllKeysMatch())
    {
        delete fZController;
        fZController = nil;
    }

    return (!fXController && !fYController && !fZController);
}

plControllerCacheInfo* plCompoundController::CreateCache() const
{
    plControllerCacheInfo* cache = new plControllerCacheInfo;
    cache->fNumSubControllers = 3;
    cache->fSubControllers = new plControllerCacheInfo*[cache->fNumSubControllers];
    int i;
    for (i = 0; i < cache->fNumSubControllers; i++)
        cache->fSubControllers[i] = (GetController(i) ? GetController(i)->CreateCache() : nil);

    return cache;
}

plController* plCompoundController::GetController(int32_t i) const
{ 
    return (i==0 ? fXController : (i==1 ? fYController : fZController)); 
}

void plCompoundController::SetController(int32_t i, plController* c) 
{ 
    delete GetController(i); 
    (i==0 ? fXController : (i==1 ? fYController : fZController)) = c; 
}

void plCompoundController::Read(hsStream* stream, hsResMgr *mgr)
{
    fXController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
    fYController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
    fZController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
}

void plCompoundController::Write(hsStream* stream, hsResMgr *mgr)
{
    mgr->WriteCreatable(stream, fXController);
    mgr->WriteCreatable(stream, fYController);
    mgr->WriteCreatable(stream, fZController);
}

