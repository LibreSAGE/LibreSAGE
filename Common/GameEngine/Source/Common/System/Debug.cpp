/*
**	Command & Conquer Generals(tm)
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Debug.cpp 
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Debug.cpp
//
// Created:   Steven Johnson, August 2001
//
// Desc:      Debug logging and other debug utilities
//
// ----------------------------------------------------------------------------

// SYSTEM INCLUDES 

// USER INCLUDES 
#define DEBUG_THREADSAFE
#ifdef DEBUG_THREADSAFE
#include "Common/CriticalSection.h"
#endif
#include "Common/Debug.h"
#include "Common/Registry.h"
#include "Common/UnicodeString.h"
#include "Common/StackDump.h"

#include <SDL3/SDL.h>
#include "osdep.h"

// Horrible reference, but we really, really need to know if we are windowed.
extern bool DX8Wrapper_IsWindowed;
extern SDL_Window* ApplicationWindow;

extern char *gAppPrefix; /// So WB can have a different log file name.

bool DebugIgnoreAsserts = false;	///< if true, ignore asserts and continue execution. (default is false)
bool DebugIgnoreStackTrace = false;	///< if true, ignore crashes and continue execution. (default is false)

// ----------------------------------------------------------------------------
// DEFINES 
// ----------------------------------------------------------------------------

#ifdef DEBUG_LOGGING

#if defined(_INTERNAL)
	#define DEBUG_FILE_NAME				"DebugLogFileI.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevI.txt"
#elif defined(_DEBUG)
	#define DEBUG_FILE_NAME				"DebugLogFileD.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrevD.txt"
#else
	#define DEBUG_FILE_NAME				"DebugLogFile.txt"
	#define DEBUG_FILE_NAME_PREV	"DebugLogFilePrev.txt"
#endif

#endif

// ----------------------------------------------------------------------------
// PRIVATE TYPES 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// PRIVATE DATA 
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
static FILE *theLogFile = NULL;
#endif
#define LARGE_BUFFER	8192
static char theBuffer[ LARGE_BUFFER ];	// make it big to avoid weird overflow bugs in debug mode
static int theDebugFlags = 0;
static DWORD theMainThreadID = 0;
// ----------------------------------------------------------------------------
// PUBLIC DATA 
// ----------------------------------------------------------------------------

char* TheCurrentIgnoreCrashPtr = NULL;
#ifdef DEBUG_LOGGING
UnsignedInt DebugLevelMask = 0;
const char *TheDebugLevels[DEBUG_LEVEL_MAX] = {
	"NET"
};
#endif

// ----------------------------------------------------------------------------
// PRIVATE PROTOTYPES 
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString(void);
static const char *getCurrentTickString(void);
static const char *prepBuffer(const char* format, char *buffer, size_t bufferSize);
#ifdef DEBUG_LOGGING
static void doLogOutput(const char *buffer);
#endif
static int doCrashBox(const char *buffer, Bool logResult);
static void whackFunnyCharacters(char *buf);
#ifdef DEBUG_STACKTRACE
static void doStackDump();
#endif

// ----------------------------------------------------------------------------
// PRIVATE FUNCTIONS 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
inline Bool ignoringAsserts()
{
#if defined(_DEBUG) || defined(_INTERNAL)
	return !DX8Wrapper_IsWindowed || DebugIgnoreAsserts;
#else
	return !DX8Wrapper_IsWindowed;
#endif
}

// ----------------------------------------------------------------------------
inline HWND getThreadHWND()
{
#ifdef _WIN32
	return (theMainThreadID == GetCurrentThreadId())?ApplicationHWnd:NULL;
#else
	return NULL;
#endif
}

// ----------------------------------------------------------------------------
// getCurrentTimeString
/** 
	Return the current time in string form
*/
// ----------------------------------------------------------------------------
static const char *getCurrentTimeString(void)
{
	time_t aclock;
	time(&aclock);
	struct tm *newtime = localtime(&aclock);
	return asctime(newtime);
}

