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

#ifndef plLODMipmap_inc
#define plLODMipmap_inc

#include "plMipmap.h"

class plLODMipmap : public plMipmap
{
protected:
	const enum 
	{
		kRefBase	= 0
	};
	const enum
	{
		kNumLODs	= 5
	};

	plMipmap*		fBase;
	int				fLOD;

	hsGDeviceRef*	fDeviceRefs[kNumLODs];

	void			ISetup();
	void			ISetupCurrLevel();
	void			IMarkDirty();
	void			INilify();


public:
	plLODMipmap();
	plLODMipmap(plMipmap* mip);
	virtual ~plLODMipmap();
	
	CLASSNAME_REGISTER( plLODMipmap );
	GETINTERFACE_ANY( plLODMipmap, plMipmap );

	virtual hsBool MsgReceive(plMessage *msg);

	void			SetLOD(int lod);
	int				GetLOD() const { return fLOD; }

	virtual hsGDeviceRef*	GetDeviceRef() const;
	virtual void			SetDeviceRef( hsGDeviceRef *const devRef );

	virtual void	Reset();

	virtual void	Read(hsStream *s, hsResMgr *mgr);
	virtual void	Write(hsStream *s, hsResMgr *mgr);

	virtual plMipmap*	Clone() const { return fBase->Clone(); }
	virtual void		CopyFrom(const plMipmap *source);

	virtual void	Composite(plMipmap *source, UInt16 x, UInt16 y, CompositeOptions *options = nil);

	virtual void	ScaleNicely(UInt32 *destPtr, UInt16 destWidth, UInt16 destHeight,
							UInt16 destStride, plMipmap::ScaleFilter filter) const;

	virtual hsBool	ResizeNicely(UInt16 newWidth, UInt16 newHeight, plMipmap::ScaleFilter filter);

	virtual void	SetCurrLevel(UInt8 level);

	const plMipmap* GetBase() const { return fBase; }
};


#endif // plLODMipmap_inc
