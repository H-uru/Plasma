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
#ifndef plNetCommonHelpers_h_inc
#define plNetCommonHelpers_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsTimer.h"
#include "../pnNetCommon/pnNetCommon.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnFactory/plCreatable.h"

////////////////////////////////////////////////////////////////////

#ifndef SERVER
class plNetCoreStatsSummary : public plCreatable
{
	static const UInt8 plNetCoreStatsSummary::StreamVersion;
	float fULBitsPS;
	float fDLBitsPS;
	float fULPeakBitsPS;
	float fDLPeakBitsPS;
	float fULPeakPktsPS;
	float fDLPeakPktsPS;
	UInt32 fDLDroppedPackets;
public:
	plNetCoreStatsSummary();
	CLASSNAME_REGISTER( plNetCoreStatsSummary );
	GETINTERFACE_ANY( plNetCoreStatsSummary, plCreatable );
	void Read(hsStream* s, hsResMgr* mgr=nil);
	void Write(hsStream* s, hsResMgr* mgr=nil);
	float GetULBitsPS() const { return fULBitsPS; }
	float GetDLBitsPS() const { return fDLBitsPS; }
	float GetULPeakBitsPS() const { return fULPeakBitsPS; }
	float GetDLPeakBitsPS() const { return fDLPeakBitsPS; }
	float GetULPeakPktsPS() const { return fULPeakPktsPS; }
	float GetDLPeakPktsPS() const { return fDLPeakPktsPS; }
	UInt32 GetDLDroppedPackets() const { return fDLDroppedPackets; }
};
#endif // SERVER


////////////////////////////////////////////////////////////////////

class plCreatableListHelper : public plCreatable
{
	enum { kDefaultCompressionThreshold	= 255 }; // bytes
	enum Flags
	{
		kWantCompression	= 1<<0,
		kCompressed			= 1<<1,
		kWritten			= 1<<2,
	};
	UInt8								fFlags;
	std::map<UInt16,plCreatable*>		fItems;
	mutable std::vector<plCreatable*>	fManagedItems;
	UInt32	fCompressionThreshold;  // NOT WRITTEN
	std::string	fWritten;
	void IClearItems();

public:
	plCreatableListHelper();
	~plCreatableListHelper() { IClearItems();}

	CLASSNAME_REGISTER( plCreatableListHelper );
	GETINTERFACE_ANY( plCreatableListHelper, plCreatable );

	void	Read( hsStream* s, hsResMgr* mgr );
	void	Write( hsStream* s, hsResMgr* mgr );
	void	Clear() { IClearItems();	}
	void	CopyFrom( const plCreatableListHelper * other, bool manageItems );

	void	SetWantCompression( bool v ) { if ( v ) fFlags|=kWantCompression; else fFlags&=~kWantCompression; }
	bool	WantCompression() const { return ( fFlags&kWantCompression )!=0; }
	UInt32	GetCompressionThreshold() const { return fCompressionThreshold; }
	void	SetCompressionThreshold( UInt32 v ) { fCompressionThreshold=v; }
	
	// support for generic arguments
	void	AddItem( UInt16 id, plCreatable * item, bool manageItem=false );
	void	AddItem( UInt16 id, const plCreatable * item, bool manageItem=false );
	plCreatable* GetItem( UInt16 id, bool unManageItem=false ) const;
	void	RemoveItem( UInt16 id, bool unManageItem=false );
	bool	ItemExists( UInt16 id ) const;
	int		GetNumItems() const { return fItems.size();}
	// helpers for typed arguments
	void	AddString( UInt16 id, const char * value );
	void	AddString( UInt16 id, std::string & value );
	const char * GetString( UInt16 id );
	void	AddInt( UInt16 id, Int32 value );
	Int32	GetInt( UInt16 id );
	void	AddDouble( UInt16 id, double value );
	double	GetDouble( UInt16 id );
	void	GetItemsAsVec( std::vector<plCreatable*>& out );
	void	GetItems( std::map<UInt16,plCreatable*>& out );
};

/////////////////////////////////////////////////////////////////////
struct plOperationTimer
{
	bool	fRunning;
	double	fStartTime;
	double	fEndTime;
	std::string fComment;
	std::string	fSpacer;
	bool	fPrintAtStart;
	std::string	fTag;
	plOperationTimer( const char * tag="", bool printAtStart=false )
		: fRunning( false )
		, fTag( tag )
		, fStartTime( 0.0 )
		, fEndTime( 0.0 )
		, fPrintAtStart( printAtStart )
	{}
	~plOperationTimer()	{ Stop(); }
	void Start( const char * comment, int level=0 )
	{
		fSpacer = std::string( level, '\t' );
		Stop();
		fRunning = true;
		fComment = comment;
		fStartTime = hsTimer::GetSeconds();
		if ( fPrintAtStart )
		{
			hsLogEntry( plNetApp::StaticDebugMsg( "%s%s Timing: %s",
			fSpacer.c_str(), fTag.c_str(), fComment.c_str() ) );
		}
	}
	void Stop()
	{
		if ( !fRunning )
			return;
		fRunning = false;
		fEndTime = hsTimer::GetSeconds()-fStartTime;
		hsLogEntry( plNetApp::StaticDebugMsg( "%s%s Timed: %f secs: %s",
			fSpacer.c_str(), fTag.c_str(), fEndTime, fComment.c_str() ) );
	}
	double GetTime() const { return fEndTime;}
};


#endif // plNetCommonHelpers_h_inc
