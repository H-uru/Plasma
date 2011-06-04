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
//																			//
//	plDynamicTextMsg Header													//
//	Message wrapper for commands to plDynamicTextMap.						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "plgDispatch.h"
#include "../plMessage/plDynamicTextMsg.h"
#include "pyKey.h"
#include "plPythonFileMod.h"
#include "pyColor.h"
#include "pyImage.h"
#include "../plGImage/plDynamicTextMap.h"

#include "pyDynamicText.h"

pyDynamicText::pyDynamicText()
{
	IInit();
}

pyDynamicText::pyDynamicText(pyKey& key) 
{
	IInit();
	fReceivers.Append(key.getKey());
}

pyDynamicText::pyDynamicText(plKey key) 
{
	IInit();
	fReceivers.Append(key);
}


void pyDynamicText::IInit()
{
	fSenderKey = nil;
	fNetPropagate = false;
	fNetForce = false;
	fWrap = false;
	fClip = false;
}


// methods that will be exposed to Python
void pyDynamicText::SetSender(pyKey& selfKey)
{
	fSenderKey = selfKey.getKey();
}

void pyDynamicText::ClearReceivers()
{
	fReceivers.Reset();
}

void pyDynamicText::AddReceiver(pyKey& key)
{
	fReceivers.Append(key.getKey());
}


void pyDynamicText::SetNetPropagate(hsBool propagate)
{
	fNetPropagate = propagate;
}

void pyDynamicText::SetNetForce(hsBool force)
{
	fNetForce = force;
}

//////////////////////////////////////////////
//   internal methods
//////////////////////////////////////////////

plDynamicTextMsg* pyDynamicText::ICreateDTMsg()
{
	// must have a receiver!
	if ( fReceivers.Count() > 0 )
	{
		plDynamicTextMsg* pMsg = TRACKED_NEW plDynamicTextMsg;
		if ( fSenderKey )
			pMsg->SetSender(fSenderKey);
		if ( fNetPropagate )
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
		if ( fNetForce )
			pMsg->SetBCastFlag(plMessage::kNetForce);
		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fReceivers.Count(); i++ )
		{
			pMsg->AddReceiver(fReceivers[i]);
		}

		return pMsg;
	}
	else
		return nil;
}

//////////////////////////////////////////////
//     dynamicText commands
//////////////////////////////////////////////

//
//  Need to determine whether to send over the network (netpropagate) or not
//

void pyDynamicText::ClearToColor( pyColor& color )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->ClearToColor(color.getColor());

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::Flush( void )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->Flush();
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::PurgeImage( void )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->PurgeImage();
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::SetTextColor( pyColor& color  )
{
	SetTextColor2(color,false);
}

void pyDynamicText::SetTextColor2( pyColor& color, bool blockRGB )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->SetTextColor(color.getColor(),blockRGB);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::SetFont( const char *facename, Int16 size )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->SetFont(facename,size);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::FillRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, pyColor& color )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->FillRect(left,top,right,bottom,color.getColor());
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::FrameRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, pyColor& color )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->FrameRect(left,top,right,bottom,color.getColor());
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::SetClipping( UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom)
{
	fClip = true;
	fClipLeft = clipLeft;
	fClipTop = clipTop;
	fClipRight = clipRight;
	fClipBottom = clipBottom;
}

void pyDynamicText::UnsetClipping()
{
	fClip = false;
}

void pyDynamicText::SetWrapping( UInt16 wrapWidth, UInt16 wrapHeight )
{
	fWrap = true;
	fWrapWidth = wrapWidth;
	fWrapHeight = wrapHeight;
}

void pyDynamicText::UnsetWrapping()
{
	fWrap = false;
}

