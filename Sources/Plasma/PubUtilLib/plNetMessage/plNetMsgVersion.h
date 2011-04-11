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
#ifndef plNetMsgVersion_h_inc
#define plNetMsgVersion_h_inc


// Changing the version number(s)? Make an entry in the corresponding log below.
#define PLASMA2_NETMSG_MAJOR_VERSION	12
#define PLASMA2_NETMSG_MINOR_VERSION	6
/*--- Major Version Log ---
	#	Date	Who		Comment
	2  10/05/01	eap		Moved handling of VaultRequestData message from game server to lobby server
	3  10/09/01	ee		Made Uoid changes that impact the net messages
	4  10/11/01	ee		Made Uoid changes that impact the net messages
	5  02/12/02 eap		Modified auth messages.
	6	4/18/02 MT		Changed to using SDL saveStates
	7  04/18/02 eap		Redesigned KI messaging
	8  05/17/02	Colin	Changed format of Uoid
	9  07/01/02	rje		Changed Authentication Scheme
	10	10/16/02	eap	Removed low-level KI. Replaced with plVault.
	11	07/03/03	MT	Optimized plNetMessage headers for size
	12	09/17/03	eap	Removed UInt32 acctID. Added Uuid acctUUID. Changed PlayerUpdate enum values.
*/


