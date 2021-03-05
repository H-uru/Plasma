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
#ifndef _plPhysicalSndGroup_h
#define _plPhysicalSndGroup_h

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plPhysicalSndGroup Class                                                //
//  Simplistic container class to store the matchup info for a given        //
//  physical sound group. Assigning one of these objects to a physical      //
//  specifies the sound group it's in as well as the sounds it should make  //
//  when colliding against objects of other sound groups.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "HeadSpin.h"

#include "pnKeyedObject/hsKeyedObject.h"

class plSound;
class plPhysicalSndGroup : public hsKeyedObject
{
public:

    // The group enums
    enum SoundGroup
    {
        kNone = 0,
        kMetal,
        kGrass,
        kWood
    };
    
    plPhysicalSndGroup();
    plPhysicalSndGroup( uint32_t grp );
    virtual ~plPhysicalSndGroup();

    CLASSNAME_REGISTER( plPhysicalSndGroup );
    GETINTERFACE_ANY( plPhysicalSndGroup, hsKeyedObject );

    // Our required virtual
    bool MsgReceive(plMessage *pMsg) override;

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

    void PlaySlideSound(uint32_t against);
    void StopSlideSound(uint32_t against);
    void PlayImpactSound(uint32_t against);
    void SetSlideSoundVolume(uint32_t against, float volume);
    bool HasSlideSound(uint32_t against);
    bool HasImpactSound(uint32_t against);

    uint32_t GetGroup() const { return fGroup; }

    // Export only
    void    AddImpactSound( uint32_t against, plKey receiver );
    void    AddSlideSound( uint32_t against, plKey receiver );
    bool    IsSliding() { return fPlayingSlideSound; }

protected:

    enum Refs
    {
        kRefImpactSound,
        kRefSlideSound
    };

    uint32_t  fGroup;
    bool fPlayingSlideSound;

    // Sound key arrays for, well, our sounds!
    std::vector<plKey> fImpactSounds;
    std::vector<plKey> fSlideSounds;
};

#endif //_plPhysicalSndGroup_h
