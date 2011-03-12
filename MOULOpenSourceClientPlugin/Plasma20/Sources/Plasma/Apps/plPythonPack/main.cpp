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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsStream.h"
#include "../plFile/hsFiles.h"

#include "PythonInterface.h"

#include <vector>
#include <string>

#include <direct.h>

static const char* kPackFileName = "python.pak";
static const char* kGlueFile = ".\\plasma\\glue.py";
static char* glueFile = (char*)kGlueFile;

void WritePythonFile(const char *fileName, const char* path, hsStream *s)
{
	hsUNIXStream pyStream, glueStream;
	char* pathAndFile = new char[strlen(fileName)+strlen(path)+2];
	strcpy(pathAndFile,path);
	char lastchar = pathAndFile[strlen(pathAndFile)-1];
	if (lastchar != '\\' && lastchar != '/')
		strcat(pathAndFile, "\\");
	strcat(pathAndFile,fileName);
	if (!pyStream.Open(pathAndFile) || !glueStream.Open(glueFile))
	{
		printf("Unable to open path %s, ",pathAndFile);
		return;
	}

	printf("==Packing %s, ",fileName);

	pyStream.FastFwd();
	UInt32 pyFileSize = pyStream.GetPosition();
	pyStream.Rewind();

	glueStream.FastFwd();
	UInt32 glueFileSize = glueStream.GetPosition();
	glueStream.Rewind();

	UInt32 totalSize = pyFileSize + glueFileSize + 2;

	char *code = new char[totalSize];

	UInt32 amountRead = pyStream.Read(pyFileSize, code);
	hsAssert(amountRead == pyFileSize, "Bad read");

	code[pyFileSize] = '\n';

	amountRead = glueStream.Read(glueFileSize, code+pyFileSize+1);
	hsAssert(amountRead == glueFileSize, "Bad read");

	code[totalSize-1] = '\0';

	// remove the CRs, they seem to give Python heartburn
	int k = 0;
	for (int i = 0; i < totalSize; i++)
	{
		if (code[i] != '\r')	// is it not a CR?
			code[k++] = code[i];
		// else
		//   skip the CRs
	}

	PyObject* pythonCode = PythonInterface::CompileString(code, (char*)fileName);
	if (pythonCode)
	{
		// we need to find out if this is PythonFile module
		// create a module name... with the '.' as an X
		// and create a python file name that is without the ".py"
		char* modulename = new char[strlen(fileName)+1];
		char* pythonfilename = new char[strlen(fileName)+1];
		int j;
		for (j=0; j<strlen(fileName); j++)
		{
			if (fileName[j] == '.')
			{
				modulename[j] = 'X';
				pythonfilename[j] = '\0';
			}
			else
			{
				modulename[j] = fileName[j];
				pythonfilename[j] = fileName[j];
			}
		}
		modulename[j] = '\0';
		PyObject* fModule = PythonInterface::CreateModule(modulename);
		delete [] modulename;
		// run the code
		if (PythonInterface::RunPYC(pythonCode, fModule) )
		{
	// set the name of the file (in the global dictionary of the module)
			PyObject* dict = PyModule_GetDict(fModule);
			PyObject* pfilename = PyString_FromString(pythonfilename);
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
			if ( foundID == false )		// then there was an error or no ID or somethin'
			{
				// oops, this is not a PythonFile modifier
				// re-read the source and compile it without the glue code this time
				memset(code,0,totalSize);
				pyStream.Rewind();
				amountRead = pyStream.Read(pyFileSize, code);
				hsAssert(amountRead == pyFileSize, "Bad read");
				strcat(code,"\n");
				k = 0;
				for (int i = 0; i < strlen(code)+1; i++)
				{
					if (code[i] != '\r')	// is it not a CR?
						code[k++] = code[i];
					// else
					//   skip the CRs
				}
				pythonCode = PythonInterface::CompileString(code, (char*)fileName);
				hsAssert(pythonCode,"Not sure why this didn't compile the second time???");
				printf("an import file ");
			}
			else
				printf("a PythonFile modifier(tm) ");
		}
		else
		{
			printf("......blast! Error during run-code!\n");
			s->WriteSwap32(0);

			char* errmsg;
			int chars_read = PythonInterface::getOutputAndReset(&errmsg);
			if (chars_read > 0)
			{
				printf(errmsg);
				printf("\n");
			}
		}
		delete [] pythonfilename;
	}

	// make sure that we have code to save
	if (pythonCode)
	{
		Int32 size;
		char* pycode;
		PythonInterface::DumpObject(pythonCode,&pycode,&size);

		printf("\n");
		// print any message after each module
		char* errmsg;
		int chars_read = PythonInterface::getOutputAndReset(&errmsg);
		if (chars_read > 0)
		{
			printf(errmsg);
			printf("\n");
		}
		s->WriteSwap32(size);
		s->Write(size, pycode);
	}
	else
	{
		printf("......blast! Compile error!\n");
		s->WriteSwap32(0);

		PyErr_Print();
		PyErr_Clear();

		char* errmsg;
		int chars_read = PythonInterface::getOutputAndReset(&errmsg);
		if (chars_read > 0)
		{
			printf(errmsg);
			printf("\n");
		}
	}

	delete [] code;
	delete [] pathAndFile;	// all done with the path and filename as one

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

void FindSubDirs(std::vector<std::string> &dirnames, std::vector<std::string> &pakNames, char *path)
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
				pakNames.push_back(dirName+".pak");
			}
		}
	}
}

