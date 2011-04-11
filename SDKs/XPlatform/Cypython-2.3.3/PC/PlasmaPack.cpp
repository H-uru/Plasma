// gateway to cpp code


#include "Python.h"
#include "plPythonPack.h"

#include "PlasmaPack.h"

PyObject* Pl_OpenPacked(const char* fileName)
{
	return PythonPack::OpenPythonPacked(fileName);
}

int Pl_IsItPacked(const char* fileName)
{
	return PythonPack::IsItPythonPacked(fileName);
}
