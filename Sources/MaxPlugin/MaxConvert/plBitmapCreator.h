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

#include "plFileSystem.h"

#include "MaxMain/MaxCompat.h"

class BitmapInfo;
class Bitmap;
class plBitmap;
class plErrorMsg;
class plKey;
class plLocation;
class hsMaxLayerBase;
class plMipmap;

class plBitmapData
{
public:
    enum
    {
        kClampU = 0x01,
        kClampV = 0x02
    };

    plFileName fileName;
    uint32_t texFlags;
    uint32_t createFlags;
    float detailDropoffStart;
    float detailDropoffStop;
    float detailMax;
    float detailMin;
    float sig;
    bool    isStaticCubicEnvMap;
    bool    invertAlpha;
    plFileName faceNames[ 6 ];
    uint32_t  maxDimension;
    uint8_t   clampFlags;
    bool    usePNG;

    plBitmapData()
    {
        texFlags = 0;
        createFlags = 0;
        detailDropoffStart = detailDropoffStop = 0.f;
        detailMax = detailMin = 0.f;
        sig = 0;
        isStaticCubicEnvMap = false;
        invertAlpha = false;
        maxDimension = 0;
        clampFlags = 0;
        usePNG = false;
    }
};

class plRegistryKeyIterator;
class plBitmapCreator
{
    public:

        static plBitmapCreator  &Instance();

        plBitmap    *CreateTexture( plBitmapData *bd, const plLocation &loc, int clipID = -1 );
        plMipmap    *CreateBlankMipmap( uint32_t width, uint32_t height, unsigned config, uint8_t numLevels, const ST::string &keyName, const plLocation &keyLocation );

        void    Init( bool save, plErrorMsg *msg );
        void    DeInit();
        void    CleanUpMaps();

        ~plBitmapCreator();

        // This will also set the key you pass in to nil, so be careful
        void    DeleteExportedBitmap( const plKey &key );

    protected:

        plErrorMsg              *fErrorMsg;

        plBitmapCreator();

        plBitmap    *ICreateTexture( plBitmapData *bd, const plLocation &loc, int clipID = -1 );
        plMipmap    *ICreateBitmap( plBitmapData *bd );

        void    ICheckOutBitmap( BitmapInfo *bInfo, Bitmap *bm, const plFileName &fileName );
        int     IResampBitmap( Bitmap *bm, plMipmap &hBitmap );
        int     ICopyBitmap( Bitmap *bm, plMipmap &hBitmap );
        int     IInvertAlpha( plMipmap &hBitmap );

        void    IAddBitmap( plBitmap *bitmap, bool dontRef = false );
};