// ----------------------------------------------------------------------------
// getCurrentTickString 
/** 
	Return the current TickCount in string form
*/
// ----------------------------------------------------------------------------
static const char *getCurrentTickString(void)
{
	static char TheTickString[32];
	snprintf(TheTickString, sizeof(TheTickString), "(T=%08lx)",::GetTickCount());
	return TheTickString;
}

// ----------------------------------------------------------------------------
// prepBuffer
// zap the buffer and optionally prepend the tick time.
// ----------------------------------------------------------------------------
/**
	Empty the buffer passed in, then optionally prepend the current TickCount
	value in string form, depending on the setting of theDebugFlags.
*/
static const char *prepBuffer(const char* format, char *buffer, size_t bufferSize)
{
	buffer[0] = 0;
#ifdef ALLOW_DEBUG_UTILS
	if (theDebugFlags & DEBUG_FLAG_PREPEND_TIME)
	{
		strncpy(buffer, getCurrentTickString(), bufferSize);
		buffer[bufferSize - 1] = '\0';
		strncat(buffer, " ", bufferSize - strlen(buffer) - 1);
	}
#endif
	return format;
}

// ----------------------------------------------------------------------------
// doLogOutput
/** 
	send a string directly to the log file and/or console without further processing.
*/
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
static void doLogOutput(const char *buffer)
{
	// log message to file
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_FILE)
	{
		if (theLogFile)
		{
			fprintf(theLogFile, "%s", buffer);	// note, no \n (should be there already)
			fflush(theLogFile);
		}
	}

	// log message to dev studio output window
	if (theDebugFlags & DEBUG_FLAG_LOG_TO_CONSOLE)
	{
		::OutputDebugString(buffer);
	}
}
#endif

// ----------------------------------------------------------------------------
// doCrashBox
/*
	present a messagebox with the given message. Depending on user selection,
	we exit the app, break into debugger, or continue execution. 
*/
// ----------------------------------------------------------------------------
static int doCrashBox(const char *buffer, Bool logResult)
{
	int result;

	const SDL_MessageBoxButtonData buttons[] = {
		{ /* .flags, .buttonid, .text */        0, 0, "Abort" },
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Retry" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Ignore" },
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_WARNING, /* .flags */
		NULL, /* .window */
		"Assertion Failure", /* .title */
		buffer, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		NULL /* .colorScheme */
	};

	if (!ignoringAsserts()) {
		SDL_ShowMessageBox(&messageboxdata, &result);
	}	else {
		result = 2;
	}

	switch(result)
	{
		case 0: // ABORT
#ifdef DEBUG_LOGGING
			if (logResult)
				DebugLog("[Abort]\n");
#endif
			_exit(1);
			break;
		case 1: // RETRY
#ifdef DEBUG_LOGGING
			if (logResult)
				DebugLog("[Retry]\n");
#endif
			__debugbreak();
			break;
		case 2: // IGNORE
#ifdef DEBUG_LOGGING
			// do nothing, just keep going
			if (logResult)
				DebugLog("[Ignore]\n");
#endif
			break;
	}
	return result;
}

#ifdef DEBUG_STACKTRACE
// ----------------------------------------------------------------------------
/**
	Dumps a stack trace (from the current PC) to logfile and/or console.
*/
static void doStackDump()
{
	const int STACKTRACE_SIZE	= 24;
	const int STACKTRACE_SKIP = 2;
	void* stacktrace[STACKTRACE_SIZE];

	doLogOutput("\nStack Dump:\n");
	::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
	::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, doLogOutput);
}
#endif

// ----------------------------------------------------------------------------
// whackFunnyCharacters 
/** 
	Eliminates any undesirable nonprinting characters, aside from newline,
	replacing them with spaces.
*/
// ----------------------------------------------------------------------------
static void whackFunnyCharacters(char *buf)
{
	for (char *p = buf + strlen(buf) - 1; p >= buf; --p)
	{
		// ok, these are naughty magic numbers, but I'm guessing you know ASCII.... 
		if (*p >= 0 && *p < 32 && *p != 10 && *p != 13)
			*p = 32;
	}
}

