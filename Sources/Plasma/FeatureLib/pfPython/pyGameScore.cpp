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

#include "pyGameScore.h"

#include <string_theory/string>

#include "plVault/plVault.h"

#include "pfGameScoreMgr/pfGameScoreMgr.h"

#include "pyKey.h"

pyGameScore::pyGameScore() 
    : fScore()
{ }

pyGameScore::pyGameScore(pfGameScore * score) 
    : fScore(score)
{
    hsRefCnt_SafeRef(score);
}

pyGameScore::~pyGameScore()
{
    hsRefCnt_SafeUnRef(fScore);
}

uint32_t pyGameScore::GetOwnerID() const
{
    if (fScore)
        return fScore->GetOwner();
    return 0;
}

int32_t pyGameScore::GetGameType() const
{
    if (fScore)
        return fScore->GetGameType();
    return 0;
}

int32_t pyGameScore::GetPoints() const
{
    if (fScore)
        return fScore->GetPoints();
    return 0;
}

ST::string pyGameScore::GetGameName() const
{
    if (fScore)
        return fScore->GetGameName();
    return ST::string();
}

void pyGameScore::AddPoints(int32_t numPoints, pyKey& rcvr)
{
    if (fScore)
        fScore->AddPoints(numPoints, rcvr.getKey());
}

void pyGameScore::Delete()
{
    if (fScore)
    {
        fScore->Delete();
        fScore = nullptr;
    }
}

void pyGameScore::TransferPoints(pyGameScore* dest, pyKey& rcvr)
{
    if (fScore && dest->fScore)
        fScore->TransferPoints(dest->fScore, rcvr.getKey());
}

void pyGameScore::TransferPoints(pyGameScore* dest, int32_t numPoints, pyKey& rcvr)
{
    if (fScore && dest->fScore)
        fScore->TransferPoints(dest->fScore, numPoints, rcvr.getKey());
}

void pyGameScore::SetPoints(int32_t numPoints, pyKey& rcvr)
{
    if (fScore)
        fScore->SetPoints(numPoints, rcvr.getKey());
}

void pyGameScore::CreateAgeScore(const ST::string& name, uint32_t type, int32_t points, pyKey& rcvr)
{
    if (hsRef<RelVaultNode> ageInfo = VaultGetAgeInfoNode())
    {
        uint32_t ownerId = ageInfo->GetNodeId();
        pfGameScore::Create(ownerId, name, type, points, rcvr.getKey());
    } else
        hsAssert(false, "Age has no vault... Need to rewrite score python script?");
}

void pyGameScore::CreateGlobalScore(const ST::string& name, uint32_t type, int32_t points, pyKey& rcvr)
{
    pfGameScore::Create(0, name, type, points, rcvr.getKey());
}

void pyGameScore::CreatePlayerScore(const ST::string& name, uint32_t type, int32_t points, pyKey& rcvr)
{
    if (hsRef<RelVaultNode> node = VaultGetPlayerInfoNode())
    {
        uint32_t ownerId = node->GetNodeId();
        pfGameScore::Create(ownerId, name, type, points, rcvr.getKey());
    } else
        hsAssert(false, "No PlayerInfo node... Need to rewrite python script?");
}

void pyGameScore::CreateScore(uint32_t ownerId, const ST::string& name, uint32_t type, int32_t points, pyKey& rcvr)
{
    pfGameScore::Create(ownerId, name, type, points, rcvr.getKey());
}

void pyGameScore::FindAgeScores(const ST::string& name, pyKey& rcvr)
{
    if (hsRef<RelVaultNode> ageInfo = VaultGetAgeInfoNode())
    {
        uint32_t ownerId = ageInfo->GetNodeId();
        pfGameScore::Find(ownerId, name, rcvr.getKey());
    } else
        hsAssert(false, "Age has no vault... Need to rewrite score python script?");
}

void pyGameScore::FindGlobalScores(const ST::string& name, pyKey& rcvr)
{
    pfGameScore::Find(0, name, rcvr.getKey());
}

void pyGameScore::FindPlayerScores(const ST::string& name, pyKey& rcvr)
{
    if (hsRef<RelVaultNode> node = VaultGetPlayerInfoNode())
    {
        uint32_t ownerId = node->GetNodeId();
        pfGameScore::Find(ownerId, name, rcvr.getKey());
    }
    else
        hsAssert(false, "No PlayerInfo node.. Need to rewrite python script?");
}

void pyGameScore::FindScores(uint32_t ownerId, const ST::string& name, pyKey& rcvr)
{
    pfGameScore::Find(ownerId, name, rcvr.getKey());
}

void pyGameScore::FindAgeHighScores(const ST::string& name, uint32_t maxScores, pyKey& rcvr)
{
    if (hsRef<RelVaultNode> ageInfo = VaultGetAgeInfoNode()) {
        pfGameScore::FindHighScores(ageInfo->GetNodeId(), maxScores, name, rcvr.getKey());
    }
    else
        hsAssert(false, "Age has no vault... Need to rewrite score python script?");
}

void pyGameScore::FindGlobalHighScores(const ST::string& name, uint32_t maxScores, pyKey& rcvr)
{
    pfGameScore::FindHighScores(0, maxScores, name, rcvr.getKey());
}
