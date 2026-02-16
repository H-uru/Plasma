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

// Compiles Direct3D 8/9 shader assembly source code to bytecode
// and outputs the shader bytecode as C++ declarations
// that can be included into Plasma's plShaderTable.

// To recompile Plasma's shaders:
// PS> cd Sources\Plasma\PubUtilLib\plSurface\ShaderSrc
// PS> plShaderAssembler all
// PS> Move-Item -Force *.h ../

#include "HeadSpin.h"
#include "plFileSystem.h"
#include "hsStream.h"

#include "hsWindows.h"
#include <d3d9.h>
#include <D3Dcompiler.h>

#include <memory>

#include <string_theory/format>
#include <string_theory/stdio>

static constexpr char kLicenseHeader[] =
    "/*==LICENSE==*\n"
    "\n"
    "CyanWorlds.com Engine - MMOG client, server and tools\n"
    "Copyright (C) 2011  Cyan Worlds, Inc.\n"
    "\n"
    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
    "\n"
    "Additional permissions under GNU GPL version 3 section 7\n"
    "\n"
    "If you modify this Program, or any covered work, by linking or\n"
    "combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,\n"
    "NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent\n"
    "JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK\n"
    "(or a modified version of those libraries),\n"
    "containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,\n"
    "PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG\n"
    "JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the\n"
    "licensors of this Program grant you additional\n"
    "permission to convey the resulting work. Corresponding Source for a\n"
    "non-source form of such a combination shall include the source code for\n"
    "the parts of OpenSSL and IJG JPEG Library used as well as that of the covered\n"
    "work.\n"
    "\n"
    "You can contact Cyan Worlds, Inc. by email legal@cyan.com\n"
    " or by snail mail at:\n"
    "      Cyan Worlds, Inc.\n"
    "      14617 N Newport Hwy\n"
    "      Mead, WA   99021\n"
    "\n"
    "*==LICENSE==*/\n";

class plDXShaderError
{
    ST::string fError;

public:
    plDXShaderError(ST::string msg)
        : fError(std::move(msg))
    { }

    template<typename... _Args>
    plDXShaderError(const char* fmt, _Args&&... args)
        : fError(ST::format(fmt, std::forward<_Args>(args)...))
    { }

    operator ST::string() const { return fError; }
    const char* c_str() const { return fError.c_str(); }
};

// ===================================================

class plDXShaderAssembler
{
    typedef HRESULT (WINAPI *D3DAssemble)(LPCVOID data, SIZE_T datasize, LPCSTR filename,
                                          const D3D_SHADER_MACRO* defines, ID3DInclude* include,
                                          UINT flags, ID3DBlob** shader, ID3DBlob** error_messages);

    HMODULE fLibrary;
    D3DAssemble fFuncPtr;

public:
    plDXShaderAssembler();
    ~plDXShaderAssembler();

    std::shared_ptr<ID3DBlob> AssShader(const char* shader, unsigned int shaderLen) const;
};

// ===================================================

plDXShaderAssembler::plDXShaderAssembler()
    : fLibrary(), fFuncPtr()
{
    fLibrary = LoadLibraryW(D3DCOMPILER_DLL_W);
    if (!fLibrary)
        throw plDXShaderError("Unable to load library '{}'", D3DCOMPILER_DLL_W);
    fFuncPtr = (D3DAssemble)GetProcAddress(fLibrary, "D3DAssemble");
    if (!fFuncPtr) {
        FreeLibrary(fLibrary);
        throw plDXShaderError("Unable to get D3DAssemble proc in library '{}'", D3DCOMPILER_DLL_W);
    }
}

plDXShaderAssembler::~plDXShaderAssembler()
{
    if (fLibrary)
        FreeLibrary(fLibrary);
}

static inline void IRelease(IUnknown* p)
{
    if (p)
        p->Release();
}

std::shared_ptr<ID3DBlob> plDXShaderAssembler::AssShader(const char* shader, unsigned int shaderLen) const
{
    ID3DBlob* compiled = nullptr;
    ID3DBlob* errors = nullptr;
    hsCOMError hr = fFuncPtr(shader, shaderLen, nullptr, nullptr, nullptr, 0, &compiled, &errors);
    if (SUCCEEDED(hr))
        return std::shared_ptr<ID3DBlob>(compiled, IRelease);

    ST::string strErrors(errors ? reinterpret_cast<const char*>(errors->GetBufferPointer()) : "Unspecified");
    IRelease(compiled);
    IRelease(errors);

    throw plDXShaderError("Error compiling shader: {}\n{}\n", hr, strErrors);
}

// ===================================================

// Temporary helper to reproduce the silly formatting
// caused by tabs-to-spaces conversion on the original plShaderAssembler output...
static ST::string IRightPadHexByte(uint8_t byte)
{
    if (byte < 0x10) {
        return ST::format("0x{x},    ", byte);
    } else {
        return ST::format("0x{x},   ", byte);
    }
}

