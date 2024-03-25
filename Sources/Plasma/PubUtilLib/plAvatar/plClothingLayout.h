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
#ifndef PLCLOTHINGLAYOUT_INC
#define PLCLOTHINGLAYOUT_INC

#include "HeadSpin.h"
#include <string_theory/string>
#include <vector>


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

    ST::string fName;
    uint32_t fXPos;
    uint32_t fYPos;
    uint32_t fWidth;
    uint32_t fHeight;

    plClothingElement(const ST::string &name, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height)
        : fName(name), fXPos(xPos), fYPos(yPos), fWidth(width), fHeight(height) { }

    static void GetElements(std::vector<plClothingElement *> &out)
    {
        out.insert(out.end(), {
            /*
            new plClothingElement("shirt-chest", 768, 0, 256, 512),
            new plClothingElement("shirt-sleeve", 512, 192, 256, 128),
            new plClothingElement("face", 0, 0, 512, 512),
            new plClothingElement("eyeball", 64, 512, 64, 64),
            new plClothingElement("shoe-top", 0, 832, 128, 128),
            new plClothingElement("shoe-bottom", 0, 768, 128, 64),
            new plClothingElement("pants", 512, 512, 512, 512),
            new plClothingElement("hand-LOD", 64, 576, 64, 64),
            new plClothingElement("hand-square", 128, 512, 128, 128),
            new plClothingElement("hand-wide", 0, 640, 256, 128),
            new plClothingElement("playerbook", 512, 0, 256, 128),
            new plClothingElement("backpack", 512, 256, 256, 256),
            new plClothingElement("glasses-front", 256, 512, 256, 64),
            new plClothingElement("glasses-side", 256, 576, 256, 32),
            new plClothingElement("KI", 256, 640, 256, 128),
            */
            new plClothingElement("Chest", 768, 0, 256, 512),
            new plClothingElement("Arm", 512, 192, 256, 128),
            new plClothingElement("Face", 0, 256, 512, 256),
            new plClothingElement("Eye", 64, 704, 64, 64),
            new plClothingElement("Extra Hair", 256, 0, 256, 256),
            new plClothingElement("Hat", 0, 0, 256, 256),
            new plClothingElement("Foot", 0, 768, 256, 256),
            new plClothingElement("Legs", 512, 512, 512, 512),
            new plClothingElement("LOD", 64, 640, 64, 64),
            new plClothingElement("Finger", 128, 640, 128, 128),
            new plClothingElement("Palm", 0, 512, 256, 128),
            new plClothingElement("Player Book", 256, 512, 256, 128),
            new plClothingElement("Glasses", 384, 640, 128, 128),
            new plClothingElement("KI", 256, 640, 128, 128),
        });
    }
};

class plClothingLayout
{
public:
    plClothingLayout(const ST::string &name, uint32_t origWidth)
        : fName(name), fOrigWidth(origWidth) { }

    ST::string fName;
    uint32_t fOrigWidth;
    std::vector<plClothingElement*> fElements;
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
