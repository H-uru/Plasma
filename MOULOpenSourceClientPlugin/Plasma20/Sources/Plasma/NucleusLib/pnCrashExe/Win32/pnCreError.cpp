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
*	$/Plasma20/Sources/Plasma/NucleusLib/pnCrashExe/pnCreError.cpp
*	
***/

#include "../Pch.h"
#pragma	hdrstop


namespace Crash	{
/*****************************************************************************
*
*	Private
*
***/

struct Module {
	LINK(Module)	link;
	
	unsigned_ptr	address;
	unsigned		buildId;
	unsigned		branchId;
	wchar *			name;
	wchar *			buildString;
	
	~Module	() { FREE(name); FREE(buildString); }
};

struct EmailParams : AtomicRef {
	char *			smtp;
	char *			sender;
	char *			recipients;
	char *			username;
	char *			password;
	char *			replyTo;
	
	~EmailParams () {
		FREE(smtp);
		FREE(sender);
		FREE(recipients);
		FREE(username);
		FREE(password);
		FREE(replyTo);
	}
};

struct ErrLog {
	unsigned	pos;
	wchar		name[MAX_PATH];
	char		buffer[512*1024];
	char		terminator;
};

struct DeadlockCheck {
	LINK(DeadlockCheck)	link;
	HANDLE				thread;
	unsigned			deadlockEmailMs;
	unsigned			deadlockTerminateMs;
	bool				deadlocked;
	bool				emailSent;
	wchar				debugStr[256];
};


/*****************************************************************************
*
*	Private	data
*
***/

// Assertion results from ProcessErrorLog()
const unsigned kErrFlagAssertionBreakpoint	= 0x01;
const unsigned kErrFlagAssertionExitProgram	= 0x02;

// Exception results from ProcessErrorLog()
const unsigned kErrFlagExceptionBreakpoint  = 0x04;
const unsigned kErrFlagExceptionExecHandler	= 0x08;

static const char	s_unknown[]			= "*unknown*";
static const char	s_sectionFmt_s[]	= "--------> %s <--------\r\n";

static const unsigned kStackDumpBytes	= 1024;
static const unsigned kCodeDumpBytes	= 64;

static unsigned							s_deadlockEmailMs;		// sends an email if a thread exceeds this time. If set to zero this is disabled		
static unsigned							s_deadlockTerminateMs;	// kills the process if a thread exceeds this time. If set to zero this is disabled
static CCritSect *						s_critsect;
static LISTDECL(Module,	link)			s_modules;
static EmailParams *					s_params;
static bool								s_running;

static bool								s_deadlockEnabled;
static unsigned							s_nextDeadlockCheck;
static LISTDECL(DeadlockCheck, link)	s_deadlockList;

static CCritSect						s_spareCrit;
static TSpareList<DeadlockCheck>		s_deadlockSpares;


#define SAFE_CRITSECT_ENTER()	if (s_critsect) s_critsect->Enter()
#define SAFE_CRITSECT_LEAVE()	if (s_critsect) s_critsect->Leave()


/*****************************************************************************
*
*	Internal functions
*
***/

//============================================================================
static void DelayDeadlockChecking () {
	// Delay deadlock checking for the next 2 minutes
	s_nextDeadlockCheck = 1 | (TimeGetMs() + 2 * 60 * 1000);
}

//============================================================================
static void	ReplaceEmailParams (EmailParams	* newParams) {

	if (newParams)
		newParams->IncRef();	
	
	SAFE_CRITSECT_ENTER();
	{
		SWAP(newParams,	s_params);
	}
	SAFE_CRITSECT_LEAVE();
	
	if (newParams)
		newParams->DecRef();
}

//============================================================================
static EmailParams * GetEmailParamsIncRef () {
	ref(GetEmailParamsIncRef);

	EmailParams	* params;
	
	SAFE_CRITSECT_ENTER();
	{
		if (nil	!= (params = s_params))
			params->IncRef();
	}
	SAFE_CRITSECT_LEAVE();
	
	return params;
};

//============================================================================
#ifdef _M_IX86
static void __declspec(naked) CrashFunc () {

	*(int *) 0 = 0;
}
#endif // def _M_IX86

//============================================================================
#ifdef _M_IX86
static void MakeThreadCrashOnResume (HANDLE handle) {
	
	// Point the thread's instruction pointer to CrashFunc 
	// so that it will crash once we resume it.
	CONTEXT context;
	context.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(handle, &context);
	context.Eip = (DWORD) CrashFunc;
	SetThreadContext(handle, &context);
}
#endif // def _M_IX86

//============================================================================
static inline const	char * GetExceptionString (unsigned	code) {
	#if	defined(HS_DEBUGGING) || defined(SERVER)
	switch (code) {
		#define	EXCEPTION(x) case EXCEPTION_##x: return	#x;
		EXCEPTION(ACCESS_VIOLATION)
		EXCEPTION(DATATYPE_MISALIGNMENT)
		EXCEPTION(BREAKPOINT)
		EXCEPTION(SINGLE_STEP)
		EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
		EXCEPTION(FLT_DENORMAL_OPERAND)
		EXCEPTION(FLT_DIVIDE_BY_ZERO)
		EXCEPTION(FLT_INEXACT_RESULT)
		EXCEPTION(FLT_INVALID_OPERATION)
		EXCEPTION(FLT_OVERFLOW)
		EXCEPTION(FLT_STACK_CHECK)
		EXCEPTION(FLT_UNDERFLOW)
		EXCEPTION(INT_DIVIDE_BY_ZERO)
		EXCEPTION(INT_OVERFLOW)
		EXCEPTION(PRIV_INSTRUCTION)
		EXCEPTION(IN_PAGE_ERROR)
		EXCEPTION(ILLEGAL_INSTRUCTION)
		EXCEPTION(NONCONTINUABLE_EXCEPTION)
		EXCEPTION(STACK_OVERFLOW)
		EXCEPTION(INVALID_DISPOSITION)
		EXCEPTION(GUARD_PAGE)
		EXCEPTION(INVALID_HANDLE)
		#undef EXCEPTION

		default:
		return s_unknown;
	}
	#else	// if defined(HS_DEBUGGING)	|| defined(SERVER)
	{
		ref(code);
		return "";
	}
	#endif	// if defined(HS_DEBUGGING)	|| defined(SERVER)
}

//============================================================================
static void LogWriteToDisk (ErrLog * const log) {

	HANDLE file = CreateFileW(
		log->name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES) nil,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE) nil
	);
	if (file != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		SetFilePointer(file, 0, 0, FILE_END);
		WriteFile(file, log->buffer, StrLen(log->buffer), &bytesWritten, 0);
		CloseHandle(file);
	}
}