// ----------------------------------------------------------------------------
// PUBLIC FUNCTIONS 
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// DebugInit 
// ----------------------------------------------------------------------------
#ifdef ALLOW_DEBUG_UTILS
/**
	Initialize the debug utilities. This should be called once, as near to the 
	start of the app as possible, before anything else (since other code will
	probably want to make use of it).
*/
void DebugInit(int flags)
{
//	if (theDebugFlags != 0)
//		::MessageBox(NULL, "Debug already inited", "", MB_OK|MB_APPLMODAL);

	// just quietly allow multiple calls to this, so that static ctors can call it.
	if (theDebugFlags == 0) 
	{
		theDebugFlags = flags;
	#ifdef _WINDOWS
		theMainThreadID = GetCurrentThreadId();
	#endif
	#ifdef DEBUG_LOGGING

		char dirbuf[ _MAX_PATH ];
		::GetModuleFileName( NULL, dirbuf, sizeof( dirbuf ) );
		char *pEnd = dirbuf + strlen( dirbuf );
		while( pEnd != dirbuf ) 
		{
			if( *pEnd == '\\' ) 
			{
				*(pEnd + 1) = 0;
				break;
			}
			pEnd--;
		}

		char prevbuf[ _MAX_PATH ];
		char curbuf[ _MAX_PATH ];

		strncpy(prevbuf, dirbuf, sizeof(prevbuf));
		prevbuf[sizeof(prevbuf) - 1] = '\0';
		strncat(prevbuf, gAppPrefix, sizeof(prevbuf) - strlen(prevbuf) - 1);
		strncat(prevbuf, DEBUG_FILE_NAME_PREV, sizeof(prevbuf) - strlen(prevbuf) - 1);
		strncpy(curbuf, dirbuf, sizeof(curbuf));
		curbuf[sizeof(curbuf) - 1] = '\0';
		strncat(curbuf, gAppPrefix, sizeof(curbuf) - strlen(curbuf) - 1);
		strncat(curbuf, DEBUG_FILE_NAME, sizeof(curbuf) - strlen(curbuf) - 1);

 		remove(prevbuf);
		rename(curbuf, prevbuf);
		theLogFile = fopen(curbuf, "w");
		if (theLogFile != NULL)
		{
			DebugLog("Log %s opened: %s\n", curbuf, getCurrentTimeString());
		} 
	#endif
	}

}  
#endif

// ----------------------------------------------------------------------------
// DebugLog 
// ----------------------------------------------------------------------------
#ifdef DEBUG_LOGGING
/**
	Print a character string to the logfile and/or console.
*/
void DebugLog(const char *format, ...)
{
#ifdef DEBUG_THREADSAFE
	ScopedCriticalSection scopedCriticalSection(TheDebugLogCriticalSection);
#endif

	if (theDebugFlags == 0)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "DebugLog - Debug not inited properly", "", ApplicationWindow);

	format = prepBuffer(format, theBuffer, sizeof(theBuffer));

	va_list arg;
  va_start(arg, format);
  vsnprintf(theBuffer + strlen(theBuffer), sizeof(theBuffer) - strlen(theBuffer), format, arg);
  va_end(arg);

	if (strlen(theBuffer) >= sizeof(theBuffer))
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "String too long for debug buffer", "", ApplicationWindow);

	whackFunnyCharacters(theBuffer);
	doLogOutput(theBuffer);
} 
#endif

