// AssShader.cpp : Defines the entry point for the console application.
//

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <d3d9.h>
#include <d3dx9core.h>
#include <string_theory/format>
#include <string_theory/stdio>

void ICreateHeader(const ST::string& varName, const plFileName& fileName, FILE* fp, LPD3DXBUFFER shader)
{
    fputs("\n\n\n", fp);

    int byteLen = shader->GetBufferSize();
    int quadLen = byteLen >> 2;

    unsigned char* codes = (unsigned char*)shader->GetBufferPointer();

    ST::printf(fp, "static const uint32_t {}byteLen = {};\n\n", varName, byteLen);
    ST::printf(fp, "static const uint8_t {}Codes[] = {\n", varName);

    int i;
    for( i = 0; i < quadLen-1; i++ )
    {
        ST::printf(fp, "\t{#x},", *codes++);
        ST::printf(fp, "\t{#x},", *codes++);
        ST::printf(fp, "\t{#x},", *codes++);
        ST::printf(fp, "\t{#x},\n", *codes++);
    }
    ST::printf(fp, "\t{#x},", *codes++);
    ST::printf(fp, "\t{#x},", *codes++);
    ST::printf(fp, "\t{#x},", *codes++);
    ST::printf(fp, "\t{#x}\n", *codes++);
    fputs("\t};", fp);
    fputs("\n\n", fp);

    ST::printf(fp, "static const plShaderDecl {}Decl(\"{}\", {}, {}byteLen, {}Codes);\n\n",
               varName, fileName, varName, varName, varName);
    ST::printf(fp, "static const plShaderRegister {}Register(&{}Decl);\n\n", varName, varName);
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
        numNames = arrsize(kEnumNames);
    }
    else
    {
        nameList = argv+1;
        numNames = argc-1;
    }

    for (int i = 0; i < numNames; i++ )
    {
        const char* name = nameList[i];
        ST::string varName = plFileName(name).StripFileExt().AsString();

        plFileName inFile = ST::format("{}.inl", varName);
        plFileName outFile = ST::format("{}.h", varName);

        ST::printf("Processing {} into {}\n", name, outFile);
        FILE* fp = fopen(outFile.AsString().c_str(), "w");
        if (!fp)
        {
            fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stdout);
            ST::printf("Error opening file {} for output\n", outFile);
            fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stdout);
            continue;
        }

        LPD3DXBUFFER compiledShader = nullptr;
        LPD3DXBUFFER compilationErrors = nullptr;
        DWORD   flags = 0;
        LPD3DXINCLUDE include = nullptr;

        HRESULT hr = D3DXAssembleShaderFromFile(
                        inFile.AsString().c_str(),
                        nullptr,
                        include,
                        flags,
                        &compiledShader,
                        &compilationErrors);

        if (FAILED(hr))
        {
            fputs("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n", stdout);
            fputs(compilationErrors ? (const char*)compilationErrors->GetBufferPointer()
                                    : "File not found", stdout);
            continue;
        }

        ICreateHeader(varName, ST::format("sha/{}.inl", varName), fp, compiledShader);

        fclose(fp);

    }

    return 0;
}