//============================================================================
static unsigned ProcessErrorLog (
    ErrLog * const	log,
    const char		programName[],
    const char		errorType[]
) {
	ref(programName);
	ref(errorType);
	
	LogWriteToDisk(log);
	
	// Servers email the error and continue running	
	#ifdef SERVER
	{
		// @@@ TODO: Write log to file here
		
		if (EmailParams * params = GetEmailParamsIncRef()) {
			CrashSendEmail(
				params->smtp,
				params->sender,
				params->recipients,
				params->username,
				params->password,
				params->replyTo,
				programName,
				errorType,
				log->buffer
			);
			params->DecRef();
		}
		return kErrFlagAssertionBreakpoint | kErrFlagExceptionExecHandler;
	}
	
	// Client programs display an error dialog giving the user a choice of
	// sending the error or not
	#else
	{
		// Todo: make a dialog box to handle this
		return kErrFlagAssertionExitProgram | kErrFlagExceptionExecHandler;
	}
	#endif
}

//============================================================================
static unsigned ProcessBreakException () {

	#ifdef SERVER

		// Servers running as daemons should attempt to keep running
		if (ErrorGetOption(kErrOptNonGuiAsserts))
			return kErrFlagExceptionExecHandler;
		else
			return kErrFlagExceptionBreakpoint;

	#else

		return kErrFlagExceptionBreakpoint;

	#endif
}

//============================================================================
static ErrLog *	CreateErrLog () {
	// Allocate	log	memory
	ErrLog * log = (ErrLog *) VirtualAlloc(nil, sizeof(*log), MEM_COMMIT, PAGE_READWRITE);
	log->pos		= 0;
	log->terminator	= 0;

	// Initialize log filename
	wchar srcName[MAX_PATH];
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);
	StrPrintf(
		srcName,
		arrsize(srcName),
		L"Crash%02u%02u%02u.log",
		currTime.wYear % 100,
		currTime.wMonth,
		currTime.wDay
	);

	// Set log directory + filename
	AsyncLogGetDirectory(log->name,	arrsize(log->name));
	PathAddFilename(log->name, log->name, srcName, arrsize(log->name));

	return log;
}

