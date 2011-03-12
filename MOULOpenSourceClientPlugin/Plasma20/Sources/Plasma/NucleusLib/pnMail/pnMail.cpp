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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnMail/pnMail.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

enum EMailStep {
    kMailStepConnect,       // wait for 220, then send "helo" command
    kMailStepAuth,          // wait for 250, then send "AUTH PLAIN <base64data>"
    kMailStepSender,        // wait for 235/250, then send "mail from:" command
    kMailStepRecipient,     // wait for 250, then send "rcpt to:" command
    kMailStepStartLetter,   // wait for 250, then send "data" command
    kMailStepLetter,        // wait for 354, then send letter
    kMailStepQuit,          // wait for 250, then send "quit" command
    kMailStepDisconnect,    // wait for 221, then disconnect
};


struct MailTransaction {
    LINK(MailTransaction)       link;
    AsyncSocket                 sock;
    const char **               stepTable;
    EMailStep                   step;
    unsigned                    subStep;
    EMailError                  error;
    FMailResult                 callback;
    void *                      param;

    char *                      smtp;
    char *                      sender;
    char *                      recipient;
    char *                      replyTo;
    char *                      subject;
    char *                      bodyEncoding;
    char *                      body;
    char *                      auth;
    char                        buffer[1];
};

static bool MailNotifyProc (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
);

static void __cdecl Send (
    AsyncSocket sock,
    const char  str[],
    ...
);


/*****************************************************************************
*
*   Private data
*
***/

static bool                             s_shutdown;
static CCritSect                        s_critsect;
static LISTDECL(MailTransaction, link)  s_mail;


//===========================================================================
// Authenticated email sequence
//      -> "220 catherine.cyan.com ESMTP\r\n"
//      <- "HELO cyan.com\r\n"
//      -> "250 catherine.cyan.com\r\n"
//      <- "AUTH PLAIN XXXXXXXXX\r\n"
//      -> "235 go ahead\r\n"
//      <- "MAIL FROM:<UruClient@cyanworlds.com>\r\n"
//      -> "250 ok mail from accepted\r\n"
//      <- "RCPT TO:<UruCrashReport@cyanworlds.com>\r\n"
//      -> "250 ok recipient accepted\r\n"
//      <- "DATA\r\n"
//      -> "354 ok, go ahead:\r\n"
//      ->  <message> + "\r\n.\r\n"
//      -> "250 message accepted for delivery\r\n"
//      <- "QUIT\r\n"
//      -> "221 catherine.cyan.com\r\n"
//===========================================================================
static const char * s_waitStrAuth[] = {
    "220 ", // kMailStepConnect
    "250 ", // kMailStepAuth
    "235 ", // kMailStepSender
    "250 ", // kMailStepRecipient
    "250 ", // kMailStepStartLetter
    "354 ", // kMailStepLetter
    "250 ", // kMailStepQuit
    "221 ", // kMailStepDisconnect
};

//===========================================================================
// Unauthenticated email seqeunce
//      -> "220 catherine.cyan.com ESMTP\r\n"
//      <- "HELO cyan.com\r\n"
//      -> "250 catherine.cyan.com\r\n"
//      <- "MAIL FROM:<UruClient@cyanworlds.com>\r\n"
//      -> "250 ok mail from accepted\r\n"
//      <- "RCPT TO:<UruCrashReport@cyanworlds.com>\r\n"
//      -> "250 ok recipient accepted\r\n"
//      <- "DATA\r\n"
//      -> "354 ok, go ahead:\r\n"
//      ->  <message> + "\r\n.\r\n"
//      -> "250 message accepted for delivery\r\n"
//      <- "QUIT\r\n"
//      -> "221 catherine.cyan.com\r\n"
//===========================================================================
static const char * s_waitStrNoAuth[] = {
    "220 ", // kMailStepConnect
    nil,    // kMailStepAuth
    "250 ", // kMailStepSender
    "250 ", // kMailStepRecipient
    "250 ", // kMailStepStartLetter
    "354 ", // kMailStepLetter
    "250 ", // kMailStepQuit
    "221 ", // kMailStepDisconnect
};



