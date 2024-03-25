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

#include "plNetClientMgr.h"
#include "plNetLinkingMgr.h"

#include <cmath>

#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plArmatureMod.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plPipeline/plDebugText.h"
#include "plPipeline/plPlates.h"
#include "plScene/plRelevanceMgr.h"

//
// Code which displays stuff on the screen
//

//
// show lists of members, listenList, and talkList
//
void plNetClientMgr::IShowLists()
{
    plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();
    plDebugText     &txt = plDebugText::Instance();

    int y,x,i;
    const int yOff=10, xOff=300, startY=70, startX=10;
    ST::string str;

    // My player info
    x=startX;
    y=startY;
    plSceneObject *player = plSceneObject::ConvertNoRef(GetLocalPlayer());
    hsPoint3 pos = (player ? player->GetLocalToWorld() * hsPoint3(0, 0, 0) : hsPoint3(0, 0, 0));
    str = ST::format("{}{} PlyrName={} PlyrID={} Join#={} ILIAS={} {.1f},{.1f},{.1f}",
        GetFlagsBit(kSendingVoice) ? "V" : " ",
        GetFlagsBit(kSendingActions) ? "A" : " ",
        GetPlayerName().c_str(), GetPlayerID(),
        GetJoinOrder(),
        IsLoadingInitialAgeState(),
        pos.fX, pos.fY, pos.fZ);
    txt.DrawString(x,y,str,255,255,255,255);
    SetFlagsBit(kSendingVoice, 0);
    SetFlagsBit(kSendingActions, 0);

    // MEMBERS
    y+=yOff;
    int baseY=y;
    txt.DrawString(x, y, ST_LITERAL("   Members"), 255, 255, 255, 255, plDebugText::kStyleBold);
    y+=yOff;
    std::vector<plNetTransportMember*> members = fTransport.GetMemberListDistSorted();
    for (plNetTransportMember* mbr : members)
    {
        hsAssert(mbr, "ShowLists: nil member?");
        if (mbr->IsServer())
            continue;
        player = (mbr->GetAvatarKey() ? plSceneObject::ConvertNoRef(mbr->GetAvatarKey()->ObjectIsLoaded()) : nullptr);
        str = ST::format("{}{} {} dist={.1f}",
            mbr->GetTransportFlags() & plNetTransportMember::kSendingVoice ? "V" : " ",
            mbr->GetTransportFlags() & plNetTransportMember::kSendingActions ? "A" : " ",
            mbr->AsString().c_str(),
            mbr->GetDistSq() != FLT_MAX ? sqrt(mbr->GetDistSq()) :-1.f);
        txt.DrawString(x,y,str);
        y+=yOff;
        mbr->SetTransportFlags(mbr->GetTransportFlags() & 
            ~(plNetTransportMember::kSendingVoice|plNetTransportMember::kSendingActions));
    }

    // LISTENLIST
    x+=xOff;
    y=baseY;
    txt.DrawString(x, y, ST_LITERAL("ListenList"), 255, 255, 255, 255, plDebugText::kStyleBold);
    y+=yOff;
    for(i=0;i<GetListenList()->GetNumMembers();i++)
    {
        txt.DrawString(x, y, ST::format("name={}", GetListenList()->GetMember(i)->AsString()));
        y+=yOff;
    }

    // TALKLIST
    x+=xOff;
    y=baseY;
    txt.DrawString(x, y, ST_LITERAL("TalkList"), 255, 255, 255, 255, plDebugText::kStyleBold);
    y+=yOff;
    for(i=0;i<GetTalkList()->GetNumMembers();i++)
    {
        txt.DrawString(x, y, ST::format("name={}", GetTalkList()->GetMember(i)->AsString()));
        y+=yOff;
    }
}