/*--- Minor Version Log ---
	#	Date	Who		Comment
	5	9/28/01	MT		Added senderClientNum to GameMessages (mostly for debugging)	
	0  10/05/01	eap		Reset on Major Version change
	1  10/26/01 eap		Upon sending join ack, the game server now sends a set of initial local unique ids to client.
	2  10/31/01 eap		Added (Un)RegisterServer msgs (actually renamed from ServerStarted family msgs). Changed StartProcess msg a little.
	3  11/05/01 eap		Changes to request/receive avatar msgs.
	4  11/21/01 MT		Removed obsolete plNetMessage flags due to client task reorg
	5  02/02/02 eap		Added/Updated KI message classes.
	0  02/12/02 eap		Reset on major version change
	1  03/22/02 eap		Removed acctID from plNetMsgCreatePlayer.
	2	3/31/02	MT		moved uncompressed size from voiceMsg to streamHelper
	0  04/18/02 MT		Reset on major version change.
	0  04/18/02 eap		Reset on major version change
	1	4/29/02	MT		added joinOrder to joinAck
	2  05/02/02 eap		Changed plNetServerSessionInfo stream format.
	3  05/08/02 eap		Changed KI stream formats.
	0  05/17/02 Colin	Reset on major version change
	1  06/03/02 eap		Changed KIOperations enum values. Affects KI messages.
	2  06/06/02 eap		More changes to KI message format.
	3	6/07/02 MT		Enabled compression on SDL msgs
	4	6/19/02	MT		Removed general timeOffset in favor of UnifiedTime in game msg
	4  06/12/02 eap		Yet more changes to KI messaging stuff
	1	7/01/02	MT		Added a member to SDLBCast msgs
	2  07/16/02 eap		Added linking rule info to net msgs related to age linking.
	3  08/15/02 eap		Changed plNetMsgKI format.
	4	8/15/02	MT		Changes related to cloning reorg
	5  08/21/02 eap		Changed plNetMsgKI format.
	6	9/17/02	MT		Added fIsPlayer to plNetMsgLoadClone
	7	9/24/02 rje		Added Packet Size in Client Hello
	8	10/01/02	eap		Changed the way the KI is fetched.
	9	10/02/02	eap		Changed KI storable stream format (made flag-based instead of stream version-based)
	10	10/03/02	eap		Changed KI manifest stream format.
	11	10/04/02	thamer	Changed timeSent to be unified time not double, short-circuit version checking
	0   10/16/02	eap		Reset on major version change.
	1	10/15/02	thamer	minor changes for CCR
	2	11/04/02	eap		Changed plNetMsgVault format.
	3	12/04/02	eap		Moved compression into plNetMsgStreamHelper. Changed plNetMsgVault format.
	4	12/04/02	eap		Changed plNetMsgStreamHelper fUncompressedSize type to UInt32.
	5	12/05/02	eap		Added PlayerName and AvatarShape to CreatePlayer msg.
	6	12/11/02	thamer	Moved PlayerID into the base class
	7	12/18/02	thamer	Changed SDL format
	8	12/17/02	eap		Changed format of vault negotiate manifest msg.
	9	01/14/03	eap		Added CCRLevel to plClientGuid and plNetMsgSetMyActivePlayer. Removed from plNetMsgJoinReq
	10	01/30/03	eap		Changed linking rules and associated net msgs.
	11	02/04/03	eap		Changed vault msg format.
	12	02/05/03	thamer	Added initial age state to joinAck
	13	02/10/03	eap		Changed format of vault FetchNodes msg to support bundling of multiple nodes into one msg.
	14	02/12/03	eap		Changed the way ages are (un)registered. client used to do it. now vault server does it.
	15	02/24/03	eap		Added a byte to plNetMsgLeave to specify the reason for leaving.
	16	02/25/03	thamer	Changed the auth response generation
	17	02/26/03	thamer	again
	18	02/28/03	eap		Support for multiple spawn points for vault age link nodes.
	19	03/14/03	eap		Changed plVaultNode format.
	20	03/14/03	thamer	Added buildType and 'experimental' values to authHello and JoinAck msgs
	21	03/17/03	thamer	Added streamSubType var to StreamHelper
	22	03/21/03	eap		Changed auth error enum values to be negative.
	23	03/24/03	rje		Added Invites to CreatePlayer.
	24	04/11/03	eap		Changed create player error enum values to be negative.
	25	04/14/03	thamer	Changed SharedState R/W format
	26	05/13/03	eap		Changed plNetMsgVault a little bit to allow multiple age vaults to live in one process.
	27	05/16/03	thamer	Bob changed the LoadClone msg format
	28	05/30/03	thamer	Optimized the Uoid read/write format
	29	06/01/03	eap		Changed stream format of plGenericType class
	30	06/01/03	eap		Changed stream format of plGenericType class
	31	06/02/03	thamer	Changed plNetMsgLoadClone format
	32	06/06/03	eap		Reimplemented inter-age messaging. Removed vaultserver from the process.
	33	06/10/03	eap		Changed plVaultNode stream format
	34	06/24/03	eap		Client is now in charge of creating personal age when needed.
	35	06/27/03	eap		Added reply msg to SetActivePlayer
	36	06/25/03	thamer	SDL size optimizations
	37	07/01/03	eap		Vault db version bumped.
	0   07/03/03	thamer	Reset on major version change.
	1	07/11/03	eap		Added fCreateFlags to CreatePlayer msg.
	2	07/16/03	thamer	Added flags to vault player list desc
	3	07/22/03	eap		Changed plNetMsgVault and plNetMsgVaultTask format.
	4	07/23/03	eap		Changed plNetMsgDeletePlayer format.
	5	07/28/03	thamer	Changed StreamHelper format.
	6	08/01/03	eap		Changed the format of some vault operations (RegisterOwnedAge et.al.)
	7	08/01/03	eap		Added disconnect reply msg to vault protocol.
	8	08/06/03	eap		Added some buffer room to the last enum value in plNetMsgTerminated/Leave/ServerMsgUpdatePlayer
	9	08/07/03	eap		Fixed enum values in plNetMsgTerminated/Leave/ServerMsgUpdatePlayer
	10	08/22/03	eap		Game server no longer queries auth server when authenticating a client.
	11	09/04/03	eap		Added camera stack to plSpawnPointInfo.
	12	09/08/03	eap		Added server guid to plNetMsgAuthenticated
	13  09/17/03	bob		Changed format of Read/WriteSafeString, (and the "long" versions)
	0	09/17/03	eap		Reset on major version change.
	1	10/22/03	eap		Changed format of VaultFetchNodes message
	2	10/23/03	eap		Changed format of VaultFetchNodes message again
	3	10/25/03	bob		Changed the format of plLinkEffectsTriggerMsg, which the NetMsgScreener reads.
	4	11/18/03	eap		Changed c/s initial SDL state send transaction.
	5	10/29/03	jeffrey Changed the plDynamicTextMsg to use unicode
	6	12/01/03	eap		Changed plNetMessage flags (kNoGameTimeSent became kTimeSent)
*/


#endif
