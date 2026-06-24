/*
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

// FILE: GameSpyGP.cpp //////////////////////////////////////////////////////
// GameSpy GP callbacks, utils, etc
// Author: Matthew D. Campbell, February 2002

// --- inlined from former Precompiled/PreRTS.h ---
// NOTE: this file is excluded from the Linux build (GameSpy SDK not ported),
// so these includes are NOT compile-verified. Trim when GameSpy is ported.
//-----------------------------------------------------------------------------
// srj sez: this must come first, first, first.
#define _STLP_USE_NEWALLOC					1
//#define _STLP_USE_CUSTOM_NEWALLOC		STLSpecialAlloc
class STLSpecialAlloc;


// We actually don't use Windows for much other than timeGetTime, but it was included in 40 
// different .cpp files, so I bit the bullet and included it here.
// PLEASE DO NOT ABUSE WINDOWS OR IT WILL BE REMOVED ENTIRELY. :-)
//--------------------------------------------------------------------------------- System Includes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <atlbase.h>
#include <windows.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <fstream>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <objbase.h>
#include <ocidl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <tchar.h>
#include <time.h>

#ifdef _WIN32
#include <mmsystem.h>
#include <process.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>
#include <snmp.h>
#include <vfw.h>
#include <winerror.h>
#include <wininet.h>
#include <winreg.h>

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION	0x800
#endif
#include <dinput.h>
#endif

//------------------------------------------------------------------------------------ STL Includes
// srj sez: no, include STLTypesdefs below, instead, thanks
//#include <algorithm>
//#include <bitset>
//#include <unordered_map>
//#include <list>
//#include <map>
//#include <queue>
//#include <set>
//#include <stack>
//#include <string>
//#include <vector>

//------------------------------------------------------------------------------------ RTS Includes
// Icky. These have to be in this order.
#include "Lib/BaseType.h"
#include "Common/STLTypedefs.h"
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/AsciiString.h"
#include "Common/SubsystemInterface.h"

#include "Common/GameCommon.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"

// You might not want Kindof in here because it seems like it changes frequently, but the problem
// is that Kindof is included EVERYWHERE, so it might as well be precompiled.
#include "Common/INI.h"
#include "Common/KindOf.h"
#include "Common/DisabledTypes.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/LogicRandomValue.h"
#include "Common/ObjectStatusTypes.h"

#include "Common/Thing.h"
#include "Common/UnicodeString.h"
// --- end inlined PreRTS.h ---

#include "GameClient/GameText.h"
#include "GameNetwork/GameSpy.h"
#include "GameNetwork/GameSpyGP.h"
#include "GameNetwork/GameSpyOverlay.h"

GPConnection TheGPConnectionObj;
GPConnection *TheGPConnection = &TheGPConnectionObj;
GPProfile GameSpyLocalProfile = 0;
char GameSpyProfilePassword[64];

void GPRecvBuddyMessageCallback(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyMessageCallback: message from %d is %s\n", arg->profile, arg->message));

	//gpGetInfo(pconn, arg->profile, GP_DONT_CHECK_CACHE, GP_BLOCKING, (GPCallback)Whois, NULL);
	//printf("MESSAGE (%d): %s: %s\n", msgCount,whois, arg->message);
}

static void buddyTryReconnect( void )
{
	TheGameSpyChat->reconnectProfile();
}

void GPErrorCallback(GPConnection * pconnection, GPErrorArg * arg, void * param)
{
	DEBUG_LOG(("GPErrorCallback\n"));

	AsciiString errorCodeString;
	AsciiString resultString;

	#define RESULT(x) case x: resultString = #x; break;
	switch(arg->result)
	{
		RESULT(GP_NO_ERROR)
		RESULT(GP_MEMORY_ERROR)
		RESULT(GP_PARAMETER_ERROR)
		RESULT(GP_NETWORK_ERROR)
		RESULT(GP_SERVER_ERROR)
	default:
		resultString = "Unknown result!";
	}
	#undef RESULT

	#define ERRORCODE(x) case x: errorCodeString = #x; break;
	switch(arg->errorCode)
	{
		ERRORCODE(GP_GENERAL)
		ERRORCODE(GP_PARSE)
		ERRORCODE(GP_NOT_LOGGED_IN)
		ERRORCODE(GP_BAD_SESSKEY)
		ERRORCODE(GP_DATABASE)
		ERRORCODE(GP_NETWORK)
		ERRORCODE(GP_FORCED_DISCONNECT)
		ERRORCODE(GP_CONNECTION_CLOSED)
		ERRORCODE(GP_LOGIN)
		ERRORCODE(GP_LOGIN_TIMEOUT)
		ERRORCODE(GP_LOGIN_BAD_NICK)
		ERRORCODE(GP_LOGIN_BAD_EMAIL)
		ERRORCODE(GP_LOGIN_BAD_PASSWORD)
		ERRORCODE(GP_LOGIN_BAD_PROFILE)
		ERRORCODE(GP_LOGIN_PROFILE_DELETED)
		ERRORCODE(GP_LOGIN_CONNECTION_FAILED)
		ERRORCODE(GP_LOGIN_SERVER_AUTH_FAILED)
		ERRORCODE(GP_NEWUSER)
		ERRORCODE(GP_NEWUSER_BAD_NICK)
		ERRORCODE(GP_NEWUSER_BAD_PASSWORD)
		ERRORCODE(GP_UPDATEUI)
		ERRORCODE(GP_UPDATEUI_BAD_EMAIL)
		ERRORCODE(GP_NEWPROFILE)
		ERRORCODE(GP_NEWPROFILE_BAD_NICK)
		ERRORCODE(GP_NEWPROFILE_BAD_OLD_NICK)
		ERRORCODE(GP_UPDATEPRO)
		ERRORCODE(GP_UPDATEPRO_BAD_NICK)
		ERRORCODE(GP_ADDBUDDY)
		ERRORCODE(GP_ADDBUDDY_BAD_FROM)
		ERRORCODE(GP_ADDBUDDY_BAD_NEW)
		ERRORCODE(GP_ADDBUDDY_ALREADY_BUDDY)
		ERRORCODE(GP_AUTHADD)
		ERRORCODE(GP_AUTHADD_BAD_FROM)
		ERRORCODE(GP_AUTHADD_BAD_SIG)
		ERRORCODE(GP_STATUS)
		ERRORCODE(GP_BM)
		ERRORCODE(GP_BM_NOT_BUDDY)
		ERRORCODE(GP_GETPROFILE)
		ERRORCODE(GP_GETPROFILE_BAD_PROFILE)
		ERRORCODE(GP_DELBUDDY)
		ERRORCODE(GP_DELBUDDY_NOT_BUDDY)
		ERRORCODE(GP_DELPROFILE)
		ERRORCODE(GP_DELPROFILE_LAST_PROFILE)
		ERRORCODE(GP_SEARCH)
		ERRORCODE(GP_SEARCH_CONNECTION_FAILED)
	default:
		errorCodeString = "Unknown error code!";
	}
	#undef ERRORCODE

	if(arg->fatal)
	{
		DEBUG_LOG(( "-----------\n"));
		DEBUG_LOG(( "GP FATAL ERROR\n"));
		DEBUG_LOG(( "-----------\n"));

		// if we're still connected to the chat server, tell the user.  He can always hit the buddy
		// button to try reconnecting.  Oh yes, also hide the buddy popup.
		GameSpyCloseOverlay(GSOVERLAY_BUDDY);
		if (TheGameSpyChat->isConnected())
		{
			GSMessageBoxYesNo(TheGameText->fetch("GUI:GPErrorTitle"), TheGameText->fetch("GUI:GPDisconnected"), buddyTryReconnect, NULL);
		}
	}
	else
	{
		DEBUG_LOG(( "-----\n"));
		DEBUG_LOG(( "GP ERROR\n"));
		DEBUG_LOG(( "-----\n"));
	}
	DEBUG_LOG(( "RESULT: %s (%d)\n", resultString.str(), arg->result));
	DEBUG_LOG(( "ERROR CODE: %s (0x%X)\n", errorCodeString.str(), arg->errorCode));
	DEBUG_LOG(( "ERROR STRING: %s\n", arg->errorString));
}

void GPRecvBuddyStatusCallback(GPConnection * connection, GPRecvBuddyStatusArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyStatusCallback: info on %d is in %d\n", arg->profile, arg->index));

	//GameSpyUpdateBuddyOverlay();
}

void GPRecvBuddyRequestCallback(GPConnection * connection, GPRecvBuddyRequestArg * arg, void * param)
{
	DEBUG_LOG(("GPRecvBuddyRequestCallback: %d wants to be our buddy because '%s'\n", arg->profile, arg->reason));
}
