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
// > cd Sources\Plasma\PubUtilLib\plSurface
// > plShaderAssembler all

#include "HeadSpin.h"
#include "plFileSystem.h"
#include "hsMain.inl"
#include "hsStream.h"

#include "hsWindows.h"
#include <d3d9.h>
#include <D3Dcompiler.h>

#include <memory>

#include <string_theory/format>
#include <string_theory/stdio>

using namespace ST::literals;

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

void ICreateHeader(const ST::string& varName, const plFileName& fileName, FILE* fp, ID3DBlob* shader)
{
    fputs(kLicenseHeader, fp);
    fputs("\n", fp);
    ST::printf(fp, "#ifndef {}_inc\n", varName);
    ST::printf(fp, "#define {}_inc\n", varName);
    fputs("\n", fp);

    SIZE_T byteLen = shader->GetBufferSize();
    hsAssert(byteLen % 4 == 0, "We expect Direct3D shader bytecode to be a multiple of 4 bytes!");

    unsigned char* codes = (unsigned char*)shader->GetBufferPointer();

    ST::printf(fp, "static const uint8_t {}Codes[] = {{\n", varName);

    for (SIZE_T i = 0; i < byteLen; i += 4) {
        ST::printf(
            fp,
            "    0x{>02x}, 0x{>02x}, 0x{>02x}, 0x{>02x},\n",
            codes[i], codes[i+1], codes[i+2], codes[i+3]
        );
    }
    fputs("};\n\n", fp);

    ST::printf(fp, "static const plShaderDecl {}Decl(\"{}\", {}, sizeof({}Codes), {}Codes);\n\n",
               varName, fileName, varName, varName, varName);
    ST::printf(fp, "static const plShaderRegister {}Register(&{}Decl);\n\n", varName, varName);

    ST::printf(fp, "#endif // {}_inc\n", varName);
}

static void IAssShader(const plDXShaderAssembler& ass, const ST::string& name)
{
    ST::string varName = plFileName(name).StripFileExt().AsString();

    plFileName inFile = ST::format("ShaderSrc/{}.inl", varName);
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
        ICreateHeader(varName, inFile, outFp.GetFILE(), compiledShader.get());
    } catch (const plDXShaderError& error) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        fputs(error.c_str(), stderr);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
    }
}

int hsMain(std::vector<ST::string> args)
{
    if (args.size() < 2) {
        ST::printf("{} <file0> <file1> ...\n", args[0]);
        return 0;
    }

    std::vector<ST::string> nameList;
    if (args[1].compare_i("all") == 0) {
        // Just copy in the enum and use Replace on
        // vs_ => "vs_
        // ps_ => "ps_
        // , => "_st,
        nameList = {
            "vs_WaveFixedFin6"_st,
            "ps_WaveFixed"_st,
            "vs_CompCosines"_st,
            "ps_CompCosines"_st,
            "vs_ShoreLeave6"_st,
            "ps_ShoreLeave6"_st,
            "vs_WaveRip"_st,
            "ps_WaveRip"_st,
            "vs_WaveDec1Lay"_st,
            "vs_WaveDec2Lay11"_st,
            "vs_WaveDec2Lay12"_st,
            "vs_WaveDecEnv"_st,
            "ps_CbaseAbase"_st,
            "ps_CalphaAbase"_st,
            "ps_CalphaAMult"_st,
            "ps_CalphaAadd"_st,
            "ps_CaddAbase"_st,
            "ps_CaddAMult"_st,
            "ps_CaddAAdd"_st,
            "ps_CmultAbase"_st,
            "ps_CmultAMult"_st,
            "ps_CmultAAdd"_st,
            "ps_WaveDecEnv"_st,
            "vs_WaveGraph2"_st,
            "ps_WaveGraph"_st,
            "vs_WaveGridFin"_st,
            "ps_WaveGrid"_st,
            "vs_BiasNormals"_st,
            "ps_BiasNormals"_st,
            "vs_ShoreLeave7"_st,
            "vs_WaveRip7"_st,
            "ps_MoreCosines"_st,
            "vs_WaveDec1Lay_7"_st,
            "vs_WaveDec2Lay11_7"_st,
            "vs_WaveDec2Lay12_7"_st,
            "vs_WaveDecEnv_7"_st,
            "vs_WaveFixedFin7"_st,
            "vs_GrassShader"_st,
            "ps_GrassShader"_st,
        };
    } else {
        nameList.assign(args.begin() + 1, args.end());
    }

    try {
        plDXShaderAssembler ass;
        for (const auto& name : nameList) {
            IAssShader(ass, name);
        }
    } catch (const plDXShaderError& error) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        fputs(error.c_str(), stderr);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        return 1;
    }

    return 0;
}

