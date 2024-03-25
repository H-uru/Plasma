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
#ifndef _pyColor_h_
#define _pyColor_h_

//////////////////////////////////////////////////////////////////////
//
// pyColor   - the wrapper class for hsColorRGBA structure
//
//////////////////////////////////////////////////////////////////////

#include "hsColorRGBA.h"

#include "pyGlueDefinitions.h"

class pyColor
{
private:
    hsColorRGBA     fColor;

protected:
    pyColor(float r=0.0f, float g=0.0f, float b=0.0f, float a=0.0f);
    pyColor(hsColorRGBA color);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptColor);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(float red, float green, float blue, float alpha = 0.0f);
    static PyObject *New(const hsColorRGBA & color);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyColor object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyColor); // converts a PyObject to a pyColor (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // non-python interface
    hsColorRGBA  getColor() { return fColor; }
    void setColor(hsColorRGBA color) { fColor = color; }
    
    // python get attributes helpers
    float    getRed() const { return fColor.r; }
    float    getGreen() const { return fColor.g; }
    float    getBlue() const { return fColor.b; }
    float    getAlpha() const { return fColor.a; }

    // python set attributes helpers
    void    setRed(float red) { fColor.r = red; }
    void    setGreen(float green) { fColor.g = green; }
    void    setBlue(float blue) { fColor.b = blue; }
    void    setAlpha(float alpha) { fColor.a = alpha; }

    // override the equals to operator
    bool operator==(const pyColor &color) const
    {
        return ((pyColor*)this)->getColor() == ((pyColor&)color).getColor();
    }
    bool operator!=(const pyColor &color) const { return !(color == *this); }

    // helper colors settings
    void White()    { fColor.Set(1.f, 1.f, 1.f, 1.f); }
    void Black()    { fColor.Set(0.f, 0.f, 0.f, 1.f); }
    void Red()      { fColor.Set(1.f, 0.f, 0.f, 1.f); }
    void Green()    { fColor.Set(0.f, 1.f, 0.f, 1.f); }
    void Blue()     { fColor.Set(0.f, 0.f, 1.f, 1.f); }
    void Magenta()  { fColor.Set(1.f, 0.f, 1.f, 1.f); }
    void Cyan()     { fColor.Set(0.f, 1.f, 1.f, 1.f); }
    void Yellow()   { fColor.Set(1.f, 1.f, 0.f, 1.f); }

    void Brown()        { fColor.Set(0.65f,  0.165f, 0.165f, 1.f); }
    void Gray()         { fColor.Set(0.75f,  0.75f,  0.75f,  1.f); }
    void Orange()       { fColor.Set(1.0f,   0.5f,   0.0f,   1.f); }
    void Pink()         { fColor.Set(0.73f,  0.56f,  0.56f,  1.f); }
    void DarkBrown()    { fColor.Set(0.36f,  0.25f,  0.20f,  1.f); }
    void DarkGreen()    { fColor.Set(0.18f,  0.31f,  0.18f,  1.f); }
    void DarkPurple()   { fColor.Set(0.53f,  0.12f,  0.47f,  1.f); }
    void NavyBlue()     { fColor.Set(0.137f, 0.137f, 0.557f, 1.f); }
    void Maroon()       { fColor.Set(0.557f, 0.137f, 0.42f,  1.f); }
    void Tan()          { fColor.Set(0.858f, 0.576f, 0.439f, 1.f); }
    void SlateBlue()    { fColor.Set(0.0f,   0.495f, 1.0f,   1.f); }
    void SteelBlue()    { fColor.Set(0.137f, 0.42f,  0.557f, 1.f); }

};


#endif // _pyColor_h_
