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

#include "PythonInterface.h"

#include "hsStream.h"
#include "plCmdParser.h"
#include "hsMain.inl"

#include <vector>
#include <string>
#include <algorithm>
#include <string_theory/stdio>
#include <unordered_set>

static const char* kPackFileName = "python.pak";
static const char* kModuleFile = "__init__.py";

/** Directories that should not generate their own .pak file. */
static const std::unordered_set<ST::string, ST::hash_i, ST::equal_i> s_ignoreSubdirs{
    ST_LITERAL("plasma"),
    ST_LITERAL("system"),
    ST_LITERAL("__pycache__"),
};

#if HS_BUILD_FOR_WIN32
    #define NULL_DEVICE "NUL:"
#else
    #define NULL_DEVICE "/dev/null"
#endif

FILE* out = stdout;

void WritePythonFile(const plFileName &fileName, const plFileName &path, hsStream *s)
{
    hsUNIXStream pyStream, glueStream;
    plFileName filePath;
    ST_ssize_t filestart = fileName.AsString().find_last('.');
    plFileName glueFile = plFileName::Join("plasma", "glue.py");

    if (filestart >= 0)
        filePath = fileName.AsString().substr(filestart+1);
    else
        filePath = fileName;
    filePath = plFileName::Join(path, filePath + ".py");

    if (!pyStream.Open(filePath) || !glueStream.Open(glueFile))
    {
        ST::printf(stderr, "Unable to open path {}, ", filePath);
        return;
    }

    ST::printf(out, "== Packing {}, ", fileName);

    pyStream.FastFwd();
    uint32_t pyFileSize = pyStream.GetPosition();
    pyStream.Rewind();

    glueStream.FastFwd();
    uint32_t glueFileSize = glueStream.GetPosition();
    glueStream.Rewind();

    uint32_t totalSize = pyFileSize + glueFileSize + 2;

    char *code = new char[totalSize];

    uint32_t amountRead = pyStream.Read(pyFileSize, code);
    hsAssert(amountRead == pyFileSize, "Bad read");

    code[pyFileSize] = '\n';

    amountRead = glueStream.Read(glueFileSize, code+pyFileSize+1);
    hsAssert(amountRead == glueFileSize, "Bad read");

    code[totalSize-1] = '\0';

    // remove the CRs, they seem to give Python heartburn
    int k = 0;
    for (int i = 0; i < totalSize; i++)
    {
        if (code[i] != '\r')    // is it not a CR?
            code[k++] = code[i];
        // else
        //   skip the CRs
    }

    // import the module first, to make packages work correctly
    PyObject* fModule = PyImport_ImportModule(fileName.AsString().c_str());
    if (!fModule)
        ST::printf(stderr, "......import failed ");

    PyObject* pythonCode = PythonInterface::CompileString(code, fileName);
    if (pythonCode)
    {
        // run the code
        if (PythonInterface::RunPYC(pythonCode, fModule) )
        {
            // set the name of the file (in the global dictionary of the module)
            PyObject* dict = PyModule_GetDict(fModule);
            PyObject* pfilename = PyUnicode_FromString(fileName.AsString().c_str());
            PyDict_SetItemString(dict, "glue_name", pfilename);
            Py_DECREF(pfilename);

            // next we need to:
            //  - create instance of class
            PyObject* getID = PythonInterface::GetModuleItem("glue_getBlockID",fModule);
            bool foundID = false;
            if (getID != nullptr && PyCallable_Check(getID))
            {
                PyObject* id = _PyObject_Vectorcall(getID, nullptr, 0, nullptr);
                if ( id && PyLong_Check(id) )
                    foundID = true;
            }
            if ( foundID == false )     // then there was an error or no ID or somethin'
            {
                // oops, this is not a PythonFile modifier
                // re-read the source and compile it without the glue code this time
                pyStream.Rewind();
                amountRead = pyStream.Read(pyFileSize, code);
                hsAssert(amountRead == pyFileSize, "Bad read");
                code[amountRead] = '\n';
                code[amountRead+1] = '\0';
                k = 0;
                int len = strlen(code)+1;
                for (int i = 0; i < len; i++)
                {
                    if (code[i] != '\r')    // is it not a CR?
                        code[k++] = code[i];
                    // else
                    //   skip the CRs
                }
                pythonCode = PythonInterface::CompileString(code, fileName);
                hsAssert(pythonCode,"Not sure why this didn't compile the second time???");
                ST::printf(out, "an import file ");
            }
            else
                ST::printf(out, "a PythonFile modifier(tm) ");
        }
        else
        {
            ST::printf(stderr, "......blast! Error during run-code in {}!\n", fileName);
            PyErr_Print();
        }
    }

    // make sure that we have code to save
    if (pythonCode)
    {
        Py_ssize_t size;
        char* pycode;
        PythonInterface::DumpObject(pythonCode,&pycode,&size);

        ST::printf(out, "\n");

        s->WriteLE32((int32_t)size);
        s->Write((uint32_t)size, pycode);
        delete[] pycode;
    }
    else
    {
        ST::printf(stderr, "......blast! Compile error in {}!\n", fileName);
        s->WriteLE32(0);

        PyErr_Print();
        PyErr_Clear();
    }

    delete [] code;
    Py_XDECREF(pythonCode);
    Py_XDECREF(fModule);
}

