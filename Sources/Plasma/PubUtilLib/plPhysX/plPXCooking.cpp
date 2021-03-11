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

#include "plPXCooking.h"
#include "hsGeometry3.h"
#include "hsStream.h"

// ==========================================================================

bool plPXCooking::IsCooked(hsStream* s)
{
    // PhysX streams begin with the magic string "NXS\x01"
    // Our hacked streams begin with the magic string "HSP\x01"
    char magic[4];
    s->Read(sizeof(magic), magic);

    if (memcmp("HSP\x01", magic, sizeof(magic)) == 0)
        return false;
    if (memcmp("NXS\x01", magic, sizeof(magic)) == 0)
        return true;

    throw plPXCookingException(ST::format("Invalid header magic value {x}", *(uint32_t*)magic));
}

// ==========================================================================

static void ISkipMaxDependantList(hsStream* s, uint32_t size)
{
    uint32_t max = s->ReadLE32();
    if (max > 0xFFFF)
        s->Skip(size * 4);
    else if (max > 0xFF)
        s->Skip(size * 2);
    else
        s->Skip(size);
}

static void IReadSuffix(hsStream* s)
{
    // HBM - who gives a shit
    s->Skip(s->ReadLE32());

    s->Skip(11 * sizeof(float));
    if (s->ReadLEFloat() > -1.f)
        s->Skip(12 * sizeof(float));
}

// ==========================================================================

void plPXCooking::ReadConvexHull26(hsStream* s, std::vector<uint32_t>& tris, std::vector<hsPoint3>& verts)
{
    if (!IsCooked(s)) {
        verts.resize(s->ReadLE32());
        for (auto& vert : verts)
            vert.Read(s);
        return;
    }

    char tag[4];
    uint32_t unk1, unk2, unk3, unk4, unk5;
    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "CVXM", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid CVXM header");
    if (s->ReadLE32() != 2)
        throw plPXCookingException("Invalid CVXM: Only version 2 supported");
    s->ReadLE32();

    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "ICE\x01", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid ICE header");
    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "CLHL", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid CLHL header");
    if (s->ReadLE32() != 0)
        throw plPXCookingException("Invalid CLHL: Only version 0 supported");

    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "ICE\x01", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid ICE header");
    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "CVHL", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid CVHL header");
    if (s->ReadLE32() != 5)
        throw plPXCookingException("Invalid CVHL: Only version 5 supported");

    uint32_t nxNumVerts = s->ReadLE32();
    uint32_t nxNumTris = s->ReadLE32();
    unk2 = s->ReadLE32();
    unk3 = s->ReadLE32();
    unk4 = s->ReadLE32();
    if (unk4 != unk2 * 2)
        throw plPXCookingException("Invalid u4");
    unk5 = s->ReadLE32();
    if (unk4 != unk5)
        throw plPXCookingException("Invalid u5");

    verts.resize(nxNumVerts);
    for (auto& vert : verts)
        vert.Read(s);

    // Triangles - who gives a shit
    uint32_t maxVertIndex = s->ReadLE32();
    if (maxVertIndex + 1 != nxNumVerts)
        throw plPXCookingException("Invalid maxVertIndex");
    if (maxVertIndex > 0xFFFF)
        s->Skip(sizeof(uint32_t) * nxNumTris * 3);
    else if (maxVertIndex > 0xFF)
        s->Skip(sizeof(uint16_t) * nxNumTris * 3);
    else
        s->Skip(nxNumTris * 3);

    if (s->ReadLE16() != 0)
        throw plPXCookingException("Invalid u7: only 0 supported");

    s->Skip(nxNumVerts * 2);
    s->ReadLEFloat();
    s->ReadLEFloat();
    s->ReadLEFloat();
    s->Skip(unk3 * 36);
    s->Skip(unk4);

    // a list of size unk4, with the maximum first to determine the size of the elements
    ISkipMaxDependantList(s, unk4);

    s->ReadLE32();
    s->ReadLE32();
    s->Skip(unk2 * 2);
    s->Skip(unk2 * 2);

    // three lists of size u2 in the format: first the maximum, then the elements are only large
    // enough to hold values <= max
    ISkipMaxDependantList(s, unk2);
    ISkipMaxDependantList(s, unk2);
    ISkipMaxDependantList(s, unk2);
    // and another fixed-size list
    s->Skip(unk2 * 2);

    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "ICE\x01", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid ICE header");
    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "VALE", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid VALE header");
    if (s->ReadLE32() != 2)
        throw plPXCookingException("Invalid VALE: Only version 2 supported");

    unk1 = s->ReadLE32();
    unk2 = s->ReadLE32();

    ISkipMaxDependantList(s, unk1);
    s->Skip(unk2);

    IReadSuffix(s);

    if (nxNumVerts > 0x20) {
        s->Read(sizeof(tag), tag);
        if (memcmp(tag, "ICE\x01", sizeof(tag)) != 0)
            throw plPXCookingException("Invalid ICE header");
        s->Read(sizeof(tag), tag);
        if (memcmp(tag, "SUPM", sizeof(tag)) != 0)
            throw plPXCookingException("Invalid SUPM header");
        if (s->ReadLE32() != 0)
            throw plPXCookingException("Invalid SUPM: Only version 0 supported");

        s->Read(sizeof(tag), tag);
        if (memcmp(tag, "ICE\x01", sizeof(tag)) != 0)
            throw plPXCookingException("Invalid ICE header");
        s->Read(sizeof(tag), tag);
        if (memcmp(tag, "GAUS", sizeof(tag)) != 0)
            throw plPXCookingException("Invalid GAUS header");
        if (s->ReadLE32() != 0)
            throw plPXCookingException("Invalid GAUS: Only version 0 supported");

        s->ReadLE32();
        unk2 = s->ReadLE32();
        s->Skip(unk2 * 2);
    }
}