//============================================================================
static void	DestroyErrLog (ErrLog *	errLog)	{
	VirtualFree(errLog,	0, MEM_RELEASE);
}

//============================================================================
static void	__cdecl	LogPrintf (
	ErrLog * const	log,
	const char		fmt[],
	...
) {
	va_list	args;
	va_start(args, fmt);
	char * pos = log->buffer + log->pos;
	unsigned len = StrPrintfV(
		pos,
		arrsize(log->buffer) - log->pos,
		fmt,
		args
	);
	va_end(args);
	log->pos +=	len;
}

//============================================================================
static inline void LogSectionHeader	(
	ErrLog * const	log,
	const char		section[]
) {
	LogPrintf(log, s_sectionFmt_s, section);
}

//============================================================================
#ifdef _M_IX86
static void	LogStackWalk (
	ErrLog * const		log,
	const CImageHelp &	ih,	
	HANDLE				hThread, 
	const CONTEXT &		ctx
) {
	STACKFRAME stk;
	ZeroMemory(&stk, sizeof(stk));
	stk.AddrPC.Offset		= ctx.Eip;
	stk.AddrPC.Mode			= AddrModeFlat;
	stk.AddrStack.Offset	= ctx.Esp;
	stk.AddrStack.Mode		= AddrModeFlat;
	stk.AddrFrame.Offset	= ctx.Ebp;
	stk.AddrFrame.Mode		= AddrModeFlat;

	LogSectionHeader(log, "Trace");
	for	(unsigned i	= 0; i < 100; i++) {
		const bool result =	ih.StackWalk(
			IMAGE_FILE_MACHINE_I386,
			ih.Process(),
			hThread,
			&stk,
			nil,	// context
			nil,	// read	memory routine
			ih.SymFunctionTableAccess,
			ih.SymGetModuleBase,
			nil	// translate address routine
		);
		if (!result)
			break;

		LogPrintf(
			log,
			"Pc:%08x Fr:%08x Rt:%08x Arg:%08x %08x %08x %08x ",
			stk.AddrPC.Offset,
			stk.AddrFrame.Offset,
			stk.AddrReturn.Offset,
			stk.Params[0],
			stk.Params[1],
			stk.Params[2],
			stk.Params[3]
		);

		LogPrintf(log, "\r\n");
	}

	LogPrintf(log, "\r\n");
}
#endif // _M_IX86

//============================================================================
static void	LogThread (
	ErrLog * const		log,
	const CImageHelp &	ih,
	HANDLE				hThread,
	const wchar			name[],
	const CONTEXT &		ctx
) {
	char threadName[256];
	StrPrintf(threadName, arrsize(threadName), "%SThread %#x", name, hThread);
	LogSectionHeader(log, threadName);
	
	LogPrintf(
		log,
		 "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx\r\n"
		 "esi=%08lx edi=%08lx\r\n"
		 "eip=%08lx esp=%08lx ebp=%08lx\r\n"
		 "cs=%04lx ss=%04lx ds=%04lx es=%04lx fs=%04lx gs=%04lx efl=%08lx\r\n\r\n",
		ctx.Eax, ctx.Ebx, ctx.Ecx, ctx.Edx,	ctx.Esi, ctx.Edi,
		ctx.Eip, ctx.Esp, ctx.Ebp, ctx.EFlags,
		ctx.SegCs, ctx.SegSs, ctx.SegDs, ctx.SegEs,	ctx.SegFs, ctx.SegGs, ctx.EFlags
	);

	#ifdef _M_IX86
	LogStackWalk(log, ih, hThread, ctx);
	#else
	ref(ih);
	ref(hThread);
	#endif
}

