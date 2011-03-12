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
/******************************************************************************
 ValdezInterface.h

 Eric Ellis
******************************************************************************/

#ifndef _JV_VALDEZINTERFACE_H_
#define _JV_VALDEZINTERFACE_H_

#define MAXASS_CLASS_ID	Class_ID(0x5c61b5a6, 0x3b298521)
#define kMaxAssGetValdezInterface 33

#include <commdlg.h>
#include <bmmlib.h>
#include <guplib.h>
#include <vector>
#include <string>
#include <comdef.h>

using std::vector;
using std::string;

#pragma warning(disable:4786)


#define kAssetTypeIdTexure    1
#define kAssetTypeIdSound     2
#define kAssetTypeIdMaxFile   4
#define kAssetTypeIdAge       7

#define kStatusIdDraft        1
#define kStatusIdStable       2



class jvValdezInterface
{
public:
	// MAX File Replacement Operations
	virtual int ChooseAssetAndOpen() = 0;
	virtual int Save() = 0;
	virtual int SaveAs() = 0;
	virtual int Add() = 0;
	virtual int OpenBitmapDlg(VARIANT* assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;
	virtual int OpenSoundDlg(VARIANT* assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;
	virtual int NewAgeDlg(VARIANT* assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;
	virtual int NewTextureDlg(VARIANT* assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;

	// Asset Database Operations
	virtual int GetLatestVersionFile(VARIANT& assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;
	virtual int GetAssetsByType(int assetTypeId, vector<_variant_t>& assetIds, vector<string>& assetNames) = 0;
	virtual int CheckOutAsset(VARIANT& assetId, TCHAR* localFilenameRet, int localFilenameBufSize) = 0;
	virtual int CheckInAsset(VARIANT& assetId, TCHAR* localFilename, int statusId, TCHAR* comments) = 0;

	virtual int FindAndCompareAssetByFilename(const TCHAR* localFilename, VARIANT* assetId, bool* filesMatch) = 0;
	virtual int FindAssetsByFilename(const TCHAR* filename, vector<_variant_t>& assets) = 0;

	virtual int IsAssetCheckedOutLocally(VARIANT& assetId, bool& checkedOut) = 0;
};


inline jvValdezInterface* GetValdezInterface() 
{ 
   GUP* maxAssGup = OpenGupPlugIn(MAXASS_CLASS_ID);
   
   if(!maxAssGup) return NULL;

   return (jvValdezInterface*)maxAssGup->Control(kMaxAssGetValdezInterface);
}

#endif _JV_VALDEZINTERFACE_H_
