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
#include "HeadSpin.h"
#include "plBufferedSocketWriter.h"
#include "plTcpSocket.h"

#include <algorithm>

plBufferedSocketWriter::plBufferedSocketWriter(int size, int bytesPerFlush, bool blockOnSend, int flushPoint)
:   plRingBuffer(size)
,   fFlushPoint(flushPoint)
,   fBlockOnSend(blockOnSend)
,   fBytesPerFlush(bytesPerFlush)
{}


int plBufferedSocketWriter::WriteBlock(const char * data, int len, plTcpSocket & sck)
{
    int ans = kSuccessNoDataSent;
    
    if(len > BufferAvailable())
        ans = Flush(sck);
    
    if(ans >= 0)
        ans = WriteBlock(data,len);
    
    if(ans >= 0 && fFlushPoint >= 0 && fFlushPoint < AmountBuffered())
        ans = Flush(sck);
            
    return ans;
}

int plBufferedSocketWriter::WriteBlock(const char * data, int len)
{
    int ans = kSuccessNoDataSent;
    if(!Put(data,len))
        ans = kFailedNoBufferSpace;
    return ans;
}

int plBufferedSocketWriter::Flush(plTcpSocket & sck)    // this is where things get ugly.
{
    int ans = kSuccessNoDataSent;

    int writeSize = std::min(FastAmountBuffered(), fBytesPerFlush);
    
    if(writeSize > 0)
    {
        int nBytesWritten = 0;
//      int nBytesWritten = sck.SendData(FastGetBufferStart(), writeSize);

//      if (nBytesWritten<0 && fBlockOnSend
//          && plNet::GetError()==kBlockingError)
        {
            bool wasBlockingOrNot = sck.IsBlocking();
            sck.SetBlocking(fBlockOnSend);
            nBytesWritten = sck.SendData(FastGetBufferStart(), writeSize);        
            sck.SetBlocking(wasBlockingOrNot);
        }

        if (nBytesWritten > 0)
        {
            fStartPos += nBytesWritten;
            FullCompress();
            ans = kSuccessDataSent;
        }
        else if (nBytesWritten < 0)
        {
            if (plNet::GetError()!=kBlockingError)
                ans = kFailedWriteError;
        }
        else
        {
            ans = kFailedSocketClosed;
        }
    }        
    return ans;
}

bool plBufferedSocketWriter::IsEmpty()
{
    return !(FastAmountBuffered() > 0);
}

void plBufferedSocketWriter::Reset()
{
    plRingBuffer::Reset();
}