//============================================================================
static void	LogCrashInfo (
	const char			msg[],
	ErrLog * const		log,
	const CImageHelp &	ih
) {
	// Log application information
	{
		LogSectionHeader(log, "Program");
		
		wchar productIdStr[64];
		GuidToString(ProductId(), productIdStr, arrsize(productIdStr));
		
		wchar productBuildTag[128];
		ProductString(productBuildTag, arrsize(productBuildTag));
		
		SYSTEMTIME currtime;
		GetLocalTime(&currtime);
		LogPrintf(
			log,
			// beginning of	format string
			"App        : %s\r\n"
			"Build      : "
			#ifdef PLASMA_EXTERNAL_RELEASE
			"External "
			#else
			"Internal "
			#endif
			#ifdef HS_DEBUGGING
			"Debug"
			#else
			"Release"
			#endif
			"\r\n"
			"BuildMark  : %S\r\n"
			"ProductTag : %S\r\n"
			"ProductId  : %S\r\n"
			"Crashed    : %d/%d/%d %02d:%02d:%02d\r\n"
			"Msg        : %s\r\n"
			"\r\n",
			// end of format string
			ih.GetProgramName(),
			ProductBuildString(),
			productBuildTag,
			productIdStr,
			currtime.wMonth,
			currtime.wDay,
			currtime.wYear,
			currtime.wHour,
			currtime.wMinute,
			currtime.wSecond,
			msg
		);
	}

	// Log system information
	{
		LogSectionHeader(log, "System");

		char machineName[128];
		DWORD len =	arrsize(machineName);
		if (!GetComputerName(machineName, &len))
			StrCopy(machineName, s_unknown,	arrsize(machineName));
		LogPrintf(log, "Machine    : %s\r\n", machineName);

		for (;;) {
			WSADATA wsaData;
			if (WSAStartup(0x101, &wsaData) || (wsaData.wVersion != 0x101))
				break;

			wchar ipAddress[256];
			NetAddressNode addrNodes[16];
			unsigned addrCount = NetAddressGetLocal(16, addrNodes);
			LogPrintf(log, "IpAddrs    : ");
			for (unsigned i = 0; i < addrCount; ++i) {
				NetAddressNodeToString(addrNodes[i], ipAddress, arrsize(ipAddress));
				LogPrintf(log, "%S, ", ipAddress);
			}
			LogPrintf(log, "\r\n");

			WSACleanup();			
			break;
		}
		
		SYSTEM_INFO	si;
		GetSystemInfo(&si);
		DWORD ver =	GetVersion();

		LogPrintf(
			log,
			"OS Version : %u.%u\r\n"
			"CPU Count  : %u\r\n"
			"\r\n",
			LOBYTE(LOWORD(ver)), HIBYTE(LOWORD(ver)),
			si.dwNumberOfProcessors
	   );
	}

	// Log loaded modules
	{
		LogSectionHeader(log, "Modules");
		SAFE_CRITSECT_ENTER();
		for	(const Module *	p =	s_modules.Head(); p; p = p->link.Next()) {
			LogPrintf(
				log,
				"%p %S (%u.%u.%S)\r\n",
				p->address,
				p->name,
				p->buildId,
				p->branchId,
				p->buildString
			);
		}
		SAFE_CRITSECT_LEAVE();
		LogPrintf(log, "\r\n");
	}
}

