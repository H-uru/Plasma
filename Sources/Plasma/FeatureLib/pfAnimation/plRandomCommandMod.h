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

#ifndef plRandomCommandMod_inc
#define plRandomCommandMod_inc

#include "pnModifier/plSingleModifier.h"

class plRandomCommandMod : public plSingleModifier
{
public:
    enum {
        kNormal             = 0x0,  // randomly select the next
        kNoRepeats          = 0x1,  // random, but no cmd twice in a row
        kCoverall           = 0x2,  // random, but no cmd played twice till all cmds played
        kOneCycle           = 0x4,  // after playing through all cmds, stop
        kOneCmd             = 0x8,  // after playing a random cmd, stop until started again.
        kDelayFromEnd       = 0x10,
        kSequential         = 0x20
    };

    enum {
        kStopped            = 0x1
    };
protected:

    // These are only lightly synched, the only synched state is whether
    // they are currently active.
    uint8_t             fState;

    hsBitVector         fExcluded;
    int8_t              fCurrent;
    uint8_t             fMode; // static, if it becomes dynamic, move to SynchedValue
    std::vector<double> fEndTimes;

    float               fMinDelay;
    float               fMaxDelay;
    
    void            IStart();
    virtual void    IStop();
    bool            IStopped() const { return hsCheckBits(fState, kStopped); }
    void            IRetry(float secs);
    virtual void    IPlayNextIfMaster();

    void            IReset();
    
    float        IGetDelay(float len) const;      
    
    int             IExcludeSelections(int ncmds);
    bool            ISelectNext(int nAnim); // return false if we should stop, else set fCurrent to next index

    // Once fCurrent is set to the next animation index to play, 
    // IPlayNext() does whatever it takes to actually play it.
    virtual void        IPlayNext() = 0;

    // We only act in response to messages.
    bool IEval(double secs, float del, uint32_t dirty) override { return false; }

public:
    plRandomCommandMod();
    ~plRandomCommandMod();

    CLASSNAME_REGISTER( plRandomCommandMod );
    GETINTERFACE_ANY( plRandomCommandMod, plSingleModifier );

    bool MsgReceive(plMessage* pMsg) override;
    
    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    // Export only
    void    SetMode(uint8_t m) { fMode = m; }
    uint8_t GetMode() const { return fMode; }

    void    SetState(uint8_t s) { fState = s; }
    uint8_t GetState() const { return fState; }

    void    SetMinDelay(float f) { fMinDelay = f; }
    float   GetMinDelay() const { return fMinDelay; }

    void    SetMaxDelay(float f) { fMaxDelay = f; }
    float   GetMaxDelay() const { return fMaxDelay; }
};


#endif // plRandomCommandMod_inc

