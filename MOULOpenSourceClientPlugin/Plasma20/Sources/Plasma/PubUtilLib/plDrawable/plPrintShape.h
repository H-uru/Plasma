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

#ifndef plPrintShape_inc
#define plPrintShape_inc

#include "../pnSceneObject/plObjInterface.h"

class plPrintShape : public plObjInterface
{
protected:
	hsScalar			fWidth;
	hsScalar			fLength;
	hsScalar			fHeight;
public:
	plPrintShape();
	virtual ~plPrintShape();

	CLASSNAME_REGISTER( plPrintShape );
	GETINTERFACE_ANY( plPrintShape, plObjInterface );


	virtual Int32   GetNumProperties() const { return 1; }
	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	hsScalar GetWidth() const { return fWidth; }
	hsScalar GetLength() const { return fLength; }
	hsScalar GetHeight() const { return fHeight; }

	void SetWidth(hsScalar f) { fWidth = f; }
	void SetLength(hsScalar f) { fLength = f; }
	void SetHeight(hsScalar f) { fHeight = f; }
};

#endif // plPrintShape_inc
