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
#include "plFile/hsFiles.h"

#include <vector>
#include <string>
#include <algorithm>

#if HS_BUILD_FOR_WIN32
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
#if HS_BUILD_FOR_WIN32
    static const char* kGlueFile = ".\\plasma\\glue.py";
#else
    static const char* kGlueFile = "./plasma/glue.py";
#endif
static char* glueFile = (char*)kGlueFile;

void WritePythonFile(std::string fileName, std::string path, hsStream *s)
{
    hsUNIXStream pyStream, glueStream;
    std::string filePath;
    size_t filestart = fileName.find_last_of('.');
    if(filestart != std::string::npos)
        filePath = fileName.substr(filestart+1, std::string::npos);
    else
        filePath = fileName;
    filePath += ".py";
    filePath = path + filePath;

    if (!pyStream.Open(filePath.c_str()) || !glueStream.Open(glueFile))
    {
        printf("Unable to open path %s, ",filePath.c_str());
        return;
    }

    printf("==Packing %s, ",fileName.c_str());

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
    PyImport_ImportModule(fileName.c_str());
    PyObject* pythonCode = PythonInterface::CompileString(code, fileName.c_str());
    if (pythonCode)
    {
        // we need to find out if this is PythonFile module
        // create a module name... with the '.' as an X
        // and create a python file name that is without the ".py"
        PyObject* fModule = PythonInterface::CreateModule(fileName.c_str());
        // run the code
        if (PythonInterface::RunPYC(pythonCode, fModule) )
        {
    // set the name of the file (in the global dictionary of the module)
            PyObject* dict = PyModule_GetDict(fModule);
            PyObject* pfilename = PyString_FromString(fileName.c_str());
            PyDict_SetItemString(dict, "glue_name", pfilename);
    // next we need to:
    //  - create instance of class
            PyObject* getID = PythonInterface::GetModuleItem("glue_getBlockID",fModule);
            hsBool foundID = false;
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
                pythonCode = PythonInterface::CompileString(code, fileName.c_str());
                hsAssert(pythonCode,"Not sure why this didn't compile the second time???");
                printf("an import file ");
            }
            else
                printf("a PythonFile modifier(tm) ");
        }
        else
        {
            printf("......blast! Error during run-code!\n");

            char* errmsg;
            int chars_read = PythonInterface::getOutputAndReset(&errmsg);
            if (chars_read > 0)
            {
                printf("%s\n", errmsg);
            }
        }
    }

    // make sure that we have code to save
    if (pythonCode)
    {
        int32_t size;
        char* pycode;
        PythonInterface::DumpObject(pythonCode,&pycode,&size);

        printf("\n");
        // print any message after each module
        char* errmsg;
        int chars_read = PythonInterface::getOutputAndReset(&errmsg);
        if (chars_read > 0)
        {
            printf("%s\n", errmsg);
        }
        s->WriteLE32(size);
        s->Write(size, pycode);
    }
    else
    {
        printf("......blast! Compile error!\n");
        s->WriteLE32(0);

        PyErr_Print();
        PyErr_Clear();

        char* errmsg;
        int chars_read = PythonInterface::getOutputAndReset(&errmsg);
        if (chars_read > 0)
        {
            printf("%s\n", errmsg);
        }
    }

    delete [] code;

    pyStream.Close();
    glueStream.Close();
}

