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

#ifndef plAccessVtxSpan_inc
#define plAccessVtxSpan_inc

#include "hsGeometry3.h"
#include "hsColorRGBA.h"

class hsGMaterial;
struct hsMatrix44;
class hsGDeviceRef;

// AccessSpan - a generic "vertex array". It'll at least act like one,
// whatever the underlying storage mechanism. For random access, just
// use the accessor functions. For faster sequential iteration, use
// the iterator classes defined further down. I'm leaving the actual
// data (fChannels & fStrides) public, but I'll probably regret it.
// I have to dig pretty deep to come up with a reason not to use
// either the indexed accessors or the iterator classes.
//
// You may be wondering why have separate strides for each of the data channels,
// especially since this means doing nChan pointer adds to advance rather than
// one. The answer is two part. First, the different channels may be in different
// buffers, either because we're on the export side and haven't glommed them together
// into a single stream yet, or because we finally got around to using multiple
// streams on the runtime side (say to support instancing). The point is for you
// to be able to write your code without knowing or caring. The second part is that
// those nChan adds aren't really costing you anything. Say the data is stored contiguously,
// and we keep a base pointer and an advance just adds the (single) stride to the base
// pointer. That's probably your position, and we've just saved a bunch of adds in advancing
// it. Now you want to access your normal, well, you still have to do an add on the base pointer
// to get to your normal, and another to get to your color, etc. So if you really want to 
// eliminate those extra adds, go ahead and use the iterator, just make sure you use
// the most focused iterator available. Like if you're just looking at position and
// normal, use the plAccPosNormIterator.
class plAccessVtxSpan
{
public:
    enum Channel
    {
        kPosition = 0,
        kWeight,
        kWgtIndex,
        kNormal,
        kDiffuse,
        kSpecular,
        kUVW,
        kNumValidChans,
        kInvalid = 0xffff
    };
    // Mask versions, to be or'ed together and passed in when using plAccessGeometry::SnapShotStuff.
    enum
    {
        kPositionMask   = (1 << kPosition),
        kWeightMask     = (1 << kWeight),
        kWgtIndexMask   = (1 << kWgtIndex),
        kNormalMask     = (1 << kNormal),
        kDiffuseMask    = (1 << kDiffuse),
        kSpecularMask   = (1 << kSpecular),
        kUVWMask        = (1 << kUVW)
    };

    hsGDeviceRef*   fVtxDeviceRef;

    uint8_t*          fChannels[kNumValidChans];
    uint16_t          fStrides[kNumValidChans];
    int32_t           fOffsets[kNumValidChans];

    uint16_t          fNumWeights;
    uint16_t          fNumUVWsPerVert;
    uint16_t          fNumVerts;

    //////////////////////////////////
    // QUERY SECTION
    // Queries on how much of what we got.
    uint32_t          VertCount() const { return fNumVerts; }
    bool            HasChannel(Channel chan) const { return fStrides[chan] > 0; }
    bool                HasPositions() const { return HasChannel(kPosition); }
    bool                HasWeights() const { return HasChannel(kWeight); }
    int                 NumWeights() const { return fNumWeights; }
    bool                HasWgtIndex() const { return HasChannel(kWgtIndex); }
    bool                HasNormals() const { return HasChannel(kNormal); }
    bool                HasDiffuse() const { return HasChannel(kDiffuse); }
    bool                HasSpecular() const { return HasChannel(kSpecular); }
    bool                HasUVWs() const { return HasChannel(kUVW); }
    bool                    HasUVWs(int n) { return HasChannel(kUVW) && (n <= fNumUVWsPerVert); }
    uint16_t            NumUVWs() const { return fNumUVWsPerVert; }

    //////////////////////////////////
    // ACCESSOR SECTION
    hsPoint3&       Position(int i) const { return *(hsPoint3*)(fChannels[kPosition] + i*fStrides[kPosition]); }
    hsVector3&      Normal(int i) const { return *(hsVector3*)(fChannels[kNormal] + i*fStrides[kNormal]); }