void ICreateHeader(const ST::string& varName, const plFileName& fileName, FILE* fp, ID3DBlob* shader)
{
    fputs(kLicenseHeader, fp);
    fputs("\n\n", fp);

    hsSsize_t byteLen = shader->GetBufferSize();
    hsSsize_t quadLen = byteLen >> 2;

    unsigned char* codes = (unsigned char*)shader->GetBufferPointer();

    ST::printf(fp, "static const uint32_t {}byteLen = {};\n\n", varName, byteLen);
    ST::printf(fp, "static const uint8_t {}Codes[] = {{\n", varName);

    for (hsSsize_t i = 0; i < quadLen-1; i++)
    {
        ST::printf(fp, "    {}", IRightPadHexByte(*codes++));
        ST::printf(fp, "{}", IRightPadHexByte(*codes++));
        ST::printf(fp, "{}", IRightPadHexByte(*codes++));
        ST::printf(fp, "0x{x},\n", *codes++);
    }
    ST::printf(fp, "    {}", IRightPadHexByte(*codes++));
    ST::printf(fp, "{}", IRightPadHexByte(*codes++));
    ST::printf(fp, "{}", IRightPadHexByte(*codes++));
    ST::printf(fp, "0x{x}\n", *codes++);
    fputs("    };", fp);
    fputs("\n\n", fp);

    ST::printf(fp, "static const plShaderDecl {}Decl(\"{}\", {}, {}byteLen, {}Codes);\n\n",
               varName, fileName, varName, varName, varName);
    ST::printf(fp, "static const plShaderRegister {}Register(&{}Decl);\n\n", varName, varName);
}

static void IAssShader(const plDXShaderAssembler& ass, const char* name)
{
    ST::string varName = plFileName(name).StripFileExt().AsString();

    plFileName inFile = ST::format("{}.inl", varName);
    plFileName outFile = ST::format("{}.h", varName);

    ST::printf("Processing {} into {}\n", name, outFile);
    hsUNIXStream outFp;
    // Write in binary mode to get LF line endings even on Windows.
    if (!outFp.Open(outFile, "wb")) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        ST::printf(stderr, "Error opening file {} for output\n", outFile);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        return;
    }

    hsUNIXStream inFp;
    if (!inFp.Open(inFile, "r")) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        ST::printf(stderr, "Error opening file {} for input\n", inFile);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        return;
    }

    uint32_t shaderCodeLen = inFp.GetEOF();
    auto shaderCode = std::make_unique<char[]>(shaderCodeLen);
    inFp.Read(shaderCodeLen, shaderCode.get());

    try {
        auto compiledShader = ass.AssShader(shaderCode.get(), shaderCodeLen);
        ICreateHeader(varName, ST::format("sha/{}.inl", varName), outFp.GetFILE(), compiledShader.get());
    } catch (const plDXShaderError& error) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        fputs(error.c_str(), stderr);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
    }
}

int main(int argc, char* argv[])
{
    if( argc < 2 )
    {
        ST::printf("{} <file0> <file1> ...\n", argv[0]);
        return 0;
    }

    const char* const * nameList = nullptr;
    int numNames = 0;
    if (stricmp(argv[1], "all") == 0)
    {
        // Just copy in the enum and use Replace on
        // vs_ => "vs_
        // ps_ => "ps_
        // , => ",
        static const char* kEnumNames[] = {
            "vs_WaveFixedFin6",
            "ps_WaveFixed",
            "vs_CompCosines",
            "ps_CompCosines",
            "vs_ShoreLeave6",
            "ps_ShoreLeave6",
            "vs_WaveRip",
            "ps_WaveRip",
            "vs_WaveDec1Lay",
            "vs_WaveDec2Lay11",
            "vs_WaveDec2Lay12",
            "vs_WaveDecEnv",
            "ps_CbaseAbase",
            "ps_CalphaAbase",
            "ps_CalphaAMult",
            "ps_CalphaAadd",
            "ps_CaddAbase",
            "ps_CaddAMult",
            "ps_CaddAAdd",
            "ps_CmultAbase",
            "ps_CmultAMult",
            "ps_CmultAAdd",
            "ps_WaveDecEnv",
            "vs_WaveGraph2",
            "ps_WaveGraph",
            "vs_WaveGridFin",
            "ps_WaveGrid",
            "vs_BiasNormals",
            "ps_BiasNormals",
            "vs_ShoreLeave7",
            "vs_WaveRip7",
            "ps_MoreCosines",
            "vs_WaveDec1Lay_7",
            "vs_WaveDec2Lay11_7",
            "vs_WaveDec2Lay12_7",
            "vs_WaveDecEnv_7",
            "vs_WaveFixedFin7",
            "vs_GrassShader",
            "ps_GrassShader"
        };

        nameList = kEnumNames;
        numNames = std::size(kEnumNames);
    }
    else
    {
        nameList = argv+1;
        numNames = argc-1;
    }

    try {
        plDXShaderAssembler ass;
        for (int i = 0; i < numNames; i++ ) {
            IAssShader(ass, nameList[i]);
        }
    } catch (const plDXShaderError& error) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        fputs(error.c_str(), stderr);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        return 1;
    }

    return 0;
}

