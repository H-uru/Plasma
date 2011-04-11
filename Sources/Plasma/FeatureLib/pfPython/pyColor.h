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
#ifndef _pyColor_h_
#define _pyColor_h_

//////////////////////////////////////////////////////////////////////
//
// pyColor   - the wrapper class for hsColorRGBA structure
//
//////////////////////////////////////////////////////////////////////

#include "hsColorRGBA.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyColor
{
private:
	hsColorRGBA		fColor;

protected:
	pyColor(hsScalar r=0.0f, hsScalar g=0.0f, hsScalar b=0.0f, hsScalar a=0.0f);
	pyColor(hsColorRGBA color);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptColor);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(hsScalar red, hsScalar green, hsScalar blue, hsScalar alpha = 0.0f);
	static PyObject *New(const hsColorRGBA & color);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyColor object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyColor); // converts a PyObject to a pyColor (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// non-python interface
	hsColorRGBA  getColor() { return fColor; }
	void setColor(hsColorRGBA color) { fColor = color; }
	
	// python get attributes helpers
	hsScalar	getRed() const { return fColor.r; }
	hsScalar	getGreen() const { return fColor.g; }
	hsScalar	getBlue() const { return fColor.b; }
	hsScalar	getAlpha() const { return fColor.a; }

	// python set attributes helpers
	void	setRed(hsScalar red) { fColor.r = red; }
	void	setGreen(hsScalar green) { fColor.g = green; }
	void	setBlue(hsScalar blue) { fColor.b = blue; }
	void	setAlpha(hsScalar alpha) { fColor.a = alpha; }

	// override the equals to operator
	hsBool operator==(const pyColor &color) const
	{
		return ((pyColor*)this)->getColor() == ((pyColor&)color).getColor();
	}
	hsBool operator!=(const pyColor &color) const { return !(color == *this); }

	// helper colors settings
	void White()	{ fColor.Set(1.0,1.0,1.0,1.0); }
	void Black()	{ fColor.Set(0.0,0.0,0.0,1.0); }
	void Red()		{ fColor.Set(1.0,0.0,0.0,1.0); }
	void Green()	{ fColor.Set(0.0,1.0,0.0,1.0); }
	void Blue()		{ fColor.Set(0.0,0.0,1.0,1.0); }
	void Magenta()	{ fColor.Set(1.0,0.0,1.0,1.0); }
	void Cyan()		{ fColor.Set(0.0,1.0,1.0,1.0); }
	void Yellow()	{ fColor.Set(1.0,1.0,0.0,1.0); }

	void Brown()		{ fColor.Set(0.65, 0.165,0.165,1.0); }
	void Gray()			{ fColor.Set(0.75, 0.75, 0.75, 1.0); }
	void Orange()		{ fColor.Set(1.0,  0.5,  0.0,  1.0); }
	void Pink()			{ fColor.Set(0.73, 0.56, 0.56, 1.0); }
	void DarkBrown()	{ fColor.Set(0.36, 0.25, 0.20, 1.0); }
	void DarkGreen()	{ fColor.Set(0.18, 0.31, 0.18, 1.0); }
	void DarkPurple()	{ fColor.Set(0.53, 0.12, 0.47, 1.0); }
	void NavyBlue()		{ fColor.Set(0.137,0.137,0.557,1.0); }
	void Maroon()		{ fColor.Set(0.557,0.137,0.42, 1.0); }
	void Tan()			{ fColor.Set(0.858,0.576,0.439,1.0); }
	void SlateBlue()	{ fColor.Set(0.0,  0.495,1.0,  1.0); }
	void SteelBlue()	{ fColor.Set(0.137,0.42, 0.557,1.0); }

};


#endif // _pyColor_h_
