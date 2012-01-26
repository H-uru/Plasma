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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNetCli.h
*
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETCLI_PNNETCLI_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETCLI_PNNETCLI_H


/*****************************************************************************
*
*

How to create a message sender/receiver:

1. Define message ids

    enum {
        kMsgMoveObject,
        kMsgPlayerJoin,
        kMsgPing,
        kMsgPingReply,
        // etc.
    };


2. Define packed message structures

    #include <PshPack1.h>
    struct MoveObject {
        uint32_t   messageId;
        uint32_t   objectId;
        float   newPos[3];
    };
    struct PlayerJoin {
        uint32_t   messageId;
        uint32_t   playerId;
        wchar_t   name[kPlayerNameMaxLength];
        uint8_t    data[kPlayerDataMaxLength];
        uint32_t   vaultDataLen;
        uint8_t    vaultData[1];   // vaultData[vaultDataLen], actually
        // no more fields after variable-length data
    };
    struct Ping {
        uint32_t   messageId;
        uint32_t   pingTimeMs;
    };
    struct PingReply {
        uint32_t   messageId;
        uint32_t   pingTimeMs;
    }
    #include <PopPack.h>


3. Define message fields

    static const NetMsgField kFieldObjectId     = NET_MSG_FIELD_DWORD();
    static const NetMsgField kFieldPlayerId     = NET_MSG_FIELD_DWORD();
    static const NetMsgField kFieldObjectPos    = NET_MSG_FIELD_FLOAT_ARRAY(3);
    static const NetMsgField kFieldPlayerName   = NET_MSG_FIELD_STRING(kPlayerNameMaxLength);
    static const NetMsgField kFieldPlayerData   = NET_MSG_FIELD_PTR(kPlayerDataMaxLength);
    static const NetMsgField kFieldVaultDataLen = NET_MSG_FIELD_VAR_COUNT(kVaultDataMaxLength);
    static const NetMsgField kFieldVaultData    = NET_MSG_FIELD_VAR_PTR();
    static const NetMsgField kFieldPingTimeMs   = NET_MSG_FIELD_DWORD();


4. Build field and message description blocks

    // MoveObject
    static const NetMsgField s_moveObjectFields[] = {
        kFieldObjectId,
        kFieldObjectPos,
    };
    static const NetMsg s_msgMoveObject = NET_MSG(kMsgMoveObject, s_msgMoveObjectFields);

    // PlayerJoin
    static const NetMsgField s_playerJoinFields[] = {
        kFieldPlayerId,
        kFieldPlayerName,
        kFieldPlayerData,
        kFieldVaultDataLen,
        kFieldVaultData,
    };
    static const NetMsg s_playerJoin = NET_MSG(kMsgPlayerJoin, s_msgPlayerJoin);



5. Register message description blocks

// for server:
    static const NetMsgInitSend s_srvSend[] = {
        { s_moveObject  },
        { s_playerJoin  },
        { s_ping        },
    };

    static const NetMsgInitRecv s_srvRecv[] = {
        { s_pingReply,  RecvMsgPingReply    },
    };

    s_srvConn = NetMsgProtocolRegister(
        kNetProtocolCliToGame,
        true,
        s_srvSend, arrsize(s_srvSend),
        s_srvRecv, arrsize(s_srvRecv)
    );


// for client:
    static const NetMsgInitSend s_cliSend[] = {
        { s_pingReply },
    };

    static const NetMsgInitRecv s_cliRecv[] = {
        { s_moveObject, RecvMsgMoveObject   },
        { s_playerJoin, RecvMsgPlayerJoin   },
        { s_ping,       RecvMsgPing         },
    };

    s_cliConn = NetMsgProtocolRegister(
        kNetProtocolCliToGame,
        false,
        s_cliSend, arrsize(s_cliSend),
        s_cliRecv, arrsize(s_cliRecv)
    );


5. Send messages

    static void SendMoveObject (NetCli client, const Object * obj) {
        const uintptr_t msgMoveObject[] = {
            kMsgMoveObject,
            obj->id,
            3, (uintptr_t) &obj->pos,
        };
        NetCliSend(client, msgMoveObject, arrsize(msgMoveObject));
    }

    static void SendPlayerJoin (NetCli client, const Player * player) {
        const uintptr_t msgPlayerJoin[] = {
            kMsgPlayerJoin,
            player->name,
            player->data,
            player->vault->Count(),
            player->vault->Ptr()
        };
        NetCliSend(client, msgPlayerJoin, arrsize(msgPlayerJoin));
    };

    static void SendPing (NetCli player) {
        const uintptr_t msgPing[] = {
            kMsgPing,
            TimeGetMs(),
        };
        NetCliSend(player, msgPing, arrsize(msgPing));
    }


6. Receive messages

    bool RecvMsgPing (const uint8_t msg[], NetCli * from, void * param) {
        Player * player = (Player *) param;
        const Ping * ping = (const Ping *) msg;
        const uintptr_t msgPingReply[] = {
            kMsgPingReply,
            ping->pingTimeMs,
        };
        MsgConnSend(from, msgPingReply, arrsize(msgPingReply));
    }

*
***/


