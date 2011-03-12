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
#ifndef pyImage_h
#define pyImage_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyImage
//
// PURPOSE: Class wrapper for Python to a plMipMap image
//

#include "hsStlUtils.h"

#include "pyKey.h"
#include "pyColor.h"

#ifndef BUILDING_PYPLASMA
#include "pyGeometry3.h"
#include "../plGImage/plMipmap.h"
#endif

#include <python.h>
#include "pyGlueHelpers.h"

class plKey;

class pyImage
{
protected:
	plKey			fMipMapKey;
#ifndef BUILDING_PYPLASMA
	plMipmap*		fMipmap;
#endif

	pyImage() // for python glue only, do NOT call
	{
		fMipMapKey = nil;
#ifndef BUILDING_PYPLASMA
		fMipmap = nil;
#endif
	} 

	// Constructor from C++
	pyImage(plKey mipmapKey)
	{
		fMipMapKey = mipmapKey;
#ifndef BUILDING_PYPLASMA
		fMipmap = nil;
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

	// contructor from Python
	pyImage(pyKey& mipmapKey)
	{
		fMipMapKey = mipmapKey.getKey();
#ifndef BUILDING_PYPLASMA
		fMipmap = nil;
#endif
	}

public:
#ifndef BUILDING_PYPLASMA
	pyImage::~pyImage()
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
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);

	void setKey(pyKey& mipmapKey) // only for python glue, do NOT call
	{
#ifndef BUILDING_PYPLASMA
		if (fMipmap && fMipMapKey)
			fMipMapKey->UnRefObject();
		fMipmap = nil;
#endif
		fMipMapKey = mipmapKey.getKey();
	}

	// override the equals to operator
	hsBool operator==(const pyImage &image) const
	{
		// only thing that needs testing is the plKey, which is unique for all
		if ( fMipMapKey == ((pyImage&)image).GetKey() )
			return true;
		else
			return false;
	}
	hsBool operator!=(const pyImage &image) const { return !(image == *this);	}

	// for C++ access
	plKey GetKey() { return fMipmap ? fMipmap->GetKey() : fMipMapKey; }
#ifndef BUILDING_PYPLASMA
	plMipmap* GetImage();

	// for python access
	PyObject *GetPixelColor(float x, float y); // returns the color at a specific x,y position (x and y from 0 to 1) - returns pyColor
	PyObject *GetColorLoc(const pyColor &color); // returns the x,y position of a color (x and y from 0 to 1) - returns pyPoint3
	UInt32 GetWidth(); // returns the width of the image
	UInt32 GetHeight(); // returns the height of the image
	void SaveAsJPEG(const wchar* fileName, UInt8 quality = 75);
	static PyObject* LoadJPEGFromDisk(const wchar* filename, UInt16 width, UInt16 height); // returns pyImage
#endif
};

#endif  // pyImage_h
