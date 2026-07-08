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


#include "Common/Thing.h"

#include "GameNetwork/NetworkUtil.h"

#include <SDL3_net/SDL_net.h>

Int MAX_FRAMES_AHEAD = 128;
Int MIN_RUNAHEAD = 10;
Int FRAME_DATA_LENGTH = (MAX_FRAMES_AHEAD+1)*2;
Int FRAMES_TO_KEEP = (MAX_FRAMES_AHEAD/2) + 1;

#ifdef DEBUG_LOGGING

void dumpBufferToLog(const void *vBuf, Int len, const char *fname, Int line)
{
	DEBUG_LOG(("======= dumpBufferToLog() %d bytes =======\n", len));
	DEBUG_LOG(("Source: %s:%d\n", fname, line));
	const char *buf = (const char *)vBuf;
	Int numLines = len / 8;
	if ((len % 8) != 0)
	{
		++numLines;
	}
	for (Int dumpindex = 0; dumpindex < numLines; ++dumpindex)
	{
		Int offset = dumpindex*8;
		DEBUG_LOG(("\t%5.5d\t", offset));
		Int dumpindex2;
		Int numBytesThisLine = min(8, len - offset);
		for (dumpindex2 = 0; dumpindex2 < numBytesThisLine; ++dumpindex2)
		{
			Int c = (buf[offset + dumpindex2] & 0xff);
			DEBUG_LOG(("%02X ", c));
		}
		for (; dumpindex2 < 8; ++dumpindex2)
		{
			DEBUG_LOG(("   "));
		}
		DEBUG_LOG((" | "));
		for (dumpindex2 = 0; dumpindex2 < numBytesThisLine; ++dumpindex2)
		{
			char c = buf[offset + dumpindex2];
			DEBUG_LOG(("%c", (isprint(c)?c:'.')));
		}
		DEBUG_LOG(("\n"));
	}
	DEBUG_LOG(("End of packet dump\n"));
}

#endif // DEBUG_LOGGING

/**
 * ResolveIP turns a string ("games2.westwood.com", or "192.168.0.1") into
 * a 32-bit unsigned integer.
 */
UnsignedInt ResolveIP(AsciiString host)
{
  if (host.getLength() == 0)
  {
	  DEBUG_LOG(("ResolveIP(): Can't resolve NULL\n"));
	  return 0;
  }

  // Resolve the host name (or numeric IP string) via SDL3_net. Numeric strings
  // resolve immediately; host names hit the resolver thread, so block until done.
  NET_Address *addr = NET_ResolveHostname(host.str());
  if (addr == NULL)
  {
	  DEBUG_LOG(("ResolveIP(): Can't resolve %s\n", host.str()));
	  return 0;
  }

  UnsignedInt ip = 0;
  if (NET_WaitUntilResolved(addr, -1) == NET_SUCCESS)
  {
    ip = NetAddressToIP(addr);
  }
  else
  {
	  DEBUG_LOG(("ResolveIP(): Can't resolve %s\n", host.str()));
  }

  NET_UnrefAddress(addr);
  return ip;
}

/**
 * Cache of host-order IPv4 -> NET_Address* mappings. SDL3_net resolves every
 * address (even numeric ones) on a background thread, so we keep the results
 * around rather than re-resolving for every outgoing packet. The cached
 * addresses live for the duration of the program.
 */
