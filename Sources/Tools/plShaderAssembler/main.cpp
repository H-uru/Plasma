// AssShader.cpp : Defines the entry point for the console application.
//

#include <D3d9.h>
#include <D3dx9core.h>

#include "plFileSystem.h"

void ICreateHeader(const char* const varName, const char* const fileName, FILE* fp, LPD3DXBUFFER shader)
{
    fprintf(fp, "\n\n\n");

    int byteLen = shader->GetBufferSize();
    int quadLen = byteLen >> 2;

    unsigned char* codes = (unsigned char*)shader->GetBufferPointer();

    fprintf(fp, "static const uint32_t %sbyteLen = %d;\n\n", varName, byteLen);
    fprintf(fp, "static const uint8_t %sCodes[] = {\n", varName);

    int i;
    for( i = 0; i < quadLen-1; i++ )
    {
        fprintf(fp, "\t0x%x,", *codes++);
        fprintf(fp, "\t0x%x,", *codes++);
        fprintf(fp, "\t0x%x,", *codes++);
        fprintf(fp, "\t0x%x,\n", *codes++);
    }
    fprintf(fp, "\t0x%x,", *codes++);
    fprintf(fp, "\t0x%x,", *codes++);
    fprintf(fp, "\t0x%x,", *codes++);
    fprintf(fp, "\t0x%x\n", *codes++);
    fprintf(fp, "\t};");
    fprintf(fp, "\n\n");

    fprintf(fp, "static const plShaderDecl %sDecl(\"%s\", %s, %sbyteLen, %sCodes);\n\n", varName, fileName, varName, varName, varName);
    fprintf(fp, "static const plShaderRegister %sRegister(&%sDecl);\n\n", varName, varName);
}

int main(int argc, char* argv[])
{
    if( argc < 2 )
    {
        printf("%s <file0> <file1> ...\n", argv[0]);
        return 0;
    }
    const char* const * nameList = 0L;
    int numNames = 0;
    if( !_stricmp(argv[1], "all") )
    {
        // Cut and paste enum and add kNumEnumNames to end
        // so you don't have to count.
        enum {
            vs_WaveFixedFin6,
            ps_WaveFixed,
            vs_CompCosines,
            ps_CompCosines,
            vs_ShoreLeave6,
            ps_ShoreLeave6,
            vs_WaveRip,
            ps_WaveRip,
            vs_WaveDec1Lay,
            vs_WaveDec2Lay11,
            vs_WaveDec2Lay12,
            vs_WaveDecEnv,
            ps_CbaseAbase,
            ps_CalphaAbase,
            ps_CalphaAMult,
            ps_CalphaAadd,
            ps_CaddAbase,
            ps_CaddAMult,
            ps_CaddAAdd,
            ps_CmultAbase,
            ps_CmultAMult,
            ps_CmultAAdd,
            ps_WaveDecEnv,
            vs_WaveGraph2,
            ps_WaveGraph,
            vs_WaveGridFin,
            ps_WaveGrid,
            vs_BiasNormals,
            ps_BiasNormals,
            vs_ShoreLeave7,
            vs_WaveRip7,
            ps_MoreCosines,
            vs_WaveDec1Lay_7,
            vs_WaveDec2Lay11_7,
            vs_WaveDec2Lay12_7,
            vs_WaveDecEnv_7,
            vs_WaveFixedFin7,
            vs_GrassShader,
            ps_GrassShader,

            kNumEnumNames
        };

        // Just copy in the enum and use Replace on
        // vs_ => "vs_
        // ps_ => "ps_
        // , => ",
        const char* kEnumNames[kNumEnumNames] = {
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
        numNames = kNumEnumNames;
    }
    else
    {
        nameList = argv+1;
        numNames = argc-1;
    }
    int i;
    for( i = 0; i < numNames; i++ )
    {
        const char* name = nameList[i];

        char varName[512];
        strcpy(varName, name);
        char* p = strrchr(varName, '.');
        if( p )
            *p = 0;

        char inFile[512];
        sprintf(inFile, "%s.inl", varName);

        char outFile[512];
        sprintf(outFile, "%s.h", varName);

        printf("Processing %s into %s\n", name, outFile);
        FILE* fp = fopen(outFile, "w");
        if( !fp )
        {
            printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
            printf("Error opening file %s for output\n");
            printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
            continue;
        }

        LPD3DXBUFFER compiledShader = 0L;
        LPD3DXBUFFER compilationErrors = 0L;
        DWORD   flags = 0;
        LPD3DXINCLUDE include = 0L;

        HRESULT hr = D3DXAssembleShaderFromFile(
                        inFile,
                        0L,
                        include,
                        flags,
                        &compiledShader,
                        &compilationErrors);

        if( FAILED(hr) )
        {
            printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
            printf(compilationErrors ? (char*)compilationErrors->GetBufferPointer() : "File not found");
            continue;
        }
        sprintf(inFile, "sha/%s.inl", varName);

        ICreateHeader(varName, inFile, fp, compiledShader);

        fclose(fp);

    }

    return 0;
}