//
// Draw text paying attention to Clipping and Wrapping if user wanted it
//
void pyDynamicText::DrawText( Int16 x, Int16 y, const char *text )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		// The priority is:
		//  1) wrap (if you wrap you probably don't need to clip)
		//  2) clip
		//  3) just draw
		if ( fWrap )
			pMsg->DrawWrappedString(x,y,fWrapWidth,fWrapHeight,text);
		else if ( fClip )
			pMsg->DrawClippedString(x,y,fClipLeft,fClipTop,fClipRight,fClipBottom,text);
		else
			pMsg->DrawString(x,y,text);

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void pyDynamicText::DrawTextW( Int16 x, Int16 y, std::wstring text )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		// The priority is:
		//  1) wrap (if you wrap you probably don't need to clip)
		//  2) clip
		//  3) just draw
		if ( fWrap )
			pMsg->DrawWrappedString(x,y,fWrapWidth,fWrapHeight,text.c_str());
		else if ( fClip )
			pMsg->DrawClippedString(x,y,fClipLeft,fClipTop,fClipRight,fClipBottom,text.c_str());
		else
			pMsg->DrawString(x,y,text.c_str());

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

//
// Draw an image on the DynamicMap
//
void pyDynamicText::DrawImage( UInt16 x, UInt16 y, pyImage& image, hsBool respectAlpha )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->DrawImage( x, y, image.GetKey(), respectAlpha);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

//
// Draw an image on the DynamicMap
//
void pyDynamicText::DrawImageClipped( UInt16 x, UInt16 y, pyImage& image, UInt16 cx, UInt16 cy, UInt16 cw, UInt16 ch,
										hsBool respectAlpha )
{
	// create message
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if ( pMsg )
	{
		pMsg->DrawClippedImage( x, y, image.GetKey(), cx, cy, cw, ch, respectAlpha);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


UInt16	pyDynamicText::GetWidth( void )
{
	// We better just pick our first key. Note that the ONLY time we should be getting multiple receivers
	// is if the export process ends up creating multiple copies of the material. Now, WHY you'd be wanting
	// to draw all of them is a question for wiser men, but they should all be the same size regardless
	if( fReceivers.GetCount() == 0 )
		return 0;

	plDynamicTextMap *dtMap = plDynamicTextMap::ConvertNoRef( fReceivers[ 0 ]->ObjectIsLoaded() );
	if( dtMap == nil )
		return 0;

	return (UInt16)dtMap->GetWidth();
}

UInt16	pyDynamicText::GetHeight( void )
{
	// We better just pick our first key. Note that the ONLY time we should be getting multiple receivers
	// is if the export process ends up creating multiple copies of the material. Now, WHY you'd be wanting
	// to draw all of them is a question for wiser men, but they should all be the same size regardless
	if( fReceivers.GetCount() == 0 )
		return 0;

	plDynamicTextMap *dtMap = plDynamicTextMap::ConvertNoRef( fReceivers[ 0 ]->ObjectIsLoaded() );
	if( dtMap == nil )
		return 0;

	return (UInt16)dtMap->GetHeight();
}

void pyDynamicText::CalcTextExtents( std::wstring text, unsigned &width, unsigned &height )
{
	width = 0;
	height = 0;

	if (fReceivers.GetCount() == 0)
		return;

	plDynamicTextMap* dtMap = plDynamicTextMap::ConvertNoRef(fReceivers[0]->ObjectIsLoaded());
	if (!dtMap)
		return;

	width = dtMap->CalcStringWidth(text.c_str(), (UInt16*)&height);
}

void pyDynamicText::SetJustify(UInt8 justify)
{
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if (pMsg)
	{
		pMsg->SetJustify(justify);
		plgDispatch::MsgSend(pMsg);
	}
}

void pyDynamicText::SetLineSpacing(Int16 spacing)
{
	plDynamicTextMsg* pMsg = ICreateDTMsg();
	if (pMsg)
	{
		pMsg->SetLineSpacing(spacing);
		plgDispatch::MsgSend(pMsg);
	}
}

plKey pyDynamicText::GetImage()
{
	if (fReceivers.GetCount() > 0)
		return fReceivers[0];
	else
		return nil;
}