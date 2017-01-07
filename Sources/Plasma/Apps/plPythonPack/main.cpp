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

#include <vector>
#include <string>
#include <algorithm>
#include <string_theory/stdio>

#if HS_BUILD_FOR_WIN32
#    include "hsWindows.h"
#    include <direct.h>

#    define getcwd _getcwd
#    define chdir _chdir

#    ifndef MAXPATHLEN
#        define MAXPATHLEN MAX_PATH
#    endif
#elif HS_BUILD_FOR_UNIX
#    include <unistd.h>
#    include <sys/param.h>
#endif

static const char* kPackFileName = "python.pak";
static const char* kModuleFile = "__init__.py";
#if HS_BUILD_FOR_WIN32
    static const char* kGlueFile = ".\\plasma\\glue.py";
#else
    static const char* kGlueFile = "./plasma/glue.py";
#endif
static char* glueFile = (char*)kGlueFile;

void WritePythonFile(const plFileName &fileName, const plFileName &path, hsStream *s)
{
    hsUNIXStream pyStream, glueStream;
    plFileName filePath;
    ST_ssize_t filestart = fileName.AsString().find_last('.');
    if (filestart >= 0)
        filePath = fileName.AsString().substr(filestart+1);
    else
        filePath = fileName;
    filePath = plFileName::Join(path, filePath + ".py");

    if (!pyStream.Open(filePath) || !glueStream.Open(glueFile))
    {
        ST::printf("Unable to open path {}, ", filePath);
        return;
    }

    ST::printf("==Packing {}, ", fileName);

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
    PyImport_ImportModule(fileName.AsString().c_str());
    PyObject* pythonCode = PythonInterface::CompileString(code, fileName);
    if (pythonCode)
    {
        // we need to find out if this is PythonFile module
        // create a module name... with the '.' as an X
        // and create a python file name that is without the ".py"
        PyObject* fModule = PythonInterface::CreateModule(fileName.AsString().c_str());
        // run the code
        if (PythonInterface::RunPYC(pythonCode, fModule) )
        {
    // set the name of the file (in the global dictionary of the module)
            PyObject* dict = PyModule_GetDict(fModule);
            PyObject* pfilename = PyString_FromString(fileName.AsString().c_str());
            PyDict_SetItemString(dict, "glue_name", pfilename);
    // next we need to:
    //  - create instance of class
            PyObject* getID = PythonInterface::GetModuleItem("glue_getBlockID",fModule);
            bool foundID = false;
            if ( getID!=nil && PyCallable_Check(getID) )
            {
                PyObject* id = PyObject_CallFunction(getID,nil);
                if ( id && PyInt_Check(id) )
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
                fputs("an import file ", stdout);
            }
            else
                fputs("a PythonFile modifier(tm) ", stdout);
        }
        else
        {
            fputs("......blast! Error during run-code!\n", stdout);

            char* errmsg;
            int chars_read = PythonInterface::getOutputAndReset(&errmsg);
            if (chars_read > 0)
            {
                puts(errmsg);
            }
        }
    }

    // make sure that we have code to save
    if (pythonCode)
    {
        int32_t size;
        char* pycode;
        PythonInterface::DumpObject(pythonCode,&pycode,&size);

        fputc('\n', stdout);
        // print any message after each module
        char* errmsg;
        int chars_read = PythonInterface::getOutputAndReset(&errmsg);
        if (chars_read > 0)
        {
            puts(errmsg);
        }
        s->WriteLE32(size);
        s->Write(size, pycode);
    }
    else
    {
        fputs("......blast! Compile error!\n", stdout);
        s->WriteLE32(0);

        PyErr_Print();
        PyErr_Clear();

        char* errmsg;
        int chars_read = PythonInterface::getOutputAndReset(&errmsg);
        if (chars_read > 0)
        {
            puts(errmsg);
        }
    }

    delete [] code;

    pyStream.Close();
    glueStream.Close();
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

std::string ToLowerCase(std::string str)
{
    std::string retVal = "";
    for (int i=0; i<str.length(); i++)
    {
        if ((str[i]>='A')&&(str[i]<='Z'))
            retVal += (char)tolower(str[i]);
        else
            retVal += str[i];
    }
    return retVal;
}

void FindSubDirs(std::vector<plFileName> &dirnames, const plFileName &path)
{
    std::vector<plFileName> subdirs = plFileSystem::ListSubdirs(path);
    for (auto iter = subdirs.begin(); iter != subdirs.end(); ++iter) {
        ST::string name = iter->GetFileName();
        if (name.compare_i("system") != 0 && name.compare_i("plasma") != 0)
            dirnames.push_back(name);
    }
}

// adds or removes the ending slash in a path as necessary
std::string AdjustEndingSlash(std::string path, bool endingSlash = false)
{
#if HS_BUILD_FOR_WIN32
    char slash = '\\';
#else
    char slash = '/';
#endif

    std::string retVal = path;
    bool endSlashExists = false;
    char temp = path[path.length()-1];
    if (temp == slash)
        endSlashExists = true;

    if (endingSlash)
    {
        if (!endSlashExists)
            retVal += slash;
    }
    else
    {
        if (endSlashExists)
        {
            std::string temp = "";
            for (int i=0; i<retVal.length()-1; i++)
                temp += retVal[i];
            retVal = temp;
        }
    }
    return retVal;
}

// appends partialPath onto the end of fullPath, inserting or removing slashes as necesssary
std::string ConcatDirs(std::string fullPath, std::string partialPath)
{
#if HS_BUILD_FOR_WIN32
    char slash = '\\';
#else
    char slash = '/';
#endif

    bool fullSlash = false, partialSlash = false;
    char temp = fullPath[fullPath.length()-1];
    if (temp == slash)
        fullSlash = true;
    temp = partialPath[0];
    if (temp == slash)
        partialSlash = true;

    std::string retVal = "";
    if (!fullSlash)
        retVal = fullPath + slash;
    if (partialSlash)
    {
        std::string temp = "";
        for (int i=1; i<partialPath.length(); i++)
            temp += partialPath[i];
        partialPath = temp;
    }
    retVal += partialPath;
    return retVal;
}

void FindPackages(std::vector<plFileName>& fileNames, std::vector<plFileName>& pathNames, const plFileName& path, const ST::string& parent_package=ST::null)
{
    std::vector<plFileName> packages;
    FindSubDirs(packages, path);
    for (int i = 0; i < packages.size(); i++)
    {
        ST::string packageName;
        if (!parent_package.is_empty())
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
    ST::printf("\nCreating {} using the contents of {}\n", pakName, dir);
    ST::printf("Changing working directory to {}\n", rootPath);
    if (!plFileSystem::SetCWD(rootPath))
    {
        ST::printf("ERROR: Directory change to {} failed for some reason\n", rootPath);
        fputs("Unable to continue with the packing of this directory, aborting...\n", stdout);
        return;
    }
    else
        ST::printf("Directory changed to {}\n", rootPath);

    std::vector<plFileName> fileNames;
    std::vector<plFileName> pathNames;

    FindFiles(fileNames, pathNames, dir);
    FindPackages(fileNames, pathNames, dir);
    if (packSysAndPlasma)
    {
        fputs("Adding the system and plasma directories to this pack file\n", stdout);
        plFileName tempPath;
        tempPath = plFileName::Join(dir, "system");
        FindFiles(fileNames, pathNames, tempPath);
        FindPackages(fileNames, pathNames, tempPath);
        tempPath = plFileName::Join(dir, "plasma");
        FindFiles(fileNames, pathNames, tempPath);
        FindPackages(fileNames, pathNames, tempPath);
    }


    // ok, we know how many files we're gonna pack, so make a fake index (we'll fill in later)
    hsUNIXStream s;
    if (!s.Open(pakName, "wb"))
        return;

    s.WriteLE32(fileNames.size());

    int i;
    for (i = 0; i < fileNames.size(); i++)
    {
        s.WriteSafeString(fileNames[i].AsString());
        s.WriteLE32(0);
    }

    PythonInterface::initPython(rootPath);
    for (i = 0; i < extraDirs.size(); i++)
        PythonInterface::addPythonPath(plFileName::Join(rootPath, extraDirs[i]));

    // set to maximum optimization (includes removing __doc__ strings)
    Py_OptimizeFlag = 2;

    std::vector<uint32_t> filePositions;
    filePositions.resize(fileNames.size());

    for (i = 0; i < fileNames.size(); i++)
    {
        // strip '.py' from the file name
        plFileName properFileName = fileNames[i].StripFileExt();
        uint32_t initialPos = s.GetPosition();
        WritePythonFile(properFileName, pathNames[i], &s);
        uint32_t endPos = s.GetPosition();

        filePositions[i] = initialPos;
    }

    s.SetPosition(sizeof(uint32_t));
    for (i = 0; i < fileNames.size(); i++)
    {
        s.WriteSafeString(fileNames[i].AsString());
        s.WriteLE32(filePositions[i]);
    }

    s.Close();

    PythonInterface::finiPython();
}

void PrintUsage()
{
    fputs("Usage:\n", stdout);
    fputs("plPythonPack [directory to pack...]\n", stdout);
    fputs("NOTE: the directory to pack must have full system and plasma dirs and\n", stdout);
    fputs("      must be a relative path to the current working directory\n", stdout);
}

int main(int argc, char *argv[])
{
    fputs("The Python Pack Utility\n", stdout);

    plFileName baseWorkingDir = plFileSystem::GetCWD();

    // are they asking for usage?
    if (argc == 2)
    {
        std::string temp = argv[1];
        temp = ToLowerCase(temp);
        if ((temp == "?") || (temp == "-?") || (temp == "/?") || (temp == "-help") || (temp == "/help")
            || (temp == "-h") || (temp == "/h"))
        {
            PrintUsage();
            return -1;
        }
    }
    // wrong number of args, print usage
    if (argc > 2)
    {
        PrintUsage();
        return -1;
    }

    std::vector<plFileName> dirNames;
    plFileName rootPath;

    if (argc == 1)
    {
        FindSubDirs(dirNames, "");
        rootPath = baseWorkingDir;
    }
    else
    {
        plFileName path = argv[1];
        FindSubDirs(dirNames, argv[1]);
        rootPath = plFileName::Join(baseWorkingDir, path);
    }

    PackDirectory(rootPath, rootPath, plFileName::Join(rootPath, kPackFileName), dirNames, true);
    for (auto it = dirNames.begin(); it != dirNames.end(); ++it)
    {
        // Make sure this subdirectory is not just a python module. Those are packed into the
        // main python root package...
        plFileName dir = plFileName::Join(rootPath, *it);
        if (plFileSystem::ListDir(dir, kModuleFile).empty())
            PackDirectory(*it, rootPath, plFileName::Join(rootPath, *it + ".pak"), dirNames);
    }

    return 0;
}