//============================================================================
static LONG	ProcessException (const char occasion[], EXCEPTION_POINTERS * ep) {

	// log non-breakpont exceptions
	unsigned result;
	if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT)	{

		// if this is a	stack fault, allocate a	new	stack so we	have
		// enough space	to log the error.
		static void	* newStack;
		#define	STACKBYTES (64 * 1024)
		if (ep->ExceptionRecord->ExceptionCode == STATUS_STACK_OVERFLOW) {
			newStack = VirtualAlloc(nil, STACKBYTES, MEM_COMMIT, PAGE_READWRITE);
			__asm {
				mov		eax, [newStack]			// get new memory block
				add		eax, STACKBYTES	- 4		// point to	end	of block
				mov		[eax], esp				// save	current	stack pointer
				mov		esp, eax				// set new stack pointer
			}
		}

		DelayDeadlockChecking();

		// Allocate	error log memory
		ErrLog * log = CreateErrLog();

		// get crash log data
		static const char s_exception[]	= "Exception";
		LogSectionHeader(log, "Crash");
		LogPrintf(
			log,
			"%s: %08x %s\r\n",
			s_exception,
			ep->ExceptionRecord->ExceptionCode,
			GetExceptionString(ep->ExceptionRecord->ExceptionCode)
		);
		if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
			LogPrintf(
				log,
				"Memory at address %08x could not be %s\r\n",
				ep->ExceptionRecord->ExceptionInformation[1],
				ep->ExceptionRecord->ExceptionInformation[0] ? "written" : "read"
			);
		}
		LogPrintf(log, "\r\n");
		
		CImageHelp ih((HINSTANCE)ModuleGetInstance());
		LogCrashInfo(occasion, log, ih);

		// log crashed thread
		LogThread(
			log,
			ih,
			GetCurrentThread(),
			L"",
			*ep->ContextRecord
		);

		// display the error
		result = ProcessErrorLog(
			log,
			ih.GetProgramName(),
			occasion
		);

		DestroyErrLog(log);
		ErrorSetOption(kErrOptDisableMemLeakChecking, true);

		// if this is a	stack overflow,	restore	original stack
		if (ep->ExceptionRecord->ExceptionCode == STATUS_STACK_OVERFLOW) {
			__asm {
				mov		eax, [newStack]			// get new memory block
				add		eax, STACKBYTES	- 4		// point to	end	of block
				mov		esp, [eax]				// restore old stack pointer
			}

			VirtualFree(newStack, 0, MEM_RELEASE);
		}
	}
	else {
		result = ProcessBreakException();
	}

	// If this is a	debug build	and	the	user pressed the "debug" button
	// then	we return EXCEPTION_CONTINUE_SEARCH	so the program will	
	// activate	just-in-time debugging,	or barring that, bring up the 
	// Microsoft error handler.
	if (result & kErrFlagExceptionBreakpoint)
		return EXCEPTION_CONTINUE_SEARCH;

	return EXCEPTION_EXECUTE_HANDLER;
}


//============================================================================
static LONG	WINAPI ExceptionFilter (EXCEPTION_POINTERS * ep) {
	ref(ExceptionFilter);
	
	LONG result	= ProcessException("Unhandled Exception", ep);

	// If the instruction pointer is inside	CrashFunc then this	exception
	// is a	deadlock that couldn't be resumed, so terminate	the	program.
	#ifdef _M_IX86
	if ((ep->ContextRecord->Eip	>= (DWORD) CrashFunc)
	&&	(ep->ContextRecord->Eip	<= (DWORD) CrashFunc + 5))
		TerminateProcess(GetCurrentProcess(), 1);
	#else
	# error	"ExceptionFilter not implemented for this CPU"
	#endif	// def _M_IX86

	return result;
}

//============================================================================
static void ProcessDeadlock_CS (const char occasion[], bool crashIt = true) {

	unsigned currTimeMs = TimeGetMs();
	unsigned crashCount = 0;

	// Suspend all threads so that we can dump their callstacks
	DeadlockCheck * next, * check = s_deadlockList.Head();
	for (; check; check = next) {
		next = s_deadlockList.Next(check);
		SuspendThread(check->thread);
	}
	
	// Allocate	error log memory
	ErrLog * log = CreateErrLog();

	// Log report header and system data
	CImageHelp ih((HINSTANCE)ModuleGetInstance());
	LogCrashInfo(occasion, log, ih);

	// Log all threads
	if (!s_deadlockList.Head())
		LogPrintf(log, "*** No threads queued for deadlock check ***\r\n\r\n");
		
	check = s_deadlockList.Head();
	for (; check; check = next) {
		next = s_deadlockList.Next(check);
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_SEGMENTS | CONTEXT_INTEGER | CONTEXT_CONTROL;
        GetThreadContext(
            check->thread,
            &ctx
        );
		if (true == (check->deadlocked = (int)(check->deadlockEmailMs - currTimeMs) < 0) && !check->emailSent && s_deadlockEmailMs) {
			LogPrintf(log, "This thread has hit the min deadlock check time\r\n");
			check->emailSent = true;
		}
		else if (true == (check->deadlocked = (int)(check->deadlockTerminateMs - currTimeMs) < 0) && s_deadlockTerminateMs) {
			LogPrintf(log, "This thread has hit the max deadlock check time\r\n");
			if (crashIt) {
				MakeThreadCrashOnResume(check->thread);
				++crashCount;
			}
		}

		if(check->deadlocked) {
			LogPrintf(log, "Debug information: ");
			LogPrintf(log, "%S\r\n", check->debugStr);
		}

		LogThread(
			log,
			ih,
			check->thread,
			L"",
			ctx
		);
	}	

	(void) ProcessErrorLog(
		log,
		ih.GetProgramName(),
		occasion
	);
	
	DestroyErrLog(log);
	ErrorSetOption(kErrOptDisableMemLeakChecking, true);

	// Resume all threads
	unsigned elapsedMs = TimeGetMs() - currTimeMs;
	check = s_deadlockList.Head();
	for (; check; check = next) {
		next = s_deadlockList.Next(check);

		if (check->deadlocked && crashIt)
			// remove 'check' from list since we're crashing the thread on resume
			check->link.Unlink();
		else {
			// Update deadlock time to offset by the amount of time we had them suspended.
			check->deadlockEmailMs += elapsedMs;
			check->deadlockTerminateMs += elapsedMs;
		}

		ResumeThread(check->thread);
	}

	// Allow the resumed thread a bit of time to crash and
	// send the resulting email then terminate the process
	if (crashCount) {
		Sleep(60 * 1000);
		TerminateProcess(GetCurrentProcess(), 1);
	}	
}