// ----------------------------------------------------------------------------
// DebugCrash
// ----------------------------------------------------------------------------
#ifdef DEBUG_CRASHING
/**
	Print a character string to the logfile and/or console, then halt execution
	while presenting the user with an exit/debug/ignore dialog containing the same
	text message.
*/
void DebugCrash(const char *format, ...)
{
	// Note: You might want to make this thread safe, but we cannot. The reason is that 
	// there is an implicit requirement on other threads that the message loop be running.

	// make it not static so that it'll be thread-safe.
	// make it big to avoid weird overflow bugs in debug mode
	char theCrashBuffer[ LARGE_BUFFER ];	
	if (theDebugFlags == 0)
	{
		if (!DX8Wrapper_IsWindowed) {
			if (ApplicationWindow) {
				SDL_HideWindow(ApplicationWindow);
			}
		}
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "", "DebugCrash - Debug not inited properly", ApplicationWindow);
	}

	format = prepBuffer(format, theCrashBuffer, sizeof(theCrashBuffer));
	strncat(theCrashBuffer, "ASSERTION FAILURE: ", sizeof(theCrashBuffer) - strlen(theCrashBuffer) - 1);

	va_list arg;
  va_start(arg, format);
  vsnprintf(theCrashBuffer + strlen(theCrashBuffer), sizeof(theCrashBuffer) - strlen(theCrashBuffer), format, arg);
  va_end(arg);

	if (strlen(theCrashBuffer) >= sizeof(theCrashBuffer))
	{
		if (!DX8Wrapper_IsWindowed) {
			if (ApplicationWindow) {
				SDL_HideWindow(ApplicationWindow);
			}
		}
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "", "String too long for debug buffers", ApplicationWindow);
	}

#ifdef DEBUG_LOGGING
	if (ignoringAsserts()) 
	{
		doLogOutput("**** CRASH IN FULL SCREEN - Auto-ignored, CHECK THIS LOG!\n");
	}
	whackFunnyCharacters(theCrashBuffer);
	doLogOutput(theCrashBuffer);
#endif
#ifdef DEBUG_STACKTRACE
	if (!DebugIgnoreStackTrace) 
	{
		doStackDump();
	}
#endif

	strncat(theCrashBuffer, "\n\nAbort->exception; Retry->debugger; Ignore->continue\n", sizeof(theCrashBuffer) - strlen(theCrashBuffer) - 1);

	int result = doCrashBox(theCrashBuffer, true);

	// doCrashBox returns 2 for IGNORE
	if (result == 2 && TheCurrentIgnoreCrashPtr != NULL)
	{
		bool ignoreFromNowOn = true;
		if (!ignoringAsserts())
		{
			const SDL_MessageBoxButtonData buttons[] = {
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" },
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
			};
			const SDL_MessageBoxData messageboxdata = {
				SDL_MESSAGEBOX_WARNING, /* .flags */
				NULL, /* .window */
				"", /* .title */
				"Ignore this crash from now on?", /* .message */
				SDL_arraysize(buttons), /* .numbuttons */
				buttons, /* .buttons */
				NULL /* .colorScheme */
			};
			int yn = 1;
			SDL_ShowMessageBox(&messageboxdata, &yn);
			ignoreFromNowOn = (yn == 1);
		}
		if (ignoreFromNowOn)
			*TheCurrentIgnoreCrashPtr = 1;
	}

}
#endif

// ----------------------------------------------------------------------------
// DebugShutdown 
// ----------------------------------------------------------------------------
#ifdef ALLOW_DEBUG_UTILS
/**
	Shut down the debug utilities. This should be called once, as near to the 
	end of the app as possible, after everything else (since other code will
	probably want to make use of it).
*/
void DebugShutdown()
{
#ifdef DEBUG_LOGGING
	if (theLogFile)
	{
		DebugLog("Log closed: %s\n", getCurrentTimeString());
		fclose(theLogFile);
	}
	theLogFile = NULL;
#endif
	theDebugFlags = 0;
}  

