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

#include "plDXVertexShader.h"

#include "HeadSpin.h"
#include "hsWindows.h"
#include <d3d9.h>

#include "plDXPipeline.h"

#include "plDrawable/plGBufferGroup.h"
#include "plSurface/plShader.h"

plDXVertexShader::plDXVertexShader(plShader* owner)
:   plDXShader(owner), fHandle()
{
}

plDXVertexShader::~plDXVertexShader()
{
    Release();
}

void plDXVertexShader::Release()
{
    ReleaseObject(fHandle);
    fHandle = nullptr;
    fPipe = nullptr;
    fErrorString.clear();
}

bool plDXVertexShader::VerifyFormat(uint8_t format) const
{
    return (fOwner->GetInputFormat() & format) == fOwner->GetInputFormat();
}

IDirect3DVertexShader9 *plDXVertexShader::GetShader(plDXPipeline* pipe)
{
    HRESULT hr = S_OK;
    if ( !fHandle )
    {
        if( FAILED(hr = ICreate(pipe)) )
            return nullptr;
    }

    if( FAILED(hr = ISetConstants(pipe)) )
        return nullptr;

    return fHandle;
}

HRESULT plDXVertexShader::ICreate(plDXPipeline* pipe)
{
    fHandle = nullptr; // in case something goes wrong.
    fPipe = nullptr;
    fErrorString.clear();

    // We could store the compiled buffer and skip the assembly step
    // if we need to recreate the shader (e.g. on device lost).
    // But whatever.
    DWORD* shaderCodes = (DWORD*)(fOwner->GetDecl()->GetCodes());

    if( !shaderCodes )
        return IOnError(-1, ST_LITERAL("Shaders must be compiled into the engine."));

    HRESULT hr = pipe->GetD3DDevice()->CreateVertexShader(shaderCodes, &fHandle);

    if( FAILED(hr) )
        return IOnError(hr, ST_LITERAL("Error on CreateVertexShader"));

    hsAssert(fHandle, "No error, but no vertex shader handle. Grrrr.");

    fPipe = pipe;

    return S_OK;
}

HRESULT plDXVertexShader::ISetConstants(plDXPipeline* pipe)
{
    hsAssert(fHandle, "Vertex shader called to set constants without initialization");
    if( fOwner->GetNumConsts() )
    {
        HRESULT hr = pipe->GetD3DDevice()->SetVertexShaderConstantF(0,
                                        (const float*)fOwner->GetConstBasePtr(),
                                        fOwner->GetNumConsts());
        if( FAILED(hr) )
            return IOnError(hr, ST_LITERAL("Failure setting vertex shader constants"));
    }

    return S_OK;
}