/*****************************************************************************
*
*   Constants
*
***/



/*****************************************************************************
*
*   Field definition types
*
***/

enum ENetMsgFieldType {
    // Compressable fields
    kNetMsgFieldInteger,
    kNetMsgFieldReal,
    kNetMsgFieldString,             // variable length unicode string
    kNetMsgFieldData,               // data with length <= sizeof(uint32_t)
    kNetMsgFieldPtr,                // pointer to fixed length data
    kNetMsgFieldVarPtr,             // pointer to variable length data

    // Non-compressible fields (images, sounds, encrypted data, etc)
    kNetMsgFieldRawData,            // data with length <= sizeof(uint32_t)
    kNetMsgFieldRawPtr,             // pointer to fixed length data
    kNetMsgFieldRawVarPtr,          // pointer to variable length data

    kNetMsgFieldVarCount,           // count for kNetMsgFieldVarPtr and kNetMsgFieldRawVarPtr

    kNumNetMsgFieldTypes
};


struct NetMsgField {
    ENetMsgFieldType    type;   // element type
    unsigned            count;  // max number of elements
    unsigned            size;   // element size
};

struct NetMsg {
    const char *        name;
    unsigned            messageId;
    const NetMsgField * fields;
    unsigned            count;
};

// Opaque types
struct NetCli;
struct NetCliQueue;


/*****************************************************************************
*
*   Msg and field definition macros
*
***/

#define NET_MSG(msgId, msgFields)               { #msgId, msgId, msgFields, arrsize(msgFields) }

#define NET_MSG_FIELD(type, count, size)        { type, count, size }

#define NET_MSG_FIELD_BYTE()                    NET_MSG_FIELD(kNetMsgFieldInteger, 0, sizeof(uint8_t))
#define NET_MSG_FIELD_WORD()                    NET_MSG_FIELD(kNetMsgFieldInteger, 0, sizeof(uint16_t))
#define NET_MSG_FIELD_DWORD()                   NET_MSG_FIELD(kNetMsgFieldInteger, 0, sizeof(uint32_t))
#define NET_MSG_FIELD_QWORD()                   NET_MSG_FIELD(kNetMsgFieldInteger, 0, sizeof(uint64_t))
#define NET_MSG_FIELD_FLOAT()                   NET_MSG_FIELD(kNetMsgFieldReal, 0, sizeof(float))
#define NET_MSG_FIELD_DOUBLE()                  NET_MSG_FIELD(kNetMsgFieldReal, 0, sizeof(double))

#define NET_MSG_FIELD_BYTE_ARRAY(maxCount)      NET_MSG_FIELD(kNetMsgFieldInteger, maxCount, sizeof(uint8_t))
#define NET_MSG_FIELD_WORD_ARRAY(maxCount)      NET_MSG_FIELD(kNetMsgFieldInteger, maxCount, sizeof(uint16_t))
#define NET_MSG_FIELD_DWORD_ARRAY(maxCount)     NET_MSG_FIELD(kNetMsgFieldInteger, maxCount, sizeof(uint32_t))
#define NET_MSG_FIELD_QWORD_ARRAY(maxCount)     NET_MSG_FIELD(kNetMsgFieldInteger, maxCount, sizeof(uint64_t))
#define NET_MSG_FIELD_FLOAT_ARRAY(maxCount)     NET_MSG_FIELD(kNetMsgFieldReal, maxCount, sizeof(float))
#define NET_MSG_FIELD_DOUBLE_ARRAY(maxCount)    NET_MSG_FIELD(kNetMsgFieldReal, maxCount, sizeof(double))

