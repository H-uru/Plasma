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
    fFuncPtr = reinterpret_cast<D3DAssemble>(GetProcAddress(fLibrary, "D3DAssemble"));
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
        return {compiled, IRelease};

    ST::string strErrors(errors ? static_cast<const char*>(errors->GetBufferPointer()) : "Unspecified");
    IRelease(compiled);
    IRelease(errors);

    throw plDXShaderError("Error compiling shader: {}\n{}\n", hr, strErrors);
}

// ===================================================

void ICreateHeader(const ST::string& varName, const plFileName& fileName, FILE* fp, ID3DBlob* shader)
{
    ST::printf(fp, "\n\n\n");

    hsSsize_t byteLen = shader->GetBufferSize();
    hsSsize_t quadLen = byteLen >> 2;

    auto* codes = static_cast<unsigned char*>(shader->GetBufferPointer());

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
    ST::printf(fp, "\t];");
    ST::printf(fp, "\n\n");

    ST::printf(fp, "static const plShaderDecl {}Decl(\"{}\", {}, {}byteLen, {}Codes);\n\n",
               varName, fileName, varName, varName, varName);
    ST::printf(fp, "static const plShaderRegister {}Register(&{}Decl);\n\n", varName, varName);
}

static bool AttemptToOpenOutFile(plFileName outFile, hsUNIXStream& outFp)
{
	if (!outFp.Open(outFile, "w")) {
		fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
		ST::printf(stderr, "Error opening file {} for output\n", outFile);
		fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
		return true;
	}
	return false;
}

static bool AttemptToOpenInputFile(plFileName inFile, hsUNIXStream& inFp)
{
	if (!inFp.Open(inFile, "r")) {
		fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
		ST::printf(stderr, "Error opening file {} for input\n", inFile);
		fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
		return true;
	}
	return false;
}

static void IAssShader(const plDXShaderAssembler& ass, const char* name)
{
    ST::string varName = plFileName(name).StripFileExt().AsString();

    plFileName inFile = ST::format("{}.inl", varName);
    plFileName outFile = ST::format("{}.h", varName);

    ST::printf("Processing {} into {}\n", name, outFile);

    hsUNIXStream outFp;
    if (AttemptToOpenOutFile(outFile, outFp))
        return;

    hsUNIXStream inFp;
    if (AttemptToOpenInputFile(inFile, inFp))
        return;

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
// Just copy in the enum and use Replace on
// vs_ => "vs_
// ps_ => "ps_
// , => ",
static constexpr std::string_view kEnumNames[] {
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
int main(int argc, char* argv[])
{
    std::vector<std::string_view> allArgs(argv, argv + argc);
    //remove command
    allArgs.erase(allArgs.begin());

    if( allArgs.empty() )
    {
        ST::printf("{} <file0> <file1> ...\n", argv[0]);
        return 0;
    }


    const std::string_view* nameList = nullptr;
    int numNames = 0;
    if (allArgs.at(0).compare("all") == 0)
    {
        nameList = kEnumNames;
        numNames = std::size(kEnumNames);
    }
    else
    {

        nameList = allArgs.data();
        numNames = allArgs.size();
    }

    try {
	    for (int i = 0; i < numNames; i++) {
		    plDXShaderAssembler ass;
		    IAssShader(ass, nameList[i].data());
        }
    } catch (const plDXShaderError& error) {
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        fputs(error.c_str(), stderr);
        fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stderr);
        return 1;
    }

    return 0;
}

