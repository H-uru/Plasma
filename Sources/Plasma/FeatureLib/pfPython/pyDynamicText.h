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
#ifndef _pyDynamicText_h_
#define _pyDynamicText_h_

//////////////////////////////////////////////////////////////////////
//
// pyDynamicText   - a wrapper class interface to plDynamicTextMsg
//
//////////////////////////////////////////////////////////////////////

class plDynamicTextMsg;
class pyColor;
class pyImage;
class pyKey;
namespace ST { class string; }

#include "pyGlueDefinitions.h"
#include "pnKeyedObject/plKey.h"

class pyDynamicText
{
private:
    plKey           fSenderKey;     // the holder of the who (the modifier) we are
    // the list of receivers that want to be notified
    std::vector<plKey> fReceivers;

    bool            fNetPropagate;
    bool            fNetForce;

    // clipping
    bool              fClip;
    uint16_t          fClipLeft;
    uint16_t          fClipTop;
    uint16_t          fClipRight;
    uint16_t          fClipBottom;

    // wrapping
    bool            fWrap;
    uint16_t          fWrapWidth;
    uint16_t          fWrapHeight;

    plDynamicTextMsg* ICreateDTMsg();

protected:
    pyDynamicText();
    pyDynamicText(const pyKey& key);
    pyDynamicText(plKey key);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptDynamicMap);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject* New(const pyKey& key);
    static PyObject* New(plKey key);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyDynamicText object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyDynamicText); // converts a PyObject to a pyDynamicText (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaConstantsClasses(PyObject *m);

// methods that will be exposed to Python
    // message stuff
    void SetSender(const pyKey& selfKey);
    void ClearReceivers();
    void AddReceiver(const pyKey& key);
    void SetNetPropagate(bool propagate);
    void SetNetForce(bool force);

    // dynamicText commands
    void ClearToColor(pyColor& color );
    void Flush();
    void SetTextColor(pyColor& color );
    void SetTextColor2(pyColor& color, bool blockRGB );
    void SetFont(ST::string facename, int16_t size);
    void FillRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, pyColor& color );
    void FrameRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, pyColor& color );
    void SetClipping(uint16_t clipLeft, uint16_t clipTop, uint16_t clipRight, uint16_t clipBottom);
    void UnsetClipping();
    void SetWrapping(uint16_t wrapWidth, uint16_t wrapHeight);
    void UnsetWrapping();
    void DrawText(int16_t x, int16_t y, const ST::string& text);
    void DrawImage(uint16_t x, uint16_t y, pyImage& image, bool respectAlpha);
    void DrawImageClipped(uint16_t x, uint16_t y, pyImage& image, uint16_t cx, uint16_t cy,
                          uint16_t cw, uint16_t ch, bool respectAlpha);
    void PurgeImage();

    // Actually return the visible width and height, since that's what you want to be drawing to
    uint16_t  GetWidth();
    uint16_t  GetHeight();
    void      CalcTextExtents(const ST::string& text, uint16_t& width, uint16_t& height);

    void      SetJustify(uint8_t justify);
    void      SetLineSpacing(int16_t spacing);

    plKey     GetImage();
};

#endif // _pyDynamicText_h_
