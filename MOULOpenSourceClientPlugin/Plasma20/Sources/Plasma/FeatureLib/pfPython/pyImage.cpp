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
#include "pyImage.h"

#include "cyMisc.h"

#ifndef BUILDING_PYPLASMA
#include "../plGImage/plMipmap.h"
#endif

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
		UInt32 height = image->GetHeight();
		UInt32 width = image->GetWidth();
		UInt32 iX = (UInt32)((float)width * x);
		UInt32 iY = (UInt32)((float)height * y);
		hsColorRGBA pixColor;
		image->SetCurrLevel(0);
		UInt32 *color = image->GetAddr32(iX,iY);
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
		UInt32 height = image->GetHeight();
		UInt32 width = image->GetWidth();
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
					return pyPoint3::New(hsPoint3(fX, fY, 0));
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
UInt32 pyImage::GetWidth()
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
UInt32 pyImage::GetHeight()
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

#include "../plJPEG/plJPEG.h"
void pyImage::SaveAsJPEG(const wchar* fileName, UInt8 quality)
{
	if (quality <= 0 || quality > 100)
	{
			quality = 75;
	}

	plJPEG::Instance().SetWriteQuality( quality );
	plJPEG::Instance().WriteToFile( fileName, this->GetImage() );
}

#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"
PyObject* pyImage::LoadJPEGFromDisk(const wchar* filename, UInt16 width, UInt16 height)
{
	plMipmap* theMipmap = plJPEG::Instance().ReadFromFile(filename);
	if (theMipmap)
	{
		if (width > 0 && height > 0)
		{
			if (!theMipmap->ResizeNicely(width, height, plMipmap::ScaleFilter::kDefaultFilter))
			{
				delete theMipmap;
				PYTHON_RETURN_NONE;
			}
		}

		// let's create a nice name for this thing based on the filename
		std::string name = "PtImageFromDisk_";
		const wchar* i = filename;
		int charsChecked = 0;

		while (*i != '\\' && *i != '\0' && charsChecked < 1024)
		{
			i++;
			charsChecked++;
		}
		
		if (*i == '\0')
		{
			i = filename;
		}
		else
		{
			i++;
		}

		char* cName = hsWStringToString(i);
		name = name + cName;

		hsgResMgr::ResMgr()->NewKey(name.c_str(), theMipmap, plLocation::kGlobalFixedLoc);
		
		return pyImage::New( theMipmap );
	}
	else
		PYTHON_RETURN_NONE;
}

#endif
