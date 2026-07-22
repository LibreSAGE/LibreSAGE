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


#pragma once
#ifndef __NETWORKUTIL_H
#define __NETWORKUTIL_H

#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/NetworkInterface.h"

// Forward declaration so callers don't have to pull in <SDL3_net/SDL_net.h>.
typedef struct NET_Address NET_Address;

UnsignedInt ResolveIP(AsciiString host);

/**
 * Convert a host-order IPv4 address (the engine's internal representation) into
 * an SDL3_net NET_Address*. The returned address is owned by an internal cache;
 * do NOT unref it. Returns NULL if the address could not be resolved.
 */
NET_Address *IPToNetAddress(UnsignedInt ip);

/**
 * Convert a resolved SDL3_net NET_Address* into a host-order IPv4 address.
 * Returns 0 if the address is NULL, unresolved, or not an IPv4 address.
 */
UnsignedInt NetAddressToIP(NET_Address *addr);
UnsignedShort GenerateNextCommandID();
Bool DoesCommandRequireACommandID(NetCommandType type);
Bool CommandRequiresAck(NetCommandMsg *msg);
Bool CommandRequiresDirectSend(NetCommandMsg *msg);
Bool IsCommandSynchronized(NetCommandType type);
AsciiString GetAsciiNetCommandType(NetCommandType type);

#ifdef DEBUG_LOGGING
void dumpBufferToLog(const void *vBuf, Int len, const char *fname, Int line);
#define LOGBUFFER(buf, len) dumpBufferToLog(buf, len, __FILE__, __LINE__)
#else
#define LOGBUFFER(buf, len) {}
#endif // DEBUG_LOGGING

#endif
