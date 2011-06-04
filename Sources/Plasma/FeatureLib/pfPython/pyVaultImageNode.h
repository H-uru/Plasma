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
#ifndef _pyVaultImageNode_h_
#define _pyVaultImageNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultImageNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"
#include "../pnKeyedObject/plKey.h"

struct RelVaultNode;


class pyVaultImageNode : public pyVaultNode
{
	plKey		fMipmapKey;
	plMipmap *	fMipmap;
	
protected:
	// should only be created from C++ side
	pyVaultImageNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultImageNode(int n=0);
	

public:
	~pyVaultImageNode ();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultImageNode);
	static PyObject *New(RelVaultNode* nfsNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultImageNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultImageNode); // converts a PyObject to a pyVaultImageNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
	void Image_SetTitle( const char * text );
	void Image_SetTitleW( const wchar_t * text );
	std::string Image_GetTitle( void );
	std::wstring Image_GetTitleW( void );

	PyObject* Image_GetImage( void ); // returns pyImage
	void Image_SetImage(pyImage& image);

	void SetImageFromBuf( PyObject * buf );
	
	void SetImageFromScrShot();

};

#endif // _pyVaultImageNode_h_