void FindFiles(std::vector<std::string> &filenames, std::vector<std::string> &pathnames, const char* path)
{
    // Get the names of all the python files
    hsFolderIterator folder;

    // if there is a path... set it
    if ( path )
        folder.SetPath(path);

    while (folder.NextFileSuffix(".py"))
    {
        const char *fileName = folder.GetFileName();
        filenames.push_back(fileName);
        if ( path )
            pathnames.push_back(path);
        else
            pathnames.push_back("");
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

void FindSubDirs(std::vector<std::string> &dirnames, const char *path)
{
    hsFolderIterator folder;
    if (path)
        folder.SetPath(path);

    while (folder.NextFile())
    {
        if (folder.IsDirectory())
        {
            std::string dirName = folder.GetFileName();
            if ((dirName != ".")&&(dirName != "..")&&(ToLowerCase(dirName) != "system") && (ToLowerCase(dirName) != "plasma"))
            {
                dirnames.push_back(dirName);
            }
        }
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

void FindPackages(std::vector<std::string>& fileNames, std::vector<std::string>& pathNames, const char* path, std::string parent_package="")
{
    std::vector<std::string> packages;
    FindSubDirs(packages, path);
    for (int i = 0; i < packages.size(); i++)
    {
        std::string packageName;
        if(!parent_package.empty())
            packageName = parent_package + ".";
        packageName += packages[i];
        std::vector<std::string> packageFileNames;
        std::vector<std::string> packagePathNames;
        std::string packagePath = path;
        packagePath += "/" + packages[i];
        FindFiles(packageFileNames, packagePathNames, packagePath.c_str());
        for (int j = 0; j < packageFileNames.size(); j++) {
            fileNames.push_back(packageName+"."+packageFileNames[j]);
            pathNames.push_back(packagePathNames[j]+"/");
        }
        FindPackages(fileNames, pathNames, packagePath.c_str(), packageName);
    }
}

void PackDirectory(std::string dir, std::string rootPath, std::string pakName, std::vector<std::string>& extraDirs, bool packSysAndPlasma = false)
{
    // make sure the dir ends in a slash
    dir = AdjustEndingSlash(dir,true);

    printf("\nCreating %s using the contents of %s\n",pakName.c_str(),dir.c_str());
    printf("Changing working directory to %s\n",rootPath.c_str());
    if (chdir(rootPath.c_str()))
    {
        printf("ERROR: Directory change to %s failed for some reason\n",rootPath.c_str());
        printf("Unable to continue with the packing of this directory, aborting...\n");
        return;
    }
    else
        printf("Directory changed to %s\n",rootPath.c_str());

    std::vector<std::string> fileNames;
    std::vector<std::string> pathNames;

    FindFiles(fileNames,pathNames,dir.c_str());
    FindPackages(fileNames,pathNames,dir.c_str());
    if (packSysAndPlasma)
    {
        printf("Adding the system and plasma directories to this pack file\n");
        std::string tempPath;
        tempPath = dir + "system/";
        FindFiles(fileNames,pathNames,tempPath.c_str());
        FindPackages(fileNames,pathNames,tempPath.c_str());
        tempPath = dir + "plasma/";
        FindFiles(fileNames,pathNames,tempPath.c_str());
        FindPackages(fileNames,pathNames,tempPath.c_str());
    }


    // ok, we know how many files we're gonna pack, so make a fake index (we'll fill in later)
    hsUNIXStream s;
    if (!s.Open(pakName.c_str(), "wb"))
        return;

    s.WriteLE32(fileNames.size());

    int i;
    for (i = 0; i < fileNames.size(); i++)
    {
        s.WriteSafeString(fileNames[i].c_str());
        s.WriteLE32(0);
    }

    PythonInterface::initPython(rootPath);
    for (i = 0; i < extraDirs.size(); i++)
        PythonInterface::addPythonPath(rootPath + extraDirs[i]);

    // set to maximum optimization (includes removing __doc__ strings)
    Py_OptimizeFlag = 2;

    std::vector<uint32_t> filePositions;
    filePositions.resize(fileNames.size());

    for (i = 0; i < fileNames.size(); i++)
    {
        // strip '.py' from the file name
        std::string properFileName = fileNames[i].substr(0, fileNames[i].size()-3);
        uint32_t initialPos = s.GetPosition();
        WritePythonFile(properFileName, pathNames[i], &s);
        uint32_t endPos = s.GetPosition();

        filePositions[i] = initialPos;
    }

    s.SetPosition(sizeof(uint32_t));
    for (i = 0; i < fileNames.size(); i++)
    {
        s.WriteSafeString(fileNames[i].c_str());
        s.WriteLE32(filePositions[i]);
    }

    s.Close();

    PythonInterface::finiPython();
}

void PrintUsage()
{
    printf("Usage:\n");
    printf("plPythonPack [directory to pack...]\n");
    printf("NOTE: the directory to pack must have full system and plasma dirs and\n");
    printf("      must be a relative path to the current working directory\n");
}

int main(int argc, char *argv[])
{
    printf("The Python Pack Utility\n");

    char buffer[MAXPATHLEN];
    getcwd(buffer, MAXPATHLEN);
    std::string baseWorkingDir = buffer;

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

    std::vector<std::string> dirNames;
    std::string rootPath;

    if (argc == 1)
    {
        FindSubDirs(dirNames,nil);
        rootPath = AdjustEndingSlash(baseWorkingDir,true);
    }
    else
    {
        std::string path = argv[1];
        FindSubDirs(dirNames,argv[1]);
        rootPath = ConcatDirs(baseWorkingDir,path);
        rootPath = AdjustEndingSlash(rootPath,true);
    }
    
    PackDirectory(rootPath,rootPath,rootPath+kPackFileName,dirNames,true);
    for (int i=0; i<dirNames.size(); i++)
    {
        PackDirectory(dirNames[i],rootPath,rootPath+dirNames[i]+".pak",dirNames);
    }

    return 0;
}
