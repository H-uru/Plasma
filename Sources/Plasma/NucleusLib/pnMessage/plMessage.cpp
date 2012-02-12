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
#define PLMESSAGE_PRIVATE
#include "plMessage.h"
#include "hsStream.h"
#include "pnKeyedObject/plKey.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "hsTemplates.h"
#include "plgDispatch.h"
#include "hsBitVector.h"
#include <algorithm>
#include <iterator>

plMessage::plMessage()
:   fSender(nil),
    fBCastFlags(kLocalPropagate),
    fTimeStamp(0),
    fNetRcvrPlayerIDs(nil),
    dispatchBreak(false)
{
}

plMessage::plMessage(const plKey &s, 
            const plKey &r, 
            const double* t)
:   fSender(s),
    fBCastFlags(kLocalPropagate),
    fNetRcvrPlayerIDs(nil),
    dispatchBreak(false)
{
    if( r )
    {
        fReceivers.SetCount(1);
        fReceivers[0] = r;
    }
    fTimeStamp = t ? *t : hsTimer::GetSysSeconds();
}

plMessage::~plMessage()
{
    delete fNetRcvrPlayerIDs;
}

plMessage&      plMessage::SetNumReceivers(int n) { fReceivers.SetCount(n); return *this; }
uint32_t          plMessage::GetNumReceivers() const { return fReceivers.GetCount(); }
const plKey&    plMessage::GetReceiver(int i) const { return fReceivers[i]; }
plMessage&      plMessage::RemoveReceiver(int i) { fReceivers.Remove(i); return *this; }

plMessage&      plMessage::ClearReceivers() { fReceivers.SetCount(0); return *this; }
plMessage&      plMessage::AddReceiver(const plKey &r) { fReceivers.Append(r); return *this; }

plMessage& plMessage::AddReceivers(const hsTArray<plKey>& rList)
{
    int i;
    for( i = 0; i < rList.GetCount(); i++ )
        AddReceiver(rList[i]);

    return *this;
}

hsBool plMessage::Send(const plKey r, hsBool async)
{
    if( r )
        AddReceiver(r);
    return plgDispatch::MsgSend(this,async);
}

hsBool plMessage::SendAndKeep(const plKey r, hsBool async)
{
    Ref();
    return Send(r, async);
}

void plMessage::IMsgRead(hsStream* s, hsResMgr* mgr)
{
    plCreatable::Read(s, mgr);

    fSender = mgr->ReadKey(s);
    int n;
    s->LogReadLE(&n,"NumberOfReceivers"); 
    fReceivers.SetCount(n);
    int i;
    for( i = 0; i < fReceivers.GetCount(); i++ )
        fReceivers[i] = mgr->ReadKey(s);

    s->LogReadLE(&fTimeStamp,"TimeStamp");    // read as double
    s->LogReadLE(&fBCastFlags, "BCastFlags");
}

void plMessage::IMsgWrite(hsStream* s, hsResMgr* mgr)
{
    plCreatable::Write(s, mgr);
    
    mgr->WriteKey(s,fSender);
    s->WriteLE32(fReceivers.GetCount());
    int i;
    for( i = 0; i < fReceivers.GetCount(); i++ )
        mgr->WriteKey(s,fReceivers[i]);

    s->WriteLE(fTimeStamp);   // write as double
    s->WriteLE32(fBCastFlags);
}

enum MsgFlags
{
    kMsgSender,
    kMsgReceivers,
    kMsgTimeStamp,
    kMsgBCastFlags,
};

void plMessage::IMsgReadVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kMsgSender))
        fSender = mgr->ReadKey(s);

    if (contentFlags.IsBitSet(kMsgReceivers))
    {
        int n = s->ReadLE32();
        fReceivers.SetCount(n);
        int i;
        for( i = 0; i < fReceivers.GetCount(); i++ )
            fReceivers[i] = mgr->ReadKey(s);
    }

    if (contentFlags.IsBitSet(kMsgTimeStamp))
        s->ReadLE(&fTimeStamp);   // read as double

    if (contentFlags.IsBitSet(kMsgBCastFlags))
        fBCastFlags = s->ReadLE32();
}