//============================================================================
static void DeadlockCheckProc (void *) {
	ref(DeadlockCheckProc);

	while (s_running) {
		Sleep(5 * 1000);

		if (!s_deadlockEnabled)
			continue;

		unsigned currTimeMs = TimeGetMs();

		// Check for a forced delay in deadlock checking
		if (s_nextDeadlockCheck && (int)(s_nextDeadlockCheck - currTimeMs) > 0)
			continue;
		s_nextDeadlockCheck = 0;
		
		SAFE_CRITSECT_ENTER();
		for (;;) {
			DeadlockCheck * next, * check = s_deadlockList.Head();
			for (; check; check = next) {
				next = s_deadlockList.Next(check);
				if ((int)(check->deadlockEmailMs - currTimeMs) <= 0 && !check->emailSent && s_deadlockEmailMs) {
					// we have hit our minimum deadlock check time. Send and email but dont crash the process, yet.
					ProcessDeadlock_CS("DeadlockChecker", false);
					check->emailSent = true;
					break;
				}
				else if ((int)(check->deadlockTerminateMs - currTimeMs) <= 0 && s_deadlockTerminateMs){
					// we have hit out max deadlock check time. This will send an email and crash the process so the service can restart.
					ProcessDeadlock_CS("DeadlockChecker(Process Terminated)");
					break;
				}
			}
			break;
		}
		SAFE_CRITSECT_LEAVE(); 
	}
}

//============================================================================
#ifdef SERVER
static void StartDeadlockThread () {
	(void)_beginthread(DeadlockCheckProc, 0, nil);
}
#endif

//============================================================================
#ifdef SERVER
static void DeadlockCheckNowProc (void *) {
	SAFE_CRITSECT_ENTER();
	{
		ProcessDeadlock_CS("Deadlock Check");
	}
	SAFE_CRITSECT_LEAVE();
}
#endif

//============================================================================
#ifdef SERVER
static void ThreadReportProc (void *) {
	SAFE_CRITSECT_ENTER();
	{
		ProcessDeadlock_CS("Thread Report", false);
	}
	SAFE_CRITSECT_LEAVE();
}
#endif

//============================================================================
static void	pnCrashExeShutdown () {
	s_running = false;

	ReplaceEmailParams(nil);
	ASSERT(!s_deadlockList.Head());
	s_deadlockSpares.CleanUp();
}

//============================================================================
AUTO_INIT_FUNC(pnCrashExe) {
    // The critical section has to be initialized
    // before program startup and never freed
    static byte rawMemory[sizeof CCritSect];
    s_critsect = new(rawMemory) CCritSect;

	s_running = true;
	
	atexit(pnCrashExeShutdown);

	#ifdef SERVER
	SetUnhandledExceptionFilter(ExceptionFilter);
	StartDeadlockThread();
	#endif
}



/*****************************************************************************
*
*	Module functions
*
***/


} // namespace Crash


/*****************************************************************************
*
*	Exports
*
***/

//============================================================================
void CrashExceptionDump	(const char occasion[], void * info) {

	(void) ProcessException(occasion, (EXCEPTION_POINTERS *) info);
}

