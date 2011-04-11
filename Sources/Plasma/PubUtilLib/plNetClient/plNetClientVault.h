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
#ifndef plNetClientVault_h_inc
#define plNetClientVault_h_inc

#include "../plVault/plVaultClient.h"

class plMipmap;
class plVaultImageNode;


////////////////////////////////////////////////////////////////////

class plNetClientVault : public plVaultClient
{
protected:
	void IInitNode( plVaultNode * node );
	void IFiniNode( plVaultNode * node );
	int ISendNetMsg( plNetMsgVault * msg, UInt32 sendFlags=0 );
	void IOnTaskTimedOut( plVaultTask * task );
	bool IAmOnline( void ) const;

public:
	plNetClientVault();

	plNetApp * GetNetApp( void ) const;
	plAgeInfoSource * GetAgeInfo( void ) const;

	// static helpers to convert between plMipmap and plVaultImageNode
	static hsBool StuffImageIntoNode( plMipmap * src, plVaultImageNode * dst );
	static hsBool ExtractImageFromNode( plVaultImageNode * src );
};

////////////////////////////////////////////////////////////////////

class plNetPlayerVault : public plNetClientVault
{
protected:
	bool IIsThisMe( plVaultPlayerInfoNode * node ) const;
	void IFillOutConnectFields( plNetMsgVault * msg ) const;
	bool IIsThisMsgMine( plNetMsgVault * msg ) const;

public:
	plNetPlayerVault();
	plVaultPlayerNode * GetPlayer( void ) const;
};

#endif // plNetClientVault_h_inc