/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static bool AdvanceStep (
    MailTransaction *   transaction,
    AsyncSocket         sock
) {
    switch (transaction->step) {
        case kMailStepConnect: {
            const char * host      = transaction->sender;
            const char * separator = StrChr(host, '@');
            if (separator)
                host = separator + 1;
            Send(sock, "helo ", host, "\r\n", nil);
            transaction->step = kMailStepSender;

            if (transaction->auth[0])
                transaction->step = kMailStepAuth;
        }
        break;

        case kMailStepAuth:
            Send(sock, "AUTH PLAIN ", transaction->auth, "\r\n", nil);
            transaction->step = kMailStepSender;
        break;

        case kMailStepSender:
            Send(sock, "mail from:<", transaction->sender, ">\r\n", nil);
            transaction->step    = kMailStepRecipient;
        break;

        case kMailStepRecipient: {
            const char * start = transaction->recipient + transaction->subStep;
            while (*start == ' ')
                ++start;
            const char * term = StrChr(start, ';');
            if (term) {
                char * buffer = ALLOCA(char, term + 1 - start);
                StrCopy(buffer, start, term + 1 - start);
                Send(sock, "rcpt to:<", buffer, ">\r\n", nil);
                transaction->subStep = term + 1 - transaction->recipient;
            }
            else {
                Send(sock, "rcpt to:<", start, ">\r\n", nil);
                transaction->step       = kMailStepStartLetter;
                transaction->subStep    = 0;
            }
        }
        break;

        case kMailStepStartLetter:
            Send(sock, "data\r\n", nil);
            transaction->step = kMailStepLetter;
        break;

        case kMailStepLetter:
            if (transaction->replyTo[0]) {
                Send(
                    sock,
                    "Reply-to: ",
                    transaction->replyTo,
                    "\r\n",
                    nil
                );
            }
            if (transaction->bodyEncoding) {
                Send(
                    sock, 
                    "From: ",
                    transaction->sender,
                    "\r\nTo: ",
                    transaction->recipient,
                    "\r\nSubject: ",
                    transaction->subject,
                    "\r\nContent-Type: text/plain; charset=",
                    transaction->bodyEncoding,
                    "\r\n\r\n",
                    transaction->body,
                    "\r\n.\r\n",
                    nil
                );
            }
            else {
                Send(
                    sock, 
                    "From: ",
                    transaction->sender,
                    "\r\nTo: ",
                    transaction->recipient,
                    "\r\nSubject: ",
                    transaction->subject,
                    "\r\n\r\n",
                    transaction->body,
                    "\r\n.\r\n",
                    nil
                );
            }
            transaction->step = kMailStepQuit;
        break;

        case kMailStepQuit:
            Send(sock, "quit\r\n", nil);
            transaction->step = kMailStepDisconnect;
        break;

        case kMailStepDisconnect:
        return false;

        DEFAULT_FATAL(transaction->step);
    }

    return true;
}

//===========================================================================
static bool NotifyRead (
    AsyncSocket             sock,
    AsyncNotifySocketRead * read,
    MailTransaction *       transaction
) {

    // Parse available lines looking for an acknowledgement of last command
    const char * compareStr = transaction->stepTable[transaction->step];
    unsigned     compareLen = StrLen(compareStr);
    unsigned     offset     = 0;
    for (;;) {
        const char * source = (const char *)&read->buffer[offset];

        // Search for an end-of-line marker
        const char * eol = StrChr(source, '\n', read->bytes - offset);
        if (!eol)
            break;
        offset += (eol + 1 - source);

        // Search for an acknowledgement
        if (!StrCmp(source, compareStr, compareLen)) {
            read->bytesProcessed = offset;
            return AdvanceStep(transaction, sock);
        }

        // An error was received; log it and bail
        LogMsg(
            kLogError,
            "Mail: step %u error '%.*s'",
            transaction->step,
            eol - source - 1,
            source
        );
        transaction->error = kMailErrServerError;
        return false;
    }

    // We did not find an acknowledgement, so skip past fully-received lines
    // and wait for more data to arrive
    read->bytesProcessed = offset;
    return true;

}

//===========================================================================
static void DestroyTransaction (MailTransaction * transaction) {
   
    // Remove transaction from list so that it can
    // be safely deleted outside the critical section
    s_critsect.Enter();
    {
        s_mail.Unlink(transaction);
    }
    s_critsect.Leave();

    // Perform callback if requested
    if (transaction->callback) {
        transaction->callback(
            transaction->param,
            transaction->error
        );
    }

    DEL(transaction);
}

//===========================================================================
static void MailLookupProc (
    void *              param,
    const wchar *       ,
    unsigned            addrCount,
    const NetAddress    addrs[]
) {

    // If no address was found, cancel the transaction
    MailTransaction * transaction = (MailTransaction *) param;
    if (!addrCount) {
        LogMsg(kLogError,"Mail: failed to resolve %s", transaction->smtp);
        transaction->error = kMailErrDnsFailed;
        DestroyTransaction(transaction);
        return;
    }

    // Initiate a connection
    AsyncCancelId cancelId;
    AsyncSocketConnect(
        &cancelId,
        addrs[0],
        MailNotifyProc,
        transaction
    );

}