void plMessage::IMsgWriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kMsgSender);
    contentFlags.SetBit(kMsgReceivers);
    contentFlags.SetBit(kMsgTimeStamp);
    contentFlags.SetBit(kMsgBCastFlags);
    contentFlags.Write(s);

    // kMsgSender
    mgr->WriteKey(s,fSender);

    // kMsgReceivers
    s->WriteLE32(fReceivers.GetCount());
    int i;
    for( i = 0; i < fReceivers.GetCount(); i++ )
        mgr->WriteKey(s,fReceivers[i]);

    // kMsgTimeStamp
    s->WriteLE(fTimeStamp);   // write as double

    // kMsgBCastFlags
    s->WriteLE32(fBCastFlags);
}

void plMessage::AddNetReceiver( uint32_t plrID )
{
    if ( !fNetRcvrPlayerIDs )
        fNetRcvrPlayerIDs = new std::vector<uint32_t>;
    fNetRcvrPlayerIDs->push_back( plrID );
}

void plMessage::AddNetReceivers( const std::vector<uint32_t> & plrIDs )
{
    if ( !fNetRcvrPlayerIDs )
        fNetRcvrPlayerIDs = new std::vector<uint32_t>;
    std::copy( plrIDs.begin(), plrIDs.end(), std::back_inserter( *fNetRcvrPlayerIDs ) );
}

/////////////////////////////////////////////////////////////////

// STATIC
int plMsgStdStringHelper::Poke(const std::string & stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    hsAssert( stringref.length()<0xFFFF, "buf too big for plMsgStdStringHelper" );
    strlen = stringref.length();
    stream->WriteLE(strlen);
    if (strlen)
        stream->Write(strlen,stringref.data());
    return stream->GetPosition();
}

int plMsgStdStringHelper::PokeBig(const std::string & stringref, hsStream* stream, const uint32_t peekOptions)
{
    uint32_t strlen = stringref.length();
    stream->WriteLE(strlen);
    if (strlen)
        stream->Write(strlen,stringref.data());
    return stream->GetPosition();
}

int plMsgStdStringHelper::Poke(const char * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    hsAssert( bufsz<0xFFFF, "buf too big for plMsgStdStringHelper" );
    strlen = (plMessage::plStrLen)bufsz;
    stream->WriteLE(strlen);
    if (strlen)
        stream->Write(strlen,buf);
    return stream->GetPosition();
}

int plMsgStdStringHelper::PokeBig(const char * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    stream->WriteLE(bufsz);
    if (bufsz)
        stream->Write(bufsz,buf);
    return stream->GetPosition();
}

int plMsgStdStringHelper::Poke(const plString & stringref, hsStream* stream, const uint32_t peekOptions)
{
    std::string temp = stringref.c_str();
    return Poke(temp, stream, peekOptions);
}

int plMsgStdStringHelper::PokeBig(const plString & stringref, hsStream* stream, const uint32_t peekOptions)
{
    std::string temp = stringref.c_str();
    return PokeBig(temp, stream, peekOptions);
}

// STATIC
int plMsgStdStringHelper::Peek(std::string  & stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    stream->LogSubStreamStart("push this");
    stream->LogReadLE(&strlen,"StrLen");
    stringref.erase();
    if (strlen <= stream->GetSizeLeft())
    {
        stringref.resize(strlen);
        if (strlen){
            stream->LogRead(strlen,(void*)stringref.data(),"StdString");
            stream->LogStringString(xtl::format("Value: %s", stringref.data()).c_str());
        }
    }
    else
    {
        hsAssert( false, "plMsgStdStringHelper::Peek: overflow peeking string." );
    }
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}

