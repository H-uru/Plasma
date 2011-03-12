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
//////////////////////////////////////////////////////////////////////
//
// pyVaultImageNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultImageNode.h"
#ifndef BUILDING_PYPLASMA
#include "pyVault.h"
#endif
#include "pyImage.h"
#include "plPipeline.h"
#include "cyMisc.h"

#include "hsResMgr.h"
#include "../plGImage/plMipmap.h"
#include "../plVault/plVault.h"
#include "../pnMessage/plRefMsg.h"
#include "../plNetClient/plNetClientMgr.h"


static unsigned s_keyseq;

//============================================================================
static plKey CreateAndRefImageKey (unsigned nodeId, plMipmap * mipmap) {
	char keyName[MAX_PATH];
	StrPrintf(keyName, arrsize(keyName), "VaultImg_%u_%u", nodeId, s_keyseq++);

	plKey key = hsgResMgr::ResMgr()->NewKey(keyName, mipmap, plLocation::kGlobalFixedLoc);

	hsgResMgr::ResMgr()->AddViaNotify(
		key,
		NEW(plGenRefMsg)(
			plNetClientMgr::GetInstance()->GetKey(),
			plRefMsg::kOnCreate,
			0,
			plNetClientMgr::kVaultImage
		),
		plRefFlags::kActiveRef
	);
	
	return key;
}

// should only be created from C++ side
pyVaultImageNode::pyVaultImageNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
, fMipmapKey(nil)
, fMipmap(nil)
{
}

//create from the Python side
pyVaultImageNode::pyVaultImageNode(int n)
: pyVaultNode(NEWZERO(RelVaultNode))
, fMipmapKey(nil)
, fMipmap(nil)
{
	fNode->SetNodeType(plVault::kNodeType_Image);
}

pyVaultImageNode::~pyVaultImageNode () {
	if (fMipmap && fMipmapKey)
		fMipmapKey->UnRefObject();
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultImageNode::Image_SetTitle( const char * text )
{
	if (!fNode)
		return;
		
	wchar * wStr = hsStringToWString(text);

	VaultImageNode image(fNode);
	image.SetImageTitle(wStr);
	delete [] wStr;
}

void pyVaultImageNode::Image_SetTitleW(	const wchar_t* text )
{
	if (!fNode)
		return;

	VaultImageNode image(fNode);
	image.SetImageTitle(text);
}

std::string pyVaultImageNode::Image_GetTitle( void )
{
	if (!fNode)
		return "";

	VaultImageNode image(fNode);

	std::string retVal = "";
	if (image.title)
	{
		char* temp = hsWStringToString(image.title);
		retVal = temp;
		delete [] temp;
	}
	
	return retVal;
}

std::wstring pyVaultImageNode::Image_GetTitleW( void )
{
	if (!fNode)
		return L"";

	VaultImageNode image(fNode);
	return image.title ? image.title : L"";
}

PyObject* pyVaultImageNode::Image_GetImage( void )
{
	if (!fNode)
		PYTHON_RETURN_NONE;
		
	if (!fMipmap) {
		VaultImageNode access(fNode);
		if (access.ExtractImage(&fMipmap)) {
			fMipmapKey = fMipmap->GetKey();
			if (!fMipmapKey)
				fMipmapKey = CreateAndRefImageKey(fNode->nodeId, fMipmap);
			else
				fMipmapKey->RefObject();
		}
	}
	
	return pyImage::New(fMipmap);	
}

void pyVaultImageNode::Image_SetImage(pyImage& image)
{
	if (!fNode)
		return;

	if (fMipmapKey) {
		fMipmapKey->UnRefObject();
		fMipmapKey = nil;
		fMipmap = nil;
	}

	if (fMipmap = image.GetImage()) {
		VaultImageNode access(fNode);
		access.StuffImage(fMipmap);
		
		fMipmapKey = image.GetKey();
		if (!fMipmapKey)
			fMipmapKey = CreateAndRefImageKey(fNode->nodeId, fMipmap);
		else
			fMipmapKey->RefObject();
	}
}

void pyVaultImageNode::SetImageFromBuf( PyObject * pybuf )
{
	if (!fNode)
		return;

	if (fMipmapKey) {
		fMipmapKey->UnRefObject();
		fMipmapKey = nil;
		fMipmap = nil;
	}

	byte * buffer = nil;
	int bytes;
	PyObject_AsReadBuffer(pybuf, (const void **)&buffer, &bytes);
	if (buffer) {
		VaultImageNode access(fNode);
		access.SetImageData(buffer, bytes);
		access.SetImageType(VaultImageNode::kJPEG);
	}
}

void pyVaultImageNode::SetImageFromScrShot()
{
	if (!fNode)
		return;

	if (fMipmapKey) {
		fMipmapKey->UnRefObject();
		fMipmapKey = nil;
		fMipmap = nil;
	}

	if (cyMisc::GetPipeline()) {
		VaultImageNode access(fNode);
		fMipmap = NEW(plMipmap);
		if (cyMisc::GetPipeline()->CaptureScreen(fMipmap, false, 800, 600)) {
			fMipmapKey = fMipmap->GetKey();
			if (!fMipmapKey)
				fMipmapKey = CreateAndRefImageKey(fNode->nodeId, fMipmap);
			else
				fMipmapKey->RefObject();
			access.StuffImage(fMipmap);
		}
		else {
			access.SetImageData(nil, 0);
			access.SetImageType(VaultImageNode::kNone);
			DEL(fMipmap);
			fMipmap = nil;
		}
	}
}