//===========================================================================
static bool MailNotifyProc (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    switch (code) {
        case kNotifySocketConnectFailed: {
            MailTransaction * transaction = (MailTransaction *)notify->param;
            LogMsg(kLogError, "Mail: unable to connect to %s", transaction->smtp);
            transaction->error = kMailErrConnectFailed;
            DestroyTransaction(transaction);
        }
        break;

        case kNotifySocketConnectSuccess: {
            MailTransaction * transaction   = (MailTransaction *) notify->param;
            *userState                      = notify->param;
            transaction->sock               = sock;
        }
        break;

        case kNotifySocketRead: {
            MailTransaction * transaction = (MailTransaction *) *userState;
            return NotifyRead(sock, (AsyncNotifySocketRead *) notify, transaction);
        }
        break;

        case kNotifySocketDisconnect: {
            MailTransaction * transaction = (MailTransaction *) *userState;
            if ((transaction->step != kMailStepDisconnect) && !transaction->error) {
                transaction->error = kMailErrDisconnected;
                LogMsg(kLogError, "Mail: unexpected disconnection from %s", transaction->smtp);
            }
            DestroyTransaction(transaction);
            AsyncSocketDelete(sock);
        }
        break;
    }

    return !s_shutdown;
}

//===========================================================================
static void __cdecl Send (
    AsyncSocket sock,
    const char  str[],
    ...
) {
    // Count bytes
    unsigned bytes = 1;
    {
        va_list argList;
        va_start(argList, str);
        for (const char * source = str; source; source = va_arg(argList, const char *))
            bytes += StrLen(source);
        va_end(argList);
    }

    // Allocate string buffer
    char * packed;
    const unsigned kStackBufSize = 8 * 1024;
    if (bytes > kStackBufSize)
        packed = (char *) ALLOC(bytes);
    else
        packed = (char *) _alloca(bytes);

    // Pack the string
    {
        va_list argList;
        va_start(argList, str);
        char * dest = packed;
        for (const char * source = str; source; source = va_arg(argList, const char *))
            dest += StrCopyLen(dest, source, packed + bytes - dest);
        va_end(argList);
    }

    // Send the string
    AsyncSocketSend(sock, packed, bytes - 1);

    // Free the string
    if (bytes > kStackBufSize)
        FREE(packed);
}

//===========================================================================
static void IMail (
    const char *    stepTable[],
    const char      smtp[],
    const char      sender[],
    const char      recipient[],
    const char      replyTo[],
    const char      subject[],
    const char      body[],
    const char      auth[],
    EMailEncoding   bodyEncoding,
    FMailResult     callback,
    void *          param
) {
    static const char * utf8  = "\"utf-8\"";
    const char * encodingStr;

    if (bodyEncoding == kMailEncodeUtf8)
        encodingStr = utf8;
    else
        encodingStr = "";
    
    // Calculate string lengths
    unsigned lenSmtp         = StrLen(smtp);
    unsigned lenSender       = StrLen(sender);
    unsigned lenRecipient    = StrLen(recipient);
    unsigned lenReplyTo      = StrLen(replyTo);
    unsigned lenSubject      = StrLen(subject);
    unsigned lenBodyEncoding = StrLen(encodingStr);
    unsigned lenBody         = StrLen(body);
    unsigned lenAuth         = StrLen(auth);

    // Calculate the required buffer size for all strings
    unsigned bytes = (
        lenSmtp         + 1 +
        lenSender       + 1 +
        lenRecipient    + 1 +
        lenReplyTo      + 1 +
        lenSubject      + 1 +
        lenBody         + 1 +
        lenAuth         + 1
    ) * sizeof(char);

    if (lenBodyEncoding)
        bytes += (lenBodyEncoding + 1) * sizeof(char);

    // Create a transaction record
    MailTransaction * transaction = new(
        ALLOC(offsetof(MailTransaction, buffer) + bytes)
    ) MailTransaction;
    transaction->stepTable  = stepTable;
    transaction->sock       = nil;
    transaction->step       = kMailStepConnect;
    transaction->subStep    = 0;
    transaction->error      = kMailSuccess;
    transaction->callback   = callback;
    transaction->param      = param;

    unsigned offset = 0;
    transaction->smtp     = transaction->buffer + offset;
    offset += StrCopyLen(transaction->smtp,      smtp,       lenSmtp       + 1) + 1;
    transaction->sender    = transaction->buffer + offset;
    offset += StrCopyLen(transaction->sender,    sender,     lenSender     + 1) + 1;
    transaction->recipient = transaction->buffer + offset;
    offset += StrCopyLen(transaction->recipient, recipient,  lenRecipient  + 1) + 1;
    transaction->replyTo   = transaction->buffer + offset;
    offset += StrCopyLen(transaction->replyTo,   replyTo,    lenReplyTo    + 1) + 1;
    transaction->subject   = transaction->buffer + offset;
    offset += StrCopyLen(transaction->subject,   subject,    lenSubject    + 1) + 1;
    if (lenBodyEncoding) {
        transaction->bodyEncoding = transaction->buffer + offset;
        offset += StrCopyLen(transaction->bodyEncoding, encodingStr, lenBodyEncoding + 1) + 1;
    }
    else {
        transaction->bodyEncoding = nil;
    }
    transaction->body      = transaction->buffer + offset;
    offset += StrCopyLen(transaction->body,      body,       lenBody       + 1) + 1;
    transaction->auth      = transaction->buffer + offset;
    offset += StrCopyLen(transaction->auth,      auth,       lenAuth       + 1) + 1;
    ASSERT(offset == bytes);

    // Start the transaction with a dns lookup
    const unsigned kSmtpPort = 25;
    wchar smtpName[256];
    StrToUnicode(smtpName, smtp, arrsize(smtpName));

    // Add transaction to global list
    bool shutdown;
    s_critsect.Enter();
    {
		shutdown = s_shutdown;
        s_mail.Link(transaction);
    }
    s_critsect.Leave();

	if (shutdown) {
		DestroyTransaction(transaction);
	}
	else {
		NetAddress addr;
		if (NetAddressFromString(&addr, smtpName, kSmtpPort)) {
			AsyncCancelId cancelId;
			AsyncSocketConnect(
				&cancelId,
				addr,
				MailNotifyProc,
				transaction
			);
		}
		else {
			AsyncCancelId cancelId;
			AsyncAddressLookupName(
				&cancelId,
				MailLookupProc,
				smtpName,
				kSmtpPort,
				transaction
			);
		}
	}
}