//============================================================================
void CrashSetEmailParameters (
	const char		smtp[],
	const char		sender[],
	const char		recipients[],
	const char		username[],
	const char		password[],
	const char		replyTo[]
) {
	ASSERT(smtp);
	ASSERT(sender);
	ASSERT(recipients);
	
	EmailParams	* params = NEWZERO(EmailParams);
	
	params->smtp		= StrDup(smtp);
	params->sender		= StrDup(sender);
	params->recipients	= StrDup(recipients);
	if (username)
		params->username = StrDup(username);
	if (password)
		params->password = StrDup(password);
	if (replyTo)
		params->replyTo	 = StrDup(replyTo);
		
	ReplaceEmailParams(params);
}

//============================================================================
void * CrashAddModule (
	unsigned_ptr	address,
	unsigned		buildId,
	unsigned		branchId,
	const wchar		name[],
	const wchar		buildString[]
) {
	ASSERT(name);
	Module * module	= NEWZERO(Module);
	
	module->address		= address;
	module->buildId		= buildId;
	module->branchId	= branchId;
	module->name		= StrDup(name);
	module->buildString	= StrDup(buildString);
	// trim trailing spaces from buildString
	for (unsigned i = StrLen(buildString) - 1; i > 0; --i) {
		if (module->buildString[i] != L' ')
			break;
		module->buildString[i] = 0;
	}
	
	SAFE_CRITSECT_ENTER();
	{
		s_modules.Link(module);
	}
	SAFE_CRITSECT_LEAVE();
	
	return module;
}

//============================================================================
void CrashRemoveModule (
	void *			param
) {
	Module * module	= (Module *) param;
	SAFE_CRITSECT_ENTER();
	{
		DEL(module);
	}
	SAFE_CRITSECT_LEAVE();
}


/*****************************************************************************
*
*   Deadlock detection (server only)
*
***/

//============================================================================
#ifdef SERVER
void * CrashAddDeadlockCheck (
	void *		thread,
	const wchar debugStr[]
) {
	s_spareCrit.Enter();
	DeadlockCheck * check = (DeadlockCheck *)s_deadlockSpares.Alloc();
	s_spareCrit.Leave();
	
	(void) new(check) DeadlockCheck;

	check->deadlockEmailMs = TimeGetMs() + s_deadlockEmailMs;
	check->deadlockTerminateMs = TimeGetMs() + s_deadlockTerminateMs;
	check->thread = (HANDLE) thread;
	check->emailSent = false;
	StrCopy(check->debugStr, debugStr, arrsize(check->debugStr));
	
	SAFE_CRITSECT_ENTER();
	{
		s_deadlockList.Link(check);
	}
	SAFE_CRITSECT_LEAVE();
	
	return check;
}
#endif

//============================================================================
#ifdef SERVER
void CrashRemoveDeadlockCheck (
	void *		ptr
) {
	ASSERT(ptr);
	DeadlockCheck * check = (DeadlockCheck *)ptr;
	SAFE_CRITSECT_ENTER();
	{
		s_deadlockList.Unlink(check);
	}
	SAFE_CRITSECT_LEAVE();
	
	check->~DeadlockCheck();
	s_spareCrit.Enter();
	s_deadlockSpares.Free(check);
	s_spareCrit.Leave();
}
#endif

//============================================================================
#ifdef SERVER
bool CrashEnableDeadlockChecking (
	bool			enable
) {
	SWAP(s_deadlockEnabled, enable);
	return enable;
}
#endif

//============================================================================
#ifdef SERVER
void CrashSetDeadlockCheckTimes(unsigned emailSec, unsigned terminateSec) {
	if(!emailSec && !terminateSec)
		CrashEnableDeadlockChecking(false);
	else {
		CrashEnableDeadlockChecking(true);
		s_deadlockEmailMs = emailSec * 1000;
		s_deadlockTerminateMs = terminateSec * 1000;
	}
}
#endif

//============================================================================
#ifdef SERVER
void CrashDeadlockCheckNow () {
	// Perform in a thread not queued for deadlock checking
	(void)_beginthread(DeadlockCheckNowProc, 0, nil);
}
#endif

//============================================================================
#ifdef SERVER
void CrashSendThreadReport () {
	// Perform in a thread not queued for deadlock checking
	(void)_beginthread(ThreadReportProc, 0, nil);
}
#endif
