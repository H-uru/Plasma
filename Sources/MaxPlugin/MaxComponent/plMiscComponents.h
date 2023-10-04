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

#ifndef plMiscComponents_inc
#define plMiscComponents_inc

#include <tchar.h>

#include "plComponent.h"

#define ROOM_CID Class_ID(0x70a1570d, 0x472f5647)
#define PAGEINFO_CID Class_ID(0x54ee40f1, 0x4de45acc)
#define IGNORELITE_CID Class_ID(0x6abb5b4b, 0x6ba27af7)
#define RAIL_CAM_CID Class_ID(0x1b097048, 0xc94038b)
#define CIRCLE_CAM_CID Class_ID(0x66f85282, 0x4daa1b8e)
#define IMAGE_LIB_CID   Class_ID(0x736c18d3, 0x6a6d5dde)

class plFileName;
class plAgeDescription;
class plComponentBase;

namespace ST { class string; }

const MCHAR* LocCompGetPage(plComponentBase* comp);

namespace plPageInfoUtils
{
    plFileName  GetAgeFolder();
    int32_t     GetSeqNumFromAgeDesc( const ST::string& ageName, const ST::string& pageName );
    int32_t     CombineSeqNum( int prefix, int suffix );
    int32_t     GetCommonSeqNumFromNormal( int32_t normalSeqNumber, int whichCommonPage );

    plAgeDescription    *GetAgeDesc( const ST::string &ageName );
};

// PageInfo component definition, here so other components can get to the static function(s)
class plPageInfoComponent : public plComponent
{
protected:
    bool        fSeqNumValidated;
    bool        fItinerant;
    static TCHAR fCurrExportedAge[ 256 ];

    void    IVerifyLatestAgeAsset( const ST::string &ageName, const plFileName &localPath, plErrorMsg *errMsg );
    void    IUpdateSeqNumbersFromAgeFile( plErrorMsg *errMsg );
    
public:
    plPageInfoComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;
    const MCHAR* GetAgeName();
    bool GetItinerant() {return fItinerant; }
    
    enum
    {
        kInfoAge=3,
        kInfoPage,
        kInfoSeqPrefix,
        kInfoSeqSuffix,
        kRefVolatile_PageInfoUpdated,
        kItinerant
    };

    static TCHAR* GetCurrExportAgeName() { return (TCHAR*)&fCurrExportedAge; }
    static void NotifyProc(void *param, NotifyInfo *info);  
};

//Class that accesses the paramblock below.
class plLayerTex;
class pfImageLibComponent : public plComponent
{
public:
    pfImageLibComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum ParamIDs
    {
        kRefImageList,
        kCompressImage,
    };

    int         GetNumBitmaps() const;
    plLayerTex  *GetBitmap( int idx );
    int         AppendBitmap( plLayerTex *tex );
    void        RemoveBitmap( int idx );
    bool        GetCompress(int idx);
    void        SetCompress(int idx, bool compress);

    void Validate();
};

#endif // plMiscComponents_inc
