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
#ifndef _plDXTextFont_h
#define _plDXTextFont_h

#include "plTextFont.h"
//#include "hsGDirect3DTnLDevice.h"


//// plDXTextFont Class Definition ///////////////////////////////////////////

struct IDirect3DTexture9;
struct IDirect3DDevice9;
struct IDirect3DStateBlock9;
class plPipeline;

class plDXTextFont : public plTextFont
{
protected:
	IDirect3DTexture9			*fD3DTexture;
	IDirect3DDevice9			*fDevice; 

	static IDirect3DVertexBuffer9		*fBuffer;
	static UInt32						fBufferCursor;

	IDirect3DStateBlock9 *fOldStateBlock;
	IDirect3DStateBlock9 *fTextStateBlock;

	virtual void	ICreateTexture( UInt16 *data );
	virtual void	IInitStateBlocks( void );
	virtual void	IDrawPrimitive( UInt32 count, plFontVertex *array );
	virtual void	IDrawLines( UInt32 count, plFontVertex *array );

public:
	plDXTextFont( plPipeline *pipe, IDirect3DDevice9 *device );
	~plDXTextFont();

	static	void CreateShared(IDirect3DDevice9* device);
	static	void ReleaseShared(IDirect3DDevice9* device);

	virtual void	FlushDraws( void );
	virtual void	SaveStates( void );
	virtual void	RestoreStates( void );
	virtual void	DestroyObjects( void );

	static const DWORD kFVF;
};


#endif // _plDXTextFont_h

