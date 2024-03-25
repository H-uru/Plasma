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
#include "hsCodecManager.h"
#include "plMipmap.h"
#include "hsDXTSoftwareCodec.h"

hsCodecManager& hsCodecManager::Instance()
{
    static hsCodecManager the_instance;
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        hsDXTSoftwareCodec::Init();
    }

    return the_instance;
}

hsCodecManager::hsCodecManager()
{
}

plMipmap *hsCodecManager::CreateCompressedMipmap(uint32_t compressionFormat, plMipmap *uncompressed)
{
    for (const hsCodecList& codecList : fCodecTable)
    {
        if (codecList.fCompressionFormat == compressionFormat)
        {
            for (const hsCodecEntry& entry : codecList.fCodecList)
            {
                hsAssert(entry.fCodec != nullptr,
                    "Nil codec in hsCodecManager::CreateCompressedMipmap.");

                plMipmap *bm = entry.fCodec->CreateCompressedMipmap(uncompressed);

                if (bm)
                {
                    return bm;
                }
            }
            
            return nullptr;
        }
    }

    return nullptr;
}

plMipmap *hsCodecManager::CreateUncompressedMipmap(plMipmap *compressed, uint8_t bitDepth)
{
    for (const hsCodecList& codecList : fCodecTable)
    {
        if (codecList.fCompressionFormat == compressed->fCompressionType)
        {
            for (const hsCodecEntry& entry : codecList.fCodecList)
            {
                hsAssert(entry.fCodec != nullptr,
                    "Nil codec in hsCodecManager::CreateUncompressedMipmap.");

                plMipmap *bm = entry.fCodec->CreateUncompressedMipmap(compressed, bitDepth);

                if (bm)
                {
                    return bm;
                }
            }
            
            return nullptr;
        }
    }

    return nullptr;
}

bool hsCodecManager::ColorizeCompMipmap( plMipmap *bMap, const uint8_t *colorMask )
{
    for (const hsCodecList& codecList : fCodecTable)
    {
        if (codecList.fCompressionFormat == bMap->fCompressionType)
        {
            for (const hsCodecEntry& entry : codecList.fCodecList)
            {
                hsAssert(entry.fCodec != nullptr,
                    "Nil codec in hsCodecManager::CreateUncompressedMipmap.");

                if (entry.fCodec->ColorizeCompMipmap(bMap, colorMask))
                    return true;
            }
            return false;
        }
    }
    return false;
}

bool hsCodecManager::Register(hsCodec *codec, uint32_t compressionFormat, float priority)
{
    for (hsCodecList& codecList : fCodecTable)
    {
        if (codecList.fCompressionFormat == compressionFormat)
        {
            auto iter = codecList.fCodecList.cbegin();
            while ((iter != codecList.fCodecList.cend()) && iter->fPriority > priority)
                ++iter;

            codecList.fCodecList.emplace(iter, priority, codec);

            return true;
        }
    }

    fCodecTable.emplace_back(compressionFormat);
    fCodecTable.back().fCodecList.emplace_back(priority, codec);

    return true;
}