int plMsgStdStringHelper::PeekBig(std::string  & stringref, hsStream* stream, const uint32_t peekOptions)
{
    uint32_t bufsz;
    stream->LogSubStreamStart("push this");
    stream->LogReadLE(&bufsz,"Bufsz");
    stringref.erase();
    if (bufsz <= stream->GetSizeLeft())
    {
        stringref.resize(bufsz);
        if (bufsz){
            stream->LogRead(bufsz,(void*)stringref.data(),"StdString");
            stream->LogStringString(xtl::format("Value: %s", stringref.data()).c_str());
        }
    }
    else
    {
        hsAssert( false, "plMsgStdStringHelper::PeekBig: overflow peeking string." );
    }
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}

int plMsgStdStringHelper::Peek(plString & stringref, hsStream* stream, const uint32_t peekOptions)
{
    std::string temp;
    int pos = Peek(temp, stream, peekOptions);
    stringref = plString::FromIso8859_1(temp.c_str(), temp.size());
    return pos;
}

int plMsgStdStringHelper::PeekBig(plString & stringref, hsStream* stream, const uint32_t peekOptions)
{
    std::string temp;
    int pos = PeekBig(temp, stream, peekOptions);
    stringref = plString::FromIso8859_1(temp.c_str(), temp.size());
    return pos;
}

/////////////////////////////////////////////////////////////////

// STATIC
int plMsgXtlStringHelper::Poke(const xtl::istring & stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    strlen = stringref.length();
    stream->WriteLE(strlen);
    if (strlen)
        stream->Write(strlen,stringref.data());
    return stream->GetPosition();
}

// STATIC
int plMsgXtlStringHelper::Peek(xtl::istring & stringref, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    stream->LogSubStreamStart("push me");
    stream->LogReadLE(&strlen,"StrLen");
    stringref.erase();
    if (strlen <= stream->GetSizeLeft())
    {
        stringref.resize(strlen);
        if (strlen){
            stream->LogRead(strlen,(void*)stringref.data(),"XtlString");
            stream->LogStringString(xtl::format("Value: %s", stringref.data()).c_str());
        }
    }
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}


/////////////////////////////////////////////////////////////////

// STATIC
int plMsgCStringHelper::Poke(const char * str, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    strlen = (str)?hsStrlen(str):0;
    stream->WriteLE(strlen);
    if (strlen)
        stream->Write(strlen,str);
    return stream->GetPosition();
}

// STATIC
int plMsgCStringHelper::Peek(char *& str, hsStream* stream, const uint32_t peekOptions)
{
    plMessage::plStrLen strlen;
    stream->LogSubStreamStart("push me");
    stream->LogReadLE(&strlen,"StrLen");
    delete [] str;
    str = nil;
    if (strlen <= stream->GetSizeLeft())
    {
        if (strlen)
        {
            str = new char[strlen+1];
            str[strlen] = '\0';
            if (strlen) {
                stream->LogRead(strlen,str,"CString");
                stream->LogStringString(xtl::format("Value: %s",str).c_str());
            }
        }
    }
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}

int plMsgCStringHelper::Poke(const plString & str, hsStream* stream, const uint32_t peekOptions)
{
    return Poke(str.c_str(), stream, peekOptions);
}

int plMsgCStringHelper::Peek(plString & str, hsStream* stream, const uint32_t peekOptions)
{
    char * temp = nil;
    int pos = Peek(temp, stream, peekOptions);
    str = plString::FromIso8859_1(temp);
    delete [] temp;
    return pos;
}


/////////////////////////////////////////////////////////////////

// STATIC
int plMsgCArrayHelper::Poke(const void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    stream->Write(bufsz,buf);
    return stream->GetPosition();
}

// STATIC
int plMsgCArrayHelper::Peek(void * buf, uint32_t bufsz, hsStream* stream, const uint32_t peekOptions)
{
    stream->LogSubStreamStart("push me");
    stream->LogRead(bufsz,buf,"CArray");
    stream->LogSubStreamEnd();
    return stream->GetPosition();
}