// adds or removes the ending slash in a path as necessary
std::string AdjustEndingSlash(std::string path, bool endingSlash = false)
{
	std::string retVal = path;
	bool endSlashExists = false;
	char temp = path[path.length()-1];
	if (temp == '\\')
		endSlashExists = true;

	if (endingSlash)
	{
		if (!endSlashExists)
			retVal += "\\";
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
	bool fullSlash = false, partialSlash = false;
	char temp = fullPath[fullPath.length()-1];
	if (temp == '\\')
		fullSlash = true;
	temp = partialPath[0];
	if (temp == '\\')
		partialSlash = true;

	std::string retVal = "";
	if (!fullSlash)
		retVal = fullPath + "\\";
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

void PackDirectory(std::string dir, std::string rootPath, std::string pakName, std::vector<std::string>& extraDirs, bool packSysAndPlasma = false)
{
	// make sure the dir ends in a slash
	dir = AdjustEndingSlash(dir,true);

	printf("\nCreating %s using the contents of %s\n",pakName.c_str(),dir.c_str());
	printf("Changing working directory to %s\n",rootPath.c_str());
	if (_chdir(rootPath.c_str()))
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
	if (packSysAndPlasma)
	{
		printf("Adding the system and plasma directories to this pack file\n");
		std::string tempPath;
		tempPath = dir + "system";
		FindFiles(fileNames,pathNames,tempPath.c_str());
		tempPath = dir + "plasma";
		FindFiles(fileNames,pathNames,tempPath.c_str());
	}

	// ok, we know how many files we're gonna pack, so make a fake index (we'll fill in later)
	hsUNIXStream s;
	if (!s.Open(pakName.c_str(), "wb"))
		return;

	s.WriteSwap32(fileNames.size());

	int i;
	for (i = 0; i < fileNames.size(); i++)
	{
		s.WriteSafeString(fileNames[i].c_str());
		s.WriteSwap32(0);
	}

	PythonInterface::initPython(rootPath);
	for (i = 0; i < extraDirs.size(); i++)
		PythonInterface::addPythonPath(rootPath + extraDirs[i]);

	// set to maximum optimization (includes removing __doc__ strings)
	Py_OptimizeFlag = 2;

	std::vector<UInt32> filePositions;
	filePositions.resize(fileNames.size());

	for (i = 0; i < fileNames.size(); i++)
	{
		UInt32 initialPos = s.GetPosition();
		WritePythonFile(fileNames[i].c_str(), pathNames[i].c_str(), &s);
		UInt32 endPos = s.GetPosition();

		filePositions[i] = initialPos;
	}

	s.SetPosition(sizeof(UInt32));
	for (i = 0; i < fileNames.size(); i++)
	{
		s.WriteSafeString(fileNames[i].c_str());
		s.WriteSwap32(filePositions[i]);
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

void main(int argc, char *argv[])
{
	printf("The Python Pack Utility\n");

	char buffer[_MAX_PATH];
	_getcwd(buffer,_MAX_PATH);
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
			return;
		}
	}
	// wrong number of args, print usage
	if (argc > 2)
	{
		PrintUsage();
		return;
	}

	std::vector<std::string> dirNames;
	std::vector<std::string> pakNames;
	std::string rootPath;

	if (argc == 1)
	{
		FindSubDirs(dirNames,pakNames,nil);
		rootPath = AdjustEndingSlash(baseWorkingDir,true);
	}
	else
	{
		std::string path = argv[1];
		FindSubDirs(dirNames,pakNames,argv[1]);
		rootPath = ConcatDirs(baseWorkingDir,path);
		rootPath = AdjustEndingSlash(rootPath,true);
	}
	
	PackDirectory(rootPath,rootPath,rootPath+kPackFileName,dirNames,true);
	for (int i=0; i<dirNames.size(); i++)
	{
		PackDirectory(dirNames[i],rootPath,rootPath+pakNames[i],dirNames);
	}
}
