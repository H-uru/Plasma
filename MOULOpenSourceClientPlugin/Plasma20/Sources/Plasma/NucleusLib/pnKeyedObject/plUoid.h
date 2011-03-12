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
//	plUoid - A Unique Object IDentifier -- basically, each unique Uoid refers
//			 to one and exactly one object.
//			 To define such, it contains three elements:
//				 - A plLocation, which specifies an (age,chapter,page) combo
//				   (as a sequence number)
//				 - A creatable class type (from plFactory)
//				 - An object name
//
//////////////////////////////////////////////////////////////////////////////

#ifndef plUoid_h_inc
#define plUoid_h_inc

#include "hsTypes.h"
#include "plFixedKey.h"
#include "plLoadMask.h"

class hsStream;

//// plLocation //////////////////////////////////////////////////////////////

class plLocation
{
public:
	enum LocFlags
	{
		kLocalOnly	= 0x1,	// Set if nothing in the room saves state.
		kVolatile	= 0x2,	// Set is nothing in the room persists when the server exits.
		kReserved	= 0x4,
		kBuiltIn	= 0x8,
		kItinerant	= 0x10,
	};

protected:
	UInt32 fSequenceNumber;
	UInt16 fFlags;

	enum 
	{
		kGlobalFixedLocIdx = 0,		// Fixed keys go here, think of as "global,fixed,keys"
		kSceneViewerLocIdx = 1,

		kLocalLocStartIdx = 3,		// These are a range of #s that go to local, testing-only pages.
		kLocalLocEndIdx = 32,		// You can't go over the network with any keys with these locs.

		kNormalLocStartIdx = kLocalLocEndIdx + 1,

		kReservedLocStart = 0xff000000,	// Reserved locations are ones that aren't real game locations,										
		kGlobalServerLocIdx = kReservedLocStart,	// Global pool room for the server. Only the server gets this one

		kReservedLocAvailableStart = kGlobalServerLocIdx + 1,	// This is the start of the *really* available ones
		kReservedLocEnd = 0xfffffffe,	// But instead act as a holding place for data

		kInvalidLocIdx = 0xffffffff
	};

	plLocation(UInt32 seqNum, UInt16 flags=0) : fFlags(flags) { Set(seqNum); }

public:
	plLocation() { Invalidate(); }
	plLocation(const plLocation& toCopyFrom);
	~plLocation() {}

	void	Invalidate();
	hsBool	IsValid() const;
	hsBool	IsReserved() const;
	hsBool	IsItinerant() const;
	void	Set(UInt32 seqNum);
	UInt32	GetSequenceNumber() const { return fSequenceNumber; }
	hsBool	IsVirtual() const;

	void	SetFlags(UInt16 flags) { fFlags |= flags; }
	UInt16	GetFlags() const { return fFlags; }

	void	Read(hsStream* s);
	void	Write(hsStream* s) const;

	hsBool operator==(const plLocation& loc) const;
	hsBool operator!=(const plLocation& loc) const { return !(loc == *this); }
	plLocation& operator=(const plLocation& loc);
	bool operator<(const plLocation& loc ) const { return fSequenceNumber < loc.fSequenceNumber; }

	// THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
	char* StringIze(char* str) const;  // Format to displayable string. Returns the same string for convenience

	static plLocation MakeReserved(UInt32 number);
	static plLocation MakeNormal(UInt32 number);

	static const plLocation	kGlobalFixedLoc;
	static const plLocation	kSceneViewerLoc;
	static const plLocation	kLocalStartLoc;
	static const plLocation	kLocalEndLoc;
	static const plLocation	kNormalStartLoc;
	static const plLocation kGlobalServerLoc;
	static const plLocation kInvalidLoc;
};

//// plUoid //////////////////////////////////////////////////////////////////

class plUoid
{
public:
	plUoid() { fObjectName = nil; Invalidate(); }
	plUoid(const plLocation& location, UInt16 classType, const char* objectName, const plLoadMask& m=plLoadMask::kAlways);
	plUoid(plFixedKeyId fixedKey);
	plUoid(const plUoid& src);
	~plUoid();

	const plLocation&	GetLocation() const { return fLocation; }
	UInt16				GetClassType() const { return fClassType; }
	const char*			GetObjectName() const { return fObjectName; }
	const plLoadMask&	GetLoadMask() const { return fLoadMask; }

	void Read(hsStream* s);
	void Write(hsStream* s) const;

	void Invalidate();
	hsBool IsValid() const;

	plUoid&	operator=(const plUoid& u);
	hsBool	operator==(const plUoid& u) const;
	hsBool	operator!=(const plUoid& u) const { return !operator==(u); }

	hsBool	IsClone() const				{ return fCloneID != 0; }
	UInt32	GetClonePlayerID() const	{ return fClonePlayerID; }
	UInt32	GetCloneID() const			{ return fCloneID; }
	void	SetClone(UInt32 playerID, UInt32 cloneID) { hsAssert(cloneID < 0xffff, "Clone id too high"); fCloneID = UInt16(cloneID); fClonePlayerID = playerID; }

	UInt32 GetObjectID() const { return fObjectID; }
	// Export time only.  Only plRegistryKeyList should call this.
	void SetObjectID(UInt32 id) { fObjectID = id; }

	// THIS SHOULD BE FOR DEBUGGING ONLY <hint hint>
	char* StringIze(char* str) const;  // Format to displayable string

protected:
	enum ContentsFlags	// for read/write functions
	{
		kHasCloneIDs	= 0x1,
		kHasLoadMask	= 0x2,
	};

	UInt32		fObjectID;
	UInt32		fClonePlayerID;	// The ID of the player who made this clone
	UInt16		fCloneID;		// The ID of this clone (unique per client)
	UInt16		fClassType;
	char*		fObjectName;
	plLocation	fLocation;
	plLoadMask	fLoadMask;
};

#endif // plUoid_h_inc
