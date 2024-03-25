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

#ifndef plAccessTriSpan_inc
#define plAccessTriSpan_inc

#include "plAccessVtxSpan.h"

class plAccessTriSpan : public plAccessVtxSpan
{
public:
    hsGDeviceRef*   fIdxDeviceRef;

    uint16_t*         fTris; // array length fNumTris*3
    
    uint32_t          fNumTris;

    void            ClearTris() { fTris = nullptr; fNumTris = 0; }

    uint32_t          TriCount() const { return fNumTris; }

    void            SetIdxDeviceRef(hsGDeviceRef* ref) { fIdxDeviceRef = ref; }
    hsGDeviceRef*   GetIdxDeviceRef() const { return fIdxDeviceRef; }
};

class plAccTriIterator
{
protected:

    uint16_t*                 fCurrIdx;
    uint16_t*                 fEndIdx;

    const plAccessTriSpan*    fAccess;
public:
    plAccTriIterator() { Set(nullptr); }
    plAccTriIterator(const plAccessTriSpan* acc) { Set(acc); }

    void            Set(const plAccessTriSpan* acc) { fAccess = acc; fCurrIdx = nullptr; }

    void            Begin();
    void            Advance();
    void            SetTri(int i);
    bool            More() const;

    uint32_t          NumTris() const { return fAccess->fNumTris; }

    uint16_t&         RawIndex(int iVtx) const { return fCurrIdx[iVtx]; }

    hsPoint3&       Position(int iVtx) const { return fAccess->PositionOff(fCurrIdx[iVtx]); }
    hsVector3&      Normal(int iVtx) const { return fAccess->NormalOff(fCurrIdx[iVtx]); }
    uint32_t          Diffuse32(int iVtx) const { return fAccess->Diffuse32Off(fCurrIdx[iVtx]); }
    uint32_t          Specular32(int iVtx) const { return fAccess->Specular32Off(fCurrIdx[iVtx]); }
    hsColorRGBA     DiffuseRGBA(int iVtx) const { return fAccess->DiffuseRGBAOff(fCurrIdx[iVtx]); }
    hsColorRGBA     SpecularRGBA(int iVtx) const { return fAccess->SpecularRGBAOff(fCurrIdx[iVtx]); }
    hsPoint3*       UVWs(int iVtx) const { return fAccess->UVWsOff(fCurrIdx[iVtx]); }
    hsPoint3&       UVW(int iVtx, int iUVW) const { return fAccess->UVWOff(fCurrIdx[iVtx], iUVW); }

    hsPoint3        InterpPosition(const hsPoint3& bary) const;
    hsVector3       InterpNormal(const hsPoint3& bary) const;
    uint32_t          InterpDiffuse32(const hsPoint3& bary) const;
    uint32_t          InterpSpecular32(const hsPoint3& bary) const;
    hsColorRGBA     InterpDiffuseRGBA(const hsPoint3& bary) const;
    hsColorRGBA     InterpSpecularRGBA(const hsPoint3& bary) const;
    hsPoint3        InterpUVW(const hsPoint3& bary, int i) const;


};

inline void plAccTriIterator::Begin()
{
    fCurrIdx = fAccess->fTris;
    fEndIdx = fCurrIdx + fAccess->fNumTris*3;
}

inline void plAccTriIterator::Advance()
{
    fCurrIdx += 3;
}

inline void plAccTriIterator::SetTri(int i)
{
    fCurrIdx = fAccess->fTris + i * 3;
}

inline bool plAccTriIterator::More() const
{
    return fCurrIdx < fEndIdx;
}

inline hsPoint3 plAccTriIterator::InterpPosition(const hsPoint3& bary) const
{
    return Position(0) * bary[0]
        + Position(1) * bary[1]
        + Position(2) * bary[2];
}

inline hsVector3 plAccTriIterator::InterpNormal(const hsPoint3& bary) const
{
    return Normal(0) * bary[0]
        + Normal(1) * bary[1]
        + Normal(2) * bary[2];
}

inline hsColorRGBA plAccTriIterator::InterpDiffuseRGBA(const hsPoint3& bary) const
{
    return DiffuseRGBA(0) * bary[0]
                + DiffuseRGBA(1) * bary[1]
                + DiffuseRGBA(2) * bary[2];
}

inline hsColorRGBA plAccTriIterator::InterpSpecularRGBA(const hsPoint3& bary) const
{
    return SpecularRGBA(0) * bary[0]
                + SpecularRGBA(1) * bary[1]
                + SpecularRGBA(2) * bary[2];
}

inline uint32_t plAccTriIterator::InterpDiffuse32(const hsPoint3& bary) const
{
    return InterpDiffuseRGBA(bary).ToARGB32();
}

inline uint32_t plAccTriIterator::InterpSpecular32(const hsPoint3& bary) const
{
    return InterpSpecularRGBA(bary).ToARGB32();
}

inline hsPoint3 plAccTriIterator::InterpUVW(const hsPoint3& bary, int i) const
{
    return UVW(0, i) * bary[0]
        + UVW(1, i) * bary[1]
        + UVW(2, i) * bary[2];
}

#endif // plAccessTriSpan_inc