#define NET_MSG_FIELD_STRING(maxLength)         NET_MSG_FIELD(kNetMsgFieldString, maxLength, sizeof(wchar_t))

#define NET_MSG_FIELD_DATA(maxBytes)            NET_MSG_FIELD(kNetMsgFieldData,     maxBytes, 1)
#define NET_MSG_FIELD_PTR(maxBytes)             NET_MSG_FIELD(kNetMsgFieldPtr,      maxBytes, 1)
#define NET_MSG_FIELD_RAW_DATA(maxBytes)        NET_MSG_FIELD(kNetMsgFieldRawData,  maxBytes, 1)
#define NET_MSG_FIELD_RAW_PTR(maxBytes)         NET_MSG_FIELD(kNetMsgFieldRawPtr,   maxBytes, 1)

#define NET_MSG_FIELD_VAR_PTR()                 NET_MSG_FIELD(kNetMsgFieldVarPtr,    0, 0)
#define NET_MSG_FIELD_RAW_VAR_PTR()             NET_MSG_FIELD(kNetMsgFieldRawVarPtr, 0, 0)

#define NET_MSG_FIELD_VAR_COUNT(elemSize, maxCount) NET_MSG_FIELD(kNetMsgFieldVarCount, maxCount, elemSize)


/*****************************************************************************
*
*   Message channels
*
***/

struct NetMsgInitSend {
    NetMsg  msg;
};
struct NetMsgInitRecv {
    NetMsg  msg;
    bool (* recv)(const uint8_t msg[], unsigned bytes, void * param);
};

void NetMsgProtocolRegister (
    unsigned                protocol,       // from pnNetBase/pnNbProtocol.h
    bool                    server,
    const NetMsgInitSend    sendMsgs[],     // messages this program can send
    unsigned                sendMsgCount,
    const NetMsgInitRecv    recvMsgs[],     // messages this program can receive
    unsigned                recvMsgCount,
    // Diffie-Hellman keys
    unsigned                dh_g,
    const BigNum &          dh_xa,          // client: dh_x     server: dh_a
    const BigNum &          dh_n
);

void NetMsgProtocolDestroy (
    unsigned                protocol,
    bool                    server
);


/*****************************************************************************
*
*   NetCliQueue
*
***/

NetCliQueue * NetCliQueueCreate (
    unsigned        flushTimeMs
);

void NetCliQueueDestroy (
    NetCliQueue *   queue
);

void NetCliQueueFlush (
    NetCliQueue *   queue
);

float NetCliQueueQueryFlush (
    NetCliQueue *   queue
);


/*****************************************************************************
*
*   NetCli
*
***/

typedef bool (* FNetCliEncrypt) (
    ENetError       error,
    void *          encryptParam
);

NetCli * NetCliConnectAccept (
    AsyncSocket         sock,
    unsigned            protocol,
    bool                unbuffered,
    FNetCliEncrypt      encryptFcn,
    unsigned            seedBytes,      // optional
    const uint8_t          seedData[],     // optional
    void *              encryptParam    // optional
);

#ifdef SERVER
NetCli * NetCliListenAccept (
    AsyncSocket         sock,
    unsigned            protocol,
    bool                unbuffered,
    FNetCliEncrypt      encryptFcn,
    unsigned            seedBytes,      // optional
    const uint8_t          seedData[],     // optional
    void *              encryptParam    // optional
);
#endif

#ifdef SERVER
void NetCliListenReject (
    AsyncSocket     sock,
    ENetError       error
);
#endif

void NetCliClearSocket (
    NetCli *        cli
);

void NetCliSetQueue (
    NetCli *        cli,
    NetCliQueue *   queue
);

void NetCliDisconnect (
    NetCli *        cli,
    bool            hardClose
);

void NetCliDelete (
    NetCli *        cli, 
    bool            deleteSocket
);

void NetCliFlush (
    NetCli *        cli
);

void NetCliSend (
    NetCli *            cli,
    const uintptr_t  msg[], 
    unsigned            count
);

bool NetCliDispatch (
    NetCli *        cli,
    const uint8_t      buffer[],
    unsigned        bytes,
    void *          param
);


#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETCLI_PNNETCLI_H
