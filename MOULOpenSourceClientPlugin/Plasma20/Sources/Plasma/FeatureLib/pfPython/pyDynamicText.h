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
#ifndef _pyDynamicText_h_
#define _pyDynamicText_h_

//////////////////////////////////////////////////////////////////////
//
// pyDynamicText   - a wrapper class interface to plDynamicTextMsg
//
//////////////////////////////////////////////////////////////////////

class plDynamicTextMsg;
class pyKey;
class pyColor;
class pyImage;

#include "hsTemplates.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pyDynamicText
{
private:
	plKey			fSenderKey;		// the holder of the who (the modifier) we are
	// the list of receivers that want to be notified
	hsTArray<plKey> fReceivers;

	hsBool			fNetPropagate;
	hsBool			fNetForce;

	// clipping
	hsBool			fClip;
	UInt16			fClipLeft;
	UInt16			fClipTop;
	UInt16			fClipRight;
	UInt16			fClipBottom;

	// wrapping
	hsBool			fWrap;
	UInt16			fWrapWidth;
	UInt16			fWrapHeight;

	virtual plDynamicTextMsg* ICreateDTMsg();
	virtual void IInit();

protected:
	pyDynamicText();
	pyDynamicText(pyKey& key);
	pyDynamicText(plKey key);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptDynamicMap);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(pyKey& key);
	static PyObject *New(plKey key);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyDynamicText object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyDynamicText); // converts a PyObject to a pyDynamicText (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void	AddPlasmaConstantsClasses(PyObject *m);

// methods that will be exposed to Python
	// message stuff
	virtual void SetSender(pyKey& selfKey);
	virtual void ClearReceivers();
	virtual void AddReceiver(pyKey& key);
	virtual void SetNetPropagate(hsBool propagate);
	virtual void SetNetForce(hsBool force);

	// dynamicText commands
	virtual void ClearToColor( pyColor& color );
	virtual void Flush( void );
	virtual void SetTextColor( pyColor& color );
	virtual void SetTextColor2( pyColor& color, bool blockRGB );
	virtual void SetFont( const char *facename, Int16 size );
	virtual void FillRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, pyColor& color );
	virtual void FrameRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, pyColor& color );
	virtual void SetClipping( UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom);
	virtual void UnsetClipping();
	virtual void SetWrapping( UInt16 wrapWidth, UInt16 wrapHeight );
	virtual void UnsetWrapping();
	virtual void DrawText( Int16 x, Int16 y, const char *text );
	virtual void DrawTextW( Int16 x, Int16 y, std::wstring text );
	virtual	void DrawImage( UInt16 x, UInt16 y, pyImage& image, hsBool respectAlpha );
	virtual void DrawImageClipped( UInt16 x, UInt16 y, pyImage& image, UInt16 cx, UInt16 cy, UInt16 cw, UInt16 ch,
										hsBool respectAlpha );
	virtual void PurgeImage( void );

	// Actually return the visible width and height, since that's what you want to be drawing to
	virtual UInt16	GetWidth( void );
	virtual UInt16	GetHeight( void );
	virtual void CalcTextExtents( std::wstring text, unsigned &width, unsigned &height );
	
	virtual void SetJustify(UInt8 justify);
	virtual void SetLineSpacing(Int16 spacing);

	virtual plKey GetImage();
};

#endif // _pyDynamicText_h_