// ----------------------------------------------------------------------------
// DebugGetFlags 
// ----------------------------------------------------------------------------
/**
	Get the current values for the flags passed to DebugInit. Most code will never
	need to use this; the most common usage would be to temporarily enable or disable
	the DEBUG_FLAG_PREPEND_TIME bit for complex logfile messages.
*/
int DebugGetFlags()
{
	return theDebugFlags;
}

// ----------------------------------------------------------------------------
// DebugSetFlags 
// ----------------------------------------------------------------------------
/**
	Set the current values for the flags passed to DebugInit. Most code will never
	need to use this; the most common usage would be to temporarily enable or disable
	the DEBUG_FLAG_PREPEND_TIME bit for complex logfile messages.
*/
void DebugSetFlags(int flags)
{
	theDebugFlags = flags;
}

#endif	// ALLOW_DEBUG_UTILS

#ifdef DEBUG_PROFILE
// ----------------------------------------------------------------------------
SimpleProfiler::SimpleProfiler()
{
#ifdef _WINDOWS
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
#endif
	m_startThisSession = 0;
	m_totalThisSession = 0;
	m_totalAllSessions = 0;
	m_numSessions = 0;
}

// ----------------------------------------------------------------------------
void SimpleProfiler::start()
{
	DEBUG_ASSERTCRASH(m_startThisSession == 0, ("already started"));
#ifdef _WINDOWS
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startThisSession);
#endif
}

// ----------------------------------------------------------------------------
void SimpleProfiler::stop()
{
	if (m_startThisSession != 0) 
	{
		int64_t stop;
	#ifdef _WINDOWS
		QueryPerformanceCounter((LARGE_INTEGER*)&stop);
	#endif
		m_totalThisSession = stop - m_startThisSession;
		m_totalAllSessions += stop - m_startThisSession;
		m_startThisSession = 0;
		++m_numSessions;
	}
}