    float&       Weight(int iVtx, int iWgt) const { return *(float*)(fChannels[kWeight] + iVtx*fStrides[kWeight] + iWgt*sizeof(float)); }
    float*       Weights(int i) const { return (float*)(fChannels[kWeight] + i*fStrides[kWeight]); }
    uint32_t&         WgtIndices(int i) const { return *(uint32_t*)(fChannels[kWgtIndex] + i * fStrides[kWgtIndex]); }
    uint8_t&          WgtIndex(int iVtx, int iWgt) const { return *(fChannels[kWgtIndex] + iVtx*fStrides[kWgtIndex] + iWgt); }

    uint32_t&         Diffuse32(int i) const { return *(uint32_t*)(fChannels[kDiffuse] + i*fStrides[kDiffuse]); }
    uint32_t&         Specular32(int i) const { return *(uint32_t*)(fChannels[kSpecular] + i*fStrides[kSpecular]); }

    hsColorRGBA     DiffuseRGBA(int i) const { return hsColorRGBA().FromARGB32(Diffuse32(i)); }
    hsColorRGBA     SpecularRGBA(int i) const { return hsColorRGBA().FromARGB32(Specular32(i)); }

        // Two versions of UVW, first to get the iVtx'th vertex's iUVW'th uvw. 
        // Second just gets the vertex's uvw array, which is hopefully stored contiguously or we're screwed.
    hsPoint3&       UVW(int iVtx, int iUVW) const { return *(hsPoint3*)(fChannels[kUVW] + iVtx*fStrides[kUVW] + iUVW*sizeof(hsPoint3)); }
    hsPoint3*       UVWs(int i) const { return (hsPoint3*)(fChannels[kUVW] + i*fStrides[kUVW]); }

    // OFFSET VERSIONS
    // Tri and particle accessors call the offset versions because they have indices into the original
    // buffers, but our pointers are based at the beginning of our data.
    hsPoint3&       PositionOff(int i) const { return *(hsPoint3*)(fChannels[kPosition] + i*fStrides[kPosition] + fOffsets[kPosition]); }
    hsVector3&      NormalOff(int i) const { return *(hsVector3*)(fChannels[kNormal] + i*fStrides[kNormal] + fOffsets[kNormal]); }

    float&       WeightOff(int iVtx, int iWgt) const { return *(float*)(fChannels[kWeight] + iVtx*fStrides[kWeight] + fOffsets[kWeight] + iWgt*sizeof(float)); }
    float*       WeightsOff(int i) const { return (float*)(fChannels[kWeight] + i*fStrides[kWeight] + fOffsets[kWeight]); }
    uint32_t&    WgtIndicesOff(int i) const { return *(uint32_t*)(fChannels[kWgtIndex] + i * fStrides[kWgtIndex] + fOffsets[kWgtIndex]); }
    uint8_t&     WgtIndexOff(int iVtx, int iWgt) const { return *(fChannels[kWgtIndex] + iVtx*fStrides[kWgtIndex] + fOffsets[kWgtIndex] + iWgt); }

    uint32_t&       Diffuse32Off(int i) const { return *(uint32_t*)(fChannels[kDiffuse] + i*fStrides[kDiffuse] + fOffsets[kDiffuse]); }
    uint32_t&       Specular32Off(int i) const { return *(uint32_t*)(fChannels[kSpecular] + i*fStrides[kSpecular] + fOffsets[kSpecular]); }

    hsColorRGBA     DiffuseRGBAOff(int i) const { return hsColorRGBA().FromARGB32(Diffuse32Off(i)); }
    hsColorRGBA     SpecularRGBAOff(int i) const { return hsColorRGBA().FromARGB32(Specular32Off(i)); }

        // Two versions of UVW, first to get the iVtx'th vertex's iUVW'th uvw. 
        // Second just gets the vertex's uvw array, which is hopefully stored contiguously or we're screwed.
    hsPoint3&       UVWOff(int iVtx, int iUVW) const { return *(hsPoint3*)(fChannels[kUVW] + iVtx*fStrides[kUVW] + fOffsets[kUVW] + iUVW*sizeof(hsPoint3)); }
    hsPoint3*       UVWsOff(int i) const { return (hsPoint3*)(fChannels[kUVW] + i*fStrides[kUVW] + fOffsets[kUVW]); }
    
    //////////////////////////////////
    // SETUP SECTION.
    //////////////////////////////////

    //////////////////////////////////
    // Call Clear to initialize.
    void            ClearVerts();