NET_Address *IPToNetAddress(UnsignedInt ip)
{
  static std::map<UnsignedInt, NET_Address *> theAddressCache;

  std::map<UnsignedInt, NET_Address *>::iterator it = theAddressCache.find(ip);
  if (it != theAddressCache.end())
  {
    return it->second;
  }

  char buf[24];
  snprintf(buf, sizeof(buf), "%u.%u.%u.%u", (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);

  NET_Address *addr = NET_ResolveHostname(buf);
  if (addr != NULL && NET_WaitUntilResolved(addr, -1) != NET_SUCCESS)
  {
    DEBUG_LOG(("IPToNetAddress(): Can't resolve %s\n", buf));
    NET_UnrefAddress(addr);
    addr = NULL;
  }

  theAddressCache[ip] = addr;
  return addr;
}

UnsignedInt NetAddressToIP(NET_Address *addr)
{
  if (addr == NULL)
  {
    return 0;
  }

  const char *str = NET_GetAddressString(addr);
  if (str == NULL)
  {
    return 0;
  }

  // NET_GetAddressBytes() returns the raw, protocol-specific sockaddr, so parse
  // the human-readable string instead. Non-IPv4 (e.g. IPv6) addresses won't
  // match and resolve to 0.
  unsigned int a = 0, b = 0, c = 0, d = 0;
  if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
  {
    return 0;
  }

  return (UnsignedInt)(((a & 0xff) << 24) | ((b & 0xff) << 16) | ((c & 0xff) << 8) | (d & 0xff));
}

/**
 * Returns the next network command ID.
 */
UnsignedShort GenerateNextCommandID() {
	static UnsignedShort commandID = 64000;
	++commandID;
	return commandID;
}

/**
 * Returns true if this type of command requires a unique command ID.
 */
Bool DoesCommandRequireACommandID(NetCommandType type) {
	if ((type == NETCOMMANDTYPE_GAMECOMMAND) ||
			(type == NETCOMMANDTYPE_FRAMEINFO) ||
			(type == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(type == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(type == NETCOMMANDTYPE_RUNAHEADMETRICS) ||
			(type == NETCOMMANDTYPE_RUNAHEAD) ||
			(type == NETCOMMANDTYPE_CHAT) ||
			(type == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(type == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(type == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(type == NETCOMMANDTYPE_WRAPPER) ||
			(type == NETCOMMANDTYPE_FILE) ||
			(type == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(type == NETCOMMANDTYPE_FILEPROGRESS) ||
			(type == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(type == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(type == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(type == NETCOMMANDTYPE_FRAMERESENDREQUEST))
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Returns true if this type of network command requires an ack.
 */
Bool CommandRequiresAck(NetCommandMsg *msg) {
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_GAMECOMMAND) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMEINFO) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_RUNAHEADMETRICS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_RUNAHEAD) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_CHAT) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_WRAPPER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEPROGRESS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMERESENDREQUEST))
	{
		return TRUE;
	}
	return FALSE;
}

Bool IsCommandSynchronized(NetCommandType type) {
	if ((type == NETCOMMANDTYPE_GAMECOMMAND) ||
			(type == NETCOMMANDTYPE_FRAMEINFO) ||
			(type == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(type == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(type == NETCOMMANDTYPE_RUNAHEAD))
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Returns true if this type of network command requires the ack to be sent directly to the player
 * rather than going through the packet router.  This should really only be used by commands
 * used on the disconnect screen.
 */
Bool CommandRequiresDirectSend(NetCommandMsg *msg) {
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEPROGRESS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMERESENDREQUEST)) {
		return TRUE;
	}
	return FALSE;
}

AsciiString GetAsciiNetCommandType(NetCommandType type) {
	AsciiString s;
	if (type == NETCOMMANDTYPE_FRAMEINFO) {
		s.set("NETCOMMANDTYPE_FRAMEINFO");
	} else if (type == NETCOMMANDTYPE_GAMECOMMAND) {
		s.set("NETCOMMANDTYPE_GAMECOMMAND");
	} else if (type == NETCOMMANDTYPE_PLAYERLEAVE) {
		s.set("NETCOMMANDTYPE_PLAYERLEAVE");
	} else if (type == NETCOMMANDTYPE_RUNAHEADMETRICS) {
		s.set("NETCOMMANDTYPE_RUNAHEADMETRICS");
	} else if (type == NETCOMMANDTYPE_RUNAHEAD) {
		s.set("NETCOMMANDTYPE_RUNAHEAD");
	} else if (type == NETCOMMANDTYPE_DESTROYPLAYER) {
		s.set("NETCOMMANDTYPE_DESTROYPLAYER");
	} else if (type == NETCOMMANDTYPE_ACKBOTH) {
		s.set("NETCOMMANDTYPE_ACKBOTH");
	} else if (type == NETCOMMANDTYPE_ACKSTAGE1) {
		s.set("NETCOMMANDTYPE_ACKSTAGE1");
	} else if (type == NETCOMMANDTYPE_ACKSTAGE2) {
		s.set("NETCOMMANDTYPE_ACKSTAGE2");
	} else if (type == NETCOMMANDTYPE_FRAMEINFO) {
		s.set("NETCOMMANDTYPE_FRAMEINFO");
	} else if (type == NETCOMMANDTYPE_KEEPALIVE) {
		s.set("NETCOMMANDTYPE_KEEPALIVE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTCHAT) {
		s.set("NETCOMMANDTYPE_DISCONNECTCHAT");
	} else if (type == NETCOMMANDTYPE_CHAT) {
		s.set("NETCOMMANDTYPE_CHAT");
	} else if (type == NETCOMMANDTYPE_MANGLERQUERY) {
		s.set("NETCOMMANDTYPE_MANGLERQUERY");
	} else if (type == NETCOMMANDTYPE_MANGLERRESPONSE) {
		s.set("NETCOMMANDTYPE_MANGLERRESPONSE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTKEEPALIVE) {
		s.set("NETCOMMANDTYPE_DISCONNECTKEEPALIVE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTPLAYER) {
		s.set("NETCOMMANDTYPE_DISCONNECTPLAYER");
	} else if (type == NETCOMMANDTYPE_PACKETROUTERQUERY) {
		s.set("NETCOMMANDTYPE_PACKETROUTERQUERY");
	} else if (type == NETCOMMANDTYPE_PACKETROUTERACK) {
		s.set("NETCOMMANDTYPE_PACKETROUTERACK");
	} else if (type == NETCOMMANDTYPE_DISCONNECTVOTE) {
		s.set("NETCOMMANDTYPE_DISCONNECTVOTE");
	} else if (type == NETCOMMANDTYPE_PROGRESS) {
		s.set("NETCOMMANDTYPE_PROGRESS");
	} else if (type == NETCOMMANDTYPE_LOADCOMPLETE) {
		s.set("NETCOMMANDTYPE_LOADCOMPLETE");
	} else if (type == NETCOMMANDTYPE_TIMEOUTSTART) {
		s.set("NETCOMMANDTYPE_TIMEOUTSTART");
	} else if (type == NETCOMMANDTYPE_WRAPPER) {
		s.set("NETCOMMANDTYPE_WRAPPER");
	} else if (type == NETCOMMANDTYPE_FILE) {
		s.set("NETCOMMANDTYPE_FILE");
	} else if (type == NETCOMMANDTYPE_FILEANNOUNCE) {
		s.set("NETCOMMANDTYPE_FILEANNOUNCE");
	} else if (type == NETCOMMANDTYPE_FILEPROGRESS) {
		s.set("NETCOMMANDTYPE_FILEPROGRESS");
	} else if (type == NETCOMMANDTYPE_DISCONNECTFRAME) {
		s.set("NETCOMMANDTYPE_DISCONNECTFRAME");
	} else if (type == NETCOMMANDTYPE_DISCONNECTSCREENOFF) {
		s.set("NETCOMMANDTYPE_DISCONNECTSCREENOFF");
	} else if (type == NETCOMMANDTYPE_FRAMERESENDREQUEST) {
		s.set("NETCOMMANDTYPE_FRAMERESENDREQUEST");
	} else {
		s.set("UNKNOWN");
	}
	return s;
}
