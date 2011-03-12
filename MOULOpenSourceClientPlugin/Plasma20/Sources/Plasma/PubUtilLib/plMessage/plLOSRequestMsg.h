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
#ifndef	plLOSRequestMsg_inc
#define	plLOSRequestMsg_inc

#include "../pnMessage/plMessage.h"
#include "../plPhysical/plSimDefs.h"
#include "hsGeometry3.h"

class plLOSRequestMsg :	public plMessage
{
public:
	enum TestType {
		kTestClosest,		// this	test has to	keep going after finding a hit until it's ruled	out	others
		kTestAny			// this	is faster because it will stop as soon as it finds any hit
	};

	enum ReportType
	{
		kReportHit,
		kReportMiss,
		kReportHitOrMiss
	};

	plLOSRequestMsg();
	plLOSRequestMsg(const plKey& sender, hsPoint3& fromPoint, hsPoint3&	toPoint, plSimDefs::plLOSDB	db,
		TestType test =	kTestAny, ReportType report	= kReportHit);

	void SetFrom(const hsPoint3& from) { fFrom = from; }
	void SetTo(const hsPoint3& to) { fTo = to; }

	void SetWorldKey(plKey world) { fWorldKey = world; }

	/**	Determines which database we're	testing	against.
		See	plSimDefs::plLOSDB for a list of valid databases. */
	void SetRequestType(plSimDefs::plLOSDB type) { fRequestType	= type;	}
	plSimDefs::plLOSDB GetRequestType()	const {	return fRequestType; }

	/**	Determine whether we'll	return on the first	hit	we get (kTestAny)
		or keep	looking	until we're	sure we've found the closest hit (kTestClosest)
		The	latter is slower because it	has	to look	at all possible	hits. */
	void SetTestType(TestType type)	{ fTestType	= type;	}
	TestType GetTestType() const { return fTestType; }

	/**	Do we report when we find a	hit, when we don't find	a hit, or both?	*/
	void SetReportType(ReportType type)	{ fReportType =	type; }
	ReportType GetReportType() const { return fReportType; }

	/**	An ID invented by the caller for their own bookkeeping.	*/
	void SetRequestID(UInt32 id) { fRequestID =	id;	}
	UInt32 GetRequestID() {	return fRequestID; }

	/**	If we get a	hit	on the first pass, we'll then double-check the remaining
		segment	(start->first hit) against the "cull db". If *any* hit is found,
		the	entire test	fails. */
	void SetCullDB(plSimDefs::plLOSDB db) {	fCullDB	= db; }
	plSimDefs::plLOSDB GetCullDB() { return	fCullDB; }

	CLASSNAME_REGISTER(	plLOSRequestMsg	);
	GETINTERFACE_ANY( plLOSRequestMsg, plMessage );

	// Local only, runtime,	no IO necessary
	virtual	void Read(hsStream*	s, hsResMgr* mgr) {}
	virtual	void Write(hsStream* s,	hsResMgr* mgr) {}
	
private:
	friend class plLOSDispatch;
	hsPoint3				fFrom;
	hsPoint3				fTo;
	plKey					fWorldKey;			// Force the query to happen in	a particular subworld.
	plSimDefs::plLOSDB		fRequestType;		// which database are we probing?
	plSimDefs::plLOSDB		fCullDB;			// if we find a	hit, see if	anything in	this DB	blocks it.
	TestType				fTestType;			// testing closest hit or just any?
	ReportType				fReportType;		// reporting hits, misses, or both?
	UInt32					fRequestID;
};

#endif // plLOSRequestMsg_inc