    // Must at least set a count and the valid channels. Note below on setting up for UVW access.
    plAccessVtxSpan&    SetVertCount(uint16_t c) { fNumVerts = c; return *this; }
    plAccessVtxSpan&    SetStream(void* p, uint16_t stride, int32_t offset, Channel chan) { fChannels[chan] = (uint8_t*)p; fStrides[chan] = stride; fOffsets[chan] = offset; return *this; }

    //////////////////////////////////
    // Convenience versions. You get the idea.
    plAccessVtxSpan&    PositionStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kPosition); }
    plAccessVtxSpan&    NormalStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kNormal); }
    plAccessVtxSpan&    DiffuseStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kDiffuse); }
    plAccessVtxSpan&    SpecularStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kSpecular); }
    plAccessVtxSpan&    WeightStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kWeight); }
    plAccessVtxSpan&    WgtIndexStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kWgtIndex); }
    plAccessVtxSpan&    SetNumWeights(int n) { if (!(fNumWeights = n)) SetStream(nullptr, 0, 0, kWeight).SetStream(nullptr, 0, 0, kWgtIndex); return *this; }
    // Note on UVW access setup, you don't actually "have" to set the number of uvws per vertex,
    // but one way or another you'll need to know to access the uvws. If you're going to be setting
    // up the AccessSpan and passing it off for processing, you definitely better set it. But the
    // accessor and iterators don't ever use it.
    // Note that this means the UVWStream stride is from uvw[0][0] to uvw[1][0] in bytes.
    plAccessVtxSpan&    UVWStream(void* p, uint16_t stride, int32_t offset) { return SetStream(p, stride, offset, kUVW); }
    plAccessVtxSpan&    SetNumUVWs(int n) { if (!(fNumUVWsPerVert = n)) SetStream(nullptr, 0, 0, kUVW); return *this; }
    //////////////////////////////////

    // Cache away any device stuff that needed to be opened (locked) to get this data, so
    // we can close (unlock) it later.
    void            SetVtxDeviceRef(hsGDeviceRef* ref) { fVtxDeviceRef = ref; }
    hsGDeviceRef*   GetVtxDeviceRef() const { return fVtxDeviceRef; }
};

inline void plAccessVtxSpan::ClearVerts()
{
    int i;
    for( i = 0; i < kNumValidChans; i++ )
    {
        fChannels[i] = nullptr;
        fStrides[i] = 0;
    }
    fNumVerts = 0;
    fNumUVWsPerVert = 0;
    fNumWeights = 0;
}

template <class T> class plAccIterator
{
private:
    union
    {
        T*          fValue;
        uint8_t*    fValueByte;
    };
    uint8_t*                    fValueEnd;
    const plAccessVtxSpan*      fAccess;
    plAccessVtxSpan::Channel    fChan;

    void                        ISetEnd() { fValueEnd = fAccess->fChannels[fChan] + fAccess->fNumVerts * fAccess->fStrides[fChan]; }

public:
    plAccIterator(const plAccessVtxSpan* acc, plAccessVtxSpan::Channel chan) { Set(acc, chan); }
    plAccIterator(const plAccIterator& accIt) { *this = accIt; }
    plAccIterator() : fValueByte(), fValueEnd(), fAccess(), fChan(plAccessVtxSpan::kInvalid) { }

    void Set(const plAccessVtxSpan* acc, plAccessVtxSpan::Channel chan) { fAccess = acc; fChan = chan; ISetEnd(); }

    T*          Value() const { return fValue; }

    int         Count() const { return fAccess->VertCount(); }
    int         GetCount() const { return Count(); }

    void        Begin() { fValueByte = fAccess->fChannels[fChan]; }
    void        Advance() { fValueByte += fAccess->fStrides[fChan]; }
    bool        More() const { return fValueByte < fValueEnd; }
};

class plAccPositionIterator
{
protected:
    plAccIterator<hsPoint3>     fPosition;

public:
    plAccPositionIterator(const plAccessVtxSpan* acc)
        :   fPosition(acc, plAccessVtxSpan::kPosition) {}

    plAccPositionIterator() {}

    plAccPositionIterator& Set(const plAccessVtxSpan* acc)
    { 
        fPosition.Set(acc, plAccessVtxSpan::kPosition); 
        return *this; 
    }

    hsPoint3*           Position() const { return fPosition.Value(); }

    void                Begin() { fPosition.Begin(); }
    void                Advance() { fPosition.Advance(); }
    bool                More() const { return fPosition.More(); }
};