//
// show lists of members, listenList, and talkList
//
void plNetClientMgr::IShowRooms()
{
    plDebugText     &txt = plDebugText::Instance();

    int y,x;
    const int yOff=10, xOff=300, startY=70, startX=10;

    // OWNEDLIST
    x=startX;
    y=startY;
    txt.DrawString(x, y, ST_LITERAL("RoomsOwned"), 255, 255, 255, 255, plDebugText::kStyleBold);
    y+=yOff;
    std::set<plNetClientGroups::OwnedGroup>::iterator it=GetNetGroups()->fGroups.begin();
    for(;it != GetNetGroups()->fGroups.end(); it++)
    {
        if ((*it).fOwnIt)
        {
            txt.DrawString(x, y, it->fGroup.GetDesc());
            y+=yOff;
        }
    }

    // NOTOWNEDLIST
    x+=xOff;
    y=startY;
    txt.DrawString(x, y, ST_LITERAL("RoomsNotOwned"), 255, 255, 255, 255, plDebugText::kStyleBold);
    y+=yOff;
    it=GetNetGroups()->fGroups.begin();
    for(;it != GetNetGroups()->fGroups.end(); it++)
    {
        if (!(*it).fOwnIt)
        {
            txt.DrawString(x, y, it->fGroup.GetDesc());
            y+=yOff;
        }
    }
}

uint32_t IPrintRelRegion(const hsBitVector& region, int x, int y, const hsBitVector* cmpRegion)
{
    uint32_t num = plRelevanceMgr::Instance()->GetNumRegions();

    bool orTrue = false;

    ST::string_stream buf;
    int i;
    for (i = 0; i < num; i++)
    {
        buf << (region.IsBitSet(i) ? '1' : '0');
        if (cmpRegion && cmpRegion->IsBitSet(i) && region.IsBitSet(i))
            orTrue = true;
    }
    ST::string str = buf.to_string();

    plDebugText& txt = plDebugText::Instance()  ;
    if (orTrue)
        txt.DrawString(x, y, str, 0, 255, 0);
    else
        txt.DrawString(x, y, str);

    return txt.CalcStringWidth(str);
}

void plNetClientMgr::IShowRelevanceRegions()
{
    plDebugText& txt = plDebugText::Instance();

    const int yOff = 12, xOff = 20, startY=70, startX=10;
    int x = startX, y = startY;

    txt.DrawString(x, y - yOff, ST_LITERAL("Name / In / Care"), 255, 255, 255, 255, plDebugText::kStyleBold);

    std::vector<plNetTransportMember*> members =  fTransport.GetMemberListDistSorted();

    //
    // Print out the player names in the first column
    //
    uint32_t maxPlayerName = 0;

    txt.DrawString(x, y, GetPlayerName());
    maxPlayerName = std::max(maxPlayerName, txt.CalcStringWidth(GetPlayerName()));
    y += yOff;

    for (plNetTransportMember* mbr : members)
    {
        hsAssert(mbr, "ShowLists: nil member?");
        if (mbr->IsServer())
            continue;

        ST::string name = mbr->GetPlayerName();
        txt.DrawString(x, y, name);
        maxPlayerName = std::max(maxPlayerName, txt.CalcStringWidth(name));
        y += yOff;
    }

    x = startX + maxPlayerName + xOff;
    y = startY;

    //
    // Print out the regions
    //
    const hsBitVector* ourCare = nullptr;
    const hsBitVector* ourIn = nullptr;

    plSceneObject* player = plSceneObject::ConvertNoRef(GetLocalPlayer());
    if (player)
    {
        const plArmatureMod *avMod = plArmatureMod::ConvertNoRef(player->GetModifierByType(plArmatureMod::Index()));
        if (avMod)
        {
            ourIn = &avMod->GetRelRegionImIn();
            uint32_t width = IPrintRelRegion(*ourIn, x, y, nullptr);

            ourCare = &avMod->GetRelRegionCareAbout();
            IPrintRelRegion(*ourCare, x + width + xOff, y, nullptr);

            y += yOff;
        }
    }

    for (plNetTransportMember* mbr : members)
    {
        if (mbr->IsServer())
            continue;

        player = (mbr->GetAvatarKey() ? plSceneObject::ConvertNoRef(mbr->GetAvatarKey()->ObjectIsLoaded()) : nullptr);
        if (player)
        {
            const plArmatureMod* avMod = plArmatureMod::ConvertNoRef(player->GetModifierByType(plArmatureMod::Index()));
            if (avMod)
            {
                const hsBitVector& in = avMod->GetRelRegionImIn();
                uint32_t width = IPrintRelRegion(in, x, y, ourCare);

                const hsBitVector& care = avMod->GetRelRegionCareAbout();
                IPrintRelRegion(care, x + width + xOff, y, ourIn);

                y += yOff;
            }
        }
    }
}