void FindFiles(std::vector<plFileName> &filenames, std::vector<plFileName> &pathnames, const plFileName& path)
{
    // Get the names of all the python files
    std::vector<plFileName> pys = plFileSystem::ListDir(path, "*.py");

    for (auto iter = pys.begin(); iter != pys.end(); ++iter)
    {
        filenames.push_back(iter->GetFileName());
        pathnames.push_back(path);
    }
}

void FindSubDirs(std::vector<plFileName> &dirnames, const plFileName &path)
{
    std::vector<plFileName> subdirs = plFileSystem::ListSubdirs(path);
    for (auto iter = subdirs.begin(); iter != subdirs.end(); ++iter) {
        ST::string name = iter->GetFileName();
        if (s_ignoreSubdirs.find(name) == s_ignoreSubdirs.end())
            dirnames.push_back(name);
    }
}

void FindPackages(std::vector<plFileName>& fileNames, std::vector<plFileName>& pathNames, const plFileName& path, const ST::string& parent_package={})
{
    std::vector<plFileName> packages;
    FindSubDirs(packages, path);
    for (int i = 0; i < packages.size(); i++)
    {
        ST::string packageName;
        if (!parent_package.empty())
            packageName = parent_package + ".";
        packageName += packages[i].AsString();
        std::vector<plFileName> packageFileNames;
        std::vector<plFileName> packagePathNames;
        plFileName packagePath = plFileName::Join(path, packages[i]);
        FindFiles(packageFileNames, packagePathNames, packagePath);

        // Check for the magic file to make sure this is really a package...
        if (std::find(packageFileNames.begin(), packageFileNames.end(), kModuleFile) != packageFileNames.end()) {
            for (int j = 0; j < packageFileNames.size(); j++) {
                fileNames.push_back(packageName+"."+packageFileNames[j].AsString());
                pathNames.push_back(packagePathNames[j]);
            }
            FindPackages(fileNames, pathNames, packagePath, packageName);
        }
    }
}

