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
//////////////////////////////////////////////////////////////////////////////
//
//	plNetResManager
//
//// Philosophy //////////////////////////////////////////////////////////////
//
//	"Cannot say. Saying, I would know. Do not know, so cannot say."
//												-- Zathras, "Babylon 5"
//
//	Normally, plResManager would be plenty for the servers and then some.
//	However, the normal resManager tries to do things smart, such as read in
//	keys from disk if they don't already exist and so forth. However, all the
//	servers care about is reading in enough of a key to be able to turn around
//	and write it back out to a stream. So, we overload ReadKeyAndReg() to just
//	read in a new key and return it. Our new key reffing system will guarantee
//	that the key eventually gets freed once we're done with it, and we don't
//	care about sharing keys because all we're interested in is the uoid anyway,
//	so no need to store the keys in the registry or anything.
//
//////////////////////////////////////////////////////////////////////////////

#include "plNetResManager.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnFactory/plCreatable.h"
#include "../pnNetCommon/plNetApp.h"
#include "hsStream.h" 

plNetResManager::plNetResManager()
{
}

plNetResManager::~plNetResManager()
{
}

plCreatable* plNetResManager::IReadCreatable(hsStream* s) const
{
	UInt16 hClass = s->ReadSwap16();
	if (plFactory::CanCreate(hClass))
	{
		plCreatable *pCre = plFactory::Create(hClass);
		if (!pCre)
			hsAssert( hClass == 0x8000, "Invalid creatable index" );

		return pCre;
	}

	plNetApp::StaticWarningMsg("NetResMgr::Can't create class %s", plFactory::GetNameOfClass(hClass));
	return nil;
}

void plNetResManager::IKeyReffed(plKeyImp* key) 
{
}

void plNetResManager::IKeyUnreffed(plKeyImp* key) 
{ 
	delete key; 
}
