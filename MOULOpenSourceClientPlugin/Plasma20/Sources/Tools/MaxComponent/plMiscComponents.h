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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plMiscComponents_inc
#define plMiscComponents_inc

#include "plComponent.h"
#include "Notify.h"

#define ROOM_CID Class_ID(0x70a1570d, 0x472f5647)
#define PAGEINFO_CID Class_ID(0x54ee40f1, 0x4de45acc)
#define IGNORELITE_CID Class_ID(0x6abb5b4b, 0x6ba27af7)
#define RAIL_CAM_CID Class_ID(0x1b097048, 0xc94038b)
#define CIRCLE_CAM_CID Class_ID(0x66f85282, 0x4daa1b8e)
#define IMAGE_LIB_CID	Class_ID(0x736c18d3, 0x6a6d5dde)

class plComponentBase;
class plAgeDescription;

const char* LocCompGetPage(plComponentBase* comp);

namespace plPageInfoUtils
{
	const char *GetAgeFolder();
	Int32		GetSeqNumFromAgeDesc( const char *ageName, const char *pageName );
	Int32		CombineSeqNum( int prefix, int suffix );
	Int32		GetCommonSeqNumFromNormal( Int32 normalSeqNumber, int whichCommonPage );

	plAgeDescription	*GetAgeDesc( const char *ageName );
};

// PageInfo component definition, here so other components can get to the static function(s)
class plPageInfoComponent : public plComponent
{
protected:
	hsBool		fSeqNumValidated;
	hsBool		fItinerant;
	static char	fCurrExportedAge[ 256 ];

	void	IVerifyLatestAgeAsset( const char *ageName, const char *localPath, plErrorMsg *errMsg );
	void	IUpdateSeqNumbersFromAgeFile( plErrorMsg *errMsg );		
	
public:
	plPageInfoComponent();

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);
	char *GetAgeName();
	hsBool GetItinerant() {return fItinerant; }
	
	enum
	{
		kInfoAge=3,
		kInfoPage,
		kInfoSeqPrefix,
		kInfoSeqSuffix,
		kRefVolatile_PageInfoUpdated,
		kItinerant
	};

	static char *GetCurrExportAgeName() { return (char *)&fCurrExportedAge; }
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
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum ParamIDs
	{
		kRefImageList,
		kCompressImage,
	};

	int			GetNumBitmaps( void ) const;
	plLayerTex	*GetBitmap( int idx );
	int			AppendBitmap( plLayerTex *tex );
	void		RemoveBitmap( int idx );
	bool		GetCompress(int idx);
	void		SetCompress(int idx, bool compress);

	void Validate();
};

#endif // plMiscComponents_inc
