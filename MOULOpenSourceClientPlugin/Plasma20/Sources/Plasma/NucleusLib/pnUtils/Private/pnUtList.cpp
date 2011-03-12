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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtList.cpp
*   
***/

#include "../Pch.h"
#pragma  hdrstop


/****************************************************************************
*
*   CBaseList
*
***/

//===========================================================================
void CBaseList::Link (CBaseList * list, byte * afterNode, byte * beforeNode, ELinkType linkType, byte * existingNode) {

    // Verify that the two lists share a common link offset
    ASSERT(m_linkOffset != LINK_OFFSET_UNINIT);
    ASSERT(m_linkOffset == list->m_linkOffset);

    // Verify that the two lists are distinct
    CBaseLink * sourceTerminator = &list->m_terminator;
    ASSERT(sourceTerminator != &m_terminator);

    // Find the first and last nodes to move from the source list
    CBaseLink * afterLink  = afterNode  ? GetLink(afterNode)  : sourceTerminator;
    CBaseLink * beforeLink = beforeNode ? GetLink(beforeNode) : sourceTerminator;
    CBaseLink * firstLink  = afterLink->NextLink();
    CBaseLink * lastLink   = beforeLink->m_prevLink;
    if (lastLink == afterLink)
        return;
    ASSERT(firstLink != beforeLink);

    // Store nodes for later use in linking
    byte * firstNode    = afterLink->m_next;
    byte * lastNextNode = lastLink->m_next;
    ASSERT(firstNode);
    ASSERT(lastNextNode);

    // Find the previous and next nodes in the destination list which will 
    // bound all of the nodes of the source list
    CBaseLink * existingLink = existingNode ? GetLink(existingNode) : &m_terminator;
    CBaseLink * prevLink, * nextLink;
    switch (linkType) {
    
        case kListLinkAfter:
            prevLink = existingLink;
            nextLink = existingLink->NextLink();
        break;

        case kListLinkBefore:
            prevLink = existingLink->m_prevLink;
            nextLink = existingLink;
        break;

        DEFAULT_FATAL(linkType);

    }

    // Update the first and last nodes of the moved range to point to the 
    // previous and next nodes in the destination list
    firstLink->m_prevLink = prevLink;
    lastLink->m_next      = prevLink->m_next;

    // Update the previous and next nodes in the destination list to point to
    // the first and last nodes of the source range
    nextLink->m_prevLink = lastLink;
    prevLink->m_next     = firstNode;

    // Update the before and after links from the source list to point to
    // each other
    afterLink->m_next      = lastNextNode;
    beforeLink->m_prevLink = afterLink;

}

//===========================================================================
void CBaseList::UnlinkAll () {
    for (CBaseLink * link = m_terminator.m_prevLink, * prev; link != &m_terminator; link = prev) {
        prev = link->m_prevLink;
        link->InitializeLinksWithOffset(m_linkOffset);
    }
    m_terminator.InitializeLinksWithOffset(m_linkOffset);
}