/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void MailEncodePassword (
    const char      username[],
    const char      password[],
    ARRAY(char) *   emailAuth
) {
    // base64_encode("\0#{user}\0#{secret}")
    emailAuth->Reserve(512);
    emailAuth->Push(0);
    emailAuth->Add(username, StrBytes(username));
    emailAuth->Add(password, StrBytes(password));
    unsigned srcChars = emailAuth->Bytes();

    // Allocate space for encoded data
    unsigned dstChars   = Base64EncodeSize(srcChars);
    char * dstData      = emailAuth->New(dstChars);

    // Encode data and move it back to the front of the array
    dstChars = Base64Encode(
        srcChars,
        (const byte *) emailAuth->Ptr(),
        dstChars,
        dstData
    );
    emailAuth->Move(
        0,
        srcChars,
        dstChars
    );
    emailAuth->SetCountFewer(dstChars);
}

//===========================================================================
void Mail (
    const char      smtp[],
    const char      sender[],
    const char      recipient[], // multiple recipients separated by semicolons
    const char      subject[],
    const char      body[],
    const char      username[],
    const char      password[],
    const char      replyTo[],
    EMailEncoding   bodyEncoding,
    FMailResult     callback,
    void *          param
) {
	s_shutdown = false;

    // Get email authorization
    const char * auth;
    const char ** stepTable;
    ARRAY(char) authBuffer;
    if (!password || !*password) {
        // No password is specified, use non-authenticated email
        auth        = "";
        stepTable   = s_waitStrNoAuth;
    }
    else if (!username || !*username) {
        // No username specified, user is providing the base64 encoded secret
        auth        = password;
        stepTable   = s_waitStrAuth;
    }
    else {
        MailEncodePassword(username, password, &authBuffer);
        auth        = authBuffer.Ptr();
        stepTable   = s_waitStrAuth;
    }

    IMail(
        stepTable,
        smtp,
        sender,
        recipient,
        replyTo ? replyTo : "",
        subject,
        body,
        auth,
        bodyEncoding,
        callback,
        param
    );
}

//===========================================================================
void MailStop () {
    s_critsect.Enter();
    {
        s_shutdown = true;
        MailTransaction * transaction = s_mail.Head();
        for (; transaction; transaction = s_mail.Next(transaction)) {
            if (transaction->sock)
                AsyncSocketDisconnect(transaction->sock, true);
            transaction->error = kMailErrClientCanceled;
        }
    }
    s_critsect.Leave();

    AsyncAddressLookupCancel(
        MailLookupProc,
        0   // cancel all
    );
    AsyncSocketConnectCancel(
        MailNotifyProc,
        0   // cancel all
    );
}

//===========================================================================
bool MailQueued () {
    bool queued;
    s_critsect.Enter();
    queued = s_mail.Head() != nil;
    s_critsect.Leave();
    return queued;
}

//============================================================================
const wchar * MailErrorToString (EMailError error) {

	switch (error) {
		case kMailSuccess:				return L"kMailSuccess";
		case kMailErrDnsFailed:			return L"kMailErrDnsFailed";
		case kMailErrConnectFailed:		return L"kMailErrConnectFailed";
		case kMailErrDisconnected:		return L"kMailErrDisconnected";
		case kMailErrClientCanceled:	return L"kMailErrClientCanceled";
		case kMailErrServerError:		return L"kMailErrServerError";
		DEFAULT_FATAL(error);
	}
}