void PackDirectory(const plFileName& dir, const plFileName& rootPath, const plFileName& pakName, std::vector<plFileName>& extraDirs, bool packSysAndPlasma = false)
{
    ST::printf(out, "\nCreating {} using the contents of {}\n", pakName, dir);
    ST::printf(out, "Changing working directory to {}\n", rootPath);
    if (!plFileSystem::SetCWD(rootPath))
    {
        ST::printf(stderr, "ERROR: Directory change to {} failed for some reason\n", rootPath);
        ST::printf(stderr, "Unable to continue with the packing of this directory, aborting...\n");
        return;
    }
    else
        ST::printf(out, "Directory changed to {}\n", rootPath);

    std::vector<plFileName> fileNames;
    std::vector<plFileName> pathNames;

    FindFiles(fileNames, pathNames, dir);
    FindPackages(fileNames, pathNames, dir);
    if (packSysAndPlasma)
    {
        ST::printf(out, "\nAdding the system and plasma directories to this pack file...\n");
        plFileName tempPath;
        tempPath = plFileName::Join(dir, "system");
        FindFiles(fileNames, pathNames, tempPath);
        FindPackages(fileNames, pathNames, tempPath);
        tempPath = plFileName::Join(dir, "plasma");
        FindFiles(fileNames, pathNames, tempPath);
        FindPackages(fileNames, pathNames, tempPath);
    }


    // ok, we know how many files we're gonna pack, so make a fake index (we'll fill in later)
    {
        hsUNIXStream s;
        if (!s.Open(pakName, "wb"))
            return;

        s.WriteLE32((uint32_t)fileNames.size());
        for (const plFileName& fn : fileNames)
        {
            s.WriteSafeString(fn.AsString());
            s.WriteLE32(0);
        }

        PythonInterface::initPython(rootPath, extraDirs, out, stderr);
    
        std::vector<uint32_t> filePositions(fileNames.size());

        for (size_t i = 0; i < fileNames.size(); i++)
        {
            // strip '.py' from the file name
            plFileName properFileName = fileNames[i].StripFileExt();
            uint32_t initialPos = s.GetPosition();
            WritePythonFile(properFileName, pathNames[i], &s);
    
            filePositions[i] = initialPos;
        }

        s.SetPosition(sizeof(uint32_t));
        for (size_t i = 0; i < fileNames.size(); i++)
        {
            s.WriteSafeString(fileNames[i].AsString());
            s.WriteLE32(filePositions[i]);
        }
    }

    ST::printf(out, "\nPython Package written to {}\n", pakName);

    PythonInterface::finiPython();
}

void PrintUsage()
{
    ST::printf("The Python Pack Utility\n");
    ST::printf("Usage:\n");
    ST::printf("plPythonPack [options] <directory to pack...>\n");
    ST::printf("NOTE: the directory to pack must have full system and plasma dirs.\n");
    ST::printf("\nAvailable options:\n");
    ST::printf("\t-q\tQuiet  - Only print errors\n");
    ST::printf("\t-h\tHelp   - Print this help\n");
}

static int hsMain(std::vector<ST::string> args)
{
    // Parse arguments
    ST::string packDir = ".";

    enum { kArgPath, kArgQuiet, kArgHelp1, kArgHelp2 };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgOptional | kCmdTypeString, "path",  kArgPath},
        { kCmdArgFlagged  | kCmdTypeBool,   "quiet", kArgQuiet},
        { kCmdArgFlagged  | kCmdTypeBool,   "help",  kArgHelp1},
        { kCmdArgFlagged  | kCmdTypeBool,   "?",     kArgHelp2},
    };

    plCmdParser cmdParser(cmdLineArgs, std::size(cmdLineArgs));
    if (cmdParser.Parse(args)) {
        if (cmdParser.GetBool(kArgHelp1) || cmdParser.GetBool(kArgHelp2)) {
            PrintUsage();
            return 0;
        }

        if (cmdParser.GetBool(kArgQuiet))
            out = fopen(NULL_DEVICE, "w");

        if (cmdParser.IsSpecified(kArgPath))
            packDir = cmdParser.GetString(kArgPath);
    } else {
        ST::printf(stderr, "An error occurred while parsing the provided arguments.\n");
        ST::printf(stderr, "Use the --help option to display usage information.\n");
        return 1;
    }

    ST::printf(out, "The Python Pack Utility\n");

    std::vector<plFileName> dirNames;

    FindSubDirs(dirNames, packDir);
    plFileName rootPath(packDir);
    rootPath = rootPath.AbsolutePath();

    PackDirectory(rootPath, rootPath, plFileName::Join(rootPath, kPackFileName), dirNames, true);
    for (auto it = dirNames.begin(); it != dirNames.end(); ++it) {
        // Make sure this subdirectory is not just a python module. Those are packed into the
        // main python root package...
        plFileName dir = plFileName::Join(rootPath, *it);
        if (plFileSystem::ListDir(dir, kModuleFile).empty())
            PackDirectory(*it, rootPath, plFileName::Join(rootPath, *it + ".pak"), dirNames);
    }

    if (out && out != stdout)
        fclose(out);

    return 0;
}
