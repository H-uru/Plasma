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
#ifndef PLCLOTHINGLAYOUT_INC
#define PLCLOTHINGLAYOUT_INC

#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsUtils.h"

// This file is intended to be an independent section so that plClothingMtl and plAvatarClothing
// can both use these structures, without all the plCreatable/hsKeyedObject/plSynchedObject stuff
// No virtual methods allowed, and define everything inside the class declaration. (So that we can
// #include the file where necessary and not get redundant method definition errors.)

class plClothingElement
{
public:
	enum
	{
		kLayerBase,
		kLayerSkin,
		kLayerSkinBlend1,
		kLayerSkinBlend2,
		kLayerSkinBlend3,
		kLayerSkinBlend4,
		kLayerSkinBlend5,
		kLayerSkinBlend6,
		kLayerTint1,
		kLayerTint2,
		kLayerMax,
		kLayerSkinFirst = kLayerSkin,
		kLayerSkinLast = kLayerSkinBlend6,
	};
	
	char *fName;
	UInt32 fXPos;
	UInt32 fYPos;
	UInt32 fWidth;
	UInt32 fHeight;

	plClothingElement(char *name, UInt32 xPos, UInt32 yPos, UInt32 width, UInt32 height)
	{
		fName = hsStrcpy(name);
		fXPos = xPos;
		fYPos = yPos;
		fWidth = width;
		fHeight = height;
	}
	~plClothingElement() { delete [] fName; }

	static void GetElements(hsTArray<plClothingElement *> &out)
	{
	/*
		out.Append(TRACKED_NEW plClothingElement("shirt-chest", 768, 0, 256, 512));
		out.Append(TRACKED_NEW plClothingElement("shirt-sleeve", 512, 192, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("face", 0, 0, 512, 512));
		out.Append(TRACKED_NEW plClothingElement("eyeball", 64, 512, 64, 64));
		out.Append(TRACKED_NEW plClothingElement("shoe-top", 0, 832, 128, 128));
		out.Append(TRACKED_NEW plClothingElement("shoe-bottom", 0, 768, 128, 64));
		out.Append(TRACKED_NEW plClothingElement("pants", 512, 512, 512, 512));
		out.Append(TRACKED_NEW plClothingElement("hand-LOD", 64, 576, 64, 64));
		out.Append(TRACKED_NEW plClothingElement("hand-square", 128, 512, 128, 128));
		out.Append(TRACKED_NEW plClothingElement("hand-wide", 0, 640, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("playerbook", 512, 0, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("backpack", 512, 256, 256, 256));
		out.Append(TRACKED_NEW plClothingElement("glasses-front", 256, 512, 256, 64));
		out.Append(TRACKED_NEW plClothingElement("glasses-side", 256, 576, 256, 32));
		out.Append(TRACKED_NEW plClothingElement("KI", 256, 640, 256, 128));
	*/	
		out.Append(TRACKED_NEW plClothingElement("Chest", 768, 0, 256, 512));
		out.Append(TRACKED_NEW plClothingElement("Arm", 512, 192, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("Face", 0, 256, 512, 256));
		out.Append(TRACKED_NEW plClothingElement("Eye", 64, 704, 64, 64));
		out.Append(TRACKED_NEW plClothingElement("Extra Hair", 256, 0, 256, 256));
		out.Append(TRACKED_NEW plClothingElement("Hat", 0, 0, 256, 256));
		out.Append(TRACKED_NEW plClothingElement("Foot", 0, 768, 256, 256));
		out.Append(TRACKED_NEW plClothingElement("Legs", 512, 512, 512, 512));
		out.Append(TRACKED_NEW plClothingElement("LOD", 64, 640, 64, 64));
		out.Append(TRACKED_NEW plClothingElement("Finger", 128, 640, 128, 128));
		out.Append(TRACKED_NEW plClothingElement("Palm", 0, 512, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("Player Book", 256, 512, 256, 128));
		out.Append(TRACKED_NEW plClothingElement("Glasses", 384, 640, 128, 128));
		out.Append(TRACKED_NEW plClothingElement("KI", 256, 640, 128, 128));		
		
	}
};

class plClothingLayout
{
public:
	plClothingLayout(char *name, UInt32 origWidth) { fName = hsStrcpy(name); fOrigWidth = origWidth; }
	~plClothingLayout() { delete [] fName; }

	char *fName;
	UInt32 fOrigWidth;
	hsTArray<plClothingElement*> fElements;
/*
	enum
	{
		kSetShirt,
		kSetFace,
		kSetEye,
		kSetShoe,
		kSetPants,
		kSetHand,
		kSetPlayerBook,
		kSetBackpack,
		kSetGlasses,
		kSetKI,
		kMaxTileset,
	};
*/
	enum
	{
		kSetShirt,
		kSetFace,
		kSetShoe,
		kSetPants,
		kSetHand,
		kSetPlayerBook,
		kSetGlasses,
		kSetKI,
		kSetEye, // UNUSED
		kSetBackpack, // UNUSED
		kMaxTileset,
	};
	
};

#endif