void plNetClientMgr::IShowAvatars()
{
    plDebugText     &txt = plDebugText::Instance();
    txt.SetFont(ST_LITERAL("Courier New"), 6);

    int y, x;
    const int yOff=10, xOff=285, startY=100, startX=10;
    ST::string str;

    // Me
    x=startX;
    y=startY-yOff*3;
    plSceneObject *player = plSceneObject::ConvertNoRef(GetLocalPlayer());
    hsPoint3 pos = (player ? player->GetLocalToWorld() * hsPoint3() : hsPoint3());
    hsVector3 ori = (player ? player->GetLocalToWorld() * hsVector3(0.f, -1.f, 0.f) : hsVector3());
    str = ST::format("{}: pos({.2f}, {.2f}, {.2f}) ori({.2f}, {.2f}, {.2f})",
        GetPlayerName(), pos.fX, pos.fY, pos.fZ, ori.fX, ori.fY, ori.fZ);
    txt.DrawString(x,y,str,255,255,255,255);


    if (player)
    {
        const plArmatureMod *avMod = plArmatureMod::ConvertNoRef(player->GetModifierByType(plArmatureMod::Index()));
        if (avMod)
        {
            plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(avMod);
            plSceneObject* pObj = pNonConstArm->GetFollowerParticleSystemSO();
            if (pObj)
            {
                y+=yOff;
                y+=yOff;
                hsPoint3 pos = (pObj ? pObj->GetLocalToWorld() * hsPoint3() : hsPoint3());
                hsVector3 ori = (pObj ? pObj->GetLocalToWorld() * hsVector3(0.f, -1.f, 0.f) : hsVector3());
                str = ST::format("{}: pos({.2f}, {.2f}, {.2f}) ori({.2f}, {.2f}, {.2f})",
                    pObj->GetKeyName(), pos.fX, pos.fY, pos.fZ, ori.fX, ori.fY, ori.fZ);
                txt.DrawString(x,y,str,255,255,255,255);
            }
        }
    }


    // Others
    y=startY;
    x=startX;

    std::vector<plNetTransportMember*> members = fTransport.GetMemberListDistSorted();
    for (plNetTransportMember* mbr : members)
    {
        hsAssert(mbr, "ShowLists: nil member?");
        if (mbr->IsServer())
            continue;

        player = (mbr->GetAvatarKey() ? plSceneObject::ConvertNoRef(mbr->GetAvatarKey()->ObjectIsLoaded()) : nullptr);
        pos = (player ? player->GetLocalToWorld() * hsPoint3() : hsPoint3());
        ori = (player ? player->GetLocalToWorld() * hsVector3(0.f, -1.f, 0.f) : hsVector3());

        str = ST::format("{}: pos({.2f}, {.2f}, {.2f}) ori({.2f}, {.2f}, {.2f})",
            mbr->AsString(), pos.fX, pos.fY, pos.fZ, ori.fX, ori.fY, ori.fZ);
        txt.DrawString(x,y,str);
        y+=yOff;

        if (player)
        {
            const plArmatureMod *avMod = plArmatureMod::ConvertNoRef(player->GetModifierByType(plArmatureMod::Index()));
            if (avMod)
            {
                plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(avMod);
                plSceneObject* pObj = pNonConstArm->GetFollowerParticleSystemSO();
                if (pObj)
                {
                    y+=yOff;
                    y+=yOff;
                    hsPoint3 pos = (pObj ? pObj->GetLocalToWorld() * hsPoint3() : hsPoint3());
                    hsVector3 ori = (pObj ? pObj->GetLocalToWorld() * hsVector3(0.f, -1.f, 0.f) : hsVector3());
                    str = ST::format("{}: pos({.2f}, {.2f}, {.2f}) ori({.2f}, {.2f}, {.2f})",
                        pObj->GetKeyName(), pos.fX, pos.fY, pos.fZ, ori.fX, ori.fY, ori.fZ);
                    txt.DrawString(x,y,str,255,255,255,255);
                }
            }
        }

    }

    txt.SetFont(ST_LITERAL("Courier New"), 8);
}
