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

#include "pyImage.h"

#include <string_theory/format>

#include "hsResMgr.h"
#include "plFileSystem.h"

#include "pnKeyedObject/plUoid.h"

#include "plGImage/plJPEG.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plPNG.h"

#include "pyColor.h"
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"

void pyImage::setKey(pyKey& mipmapKey) // only for python glue, do NOT call
{
#ifndef BUILDING_PYPLASMA
    if (fMipmap && fMipMapKey)
        fMipMapKey->UnRefObject();
    fMipmap = nullptr;
#endif
    fMipMapKey = mipmapKey.getKey();
}

#ifndef BUILDING_PYPLASMA
plMipmap* pyImage::GetImage()
{
    if (fMipmap)
        return fMipmap;
    return ( plMipmap::ConvertNoRef(fMipMapKey->ObjectIsLoaded()) );
}

// GetPixelColor
// takes an x and y coord (x and y from 0 to 1) and returns the pixel color at that location
PyObject* pyImage::GetPixelColor(float x, float y)
{
    if (x > 1.0) x = 1.0;
    if (x < 0.0) x = 0.0;
    if (y > 1.0) y = 1.0;
    if (y < 0.0) y = 0.0;
    
    plMipmap* image;
    if (fMipmap)
        image = fMipmap;
    else
        image = plMipmap::ConvertNoRef(fMipMapKey->ObjectIsLoaded());
    if (image)
    {
        uint32_t height = image->GetHeight();
        uint32_t width = image->GetWidth();
        uint32_t iX = (uint32_t)((float)width * x);
        uint32_t iY = (uint32_t)((float)height * y);
        hsColorRGBA pixColor;
        image->SetCurrLevel(0);
        uint32_t *color = image->GetAddr32(iX,iY);
        pixColor.FromARGB32(*color);
        return pyColor::New(pixColor);
    }
    PYTHON_RETURN_NONE;
}

// GetColorLoc
// takes a color to look for and returns the x and y coord for its location (x and y from 0 to 1), if the
// color exists in more than one location, then the location with the lowest x and y will be returned.
// if the color is not found, it trys to return the closest match
PyObject* pyImage::GetColorLoc(const pyColor &color)
{
    plMipmap* image;
    if (fMipmap)
        image = fMipmap;
    else
        image = plMipmap::ConvertNoRef(fMipMapKey->ObjectIsLoaded());
    if (image)
    {
        uint32_t height = image->GetHeight();
        uint32_t width = image->GetWidth();
        double minSqrDist = 9999999;
        hsPoint3 closestMatch;
        image->SetCurrLevel(0);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                hsColorRGBA pixColor;
                pixColor.FromARGB32(*(image->GetAddr32(x,y)));
                PyObject* imgColorObj = pyColor::New(pixColor);
                pyColor* imgColor = pyColor::ConvertFrom(imgColorObj);
                if ((*imgColor) == color)
                {
                    Py_DECREF(imgColorObj);
                    float fX, fY;
                    fX = (float)x / (float)width;
                    fY = (float)y / (float)height;
                    return pyPoint3::New(hsPoint3(fX, fY, 0.f));
                }
                double dist = pow((imgColor->getRed() - color.getRed()),2) + pow((imgColor->getGreen() - color.getGreen()),2) + pow((imgColor->getBlue() - color.getBlue()),2);
                if (dist < minSqrDist)
                {
                    minSqrDist = dist;
                    float fX, fY;
                    fX = (float)x / (float)width;
                    fY = (float)y / (float)height;
                    closestMatch.fX = fX;
                    closestMatch.fY = fY;
                }
                Py_DECREF(imgColorObj);
            }
        }
        return pyPoint3::New(closestMatch);
    }
    PYTHON_RETURN_NONE;
}

// GetWidth
// returns the width of the image
uint32_t pyImage::GetWidth()
{
    plMipmap* image;
    if (fMipmap)
        image = fMipmap;
    else
        image = plMipmap::ConvertNoRef(fMipMapKey->ObjectIsLoaded());
    if (image)
        return image->GetWidth();
    return 0;
}

// GetHeight
// returns the height of the image
uint32_t pyImage::GetHeight()
{
    plMipmap* image;
    if (fMipmap)
        image = fMipmap;
    else
        image = plMipmap::ConvertNoRef(fMipMapKey->ObjectIsLoaded());
    if (image)
        return image->GetHeight();
    return 0;
}

void pyImage::SaveAsJPEG(const plFileName& fileName, uint8_t quality)
{
    if (quality <= 0 || quality > 100)
    {
            quality = 75;
    }

    plJPEG::Instance().SetWriteQuality(quality);
    plJPEG::Instance().WriteToFile(fileName, this->GetImage());
}

void pyImage::SaveAsPNG(const plFileName& fileName, const std::multimap<ST::string, ST::string>& textFields)
{
    plPNG::Instance().WriteToFile(fileName, this->GetImage(), textFields);
}

PyObject* pyImage::LoadJPEGFromDisk(const plFileName& filename, uint16_t width, uint16_t height)
{
    plMipmap* theMipmap = plJPEG::Instance().ReadFromFile(filename);
    if (theMipmap)
    {
        if (width > 0 && height > 0)
        {
            if (!theMipmap->ResizeNicely(width, height, plMipmap::kDefaultFilter))
            {
                delete theMipmap;
                PYTHON_RETURN_NONE;
            }
        }

        // let's create a nice name for this thing based on the filename
        ST::string name = ST::format("PtImageFromDisk_{}", filename);

        hsgResMgr::ResMgr()->NewKey(name, theMipmap, plLocation::kGlobalFixedLoc);
        
        return pyImage::New( theMipmap );
    }
    else
        PYTHON_RETURN_NONE;
}

PyObject* pyImage::LoadPNGFromDisk(const plFileName& filename, uint16_t width, uint16_t height)
{
    plMipmap* theMipmap = plPNG::Instance().ReadFromFile(filename);
    if (theMipmap)
    {
        if (width > 0 && height > 0)
        {
            if (!theMipmap->ResizeNicely(width, height, plMipmap::kDefaultFilter))
            {
                delete theMipmap;
                PYTHON_RETURN_NONE;
            }
        }

        // let's create a nice name for this thing based on the filename
        ST::string name = ST::format("PtImageFromDisk_{}", filename);

        hsgResMgr::ResMgr()->NewKey(name, theMipmap, plLocation::kGlobalFixedLoc);

        return pyImage::New( theMipmap );
    }
    else
        PYTHON_RETURN_NONE;
}

#endif