// ==========================================================================

void plPXCooking::ReadTriMesh26(hsStream* s, std::vector<uint32_t>& tris, std::vector<hsPoint3>& verts)
{
    if (!IsCooked(s)) {
        verts.resize(s->ReadLE32());
        for (auto& vert : verts)
            vert.Read(s);
        tris.resize(s->ReadLE32() * 3);
        for (auto& tri : tris)
            tri = s->ReadLE32();
        return;
    }

    char tag[4];
    s->Read(sizeof(tag), tag);
    if (memcmp(tag, "MESH", sizeof(tag)) != 0)
        throw plPXCookingException("Invalid Mesh header");
    if (s->ReadLE32() != 0)
        throw plPXCookingException("Invalid MESH: Only version 0 supported");

    uint32_t nxFlags = s->ReadLE32();
    s->ReadLEFloat();
    s->ReadLE32();
    s->ReadLEFloat();
    uint32_t nxNumVerts = s->ReadLE32();
    uint32_t nxNumTris = s->ReadLE32();

    verts.resize(nxNumVerts);
    tris.resize(nxNumTris * 3);
    for (auto& vert : verts)
        vert.Read(s);
    for (auto& tri : tris) {
        if (nxFlags & 0x08)
            tri = s->ReadByte();
        else if (nxFlags & 0x10)
            tri = s->ReadLE16();
        else
            tri = s->ReadLE32();
    }

    if (nxFlags & 0x01)
        s->Skip(nxNumTris * sizeof(uint16_t));
    if (nxFlags & 0x02) {
        uint32_t max = s->ReadLE32();
        if (max > 0xFFFF)
            s->Skip(nxNumTris * sizeof(uint32_t));
        else if (max > 0xFF)
            s->Skip(nxNumTris * sizeof(uint16_t));
        else
            s->Skip(nxNumTris);
    }

    uint32_t nxNumConvexParts = s->ReadLE32();
    uint32_t nxNumFlatParts = s->ReadLE32();

    if (nxNumConvexParts)
        s->Skip(nxNumTris * sizeof(uint16_t));
    if (nxNumFlatParts) {
        if (nxNumFlatParts > 0xFF)
            s->Skip(nxNumTris * sizeof(uint16_t));
        else
            s->Skip(nxNumTris);
    }

    IReadSuffix(s);

    if (s->ReadLE32())
        s->Skip(nxNumTris);
}

// ==========================================================================

void plPXCooking::WriteConvexHull(hsStream* s, uint32_t nverts, const hsPoint3* const verts)
{
    s->Write(4, "HSP\x01");
    s->WriteLE32((uint32_t)nverts);
    for (size_t i = 0; i < nverts; ++i)
        verts[i].Write(s);
}

// ==========================================================================

void plPXCooking::WriteTriMesh(hsStream* s, uint32_t nfaces, const uint32_t* const tris,
                               uint32_t nverts, const hsPoint3* const verts)
{
    s->Write(4, "HSP\x01");
    s->WriteLE32(nverts);
    for (size_t i = 0; i < nverts; ++i)
        verts[i].Write(s);
    s->WriteLE32(nfaces);
    for (size_t i = 0; i < nfaces * 3; ++i)
        s->WriteLE32(tris[i]);
}

void plPXCooking::WriteTriMesh(hsStream* s, uint32_t nfaces, const uint16_t* const tris,
                               uint32_t nverts, const hsPoint3* const verts)
{
    s->Write(4, "HSP\x01");
    s->WriteLE32(nverts);
    for (size_t i = 0; i < nverts; ++i)
        verts[i].Write(s);
    s->WriteLE32(nfaces);
    for (size_t i = 0; i < nfaces * 3; ++i)
        s->WriteLE32((uint32_t)tris[i]);
}