class plAccPosNormIterator
{
protected:
    plAccIterator<hsPoint3>     fPosition;
    plAccIterator<hsVector3>    fNormal;
public:
    plAccPosNormIterator(plAccessVtxSpan* acc) 
        :   fPosition(acc, plAccessVtxSpan::kPosition), 
            fNormal(acc, plAccessVtxSpan::kNormal) {}

    plAccPosNormIterator() {}

    plAccPosNormIterator& Set(plAccessVtxSpan* acc) 
    { 
        fPosition.Set(acc, plAccessVtxSpan::kPosition); 
        fNormal.Set(acc, plAccessVtxSpan::kNormal); 
        return *this; 
    }

    hsPoint3*           Position() const { return fPosition.Value(); }
    hsVector3*          Normal() const { return fNormal.Value(); }

    void                Begin() { fPosition.Begin(); fNormal.Begin(); }
    void                Advance() { fPosition.Advance(); fNormal.Advance(); }
    bool                More() const { return fPosition.More(); }
};

class plAccPosNormUVWIterator
{
protected:
    plAccIterator<hsPoint3>     fPosition;
    plAccIterator<hsVector3>    fNormal;
    plAccIterator<hsPoint3>     fUVW;
public:
    plAccPosNormUVWIterator(plAccessVtxSpan* acc) 
        :   fPosition(acc, plAccessVtxSpan::kPosition), 
            fNormal(acc, plAccessVtxSpan::kNormal), 
            fUVW(acc, plAccessVtxSpan::kUVW) {}
    plAccPosNormUVWIterator() {}

    plAccPosNormUVWIterator& Set(plAccessVtxSpan* acc) 
    { 
        fPosition.Set(acc, plAccessVtxSpan::kPosition); 
        fNormal.Set(acc, plAccessVtxSpan::kNormal); 
        fUVW.Set(acc, plAccessVtxSpan::kUVW); 
        return *this; 
    }

    hsPoint3*           Position() const { return fPosition.Value(); }
    hsVector3*          Normal() const { return fNormal.Value(); }
    hsPoint3*           UVWs() const { return fUVW.Value(); }
    hsPoint3*           UVW(int i) const { return fUVW.Value() + i; }

    void                Begin() { fPosition.Begin(); fNormal.Begin(); fUVW.Begin(); }
    void                Advance() { fPosition.Advance(); fNormal.Advance(); fUVW.Advance(); }
    bool                More() const { return fPosition.More(); }
};

class plAccUVWIterator
{
    plAccIterator<hsPoint3>     fUVW;
public:
    plAccUVWIterator(plAccessVtxSpan* acc) 
        :   fUVW(acc, plAccessVtxSpan::kUVW) {}
    plAccUVWIterator() {}

    plAccUVWIterator& Set(plAccessVtxSpan* acc) 
    { 
        fUVW.Set(acc, plAccessVtxSpan::kUVW); 
        return *this; 
    }

    hsPoint3*           UVWs() const { return fUVW.Value(); }
    hsPoint3*           UVW(int i) const { return fUVW.Value() + i; }

    void                Begin() { fUVW.Begin(); }
    void                Advance() { fUVW.Advance(); }
    bool                More() const { return fUVW.More(); }
};

class plAccDiffuseIterator
{
    plAccIterator<uint32_t>       fDiffuse;
public:
    plAccDiffuseIterator(plAccessVtxSpan* acc) 
        :   fDiffuse(acc, plAccessVtxSpan::kDiffuse) {}
    plAccDiffuseIterator() {}

    plAccDiffuseIterator& Set(plAccessVtxSpan* acc) 
    { 
        fDiffuse.Set(acc, plAccessVtxSpan::kDiffuse);
        return *this; 
    }

    uint32_t*             Diffuse32() const { return fDiffuse.Value(); }

    hsColorRGBA         DiffuseRGBA() const { return hsColorRGBA().FromARGB32(*Diffuse32()); }

    void                Begin() { fDiffuse.Begin(); }
    void                Advance() { fDiffuse.Advance(); }
    bool                More() const { return fDiffuse.More(); }
};

