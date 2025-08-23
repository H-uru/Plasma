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
#ifndef pyImage_h
#define pyImage_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyImage
//
// PURPOSE: Class wrapper for Python to a plMipMap image
//

#include <map>

#include "pnKeyedObject/plKey.h"

#ifndef BUILDING_PYPLASMA
#   include "plGImage/plMipmap.h"
#endif

#include "pyGlueDefinitions.h"

class plFileName;
class pyColor;
class pyKey;

class pyImage
{
protected:
    plKey           fMipMapKey;
#ifndef BUILDING_PYPLASMA
    plMipmap*       fMipmap;
#endif

    pyImage() // for python glue only, do NOT call
    {
        fMipMapKey = nullptr;
#ifndef BUILDING_PYPLASMA
        fMipmap = nullptr;
#endif
    } 

    // Constructor from C++
    pyImage(plKey mipmapKey)
    {
        fMipMapKey = std::move(mipmapKey);
#ifndef BUILDING_PYPLASMA
        fMipmap = nullptr;
#endif
    }

#ifndef BUILDING_PYPLASMA
    // Constructor from C++ ... use pointer to instead of plKey
    pyImage(plMipmap* mipmap)
    {
        fMipmap = mipmap;
        fMipMapKey = fMipmap->GetKey();

        if (fMipMapKey)
        {
            fMipMapKey->RefObject();
        }
    }
#endif

public:
#ifndef BUILDING_PYPLASMA
    ~pyImage()
    {
        if (fMipmap && fMipMapKey)
            fMipMapKey->UnRefObject();
    }
#endif

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptImage);
#ifndef BUILDING_PYPLASMA
    static PyObject *New(plMipmap* mipmap);
#endif
    static PyObject *New(plKey mipmapKey);
    static PyObject *New(pyKey& mipmapKey);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyImage object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyImage); // converts a PyObject to a pyImage (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(PyObject* m);

    void setKey(pyKey& mipmapKey);

    // override the equals to operator
    bool operator==(const pyImage &image) const
    {
        // only thing that needs testing is the plKey, which is unique for all
        if ( fMipMapKey == ((pyImage&)image).GetKey() )
            return true;
        else
            return false;
    }
    bool operator!=(const pyImage &image) const { return !(image == *this);   }

    // for C++ access
    plKey GetKey() { return fMipmap ? fMipmap->GetKey() : fMipMapKey; }
#ifndef BUILDING_PYPLASMA
    plMipmap* GetImage();

    // for python access
    PyObject *GetPixelColor(float x, float y); // returns the color at a specific x,y position (x and y from 0 to 1) - returns pyColor
    PyObject *GetColorLoc(const pyColor &color); // returns the x,y position of a color (x and y from 0 to 1) - returns pyPoint3
    uint32_t GetWidth(); // returns the width of the image
    uint32_t GetHeight(); // returns the height of the image
    void SaveAsJPEG(const plFileName& fileName, uint8_t quality = 75);
    void SaveAsPNG(const plFileName& fileName, const std::multimap<ST::string, ST::string>& textFields = std::multimap<ST::string, ST::string>());

    static PyObject* Find(const ST::string& name);
    static PyObject* LoadJPEGFromDisk(const plFileName& filename, uint16_t width, uint16_t height); // returns pyImage
    static PyObject* LoadPNGFromDisk(const plFileName& filename, uint16_t width, uint16_t height); // returns pyImage
#endif
};

#endif  // pyImage_h
