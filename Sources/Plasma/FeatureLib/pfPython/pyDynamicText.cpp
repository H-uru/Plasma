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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDynamicTextMsg Header                                                 //
//  Message wrapper for commands to plDynamicTextMap.                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <Python.h>
#include "plgDispatch.h"
#include "pyKey.h"
#pragma hdrstop

#include "pyDynamicText.h"
#include "plMessage/plDynamicTextMsg.h"
#include "pyColor.h"
#include "pyImage.h"
#include "plGImage/plDynamicTextMap.h"


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
        plDynamicTextMsg* pMsg = new plDynamicTextMsg;
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
        hsColorRGBA col = color.getColor();
        pMsg->ClearToColor(col);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::Flush( void )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        pMsg->Flush();
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::PurgeImage( void )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        pMsg->PurgeImage();
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
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
        hsColorRGBA col = color.getColor();
        pMsg->SetTextColor(col,blockRGB);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::SetFont( const char *facename, int16_t size )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        pMsg->SetFont(facename,size);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::FillRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, pyColor& color )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        hsColorRGBA col = color.getColor();
        pMsg->FillRect(left,top,right,bottom,col);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::FrameRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, pyColor& color )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        hsColorRGBA col = color.getColor();
        pMsg->FrameRect(left,top,right,bottom,col);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::SetClipping( uint16_t clipLeft, uint16_t clipTop, uint16_t clipRight, uint16_t clipBottom)
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

void pyDynamicText::SetWrapping( uint16_t wrapWidth, uint16_t wrapHeight )
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
void pyDynamicText::DrawText( int16_t x, int16_t y, const char *text )
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

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void pyDynamicText::DrawTextW( int16_t x, int16_t y, std::wstring text )
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

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

//
// Draw an image on the DynamicMap
//
void pyDynamicText::DrawImage( uint16_t x, uint16_t y, pyImage& image, hsBool respectAlpha )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        plKey k = image.GetKey();
        pMsg->DrawImage( x, y, k, respectAlpha);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

//
// Draw an image on the DynamicMap
//
void pyDynamicText::DrawImageClipped( uint16_t x, uint16_t y, pyImage& image, uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch,
                                        hsBool respectAlpha )
{
    // create message
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if ( pMsg )
    {
        plKey k = image.GetKey();
        pMsg->DrawClippedImage( x, y, k, cx, cy, cw, ch, respectAlpha);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}


uint16_t  pyDynamicText::GetWidth( void )
{
    // We better just pick our first key. Note that the ONLY time we should be getting multiple receivers
    // is if the export process ends up creating multiple copies of the material. Now, WHY you'd be wanting
    // to draw all of them is a question for wiser men, but they should all be the same size regardless
    if( fReceivers.GetCount() == 0 )
        return 0;

    plDynamicTextMap *dtMap = plDynamicTextMap::ConvertNoRef( fReceivers[ 0 ]->ObjectIsLoaded() );
    if( dtMap == nil )
        return 0;

    return (uint16_t)dtMap->GetWidth();
}

uint16_t  pyDynamicText::GetHeight( void )
{
    // We better just pick our first key. Note that the ONLY time we should be getting multiple receivers
    // is if the export process ends up creating multiple copies of the material. Now, WHY you'd be wanting
    // to draw all of them is a question for wiser men, but they should all be the same size regardless
    if( fReceivers.GetCount() == 0 )
        return 0;

    plDynamicTextMap *dtMap = plDynamicTextMap::ConvertNoRef( fReceivers[ 0 ]->ObjectIsLoaded() );
    if( dtMap == nil )
        return 0;

    return (uint16_t)dtMap->GetHeight();
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

    width = dtMap->CalcStringWidth(text.c_str(), (uint16_t*)&height);
}

void pyDynamicText::SetJustify(uint8_t justify)
{
    plDynamicTextMsg* pMsg = ICreateDTMsg();
    if (pMsg)
    {
        pMsg->SetJustify(justify);
        plgDispatch::MsgSend(pMsg);
    }
}

void pyDynamicText::SetLineSpacing(int16_t spacing)
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
