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

#ifndef plDetectorModifier_inc
#define plDetectorModifier_inc

#include "pnModifier/plSingleModifier.h"
#include "pnMessage/plObjRefMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"

class plDetectorModifier : public plSingleModifier
{
protected:
    virtual bool IEval(double secs, float del, uint32_t dirty){ return true; }

    hsTArray<plKey>     fReceivers;
    plModifier*         fRemoteMod;
    plKey               fProxyKey;

public:
    plDetectorModifier() : fRemoteMod(nil),fProxyKey(nil) { }
    virtual ~plDetectorModifier() { }
    
//  virtual bool MsgReceive(plMessage* msg) = 0;

    CLASSNAME_REGISTER( plDetectorModifier );
    GETINTERFACE_ANY( plDetectorModifier, plSingleModifier );
    void AddLogicObj(plKey pKey) { fReceivers.Append(pKey); }
    void SetRemote(plModifier* p) { fRemoteMod = p; }
    plModifier* RemoteMod() { return fRemoteMod; }
    virtual void SetType(int8_t i) { }
    int GetNumReceivers() const { return fReceivers.Count(); }
    plKey GetReceiver(int i) const { return fReceivers[i]; }
    void SetProxyKey(const plKey &k) { fProxyKey = k; }
    void Read(hsStream* stream, hsResMgr* mgr)
    {
        plSingleModifier::Read(stream, mgr);
        int n = stream->ReadLE32();
        fReceivers.Reset();
        for(int i = 0; i < n; i++ )
        {   
            fReceivers.Append(mgr->ReadKey(stream));
        }
        mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
        fProxyKey = mgr->ReadKey(stream);
    }

    void Write(hsStream* stream, hsResMgr* mgr)
    {
        plSingleModifier::Write(stream, mgr);
        stream->WriteLE32(fReceivers.GetCount());
        for( int i = 0; i < fReceivers.GetCount(); i++ )
            mgr->WriteKey(stream, fReceivers[i]);
        
        mgr->WriteKey(stream, fRemoteMod);
        mgr->WriteKey(stream, fProxyKey);
    }
};


#endif //plDetectorModifier_inc
