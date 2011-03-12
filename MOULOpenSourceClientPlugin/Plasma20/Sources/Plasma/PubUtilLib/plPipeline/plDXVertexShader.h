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

#ifndef plDXVertexSahder_inc
#define plDXVertexSahder_inc

#include "plDXShader.h"
#include "hsTemplates.h"

struct IDirect3DDevice9;
class plShader;
class plDXPipeline;

class plDXVertexShader : public plDXShader
{
protected:
	IDirect3DVertexShader9* fHandle;
	hsTArray<DWORD>&	IMakeDecl(hsTArray<DWORD>& decl) const;
	virtual HRESULT		ICreate(plDXPipeline* pipe);  // On error, sets error string.

	virtual HRESULT		ISetConstants(plDXPipeline* pipe); // On error, sets error string.

public:
	plDXVertexShader(plShader* owner);
	virtual ~plDXVertexShader();

	virtual void	Release();
	void			Link(plDXVertexShader** back) { plDXDeviceRef::Link((plDXDeviceRef**)back); }

	hsBool			VerifyFormat(UInt8 format) const;
	IDirect3DVertexShader9* GetShader(plDXPipeline* pipe);
};

#endif // plDXVertexSahder_inc