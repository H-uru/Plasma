// AssShader.cpp : Defines the entry point for the console application.
//

#include "HeadSpin.h"
#include "plFileSystem.h"
#include "hsStream.h"

#include "hsWindows.h"
#include <d3d9.h>
#include <D3Dcompiler.h>

#include <memory>

#include <string_theory/format>
#include <string_theory/stdio>

class plDXShaderError : std::exception
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
    fputs("\n\n\n", fp);

    hsSsize_t byteLen = shader->GetBufferSize();
    hsSsize_t quadLen = byteLen >> 2;

    unsigned char* codes = (unsigned char*)shader->GetBufferPointer();

    ST::printf(fp, "static const uint32_t {}byteLen = {};\n\n", varName, byteLen);
    ST::printf(fp, "static const uint8_t {}Codes[] = {{\n", varName);

    for (hsSsize_t i = 0; i < quadLen-1; i++)
    {
        ST::printf(fp, "\t0x{x},", *codes++);
        ST::printf(fp, "\t0x{x},", *codes++);
        ST::printf(fp, "\t0x{x},", *codes++);
        ST::printf(fp, "\t0x{x},\n", *codes++);
    }
    ST::printf(fp, "\t0x{x},", *codes++);
    ST::printf(fp, "\t0x{x},", *codes++);
    ST::printf(fp, "\t0x{x},", *codes++);
    ST::printf(fp, "\t0x{x}\n", *codes++);
    fputs("\t};", fp);
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
    if (!outFp.Open(outFile, "w")) {
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