// ----------------------------------------------------------------------------
void SimpleProfiler::stopAndLog(const char *msg, int howOftenToLog, int howOftenToResetAvg)
{
	stop();
	// howOftenToResetAvg==0 means "never reset"
	if (howOftenToResetAvg > 0 && m_numSessions >= howOftenToResetAvg)
	{
		m_numSessions = 0;
		m_totalAllSessions = 0;
		DEBUG_LOG(("%s: reset averages\n",msg));
	}
	DEBUG_ASSERTLOG(m_numSessions % howOftenToLog != 0, ("%s: %f msec, total %f msec, avg %f msec\n",msg,getTime(),getTotalTime(),getAverageTime()));
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getTime()
{
	stop();
	return (double)m_totalThisSession / (double)m_freq * 1000.0;
}

// ----------------------------------------------------------------------------
int SimpleProfiler::getNumSessions()
{
	stop();
	return m_numSessions;
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getTotalTime()
{
	stop();
	if (!m_numSessions)
		return 0.0;

	return (double)m_totalAllSessions * 1000.0 / ((double)m_freq);
}

// ----------------------------------------------------------------------------
double SimpleProfiler::getAverageTime()
{
	stop();
	if (!m_numSessions)
		return 0.0;

	return (double)m_totalAllSessions * 1000.0 / ((double)m_freq * (double)m_numSessions);
}

#endif	// ALLOW_DEBUG_UTILS

// ----------------------------------------------------------------------------
// ReleaseCrash
// ----------------------------------------------------------------------------
/**
	Halt the application, EVEN IN FINAL RELEASE BUILDS. This should be called
	only when a crash is guaranteed by continuing, and no meaningful continuation
	of processing is possible, even by throwing an exception.
*/

	#define RELEASECRASH_FILE_NAME				"ReleaseCrashInfo.txt"
	#define RELEASECRASH_FILE_NAME_PREV		"ReleaseCrashInfoPrev.txt"

	static FILE *theReleaseCrashLogFile = NULL;

	static void releaseCrashLogOutput(const char *buffer)
	{
		if (theReleaseCrashLogFile)
		{
			fprintf(theReleaseCrashLogFile, "%s", buffer);	// note, no \n (should be there already)
			fflush(theReleaseCrashLogFile);
		}
	}

void ReleaseCrash(const char *reason)
{
	/// do additional reporting on the crash, if possible

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationWindow) {
			SDL_HideWindow(ApplicationWindow);
		}
	}
//#if defined(_DEBUG) || defined(_INTERNAL)
//	/* static */ char buff[8192]; // not so static so we can be threadsafe
//	_snprintf(buff, 8192, "Sorry, a serious error occurred. (%s)", reason);/
//	buff[8191] = 0;
//	::MessageBox(NULL, buff, "Technical Difficulties...", MB_OK|MB_SYSTEMMODAL|MB_ICONERROR);
//#else
//	::MessageBox(NULL, "Sorry, a serious error occurred.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);
//#endif

	char prevbuf[ _MAX_PATH ] = { 0 };
	char curbuf[ _MAX_PATH ] = { 0 };

	strncpy(prevbuf, RELEASECRASH_FILE_NAME_PREV, sizeof(prevbuf));
	prevbuf[sizeof(prevbuf) - 1] = '\0';
	strncpy(curbuf, RELEASECRASH_FILE_NAME, sizeof(curbuf));
	curbuf[sizeof(curbuf) - 1] = '\0';

 	remove(prevbuf);
	rename(curbuf, prevbuf);

	theReleaseCrashLogFile = fopen(curbuf, "w");
	if (theReleaseCrashLogFile)
	{
		fprintf(theReleaseCrashLogFile, "Release Crash at %s; Reason %s\n", getCurrentTimeString(), reason);
		fprintf(theReleaseCrashLogFile, "\nLast error:\n%s\n\nCurrent stack:\n", g_LastErrorDump.str());
		const int STACKTRACE_SIZE	= 12;
		const int STACKTRACE_SKIP = 6;
		void* stacktrace[STACKTRACE_SIZE];
		::FillStackAddresses(stacktrace, STACKTRACE_SIZE, STACKTRACE_SKIP);
		::StackDumpFromAddresses(stacktrace, STACKTRACE_SIZE, releaseCrashLogOutput);

		fflush(theReleaseCrashLogFile);
		fclose(theReleaseCrashLogFile);
		theReleaseCrashLogFile = NULL;
	}

	if (!DX8Wrapper_IsWindowed) {
		if (ApplicationWindow) {
			SDL_HideWindow(ApplicationWindow);
		}
	}

#if defined(_DEBUG) || defined(_INTERNAL)
	/* static */ char buff[8192]; // not so static so we can be threadsafe
	_snprintf(buff, 8192, "Sorry, a serious error occurred. (%s)", reason);
	buff[8191] = 0;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Technical Difficulties...", buff, ApplicationWindow);
#else
// crash error messaged changed 3/6/03 BGC
//	::MessageBox(NULL, "Sorry, a serious error occurred.", "Technical Difficulties...", MB_OK|MB_TASKMODAL|MB_ICONERROR);

	if (!GetRegistryLanguage().compareNoCase("german2") || !GetRegistryLanguage().compareNoCase("german") )
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fehler...", u8"Es ist ein gravierender Fehler aufgetreten. Solche Fehler können durch viele verschiedene Dinge wie Viren, Überhitzte Hardware und Hardware, die den Mindestanforderungen des Spiels nicht entspricht, ausgelöst werden. Tipps zur Vorgehensweise findest du in den Foren unter www.generals.ea.com, Informationen zum Technischen Kundendienst im Handbuch zum Spiel.", ApplicationWindow);
	}
	else
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Technical Difficulties...", "You have encountered a serious error.  Serious errors can be caused by many things including viruses, overheated hardware and hardware that does not meet the minimum specifications for the game. Please visit the forums at www.generals.ea.com for suggested courses of action or consult your manual for Technical Support contact information.", ApplicationWindow);
	}

#endif

	_exit(1);
}
