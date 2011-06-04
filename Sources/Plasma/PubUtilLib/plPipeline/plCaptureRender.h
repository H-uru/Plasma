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

#ifndef plCaptureRender_inc
#define plCaptureRender_inc

#ifndef MF_FRONTBUFF_CAPTURE

#include "../plScene/plRenderRequest.h"

class plRenderTarget;
class plCaptureRenderMsg;

class plCaptureRenderRequest : public plRenderRequest
{
public:

	virtual void	Render(plPipeline* pipe, plPageTreeMgr* pageMgr);
};

class plCaptureRender
{
protected:
	static hsTArray<plCaptureRenderMsg*>		fProcessed;

	static hsBool		IProcess(plPipeline* pipe, const plKey& ack, plRenderTarget* targ);

	friend class plCaptureRenderRequest;

	// Only the client calls this (during the update phase).
	static void			Update();
	friend class plClient;
public:

	static hsBool		Capture(const plKey& ack, UInt16 width=800, UInt16 height=600);

};

#else // MF_FRONTBUFF_CAPTURE

#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

class plPipeline;

class plCaptureRender
{
protected:
	class CapInfo
	{
	public:
		plKey		fAck;
		UInt16		fWidth;
		UInt16		fHeight;
	};

	static hsTArray<CapInfo>		fCapReqs;

	// Only the client calls this (during the update phase).
	static void			Update(plPipeline* pipe);
	friend class plClient;
public:

	static hsBool		Capture(const plKey& ack, UInt16 width=800, UInt16 height=600);

};

#endif // MF_FRONTBUFF_CAPTURE

#endif // plCaptureRender_inc
