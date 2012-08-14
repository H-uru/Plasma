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

#if HS_BUILD_FOR_WIN32
#include "hsDXTDirectXCodec.h"
#endif

hsCodecManager& hsCodecManager::Instance()
{
    static hsCodecManager the_instance;
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        hsDXTSoftwareCodec::Init();

#if HS_BUILD_FOR_WIN32
        hsDXTDirectXCodec::Init();
#endif
    }

    return the_instance;
}

hsCodecManager::hsCodecManager()
{
}

plMipmap *hsCodecManager::CreateCompressedMipmap(uint32_t compressionFormat, plMipmap *uncompressed)
{
    int32_t i, j;
    for (i = 0; i < fCodecTable.Count(); i++)
    {
        if (fCodecTable[i].fCompressionFormat == compressionFormat)
        {
            for (j = 0; j < fCodecTable[i].fCodecList.Count(); j++)
            {
                hsAssert(fCodecTable[i].fCodecList[j].fCodec != 0, 
                    "Nil codec in hsCodecManager::CreateCompressedMipmap.");

                plMipmap *bm = 
                    fCodecTable[i].fCodecList[j].fCodec->CreateCompressedMipmap(uncompressed);

                if (bm)
                {
                    return bm;
                }
            }
            
            return nil;
        }
    }

    return nil;
}

plMipmap *hsCodecManager::CreateUncompressedMipmap(plMipmap *compressed, uint8_t bitDepth)
{
    int32_t i, j;
    for (i = 0; i < fCodecTable.Count(); i++)
    {
        if( fCodecTable[i].fCompressionFormat == compressed->fCompressionType )
        {
            for (j = 0; j < fCodecTable[i].fCodecList.Count(); j++)
            {
                hsAssert(fCodecTable[i].fCodecList[j].fCodec != 0, 
                    "Nil codec in hsCodecManager::CreateUncompressedMipmap.");

                plMipmap *bm = 
                    fCodecTable[i].fCodecList[j].fCodec->CreateUncompressedMipmap(compressed, bitDepth);

                if (bm)
                {
                    return bm;
                }
            }
            
            return nil;
        }
    }

    return nil;
}

bool hsCodecManager::ColorizeCompMipmap( plMipmap *bMap, const uint8_t *colorMask )
{
    int32_t i, j;


    for( i = 0; i < fCodecTable.Count(); i++ )
    {
        if( fCodecTable[ i ].fCompressionFormat == bMap->fCompressionType )
        {
            for( j = 0; j < fCodecTable[ i ].fCodecList.Count(); j++ )
            {
                hsAssert( fCodecTable[ i ].fCodecList[ j ].fCodec != 0, 
                    "Nil codec in hsCodecManager::CreateUncompressedMipmap." );

                if( fCodecTable[ i ].fCodecList[ j ].fCodec->ColorizeCompMipmap( bMap, colorMask ) )
                    return true;
            }
            return false;
        }
    }
    return false;
}

bool hsCodecManager::Register(hsCodec *codec, uint32_t compressionFormat, float priority)
{
    int32_t i, j;
    for (i = 0; i < fCodecTable.Count(); i++)
    {
        if (fCodecTable[i].fCompressionFormat == compressionFormat)
        {
            j = 0;
            while ((j < fCodecTable[i].fCodecList.Count()) &&
                fCodecTable[i].fCodecList[j].fPriority > priority)
                ++j;

            hsCodecEntry tempCodecEntry(priority, codec);
            fCodecTable[i].fCodecList.InsertAtIndex(j, tempCodecEntry);

            return true;
        }
    }

    hsCodecList tempCodecList(compressionFormat);
    fCodecTable.Append(tempCodecList);

    hsCodecEntry tempCodecEntry(priority, codec);
    fCodecTable[fCodecTable.Count() - 1].fCodecList.Append(tempCodecEntry);

    return true;
}