class plAccDiffSpecIterator
{
protected:
    plAccIterator<uint32_t>       fDiffuse;
    plAccIterator<uint32_t>       fSpecular;
public:
    plAccDiffSpecIterator(plAccessVtxSpan* acc) 
        :   fDiffuse(acc, plAccessVtxSpan::kDiffuse),
            fSpecular(acc, plAccessVtxSpan::kSpecular) {}
    plAccDiffSpecIterator() {}

    plAccDiffSpecIterator& Set(plAccessVtxSpan* acc) 
    { 
        fDiffuse.Set(acc, plAccessVtxSpan::kDiffuse);
        fSpecular.Set(acc, plAccessVtxSpan::kSpecular);
        return *this; 
    }

    uint32_t*             Diffuse32() const { return fDiffuse.Value(); }
    uint32_t*             Specular32() const { return fSpecular.Value(); }

    hsColorRGBA         DiffuseRGBA() const { return hsColorRGBA().FromARGB32(*Diffuse32()); }
    hsColorRGBA         SpecularRGBA() const { return hsColorRGBA().FromARGB32(*Specular32()); }

    void                Begin() { fDiffuse.Begin(); fSpecular.Begin(); }
    void                Advance() { fDiffuse.Advance(); fSpecular.Advance(); }
    bool                More() const { return fDiffuse.More(); }
};


class plAccVertexIterator
{
protected:
    plAccIterator<hsPoint3>     fPosition;
    plAccIterator<float>     fWeight;
    plAccIterator<uint8_t>        fWgtIndex;
    plAccIterator<hsVector3>    fNormal;
    plAccIterator<uint32_t>       fDiffuse;
    plAccIterator<uint32_t>       fSpecular;
    plAccIterator<hsPoint3>     fUVW;
public:
    plAccVertexIterator(plAccessVtxSpan* acc) 
        :   fPosition(acc, plAccessVtxSpan::kPosition), 
            fWeight(acc, plAccessVtxSpan::kWeight),
            fWgtIndex(acc, plAccessVtxSpan::kWgtIndex),
            fNormal(acc, plAccessVtxSpan::kNormal), 
            fDiffuse(acc, plAccessVtxSpan::kDiffuse),
            fSpecular(acc, plAccessVtxSpan::kSpecular),
            fUVW(acc, plAccessVtxSpan::kUVW) {}
    plAccVertexIterator() {}

    plAccVertexIterator& Set(plAccessVtxSpan* acc) 
    { 
        fPosition.Set(acc, plAccessVtxSpan::kPosition); 
        fWeight.Set(acc, plAccessVtxSpan::kWeight);
        fWgtIndex.Set(acc, plAccessVtxSpan::kWgtIndex);
        fNormal.Set(acc, plAccessVtxSpan::kNormal); 
        fDiffuse.Set(acc, plAccessVtxSpan::kDiffuse);
        fSpecular.Set(acc, plAccessVtxSpan::kSpecular);
        fUVW.Set(acc, plAccessVtxSpan::kUVW); 
        return *this; 
    }

    hsPoint3*           Position() const { return fPosition.Value(); }
    float*           Weights() const { return fWeight.Value(); }
    float*           Weight(int i) const { return fWeight.Value() + i; }
    uint32_t*             WgtIndices() const { return (uint32_t*)(fWgtIndex.Value()); }
    uint8_t*              WgtIndex(int i) const { return fWgtIndex.Value() + i; }
    hsVector3*          Normal() const { return fNormal.Value(); }
    hsPoint3*           UVWs() const { return fUVW.Value(); }
    hsPoint3*           UVW(int i) const { return fUVW.Value() + i; }

    uint32_t*             Diffuse32() const { return fDiffuse.Value(); }
    uint32_t*             Specular32() const { return fSpecular.Value(); }

    hsColorRGBA         DiffuseRGBA() const { return hsColorRGBA().FromARGB32(*Diffuse32()); }
    hsColorRGBA         SpecularRGBA() const { return hsColorRGBA().FromARGB32(*Specular32()); }

    void                Begin() { fPosition.Begin(); fWeight.Begin(); fNormal.Begin(); fDiffuse.Begin(); fSpecular.Begin(); fUVW.Begin(); }
    void                Advance() { fPosition.Advance(); fWeight.Advance(); fNormal.Advance(); fDiffuse.Begin(); fSpecular.Begin(); fUVW.Advance(); }
    bool                More() const { return fPosition.More(); }
};


#endif // plAccessVtxSpan_inc
